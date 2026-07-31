using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Layout;
using Paperless.Text.Shaping;

namespace Paperless.WordProcessing.Layout;

/// <summary>
/// One piece of body-level content waiting to be paginated: a paragraph or a table.
/// </summary>
/// <remarks>
/// <para>
/// The distinction Writer's own layout draws, and for the same reason: a body frame holds text frames and
/// table frames side by side, and the two flow differently. A paragraph is a run of lines that can be cut
/// anywhere a line ends; a table is a grid whose rows are sized by their tallest cell, and whose cells are
/// each a flow of their own. Neither reduces to the other — flattening a table into its cells' paragraphs
/// would give the page a height no table has.
/// </para>
/// <para>
/// A closed hierarchy of exactly two cases: sections and floating frames will be pages' business rather
/// than blocks', because a section changes the page and a floating frame is anchored rather than flowed.
/// </para>
/// </remarks>
public abstract record PageBlock
{
    /// <summary>The caller's own reference to whatever this came from.</summary>
    /// <remarks>
    /// Pagination reorders nothing but it does split and drop things, so a caller needs to get back from a
    /// laid-out line to the node it belongs to; carrying an opaque reference is cheaper than making the
    /// engine know about the document model.
    /// </remarks>
    public object? Source { get; init; }

    /// <summary>
    /// Which of the document's sections this block belongs to.
    /// </summary>
    /// <remarks>
    /// On the block rather than worked out by the paginator, because only the reader can know it: three of
    /// the four formats delimit sections by position in a stream the layout engine never sees, and ODF does
    /// not delimit them at all — a paragraph reaches its page description through its style's master page.
    /// Zero for a document with one section, which is most of them.
    /// </remarks>
    public int SectionIndex { get; init; }
}

/// <summary>
/// A paragraph waiting to be paginated: its text, its resolved formatting, and the face it is set in.
/// </summary>
/// <remarks>
/// <para>
/// The paginator's input, deliberately not the document model. Pagination needs a flat sequence of
/// things with heights, and a paragraph's height depends only on its text, its format, its face and the
/// width it is given — so taking exactly that keeps the engine testable against hand-built input rather
/// than only against a whole document, and keeps it usable by whichever pass eventually builds it.
/// </para>
/// </remarks>
public sealed record PageParagraph : PageBlock
{
    /// <summary>The paragraph's text, without its terminating mark.</summary>
    public required string Text { get; init; }

    /// <summary>The face the text is set in.</summary>
    public required OpenTypeFace Face { get; init; }

    /// <summary>
    /// The resolved font reference, for a renderer that has to name the face it is drawing with.
    /// </summary>
    /// <remarks>
    /// Kept beside the face rather than derived from it, because the two answer different questions: the
    /// face has the metrics that decided the layout, and the reference records <em>which</em> face that
    /// was and what was asked for before substitution. A PDF backend deduplicates embedded fonts on the
    /// reference's key, and a comparison against a reference renderer needs the requested family to
    /// explain a difference.
    /// </remarks>
    public FontReference? Font { get; init; }

    /// <summary>The colour the text is drawn in.</summary>
    /// <remarks>
    /// Black by default rather than nothing, since a run with no colour is drawn in the document's text
    /// colour and every format's default for that is black.
    /// </remarks>
    public Colour Colour { get; init; } = Colour.Black;

    /// <summary>Its resolved layout properties.</summary>
    public ParagraphFormat Format { get; init; } = ParagraphFormat.Default;

    /// <summary>The em size the text is set at.</summary>
    public Length EmSize { get; init; } = Length.FromPoints(12);

    /// <summary>A BCP 47 tag, for the language-specific break rules.</summary>
    public string? Language { get; init; }

    /// <summary>How the text is shaped; the default is what Writer does.</summary>
    public ShapingOptions Shaping { get; init; }

    /// <summary>
    /// The paragraph's runs, when its formatting is not uniform.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Empty means uniform: the whole paragraph is measured and drawn in <see cref="Face"/> at
    /// <see cref="EmSize"/>, which is what a paragraph of plain text is and by far the common case. When
    /// runs are present they partition the text and each carries its own face, size and colour, and the
    /// line height becomes the tallest run's on that line rather than the paragraph's.
    /// </para>
    /// <para>
    /// <see cref="Face"/> and <see cref="EmSize"/> stay required even so, because they are the
    /// paragraph's own — what its mark carries, and what an empty paragraph is as tall as.
    /// </para>
    /// </remarks>
    public IReadOnlyList<PageRun> Runs { get; init; } = [];

    /// <summary>True when the paragraph's formatting varies across its text.</summary>
    public bool HasRuns => Runs.Count > 0;
}

/// <summary>
/// One run of a paragraph: a range of its text with its own formatting.
/// </summary>
/// <remarks>
/// The measurement half and the drawing half of a run travel together here, unlike in
/// <see cref="FormattedRun"/>, which carries only what changes a width. A colour does not move a line
/// break but it does decide what a backend is handed, and splitting the two would mean matching them up
/// again by range.
/// </remarks>
/// <param name="Start">The run's first character, as an index into the paragraph's text.</param>
/// <param name="Length">How many characters it covers.</param>
/// <param name="Face">The face it is set in.</param>
/// <param name="EmSize">The em size it is set at.</param>
/// <param name="Font">The resolved reference, for a backend that has to name the face.</param>
/// <param name="Colour">The colour it is drawn in.</param>
/// <param name="Shaping">How it is shaped.</param>
public readonly record struct PageRun(
    int Start,
    int Length,
    OpenTypeFace Face,
    Length EmSize,
    FontReference? Font = null,
    Colour Colour = default,
    ShapingOptions Shaping = default)
{
    /// <summary>One past the run's last character.</summary>
    public int End => Start + Length;

    /// <summary>The colour to draw with, black when the run states none.</summary>
    /// <remarks>
    /// A <c>default</c> colour is fully transparent black, which would draw nothing — so an unstated
    /// colour has to mean the document's text colour rather than the struct's default.
    /// </remarks>
    public Colour EffectiveColour => Colour.A == 0 ? Core.Graphics.Colour.Black : Colour;

    /// <summary>The measurement half of this run.</summary>
    public FormattedRun ToFormattedRun() => new(Start, Length, Face, EmSize, Shaping);
}

/// <summary>
/// One line, placed on a page.
/// </summary>
/// <param name="ParagraphIndex">Which paragraph of the input it belongs to.</param>
/// <param name="LineIndex">Which line of that paragraph it is, counted from the paragraph's first.</param>
/// <param name="Box">The line as its paragraph laid it out, relative to the paragraph's top.</param>
/// <param name="Top">
/// Where the line's box sits on this page, measured from the top of the page's body area — so unlike
/// <see cref="LineBox.Top"/> this is a position on a page rather than within a paragraph.
/// </param>
/// <param name="Column">
/// Which column of the page it is in, counted from zero. Zero for the single-column text that most
/// documents are, so the field costs nothing to ignore — but it is what a caller has to consult to know
/// <em>which</em> rectangle <see cref="Top"/> is measured from, since a second column's lines start again
/// at the top of the page.
/// </param>
public readonly record struct PlacedLine(
    int ParagraphIndex,
    int LineIndex,
    LineBox Box,
    Length Top,
    int Column = 0)
{
    /// <summary>The baseline's distance from the top of the body area.</summary>
    public Length Baseline => Top + Box.Baseline;

    /// <summary>True when this is the first line of its paragraph.</summary>
    public bool StartsParagraph => LineIndex == 0;
}

/// <summary>
/// A flow of paragraphs laid out into a rectangle of its own: a header, a footer, or a table cell.
/// </summary>
/// <remarks>
/// <para>
/// One type for the three because they are the same thing seen three times — a list of paragraphs, the
/// lines they broke into, and the rectangle those lines are measured from. What differs is only where the
/// rectangle is and who decided its width, which is the caller's business rather than the flow's. Sharing
/// it means one drawing path serves all three, so tabs and per-run formatting cannot drift between a
/// header and a cell.
/// </para>
/// <para>
/// Its own paragraph list rather than an index into the body's, because each of the three <em>is</em> a
/// separate flow: a header's paragraphs are not the document's body text, and a
/// <see cref="PlacedLine.ParagraphIndex"/> pointing into the body would name the wrong paragraph. Two
/// pages sharing one header share this whole object.
/// </para>
/// </remarks>
public sealed record PlacedFlow
{
    /// <summary>The paragraphs the lines index into.</summary>
    public required IReadOnlyList<PageParagraph> Paragraphs { get; init; }

    /// <summary>The lines, in order, positioned relative to the area's top.</summary>
    public required IReadOnlyList<PlacedLine> Lines { get; init; }

    /// <summary>Where the furniture sits on the page.</summary>
    public required DocRect Area { get; init; }

    /// <summary>True when nothing was laid out.</summary>
    public bool IsEmpty => Lines.Count == 0;
}

/// <summary>
/// A page after pagination: how big it is, where its body sits, and which lines landed on it.
/// </summary>
/// <remarks>
/// Lines only, not paragraphs, because a paragraph can span pages and a page is defined by what fits on
/// it. A caller wanting the paragraphs asks which <see cref="PlacedLine.ParagraphIndex"/> values appear;
/// a caller wanting to know whether a paragraph was split compares that across pages.
/// </remarks>
public sealed record LaidOutPage
{
    /// <summary>The page's zero-based position in the document.</summary>
    public required int Index { get; init; }

    /// <summary>
    /// The number printed on the page, which is not the index.
    /// </summary>
    /// <remarks>
    /// A section can restart numbering, and a title page numbered zero so that the following page is
    /// one is a real thing people do — so the two are kept apart rather than one derived from the other.
    /// </remarks>
    public required int Number { get; init; }

    /// <summary>The sheet's size.</summary>
    public required DocSize Size { get; init; }

    /// <summary>Where body text goes, in page coordinates.</summary>
    /// <remarks>
    /// The whole text area, columns and the gaps between them included. A line's own coordinates are
    /// relative to <em>its column's</em> rectangle rather than to this — see
    /// <see cref="ColumnArea"/> — which for the single-column case are the same thing.
    /// </remarks>
    public required DocRect BodyArea { get; init; }

    /// <summary>How many columns the page's text area is divided into; one for ordinary text.</summary>
    public int ColumnCount { get; init; } = 1;

    /// <summary>The gap between two columns.</summary>
    public Length ColumnGap { get; init; }

    /// <summary>
    /// One column's rectangle, which is what a line's own coordinates are relative to.
    /// </summary>
    /// <remarks>
    /// Carried on the page rather than looked up from the section, because a page is what a renderer is
    /// handed: recomputing this would mean giving the renderer the section too, and the two could then
    /// disagree about a page laid out before a geometry change.
    /// </remarks>
    /// <param name="column">The column, counted from zero at the leading edge.</param>
    public DocRect ColumnArea(int column)
    {
        int columns = Math.Max(1, ColumnCount);
        int at = Math.Clamp(column, 0, columns - 1);

        Length gaps = ColumnGap * (columns - 1);
        Length width = BodyArea.Width - gaps;
        width = width > Length.Zero ? width / columns : BodyArea.Width;

        return new DocRect(
            BodyArea.X + ((width + ColumnGap) * at), BodyArea.Y, width, BodyArea.Height);
    }

    /// <summary>The lines on the page, in order.</summary>
    public required IReadOnlyList<PlacedLine> Lines { get; init; }

    /// <summary>
    /// The tables on the page, or the parts of them that landed here.
    /// </summary>
    /// <remarks>
    /// Beside the lines rather than among them, because a table is not a run of lines: its cells sit side
    /// by side and each carries its own rectangle. A table crossing a page break appears once per page it
    /// touches, each time with the rows that fit and its headings repeated.
    /// </remarks>
    public IReadOnlyList<PlacedTable> Tables { get; init; } = [];

    /// <summary>Which section's geometry the page was laid on.</summary>
    public int SectionIndex { get; init; }

    /// <summary>The page's header, or null when it has none.</summary>
    /// <remarks>
    /// Per page rather than per section, because a section's first and even pages can each take a different
    /// one — and because a page number in a header makes even two pages sharing a slot differ once fields
    /// are resolved.
    /// </remarks>
    public PlacedFlow? Header { get; init; }

    /// <summary>The page's footer, or null when it has none.</summary>
    public PlacedFlow? Footer { get; init; }

    /// <summary>How much of the body area the lines used.</summary>
    public Length UsedHeight =>
        Lines.Count == 0 ? Length.Zero : Lines[^1].Top + Lines[^1].Box.Height;

    /// <summary>True when nothing landed on the page.</summary>
    public bool IsEmpty => Lines.Count == 0 && Tables.Count == 0;
}
