using System.Xml.Linq;

namespace Paperless.Presentations.Ooxml;

/// <summary>One paragraph a presentation node took from a data node, and the level it took it at.</summary>
/// <param name="Paragraph">The <c>a:p</c> as the data model states it.</param>
/// <param name="Level">
/// The outline level, or −1 for "leave the paragraph's own". It is the data node's depth in the
/// author's tree, so a diagram's sub-bullets come out indented without the layout saying so.
/// </param>
internal readonly record struct DiagramTextParagraph(XElement Paragraph, int Level);

/// <summary>
/// The drawn geometry of every shape cloned from one <c>dgm:shape</c>, shared between them.
/// </summary>
/// <remarks>
/// <para>
/// <strong>It is shared on purpose, and the sharing is visible in the output.</strong> A
/// <c>dgm:layoutNode</c> inside a <c>forEach</c> whose iteration count is one produces several
/// shapes for the <em>same</em> presentation point — a five-node cycle makes five
/// <c>sibTrans</c> shapes, all standing for the first one — and only the last of them is in the
/// map the layouting walk looks shapes up in, so only the last runs the <c>conn</c> algorithm
/// that turns the placeholder into an arrow. LibreOffice draws all five as arrows anyway,
/// because <c>Shape</c>'s copy constructor takes <c>mpCustomShapePropertiesPtr</c> by reference
/// (<c>oox/source/drawingml/shape.cxx:210</c>, under the comment "cloned shape shares all
/// properties by reference, don't change them!") and the <c>conn</c> algorithm changes exactly
/// that. Giving each clone its own preset instead draws four of five connectors as plain
/// rectangles on <c>smartart-cycle.pptx</c>.
/// </para>
/// <para>
/// The <em>sub-type</em> is not shared, and the difference matters: it is per shape in
/// LibreOffice too, so a connector the <c>conn</c> algorithm never reached still reads as a
/// connector to the cycle and hierarchy algorithms that ask "is this child a <c>conn</c>" while
/// drawing as the arrow its sibling turned into.
/// </para>
/// </remarks>
internal sealed class DiagramGeometry
{
    /// <summary>The preset actually drawn, which the <c>conn</c> algorithm may replace.</summary>
    public string PresetType { get; set; } = "";
}

/// <summary>
/// A shape the layout-atom evaluator produced, before it becomes PresentationML.
/// </summary>
/// <remarks>
/// <para>
/// Positions and sizes are EMUs relative to the parent shape, which is what the algorithms
/// compute and what LibreOffice's <c>oox::drawingml::Shape</c> holds. They stay
/// <see cref="int"/> rather than becoming a length type because the arithmetic is what is being
/// ported: the algorithms divide integers by doubles and assign the result back to an integer,
/// so every intermediate truncates, and a rewrite in exact arithmetic quietly disagrees with the
/// reference by a few hundred EMUs per step.
/// </para>
/// <para>
/// A shape with no <see cref="PresetType"/> is a <em>group</em> — a <c>dgm:shape</c> with no
/// <c>type</c> attribute — and draws nothing of its own. Groups still matter to the layout,
/// because spacing between real shapes is expressed as an empty group taking up room, which is
/// why they are only removed once the algorithm above them has run.
/// </para>
/// </remarks>
internal sealed class DiagramShape
{
    /// <summary>The <c>dgm:layoutNode</c> name that made it, which constraints refer to.</summary>
    public string InternalName { get; set; } = "";

    /// <summary>
    /// The shape's sub-type: the preset geometry the layout definition stated, per shape.
    /// </summary>
    /// <remarks>
    /// What an algorithm asks when it wants to know whether a child is a connector. Not what is
    /// drawn — see <see cref="Geometry"/>, which the clones of one <c>dgm:shape</c> share.
    /// </remarks>
    public string PresetType { get; set; } = "";

    /// <summary>The drawn geometry, shared with every other clone of the same template.</summary>
    public DiagramGeometry Geometry { get; set; } = new();

    /// <summary>True when the layout definition asked for the geometry not to be drawn.</summary>
    public bool HideGeometry { get; set; }

    /// <summary>
    /// True when the shape is a grouping shape rather than a drawn one — a <c>dgm:shape</c> with
    /// no <c>type</c> at all.
    /// </summary>
    /// <remarks>
    /// Not the same as having no geometry: a <c>hideGeom</c> shape has none either and is still
    /// a shape, because it is where a node's text goes.
    /// </remarks>
    public bool IsGroup { get; set; }

    /// <summary>The preset actually drawn, which is nothing for a group or a hidden geometry.</summary>
    public string DrawnPreset => IsGroup || HideGeometry ? "" : Geometry.PresetType;

    /// <summary>Offset from the parent shape's top-left corner, in EMUs.</summary>
    public int X { get; set; }

    /// <summary>Offset from the parent shape's top-left corner, in EMUs.</summary>
    public int Y { get; set; }

    /// <summary>Width in EMUs.</summary>
    public int Width { get; set; }

    /// <summary>Height in EMUs.</summary>
    public int Height { get; set; }

    // AspectRatio, VerticalShapesCount, DataNodeType and CustomText below are set by the
    // creation walk and read by algorithms that are not ported yet — snake divides by the aspect
    // ratio, the hierarchy pair by the vertical count, and autofit by the custom-text flag. They
    // are kept because computing them is part of the walk rather than of those algorithms, so
    // dropping them would mean re-deriving them from a different place later.

    /// <summary>
    /// Rotation about the shape's centre, in sixtieths of a thousandth of a degree.
    /// </summary>
    /// <remarks>
    /// Only the algorithms set this — a connector turned to face along a linear layout, or a
    /// node turned around a cycle — and it is an ordinary shape rotation that maps straight onto
    /// <c>a:xfrm/@rot</c>. It is <em>not</em> <see cref="DiagramRotation"/>.
    /// </remarks>
    public int Rotation { get; set; }

    /// <summary>
    /// The <c>dgm:shape/@rot</c> the layout definition states, which rotates the geometry inside
    /// the box rather than the box.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The two are genuinely different transforms and conflating them is visible. LibreOffice
    /// applies this one to the <em>unit square, before scaling</em> — "Special for SmartArt
    /// import. Rotate diagram's shape around object's center before sizing",
    /// <c>oox/source/drawingml/shape.cxx:1099-1105</c> — so the shape's bounding box is
    /// unchanged and only its contents turn. Emitting it as an <c>a:xfrm/@rot</c> instead
    /// rotates the box: on <c>smartart-vertical-block-list.pptx</c> that turns a
    /// 307.2 × 82.6 pt band into an 82.6 × 307.2 pt column about the same centre, which is 112 pt
    /// out of place in both directions.
    /// </para>
    /// <para>
    /// A half turn is expressible without rotating the box — it is a horizontal flip and a
    /// vertical one — and is emitted that way. A quarter turn is not, and is dropped.
    /// </para>
    /// </remarks>
    public int DiagramRotation { get; set; }

    /// <summary>The <c>ar</c> parameter of the algorithm that will lay out the children.</summary>
    public double AspectRatio { get; set; }

    /// <summary>How many shapes stack vertically inside, which the hierarchy algorithms divide by.</summary>
    public int VerticalShapesCount { get; set; } = 1;

    /// <summary>The data-point type this shape presents — <c>node</c>, <c>sibTrans</c>, …</summary>
    public string DataNodeType { get; set; } = "";

    /// <summary>Children, in the order the algorithms will lay them out.</summary>
    public List<DiagramShape> Children { get; } = [];

    /// <summary>The <c>dgm:spPr</c> of the data point at depth zero, whose fill overrides the style's.</summary>
    public XElement? ShapeProperties { get; set; }

    /// <summary>The paragraphs this shape draws, gathered from every data node it presents.</summary>
    public List<DiagramTextParagraph> Text { get; } = [];

    /// <summary>The <c>a:bodyPr</c> and <c>a:lstStyle</c> the first data node's text body carried.</summary>
    public XElement? TextBodySource { get; set; }

    /// <summary>The quick-style and colour-transform label.</summary>
    public string StyleLabel { get; set; } = "";

    /// <summary>Which shape of its label this is, which the colour list cycles over.</summary>
    public int StyleIndex { get; set; }

    /// <summary>How far up the stacking order the layout definition moves it.</summary>
    public int ZOrderOffset { get; set; }

    /// <summary>Its place in the stacking order, assigned before the offsets are applied.</summary>
    public int ZOrder { get; set; }

    /// <summary>The font size the <c>tx</c> algorithm's <c>primFontSz</c> constraint asked for.</summary>
    public double FontSize { get; set; }

    /// <summary>Where the text sits vertically — <c>t</c>, <c>mid</c> or <c>b</c>.</summary>
    public string TextAnchor { get; set; } = "mid";

    /// <summary>
    /// How far the label turns inside the node, in sixtieth-thousandths of a degree.
    /// </summary>
    /// <remarks>
    /// What <c>autoTxRot</c> resolves to, and always a whole number of quarter turns. It is
    /// emitted as <c>a:bodyPr/@rot</c>, so it reaches the layouter down the same path an authored
    /// deck's would.
    /// </remarks>
    public int TextRotation { get; set; }

    /// <summary>The paragraph alignment the algorithm decided, or null to leave the text's own.</summary>
    public string? TextAlignment { get; set; }

    /// <summary>The level at and below which the <c>tx</c> algorithm draws bullets.</summary>
    public int BulletLevel { get; set; } = int.MaxValue;

    /// <summary>Text insets in EMUs: left, top, right, bottom.</summary>
    public (int Left, int Top, int Right, int Bottom) TextInsets { get; set; }

    /// <summary>True when any of the four insets was stated by a constraint.</summary>
    public bool HasTextInsets { get; set; }

    /// <summary>True when the author formatted the text, which is what stops the fit being solved.</summary>
    public bool CustomText { get; set; }

    /// <summary>
    /// True when the <c>tx</c> algorithm turned shrink-to-fit on for this node's text.
    /// </summary>
    /// <remarks>
    /// A diagram's labels are autofitted whether or not the file asks: the <c>tx</c> algorithm
    /// sets <c>TextFitToSizeType_AUTOFIT</c> on every node whose text the author did not format
    /// (<c>diagramlayoutatoms.cxx:1723-1728</c>, under its own comment "No customized text
    /// properties: enable autofit"). That is what makes a <c>primFontSz</c> of 65 pt an
    /// <em>upper bound</em> rather than a size, and it is the whole of the divergence the
    /// evaluated path had left: 65 pt drawn against the reference's 49 on
    /// <c>smartart-maxdepth.pptx</c>.
    /// </remarks>
    public bool AutoFitText { get; set; }
}
