using Paperless.Core.Charts;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Vector;

namespace Paperless.WordProcessing.Layout;

/// <summary>
/// What a floating frame is fixed to, which decides what moves it.
/// </summary>
/// <remarks>
/// The distinction is not decoration: a page-anchored frame stays where it is however the text reflows,
/// and a paragraph-anchored one follows its paragraph onto the next page. Writer's
/// <c>RndStdIds::FLY_AT_PAGE</c>, <c>FLY_AT_PARA</c>, <c>FLY_AT_CHAR</c> and <c>FLY_AS_CHAR</c>.
/// </remarks>
public enum FrameAnchor
{
    /// <summary>To a paragraph: the frame sits beside it and moves with it.</summary>
    Paragraph,

    /// <summary>To a character position, which is a finer origin for the same behaviour.</summary>
    Character,

    /// <summary>In the text, as one very large character on a line of its own making.</summary>
    AsCharacter,

    /// <summary>To the page, so reflowing the text does not move it.</summary>
    Page,
}

/// <summary>
/// How body text behaves where a frame is in its way.
/// </summary>
/// <remarks>
/// <para>
/// The names are Writer's <c>css::text::WrapTextMode</c> rather than any one format's, because the four
/// formats spell the same six things differently and one of the spellings is actively misleading: ODF's
/// <c>style:wrap="none"</c> does <em>not</em> mean "no wrapping" — it means no text beside the frame at
/// all, so the text goes above and below it. ODF's word for "ignore the frame" is <c>run-through</c>.
/// </para>
/// </remarks>
public enum TextWrap
{
    /// <summary>The text ignores the frame and runs under or over it. ODF's <c>run-through</c>.</summary>
    Through,

    /// <summary>No text beside the frame: it goes above and below. ODF's <c>none</c>.</summary>
    TopAndBottom,

    /// <summary>Text on both sides. ODF's <c>parallel</c>, OOXML's <c>bothSides</c>.</summary>
    Both,

    /// <summary>Text on the frame's left only, so the frame reaches the end margin.</summary>
    Left,

    /// <summary>Text on the frame's right only, so the frame reaches the start margin.</summary>
    Right,

    /// <summary>
    /// Whichever side has more room, decided per frame. ODF's <c>dynamic</c>, OOXML's <c>largest</c>.
    /// </summary>
    Optimal,
}

/// <summary>What a frame's horizontal position is measured from.</summary>
public enum FrameHorizontalOrigin
{
    /// <summary>The sheet's own left edge.</summary>
    Page,

    /// <summary>The text area — inside the page margins. ODF's <c>page-content</c>, OOXML's <c>margin</c>.</summary>
    PageMargin,

    /// <summary>The column the anchor is in, which for single-column text is the text area.</summary>
    Column,

    /// <summary>The anchor paragraph's own rectangle, indents included.</summary>
    Paragraph,

    /// <summary>The anchoring character's position.</summary>
    Character,
}

/// <summary>What a frame's vertical position is measured from.</summary>
public enum FrameVerticalOrigin
{
    /// <summary>The sheet's own top edge.</summary>
    Page,

    /// <summary>The text area — inside the page margins.</summary>
    PageMargin,

    /// <summary>The anchor paragraph's top.</summary>
    Paragraph,

    /// <summary>The anchoring line's top, which for a one-line anchor is the paragraph's.</summary>
    Line,
}

/// <summary>How a frame sits inside its horizontal origin.</summary>
public enum FrameHorizontalAlignment
{
    /// <summary>At a stated distance from the origin's start edge.</summary>
    Offset,

    /// <summary>Flush with the origin's start edge.</summary>
    Left,

    /// <summary>Centred in the origin.</summary>
    Centre,

    /// <summary>Flush with the origin's end edge.</summary>
    Right,

    /// <summary>Towards the binding: left on a right-hand page, right on a left-hand one.</summary>
    Inside,

    /// <summary>Away from the binding.</summary>
    Outside,
}

/// <summary>How a frame sits inside its vertical origin.</summary>
public enum FrameVerticalAlignment
{
    /// <summary>At a stated distance below the origin's top edge.</summary>
    Offset,

    /// <summary>Flush with the origin's top.</summary>
    Top,

    /// <summary>Centred in the origin.</summary>
    Middle,

    /// <summary>Flush with the origin's bottom.</summary>
    Bottom,
}

/// <summary>
/// A floating frame: a rectangle of content anchored somewhere in the text, that body text flows round.
/// </summary>
/// <remarks>
/// <para>
/// Deliberately not a <see cref="PageBlock"/>. A block is something the paginator stacks; a frame is
/// something it <em>places</em>, at a position derived from an anchor and an origin rather than from
/// where the last block ended. The two would only share the list, and putting a frame in it would mean
/// every consumer of a block list having to skip one.
/// </para>
/// <para>
/// A frame's own content is blocks, so a text frame containing a table needs no second layout path — it
/// goes through <see cref="FlowLayouter"/> exactly as a header or a table cell does. An image frame
/// carries no blocks and is recorded by its rectangle, since decoding the raster is a separate matter and
/// the wrap does not depend on it.
/// </para>
/// </remarks>
public sealed record PageFrame
{
    /// <summary>How big the frame is.</summary>
    public required DocSize Size { get; init; }

    /// <summary>
    /// The whole shape group this frame is one member of, or null for a frame that stands alone.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A group is one anchored object holding many shapes, and the members are placed <em>relative to
    /// it</em>: the anchor's position and alignment decide where the group's rectangle goes, and each
    /// member sits at a fixed offset inside that rectangle. Carrying the group's size here is what lets a
    /// centred or right-aligned group still be resolved once and its members follow — aligning each
    /// member by its own width would spread a letterhead across the page.
    /// </para>
    /// <para>
    /// The members are flattened into siblings rather than nested, because that is what the layout engine
    /// can place: <see cref="FrameLayout"/> resolves one rectangle per frame and a group's member is a
    /// rectangle like any other. What the flattening must not do is punch a hole in the text per member,
    /// so a member takes <see cref="TextWrap.Through"/> and the group's own envelope keeps the wrap.
    /// </para>
    /// </remarks>
    public DocSize? GroupSize { get; init; }

    /// <summary>Where this frame sits inside <see cref="GroupSize"/>, from the group's top-left.</summary>
    public DocPoint GroupOffset { get; init; }

    /// <summary>What the frame is fixed to.</summary>
    public FrameAnchor Anchor { get; init; } = FrameAnchor.Paragraph;

    /// <summary>
    /// How body text behaves beside it.
    /// </summary>
    /// <remarks>
    /// <see cref="TextWrap.Through"/> by default, which is the harmless answer: a frame whose wrap could
    /// not be read leaves the text exactly where it would have been rather than moving all of it.
    /// </remarks>
    public TextWrap Wrap { get; init; } = TextWrap.Through;

    /// <summary>What the horizontal position is measured from.</summary>
    public FrameHorizontalOrigin HorizontalOrigin { get; init; } = FrameHorizontalOrigin.Paragraph;

    /// <summary>How it sits inside that origin.</summary>
    public FrameHorizontalAlignment HorizontalAlignment { get; init; } = FrameHorizontalAlignment.Offset;

    /// <summary>The distance from the origin's start edge, when the alignment is an offset.</summary>
    public Length HorizontalOffset { get; init; }

    /// <summary>What the vertical position is measured from.</summary>
    public FrameVerticalOrigin VerticalOrigin { get; init; } = FrameVerticalOrigin.Paragraph;

    /// <summary>How it sits inside that origin.</summary>
    public FrameVerticalAlignment VerticalAlignment { get; init; } = FrameVerticalAlignment.Offset;

    /// <summary>The distance below the origin's top edge, when the alignment is an offset.</summary>
    public Length VerticalOffset { get; init; }

    /// <summary>
    /// How far text must stay clear of the frame on each side.
    /// </summary>
    /// <remarks>
    /// Writer keeps this as the frame's own margins and adds it to the rectangle before asking what a line
    /// overlaps — <c>SwAnchoredObject::GetObjRectWithSpaces</c>. So it widens the hole in the text without
    /// moving the frame, which is why it is here rather than folded into the position.
    /// </remarks>
    public Margins Spacing { get; init; }

    /// <summary>Where the anchoring character sits in the paragraph's text, for a character anchor.</summary>
    public int AnchorOffset { get; init; }

    /// <summary>
    /// How much of an as-character frame sits above the baseline, or null for all of it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Null is the ordinary inline picture, which rests its bottom on the baseline, and is the default so
    /// that the three readers that had no vertical rule to state keep the numbers they were measured
    /// against. Zero is the other end: the frame hangs entirely below the line and raises its descent
    /// instead, which is what Writer does for a fly whose position relative to the baseline comes back at
    /// nought or more (<c>SwFlyCntPortion::SetBase</c>).
    /// </para>
    /// <para>
    /// Only DOC sets it, and only for a shape a <c>SHAPE</c> field made as-character: those state a
    /// vertical orientation of <c>TEXT_LINE</c> with no offset, which resolves to nought. Ignored for
    /// every other anchor, since only an as-character frame has a baseline to be measured from.
    /// </para>
    /// </remarks>
    public Length? InlineAscent { get; init; }

    /// <summary>A text frame's own content, empty for an image.</summary>
    public IReadOnlyList<PageBlock> Blocks { get; init; } = [];

    /// <summary>The inset between the frame's edge and its text.</summary>
    public Margins Padding { get; init; }

    /// <summary>The frame's background, or null when it has none.</summary>
    public Colour? Fill { get; init; }

    /// <summary>The frame's border colour, or null when it has no border.</summary>
    public Colour? BorderColour { get; init; }

    /// <summary>How thick that border is.</summary>
    public Length BorderWidth { get; init; }

    /// <summary>
    /// True when the frame is a straight line across its own rectangle rather than a box.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The one preset shape whose outline is not the rectangle it is anchored by, and the commonest
    /// drawing in a form: a rule, a strike across a block, the cross over an unused half of a
    /// certificate. It has no area, so it has neither a fill nor a rectangular border — it is stroked
    /// corner to corner in <see cref="BorderColour"/> at <see cref="BorderWidth"/>.
    /// </para>
    /// <para>
    /// A flag rather than a shape-geometry model, because that is the shape of the answer here: every
    /// other preset really is drawn inside its rectangle, and the general evaluator that would draw the
    /// rest of them is a separate piece of work. Drawing this one as a box is not a small error —
    /// its fill defaults to opaque white, so it hides the text it was drawn over.
    /// </para>
    /// </remarks>
    public bool IsLine { get; init; }

    /// <summary>
    /// True when a line frame runs from its bottom-left corner to its top-right rather than from its
    /// top-left to its bottom-right.
    /// </summary>
    /// <remarks>
    /// Mirroring once turns one diagonal into the other and mirroring twice turns it back, so this is
    /// the <em>exclusive or</em> of the shape's two flip flags rather than either of them. A cross is
    /// two of these shapes over one rectangle, distinguished by nothing else.
    /// </remarks>
    public bool IsLineMirrored { get; init; }

    /// <summary>True when the frame holds a picture rather than text.</summary>
    /// <remarks>
    /// Separate from <see cref="Image"/> and <see cref="Vector"/>, because they answer different
    /// questions. This is what the document <em>declared</em> the frame to be, which is what the wrap
    /// and the extraction tree go by; the others are whether bytes were found and what kind they turned
    /// out to be. A picture whose package part is missing, and a PICT nobody here decodes, both set this
    /// and leave the other two null.
    /// </remarks>
    public bool IsImage { get; init; }

    /// <summary>
    /// The picture the frame holds, still in the bytes the file stored, or null when it holds none.
    /// </summary>
    /// <remarks>
    /// Built with <see cref="RasterImage.Encoded"/> and never decoded here: a reader that decoded would
    /// pull a codec into the extraction path, which the layering forbids and which is the reason the IR
    /// carries encoded bytes at all. Whichever backend wants pixels asks <c>RasterImageDecoder.Ensure</c>
    /// for them, and one that only wants to pass a JPEG through to <c>DCTDecode</c> never decodes at all.
    /// </remarks>
    public RasterImage? Image { get; init; }

    /// <summary>
    /// The vector picture the frame holds — an SVG, a WMF, an EMF or an EMF+ — or null when it holds
    /// none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A display list rather than pixels, and it needed nothing in <c>Paperless.Core</c>:
    /// <c>VectorImage</c> already is the abstraction a frame wants — <c>Draw(IDrawingSink, DocRect)</c>
    /// plus an intrinsic size, immutable and replayable — and the layering already permits this library
    /// to name it. A Core interface would have had those two members and one implementation.
    /// </para>
    /// <para>
    /// <strong>Not decoded until something draws.</strong> See <see cref="FramePicture"/> for the
    /// measurement that decided it; RTF and DOC read their pictures on the extraction path, where a
    /// second of font resolution would be paid by a caller that only wanted the words.
    /// </para>
    /// <para>
    /// <see cref="Image"/> may be set beside this, and means the raster fallback of a DrawingML
    /// <c>svgBlip</c> — what a consumer that cannot read SVG would have shown. Nothing else sets both.
    /// </para>
    /// </remarks>
    public Lazy<VectorImage>? Vector { get; init; }

    /// <summary>
    /// The chart the frame holds, or null when it holds none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The third thing a frame's rectangle can be filled with, beside <see cref="Image"/> and
    /// <see cref="Vector"/>, and it is a <em>model</em> rather than a picture: the marks are composed
    /// into the rectangle at drawing time by <c>Paperless.Core.Charts</c>, exactly as a slide's and a
    /// sheet's are. Nothing about the wrap depends on it, which is why it sits beside the picture
    /// rather than replacing it — a chart frame whose part could not be read still reserves its room.
    /// </para>
    /// <para>
    /// A DOCX states one as a <c>w:drawing</c> whose <c>a:graphicData</c> names the chart namespace and
    /// carries a relationship to a <c>c:chartSpace</c> part; an ODT as a <c>draw:frame</c> holding a
    /// <c>draw:object</c> whose sub-document root is a <c>chart:chart</c>. Both arrive here as one
    /// <see cref="ChartPlot"/>, so <c>PageDrawing</c> has one case rather than two.
    /// </para>
    /// </remarks>
    public ChartPlot? Chart { get; init; }

    /// <summary>
    /// The family a chart's labels are set in, or null for the drawing code's own default.
    /// </summary>
    /// <remarks>
    /// Beside <see cref="Chart"/> rather than inside it, because <see cref="ChartPlot"/> carries type
    /// <em>sizes</em> and no family — the decks and workbooks it was built for each have one obvious
    /// answer and Writer does not. Measured with <c>pdffonts</c> on LibreOffice's own PDFs:
    /// <c>chart2/qa/extras/data/odt/chart.odt</c> draws its chart in Liberation Sans and
    /// <c>docx/chart.docx</c> draws the same chart in Carlito, because an OOXML chart's text takes the
    /// theme's minor latin face and an ODF chart's takes the office default. Measuring both in one face
    /// leaves every label the wrong width, which moves the plot area rather than only the ink.
    /// </remarks>
    public string? ChartFontFamily { get; init; }

    /// <summary>What the frame was called in the document, for diagnostics.</summary>
    public string? Name { get; init; }
}

/// <summary>
/// A frame after it has been given a rectangle on a page.
/// </summary>
/// <param name="Frame">What was placed.</param>
/// <param name="Area">Where it went, in page coordinates.</param>
/// <param name="Content">Its own text laid out inside that rectangle, or null when it has none.</param>
public sealed record PlacedFrame(PageFrame Frame, DocRect Area, PlacedFlow? Content = null);
