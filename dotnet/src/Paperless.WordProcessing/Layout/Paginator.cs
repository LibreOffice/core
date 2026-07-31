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
}

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
    {
        ArgumentNullException.ThrowIfNull(blocks);

        WasTruncated = false;

        WritingSection geometry = section ?? new WritingSection();
        PageGeometry page = geometry.Page;
        Length bodyHeight = page.TextHeight;
        Length bodyWidth = page.ColumnWidth;

        List<LaidOutPage> pages = [];
        if (blocks.Count == 0 || bodyHeight <= Length.Zero || bodyWidth <= Length.Zero)
        {
            pages.Add(EmptyPage(0, startingNumber, page, Furniture(furniture, geometry, page, startingNumber, first: true)));
            return pages;
        }

        // Every block is laid out once, at the section's width. Re-laying one out because it moved to
        // another page would give the same answer — the width does not change within a section — and
        // laying out is where the shaping cost is. That matters most for a table: a long one crossing
        // several page breaks would otherwise shape all of its cells once per page it touches.
        List<LaidBlock> laid = new(blocks.Count);
        for (int i = 0; i < blocks.Count; i++)
        {
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
                    bodyWidth,
                    paragraph.Language,
                    previous)
                : layouter.Layout(
                    paragraph.Text,
                    paragraph.Format,
                    paragraph.EmSize,
                    bodyWidth,
                    paragraph.Language,
                    previous,
                    paragraph.Shaping)));
        }

        int pageNumber = geometry.RestartPageNumberAt ?? startingNumber;
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

            bool pageIsEmpty = placed.Count == 0 && tables.Count == 0;

            if (blocks[paragraphIndex] is PageTable table)
            {
                Length before = pageIsEmpty && !_options.KeepsSpacingAtTopOfPage
                    ? Length.Zero
                    : table.SpaceBefore;

                int fittedRows = FittedRows(
                    laid[paragraphIndex].RowHeights, lineIndex, used + before, bodyHeight);

                // Nothing of the table may go here. An empty page would leave the same problem, so a row
                // taller than a whole page is placed anyway and allowed to overflow.
                if (fittedRows == 0)
                {
                    if (!pageIsEmpty)
                    {
                        pages.Add(Page(
                            pages.Count, pageNumber++, page, placed, tables,
                            Furniture(furniture, geometry, page, pageNumber - 1, pages.Count == 0)));
                        placed = [];
                        tables = [];
                        used = Length.Zero;
                        continue;
                    }

                    fittedRows = 1;
                }

                (PlacedTable part, Length height) = PlaceRows(
                    table, laid[paragraphIndex], lineIndex, fittedRows, page.TextArea,
                    used + before, repeatHeadings: lineIndex > 0);

                tables.Add(part);
                used += before + height;
                lineIndex = part.RowEnd;

                if (lineIndex < laid[paragraphIndex].RowHeights.Count)
                {
                    // The table is split: the rest goes on the next page, with its headings repeated.
                    pages.Add(Page(
                        pages.Count, pageNumber++, page, placed, tables,
                        Furniture(furniture, geometry, page, pageNumber - 1, pages.Count == 0)));
                    placed = [];
                    tables = [];
                    used = Length.Zero;
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
                pages.Add(Page(
                    pages.Count,
                    pageNumber++,
                    page,
                    placed,
                    tables,
                    Furniture(furniture, geometry, page, pageNumber - 1, pages.Count == 0)));
                placed = [];
                tables = [];
                used = Length.Zero;
                continue;
            }

            Length spaceAbove = lineIndex == 0
                ? SpaceAbove(blocks, laid, paragraphIndex, atTopOfPage: pageIsEmpty)
                : Length.Zero;

            int fitted = Fit(
                layout, lineIndex, used + spaceAbove, bodyHeight, atTopOfPage: pageIsEmpty);
            int allowed = Allowed(
                paragraph.Format, layout.Lines.Count, lineIndex, fitted, pageIsEmpty);

            if (allowed <= 0)
            {
                // Nothing of this paragraph may go here. An empty page would leave the same problem, so
                // a paragraph that cannot fit a page of its own is placed anyway and allowed to overflow.
                if (pageIsEmpty)
                {
                    allowed = Math.Max(1, fitted);
                }
                else
                {
                    pages.Add(Page(
                        pages.Count,
                        pageNumber++,
                        page,
                        placed,
                        tables,
                        Furniture(furniture, geometry, page, pageNumber - 1, pages.Count == 0)));
                    placed = [];
                    tables = [];
                    used = Length.Zero;
                    continue;
                }
            }

            Length top = used + spaceAbove;
            for (int i = 0; i < allowed; i++)
            {
                LineBox box = layout.Lines[lineIndex + i];

                // The first line on a page loses the leading above its text, box and all: Writer counts
                // that leading as part of the paragraph's upper space and drops it at the top of a frame.
                // Only when it really is the page's first content — a line below a table is not.
                if (pageIsEmpty && placed.Count == 0) box = box.WithoutSpaceAbove();

                placed.Add(new PlacedLine(paragraphIndex, lineIndex + i, box, top));
                top += box.Height;
            }

            used = top;
            lineIndex += allowed;

            if (lineIndex < layout.Lines.Count)
            {
                // The paragraph is split: the rest goes on the next page.
                pages.Add(Page(
                    pages.Count,
                    pageNumber++,
                    page,
                    placed,
                    tables,
                    Furniture(furniture, geometry, page, pageNumber - 1, pages.Count == 0)));
                placed = [];
                tables = [];
                used = Length.Zero;
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
                    pages.Add(Page(
                        pages.Count,
                        pageNumber++,
                        page,
                        placed,
                        tables,
                        Furniture(furniture, geometry, page, pageNumber - 1, pages.Count == 0)));
                    placed = [];
                    tables = [];
                    used = Length.Zero;
                    paragraphIndex = movedFrom;
                    continue;
                }
            }
        }

        if (placed.Count > 0 || tables.Count > 0 || pages.Count == 0)
        {
            pages.Add(Page(
                pages.Count,
                pageNumber,
                page,
                placed,
                tables,
                Furniture(furniture, geometry, page, pageNumber, pages.Count == 0)));
        }

        return pages;
    }

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
        (PlacedFlow? Header, PlacedFlow? Footer) furniture)
        => new()
        {
            Index = index,
            Number = number,
            Size = geometry.Size,
            BodyArea = geometry.TextArea,
            Lines = [.. lines],
            Tables = [.. tables],
            Header = furniture.Header,
            Footer = furniture.Footer,
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
    /// <param name="body">The page's body area, which the cells' coordinates end up relative to.</param>
    /// <param name="top">How far below the body's top the placed part starts.</param>
    /// <param name="repeatHeadings">True when this is a continuation and the headings come again.</param>
    private static (PlacedTable Table, Length Height) PlaceRows(
        PageTable table,
        LaidBlock laid,
        int from,
        int count,
        DocRect body,
        Length top,
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
