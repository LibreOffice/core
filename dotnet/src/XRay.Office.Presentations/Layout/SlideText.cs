using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Ooxml.DrawingML;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Layout;
using XRay.Office.Vector;

namespace XRay.Office.Presentations.Layout;

/// <summary>Where a text body sits inside its shape, vertically.</summary>
/// <remarks>
/// DrawingML's <c>a:bodyPr/@anchor</c> and ODF's <c>draw:textarea-vertical-align</c>, which
/// spell the same three positions differently. Justified anchoring — spreading the paragraphs to
/// fill the shape — is a fourth value both formats have and neither corpus deck uses; it is read
/// as <see cref="Top"/> rather than silently as <see cref="Middle"/>, which is what LibreOffice
/// falls back to for a single paragraph anyway.
/// </remarks>
public enum TextAnchor
{
    /// <summary>The text block starts at the top of the text rectangle.</summary>
    Top = 0,

    /// <summary>It is centred vertically.</summary>
    Middle,

    /// <summary>It ends at the bottom.</summary>
    Bottom,
}

/// <summary>
/// A shape's text body before layout: its paragraphs, its insets, and how it is anchored.
/// </summary>
/// <remarks>
/// The presentation family's equivalent of the word processor's <c>PageParagraph</c> list, and
/// deliberately its own type rather than a reuse: a slide's text is bounded by the shape rather
/// than flowed down a page, so what layout needs to know is the rectangle and the anchor, and
/// none of the pagination properties — widows, keep-with-next, page breaks — mean anything.
/// </remarks>
public sealed record SlideTextBody
{
    /// <summary>The paragraphs, in order.</summary>
    public IReadOnlyList<SlideParagraph> Paragraphs { get; init; } = [];

    /// <summary>
    /// The insets between the shape's text rectangle and the text.
    /// </summary>
    /// <remarks>
    /// Defaulted to DrawingML's own defaults — 0.1 inch left and right, 0.05 inch top and bottom
    /// (<c>a:bodyPr</c>'s <c>lIns</c>, <c>tIns</c>, <c>rIns</c>, <c>bIns</c>) — because a body
    /// that states none gets exactly those, and a reader defaulting them to zero puts every line
    /// of every unstated text box 7.2 pt too far left.
    /// </remarks>
    public Margins Insets { get; init; } = DefaultInsets;

    /// <summary>DrawingML's default text insets: 91440 EMU across, 45720 EMU down.</summary>
    public static Margins DefaultInsets { get; } = new(
        Length.FromEmu(91440), Length.FromEmu(45720),
        Length.FromEmu(91440), Length.FromEmu(45720));

    /// <summary>Where the block sits vertically.</summary>
    public TextAnchor Anchor { get; init; }

    /// <summary>
    /// How far the text is turned inside the shape, clockwise, in radians.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>a:bodyPr/@rot</c>, and what a SmartArt <c>autoTxRot</c> resolves to. It is <em>not</em>
    /// the shape's own rotation: the shape stays where it is and only its text turns, which is
    /// why it belongs to the body rather than to the placement. LibreOffice keeps the two apart
    /// the same way, as <c>TextPreRotateAngle</c> beside <c>RotateAngle</c>.
    /// </para>
    /// <para>
    /// <c>a:bodyPr/@vert</c> arrives here too, as a quarter turn added to <c>@rot</c> — a body
    /// that reads downwards is a body whose frame is transposed and whose runs travel with a
    /// quarter-turn matrix, which is the same quantity by a different spelling. <c>vert</c> and
    /// <c>eaVert</c> add a quarter clockwise and <c>vert270</c> a quarter the other way; see
    /// <c>PptxTextBody.Vertical</c> for what was measured and for the two values that are
    /// deliberately not turns.
    /// </para>
    /// <para>
    /// A quarter turn swaps the text rectangle's width and height about its centre, because the
    /// lines then run down the shape rather than across it; a half turn leaves the rectangle
    /// alone. Only multiples of a quarter turn arise: <c>autoTxRot</c> produces nothing else.
    /// </para>
    /// </remarks>
    public double Rotation { get; init; }

    /// <summary>
    /// Whether the text is shrunk until it fits the shape — DrawingML's <c>a:normAutofit</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// When this is set the layouter solves the fit itself and <see cref="FontScale"/> is
    /// <em>ignored</em>, because that is what the reference does: LibreOffice 24.2 reads
    /// <c>a:normAutofit/@fontScale</c> into a field
    /// (<c>oox/source/drawingml/textbodypropertiescontext.cxx:236</c>) and never reads that field
    /// again, so the authoring application's stated answer is discarded and
    /// <c>SdrTextObj::autoFitTextForCompatibility</c> searches for its own. See
    /// <see cref="SlideTextLayout"/> for the search and for what it is measured against.
    /// </para>
    /// <para>
    /// <c>a:normAutofit/@lnSpcReduction</c> is modelled nowhere at all, which is deliberate: the
    /// same handler does not read it either — the <c>normAutofit</c> case reads
    /// <c>XML_fontScale</c> and nothing else — so a body carrying one must lay out exactly as a
    /// body that does not. XRay.Office did apply it, and it was worth 20 per cent of a line on the
    /// one shape in <c>slides/batch-001</c> that states it: the subtitle of
    /// <c>BMFE-06-03 (Gerflor) Smoke Density and Toxicity.pptx</c> shrank its lines, so the
    /// fit search thought the text nearly fitted unshrunk and drew it at 20 pt where the
    /// reference draws 15.
    /// </para>
    /// <para>
    /// This is a text-only fit. <c>a:spAutoFit</c> is the other direction — the shape grows to its
    /// text rather than the text shrinking to the shape — and is not this flag.
    /// </para>
    /// </remarks>
    public bool AutoFit { get; init; }

    /// <summary>
    /// The multiplier <c>a:normAutofit/@fontScale</c> asks for, or one when it states none.
    /// </summary>
    /// <remarks>
    /// Applied to every run's size when <see cref="AutoFit"/> is <em>not</em> set — which after
    /// the fit search means the ODF path and hand-built bodies only. The value in the file is what
    /// the authoring application arrived at when it last shrank the text to fit.
    /// </remarks>
    public double FontScale { get; init; } = 1.0;

    /// <summary>
    /// The WordArt preset the body is warped along — <c>a:bodyPr/a:prstTxWarp/@prst</c> — or
    /// null when it is ordinary text.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>textNoShape</c> is normalised to null on read: it is the value that means <em>no</em>
    /// warp, and the reference tests for exactly that
    /// (<c>oox/source/drawingml/textbodypropertiescontext.cxx:215-226</c> and
    /// <c>oox/source/drawingml/shape.cxx:2202-2211</c>) before putting the shape into text-path
    /// mode. So a non-null value here means Fontwork and nothing else does.
    /// </para>
    /// <para>
    /// See <see cref="IsTextPath"/> for what that costs the text layer.
    /// </para>
    /// </remarks>
    public string? WarpPreset { get; init; }

    /// <summary>
    /// The adjustment guides <c>a:prstTxWarp/a:avLst</c> states, in DrawingML units.
    /// </summary>
    /// <remarks>
    /// Carried unconverted: the same number means a degree to one preset and a per-mille of the
    /// shape to another, and the conversion into the WordArt units the geometry tables expect is
    /// per preset and per guide name (<c>oox/source/drawingml/fontworkhelpers.cxx:95-150</c>). It
    /// is done once, inside <see cref="XRay.Office.Ooxml.DrawingML.Fontwork"/>.
    /// </remarks>
    public IReadOnlyList<FontworkAdjustment> WarpAdjustments { get; init; } = [];

    /// <summary>
    /// The LibreOffice Fontwork type an ODF shape states directly, or null when the body is
    /// DrawingML's and <see cref="WarpPreset"/> names it instead.
    /// </summary>
    /// <remarks>
    /// ODF is the format LibreOffice's Fontwork model is native to, so a <c>draw:custom-shape</c>
    /// states <c>draw:type="fontwork-arch-up-curve"</c> — the very name
    /// <see cref="XRay.Office.Ooxml.DrawingML.FontworkPresets"/> is keyed by — and needs no mapping
    /// at all. <c>xmloff/source/draw/ximpcustomshape.cxx:1136-1150</c> is where the reference reads
    /// the <c>draw:text-path*</c> attributes beside it.
    /// </remarks>
    public string? WarpFontworkType { get; init; }

    /// <summary>
    /// The adjustment values an ODF shape's <c>draw:modifiers</c> states, already in WordArt units.
    /// </summary>
    /// <remarks>
    /// Unlike <see cref="WarpAdjustments"/> these need no conversion: <c>draw:modifiers</c> is
    /// written in the same 21600 viewbox the geometry tables use, and an angle handle is written as
    /// a plain degree, which is what the tables expect too.
    /// </remarks>
    public IReadOnlyList<double>? WarpAdjustmentValues { get; init; }

    /// <summary>
    /// Whether the warp keeps the run's stated size, when the file says so directly.
    /// </summary>
    /// <remarks>
    /// <c>draw:text-path-scale</c>: <c>shape</c> is <c>TextPathScaleX</c> true and <c>path</c> is
    /// false (<c>xmloff/source/draw/ximpcustomshape.cxx</c>, <c>EAS_TextPathScale</c>). LibreOffice
    /// writes <c>shape</c> on exactly the four presets the DrawingML side derives it for —
    /// <c>fontwork-arch-up-curve</c>, <c>fontwork-arch-down-curve</c>, <c>fontwork-circle-curve</c>
    /// and <c>fontwork-open-circle-curve</c> — so reading it rather than deriving it agrees with
    /// the other path and also honours a file that says otherwise.
    /// </remarks>
    public bool? WarpKeepsFontSize { get; init; }

    /// <summary>
    /// Whether the body is Fontwork: drawn as glyph outlines rather than as text.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A warped body is not text in the reference's output. <c>putCustomShapeIntoTextPathMode</c>
    /// turns the shape into a Fontwork custom shape, and
    /// <c>svx/source/customshapes/EnhancedCustomShapeFontWork.cxx</c> converts its characters to
    /// <c>tools::PolyPolygon</c> outlines, so what reaches a PDF is filled paths carrying no
    /// glyph and no <c>ToUnicode</c>. Measured on the installed 26.2.4.2 rather than assumed:
    /// the reference's page 13 of <c>FAAAIandtheArtandScienceofV&amp;Vfinal.pptx</c> holds 597
    /// curve operators where ours holds 4, and neither <c>Automation</c> nor <c>Autonomy</c> —
    /// words that exist only inside its warped bodies — appears anywhere in its text layer. The
    /// same is true of <c>prst="textPlain"</c>, which curves nothing:
    /// <c>redac-sas-201403-ppt-portfolio-rev-sim.pptx</c>'s <c>Fractographic Examinations</c>
    /// is absent from the reference too. The test really is "not <c>textNoShape</c>".
    /// </para>
    /// <para>
    /// <strong>XRay.Office now draws the curves.</strong> <see cref="WarpPreset"/> and
    /// <see cref="WarpAdjustments"/> are handed to
    /// <see cref="XRay.Office.Ooxml.DrawingML.Fontwork"/>, which builds the warped outlines, and
    /// <c>SlideFontwork</c> replaces the whole shape with them — its own fill, pen and shadow
    /// included, as the reference does. All forty <c>ST_TextShapeType</c> warps are built, so what
    /// is still drawn as nothing is a body set in a face with no <c>glyf</c> outlines. Drawing nothing
    /// stays the right fallback for those, and the measurement is unchanged: on page 13 of
    /// <c>FAAAIandtheArtandScienceofV&amp;Vfinal.pptx</c> the reference's outlines sit 14 to 40 pt
    /// away from where the unwarped runs land, ink in the wrong place counts twice in a comparison
    /// and absent ink once (<c>dotnet/probes/slides-extra-01/results.md</c>). The words side now
    /// falls back the same way, so the two families agree.
    /// </para>
    /// <para>
    /// Extraction is unaffected. <c>xray-office extract</c> reads the body through
    /// <c>DrawingTextBody</c>, which never consults this, so the words stay in the content tree
    /// — as they should: they are the document's own words, and it is only the *rendering* that
    /// turns them into a picture.
    /// </para>
    /// </remarks>
    public bool IsTextPath => WarpPreset is not null || WarpFontworkType is not null;

    /// <summary>
    /// Whether the text wraps at the shape's width.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>a:bodyPr/@wrap="none"</c> asks that it does not: the line runs on past the shape and the
    /// shape grows around it. Modelled as an unbounded width rather than as clipping, which is
    /// what makes a <c>wrap="none"</c> label come out on one line as its author saw it.
    /// </para>
    /// <para>
    /// <strong>But the attribute does not stand on its own, and reading it as though it did loses
    /// text.</strong> It suppresses wrapping only while the body's autofit leaves the shape alone.
    /// Measured on 26.2.4.2 with nine authored one-shape variants over both axes — a 236 pt box
    /// holding a 64-character line — <c>wrap="none"</c> draws **one** line with
    /// <c>a:noAutofit</c> and with no autofit element at all, and **four** lines with
    /// <c>a:spAutoFit</c> or <c>a:normAutofit</c>. <c>wrap="square"</c> draws four in all four
    /// autofit cases. See <c>PptxTextBody</c>, where the two are combined.
    /// </para>
    /// <para>
    /// The cost of the wrong reading is not cosmetic: an unbounded line runs off the *page*, not
    /// merely off the shape, and everything past the media box is gone from the text layer. On the
    /// 2026-08-19 baseline 30 of 305 slides renderings drew text outside the page where the
    /// reference drew 9, and one template family overhangs a 720 pt page by 8.7 pt so that
    /// <c>Google Slides</c> extracts as <c>Google Slid</c>.
    /// </para>
    /// </remarks>
    public bool Wraps { get; init; } = true;

    /// <summary>
    /// Whether the line height comes from the font size rather than from the font's metrics.
    /// </summary>
    /// <remarks>
    /// <para>
    /// EditEngine's <c>FixedCellHeight</c>, which ODF spells
    /// <c>style:font-independent-line-spacing</c>. When it is on the ascent is the font height
    /// outright and the line is 1.2 times it, whatever face the text is set in
    /// (<c>editeng/source/editeng/impedit3.cxx:501,3138-3141</c>); when it is off the face's own
    /// ascent and descent decide, as they do in a word processor.
    /// </para>
    /// <para>
    /// True by default because that is what a PPTX gets: the importer sets it on every text body
    /// it reads (<c>oox/source/ppt/pptshapecontext.cxx:186</c>). A natively authored ODP states
    /// it per paragraph style and usually does not, which is why the two paths give the same
    /// deck slightly different baselines and why this is a property of the body rather than a
    /// constant.
    /// </para>
    /// </remarks>
    public bool FontIndependentLineSpacing { get; init; } = true;

    /// <summary>
    /// The reference device this body's vertical metrics are quantised through.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <see cref="MetricGrid.Presentation"/> for everything Impress lays out — the one
    /// <c>VirtualDevice</c> <c>SdModule</c> makes at <c>RefDevMode::Dpi600</c> in 1/100 mm
    /// (<c>sd/source/ui/app/sdmod.cxx</c>:83-85). It is a property of the body rather than a
    /// constant in the layout because <b>not every text on a slide is laid out by Impress</b>: a
    /// chart's labels are built by <c>chart2</c>'s own view on a <c>VirtualDevice</c> it creates
    /// from <c>Application::GetDefaultDevice()</c>, which asks for no <c>RefDevMode</c> at all
    /// and is therefore <b>96 dpi</b> — a pixel of 0.75 pt against Impress's 0.24. See
    /// <see cref="MetricGrid.Chart"/>, and <c>SlideChart</c>, which is the only thing that sets
    /// this to anything else.
    /// </para>
    /// <para>
    /// It reaches the layout through the body so that the height a caller <em>measures</em> and
    /// the baselines the layout <em>places</em> cannot come from two different devices: both
    /// <c>SlideTextLayout.Height</c> and <c>SlideTextLayout.Place</c> take the body and nothing
    /// else decides it.
    /// </para>
    /// </remarks>
    public MetricGrid Device { get; init; } = MetricGrid.Presentation;
}

/// <summary>One paragraph of a shape's text.</summary>
/// <param name="Text">Its text, without a terminating mark.</param>
/// <param name="Runs">
/// Its runs, partitioning the text. Never empty for non-empty text: a paragraph with no stated
/// formatting still carries one run, so that the size an empty line is as tall as is known.
/// </param>
/// <param name="Alignment">How its lines are placed across the text rectangle.</param>
/// <param name="SpaceBefore">The space above it.</param>
/// <param name="SpaceAfter">The space below it.</param>
/// <param name="LineSpacing">Its line-spacing rule.</param>
/// <param name="StartIndent">Its indent from the start edge.</param>
/// <param name="FirstLineIndent">The extra indent on its first line, negative for a hanging one.</param>
/// <param name="Language">A BCP 47 tag, for the language-specific break rules.</param>
/// <param name="Marker">The bullet or number drawn before it, or null when it has none.</param>
public sealed record SlideParagraph(
    string Text,
    IReadOnlyList<SlideTextRun> Runs,
    TextAlignment Alignment = TextAlignment.Start,
    Length SpaceBefore = default,
    Length SpaceAfter = default,
    LineSpacingRule LineSpacing = default,
    Length StartIndent = default,
    Length FirstLineIndent = default,
    string? Language = null,
    SlideMarker? Marker = null)
{
    /// <summary>The slide formats' own default tab distance: one inch.</summary>
    public static Length DefaultTabDistance { get; } = Length.FromEmu(Length.EmuPerInch);

    /// <summary>
    /// How far apart the stops a tab advances to are, when the paragraph states none of its own.
    /// </summary>
    /// <remarks>
    /// <strong>A slide's is an inch, not the half inch a word processor uses.</strong> PowerPoint
    /// stores it as 0x240 master units and DrawingML as <c>a:defTabSz</c> defaulting to 914400
    /// EMU, and both are one inch; <see cref="ParagraphFormat.DefaultTabInterval"/> defaults to
    /// Word's 720 twips because that is what a document is. The difference compounds: a paragraph
    /// positioned by three tabs lands an inch and a half to the left of where it belongs, which on
    /// a ten-inch slide is fifteen per cent of the page.
    /// </remarks>
    public Length DefaultTabInterval { get; init; } = DefaultTabDistance;

    /// <summary>
    /// Whether the paragraph <em>states</em> its line spacing, as opposed to leaving it at the
    /// default — which is not the same question as whether <see cref="LineSpacing"/> resolves to
    /// single spacing.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>It decides whether the shrink-to-fit's line-spacing reduction reaches the
    /// paragraph at all, and only the binary PowerPoint reader ever sets it.</strong> EditEngine
    /// tests four rules in order and stops at the first that applies
    /// (<c>editeng/source/editeng/impedit3.cxx:1528-1602</c>):
    /// <c>SvxLineSpaceRule::Min</c>, <c>::Fix</c>, <c>SvxInterLineSpaceRule::Prop</c>,
    /// <c>::Off</c>. The <c>Prop</c> arm does <em>nothing at all</em> when the proportion is
    /// exactly 100, and the <c>::Off</c> arm — the only place a paragraph picks up the fit's
    /// <c>fSpacingY</c> — is then unreachable. So "states exactly 100%" and "states nothing" are
    /// different answers, and <see cref="LineSpacingRule"/> spells them the same.
    /// </para>
    /// <para>
    /// The two formats reach the item by different routes and that is what makes this
    /// binary-only. Every OOXML and ODF line spacing goes through the UNO property, and
    /// <c>SvxLineSpacingItem::PutValue</c> (<c>editeng/source/items/paraitem.cxx:194-202</c>)
    /// reads <c>style::LineSpacingMode::PROP</c> and writes <c>eInterLineSpaceRule = Off</c>
    /// <em>when the height is exactly 100</em>. The <c>.ppt</c> importer instead calls
    /// <c>SvxLineSpacingItem::SetPropLineSpace(100)</c> directly
    /// (<c>filter/source/msfilter/svdfppt.cxx:6285-6288</c>), and <c>lspcitem.hxx:86-91</c> shows
    /// that setter writes <c>Prop</c> unconditionally.
    /// </para>
    /// <para>
    /// Measured on an authored known-answer deck — <c>probes/slides-r54/make-ppt-fit-probe.py</c>
    /// through <c>soffice --convert-to ppt</c> and <c>ppt-patch-kind.py</c>, fifteen fitted boxes
    /// of one text — the <c>.pptx</c> half draws a baseline pitch of <c>1.2 × 0.8 × em</c> and the
    /// <c>.ppt</c> half draws <c>1.2 × em</c>. Same box, same text, same fit table. And because
    /// the binary side's lines are taller the fit <em>search</em> lands on a different row: at a
    /// 150 pt box the reference draws 13.011 pt on the binary side against 15.987 on the OOXML
    /// side.
    /// </para>
    /// </remarks>
    public bool LineSpacingStated { get; init; }

    /// <summary>
    /// Whether a paragraph with <em>no characters</em> still sits at the numbering level its
    /// <see cref="Marker"/> comes from — which decides whether its line is floored at the
    /// bullet's box even though no bullet is drawn.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Every reader suppresses an empty paragraph's bullet, and <strong>they do not suppress it in
    /// the same place</strong>. The OOXML importer sets the paragraph's <em>level</em> to −1
    /// (<c>oox/source/drawingml/textparagraph.cxx:192-196</c>, "empty paragraphs do not have
    /// bullets in ppt"), and <c>Outliner::GetNumberFormat</c> answers null below zero
    /// (<c>editeng/source/outliner/outliner.cxx:1289-1300</c>) — so there is no numbering format,
    /// no bullet area, and nothing to floor the line with. The binary PowerPoint importer sets
    /// only the <em>state</em>, <c>EE_PARA_BULLETSTATE</c>
    /// (<c>filter/source/msfilter/svdfppt.cxx:2363-2366</c>, "in PPT empty paragraphs never gets a
    /// bullet"), and leaves the depth it inserted the paragraph at (<c>:2309</c>) alone; ODF takes
    /// its level from the list nesting and suppresses nothing at all. On those two the format
    /// survives and the box with it.
    /// </para>
    /// <para>
    /// So this is true for <c>.ppt</c> and for ODF and false for OOXML, and the difference is
    /// worth a whole <c>constScaleLevels</c> row on an autofitted body. See
    /// <c>SlideTextLayout.BulletFloored</c>, which carries the measurement.
    /// </para>
    /// </remarks>
    public bool EmptyKeepsMarkerLevel { get; init; }
}

/// <summary>
/// The bullet or number a paragraph is labelled with.
/// </summary>
/// <remarks>
/// <para>
/// A marker is drawn as its own glyph run at its own pen, in its own face and usually at its own
/// size — LibreOffice writes it as a separate <c>/Lbl</c> block in the PDF, and on
/// <c>deck-features.pptx</c>'s outline that is a 12.6 pt run beside 28 pt text, because
/// <c>a:buSzPct val="45000"</c> says 45%.
/// </para>
/// <para>
/// It is <em>not</em> part of the paragraph's text, which is why it is here rather than prefixed
/// to it: a marker does not wrap, does not participate in the line breaking, and would change
/// every character offset the runs index by if it were spliced in.
/// </para>
/// </remarks>
/// <param name="Text">The characters to draw.</param>
/// <param name="Typeface">The family it is set in, or null for the paragraph's own.</param>
/// <param name="Scale">Its size as a fraction of the first run's, one for the same size.</param>
/// <param name="Colour">Its colour, or null for the first run's.</param>
/// <param name="IsSymbol">
/// Whether it is a fixed character rather than a generated number, which decides where it sits
/// vertically.
/// <para>
/// <strong>The two are placed by different rules and the difference is a point.</strong>
/// <c>Outliner::StripBullet</c> branches on <c>SVX_NUM_CHAR_SPECIAL</c>: a symbol is drawn from
/// the bullet <em>area's</em> bottom, which centres it against the line's text, and anything else
/// is drawn at <c>nFirstLineMaxAscent</c>, which is the text's own baseline
/// (<c>editeng/source/outliner/outliner.cxx:918</c>). Measured on
/// <c>slide-shape-features.pptx</c>, whose list is <c>a:buAutoNum</c>: LibreOffice draws its
/// first number at 89.972 and centring it would put it at 89.036.
/// </para>
/// </param>
public readonly record struct SlideMarker(
    string Text,
    string? Typeface = null,
    double Scale = 1.0,
    Colour? Colour = null,
    bool IsSymbol = true)
{
    /// <summary>
    /// The graphic the marker is, when it is a picture rather than a character.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A picture bullet is a numbering format of its own — <c>SVX_NUM_BITMAP</c> — and it is not a
    /// character drawn from a picture font. Three things follow, and each of them is why this is a
    /// separate field rather than a <see cref="Text"/> that happens to be an image:
    /// <c>Outliner::ImplGetBulletSize</c>'s box is the graphic's stated size and not a face's
    /// ascent and descent; that size is <em>absolute</em>, fixed at import from the paragraph's
    /// unscaled font height, so it does not move with <see cref="Scale"/> or with an autofit; and
    /// it contributes no glyph, so it is invisible to every text comparison.
    /// (<c>editeng/source/outliner/outliner.cxx</c>:1315-1355 for the box,
    /// <c>filter/source/msfilter/svdfppt.cxx</c>:3448-3465 for where the size comes from.)
    /// </para>
    /// <para>
    /// <see cref="Text"/> is empty when this is set, which is what keeps every character path —
    /// shaping, the recode table, the fallback search — from being reached at all.
    /// </para>
    /// </remarks>
    public SlideMarkerPicture? Picture { get; init; }
}

/// <summary>
/// A picture bullet: the graphic, and the box it is drawn in.
/// </summary>
/// <remarks>
/// The size is the <em>drawn</em> size rather than the picture's own, because the file states it:
/// a binary PowerPoint computes <c>height = round(fontHeight × 0.2540 × bulletHeight)</c> in
/// hundredths of a millimetre from the paragraph's first portion's point size and the level's
/// bullet percentage, and takes the width from the graphic's aspect ratio
/// (<c>svdfppt.cxx</c>:3450-3462). Confirmed against 26.2.4.2's own flat ODP of
/// <c>ws_prod-g-doc-Events-2007-september-M.017-(French)-France.ppt</c>, whose
/// <c>text:list-level-style-image</c> elements state 0.787 cm beside 20 pt text at 155 %,
/// 0.630 cm beside 16 pt, 0.709 cm beside 18 pt and 1.102 cm beside 28 pt — that expression to the
/// hundredth of a millimetre at all four, and 0.671, 0.503, 0.447 and 0.559 cm at 110 % likewise.
/// </remarks>
/// <param name="Image">The raster, or null when the graphic is a metafile.</param>
/// <param name="Vector">The metafile, decoded on first use, or null when it is a raster.</param>
/// <param name="Width">How wide it is drawn.</param>
/// <param name="Height">How tall it is drawn, which is also the box its line is floored at.</param>
public sealed record SlideMarkerPicture(
    RasterImage? Image,
    Lazy<VectorImage>? Vector,
    Length Width,
    Length Height);

/// <summary>
/// A run raised or lowered off its baseline, and shrunk while it is up there.
/// </summary>
/// <remarks>
/// <para>
/// Two numbers rather than one because a slide's formats state two: DrawingML's
/// <c>a:rPr/@baseline</c> gives the offset alone and the importer supplies the size
/// (<c>oox/source/drawingml/textcharacterproperties.cxx:196-199</c>), and a binary PowerPoint's
/// <c>PPT_CharAttr_Escapement</c> does the same
/// (<c>filter/source/msfilter/svdfppt.cxx:5764-5775</c>). Both end as one
/// <c>SvxEscapementItem(nEsc, nProp)</c>, which is this pair.
/// </para>
/// <para>
/// <strong>The percentage is of the em size here, not of the font's height.</strong> That is
/// where a slide differs from a word processor: EditEngine draws the run at
/// <c>GetFontSize().Height() × nEsc / 100</c> above the pen
/// (<c>editeng/source/items/svxfont.cxx:549-558</c>), where Writer's <c>swfont.cxx</c> takes the
/// same percentage of the unshrunk font's ascent-plus-descent. Using the wrong one of the two
/// misplaces a superscript by about a fifth of its rise.
/// </para>
/// <para>
/// The size matters more than the offset does, because it moves line breaks: a 12 pt run set at
/// 58% is 42% narrower, so a line that fits with the shrink wraps without it. Measured on
/// <c>slides/batch-003/pptx/NCW-2024-Guide-.pptx</c>, whose dates are written
/// <c>5<sup>th</sup> March</c>: drawing the ordinals full size wraps one line of a text box that
/// already overflows the slide, which pushes its last paragraph off the bottom edge.
/// </para>
/// </remarks>
/// <param name="Percent">
/// How far the run moves, as a percentage of its em size; positive raises it and negative lowers
/// it.
/// </param>
/// <param name="Proportion">
/// The size the run is set at, as a percentage of the size it would otherwise take. Zero and 100
/// both mean no change, so a default-constructed value is "no escapement at all".
/// </param>
public readonly record struct SlideEscapement(int Percent, int Proportion)
{
    /// <summary>The size an escaped run is set at when the file states only an offset.</summary>
    /// <remarks><c>DFLT_ESC_PROP</c>, <c>include/editeng/escapementitem.hxx:30</c>.</remarks>
    public const int AutomaticProportion = 58;

    /// <summary>Neither moved nor resized.</summary>
    public static SlideEscapement None => default;

    /// <summary>True when the run sits on its baseline at its own size.</summary>
    public bool IsNone => Percent == 0 && Proportion is 0 or 100;

    /// <summary>The size the run is actually set at, given the size it would otherwise take.</summary>
    public Length SizeOf(Length emSize)
        => Proportion is 0 or 100 ? emSize : emSize * (Proportion / 100.0);

    /// <summary>How far the run sits above its baseline, negative for a subscript.</summary>
    /// <param name="emSize">The size the run would take were it not escaped.</param>
    public Length RiseOf(Length emSize)
        => Percent == 0 ? Length.Zero : emSize * (Percent / 100.0);
}

/// <summary>One run of a paragraph: a range of its text with its own face, size and colour.</summary>
/// <param name="Start">The run's first character.</param>
/// <param name="Length">How many characters it covers.</param>
/// <param name="Typeface">The family it asks for, or null for the deck's default.</param>
/// <param name="Size">The em size.</param>
/// <param name="Weight">The weight on the OpenType 1–1000 scale.</param>
/// <param name="IsItalic">Whether it is italic.</param>
/// <param name="Colour">The colour it is drawn in.</param>
/// <param name="Tracking">
/// A fixed distance added between the run's characters — <c>a:rPr/@spc</c>, stated in hundredths
/// of a point and commonly negative. See <see cref="XRay.Office.Text.Layout.FormattedRun.Tracking"/>
/// for how it is charged.
/// </param>
/// <param name="IsUnderlined">
/// Whether a rule is drawn under it. A decoration rather than a glyph in every format here —
/// <c>a:rPr/@u</c> in DrawingML, bit 2 of a PPT character-property mask — so it moves no line
/// break and is drawn from the face's own <c>post</c> metrics after the text is placed.
/// </param>
/// <param name="IsStruckThrough">Whether a rule is drawn through it.</param>
/// <param name="IsShadowed">
/// Whether the characters cast the legacy per-character drop shadow — bit 4 of a PPT
/// character-property mask. Like the decorations above it moves no line break, because the
/// shadow is the same glyphs drawn a second time at an offset derived from the font's line
/// height rather than from anything the paragraph measured. See
/// <see cref="SlideTextLayout"/>'s <c>ShadowOffset</c> for the rule and the probe behind it.
/// </param>
/// <param name="Escapement">
/// How far off its baseline the run sits and how much it shrinks to sit there — a superscript or
/// a subscript. Unlike the decorations above, this <em>does</em> move line breaks, because the
/// shrink is what makes the run narrower.
/// </param>
/// <param name="SymbolFont">
/// The face the run's <em>private-use</em> characters are drawn from, or null when it names none.
/// <para>
/// DrawingML's <c>a:rPr/a:sym</c>, and a second family rather than a replacement for
/// <see cref="Typeface"/> because it governs only part of the run: LibreOffice switches the face
/// over each maximal stretch of characters satisfying <c>(ch &amp; 0xff00) == 0xf000</c> and
/// restores it after every one (<c>oox/source/drawingml/textrun.cxx:96-135</c>). A run reading
/// "see &#xF0E0; overleaf" is set in its own face except for the arrow.
/// </para>
/// <para>
/// It is resolved by <c>SlideSymbolRuns</c> before anything is measured, because whether the slot
/// is drawn as it stands or recoded into OpenSymbol turns on whether the named face is installed
/// — which a reader cannot know.
/// </para>
/// </param>
/// <param name="IsField">
/// Whether the run is a <em>field</em> rather than ordinary text — a hyperlink, in every reader
/// that sets this.
/// <para>
/// It is not a formatting property: it changes how the run breaks and how the lines it spills
/// onto are stacked. EditEngine holds a field as a single portion whose text is broken at every
/// cell when it does not fit (<c>editeng/source/editeng/impedit3.cxx</c>:1101-1200), and draws
/// each of those continuation lines one <em>ascent</em> below the last rather than one line
/// height (<c>:3778-3795</c>, <em>“only use GetMaxAscent(), pLine->GetHeight() will not proceed
/// as needed”</em>). See <see cref="SlideTextLayout"/> for both halves and for what they were
/// measured against.
/// </para>
/// <para>
/// A <c>text:a</c> inside a draw shape's text is one: <c>xmloff/source/text/txtparai.cxx</c>
/// :1352-1374 builds an <c>XMLUrlFieldImportContext</c> whenever the cursor has no
/// <c>HyperLinkURL</c> property, which is every Draw and Impress text and no Writer one.
/// </para>
/// <para>
/// So is a run carrying an <c>a:hlinkClick</c>, through the other importer:
/// <c>TextRun::insertAt</c> builds a <c>com.sun.star.text.TextField.URL</c> whose
/// <c>Representation</c> is the run's own <c>a:t</c>
/// (<c>oox/source/drawingml/textrun.cxx</c>:88, :149-157) — <strong>one field per
/// <c>a:r</c></strong>, so a link PowerPoint split across runs is several fields and fills
/// exactly as one does. Which elements qualify is <see cref="DrawingHyperlink.MakesField"/>'s
/// answer and not the element's presence.
/// </para>
/// <para>
/// A Writer hyperlink is <em>not</em> one, in a paragraph or inside a text box, and neither is a
/// <c>.ppt</c>'s here yet although LibreOffice's own <c>.ppt</c> filter makes one
/// (<c>filter/source/msfilter/svdfppt.cxx</c>:6936). See
/// <c>probes/pptx-field-r82/results.md</c>, which measured all three.
/// </para>
/// </param>
public readonly record struct SlideTextRun(
    int Start,
    int Length,
    string? Typeface,
    Length Size,
    int Weight,
    bool IsItalic,
    Colour Colour,
    Length Tracking = default,
    bool IsUnderlined = false,
    bool IsStruckThrough = false,
    bool IsShadowed = false,
    SlideEscapement Escapement = default,
    SlideSymbolFont? SymbolFont = null,
    bool IsField = false)
{
    /// <summary>One past the run's last character.</summary>
    public int End => Start + Length;
}

/// <summary>
/// The face a run's private-use characters are drawn from — DrawingML's <c>a:rPr/a:sym</c>.
/// </summary>
/// <remarks>
/// <para>
/// The two halves are one value because the second decides what the first means, and separating
/// them cost a measurement. <c>a:sym</c>'s <c>@charset</c> is what makes the request a
/// <em>symbol-encoded</em> one — <c>TextFont::implGetFontData</c> reports
/// <c>mnCharset == WINDOWS_CHARSET_SYMBOL</c>, which is the value 2, and nothing else
/// (<c>oox/source/drawingml/textfont.cxx:87-94</c>) — and that flag decides which of two entirely
/// different resolutions the face gets.
/// </para>
/// <para>
/// <strong>A symbol-encoded request never reaches fontconfig at all.</strong>
/// <c>FcPreMatchSubstitution::FindFontSubstitute</c> returns false immediately for one
/// (<c>vcl/unx/generic/font/fontsubst.cxx:100-104</c>), so the request falls to
/// <c>VCL.xcu</c>'s own chain, which names <c>opensymbol</c> for Wingdings and its relatives —
/// and the recode follows. A request that is <em>not</em> symbol-encoded is answered by
/// fontconfig first, and fontconfig has no idea the name meant a symbol font: it answers
/// <c>Wingdings</c> with DejaVu Sans, and the slot is then drawn from DejaVu Sans as it stands.
/// </para>
/// <para>
/// Measured against the banked 26.2.4.2 references rather than reasoned from the tree, on the
/// three corpus decks that state the two combinations:
/// </para>
/// <list type="bullet">
/// <item><description><c>Structural Testing.pptx</c> states <c>&lt;a:sym typeface="Symbol"
/// charset="0"/&gt;</c> — <em>not</em> symbol-encoded — and the reference recodes all five of its
/// slots anyway, because fontconfig answers the family "Symbol" with OpenSymbol on its own. Its
/// OpenSymbol glyphs on pages 3, 4, 5, 6 and 26 sit within 0.3 pt of ours.</description></item>
/// <item><description><c>16 - UTM - (NASA).pptx</c> and
/// <c>Stakeholders-v08052017 - v5.pptx</c> both state <c>&lt;a:sym typeface="Wingdings"/&gt;</c>
/// with no charset, and the reference draws those three slots in <b>DejaVu Sans</b> — at
/// (175.9, 94.3) and (189.2, 29.1) on the latter's page 8, where we had put OpenSymbol.
/// </description></item>
/// </list>
/// <para>
/// So the rule is not "the charset decides whether to recode". It is "the charset decides
/// whether fontconfig is consulted", and the recode then follows from where the face actually
/// landed — which is the same rule the bullet path has always had.
/// </para>
/// </remarks>
/// <param name="Typeface">The family <c>a:sym/@typeface</c> names.</param>
/// <param name="IsMicrosoftEncoded">
/// Whether <c>a:sym/@charset</c> is 2, VCL's <c>IsMicrosoftSymbolEncoded</c>. Absent and 0 both
/// mean false; <c>WINDOWS_CHARSET_DEFAULT</c> is 1, so an unstated charset is not symbol-encoded
/// either.
/// </param>
public readonly record struct SlideSymbolFont(string Typeface, bool IsMicrosoftEncoded);

/// <summary>
/// Resolves the faces a slide's text needs, once per distinct request.
/// </summary>
/// <remarks>
/// The same shape as the word processor's cache and for the same reason: a deck has a handful of
/// typefaces and hundreds of runs, and resolving one means walking a substitution chain and
/// reading a font file. Its own type rather than a shared one because the two libraries sit at
/// the same layer and neither may depend on the other.
/// </remarks>
public sealed class SlideFonts
{
    private readonly SystemFontResolver _fonts;
    private readonly Dictionary<(string?, int, bool), (OpenTypeFace? Face, FontReference? Reference)>
        _resolved = [];

    /// <summary>Creates a cache over a resolver, or over the installed fonts.</summary>
    /// <param name="fonts">The resolver to use, or null to build one over the installed fonts.</param>
    public SlideFonts(SystemFontResolver? fonts = null)
        => _fonts = fonts ?? new SystemFontResolver(SystemFontIndex.Build());

    /// <summary>
    /// The pitch the deck declares for a typeface, when its format states one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Both binary formats carry it and neither was read until it was measured to matter: PPTX puts
    /// it in the low two bits of <c>pitchFamily</c> on <c>&lt;a:latin&gt;</c>, PPT in
    /// <c>lfPitchAndFamily</c> at the end of each <c>FontEntityAtom</c>. LibreOffice sends it to
    /// fontconfig, and for a family fontconfig files under no generic it is the only thing that
    /// says the text is meant to line up in columns.
    /// </para>
    /// <para>
    /// <strong>Measured, on the corpus and then in isolation.</strong>
    /// <c>airbus-powerpoint-presentation-2019-20…pptx</c> declares <c>Lucida Console</c> with
    /// <c>pitchFamily="49"</c> — fixed pitch, modern family — and 26.2.4.2 draws it in DejaVu Sans
    /// Mono. Re-zipping the same deck with that one attribute removed and nothing else changed, it
    /// draws DejaVu Sans instead, which is fontconfig's answer for a name it files under nothing.
    /// <c>introduction_to_bea_tuxedo.ppt</c> is the same fact in the binary format:
    /// <c>lfPitchAndFamily</c> is <c>0x31</c> for Lucida Console and <c>0x12</c> for Times New
    /// Roman in the same collection.
    /// </para>
    /// <para>
    /// The pitch and not the family class, deliberately — and that abstention has now been
    /// measured on a slide twice, in both formats, and endorsed. Reading the family bits would
    /// send the whole class to fontconfig as a second generic family, which is what 26.2.4.2 does
    /// when it <em>measures</em> and not when it <em>draws</em>: on the draw layer
    /// <c>FontAttribute</c> has no class field, so the reference lays a run out in one face and
    /// paints it in another (the seventh confound, <c>probes/title-font-r92</c> for the
    /// <c>.pptx</c> half and <c>probes/ppt-spacing-r115</c> for the <c>.ppt</c> half). Leaving the
    /// nibble unread puts this tree on the face 26.2.4.2 itself draws, at that face's own
    /// advances; reading it would put us on the other, equally self-inconsistent, branch. Measured
    /// on <c>architecture6.ppt</c>, whose two <c>FontEntityAtom</c>s state <c>Helvetica</c> at
    /// <c>0x22</c>: clearing that one byte and changing nothing else takes 26.2.4.2's own output
    /// from 268 differing text runs to 52 and its worst run-origin shift against ours from
    /// 322.07 pt to 1.60 over 31 pages. The words and sheets paths do pass a declared class, and
    /// are right to — Writer body text never becomes a drawinglayer primitive, so there the
    /// reference measures and draws in the same face.
    /// </para>
    /// </remarks>
    /// <remarks>
    /// Settable rather than <c>init</c>-only because a PPT's font collection lives inside the
    /// <c>Environment</c> container, which is not read until layout starts and so is not available
    /// where the cache is constructed. The delegate is wired once, before any request reaches it.
    /// </remarks>
    public Func<string, FontPitch>? DeclaredPitches { get; set; }

    /// <summary>
    /// The face the deck carries for a request, when it carries one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Answers a path to the face, which is what <see cref="FontRequest.EmbeddedFaceKey"/> takes
    /// and what every backend downstream of resolution can open. Null means the deck embeds
    /// nothing usable for this family, which is true of all but three documents in the slides
    /// track, so that is the path that has to stay cheap.
    /// </para>
    /// <para>
    /// The weight and the slant are arguments rather than the family alone, because one
    /// <c>p:embeddedFont</c> carries up to four styles under a single name and a run picks among
    /// them. Beside <see cref="DeclaredPitches"/> and settable for the same reason: the deck's
    /// font list lives on a part the format-specific layout owns.
    /// </para>
    /// </remarks>
    public Func<string, int, bool, string?>? EmbeddedFaces { get; set; }

    /// <summary>
    /// The pitch in a Windows <c>LOGFONT.lfPitchAndFamily</c> byte.
    /// </summary>
    /// <remarks>
    /// Shared by both readers because both formats carry the same byte: PPTX writes it as the
    /// decimal <c>pitchFamily</c> attribute, PPT as the last byte of a <c>FontEntityAtom</c>, and
    /// the WW8 font table as <c>FFN.prq</c>. The low two bits are the pitch and the high four the
    /// family; only the pitch is read here.
    /// </remarks>
    /// <param name="pitchAndFamily">The byte, as written.</param>
    public static FontPitch PitchIn(int pitchAndFamily)
        => (pitchAndFamily & 0x03) switch
        {
            1 => FontPitch.Fixed,
            2 => FontPitch.Variable,
            _ => FontPitch.Unknown,
        };

    /// <summary>The substitutions made so far, which is the first thing a comparison checks.</summary>
    public IReadOnlyList<FontSubstitution> Substitutions => _fonts.Substitutions;

    /// <summary>
    /// Where a run looks for a face when the one it named has no glyph for a character.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The resolver that chose the run's face in the first place, which is what
    /// <see cref="IGlyphFallbackResolver"/> asks for: a fallback decided by a second index could
    /// name a face this one would not have chosen, and the face decides the advance. The word
    /// processor exposes the same thing off its own cache for the same reason
    /// (<c>LayoutFonts.Fallback</c>).
    /// </para>
    /// <para>
    /// Without it a character the run's face cannot draw is shaped to <c>.notdef</c> and drawn as
    /// that face's missing-glyph box — which for a face that declines to draw one is nothing at
    /// all, so the text is <em>invisible</em> and no gate column sees it: the code point still
    /// reaches the PDF with a correct <c>ToUnicode</c>, so <c>pdftotext</c> extracts it and the
    /// word count is unmoved. Measured on
    /// <c>southern-classic-kennesaw-state-university-final.pptx</c>, whose body text holds 132
    /// U+25D8 inverse bullets: LibreOffice falls back to DejaVu Sans and embeds it, and we kept
    /// Carlito and drew 132 blanks. The one column that showed it was <c>fonts</c>, at 6/7.
    /// </para>
    /// </remarks>
    public IGlyphFallbackResolver Fallback => _fonts;

    /// <summary>The mid-run fallbacks made so far, resolved or not.</summary>
    /// <remarks>
    /// Beside <see cref="Substitutions"/> and for the same reason: a fallback face is almost never
    /// metric-compatible with the one it replaces, so it moves the line breaks after it, and a
    /// comparison against a reference renderer otherwise cannot tell that from a layout bug.
    /// </remarks>
    public IReadOnlyList<GlyphFallback> GlyphFallbacks => _fonts.GlyphFallbacks;

    /// <summary>The face and reference a request resolves to, both null when nothing could be read.</summary>
    public (OpenTypeFace? Face, FontReference? Reference) Resolve(
        string? family, int weight, bool isItalic)
    {
        // The pitch is a property of the typeface rather than of the request, so it adds nothing to
        // the key: two requests naming the same family declare the same pitch.
        (string?, int, bool) key = (family, weight, isItalic);
        if (_resolved.TryGetValue(key, out (OpenTypeFace?, FontReference?) cached)) return cached;

        FontPitch pitch = family is { Length: > 0 } named && DeclaredPitches is { } declared
            ? declared(named)
            : FontPitch.Unknown;

        // The deck's own copy of the face, when it has one. It wins over everything installed and
        // over the whole substitution chain, because it is the face the author measured against —
        // see `FontRequest.EmbeddedFaceKey`.
        string? embedded = family is { Length: > 0 } carried && EmbeddedFaces is { } faces
            ? faces(carried, weight, isItalic)
            : null;

        (OpenTypeFace? Face, FontReference? Reference) resolved = default;
        try
        {
            FontReference reference = _fonts.Resolve(
                new FontRequest(family ?? string.Empty, weight, isItalic, pitch, embedded));
            resolved = (_fonts.LoadOpenType(reference), reference);
        }
        catch (Exception exception) when (exception is Core.MalformedDocumentException
                                             or IOException
                                             or UnauthorizedAccessException)
        {
            // A face that cannot be read costs the shape its text, not the deck its layout.
        }

        _resolved[key] = resolved;
        return resolved;
    }
}
