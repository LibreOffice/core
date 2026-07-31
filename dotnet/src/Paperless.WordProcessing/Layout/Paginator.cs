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
    /// Paginates a sequence of paragraphs onto one section's page geometry.
    /// </summary>
    /// <param name="paragraphs">The paragraphs, in document order.</param>
    /// <param name="section">The section whose geometry the pages use.</param>
    /// <param name="startingNumber">
    /// The number to print on the first page, when the section does not restart numbering itself.
    /// </param>
    public List<LaidOutPage> Paginate(
        IReadOnlyList<PageParagraph> paragraphs,
        WritingSection? section = null,
        int startingNumber = 1)
    {
        ArgumentNullException.ThrowIfNull(paragraphs);

        WasTruncated = false;

        WritingSection geometry = section ?? new WritingSection();
        PageGeometry page = geometry.Page;
        Length bodyHeight = page.TextHeight;
        Length bodyWidth = page.ColumnWidth;

        List<LaidOutPage> pages = [];
        if (paragraphs.Count == 0 || bodyHeight <= Length.Zero || bodyWidth <= Length.Zero)
        {
            pages.Add(EmptyPage(0, startingNumber, page));
            return pages;
        }

        // Every paragraph is laid out once, at the section's width. Re-laying one out because it moved
        // to another page would give the same answer — the width does not change within a section — and
        // laying out is where the shaping cost is.
        List<LaidOutParagraph> laid = new(paragraphs.Count);
        for (int i = 0; i < paragraphs.Count; i++)
        {
            PageParagraph paragraph = paragraphs[i];
            ParagraphLayouter layouter = new(paragraph.Face);

            laid.Add(layouter.Layout(
                paragraph.Text,
                paragraph.Format,
                paragraph.EmSize,
                bodyWidth,
                paragraph.Language,
                i > 0 ? paragraphs[i - 1].Format : null,
                paragraph.Shaping));
        }

        int pageNumber = geometry.RestartPageNumberAt ?? startingNumber;
        List<PlacedLine> placed = [];
        Length used = Length.Zero;
        int paragraphIndex = 0;
        int lineIndex = 0;

        while (paragraphIndex < paragraphs.Count)
        {
            if (pages.Count >= _options.MaxPages)
            {
                WasTruncated = true;
                break;
            }

            PageParagraph paragraph = paragraphs[paragraphIndex];
            LaidOutParagraph layout = laid[paragraphIndex];

            // A page break before a paragraph that is not already at the top of a page.
            if (lineIndex == 0 && paragraph.Format.StartsNewPage && placed.Count > 0)
            {
                pages.Add(Page(pages.Count, pageNumber++, page, placed));
                placed = [];
                used = Length.Zero;
                continue;
            }

            Length spaceAbove = lineIndex == 0
                ? SpaceAbove(paragraphs, laid, paragraphIndex, atTopOfPage: placed.Count == 0)
                : Length.Zero;

            int fitted = Fit(
                layout, lineIndex, used + spaceAbove, bodyHeight, atTopOfPage: placed.Count == 0);
            int allowed = Allowed(
                paragraph.Format, layout.Lines.Count, lineIndex, fitted, placed.Count == 0);

            if (allowed <= 0)
            {
                // Nothing of this paragraph may go here. An empty page would leave the same problem, so
                // a paragraph that cannot fit a page of its own is placed anyway and allowed to overflow.
                if (placed.Count == 0)
                {
                    allowed = Math.Max(1, fitted);
                }
                else
                {
                    pages.Add(Page(pages.Count, pageNumber++, page, placed));
                    placed = [];
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
                if (placed.Count == 0) box = box.WithoutSpaceAbove();

                placed.Add(new PlacedLine(paragraphIndex, lineIndex + i, box, top));
                top += box.Height;
            }

            used = top;
            lineIndex += allowed;

            if (lineIndex < layout.Lines.Count)
            {
                // The paragraph is split: the rest goes on the next page.
                pages.Add(Page(pages.Count, pageNumber++, page, placed));
                placed = [];
                used = Length.Zero;
                continue;
            }

            used += layout.SpaceAfter;
            paragraphIndex++;
            lineIndex = 0;

            // Keep-with-next: this paragraph may not end a page its successor does not start. Checked
            // after placing it, because whether the successor fits is only knowable once this one has.
            if (paragraph.Format.KeepWithNext
                && paragraphIndex < paragraphs.Count
                && !FirstLineFits(laid[paragraphIndex], used, bodyHeight))
            {
                MoveTrailingGroupToNextPage(
                    paragraphs, placed, out List<PlacedLine> moved, out int movedFrom);

                if (moved.Count > 0 && movedFrom > 0)
                {
                    pages.Add(Page(pages.Count, pageNumber++, page, placed));
                    placed = [];
                    used = Length.Zero;
                    paragraphIndex = movedFrom;
                    continue;
                }
            }
        }

        if (placed.Count > 0 || pages.Count == 0)
        {
            pages.Add(Page(pages.Count, pageNumber, page, placed));
        }

        return pages;
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
        IReadOnlyList<PageParagraph> paragraphs,
        List<LaidOutParagraph> laid,
        int index,
        bool atTopOfPage)
    {
        Length before = laid[index].SpaceBefore;

        if (atTopOfPage && !_options.KeepsSpacingAtTopOfPage) return Length.Zero;
        if (index == 0 || !_options.CollapsesSpacing) return before;

        // Collapsing: the gap is the larger of the two rather than their sum. The previous paragraph's
        // space-after has already been added to the running height, so what is added here is only the
        // part of space-before that exceeds it.
        Length after = paragraphs[index - 1].Format.SpaceAfter;
        Length excess = before - after;
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
        IReadOnlyList<PageParagraph> paragraphs,
        List<PlacedLine> placed,
        out List<PlacedLine> moved,
        out int movedFrom)
    {
        moved = [];
        movedFrom = -1;

        if (placed.Count == 0) return;

        int firstOnPage = placed[0].ParagraphIndex;
        int last = placed[^1].ParagraphIndex;

        // Walk back over the chain of paragraphs that each keep with the next.
        int first = last;
        while (first > firstOnPage && paragraphs[first - 1].Format.KeepWithNext) first--;

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
        int index, int number, PageGeometry geometry, List<PlacedLine> lines)
        => new()
        {
            Index = index,
            Number = number,
            Size = geometry.Size,
            BodyArea = geometry.TextArea,
            Lines = [.. lines],
        };

    private static LaidOutPage EmptyPage(int index, int number, PageGeometry geometry)
        => new()
        {
            Index = index,
            Number = number,
            Size = geometry.Size,
            BodyArea = geometry.TextArea,
            Lines = [],
        };
}
