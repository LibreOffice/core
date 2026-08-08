using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.WordProcessing.Layout;

/// <summary>
/// One side of a paragraph's border: the rule itself and the gap it keeps from the text.
/// </summary>
/// <param name="Width">The rule's thickness. Zero is a stated <em>no border</em> rather than a thin one.</param>
/// <param name="Space">The gap between the rule and the text it borders.</param>
/// <param name="Trailing">
/// A further gap on the far side of the rule, which only a <c>w:between</c> has: it separates two
/// paragraphs rather than a paragraph and its margin, so the space is kept on both sides of it.
/// </param>
/// <param name="Colour">The colour the rule is drawn in.</param>
/// <remarks>
/// The order along the axis is <em>text, space, rule</em>, measured rather than assumed: with a
/// paragraph's first line topped at 704.10 pt and a 2.25 pt rule at 1 pt distance, LibreOffice puts the
/// stroke's centre at 706.15, which is the outer edge less half the width. Putting the space outside the
/// rule instead misplaces every border by its own distance.
/// </remarks>
public readonly record struct ParagraphBorder(Length Width, Length Space, Length Trailing, Colour Colour)
{
    /// <summary>A rule at a width and distance, in the colour given.</summary>
    public ParagraphBorder(Length width, Length space, Colour colour)
        : this(width, space, Length.Zero, colour)
    {
    }

    /// <summary>How much room the side takes across the paragraph's edge.</summary>
    /// <remarks>
    /// <c>w:sz/8 + w:space</c>, in points, and nothing else — measured on sixteen probes varying both:
    /// sz 18 space 1 costs 3.25 pt, sz 18 space 0 costs 2.25, sz 4 space 1 costs 1.50 and sz 24 space 10
    /// costs 13.00. It <em>adds to</em> <c>w:spacing</c> rather than merging with it.
    /// </remarks>
    public Length Allowance => Width + Space + Trailing;

    /// <summary>True when the side states a rule that actually draws.</summary>
    public bool Draws => Width > Length.Zero;
}

/// <summary>
/// A paragraph's four borders and the rule between it and an identically bordered neighbour.
/// </summary>
/// <remarks>
/// <para>
/// <c>w:pBdr</c>, ODF's <c>fo:border-*</c>, RTF's <c>\brdrt</c> family — a decoration that is also a
/// measurement, which is what makes it worth carrying separately from <see cref="PageParagraph.Shading"/>:
/// a border draws a rule <em>and</em> takes vertical room, so a reader that ignores it both loses the rule
/// and shortens every page carrying one.
/// </para>
/// <para>
/// A null side is one nothing stated. A side stating <c>w:val="none"</c> is a <see cref="ParagraphBorder"/>
/// of zero width, which is a different answer: it beats whatever the style chain below it would have given,
/// and takes no room. The distinction is the same one <c>TableBorderSet</c> keeps and for the same reason.
/// </para>
/// <para>
/// Left and right are here because they draw, not because they measure: LibreOffice grows the box outward
/// past the page margin rather than narrowing the text, so a bordered paragraph breaks its lines exactly
/// where an unbordered one would. Only <see cref="Above"/> and <see cref="Below"/> reach the paginator.
/// </para>
/// </remarks>
public sealed record ParagraphBorderSet
{
    /// <summary>The rule down the paragraph's left edge, or null when it states none.</summary>
    public ParagraphBorder? Left { get; init; }

    /// <summary>The rule down the paragraph's right edge, or null when it states none.</summary>
    public ParagraphBorder? Right { get; init; }

    /// <summary>The rule across the top of the paragraph, or null when it states none.</summary>
    public ParagraphBorder? Top { get; init; }

    /// <summary>The rule across the bottom of the paragraph, or null when it states none.</summary>
    public ParagraphBorder? Bottom { get; init; }

    /// <summary>
    /// The rule drawn where this paragraph joins an identically bordered successor, or null.
    /// </summary>
    /// <remarks>
    /// Only ever drawn on a join, and 5 of the 158 <c>w:between</c> elements in the words corpus state a
    /// rule at all — the rest say <c>none</c>. <see cref="Join"/> is what turns it into a bottom border.
    /// </remarks>
    public ParagraphBorder? Between { get; init; }

    /// <summary>
    /// True when this paragraph continues the box the paragraph above it opened.
    /// </summary>
    /// <remarks>
    /// Only the drawing needs it, and only to close a gap: two joined paragraphs still keep whatever
    /// <c>w:spacing</c> stands between them, and LibreOffice runs the box's side rules across that space
    /// rather than stopping at each paragraph's own text. Measured on
    /// <c>batch-017/docx/Sample_SQMS_Program.docx</c> page 46, where the reference's left rule is
    /// continuous from 568.25 to 607.85 and ours broke at every 6 pt gap — which cost that page its only
    /// regression in the whole sweep.
    /// </remarks>
    public bool JoinsAbove { get; init; }

    /// <summary>True when no side draws and none takes room.</summary>
    public bool IsEmpty
        => Left is null && Right is null && Top is null && Bottom is null && Between is null;

    /// <summary>True when at least one of the four sides draws a rule.</summary>
    public bool Draws
        => (Left?.Draws ?? false) || (Right?.Draws ?? false)
           || (Top?.Draws ?? false) || (Bottom?.Draws ?? false);

    /// <summary>The room the top border takes above the paragraph's first line.</summary>
    public Length Above => Top?.Allowance ?? Length.Zero;

    /// <summary>The room the bottom border takes below the paragraph's last line.</summary>
    public Length Below => Bottom?.Allowance ?? Length.Zero;

    /// <summary>
    /// True when this paragraph and the next are bordered alike, so the two share one box.
    /// </summary>
    /// <remarks>
    /// Measured: two consecutive paragraphs stating the same four borders are drawn as a single box —
    /// the second takes no top allowance, the first no bottom allowance, and no rule is drawn where they
    /// meet. Writer joins on the borders and the indents together (<c>SwBorderAttrs::CmpLeftRight</c> and
    /// <c>JoinWithCmp</c>), because two boxes of different widths cannot be one box.
    /// </remarks>
    public bool JoinsWith(ParagraphBorderSet? next)
        => next is not null
           && Left == next.Left && Right == next.Right
           && Top == next.Top && Bottom == next.Bottom
           && Between == next.Between
           && !IsEmpty;

    /// <summary>
    /// The pair of border sets two joined paragraphs are actually drawn and measured with.
    /// </summary>
    /// <remarks>
    /// The upper keeps its top and loses its bottom to the <c>w:between</c> rule, if any; the lower loses
    /// its top and keeps its bottom. A <c>w:between</c> keeps its distance on both sides of itself, which
    /// is why it becomes a bottom border carrying <see cref="ParagraphBorder.Trailing"/>.
    /// </remarks>
    public static (ParagraphBorderSet Upper, ParagraphBorderSet Lower) Join(ParagraphBorderSet set)
    {
        ArgumentNullException.ThrowIfNull(set);

        ParagraphBorder? between = set.Between is { Draws: true } rule
            ? rule with { Trailing = rule.Space }
            : null;

        return (set with { Bottom = between, Between = null },
                set with { Top = null, Between = null, JoinsAbove = true });
    }
}

/// <summary>
/// Merges each run of identically bordered consecutive paragraphs into one box.
/// </summary>
/// <remarks>
/// <para>
/// Word and Writer both draw two adjacent paragraphs carrying the same border set as a single box: no
/// rule between them, and neither the lower one's top allowance nor the upper one's bottom allowance is
/// spent. Measured — two such paragraphs sit one plain line pitch apart, 12.65 pt, where two separately
/// boxed ones would be 6.50 pt further.
/// </para>
/// <para>
/// Done as a pass over the finished blocks rather than in the paginator because the join changes the
/// picture <em>and</em> the height, and the two have to agree: a border drawn where no room was reserved
/// lands on the text below it.
/// </para>
/// <para>
/// Here rather than in either reader because nothing in it is format-specific — <c>w:pBdr</c>,
/// <c>sprmPBrcTop</c> and <c>fo:border-top</c> all arrive as a
/// <see cref="ParagraphBorderSet"/> and the run is found by comparing those.
/// </para>
/// </remarks>
public static class ParagraphBorderJoin
{
    /// <summary>Rewrites a flow's blocks so that each run of alike-bordered paragraphs is one box.</summary>
    /// <remarks>
    /// A table between two bordered paragraphs breaks the run, which is what the type test does — only a
    /// <see cref="PageParagraph"/> can join, and anything else resets.
    /// </remarks>
    /// <param name="blocks">The blocks of one flow, in order. Rewritten in place.</param>
    public static void Apply(List<PageBlock> blocks)
    {
        ArgumentNullException.ThrowIfNull(blocks);

        // The join is decided from what the reader stated, never from what an earlier step of this walk
        // has already rewritten: the second paragraph of a run of three has had its top removed by the
        // time the third is examined, and comparing that against the third's untouched set would find
        // them different and break the run in the middle.
        ParagraphBorderSet?[] stated =
            [.. blocks.Select(block => (block as PageParagraph)?.Borders)];

        for (int i = 0; i + 1 < blocks.Count; i++)
        {
            if (stated[i] is not { } borders || !borders.JoinsWith(stated[i + 1])) continue;
            if (blocks[i] is not PageParagraph upper || blocks[i + 1] is not PageParagraph lower) continue;

            (ParagraphBorderSet above, ParagraphBorderSet below) = ParagraphBorderSet.Join(borders);

            // Only the sides the join settles are taken from the pair, so a paragraph in the middle of a
            // run keeps the top the previous step already removed.
            blocks[i] = upper with
            {
                Borders = (upper.Borders ?? borders) with { Bottom = above.Bottom, Between = null },
            };

            blocks[i + 1] = lower with
            {
                Borders = (lower.Borders ?? borders) with
                {
                    Top = below.Top, Between = null, JoinsAbove = true,
                },
            };
        }
    }
}
