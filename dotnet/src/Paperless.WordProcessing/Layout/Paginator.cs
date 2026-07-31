using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.Text.Layout;
using Paperless.WordProcessing.Model;

namespace Paperless.WordProcessing.Layout;

/// <summary>
/// The choices about page filling that a document does not settle on its own.
/// </summary>
/// <remarks>
/// Every one of these is a place where Word and Writer behave differently and the file says which by a
/// compatibility flag rather than by a property. Putting them here rather than on the paragraph is what
/// lets one engine paginate a DOCX the way Word would and an ODT the way Writer would.
/// </remarks>
public sealed record PaginationOptions
{
    /// <summary>Writer's behaviour, which is what an ODF document expects.</summary>
    public static PaginationOptions Default { get; } = new();

    /// <summary>
    /// Word's behaviour, which is what an OOXML document expects.
    /// </summary>
    /// <remarks>
    /// Established by measurement rather than from the specifications, and worth stating how: the same
    /// document was exported from LibreOffice to both ODT and DOCX and rendered, and the two paginate
    /// differently. Its 41st line sits at 762.1 pt on the DOCX's first page and 767.8 pt on the ODT's —
    /// 5.65 pt apart, which is exactly the one paragraph space-after on that page. So LibreOffice
    /// <em>adds</em> space-after to space-before for an ODF document and takes the <em>larger</em> for an
    /// OOXML one, and the difference moves a page break within five pages.
    /// </remarks>
    public static PaginationOptions Word { get; } = new()
    {
        KeepsSpacingAtTopOfPage = true,
        CollapsesSpacing = true,
    };

    /// <summary>
    /// Whether a paragraph keeps its space-before when it starts a page.
    /// </summary>
    /// <remarks>
    /// Writer's <c>PARA_SPACE_MAX_AT_PAGES</c>: it drops the spacing, so the first line of a page sits at
    /// the top margin whatever the paragraph asks for, while Word keeps it. Getting it wrong moves the
    /// first baseline of every page after the first, which then changes how much fits and where the next
    /// break falls. Unlike <see cref="CollapsesSpacing"/> this is not pinned by a comparison yet: the
    /// corpus document's page tops all fall mid-paragraph or on paragraphs with no space-before, so it
    /// makes no difference there.
    /// </remarks>
    public bool KeepsSpacingAtTopOfPage { get; init; }

    /// <summary>
    /// Whether the space between two paragraphs is the larger of the two rather than their sum.
    /// </summary>
    /// <remarks>
    /// Writer's <c>PARA_SPACE_MAX</c> compatibility flag, and the one difference between the two presets
    /// that a comparison actually pins down. LibreOffice enables it when it imports an OOXML document and
    /// leaves it off for a native one, so a paragraph with 0.2 cm of space-after followed by one with
    /// 0.4 cm of space-before gets 0.4 cm between them in a DOCX and 0.6 cm in an ODT — from the same
    /// source document, exported both ways. On a five-page document that is one page break.
    /// </remarks>
    public bool CollapsesSpacing { get; init; }

    /// <summary>
    /// Upper bound on pages, as a guard against a document that cannot be paginated.
    /// </summary>
    /// <remarks>
    /// Not a limit anyone should hit: a paragraph too tall for its own page, or a zero-height body area,
    /// would otherwise fill pages until memory ran out. Reaching it is reported rather than silent.
    /// </remarks>
    public int MaxPages { get; init; } = 20000;

    /// <summary>
    /// How much room the footnote separator takes above the notes.
    /// </summary>
    /// <remarks>
    /// The rule above a page's footnotes, plus the space around it. Writer's default is a quarter of the
    /// text width at half a point, with the spacing coming from the <c>Footnote Separator</c> frame style —
    /// so the <em>line</em> costs almost nothing and the spacing is nearly all of this. The value cannot be
    /// measured from a text comparison, which sees no lines, so it is a stated default rather than a
    /// measured one: 0.1 cm above and below, which is what that style ships with.
    /// </remarks>
    public Length NoteSeparatorHeight { get; init; } = Length.FromMm100(200);
}

/// <summary>
/// One of a document's sections, with the furniture that goes round it.
/// </summary>
/// <remarks>
/// A pair rather than two parallel lists, because the two are always looked up together and a mismatch
/// between them would put one section's header on another's pages — which is the sort of error that looks
/// like a header bug and is not.
/// </remarks>
/// <param name="Section">The section's geometry and slot rules.</param>
/// <param name="Furniture">
/// Its headers and footers, or null when it has none. Laid out per slot rather than per page: the same
/// header appears on most pages of a section, and laying it out again for each would shape its text over
/// and over for an answer that cannot change.
/// </param>
public sealed record PaginatedSection(WritingSection Section, PageFurnitureSet? Furniture = null);

/// <summary>
/// Fills pages: lay out a paragraph, put what fits on the page, carry the rest over.
/// </summary>
/// <remarks>
/// <para>
/// The loop is simple and the constraints are not. A paragraph does not always split where it runs out
/// of room — orphan and widow counts forbid leaving one line behind or carrying one line over,
/// keep-together forbids splitting at all, and keep-with-next forbids a paragraph ending a page that
/// its successor does not start. Each of those turns "fill until full" into "fill until full, then
/// reconsider", and the reconsidering is what moves whole paragraphs to the next page.
/// </para>
/// <para>
/// Written as a forward pass with a bounded look-back rather than as a global optimisation, because
/// that is what Writer does and the point is to break where Writer breaks. A total-fit pagination
/// would produce better-balanced pages and different ones.
/// </para>
/// </remarks>
public sealed class Paginator
{
    private readonly PaginationOptions _options;

    /// <summary>
    /// Note heights already worked out, keyed on the note.
    /// </summary>
    /// <remarks>
    /// Cached because narrowing a page's fit asks for the same note's height once per candidate line count,
    /// and laying a note out is a shaping pass. Keyed on the note itself, which is safe because a reader
    /// builds each one once — and cleared per run, since the width they were measured at belongs to the run.
    /// </remarks>
    private readonly Dictionary<PageNote, Length> _noteHeights = [];

    /// <summary>The width the cached note heights were measured at.</summary>
    private Length _noteWidth;

    /// <summary>Creates a paginator.</summary>
    /// <param name="options">The compatibility choices, or null for Writer's.</param>
    public Paginator(PaginationOptions? options = null)
        => _options = options ?? PaginationOptions.Default;

    /// <summary>The options in force.</summary>
    public PaginationOptions Options => _options;

    /// <summary>
    /// True when the last run hit <see cref="PaginationOptions.MaxPages"/> and stopped early.
    /// </summary>
    /// <remarks>
    /// Reported rather than thrown: a truncated document is more useful than none, and a caller that
    /// cares can say so. Silence would be the worst of the three.
    /// </remarks>
    public bool WasTruncated { get; private set; }

    /// <summary>
    /// Paginates a sequence of blocks onto one section's page geometry.
    /// </summary>
    /// <param name="blocks">The paragraphs and tables, in document order.</param>
    /// <param name="section">The section whose geometry the pages use.</param>
    /// <param name="startingNumber">
    /// The number to print on the first page, when the section does not restart numbering itself.
    /// </param>
    /// <param name="furniture">
    /// The section's headers and footers, or null when it has none. Laid out per slot rather than per page:
    /// the same header appears on most pages, and laying it out again for each would shape its text over
    /// and over for an answer that cannot change.
    /// </param>
    public List<LaidOutPage> Paginate(
        IReadOnlyList<PageBlock> blocks,
        WritingSection? section = null,
        int startingNumber = 1,
        PageFurnitureSet? furniture = null)
        => Paginate(blocks, [new PaginatedSection(section ?? new WritingSection(), furniture)], startingNumber);

    /// <summary>
    /// Paginates a sequence of blocks across a document's sections.
    /// </summary>
    /// <param name="blocks">
    /// The paragraphs and tables, in document order, each naming its section through
    /// <see cref="PageBlock.SectionIndex"/>.
    /// </param>
    /// <param name="sections">The sections, in document order, with their furniture.</param>
    /// <param name="startingNumber">The number to print on the first page.</param>
    /// <remarks>
    /// <para>
    /// A section change is a change of everything: the paper size, the margins, the width lines break at,
    /// and the headers. So the blocks are laid out per section rather than once — a paragraph in a landscape
    /// section breaks at the landscape width, and laying it out at the first section's would give the right
    /// page setup with the wrong lines on it.
    /// </para>
    /// <para>
    /// Whether the change costs a page is the document's to say, through
    /// <see cref="WritingSection.Break"/>. Three of the four kinds start a new page and one — continuous —
    /// deliberately does not, which is what a stretch of two-column text in the middle of a page is.
    /// </para>
    /// </remarks>
    public List<LaidOutPage> Paginate(
        IReadOnlyList<PageBlock> blocks,
        IReadOnlyList<PaginatedSection> sections,
        int startingNumber = 1)
    {
        ArgumentNullException.ThrowIfNull(blocks);
        ArgumentNullException.ThrowIfNull(sections);

        WasTruncated = false;
        _noteHeights.Clear();

        List<PaginatedSection> resolved =
            sections.Count > 0 ? [.. sections] : [new PaginatedSection(new WritingSection())];

        int sectionIndex = blocks.Count > 0 ? SectionOf(blocks[0], resolved.Count) : 0;
        WritingSection geometry = resolved[sectionIndex].Section;
        PageFurnitureSet? furnitureSet = resolved[sectionIndex].Furniture;
        PageGeometry page = geometry.Page;
        Length bodyHeight = page.TextHeight;
        Length bodyWidth = page.ColumnWidth;

        // A note breaks at the body's full width rather than a column's, which is what LibreOffice's own
        // rendering shows: the note area spans the text area even when the body above it is in columns.
        _noteWidth = page.TextWidth;

        List<LaidOutPage> pages = [];
        if (blocks.Count == 0 || bodyHeight <= Length.Zero || bodyWidth <= Length.Zero)
        {
            pages.Add(EmptyPage(
                0, startingNumber, page,
                Furniture(furnitureSet, geometry, page, startingNumber, first: true)));
            return pages;
        }

        // Every block is laid out once, at the section's width. Re-laying one out because it moved to
        // another page would give the same answer — the width does not change within a section — and
        // laying out is where the shaping cost is. That matters most for a table: a long one crossing
        // several page breaks would otherwise shape all of its cells once per page it touches.
        List<LaidBlock> laid = new(blocks.Count);
        for (int i = 0; i < blocks.Count; i++)
        {
            // The block's own section's width, not the first section's: a paragraph in a landscape section
            // breaks where landscape says it does.
            Length width = resolved[SectionOf(blocks[i], resolved.Count)].Section.Page.ColumnWidth;
            if (width <= Length.Zero) width = bodyWidth;

            if (blocks[i] is PageTable table)
            {
                (List<PlacedTableCell> cells, List<Length> rowHeights) =
                    TableLayouter.LayOut(table, new DocPoint(Length.Zero, Length.Zero));

                laid.Add(new LaidBlock(null, cells, rowHeights));
                continue;
            }

            PageParagraph paragraph = (PageParagraph)blocks[i];
            ParagraphLayouter layouter = new(paragraph.Face);
            ParagraphFormat? previous = PreviousFormat(blocks, i);

            // A paragraph with runs is measured across them, so each line is as tall as its own tallest
            // run rather than as the paragraph's font. Without runs the single-face path is not merely a
            // shortcut — it is the common case, and it avoids building a prefix table per run for a
            // paragraph that has one.
            laid.Add(new LaidBlock(paragraph.HasRuns
                ? layouter.Layout(
                    Measure(paragraph),
                    paragraph.Format,
                    width,
                    paragraph.Language,
                    previous)
                : layouter.Layout(
                    paragraph.Text,
                    paragraph.Format,
                    paragraph.EmSize,
                    width,
                    paragraph.Language,
                    previous,
                    paragraph.Shaping)));
        }

        int pageNumber = geometry.RestartPageNumberAt ?? startingNumber;
        int sectionFirstPage = 0;
        int column = 0;
        List<PageNote> notes = [];
        List<PlacedLine> placed = [];
        List<PlacedTable> tables = [];
        Length used = Length.Zero;
        int paragraphIndex = 0;
        int lineIndex = 0;

        while (paragraphIndex < blocks.Count)
        {
            if (pages.Count >= _options.MaxPages)
            {
                WasTruncated = true;
                break;
            }

            // "Nothing here yet", which is what the top-of-frame rules are about — and a frame is a column
            // rather than a page once there is more than one of them. A paragraph at the top of the second
            // column drops its leading exactly as one at the top of a page does, because Writer's rule is
            // about the frame the text flows into and not about the sheet.
            bool columnIsEmpty =
                !placed.Any(line => line.Column == column)
                && !tables.Any(part => part.Column == column);

            bool pageIsEmpty = placed.Count == 0 && tables.Count == 0;

            // A section boundary, which is a change of paper size, margins, breaking width and headers all
            // at once. Only at a block boundary: a paragraph already half-placed finishes on the geometry it
            // started on, because its lines were measured at that width and re-breaking them mid-flight
            // would leave the two halves disagreeing about where the words go.
            int blockSection = SectionOf(blocks[paragraphIndex], resolved.Count);
            if (blockSection != sectionIndex && lineIndex == 0)
            {
                if (resolved[blockSection].Section.Break != SectionBreak.Continuous)
                {
                    if (!pageIsEmpty) EmitPage();

                    // An even- or odd-page break leaves a blank page when the parity is already wrong. The
                    // filler belongs to the section that ended, so it is emitted before the geometry
                    // changes — which is also what puts the old section's header on it.
                    SectionBreak kind = resolved[blockSection].Section.Break;
                    while (kind is SectionBreak.EvenPage or SectionBreak.OddPage
                           && pageNumber % 2 == 0 != (kind == SectionBreak.EvenPage)
                           && pages.Count < _options.MaxPages)
                    {
                        EmitPage();
                    }
                }

                sectionIndex = blockSection;
                geometry = resolved[sectionIndex].Section;
                furnitureSet = resolved[sectionIndex].Furniture;
                page = geometry.Page;
                bodyHeight = page.TextHeight;
                sectionFirstPage = pages.Count;
                pageNumber = geometry.RestartPageNumberAt ?? pageNumber;
                continue;
            }

            if (blocks[paragraphIndex] is PageTable table)
            {
                Length before = columnIsEmpty && !_options.KeepsSpacingAtTopOfPage
                    ? Length.Zero
                    : table.SpaceBefore;

                int fittedRows = FittedRows(
                    laid[paragraphIndex].RowHeights, lineIndex, used + before, bodyHeight);

                // Nothing of the table may go here. An empty page would leave the same problem, so a row
                // taller than a whole page is placed anyway and allowed to overflow.
                if (fittedRows == 0)
                {
                    if (!columnIsEmpty)
                    {
                        EmitPage();
                        continue;
                    }

                    fittedRows = 1;
                }

                (PlacedTable part, Length height) = PlaceRows(
                    table, laid[paragraphIndex], lineIndex, fittedRows, page.ColumnArea(column),
                    used + before, column, lineIndex > 0);

                tables.Add(part);
                used += before + height;
                lineIndex = part.RowEnd;

                if (lineIndex < laid[paragraphIndex].RowHeights.Count)
                {
                    // The table is split: the rest goes on the next page, with its headings repeated.
                    EmitPage();
                    continue;
                }

                used += table.SpaceAfter;
                paragraphIndex++;
                lineIndex = 0;
                continue;
            }

            PageParagraph paragraph = (PageParagraph)blocks[paragraphIndex];
            LaidOutParagraph layout = laid[paragraphIndex].Paragraph!;

            // A page break before a paragraph that is not already at the top of a page.
            if (lineIndex == 0 && paragraph.Format.StartsNewPage && !pageIsEmpty)
            {
                EmitPage();
                continue;
            }

            Length spaceAbove = lineIndex == 0
                ? SpaceAbove(blocks, laid, paragraphIndex, atTopOfPage: columnIsEmpty)
                : Length.Zero;

            // The notes those lines would anchor take their room out of the body's, so how many lines fit
            // depends on which notes they cite — and which notes they cite depends on how many fit. Resolved
            // by trying the unconstrained answer and shortening until it holds, which terminates because
            // each step removes a line and so can only remove notes.
            int fitted = Fit(
                layout, lineIndex, used + spaceAbove, bodyHeight - NoteHeight(notes),
                atTopOfPage: columnIsEmpty);

            while (fitted > 0)
            {
                Length room = bodyHeight - NoteHeight(
                    notes, NotesIn(paragraph, layout, lineIndex, fitted));

                if (Fit(layout, lineIndex, used + spaceAbove, room, columnIsEmpty) >= fitted) break;

                fitted--;
            }

            int allowed = Allowed(
                paragraph.Format, layout.Lines.Count, lineIndex, fitted, columnIsEmpty);

            if (allowed <= 0)
            {
                // Nothing of this paragraph may go here. An empty column would leave the same problem, so
                // a paragraph that cannot fit a column of its own is placed anyway and allowed to overflow.
                if (columnIsEmpty)
                {
                    allowed = Math.Max(1, fitted);
                }
                else
                {
                    EmitPage();
                    continue;
                }
            }

            Length top = used + spaceAbove;
            for (int i = 0; i < allowed; i++)
            {
                LineBox box = layout.Lines[lineIndex + i];

                // The first line in a frame loses the leading above its text, box and all: Writer counts
                // that leading as part of the paragraph's upper space and drops it at the top of a frame.
                // Only when it really is the frame's first content — a line below a table is not — and
                // whether the paragraph *began* here is beside the point: a paragraph carried over from the
                // previous page drops the leading above its continuation just the same, which at 200% line
                // spacing is the difference between twenty-five lines on a page and twenty-four.
                if (columnIsEmpty && i == 0) box = box.WithoutSpaceAbove();

                placed.Add(new PlacedLine(paragraphIndex, lineIndex + i, box, top, column));
                top += box.Height;
            }

            notes.AddRange(NotesIn(paragraph, layout, lineIndex, allowed));

            used = top;
            lineIndex += allowed;

            if (lineIndex < layout.Lines.Count)
            {
                // The paragraph is split: the rest goes on the next page.
                EmitPage();
                continue;
            }

            used += layout.SpaceAfter;
            paragraphIndex++;
            lineIndex = 0;

            // Keep-with-next: this paragraph may not end a page its successor does not start. Checked
            // after placing it, because whether the successor fits is only knowable once this one has.
            if (paragraph.Format.KeepWithNext
                && paragraphIndex < blocks.Count
                && laid[paragraphIndex].Paragraph is { } next
                && !FirstLineFits(next, used, bodyHeight))
            {
                MoveTrailingGroupToNextPage(
                    blocks, placed, out List<PlacedLine> moved, out int movedFrom);

                if (moved.Count > 0 && movedFrom > 0)
                {
                    EmitPage();
                    paragraphIndex = movedFrom;
                    continue;
                }
            }
        }

        if (placed.Count > 0 || tables.Count > 0 || pages.Count == 0) EmitPage();

        // The endnotes, which are the one flow that is not part of any page's body: they collect *after* the
        // last of them, on pages of their own. Measured — LibreOffice puts a two-endnote document's notes at
        // the top of a fresh second page, in the body's own text area, and takes nothing off page one.
        pages.AddRange(
            EndnotePages(blocks, resolved[^1], pageNumber, pages.Count));

        return pages;

        // Moves on when the current column is full: to the next column of the same page if there is one,
        // and to a new page otherwise. A local function because it needs every piece of the loop's state —
        // which page geometry is in force, which section's furniture, how far into the section we are, and
        // which column we are filling — and threading that through seven call sites was how the "first page
        // of the section" test came to read `pages.Count == 0` and be wrong for every section but the first.
        void EmitPage()
        {
            if (column + 1 < Math.Max(1, page.Columns))
            {
                // The page is not finished, only this column of it. The lines already placed stay where
                // they are — each carries its own column — and the running height starts again at the top.
                column++;
                used = Length.Zero;
                return;
            }

            pages.Add(Page(
                pages.Count,
                pageNumber,
                page,
                placed,
                tables,
                Furniture(
                    furnitureSet, geometry, page, pageNumber,
                    first: pages.Count == sectionFirstPage),
                NoteArea(notes, page)));

            pageNumber++;
            column = 0;
            placed = [];
            tables = [];
            notes = [];
            used = Length.Zero;
        }
    }

    /// <summary>
    /// Which section a block belongs to, clamped to the sections that exist.
    /// </summary>
    /// <remarks>
    /// Clamped rather than trusted, because the index comes from a reader and a document can name a section
    /// it does not define — a DOCX whose last paragraph carries a <c>w:sectPr</c> the body's own does not
    /// match, for instance. An out-of-range index lands on the last section, which is the one a document's
    /// trailing content belongs to anyway.
    /// </remarks>
    private static int SectionOf(PageBlock block, int sections)
        => Math.Clamp(block.SectionIndex, 0, Math.Max(0, sections - 1));

    /// <summary>
    /// Shapes a paragraph's runs, ready for measuring across them.
    /// </summary>
    /// <remarks>
    /// The paragraph's own face and size close any gap the runs leave, so a paragraph whose runs do not
    /// cover all of its text still measures every character — a document that formats its text and leaves
    /// its paragraph mark unmentioned is normal rather than malformed.
    /// </remarks>
    private static MeasuredParagraph Measure(PageParagraph paragraph)
    {
        List<FormattedRun> runs = [.. paragraph.Runs.Select(run => run.ToFormattedRun())];

        if (runs.Count == 0)
        {
            runs.Add(new FormattedRun(
                0, paragraph.Text.Length, paragraph.Face, paragraph.EmSize, paragraph.Shaping));
        }

        return MeasuredParagraph.Measure(paragraph.Text, runs);
    }

    /// <summary>
    /// How many of a paragraph's remaining lines fit in what is left of the page.
    /// </summary>
    /// <remarks>
    /// A line fits when its whole box does. Allowing a line whose box overruns the margin by a hair
    /// would put a descender in the bottom margin and, worse, would let a page hold one more line than
    /// Writer gives it — which moves every subsequent break.
    /// </remarks>
    /// <summary>
    /// How many of a paragraph's remaining lines fit in what is left of the page.
    /// </summary>
    /// <remarks>
    /// The first line on the page is measured as it will be drawn — without the leading above its text,
    /// which Writer drops at the top of a frame. Measuring it with the leading gives the page one line
    /// fewer than Writer allows at every spacing above single, and a page one line short moves every
    /// break after it.
    /// </remarks>
    private static int Fit(
        LaidOutParagraph layout, int from, Length used, Length available, bool atTopOfPage)
    {
        Length room = available - used;
        int count = 0;

        for (int i = from; i < layout.Lines.Count; i++)
        {
            LineBox box = atTopOfPage && count == 0
                ? layout.Lines[i].WithoutSpaceAbove()
                : layout.Lines[i];

            if (box.Height > room) break;

            room -= box.Height;
            count++;
        }

        return count;
    }

    /// <summary>
    /// How many lines the keep constraints permit, given how many fit.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Zero means "none of it here", which sends the paragraph to the next page whole. The three
    /// constraints are separate questions and all have to be satisfied:
    /// </para>
    /// <list type="bullet">
    ///   <item>
    ///     Keep-together forbids splitting at all, so either the whole remainder fits or none of it does.
    ///   </item>
    ///   <item>
    ///     The orphan count is how many lines must stay behind at the foot of the page. Fewer than that
    ///     is not a legal split, so the paragraph moves rather than leaving one line stranded.
    ///   </item>
    ///   <item>
    ///     The widow count is how many must be carried over. A split that would send a single line to
    ///     the next page instead sends more, by keeping fewer here.
    ///   </item>
    /// </list>
    /// <para>
    /// A paragraph already at the top of a page is exempt from all three: it has nowhere better to go,
    /// and refusing to place it would loop forever on an empty page.
    /// </para>
    /// </remarks>
    private static int Allowed(
        ParagraphFormat format, int totalLines, int from, int fitted, bool atTopOfPage)
    {
        int remaining = totalLines - from;
        if (fitted >= remaining) return remaining;
        if (atTopOfPage) return Math.Max(fitted, 1);
        if (format.KeepTogether) return 0;

        int orphans = Math.Max(format.OrphanLines, 0);
        int widows = Math.Max(format.WidowLines, 0);

        // Only the paragraph's first split is an orphan question: once part of it is already on an
        // earlier page, the lines here are a continuation rather than a stranded beginning.
        if (from == 0 && orphans > 0 && fitted < orphans) return 0;

        if (widows > 0 && remaining - fitted < widows)
        {
            int kept = remaining - widows;
            return kept >= Math.Max(orphans, 1) || (from > 0 && kept >= 1) ? kept : 0;
        }

        return fitted;
    }

    private static bool FirstLineFits(LaidOutParagraph layout, Length used, Length available)
        => layout.Lines.Count == 0
           || used + layout.SpaceBefore + layout.Lines[0].Height <= available;

    /// <summary>
    /// The space above a paragraph, once collapsing and the top-of-page rule have applied.
    /// </summary>
    /// <remarks>
    /// Both behaviours are compatibility flags rather than properties of the paragraph, which is why
    /// they live on the paginator: the same document laid out as Word would and as Writer would differs
    /// here on every paragraph boundary and at the top of every page.
    /// </remarks>
    private Length SpaceAbove(
        IReadOnlyList<PageBlock> blocks,
        List<LaidBlock> laid,
        int index,
        bool atTopOfPage)
    {
        Length before = laid[index].Paragraph!.SpaceBefore;

        if (atTopOfPage && !_options.KeepsSpacingAtTopOfPage) return Length.Zero;
        if (index == 0 || !_options.CollapsesSpacing) return before;

        // Collapsing: the gap is the larger of the two rather than their sum. The previous paragraph's
        // space-after has already been added to the running height, so what is added here is only the
        // part of space-before that exceeds it. A table before this paragraph collapses nothing, because
        // its own space-after is a table property rather than a paragraph's and the formats do not
        // collapse the two against each other.
        if (blocks[index - 1] is not PageParagraph previous) return before;

        Length excess = before - previous.Format.SpaceAfter;
        return excess > Length.Zero ? excess : Length.Zero;
    }

    /// <summary>
    /// Takes the trailing run of keep-with-next paragraphs off a page.
    /// </summary>
    /// <remarks>
    /// A chain, not one paragraph: three headings in a row each keeping with the next all have to move
    /// together, or the group is broken at a different place instead of not at all. The run stops at the
    /// first paragraph that does not keep with its successor, and at the start of the page — a page
    /// whose every paragraph keeps with the next cannot be emptied, so nothing moves and the group is
    /// broken after all, which is what Writer does rather than looping.
    /// </remarks>
    private static void MoveTrailingGroupToNextPage(
        IReadOnlyList<PageBlock> blocks,
        List<PlacedLine> placed,
        out List<PlacedLine> moved,
        out int movedFrom)
    {
        moved = [];
        movedFrom = -1;

        if (placed.Count == 0) return;

        int firstOnPage = placed[0].ParagraphIndex;
        int last = placed[^1].ParagraphIndex;

        // Walk back over the chain of paragraphs that each keep with the next. A table ends the chain:
        // keep-with-next is a paragraph property, and a paragraph cannot be kept with a table it does not
        // know about.
        int first = last;
        while (first > firstOnPage
               && blocks[first - 1] is PageParagraph previous
               && previous.Format.KeepWithNext)
        {
            first--;
        }

        // A paragraph that started on an earlier page cannot be moved, and neither can the whole page.
        if (first <= firstOnPage) return;
        if (placed.Any(line => line.ParagraphIndex == first && !line.StartsParagraph)) return;

        int at = placed.FindIndex(line => line.ParagraphIndex == first);
        if (at <= 0) return;

        moved = [.. placed[at..]];
        placed.RemoveRange(at, placed.Count - at);
        movedFrom = first;
    }

    private static LaidOutPage Page(
        int index,
        int number,
        PageGeometry geometry,
        List<PlacedLine> lines,
        List<PlacedTable> tables,
        (PlacedFlow? Header, PlacedFlow? Footer) furniture,
        PlacedFlow? notes)
        => new()
        {
            Index = index,
            Number = number,
            Size = geometry.Size,
            BodyArea = geometry.TextArea,
            ColumnCount = geometry.Columns,
            ColumnGap = geometry.ColumnGap,
            Lines = [.. lines],
            Tables = [.. tables],
            Header = furniture.Header,
            Footer = furniture.Footer,
            Notes = notes,
        };

    private static LaidOutPage EmptyPage(
        int index,
        int number,
        PageGeometry geometry,
        (PlacedFlow? Header, PlacedFlow? Footer) furniture)
        => new()
        {
            Index = index,
            Number = number,
            Size = geometry.Size,
            BodyArea = geometry.TextArea,
            ColumnCount = geometry.Columns,
            ColumnGap = geometry.ColumnGap,
            Lines = [],
            Header = furniture.Header,
            Footer = furniture.Footer,
        };

    /// <summary>
    /// The header and footer a page takes, laid out into their areas.
    /// </summary>
    /// <remarks>
    /// The slot rules are the section's — a first page takes the first-page slot only if the section asked
    /// for one, an even page the even slot, and everything else falls back to the default — so they are
    /// asked of <see cref="PageFurnitureSlots"/> rather than restated here. The result is cached per slot
    /// inside the set, because most pages of a document share one header and laying it out again per page
    /// would shape the same text for the same answer.
    /// </remarks>
    private static (PlacedFlow? Header, PlacedFlow? Footer) Furniture(
        PageFurnitureSet? furniture,
        WritingSection section,
        PageGeometry geometry,
        int pageNumber,
        bool first)
        => furniture is null
            ? (null, null)
            : (furniture.Header(section, geometry, pageNumber, first),
               furniture.Footer(section, geometry, pageNumber, first));
    /// <summary>
    /// The notes a run of a paragraph's lines anchors.
    /// </summary>
    /// <remarks>
    /// By character offset, since that is what a note anchor is: the note belongs to whichever line contains
    /// the position its anchor occupies. A paragraph with no notes — nearly all of them — returns an empty
    /// sequence without touching its lines.
    /// </remarks>
    /// <summary>
    /// The pages an endnote flow occupies, which follow the body's rather than sitting inside them.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A footnote and an endnote are the same thing to read and different things to place: a footnote takes
    /// its room out of the page that cites it, which is what makes it a feedback loop, and an endnote takes
    /// none at all — it collects at the end of the document. Measured, on a two-endnote document: LibreOffice
    /// leaves page one holding every body paragraph, with no note area, and puts both notes at the top of a
    /// fresh page two in the body's own text area.
    /// </para>
    /// <para>
    /// So this is an ordinary pagination of an ordinary flow, done by recursion rather than by a second
    /// implementation — the notes get page breaks, headers and footers exactly as body text does, because
    /// they are body text on those pages. The last section's geometry is what they take, being what the
    /// document ends in.
    /// </para>
    /// </remarks>
    /// <param name="blocks">The body's blocks, whose paragraphs are searched for endnote anchors.</param>
    /// <param name="section">The section whose geometry and furniture the endnote pages take.</param>
    /// <param name="startingNumber">The number the first endnote page prints.</param>
    /// <param name="alreadyEmitted">How many pages the body used, which bounds how many are left.</param>
    private List<LaidOutPage> EndnotePages(
        IReadOnlyList<PageBlock> blocks,
        PaginatedSection section,
        int startingNumber,
        int alreadyEmitted)
    {
        List<PageBlock> flow = [];
        Collect(blocks, depth: 0);

        if (flow.Count == 0 || alreadyEmitted >= _options.MaxPages) return [];

        // A fresh paginator rather than a recursive call on this one, so that the run's own state — the note
        // height cache, the truncation flag — is not overwritten half way through reporting it. Its page
        // budget is what this run has left, so a document cannot buy extra pages by ending in endnotes.
        Paginator notes = new(_options with { MaxPages = _options.MaxPages - alreadyEmitted });
        List<LaidOutPage> paginated = notes.Paginate(
            flow, [section with { Section = section.Section with { Break = SectionBreak.NextPage } }],
            startingNumber);

        if (notes.WasTruncated) WasTruncated = true;

        // Each page carries the flow it was laid out from, because its line indexes count in that list and
        // not in the body's. Without this an endnote page draws the body's first paragraphs at the endnotes'
        // line lengths — which looks like a layout bug and is an indexing one.
        return [.. paginated.Select(
            (endnotePage, at) => endnotePage with
            {
                Index = alreadyEmitted + at,
                Blocks = flow,
            })];

        // Depth-first through tables, because a cell can cite an endnote and its notes still collect with
        // the rest. Endnote bodies are *not* searched: a note inside a note would collect after itself.
        void Collect(IReadOnlyList<PageBlock> from, int depth)
        {
            if (depth > FlowLayouter.MaxNesting) return;

            foreach (PageBlock block in from)
            {
                switch (block)
                {
                    case PageParagraph paragraph:
                        foreach (PageNote note in paragraph.Notes)
                        {
                            if (note.IsEndnote) flow.AddRange(note.Blocks);
                        }

                        break;

                    case PageTable table:
                        foreach (PageTableRow row in table.Rows)
                        {
                            foreach (PageTableCell cell in row.Cells) Collect(cell.Blocks, depth + 1);
                        }

                        break;
                }
            }
        }
    }

    private static IEnumerable<PageNote> NotesIn(
        PageParagraph paragraph, LaidOutParagraph layout, int from, int count)
    {
        if (paragraph.Notes.Count == 0 || count <= 0) yield break;

        int end = Math.Min(from + count, layout.Lines.Count);
        if (from >= end) yield break;

        int first = layout.Lines[from].Line.Start;
        int last = layout.Lines[end - 1].Line.End;

        foreach (PageNote note in paragraph.Notes)
        {
            // Endnotes collect at the end of the document rather than the foot of a page, so they take no
            // room here — recorded as a gap rather than placed wrongly.
            if (note.IsEndnote) continue;
            if (note.Offset >= first && note.Offset < last) yield return note;
        }
    }

    /// <summary>
    /// How tall the note area is for a set of notes, separator included.
    /// </summary>
    /// <remarks>
    /// Cached per note, because the same note's height is asked for once per candidate line count while the
    /// fit is being narrowed — and laying a note out is a shaping pass. The cache is keyed on the note
    /// itself, which is safe because a note is a record the reader built once.
    /// </remarks>
    private Length NoteHeight(List<PageNote> placed, IEnumerable<PageNote>? extra = null)
    {
        Length total = Length.Zero;
        int count = 0;

        foreach (PageNote note in extra is null ? placed : placed.Concat(extra))
        {
            total += HeightOfNote(note);
            count++;
        }

        return count == 0 ? Length.Zero : total + _options.NoteSeparatorHeight;
    }

    private Length HeightOfNote(PageNote note)
    {
        if (_noteHeights.TryGetValue(note, out Length cached)) return cached;

        Length height = FlowLayouter.HeightOf(note.Blocks, _noteWidth);
        _noteHeights[note] = height;
        return height;
    }

    /// <summary>
    /// The note area, laid out bottom-aligned in the body's own rectangle.
    /// </summary>
    /// <remarks>
    /// Bottom-aligned and inside the body area, both measured rather than assumed: LibreOffice's own
    /// rendering puts the last note line's box bottom on the body area's bottom edge. So this is the same
    /// call a Word footer makes — a flow with no stated offset — and the notes take their room out of the
    /// body's, which is what makes a page with notes hold less text.
    /// </remarks>
    private static PlacedFlow? NoteArea(List<PageNote> notes, PageGeometry page)
    {
        if (notes.Count == 0) return null;

        List<PageBlock> blocks = [];
        foreach (PageNote note in notes) blocks.AddRange(note.Blocks);

        return FlowLayouter.LayOut(blocks, page.TextArea, offsetFromTop: null);
    }

    /// <summary>
    /// The format of the nearest preceding paragraph, for the contextual-spacing comparison.
    /// </summary>
    /// <remarks>
    /// Nearest <em>paragraph</em>, not nearest block: contextual spacing suppresses the gap between two
    /// paragraphs of one style, and a table between them is not one of those. Null when there is no
    /// preceding paragraph at all, which the layouter reads as "this is the first".
    /// </remarks>
    private static ParagraphFormat? PreviousFormat(IReadOnlyList<PageBlock> blocks, int index)
        => index > 0 && blocks[index - 1] is PageParagraph previous ? previous.Format : null;

    /// <summary>
    /// How many of a table's remaining rows fit in what is left of the page.
    /// </summary>
    /// <remarks>
    /// A row fits when its whole height does, the same rule a line follows — a row split across a page
    /// break is a thing Word can do and Writer cannot, and Writer is what this matches. So a row taller
    /// than the space left moves whole, and one taller than a page overflows rather than being cut.
    /// </remarks>
    private static int FittedRows(
        List<Length> rowHeights, int from, Length used, Length available)
    {
        Length room = available - used;
        int count = 0;

        for (int row = from; row < rowHeights.Count; row++)
        {
            if (rowHeights[row] > room) break;

            room -= rowHeights[row];
            count++;
        }

        return count;
    }

    /// <summary>
    /// Puts a run of a table's rows on the page, repeating its headings when it is a continuation.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The cells were laid out once, relative to the table's own top-left, so placing them is a shift
    /// rather than a re-layout — which is the point of doing the work up front. The shift differs for the
    /// repeated headings, because they come from the top of the table and are being drawn part way down
    /// it, so they are offset separately from the rows that follow.
    /// </para>
    /// <para>
    /// A continuation whose headings would leave no room for a single ordinary row still gets them: the
    /// alternative is a page holding a heading and nothing else followed by another just like it, which
    /// does not terminate.
    /// </para>
    /// </remarks>
    /// <param name="table">The table.</param>
    /// <param name="laid">Its cells and row heights, relative to its own top-left.</param>
    /// <param name="from">The first row to place.</param>
    /// <param name="count">How many rows to place.</param>
    /// <param name="body">
    /// The column the table goes in, which the cells' coordinates end up relative to — the whole body area
    /// for single-column text, and one column of it otherwise.
    /// </param>
    /// <param name="top">How far below that area's top the placed part starts.</param>
    /// <param name="column">Which column of the page it is, recorded on the result.</param>
    /// <param name="repeatHeadings">True when this is a continuation and the headings come again.</param>
    private static (PlacedTable Table, Length Height) PlaceRows(
        PageTable table,
        LaidBlock laid,
        int from,
        int count,
        DocRect body,
        Length top,
        int column,
        bool repeatHeadings)
    {
        List<Length> heights = laid.RowHeights;
        int end = Math.Min(from + count, heights.Count);

        int headings = repeatHeadings
            ? Math.Min(Math.Max(table.HeaderRowCount, 0), from)
            : 0;

        Length headingHeight = Length.Zero;
        for (int row = 0; row < headings; row++) headingHeight += heights[row];

        Length placedHeight = headingHeight;
        for (int row = from; row < end; row++) placedHeight += heights[row];

        Length skipped = Length.Zero;
        for (int row = 0; row < from; row++) skipped += heights[row];

        List<PlacedTableCell> cells = [];

        // The headings first, moved from the top of the table to the top of this part.
        if (headings > 0)
        {
            cells.AddRange(TableLayouter.Offset(
                laid.Cells.Where(cell => cell.Row < headings),
                body.X,
                body.Y + top));
        }

        // Then the rows themselves, moved up by everything above them and down by where this part starts.
        cells.AddRange(TableLayouter.Offset(
            laid.Cells.Where(cell => cell.Row >= from && cell.Row < end),
            body.X,
            body.Y + top + headingHeight - skipped));

        return (
            new PlacedTable
            {
                Table = table,
                Area = new DocRect(
                    body.X + table.LeftIndent, body.Y + top, table.Width, placedHeight),
                Cells = cells,
                FirstRow = from,
                RowEnd = end,
                Column = column,
            },
            placedHeight);
    }

    /// <summary>
    /// One block after its content has been laid out, whichever kind of block it is.
    /// </summary>
    /// <remarks>
    /// A discriminated pair rather than two parallel lists, because the paginator walks the blocks by
    /// index and two lists with holes in different places would be one off-by-one away from placing a
    /// table's rows as a paragraph's lines.
    /// </remarks>
    /// <param name="Paragraph">The laid-out paragraph, or null when the block is a table.</param>
    /// <param name="Cells">A table's cells relative to its own top-left, empty for a paragraph.</param>
    /// <param name="RowHeights">A table's row heights, empty for a paragraph.</param>
    private readonly record struct LaidBlock(
        LaidOutParagraph? Paragraph,
        List<PlacedTableCell> Cells,
        List<Length> RowHeights)
    {
        /// <summary>Creates the paragraph case.</summary>
        public LaidBlock(LaidOutParagraph paragraph) : this(paragraph, [], []) { }
    }
}
