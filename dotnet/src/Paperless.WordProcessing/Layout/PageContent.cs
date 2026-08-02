using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Itemisation;
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

    /// <summary>
    /// Its resolved layout properties, with room made on the first line for a list label.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>The only formatting anything downstream should measure against.</strong> A list label is
    /// drawn beside the paragraph's text rather than spliced into it — see <see cref="PageLabel"/> — so
    /// something has to hold the first line's text back far enough to leave room, and it is this: the
    /// declared first-line indent, which for a list is negative, widened by the label's own advance.
    /// Writer arrives at the same first line by making the label a portion within it
    /// (<c>SwNumberPortion::Format</c>, <c>sw/source/core/text/porfld.cxx:607</c>).
    /// </para>
    /// <para>
    /// Adjusted here rather than at each of the five places that lay a paragraph out, because a paragraph
    /// measured against one first-line indent and drawn against another puts its own words in two
    /// different places. <see cref="DeclaredFormat"/> is what the reader actually said, and the label
    /// hangs at <em>its</em> <see cref="ParagraphFormat.LineStart"/>.
    /// </para>
    /// </remarks>
    public ParagraphFormat Format
    {
        get => Label is null
            ? _format
            : _format with { FirstLineIndent = _format.FirstLineIndent + LabelAdvance };
        init => _format = value;
    }

    private readonly ParagraphFormat _format = ParagraphFormat.Default;

    /// <summary>The formatting as the reader stated it, before the label was allowed for.</summary>
    /// <remarks>
    /// Where the label's own pen sits, and what a test asserting a reader's work should compare against
    /// rather than the widened <see cref="Format"/>.
    /// </remarks>
    public ParagraphFormat DeclaredFormat => _format;

    /// <summary>
    /// The label this paragraph draws in front of its first line, or null when it draws none.
    /// </summary>
    /// <remarks>
    /// Null for the overwhelming majority of paragraphs, and for the continuation paragraphs of a
    /// multi-paragraph list item as well: ODF gives the label to the first <c>text:p</c> of a
    /// <c>text:list-item</c> only, and the other three formats say the same thing by putting no list
    /// instance on the paragraph. Such a paragraph keeps the level's indents and draws nothing.
    /// </remarks>
    public PageLabel? Label { get; init; }

    /// <summary>How far the label pushes the first line's text along, or zero when there is none.</summary>
    internal Length LabelAdvance
        => Label?.Advance(-_format.FirstLineIndent, _format.StartIndent + _format.FirstLineIndent)
           ?? Length.Zero;

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

    /// <summary>
    /// The device grid the paragraph's fonts are measured through, or null to measure them exactly.
    /// </summary>
    /// <remarks>
    /// Null for every document but the few that ask to be laid out against a printer rather than against a
    /// virtual device — see <see cref="MetricGrid"/>. Carried on the paragraph rather than passed down the
    /// layout call chain because a header, a table cell and a text box all need the same answer and all
    /// reach the layouter by different routes; the reader that knows the document's answer sets it once.
    /// </remarks>
    public MetricGrid? Metrics { get; init; }

    /// <summary>
    /// The direction its bidi resolution takes as its base.
    /// </summary>
    /// <remarks>
    /// The declared writing mode first and the runs' shaping options after it, which is the rule
    /// <see cref="MeasuredParagraph"/> applies when it is handed no itemisation of its own. One
    /// rule rather than two, because measuring a paragraph at one base level and drawing it at
    /// another puts its sub-runs in an order its own widths do not describe.
    /// </remarks>
    public BidiDirection BaseDirection
        => Format.IsRightToLeft || (HasRuns ? Runs[0].Shaping : Shaping).RightToLeft
            ? BidiDirection.RightToLeft
            : BidiDirection.LeftToRight;

    /// <summary>
    /// How to cut it into sub-runs, or null for the neutral settings.
    /// </summary>
    /// <remarks>
    /// Null rather than a left-to-right instance for the paragraph that needs nothing, so a
    /// document that says nothing about direction is measured through exactly the path it took
    /// before writing modes existed — including a caller that says right-to-left on its runs and
    /// nothing on the paragraph, which is how it had to be said before.
    /// </remarks>
    internal ItemisationOptions? Itemisation
        => Format.IsRightToLeft
            ? new ItemisationOptions { BaseDirection = BidiDirection.RightToLeft }
            : null;

    /// <summary>
    /// The notes anchored in the paragraph's text, in order.
    /// </summary>
    /// <remarks>
    /// Carried on the paragraph because that is where the anchor is: a footnote occupies a character
    /// position in the sentence that cites it, and its body lives at the foot of whichever page that
    /// position lands on. Which page that is cannot be known until the paragraph is placed, which is what
    /// makes notes a pagination matter rather than a reading one.
    /// </remarks>
    public IReadOnlyList<PageNote> Notes { get; init; } = [];

    /// <summary>
    /// The floating frames anchored in this paragraph, in document order.
    /// </summary>
    /// <remarks>
    /// On the paragraph because that is where every format puts the anchor, a page-anchored frame
    /// included: even <c>text:anchor-type="page"</c> is written at a position in the text, and Word has
    /// no page anchor at all — its page-relative positions are still anchored to a paragraph. So which
    /// page a frame lands on is a pagination result rather than a property, which is what makes frames a
    /// two-pass affair; see <see cref="Paginator"/>.
    /// </remarks>
    public IReadOnlyList<PageFrame> Frames { get; init; } = [];

    /// <summary>
    /// The as-character frames among <see cref="Frames"/>: room <em>on</em> a line rather than beside it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Derived rather than stored, because a frame states one thing and layout needs it twice — an
    /// as-character frame is placed by hanging it on its line, and the <em>same</em> frame is what makes
    /// that line wider and taller. Deriving keeps the two from disagreeing, which they would the first
    /// time a reader set one and forgot the other.
    /// </para>
    /// <para>
    /// Empty for a paragraph with no inline frame, which is nearly all of them, and that is what lets the
    /// paginator keep taking the cheaper single-face measurement for those.
    /// </para>
    /// </remarks>
    public IReadOnlyList<InlineObject> InlineObjects =>
        HasInlineObjects
            ? [.. Frames
                .Where(frame => frame.Anchor == FrameAnchor.AsCharacter)
                .Select(frame => new InlineObject(
                    frame.AnchorOffset, frame.Size.Width, frame.Size.Height, frame.InlineAscent))]
            : [];

    /// <summary>True when an as-character frame is set in the paragraph's text.</summary>
    public bool HasInlineObjects
        => Frames.Count > 0 && Frames.Any(frame => frame.Anchor == FrameAnchor.AsCharacter);

    /// <summary>
    /// Shapes the paragraph's runs, ready for measuring across them.
    /// </summary>
    /// <remarks>
    /// Here rather than in the paginator because the body, a header, a table cell and a text box all need
    /// the same answer, and they used to arrive at it separately — the flow layouter's copy passed
    /// <see cref="Runs"/> straight through, so a uniform paragraph reaching the run path measured as
    /// nothing at all. The paragraph's own face and size close any gap the runs leave, so a document that
    /// formats its text and leaves its paragraph mark unmentioned is normal rather than malformed.
    /// </remarks>
    internal MeasuredParagraph Measure()
    {
        List<FormattedRun> runs = [.. Runs.Select(run => run.ToFormattedRun())];

        if (runs.Count == 0)
        {
            runs.Add(new FormattedRun(0, Text.Length, Face, EmSize, Shaping));
        }

        return MeasuredParagraph.Measure(Text, runs, shaper: null, Itemisation, InlineObjects, Metrics);
    }
}

/// <summary>
/// One note anchored in a paragraph: a footnote or an endnote.
/// </summary>
/// <remarks>
/// The body is blocks rather than paragraphs for the same reason a cell's is — a note can contain a table,
/// and it is laid out by <see cref="FlowLayouter"/> either way.
/// </remarks>
public sealed record PageNote
{
    /// <summary>The note's body.</summary>
    public required IReadOnlyList<PageBlock> Blocks { get; init; }

    /// <summary>
    /// Where its anchor sits in the citing paragraph's text.
    /// </summary>
    /// <remarks>
    /// A character offset, which the readers already mark with U+0001 — the anchor occupies a position and
    /// has a width but is not text. The offset is what decides which page the note lands on: the page
    /// holding the <em>line</em> that contains this offset.
    /// </remarks>
    public int Offset { get; init; }

    /// <summary>True for an endnote, which is a class rather than a position — see <see cref="Placement"/>.</summary>
    /// <remarks>
    /// Kept apart from the placement because the two really are different questions: an endnote numbered in
    /// roman and collected at the end of a section is still an endnote, and a reader wanting to list a
    /// document's endnotes should not have to know where they were put.
    /// </remarks>
    public bool IsEndnote { get; init; }

    /// <summary>Where the note collects.</summary>
    /// <remarks>
    /// Defaults to the foot of the page, which is what a footnote is and what an endnote becomes when the
    /// document asks for its endnotes at the end of each section.
    /// </remarks>
    public NotePlacement Placement { get; init; }

    /// <summary>Where this class of note begins counting again.</summary>
    /// <remarks>
    /// Carried on the note beside <see cref="Placement"/>, and for the same reason: both are properties of the
    /// note's <em>class</em> that only pagination can act on, and the paginator is handed notes rather than the
    /// document. This is the one numbering rule a reader cannot resolve — a note's number under a restart is
    /// its position within its page, and which page it is on is what filling the page decides.
    /// </remarks>
    public NoteRestart Restart { get; init; }

    /// <summary>
    /// How this note's class is numbered, for a pagination pass that has to number it again.
    /// </summary>
    /// <remarks>
    /// The sequence and the start value, which <see cref="Restart"/> alone cannot supply: a per-page restart
    /// says the count begins again and this says what the count is written in. Defaults to the footnote
    /// sequence, which is what a note whose reader states nothing renders as.
    /// </remarks>
    public NoteNumbering Numbering { get; init; } = NoteNumbering.Footnotes;

    /// <summary>
    /// The citation this note carries as it was read, in document order.
    /// </summary>
    /// <remarks>
    /// Kept so that a renumbering pass can find it again. It sits in the citing paragraph's text at
    /// <see cref="Offset"/> and in the note body's first paragraph at <see cref="BodyOffset"/>, in both cases
    /// exactly this many characters long — LibreOffice draws a note's number twice and the readers emit it
    /// twice, so both have to be rewritten or the sentence and the note disagree about which note it is.
    /// </remarks>
    public string Citation { get; init; } = "";

    /// <summary>
    /// Where the citation sits in the first block of <see cref="Blocks"/>.
    /// </summary>
    /// <remarks>
    /// Zero in three of the four formats, which prepend it, and not in DOCX: the note body marks where its own
    /// number goes with a <c>w:footnoteRef</c>, and a note beginning with a tab puts it at one rather than at
    /// nought. Recorded rather than searched for, because searching a note's text for the string "1" finds
    /// whatever the note happens to say first.
    /// </remarks>
    public int BodyOffset { get; init; }
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
/// <param name="Rise">
/// How far the run is raised above the baseline; negative lowers it. What a superscript is, together with
/// the smaller <paramref name="EmSize"/> that goes with it — the two are independent, and a document can
/// raise text without shrinking it.
/// </param>
/// <param name="CaseMap">
/// The case the run's text is drawn in, which is not the case it is stored in — <c>w:caps</c>,
/// <c>w:smallCaps</c> and their counterparts in the other three formats. Resolved away by
/// <see cref="CaseMapping.Apply"/> before the paragraph is measured, so nothing downstream of a reader
/// ever sees a value other than <see cref="PageCaseMap.None"/>.
/// </param>
/// <param name="MetricEmSize">
/// The size the run's line metrics are taken at, or zero for <paramref name="EmSize"/>. Set only by the
/// small-capitals split; see <see cref="FormattedRun.MetricEmSize"/> for why the two sizes differ.
/// </param>
public readonly record struct PageRun(
    int Start,
    int Length,
    OpenTypeFace Face,
    Length EmSize,
    FontReference? Font = null,
    Colour Colour = default,
    ShapingOptions Shaping = default,
    Length Rise = default,
    PageCaseMap CaseMap = PageCaseMap.None,
    Length MetricEmSize = default)
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
    public FormattedRun ToFormattedRun() => new(Start, Length, Face, EmSize, Shaping, MetricEmSize);
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
/// Its own block list rather than an index into the body's, because each of the three <em>is</em> a
/// separate flow: a header's paragraphs are not the document's body text, and a
/// <see cref="PlacedLine.ParagraphIndex"/> pointing into the body would name the wrong paragraph. Two
/// pages sharing one header share this whole object.
/// </para>
/// <para>
/// A flow holds tables as well as lines, because all three of the things it models can contain one: a
/// table inside a cell is how every format writes a nested table, and a table inside a header is how a
/// two-part running head is usually laid out. What a flow does <em>not</em> do is paginate — a nested table
/// that outgrows its cell overflows rather than splitting, since a cell belongs to its row.
/// </para>
/// </remarks>
public sealed record PlacedFlow
{
    /// <summary>The blocks the lines index into.</summary>
    public required IReadOnlyList<PageBlock> Blocks { get; init; }

    /// <summary>The lines, in order, positioned relative to the area's top.</summary>
    public required IReadOnlyList<PlacedLine> Lines { get; init; }

    /// <summary>The tables inside the flow, with page-coordinate rectangles.</summary>
    public IReadOnlyList<PlacedTable> Tables { get; init; } = [];

    /// <summary>Where the flow sits on the page.</summary>
    public required DocRect Area { get; init; }

    /// <summary>
    /// How far the flow advanced in all — the <em>last</em> block's own lower spacing included.
    /// </summary>
    /// <remarks>
    /// Different from where the ink stops, which is what <see cref="FlowLayouter.Extent"/> reports, and
    /// the difference is a table cell's whole point. Writer's
    /// <c>SwFlowFrame::CalcAddLowerSpaceAsLastInTableCell</c> adds the last frame's lower spacing to the
    /// cell under the <c>AddParaSpacingToTableCells</c> setting, which both the DOC and the DOCX
    /// importers switch on — so in a Word document every cell is as tall as its content plus the space
    /// after its final paragraph. Sizing rows from the ink instead makes each one short by that spacing,
    /// which on a long table is many pages.
    /// </remarks>
    public Length Advance { get; init; }

    /// <summary>True when nothing was laid out.</summary>
    public bool IsEmpty => Lines.Count == 0 && Tables.Count == 0;
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
    /// The blocks this page's lines index into, or null for the document's own.
    /// </summary>
    /// <remarks>
    /// Null on almost every page, and the exception is the reason it exists: the endnote pages at the end of a
    /// document are laid out from a flow assembled out of the notes' bodies rather than from the body's blocks,
    /// so their <see cref="PlacedLine.ParagraphIndex"/> counts in a different list. A page carrying its own
    /// list is how that stays correct without every page paying for a copy — and a null here is not "no
    /// blocks" but "the ones the sequence holds".
    /// </remarks>
    public IReadOnlyList<PageBlock>? Blocks { get; init; }

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
    /// True when the page's section reads right to left, so that its first column is the rightmost.
    /// </summary>
    /// <remarks>
    /// Carried on the page rather than looked up from the section for the same reason
    /// <see cref="ColumnArea"/> is: a renderer is handed a page, and a page that had to consult the
    /// section could disagree with the one that laid the lines out.
    /// </remarks>
    public bool IsRightToLeft { get; init; }

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

        // The leading edge is the right one in a right-to-left section, so its first column is the
        // rightmost — see PageGeometry.IsRightToLeft, where it is measured.
        if (IsRightToLeft) at = columns - 1 - at;

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

    /// <summary>
    /// The footnotes at the foot of the page, or null when it has none.
    /// </summary>
    /// <remarks>
    /// Bottom-aligned inside <see cref="BodyArea"/> rather than below it, which is measured rather than
    /// assumed: the last note line's box bottom coincides with the body area's bottom. So the notes take
    /// their room out of the body's, which is why a page with notes holds less text — and why adding one can
    /// push the line that cites it onto the next page.
    /// </remarks>
    public PlacedFlow? Notes { get; init; }

    /// <summary>
    /// The rule above the notes, or null when the page has none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A rectangle rather than a line, because that is what it is: Writer's <c>Footnote Separator</c> is a
    /// frame style with a width, a thickness and an alignment, and LibreOffice's PDF export writes it as a
    /// filled path rather than a stroke. Measured from that path — 56.7 to 177.15 pt on an A4 page with 2 cm
    /// margins, half a point thick — which makes it a quarter of the text width, left aligned, 0.5 pt.
    /// </para>
    /// <para>
    /// Carried on the page rather than derived by a backend, because its position depends on where the notes
    /// ended up and only pagination knows that.
    /// </para>
    /// </remarks>
    public DocRect? NoteSeparator { get; init; }

    /// <summary>
    /// The floating frames that landed on this page, with the rectangles they were given.
    /// </summary>
    /// <remarks>
    /// Beside the lines rather than among them, for the same reason a table is: a frame is placed at a
    /// resolved position rather than stacked, and the lines around it have already been shortened to make
    /// room. A renderer draws these after the body text, which is what puts an opaque frame over the text
    /// it displaced rather than under it.
    /// </remarks>
    public IReadOnlyList<PlacedFrame> Frames { get; init; } = [];

    /// <summary>How much of the body area the lines used.</summary>
    public Length UsedHeight =>
        Lines.Count == 0 ? Length.Zero : Lines[^1].Top + Lines[^1].Box.Height;

    /// <summary>True when nothing landed on the page.</summary>
    public bool IsEmpty => Lines.Count == 0 && Tables.Count == 0;
}
