using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

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

    /// <summary>True when the frame holds a picture rather than text.</summary>
    /// <remarks>
    /// Recorded rather than drawn: decoding the raster is a separate unstarted item, and the wrap — which
    /// is what moves text — depends only on the rectangle. So an image frame reserves its room correctly
    /// and draws whatever placeholder the sink is given.
    /// </remarks>
    public bool IsImage { get; init; }

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
