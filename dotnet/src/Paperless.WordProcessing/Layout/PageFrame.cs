using Paperless.Core.Geometry;
using Paperless.Core.Units;

namespace Paperless.WordProcessing.Layout;

/// <summary>What a floating frame's position is measured from.</summary>
/// <remarks>
/// Only the anchor kinds that <em>float</em> are here. An as-character frame is not one of them: it sits in
/// the line like a very large glyph, takes part in line breaking, and is already modelled as an anchor
/// character in the paragraph's text. Giving it a case would invite it to be laid out twice.
/// </remarks>
public enum FrameAnchor
{
    /// <summary>To the paragraph it is declared in, which is what most documents use.</summary>
    Paragraph,

    /// <summary>To a character position within that paragraph.</summary>
    Character,

    /// <summary>To the page the anchoring paragraph lands on.</summary>
    Page,
}

/// <summary>How text behaves where a frame is in its way.</summary>
/// <remarks>
/// The names are ODF's, and the other three formats' spellings map onto them: DOCX's <c>w:wrap</c> values,
/// RTF's <c>\wraptext</c> family, and WW8's <c>wr</c> field in the anchor record. What matters to layout is
/// only which sides a line may use, so five values cover all four formats.
/// </remarks>
public enum TextWrap
{
    /// <summary>
    /// No text beside the frame at all: a line that would meet it is pushed below it.
    /// </summary>
    /// <remarks>
    /// ODF's <c>style:wrap="none"</c>, and the one value whose name reads backwards — it means "do not wrap
    /// text <em>around</em> it", not "do not let it affect the text".
    /// </remarks>
    None,

    /// <summary>Text runs down both sides of the frame, whichever has room.</summary>
    Parallel,

    /// <summary>Text keeps only the room to the frame's left.</summary>
    Left,

    /// <summary>Text keeps only the room to the frame's right.</summary>
    Right,

    /// <summary>
    /// Text takes whichever side has more room, and neither when both are too narrow.
    /// </summary>
    /// <remarks>
    /// ODF's <c>dynamic</c>, which LibreOffice's user interface calls "optimal". Modelled as
    /// <see cref="Parallel"/> for now, since the side with more room is what the free-interval arithmetic
    /// picks anyway; what is not modelled is the threshold below which Writer gives up and pushes the line
    /// down.
    /// </remarks>
    Dynamic,

    /// <summary>
    /// The frame does not affect the text at all, which runs straight through underneath or over it.
    /// </summary>
    /// <remarks>
    /// ODF's <c>run-through</c>. A watermark is the usual reason.
    /// </remarks>
    Through,
}

/// <summary>
/// A floating frame: a rectangle beside or behind the text, and how the text treats it.
/// </summary>
/// <remarks>
/// <para>
/// Its <see cref="Offset"/> is relative to whatever <see cref="Anchor"/> names rather than to the page,
/// because that is what every format states and because the resolution needs something only layout knows —
/// where the anchoring paragraph ended up. A frame anchored to a paragraph that moves to the next page moves
/// with it.
/// </para>
/// <para>
/// Its content is a flow of its own — a frame can hold anything a body can, and it is laid out at the frame's
/// width by the same <see cref="FlowLayouter"/> a table cell's content goes through. Empty for a frame whose
/// content is not text, an image above all, which is placed and drawn but has nothing to break into lines.
/// </para>
/// </remarks>
public sealed record PageFrame
{
    /// <summary>Where the frame's top-left sits, relative to its anchor.</summary>
    public required DocPoint Offset { get; init; }

    /// <summary>How big it is.</summary>
    public required DocSize Size { get; init; }

    /// <summary>What the offset is measured from.</summary>
    public FrameAnchor Anchor { get; init; } = FrameAnchor.Paragraph;

    /// <summary>How text behaves where the frame is in its way.</summary>
    public TextWrap Wrap { get; init; } = TextWrap.Parallel;

    /// <summary>
    /// The gap kept between the frame and the text beside it, which widens the region text avoids.
    /// </summary>
    /// <remarks>
    /// Part of the wrap region rather than of the frame: the frame is drawn at
    /// <see cref="Size"/> and the text stays this much further away. Measured on the corpus document — a
    /// 5 cm frame at the left margin with a 0.2 cm right margin pushes text to 204.1 pt, which is
    /// 56.7 + 141.73 + 5.67.
    /// </remarks>
    public CellPadding Margins { get; init; }

    /// <summary>
    /// The blocks inside the frame, in order, or empty when it holds no text.
    /// </summary>
    /// <remarks>
    /// Blocks rather than paragraphs, for the same reason a cell's content is: a frame can hold a table, and
    /// it goes through the same layout path either way.
    /// </remarks>
    public IReadOnlyList<PageBlock> Blocks { get; init; } = [];

    /// <summary>
    /// The gap between the frame's own edges and its text, which comes out of the width its lines break at.
    /// </summary>
    /// <remarks>
    /// The frame's <c>fo:padding</c>, and not to be confused with <see cref="Margins"/>: padding is inside
    /// the frame and margin is outside it. Conflating the two puts the frame's own text where the body text
    /// beside it should be.
    /// </remarks>
    public CellPadding Padding { get; init; }

    /// <summary>True when the frame takes room from the text rather than being ignored by it.</summary>
    public bool Obstructs => Wrap != TextWrap.Through;

    /// <summary>
    /// The region text keeps clear of, given where the anchor resolved to.
    /// </summary>
    /// <param name="anchor">The anchor's own top-left in page coordinates.</param>
    public DocRect RegionFrom(DocPoint anchor) => new(
        anchor.X + Offset.X - Margins.Left,
        anchor.Y + Offset.Y - Margins.Top,
        Size.Width + Margins.Horizontal,
        Size.Height + Margins.Vertical);

    /// <summary>The frame itself, given where the anchor resolved to — what would be drawn.</summary>
    /// <param name="anchor">The anchor's own top-left in page coordinates.</param>
    public DocRect BoundsFrom(DocPoint anchor)
        => new(anchor.X + Offset.X, anchor.Y + Offset.Y, Size.Width, Size.Height);

    /// <summary>The rectangle the frame's own text is laid out in: its bounds less its padding.</summary>
    /// <param name="anchor">The anchor's own top-left in page coordinates.</param>
    public DocRect ContentAreaFrom(DocPoint anchor)
    {
        DocRect bounds = BoundsFrom(anchor);

        return new DocRect(
            bounds.X + Padding.Left,
            bounds.Y + Padding.Top,
            Length.Max(Length.Zero, bounds.Width - Padding.Horizontal),
            Length.Max(Length.Zero, bounds.Height - Padding.Vertical));
    }
}
