using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Vector;

namespace XRay.Office.Spreadsheets.Layout;

/// <summary>
/// A point on a sheet, stated as a cell and an offset into it.
/// </summary>
/// <remarks>
/// Which is how all three formats anchor a drawing, and why a picture on a sheet cannot be placed
/// until the column widths are known: SpreadsheetML writes
/// <c>&lt;col&gt;/&lt;colOff&gt;/&lt;row&gt;/&lt;rowOff&gt;</c>, ODF writes a
/// <c>table:end-cell-address</c> with <c>table:end-x</c> and <c>table:end-y</c>, and BIFF's
/// <c>OBJ</c> client anchor states the offset as a fraction of the cell. Insert a column and every
/// picture to its right moves, which is the behaviour this shape exists to reproduce.
/// </remarks>
/// <param name="Column">The zero-based column.</param>
/// <param name="ColumnOffset">How far into that column the point sits.</param>
/// <param name="Row">The zero-based row.</param>
/// <param name="RowOffset">How far down that row it sits.</param>
public readonly record struct SheetCellPoint(
    int Column, Length ColumnOffset, int Row, Length RowOffset);

/// <summary>How a drawing is fastened to the sheet.</summary>
public enum SheetAnchorKind
{
    /// <summary>Both corners are cells: the drawing moves and resizes with them.</summary>
    TwoCell,

    /// <summary>The top left is a cell and the size is fixed: it moves but does not resize.</summary>
    OneCell,

    /// <summary>Neither corner is a cell: the drawing sits at a fixed place on the sheet.</summary>
    Absolute,
}

/// <summary>
/// A bitmap fill, stated without the rectangle it will be painted against.
/// </summary>
/// <remarks>
/// <para>
/// The one thing a <see cref="BitmapPaint"/> needs that a reader cannot supply is where the
/// tile grid starts, which is the shape's own box and is not resolved until the page's columns
/// are. So the reader answers the tile, its size and how opaque it is, and
/// <see cref="SheetShapeInk"/> composes the paint.
/// </para>
/// <para>
/// A <see cref="Tile"/> that is empty means the image covers the whole shape once —
/// <c>mso_fillPicture</c> and ODF's <c>stretch</c> — rather than repeating.
/// </para>
/// </remarks>
/// <param name="Image">The tile.</param>
/// <param name="Tile">One tile's size on the page, or the default to stretch once.</param>
/// <param name="Opacity">How opaque the fill is, from zero to one.</param>
public sealed record SheetShapeTexture(RasterImage Image, DocSize Tile, double Opacity = 1);

/// <summary>
/// One drawing anchored on a sheet: a picture, or a chart recorded but not drawn.
/// </summary>
/// <remarks>
/// <para>
/// <strong>The image is carried encoded.</strong> <see cref="RasterImage.Encoded"/> holds the
/// bytes the file stored and nothing else; whichever backend needs pixels decodes them. That is
/// the layering rule rather than a convenience — a reader that decoded would put a codec in the
/// extraction path, which every caller pays for and almost none wants.
/// </para>
/// <para>
/// <strong>A chart carries a model rather than a picture.</strong> <see cref="IsChart"/> says a
/// frame holds one; <see cref="Chart"/> holds what it takes to draw, when the chart is of a kind
/// the layout engine draws. The two are separate because they answer different questions: a chart
/// of an undrawn kind still sets the flag, so "there is a chart here" stays distinguishable from
/// "there is nothing here" even where the picture is missing.
/// </para>
/// </remarks>
public sealed record SheetDrawing
{
    /// <summary>How the drawing is fastened.</summary>
    public SheetAnchorKind Anchor { get; init; }

    /// <summary>Its top-left corner. Unused for <see cref="SheetAnchorKind.Absolute"/>.</summary>
    public SheetCellPoint From { get; init; }

    /// <summary>
    /// Its bottom-right corner, for a two-cell anchor.
    /// </summary>
    public SheetCellPoint To { get; init; }

    /// <summary>Its size, for a one-cell or absolute anchor.</summary>
    public DocSize Extent { get; init; }

    /// <summary>Its position on the sheet, for an absolute anchor.</summary>
    public DocPoint Position { get; init; }

    /// <summary>The picture, still encoded, or null when there is nothing to paint.</summary>
    public RasterImage? Image { get; init; }

    /// <summary>
    /// The picture as a display list — an SVG, a WMF, an EMF or an EMF+ — or null when it is a raster.
    /// </summary>
    /// <remarks>
    /// <para>
    /// One or the other, never both, for every source but a DrawingML <c>svgBlip</c>: that one names an
    /// SVG and a raster fallback on the same <c>a:blip</c>, and both are kept so an empty decode still
    /// leaves a picture on the sheet.
    /// </para>
    /// <para>
    /// Decoded when something draws it and not while the sheet is read. That is the same rule as
    /// <see cref="RasterImage.Encoded"/>'s and for a sharper reason: the first metafile decode in a
    /// process costs about a second of font resolution, which a caller asking only for cell values must
    /// not pay.
    /// </para>
    /// </remarks>
    public Lazy<VectorImage>? Vector { get; init; }

    /// <summary>
    /// How much of the picture each edge throws away, or <see cref="PictureCropFractions.None"/>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>Carried rather than applied, because a sheet's drawing has no rectangle until the
    /// page does.</strong> A two-cell anchor's size is the columns and rows between its corners,
    /// which the reader cannot know; <see cref="SheetPageGraphics"/> resolves the box and applies
    /// the crop there. Cropping is drawing the picture into a <em>larger</em> rectangle and
    /// clipping to the anchor, so both halves have to happen in the same place.
    /// </para>
    /// <para>
    /// BIFF states it as Escher properties 256–259 (<c>EscherPicture.Crop</c>). SpreadsheetML's
    /// <c>a:srcRect</c> and ODF's <c>fo:clip</c> say the same thing and are not read yet, which is
    /// why this is fractions rather than anything Escher-shaped.
    /// </para>
    /// </remarks>
    public PictureCropFractions Crop { get; init; }

    /// <summary>
    /// How opaque the picture is painted, as a fraction of one; 1 when the file states nothing.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A picture on a sheet may be a watermark, and a watermark drawn opaque erases the
    /// page.</strong> SpreadsheetML states it as <c>xdr:blipFill/a:blip/a:alphaModFix/@amt</c>, the
    /// same element a slide's <c>p:blipFill</c> uses, and <c>oox</c> puts it on the graphic as
    /// <c>FillTransparence</c> without caring which family read it
    /// (<c>oox/source/drawingml/fillproperties.cxx</c>, <c>moAlphaModFix</c>).
    /// </para>
    /// <para>
    /// Measured on <c>SIL_TDB648.xlsx</c>, whose <c>General Info</c> sheet anchors a full-width
    /// product photograph at <c>amt="20000"</c> over eighteen rows of text: LibreOffice paints it
    /// as a pale ghost the type reads straight across, and painting it at full strength hides
    /// about 85% of the body copy. **No gate column moves for it** — the words stay in the PDF's
    /// text layer underneath, so the document's word count is right to two words while the page is
    /// unreadable. The reference's faded pixels confirm the number rather than merely the fact:
    /// its bluish areas sample at RGB 217/222/234 against our opaque 74/97/157, and
    /// <c>0.2·74 + 0.8·255 = 219</c>, <c>0.2·97 + 0.8·255 = 223</c>, <c>0.2·157 + 0.8·255 = 235</c>.
    /// </para>
    /// <para>
    /// It is <em>not</em> a paint-order defect, which is what it looks like: Calc prints
    /// <c>SC_LAYER_BACK</c> before the cell text and <c>SC_LAYER_FRONT</c> after
    /// (<c>printfun.cxx:1651</c> and <c>:1699</c>), a sheet picture is on the front layer, and
    /// <see cref="SheetPageGraphics"/> already draws after the strings for that reason. Moving it
    /// behind the text would have hidden the defect on this document and been wrong on every
    /// other one.
    /// </para>
    /// </remarks>
    public double Opacity { get; init; } = 1;

    /// <summary>The shape's name, as the file records it.</summary>
    public string? Name { get; init; }

    /// <summary>Its alternative text or description, where the file records one.</summary>
    public string? Description { get; init; }

    /// <summary>True when the drawing is a chart, whether or not it can be drawn.</summary>
    public bool IsChart { get; init; }

    /// <summary>
    /// The chart's model, ready to lay out, or null when the frame holds no drawable chart.
    /// </summary>
    /// <remarks>
    /// Read in the same pass as the anchor, because the anchor is what gives the chart a rectangle
    /// and there is no second walk of the drawing part on the rendering path. Null both for a frame
    /// that is not a chart at all and for a chart whose type the engine does not draw — a doughnut,
    /// say — which is why <see cref="IsChart"/> is not simply this being non-null.
    /// </remarks>
    public ChartPlot? Chart { get; init; }

    /// <summary>The text inside the shape, or null when it holds none.</summary>
    /// <remarks>
    /// A text box is a shape carrying nothing but this, and it is the only content on the sheet
    /// that no walk of the cells can find — see <see cref="SheetShapeText"/>.
    /// </remarks>
    public SheetShapeText? Text { get; init; }

    /// <summary>True when the drawing is hidden and therefore not printed.</summary>
    public bool IsHidden { get; init; }

    /// <summary>
    /// False when the file marks the object as one that does not print.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Separate from <see cref="IsHidden"/>, and deliberately, because the two differ in the one
    /// place it matters: both stop the object being painted, and <em>neither</em> stops it moving
    /// the page break. <see cref="SheetDrawingArea.Extend"/> counts every drawing it is given, as
    /// <c>ScDrawLayer::GetPrintArea</c> does (<c>sc/source/core/data/drwlayer.cxx:1395-1424</c>,
    /// whose only exclusion is the hidden-comment layer) — so an unprintable button anchored to
    /// the right of the last cell still widens the printed block, and dropping it from the model
    /// instead of flagging it would quietly narrow the print area of every workbook carrying one.
    /// </para>
    /// <para>
    /// In BIFF this is <c>ftCmo</c>'s <c>fPrint</c> bit and it is acted on for form controls
    /// alone — see <c>XlsDrawing.IsFormControl</c> for why, and for what LibreOffice does with a
    /// plain shape carrying the same bit clear.
    /// </para>
    /// </remarks>
    public bool IsPrintable { get; init; } = true;

    /// <summary>The colour the shape's box is filled with, or null when it is not filled.</summary>
    /// <remarks>
    /// <para>
    /// A shown cell comment is the case that put this here — a caption drawn as bare text over the
    /// cells under it is unreadable where they hold anything — but it is no longer only that.
    /// Every worksheet shape carries its own fill and outline and every one of them was read as
    /// nothing: censused whole-corpus, <b>644 <c>xdr:sp</c> in 49 documents</b>, of which 421
    /// state an <c>a:solidFill</c> of their own and 54 more take one out of the theme's format
    /// matrix. Nothing reads this for a picture or a chart, both of which paint their own ground.
    /// </para>
    /// <para>
    /// The interior and the outline are two properties rather than one paint because a shape may
    /// state either alone, and because an absent fill is not a white one: a shape stating
    /// <c>a:noFill</c> shows whatever is under it, which on a sheet is the cells.
    /// </para>
    /// </remarks>
    public Colour? Fill { get; init; }

    /// <summary>
    /// The ramp the box is filled with, or null when its fill is flat or absent.
    /// </summary>
    /// <remarks>
    /// Carried unplaced, exactly as <c>PageFrame.Gradient</c> is and for the same reason: a
    /// <see cref="GradientPaint"/> holds absolute points and a sheet drawing has no rectangle
    /// until the page's own columns are known. <see cref="SheetPageGraphics"/> supplies the
    /// rectangle. Set only where <see cref="Fill"/> is not — a fill is one kind or the other.
    /// </remarks>
    public GradientDescription? Gradient { get; init; }

    /// <summary>
    /// The tile the box is filled with, or null when its fill is not a bitmap.
    /// </summary>
    /// <remarks>
    /// Carried unplaced for the reason <see cref="Gradient"/> is: a <see cref="BitmapPaint"/>
    /// states the tile's origin in page coordinates and a sheet drawing has no rectangle until
    /// the page's own columns are known. Set only where <see cref="Fill"/> and
    /// <see cref="Gradient"/> are not — a fill is one kind or another.
    /// </remarks>
    public SheetShapeTexture? Texture { get; init; }

    /// <summary>The colour of the box's outline, or null when it has none.</summary>
    public Colour? Stroke { get; init; }

    /// <summary>
    /// How wide the outline is stroked; zero for a hairline.
    /// </summary>
    /// <remarks>
    /// Zero and absent are one answer here on purpose. A comment caption's border is
    /// <c>svg:stroke-width="0in"</c> in LibreOffice's own export, which is a hairline rather than
    /// an absent line, and a DrawingML <c>a:ln</c> stating no <c>w</c> takes the theme's width —
    /// which the reader resolves before setting this, so a zero arriving here really is "as thin
    /// as the device draws".
    /// </remarks>
    public Length StrokeWidth { get; init; }

    /// <summary>
    /// The preset whose outline the fill and stroke are painted through, or null for the box.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A worksheet shape is not usually a rectangle.</strong> The 644 corpus shapes name
    /// <b>28 distinct presets</b> — <c>star5</c>, <c>heart</c>, <c>cloud</c>,
    /// <c>irregularSeal1</c>, <c>lightningBolt</c> and the rest — and only 206 of them are
    /// <c>rect</c>, so filling the anchor's box instead draws a coloured rectangle where the file
    /// states a star. <see cref="XRay.Office.Ooxml.DrawingML.CustomShapeGeometry"/> resolves every
    /// one of them and has since the slide side needed it.
    /// </para>
    /// <para>
    /// <c>rect</c> itself is deliberately left null by the readers, which is what
    /// <c>DocxFrames.PresetGeometry</c> does and for the same reason: it evaluates to the bounding
    /// box the fallback already paints, so naming it would build a four-point path to arrive
    /// exactly where not naming it arrives.
    /// </para>
    /// <para>
    /// Distinct from <see cref="SheetShapeText.Preset"/>, which answers a different question about
    /// the same attribute — that one is the rectangle the <em>text</em> is laid out in, and a
    /// preset's text rectangle is usually not its outline. A shape carrying both text and ink sets
    /// both.
    /// </para>
    /// </remarks>
    public string? Preset { get; init; }

    /// <summary>The <c>a:avLst</c> the shape states, overriding the preset's own defaults.</summary>
    public IReadOnlyDictionary<string, double>? Adjustments { get; init; }

    /// <summary>True when the shape's geometry is mirrored across its vertical centre line.</summary>
    /// <remarks>
    /// <para>
    /// <strong>A connector states its direction as a flip, and drawing one without the flip puts
    /// the elbow on the wrong side.</strong> <c>a:xfrm/@flipH</c> and <c>@flipV</c> mirror the
    /// shape's own coordinate space before <c>@rot</c> turns it, so the three compose — the
    /// <c>bentConnector3</c> that joins an organisation chart's boxes is written
    /// <c>rot="10800000" flipH="1" flipV="1"</c>, whose net effect is the identity, and honouring
    /// the rotation alone leaves the path mirrored in both axes.
    /// </para>
    /// <para>
    /// Applied to the resolved outline rather than to the anchor's rectangle, because the
    /// rectangle is symmetric and only the geometry inside it is not. A shape with no preset is
    /// unaffected either way.
    /// </para>
    /// </remarks>
    public bool FlipHorizontal { get; init; }

    /// <inheritdoc cref="FlipHorizontal"/>
    /// <summary>True when the shape's geometry is mirrored across its horizontal centre line.</summary>
    public bool FlipVertical { get; init; }

    /// <summary>
    /// True when the anchor describes the shape's rectangle <em>after</em> a quarter turn, so the
    /// rectangle has to be reflected in the line <c>y = x</c> before the shape is drawn in it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>Excel rewrites a shape's anchor cells when it is turned through a quarter, and
    /// Calc undoes that before drawing anything.</strong>
    /// <c>sc/source/filter/oox/drawingfragment.cxx</c>:299-330 tests the shape's own rotation for
    /// <c>[45°, 135°)</c> or <c>[225°, 315°)</c> and, inside that range, moves the anchor
    /// rectangle by <c>X += (w − h)/2</c>, <c>Y += (h − w)/2</c> and swaps its width and height —
    /// its comment says the anchor Excel wrote "contains the original not-rotated shape" only
    /// outside that range, and that rotating the already-rotated rectangle "is an incorrect 180
    /// degrees rotation". Which is exactly the symptom: without it an elbow connector's short leg
    /// points away from the box it joins.
    /// </para>
    /// <para>
    /// It is a property of the <em>anchor</em> rather than of the geometry, so it moves the
    /// rectangle every part, picture and text body inside the drawing is placed in, and it is set
    /// by the SpreadsheetML reader alone: ODF states a turned shape's rectangle with a
    /// <c>draw:transform</c>, which needs no such correction, and BIFF's client anchor states the
    /// unrotated rectangle outright. <b>Reach: 31 shapes in 3 corpus documents</b> — 24
    /// <c>xdr:sp</c>, 6 <c>xdr:cxnSp</c> and one <c>xdr:pic</c>.
    /// </para>
    /// </remarks>
    public bool QuarterTurnedAnchor { get; init; }

    /// <summary>True when there is a fill or an outline to paint.</summary>
    public bool HasInk
        => Fill is not null || Gradient is not null || Texture is not null || Stroke is not null;

    /// <summary>
    /// The cell a shown comment's caption hangs off, or null for every other drawing.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A caption is placed relative to its cell, not to the cell its anchor names.</strong>
    /// Calc stores a shown comment as an offset from the commented cell —
    /// <c>ScNoteUtil::CreateNoteData</c> keeps <c>maCaptionOffset</c> and <c>maCaptionSize</c> and
    /// throws the absolute rectangle away (<c>sc/source/core/data/postit.cxx:966-973</c>) — and
    /// <c>ScPostIt::CreateCaptionFromInitData</c> puts it back at
    /// <c>cellRect.Right() + offset</c> (<c>:1046-1053</c>). The two differ on every page that
    /// repeats a print title: a caption anchored in row 2 and belonging to a cell in row 1 is
    /// drawn in the repeated band on <em>every</em> page, because the band prints row 1 and the
    /// object hangs off it.
    /// </para>
    /// <para>
    /// Measured on <c>Application_Compliance_Checklist_5_Apr_2021.xlsx</c>, whose
    /// <c>App. Compliance Checklist</c> repeats row 1 and carries four shown comments on it:
    /// LibreOffice draws all four on each of the sheet's six pages, at the same place on every
    /// one, and anchoring them where their VML says drew them on the first page alone.
    /// </para>
    /// </remarks>
    public (int Column, int Row)? NoteCell { get; init; }

    /// <summary>
    /// The shapes inside the drawing, in fractions of its own rectangle, or empty.
    /// </summary>
    /// <remarks>
    /// Only <see cref="SheetDrawingBounds"/> reads this, and only to answer the one question the
    /// anchor cannot: how far the drawing's <em>bounding</em> rectangle reaches. See that class
    /// for why a turned shape inside a group makes the two differ, and why the parts have to be
    /// carried rather than folded into a fixed inset at read time.
    /// </remarks>
    public IReadOnlyList<SheetDrawingPart> Parts { get; init; } = [];
}

/// <summary>
/// One leaf shape inside a drawing, stated as fractions of the drawing's own rectangle.
/// </summary>
/// <remarks>
/// Fractions rather than lengths because a two-cell anchor has no rectangle until the grid is
/// known: the reader sees the DrawingML tree and the layout sees the columns, and this is what
/// passes between them. A group's <c>a:chOff</c>/<c>a:chExt</c> mapping is already applied, so a
/// part is positioned in the anchored shape's own frame however deeply it was nested.
/// </remarks>
/// <param name="X">Its left edge, as a fraction of the drawing's width from the drawing's left.</param>
/// <param name="Y">Its top edge, as a fraction of the drawing's height.</param>
/// <param name="Width">Its width, as a fraction of the drawing's.</param>
/// <param name="Height">Its height, as a fraction of the drawing's.</param>
/// <param name="Degrees">
/// How far it is turned clockwise, in degrees. The one thing the fractions cannot express, and
/// the reason a part was first carried at all.
/// </param>
public readonly record struct SheetDrawingPart(
    double X, double Y, double Width, double Height, double Degrees)
{
    /// <summary>The part's own picture, still encoded, or null when it holds none.</summary>
    /// <remarks>
    /// <para>
    /// <strong>A picture inside a group was read for its bounds and never drawn.</strong> A
    /// spreadsheet anchor may hold an <c>xdr:grpSp</c> instead of an <c>xdr:pic</c>, and the group
    /// carries no picture of its own — so the anchor produced a drawing with nothing to paint and
    /// the whole group vanished, while still widening the print area because
    /// <see cref="SheetDrawingArea"/> counts it.
    /// </para>
    /// <para>
    /// Measured on <c>SIL_TDB648.xlsx</c>, whose eleven sheet drawings each hold one group of
    /// fourteen copies of the same faded, turned <c>Honeywell</c> wordmark: the reference's PDF
    /// carries that image on 86 of its 88 pages and ours carried it on none.
    /// </para>
    /// </remarks>
    public RasterImage? Image { get; init; }

    /// <summary>The part's picture as a display list, or null when it is a raster or absent.</summary>
    public Lazy<VectorImage>? Vector { get; init; }

    /// <summary>How much of the part's picture each edge throws away.</summary>
    public PictureCropFractions Crop { get; init; }

    /// <summary>How opaque the part's picture is painted, as a fraction of one.</summary>
    /// <remarks>
    /// The watermark case again, and the reason the fourteen copies in <c>SIL_TDB648.xlsx</c> are
    /// legible rather than a wall of red: each states <c>a:alphaModFix amt="20000"</c>.
    /// </remarks>
    public double Opacity { get; init; } = 1;

    /// <summary>The colour this leaf's own box is filled with, or null when it is not filled.</summary>
    /// <remarks>
    /// <para>
    /// <strong>A grouped shape's fill is the leaf's, and there is one of it per leaf.</strong>
    /// <see cref="SheetDrawing.Fill"/> belongs to the anchor, and an <c>xdr:grpSp</c> anchor holds
    /// many shapes with many fills — which is why the ink is stated here as well rather than only
    /// there. Censused whole-corpus, <b>174 of the 644 worksheet <c>xdr:sp</c> sit inside a
    /// group</b>, in 6 documents; reading the anchor alone would draw none of them.
    /// </para>
    /// <para>
    /// It is also where a <em>turned</em> top-level shape's ink goes, because a part is the one
    /// thing that carries an angle: <c>XlsxDrawings.Parts</c> already answers a single part
    /// restating the frame for a shape with an <c>a:xfrm/@rot</c>, and painting the fill on the
    /// drawing would draw it upright inside the turned shape's bounding box.
    /// </para>
    /// </remarks>
    public Colour? Fill { get; init; }

    /// <summary>The ramp this leaf is filled with, or null when its fill is flat or absent.</summary>
    public GradientDescription? Gradient { get; init; }

    /// <summary>The colour of this leaf's outline, or null when it has none.</summary>
    public Colour? Stroke { get; init; }

    /// <summary>How wide that outline is stroked; zero for a hairline.</summary>
    public Length StrokeWidth { get; init; }

    /// <summary>The preset the leaf's ink is painted through, or null for its box.</summary>
    public string? Preset { get; init; }

    /// <summary>The <c>a:avLst</c> the leaf states, overriding the preset's own defaults.</summary>
    public IReadOnlyDictionary<string, double>? Adjustments { get; init; }

    /// <summary>True when there is something here to paint.</summary>
    public bool HasPicture => Image is not null || Vector is not null;

    /// <inheritdoc cref="SheetDrawing.FlipHorizontal"/>
    public bool FlipHorizontal { get; init; }

    /// <inheritdoc cref="SheetDrawing.FlipVertical"/>
    public bool FlipVertical { get; init; }

    /// <summary>True when this leaf carries a fill or an outline of its own.</summary>
    public bool HasInk => Fill is not null || Gradient is not null || Stroke is not null;
}

/// <summary>
/// The drawings anchored on one sheet, in the order the file lists them.
/// </summary>
/// <remarks>
/// Order is z-order and is kept: a picture over a picture is decided by nothing else, and all
/// three formats state it the same way — the later shape is in front.
/// </remarks>
public sealed class SheetDrawings
{
    private readonly List<SheetDrawing> _items;

    /// <summary>Creates a sheet's drawings from the shapes read, in file order.</summary>
    /// <param name="items">The drawings.</param>
    public SheetDrawings(IEnumerable<SheetDrawing> items)
    {
        ArgumentNullException.ThrowIfNull(items);
        _items = [.. items];
    }

    /// <summary>A sheet with nothing drawn on it.</summary>
    public static SheetDrawings Empty { get; } = new([]);

    /// <summary>The drawings, back to front.</summary>
    public IReadOnlyList<SheetDrawing> Items => _items;

    /// <summary>True when the sheet has none.</summary>
    public bool IsEmpty => _items.Count == 0;
}
