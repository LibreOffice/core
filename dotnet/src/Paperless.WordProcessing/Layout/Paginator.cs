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
    /// <para>
    /// Writer's <c>PARA_SPACE_MAX_AT_PAGES</c>, and — this is the part that took a measurement — the flag
    /// does not mean "always keeps it". It decides whether the question is asked at all;
    /// <c>SwFlowFrame::HasParaSpaceAtPages</c> (<c>flowfrm.cxx</c>:1415) then decides where. In the
    /// document body it grants the space only on the <em>first</em> page and after an explicit break, and
    /// takes it away at every automatic one; outside the body — a header, a footer, a table cell — it
    /// grants it everywhere.
    /// </para>
    /// <para>
    /// Measured rather than read, on a DOCX whose paragraphs carry 20 pt of space-before and nothing else:
    /// LibreOffice puts page one's first line at 92.03 pt, which is the 72 pt margin plus the 20, and page
    /// two's at 72.03 pt. Keeping it on every page put every page after the first 20 pt low and eventually
    /// cost a page break.
    /// </para>
    /// <para>
    /// The synthetic that establishes this needs a <c>word/settings.xml</c>, even an empty one. Without
    /// that part LibreOffice never applies its OOXML compatibility defaults and the document lays out with
    /// Writer's, which reverses both this rule and <see cref="CollapsesSpacing"/> — a minimal test file
    /// missing a part it does not appear to need will otherwise answer the wrong question convincingly.
    /// </para>
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
    /// Whether a justified line ended by a manual break is stretched to the margin.
    /// </summary>
    /// <remarks>
    /// True everywhere except in a DOCX carrying <c>w:doNotExpandShiftReturn</c>, which is how a
    /// file asks for Word's pre-2000 behaviour: a line ended by a shift-return is left ragged,
    /// as a paragraph's last line is, rather than having its two words pushed to opposite
    /// margins. LibreOffice reads the same flag into <c>DoNotJustifyLinesWithManualBreak</c>.
    /// It changes only the drawing, never where a line breaks.
    /// </remarks>
    public bool JustifiesLinesEndedByBreak { get; init; } = true;

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

    /// <summary>
    /// How wide the rule above the notes is, as a fraction of the text width.
    /// </summary>
    /// <remarks>
    /// A quarter, which is what Writer's <c>Footnote Separator</c> frame style ships with — and measured
    /// rather than taken on trust: LibreOffice's PDF export draws the rule from 56.7 to 177.15 pt on an A4
    /// page with 2 cm margins, and 120.45 of 481.89 is exactly 25%.
    /// </remarks>
    public double NoteSeparatorWidth { get; init; } = 0.25;

    /// <summary>How thick the rule is; half a point, again measured from the path the export writes.</summary>
    public Length NoteSeparatorThickness { get; init; } = Length.FromPoints(0.5);

    /// <summary>
    /// The gap between the rule and the first note line.
    /// </summary>
    /// <remarks>
    /// 0.1 cm, the lower half of <see cref="NoteSeparatorHeight"/>'s two spacings. Kept separately because the
    /// reservation is a total and this is a position: the rule sits this far above the notes, not half way up
    /// the space reserved for it.
    /// </remarks>
    public Length NoteSeparatorSpacing { get; init; } = Length.FromMm100(100);
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

    /// <summary>
    /// What each block's lines must flow around, or null on the first pass and for documents with no
    /// floating frames — which is nearly all of them.
    /// </summary>
    /// <remarks>
    /// A field rather than a parameter because it has to reach the block-laying loop through the same
    /// entry point pagination already uses, and because it is deliberately transient: it holds one pass's
    /// answer about where the frames were, and is cleared before the result is returned.
    /// </remarks>
    private Func<int, ILineObstacles?>? _obstacles;

    /// <summary>Creates a paginator.</summary>
    /// <param name="options">The compatibility choices, or null for Writer's.</param>
    public Paginator(PaginationOptions? options = null)
        => _options = options ?? PaginationOptions.Default;

    /// <summary>The options in force.</summary>
    public PaginationOptions Options => _options;

    /// <summary>
    /// The blocks the last run really paginated, or null when they were the ones it was handed.
    /// </summary>
    /// <remarks>
    /// Non-null only where a per-page note restart rewrote a citation, which changes the text of the citing
    /// paragraph and of the note's own first line. The pages index into <em>these</em> blocks, so a caller
    /// keeping the list it passed in would draw the numbering the document was read with rather than the one
    /// its pages settled on. Each page also names the list directly through <see cref="LaidOutPage.Blocks"/>,
    /// so a caller that ignores this still draws the right thing; this is what lets the block list a caller
    /// holds agree with them.
    /// </remarks>
    public IReadOnlyList<PageBlock>? Blocks { get; private set; }

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

        Blocks = null;

        List<PaginatedSection> withFrames =
            sections.Count > 0 ? [.. sections] : [new PaginatedSection(new WritingSection())];

        List<LaidOutPage> pages = Fill(blocks, withFrames, startingNumber);

        // A per-page note restart, which is the one numbering rule that cannot be settled before the pages
        // exist — and Writer damps it rather than iterating, so this renumbers over the finished pages, lays
        // them out once more and stops. See `NoteRenumbering` for the citations and for why stopping is the
        // answer rather than a compromise. Guarded, so a document whose notes do not restart pays one walk.
        if (NoteRenumbering.Applies(blocks)
            && NoteRenumbering.Apply(blocks, pages) is { } renumbered)
        {
            blocks = renumbered;
            Blocks = renumbered;
            pages = Fill(blocks, withFrames, startingNumber);

            // Each page now has to name the list its lines index, because the caller's is the one it was
            // handed and this one is not it. An endnote page already carries its own flow and keeps it.
            for (int i = 0; i < pages.Count; i++)
            {
                if (pages[i].Blocks is null) pages[i] = pages[i] with { Blocks = blocks };
            }
        }

        // The loop frames close, and the reason pagination cannot be a single pass once one is present:
        // where a frame goes depends on where its anchor paragraph ended up, and where that paragraph's
        // lines end up depends on the hole the frame makes in them. Writer resolves the same circularity
        // by formatting the anchored objects and the text they affect in turn until neither moves
        // (`SwObjectFormatter`, `sw/source/core/layout/objectformatter.cxx`); this does the coarser thing
        // of laying the whole document out again, which converges in one further pass whenever the frames
        // stay on the page they started on — the case every real document is.
        //
        // "Has this document a frame at all" is asked of the finished pages rather than of the blocks,
        // because a frame need not be anchored in the body: one anchored in a table cell or a header is
        // reached through the flow it landed in, which exists only once a page has been filled. Scanning
        // the blocks instead returned early on exactly those documents and left their frames unplaced.
        FrameResolution resolution = FrameResolution.Of(
            blocks, withFrames, pages, _options.CollapsesSpacing);
        if (resolution.IsEmpty) return pages;

        for (int pass = 0; pass < MaxFramePasses; pass++)
        {
            _obstacles = resolution.ObstaclesFor;
            List<LaidOutPage> next;
            try
            {
                next = Fill(blocks, withFrames, startingNumber);
            }
            finally
            {
                _obstacles = null;
            }

            FrameResolution settled = FrameResolution.Of(
                blocks, withFrames, next, _options.CollapsesSpacing);
            pages = next;

            bool converged = settled.SameAs(resolution);
            resolution = settled;
            if (converged) break;
        }

        return resolution.AttachedTo(pages);
    }

    /// <summary>
    /// How many times the document may be laid out again to settle its frames' positions.
    /// </summary>
    /// <remarks>
    /// Four, which is a bound rather than a count: a document whose frames stay on their own page settles
    /// on the second pass, and the rest are for the case where wrapping pushes an anchor onto the next
    /// page and so moves the frame with it. Writer bounds its own object-formatting loop for the same
    /// reason — a frame can chase its anchor indefinitely.
    /// </remarks>
    private const int MaxFramePasses = 4;

    /// <summary>The pagination loop itself, with whatever frames the current pass believes in.</summary>
    private List<LaidOutPage> Fill(
        IReadOnlyList<PageBlock> blocks,
        List<PaginatedSection> sections,
        int startingNumber)
    {
        WasTruncated = false;
        _noteHeights.Clear();

        List<PaginatedSection> resolved =
            sections.Count > 0 ? [.. sections] : [new PaginatedSection(new WritingSection())];

        int sectionIndex = blocks.Count > 0 ? SectionOf(blocks[0], resolved.Count) : 0;
        WritingSection geometry = resolved[sectionIndex].Section;
        PageFurnitureSet? furnitureSet = resolved[sectionIndex].Furniture;
        PageGeometry page = geometry.Page;

        // The geometry the body actually gets, which is the section's own once the running head has been
        // measured against the room reserved for it — see PushedDownBy. Recomputed at the top of every
        // page, because the head a page draws is the page's and not the section's.
        PageGeometry body = page;
        Length bodyHeight = body.TextHeight;
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
                // The section's own breaking width is also what a table stating no widths of its own is
                // fitted to. A table that declares its grid is laid out exactly as it was before.
                (List<PlacedTableCell> cells, List<Length> rowHeights) =
                    TableLayouter.LayOut(
                        table,
                        new DocPoint(Length.Zero, Length.Zero),
                        0,
                        width,
                        _options.CollapsesSpacing);

                laid.Add(new LaidBlock(null, cells, rowHeights));
                continue;
            }

            PageParagraph paragraph = (PageParagraph)blocks[i];
            ParagraphLayouter layouter = new(paragraph.Face, breaker: null, paragraph.Metrics);
            ParagraphFormat? previous = PreviousFormat(blocks, i);

            // A paragraph with runs is measured across them, so each line is as tall as its own tallest
            // run rather than as the paragraph's font. Without runs the single-face path is not merely a
            // shortcut — it is the common case, and it avoids building a prefix table per run for a
            // paragraph that has one. An inline picture takes the same road as runs do, because it is the
            // prefix table that carries the room it takes on its line. So does a list label bigger than
            // the text it labels, for the same reason: the label is a portion in the first line and only
            // the per-line path can make one line taller than the rest.
            ILineObstacles? obstacles = _obstacles?.Invoke(i);

            LaidOutParagraph laidOut =
                paragraph.HasRuns || paragraph.HasInlineObjects || paragraph.LabelRaisesFirstLine
                ? layouter.Layout(
                    paragraph.Measure(),
                    paragraph.Format,
                    width,
                    paragraph.Language,
                    previous,
                    obstacles)
                : layouter.Layout(
                    paragraph.Text,
                    paragraph.Format,
                    paragraph.EmSize,
                    width,
                    paragraph.Language,
                    previous,
                    paragraph.Shaping,
                    obstacles);

            laid.Add(new LaidBlock(
                _options.JustifiesLinesEndedByBreak
                    ? laidOut
                    : ManualBreakJustification.Suppress(laidOut, paragraph.Text)));
        }

        int pageNumber = geometry.RestartPageNumberAt ?? startingNumber;
        int sectionFirstPage = 0;
        int column = 0;

        // The furniture the page being built will draw, which is the section its *first* content belongs
        // to rather than the section it happens to end in. A continuous section break puts two sections on
        // one sheet, and a sheet has one running head: Word gives it to the section the page starts in, and
        // so does LibreOffice — its rendering of a document whose first section is a title page and whose
        // second begins part way down it leaves page one bare and puts the new head on page two. Taking the
        // current section instead put the second section's head on the title page.
        PageFurnitureSet? pageFurniture = furnitureSet;
        WritingSection pageFurnitureSection = geometry;
        PageGeometry pageFurnitureGeometry = page;
        bool pageIsSectionFirst = true;

        // The geometry a continuous section is waiting for a fresh sheet to claim, or null when nothing is
        // waiting. A continuous break cannot re-cut the sheet it lands on — see the break handling below —
        // so its paper and margins are held here until the next page begins.
        PageGeometry? deferredPage = null;

        // Called wherever a page's first content is decided: at the start, after each page is emitted, and
        // at a section break that finds the page still empty.
        void AdoptSection()
        {
            pageFurniture = furnitureSet;
            pageFurnitureSection = geometry;
            pageFurnitureGeometry = page;
            pageIsSectionFirst = pages.Count == sectionFirstPage;
        }

        // The body area of the page about to be filled, which depends on how tall its own running head
        // and foot turned out. Called after every change to what those are or which page draws them.
        void MeasureBody()
        {
            body = PushedDownBy(
                page,
                pageFurniture?.Header(
                    pageFurnitureSection, pageFurnitureGeometry, pageNumber, pageIsSectionFirst,
                    _options.CollapsesSpacing));

            body = PulledUpBy(
                body,
                page,
                pageFurniture?.Footer(
                    pageFurnitureSection, pageFurnitureGeometry, pageNumber, pageIsSectionFirst,
                    _options.CollapsesSpacing));

            bodyHeight = body.TextHeight;
        }

        MeasureBody();

        List<PageNote> notes = [];
        List<PlacedLine> placed = [];
        List<PlacedTable> tables = [];
        Length used = Length.Zero;
        int paragraphIndex = 0;
        int lineIndex = 0;

        // How far into the row at `lineIndex` an earlier page already reached, for a table row that broke
        // across the break. Nought for every row of every table that did not — which, since a row only
        // splits when it has to, is nearly all of them.
        Length rowDrawn = Length.Zero;

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
                SectionBreak kind = resolved[blockSection].Section.Break;

                // A column break only *is* one when the columns line up. Word 2013 and later treat it as a
                // page break whenever they do not — no previous section, fewer than two columns, or a
                // different count than before — and LibreOffice's own importer says so in as many words:
                // "Word 2013+ seems to treat a section column break as a page break all the time"
                // (`dmapper/PropertyMap.cxx`). Its remaining column-break branch it documents as broken
                // (tdf#135343, "completely broken, producing a no-column section that starts on a new page"),
                // so that one case follows Word rather than LibreOffice and cannot be compared against it.
                bool intoTheSameColumns =
                    kind == SectionBreak.NewColumn
                    && resolved[blockSection].Section.Page.Columns > 1
                    && resolved[blockSection].Section.Page.Columns == page.Columns;

                if (kind == SectionBreak.NewColumn && !intoTheSameColumns) kind = SectionBreak.NextPage;

                if (kind != SectionBreak.Continuous)
                {
                    // A column break fills the rest of the *column*, which is what EmitPage does when the
                    // page has another column left. Every other break has to fill the rest of the *page*,
                    // columns and all — so it keeps going until a page is actually emitted, which is what
                    // FinishPage is for. A break on an empty column or an empty page has nothing to fill and
                    // must not skip one.
                    if (intoTheSameColumns)
                    {
                        if (!columnIsEmpty) EmitPage();
                    }
                    else if (!pageIsEmpty)
                    {
                        FinishPage();
                    }

                    // An even- or odd-page break leaves a blank page when the parity is already wrong. The
                    // filler belongs to the section that ended, so it is emitted before the geometry
                    // changes — which is also what puts the old section's header on it.
                    while (kind is SectionBreak.EvenPage or SectionBreak.OddPage
                           && pageNumber % 2 == 0 != (kind == SectionBreak.EvenPage)
                           && pages.Count < _options.MaxPages)
                    {
                        FinishPage();
                    }
                }

                sectionIndex = blockSection;
                geometry = resolved[sectionIndex].Section;
                furnitureSet = resolved[sectionIndex].Furniture;

                // A continuous break shares a sheet with the section above it, and a sheet has one paper
                // size and one set of margins — so the new section's take effect on the *next* page, not
                // part way down this one. The same argument the running head already follows a few lines
                // above, and the same one Writer's model forces: page geometry lives on a page style,
                // `SectionPropertyMap::CloseSectionGroup` gives a continuous section no page style of its
                // own, and `InheritOrFinalizePageStyles` then hands it the previous section's
                // (`sw/source/writerfilter/dmapper/PropertyMap.cxx`:1309-1323, 1722).
                // Measured on `b050-19.docx`, whose one-paragraph first section is half-inch-margined and
                // whose continuous second section is inch-margined: LibreOffice sets page one's text from
                // 36 pt to 574 pt and pages two and three from 72 pt to 539 pt. Switching at the break put
                // page one at 72 pt, which is a tenth of the measure lost on every line of it.
                // Columns are the exception and change at once, because Writer *does* start a text section
                // for them mid-page.
                page = kind == SectionBreak.Continuous && !pageIsEmpty
                    ? page with
                    {
                        Columns = geometry.Page.Columns,
                        ColumnGap = geometry.Page.ColumnGap,
                    }
                    : geometry.Page;

                deferredPage = kind == SectionBreak.Continuous && !pageIsEmpty ? geometry.Page : null;
                sectionFirstPage = pages.Count;
                pageNumber = geometry.RestartPageNumberAt ?? pageNumber;

                // A page with nothing on it yet belongs to the section starting here; one that already
                // carries lines keeps the head of the section it started in.
                if (placed.Count == 0 && tables.Count == 0) AdoptSection();
                MeasureBody();
                continue;
            }

            if (blocks[paragraphIndex] is PageTable table)
            {
                Length before = columnIsEmpty && !_options.KeepsSpacingAtTopOfPage
                    ? Length.Zero
                    : table.SpaceBefore;

                TablePart part = PlaceTablePart(
                    table, laid[paragraphIndex], lineIndex, rowDrawn, body.ColumnArea(column),
                    used + before, column, bodyHeight - (used + before), columnIsEmpty);

                // Nothing of the table may go here, and the column already holds something — so the page
                // ends and the table starts again at the top of the next one.
                if (part.Placed is null)
                {
                    EmitPage();
                    continue;
                }

                tables.Add(part.Placed);
                used += before + part.Height;
                lineIndex = part.NextRow;
                rowDrawn = part.NextDrawn;

                if (lineIndex < laid[paragraphIndex].RowHeights.Count || rowDrawn > Length.Zero)
                {
                    // The table is split: the rest goes on the next page, with its headings repeated.
                    EmitPage();
                    continue;
                }

                used += table.SpaceAfter;
                paragraphIndex++;
                lineIndex = 0;
                rowDrawn = Length.Zero;
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

            // Writer's `HasParaSpaceAtPages`, which is what decides whether the top-of-frame rule applies
            // at all: the document's first page keeps a paragraph's space-before, an explicit page break
            // keeps it, and an automatic break in the body drops it. Only asked where the rule can bite —
            // at the top of a column, where `SpaceAbove` would otherwise take the option's word for it.
            bool keepsSpaceHere =
                column == 0 && (pages.Count == 0 || paragraph.Format.StartsNewPage);

            Length ownSpaceAbove = Length.Zero;
            Length spaceAbove = lineIndex == 0
                ? SpaceAbove(
                    blocks, laid, paragraphIndex, atTopOfPage: columnIsEmpty,
                    keepsSpacingAtTop: keepsSpaceHere, own: out ownSpaceAbove)
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

            allowed = WholeLines(layout.Lines, lineIndex, allowed);

            Length top = used + spaceAbove;
            bool firstLineHere = columnIsEmpty;
            bool firstLineOfParagraph = lineIndex == 0;

            for (int i = 0; i < allowed; i++)
            {
                // A paragraph's first line never carries the leading proportional line spacing adds — it is
                // the paragraph above's to give, and `SpaceAbove` collects it there. Nor does the first line
                // in a frame, whatever line of its paragraph it is, since Writer drops the whole upper space
                // at the top of a text frame. See `ParagraphLeading`.
                // Both flags are per *line* rather than per stretch: a line beside a frame clear of both
                // margins is several boxes on one baseline, and they share one box's worth of geometry, so
                // the leading has to come off whichever of them is the one whose height is counted.
                LineBox box = ParagraphLeading.AsDrawn(
                    layout.Lines[lineIndex + i],
                    isFirstOfParagraph: firstLineOfParagraph,
                    isFirstInFrame: firstLineHere);
                bool shares = box.SharesLineWithNext;

                placed.Add(new PlacedLine(
                    paragraphIndex,
                    lineIndex + i,
                    box,
                    top,
                    column,
                    // The paragraph's own upper space, and only on the line the gap sits above — what a
                    // frame anchored to the paragraph measures its offset from is that line's top less
                    // this. See `PlacedLine.ParagraphTop`.
                    lineIndex + i == 0 ? ownSpaceAbove : Length.Zero));

                // A stretch that shares its line with the next one leaves the pen where it is: the box
                // after it is more of the same line, at the same top.
                if (!shares)
                {
                    top += box.Height;
                    firstLineHere = false;
                    firstLineOfParagraph = false;
                }
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

        // FinishPage rather than EmitPage, because the last page has to be *emitted*: in a multi-column
        // section EmitPage moves to the next column when there is one, so a document ending part way through
        // column one of a two-column section would advance to column two and never write the page at all.
        if (placed.Count > 0 || tables.Count > 0 || pages.Count == 0) FinishPage();

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
        // Ends the whole page rather than the column, which a section break that is not a column break has
        // to do: in a two-column section EmitPage moves to column two, and a page break that stopped there
        // would put the next section beside the last one instead of after it.
        void FinishPage()
        {
            int before = pages.Count;
            while (pages.Count == before && pages.Count < _options.MaxPages) EmitPage();
        }

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

            PlacedFlow? noteArea = NoteArea(notes, body);

            pages.Add(Page(
                pages.Count,
                pageNumber,
                body,
                placed,
                tables,
                Furniture(
                    pageFurniture, pageFurnitureSection, pageFurnitureGeometry, pageNumber,
                    first: pageIsSectionFirst),
                noteArea,
                Separator(noteArea, body)));

            // The sheet just written is the one the outgoing geometry belonged to; the next one is free to
            // be cut the way the section that is now current asks for.
            if (deferredPage is { } waiting)
            {
                page = waiting;
                deferredPage = null;
            }

            AdoptSection();
            pageNumber++;
            column = 0;
            placed = [];
            tables = [];
            notes = [];
            used = Length.Zero;
            MeasureBody();
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
    /// Every line is measured as it will be drawn, which for the two that lose their leading — a
    /// paragraph's first and a frame's first — means without it. Measuring one with the leading gives the
    /// page one line fewer than Writer allows at every spacing above single, and a page one line short
    /// moves every break after it.
    /// </remarks>
    private static int Fit(
        LaidOutParagraph layout, int from, Length used, Length available, bool atTopOfPage)
    {
        Length room = available - used;
        int count = 0;

        for (int i = from; i < layout.Lines.Count;)
        {
            // A line beside a frame clear of both margins is several boxes on one baseline and costs its
            // height once, counted from its last stretch — the one whose height and leading are the line's.
            // Taken whole, so a page can never end between the text left of a frame and the text right of it.
            int last = LastStretch(layout.Lines, i);

            LineBox box = ParagraphLeading.AsDrawn(
                layout.Lines[last],
                isFirstOfParagraph: i == 0,
                isFirstInFrame: atTopOfPage && count == 0);

            if (box.Height > room) break;

            room -= box.Height;
            count += last - i + 1;
            i = last + 1;
        }

        return count;
    }

    /// <summary>Where the line that begins at <paramref name="from"/> ends, as a box index.</summary>
    /// <remarks>
    /// Its own index for every box of a document with no floating frame in it, so the walk costs one
    /// comparison per line and changes nothing about how such a document paginates.
    /// </remarks>
    private static int LastStretch(IReadOnlyList<LineBox> lines, int from)
    {
        int last = from;
        while (last + 1 < lines.Count && lines[last].SharesLineWithNext) last++;
        return last;
    }

    /// <summary>
    /// The same count of boxes, shortened so that it does not end in the middle of a line.
    /// </summary>
    /// <remarks>
    /// The keep rules count boxes, and a line beside a frame can be more than one of them — so an orphan
    /// or widow limit, or the "place it anyway" fallback of a single box, can land between the text left
    /// of a frame and the text right of it. Splitting there would draw half a line on each of two pages
    /// and leave the second half at the wrong baseline, so the count backs off to the line's start; if
    /// that leaves nothing, the whole line goes rather than none of it, since a box that cannot be placed
    /// anywhere would loop.
    /// </remarks>
    private static int WholeLines(IReadOnlyList<LineBox> lines, int from, int count)
    {
        if (count <= 0 || from + count >= lines.Count) return count;
        if (!lines[from + count - 1].SharesLineWithNext) return count;

        int end = from + count - 1;
        while (end > from && lines[end - 1].SharesLineWithNext) end--;

        return end > from ? end - from : LastStretch(lines, from) - from + 1;
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
           || used + layout.SpaceBefore + layout.Lines[0].WithoutSpaceAbove().Height <= available;

    /// <summary>
    /// The space above a paragraph, once collapsing and the top-of-page rule have applied.
    /// </summary>
    /// <remarks>
    /// Both behaviours are compatibility flags rather than properties of the paragraph, which is why
    /// they live on the paginator: the same document laid out as Word would and as Writer would differs
    /// here on every paragraph boundary and at the top of every page.
    /// <para>
    /// The third term is neither, and is not the paragraph's either: the leading proportional line
    /// spacing adds above a first line belongs to the paragraph <em>above</em>, which is what
    /// <c>SwFlowFrame::CalcUpperSpace</c> adds as <c>nPrevLineSpacing</c> in both of its branches. See
    /// <see cref="ParagraphLeading"/> for the citations and for what it costs to get wrong.
    /// </para>
    /// </remarks>
    /// <param name="blocks">The document's blocks, for the paragraph above this one.</param>
    /// <param name="laid">Their laid-out forms, for the spacings the layout resolved.</param>
    /// <param name="index">Which block the space is being measured above.</param>
    /// <param name="atTopOfPage">True when nothing is on the column yet.</param>
    /// <param name="keepsSpacingAtTop">
    /// Whether <em>this</em> frame top is one of the places the option's rule applies — Writer's
    /// <c>SwFlowFrame::HasParaSpaceAtPages</c>. See <see cref="PaginationOptions.KeepsSpacingAtTopOfPage"/>:
    /// the flag says the document has the behaviour at all, and this says whether the paragraph in hand is
    /// somewhere it is granted.
    /// </param>
    private Length SpaceAbove(
        IReadOnlyList<PageBlock> blocks,
        List<LaidBlock> laid,
        int index,
        bool atTopOfPage,
        bool keepsSpacingAtTop = true)
        => SpaceAbove(blocks, laid, index, atTopOfPage, keepsSpacingAtTop, out _);

    /// <summary>
    /// The same, reporting separately how much of the gap the paragraph itself contributed.
    /// </summary>
    /// <param name="blocks">The document's blocks, for the paragraph above this one.</param>
    /// <param name="laid">Their laid-out forms, for the spacings the layout resolved.</param>
    /// <param name="index">Which block the space is being measured above.</param>
    /// <param name="atTopOfPage">True when nothing is on the column yet.</param>
    /// <param name="keepsSpacingAtTop">As the overload's.</param>
    /// <param name="own">
    /// The gap less the paragraph above's leading — everything Writer's <c>GetTopForObjPos</c> leaves out
    /// of a paragraph-anchored frame's origin. It adds back <c>nPrevLowerSpace + nPrevLineSpacing</c>
    /// (<c>SwFlowFrame::GetUpperSpaceAmountConsideredForPrevFrame</c>,
    /// <c>sw/source/core/layout/flowfrm.cxx:1835</c>), and each branch below has already netted the
    /// previous paragraph's lower space off <c>before</c>, so what remains to exclude is the leading.
    /// </param>
    private Length SpaceAbove(
        IReadOnlyList<PageBlock> blocks,
        List<LaidBlock> laid,
        int index,
        bool atTopOfPage,
        bool keepsSpacingAtTop,
        out Length own)
    {
        Length total = Gap(blocks, laid, index, atTopOfPage, keepsSpacingAtTop, out Length leading);
        own = total - leading;
        return total;
    }

    /// <summary>The gap above a paragraph, and how much of it is the paragraph above's leading.</summary>
    private Length Gap(
        IReadOnlyList<PageBlock> blocks,
        List<LaidBlock> laid,
        int index,
        bool atTopOfPage,
        bool keepsSpacingAtTop,
        out Length leading)
    {
        leading = Length.Zero;

        Length before = laid[index].Paragraph!.SpaceBefore;

        if (atTopOfPage && !(_options.KeepsSpacingAtTopOfPage && keepsSpacingAtTop))
        {
            return Length.Zero;
        }

        // The previous paragraph's leading, and only when there is a previous paragraph in this frame:
        // at the top of a page or a column Writer finds no previous frame at all
        // (`GetPrevFrameForUpperSpaceCalc_`) and never reaches the line-spacing term, so a page that
        // keeps its paragraph spacing still starts its first line hard against the margin. A table above
        // hands nothing down either — `GetSpacingValuesOfFrame` reports a line spacing only for a text
        // frame.
        leading = atTopOfPage || index == 0 || blocks[index - 1] is not PageParagraph
            ? Length.Zero
            : ParagraphLeading.Below(laid[index - 1].Paragraph);

        if (index == 0 || blocks[index - 1] is not PageParagraph previous) return before + leading;

        // Contextual spacing suppresses the gap entirely, which means taking back the space-after already
        // added for the paragraph above: `before` is zero here, and leaving its space-after standing would
        // keep the whole gap on a list whose style states an after and no before.
        if (blocks[index] is PageParagraph current
            && ParagraphLayouter.SharesContextualSpacing(previous.Format, current.Format))
        {
            // Only when the paragraph above is on this page: its space-after was added to the running
            // height there and has to come back off here, and at the top of a page there is nothing to
            // take off — subtracting anyway would put the first line above the margin.
            return atTopOfPage ? Length.Zero : leading - previous.Format.SpaceAfter;
        }

        if (!_options.CollapsesSpacing) return before + leading;

        // Collapsing: the gap is the larger of the two rather than their sum. The previous paragraph's
        // space-after has already been added to the running height, so what is added here is only the
        // part of space-before that exceeds it. A table before this paragraph collapses nothing, because
        // its own space-after is a table property rather than a paragraph's and the formats do not
        // collapse the two against each other.
        Length excess = before - previous.Format.SpaceAfter;
        return (excess > Length.Zero ? excess : Length.Zero) + leading;
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
        PlacedFlow? notes,
        DocRect? separator = null)
        => new()
        {
            Index = index,
            Number = number,
            Size = geometry.Size,
            BodyArea = geometry.TextArea,
            ColumnCount = geometry.Columns,
            ColumnGap = geometry.ColumnGap,
            IsRightToLeft = geometry.IsRightToLeft,
            Lines = [.. lines],
            Tables = [.. tables],
            Header = furniture.Header,
            Footer = furniture.Footer,
            Notes = notes,
            NoteSeparator = separator,
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
            IsRightToLeft = geometry.IsRightToLeft,
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
    private (PlacedFlow? Header, PlacedFlow? Footer) Furniture(
        PageFurnitureSet? furniture,
        WritingSection section,
        PageGeometry geometry,
        int pageNumber,
        bool first)
        => furniture is null
            ? (null, null)
            : (furniture.Header(section, geometry, pageNumber, first, _options.CollapsesSpacing),
               furniture.Footer(section, geometry, pageNumber, first, _options.CollapsesSpacing));

    /// <summary>
    /// A page's geometry with the body moved down to clear a running head that outgrew its margin.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A Word header is not confined to the room the top margin reserves for it. <c>w:header</c> says where
    /// the header starts and <c>w:top</c> says where the body does, and when the header's own content needs
    /// more than the difference between them the body is pushed down rather than drawn over.
    /// </para>
    /// <para>
    /// LibreOffice reaches the same answer through two properties its DOCX importer sets together:
    /// <c>SectionPropertyMap::PrepareHeaderFooterProperties</c> (<c>dmapper/PropertyMap.cxx</c>:1148) makes
    /// the page's top margin <c>w:header</c>, gives the header frame a height of
    /// <c>w:top − w:header</c> with a 1 mm floor, and turns on both dynamic height and dynamic spacing.
    /// <c>SwHeadFootFrame::FormatPrt</c> (<c>sw/source/core/layout/hffrm.cxx</c>:116) is what dynamic
    /// spacing then means: growth first eats the gap between the header and the body — so a header that
    /// still fits inside <c>w:top</c> moves nothing — and once that gap is gone the frame keeps growing and
    /// the body follows it down. The body therefore starts at
    /// <c>max(w:top, w:header + header height)</c>, which is what this computes.
    /// </para>
    /// <para>
    /// Per page rather than per section, because the height belongs to the head that page actually draws:
    /// a section whose first page carries a tall title block and whose later pages carry one line has two
    /// different body areas, and taking the tallest of its slots shortens every page for the sake of one.
    /// </para>
    /// <para>
    /// The height taken is the flow's <see cref="PlacedFlow.Advance"/>, its last paragraph's space-after
    /// included. Writer's <c>lcl_CalcContentHeight</c> sums frame heights instead, and a text frame's
    /// height excludes its own lower spacing — so a running head whose last paragraph has space after it
    /// is measured a little tall here. Measured on the corpus document above the difference is 1.2 pt the
    /// other way, which is within the ascent of the first body line, so the simpler figure is what this
    /// uses until a document is found that needs the harder one.
    /// </para>
    /// </remarks>
    private static PageGeometry PushedDownBy(PageGeometry page, PlacedFlow? header)
    {
        if (header is null || header.IsEmpty) return page;

        Length needed = page.HeaderDistance + header.Advance;
        if (needed <= page.Margins.Top) return page;

        // A head that would leave no body at all is not honoured. Writer would let the frame keep growing,
        // but a body of no height holds one overflowing line per page however much text is left, so a
        // malformed running head would turn a ten-page document into a thousand-page one.
        if (needed >= page.Size.Height - page.Margins.Bottom) return page;

        return page with { Margins = page.Margins with { Top = needed } };
    }

    /// <summary>
    /// A page's geometry with the body's foot raised to clear a running foot that outgrew its margin.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The mirror of <see cref="PushedDownBy"/>, and the same mechanism read the other way up.
    /// <c>SectionPropertyMap::PrepareHeaderFooterProperties</c> handles the two symmetrically
    /// (<c>dmapper/PropertyMap.cxx</c>:1171): a section with a footer gets a bottom margin of
    /// <c>w:footer</c>, a footer frame of <c>w:bottom − w:footer</c> with the same 1 mm floor, and the
    /// same dynamic height and dynamic spacing. <c>SwHeadFootFrame::FormatPrt</c> then treats a footer
    /// exactly as it treats a header, only growing upwards — so the body's last line can sit no lower
    /// than <c>pageHeight − w:footer − footer height</c>, and a footer that still fits inside
    /// <c>w:bottom</c> moves nothing.
    /// </para>
    /// <para>
    /// Two corpus shapes make this bind. The obvious one is a footer of several paragraphs in a small
    /// bottom margin. The other is a document whose <c>w:footer</c> is <em>larger</em> than its
    /// <c>w:bottom</c> — legal, common in the corpus, and meaning the footer overlaps the space
    /// <c>w:bottom</c> reserves. Ignoring it left the body several points taller than Writer gives it on
    /// every page of such a document, which is how a long one loses a page.
    /// </para>
    /// <para>
    /// Taken from the page's own margins rather than from the header-adjusted geometry, since the two
    /// ends are independent: <paramref name="stated"/> supplies the bottom margin and the sheet height,
    /// and <paramref name="body"/> carries whatever the head already did to the top.
    /// </para>
    /// </remarks>
    /// <param name="body">The geometry as the running head left it.</param>
    /// <param name="stated">The section's own geometry, for the margin the document asked for.</param>
    /// <param name="footer">The running foot this page draws, or null when it has none.</param>
    private static PageGeometry PulledUpBy(PageGeometry body, PageGeometry stated, PlacedFlow? footer)
    {
        if (footer is null || footer.IsEmpty) return body;

        Length needed = stated.FooterDistance + footer.Advance;
        if (needed <= stated.Margins.Bottom) return body;

        // As with a running head, a foot that would leave no body at all is not honoured: a body of no
        // height holds one overflowing line per page however much text is left.
        if (needed >= stated.Size.Height - body.Margins.Top) return body;

        return body with { Margins = body.Margins with { Bottom = needed } };
    }

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
                            if (note.Placement == NotePlacement.DocumentEnd) flow.AddRange(note.Blocks);
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
            // By where the note goes rather than by what class it is: an endnote whose document asks for its
            // endnotes at the end of each section is placed at the foot of the page, exactly as a footnote is.
            if (note.Placement == NotePlacement.DocumentEnd) continue;
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

        Length height = FlowLayouter.HeightOf(
            note.Blocks, _noteWidth, collapsesSpacing: _options.CollapsesSpacing);
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
    private PlacedFlow? NoteArea(List<PageNote> notes, PageGeometry page)
    {
        if (notes.Count == 0) return null;

        List<PageBlock> blocks = [];
        foreach (PageNote note in notes) blocks.AddRange(note.Blocks);

        return FlowLayouter.LayOut(
            blocks, page.TextArea, offsetFromTop: null,
            collapsesSpacing: _options.CollapsesSpacing);
    }

    /// <summary>
    /// The rule above a page's notes, or null when the page has none.
    /// </summary>
    /// <remarks>
    /// Positioned from where the notes actually landed rather than from the room reserved for them, which is
    /// the only way to get it right: the reservation is a total for the rule and both its spacings, and the
    /// notes are bottom-aligned inside the text area — so the top of the note flow is a measured position and
    /// the height of the reservation is not.
    /// </remarks>
    /// <param name="notes">The notes placed on the page, laid out.</param>
    /// <param name="page">The page's geometry, for the text area the rule is a fraction of.</param>
    private DocRect? Separator(PlacedFlow? notes, PageGeometry page)
    {
        if (notes is null || notes.Lines.Count == 0) return null;

        Length width = page.TextWidth * _options.NoteSeparatorWidth;
        if (width <= Length.Zero) return null;

        // The first note line's *box* top, which is where the flow put it — not its baseline.
        Length top = notes.Area.Y + notes.Lines[0].Top
            - _options.NoteSeparatorSpacing
            - _options.NoteSeparatorThickness;

        return new DocRect(
            page.TextArea.X, top, width, _options.NoteSeparatorThickness);
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
    /// One page's worth of a table: what was placed, how tall it is, and where the next page resumes.
    /// </summary>
    /// <param name="Placed">The cells that landed here, or null when nothing could.</param>
    /// <param name="Height">How much of the page's height they took.</param>
    /// <param name="NextRow">The row the next page starts at.</param>
    /// <param name="NextDrawn">
    /// How far into <paramref name="NextRow"/> this page already reached, which is nought unless the row
    /// itself was broken.
    /// </param>
    private readonly record struct TablePart(
        PlacedTable? Placed, Length Height, int NextRow, Length NextDrawn);

    /// <summary>
    /// Puts as much of a table as fits on the page, repeating its headings when it is a continuation.
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
    /// <para>
    /// Three things can go on the page, in this order: the rest of a row the previous page broke, the run
    /// of whole rows that then fits, and the first part of the row that does not. The last is Writer's
    /// follow flow line and the reason this is not simply "how many rows fit" — see
    /// <see cref="TableLayouter.SliceRow"/>.
    /// </para>
    /// </remarks>
    /// <param name="table">The table.</param>
    /// <param name="laid">Its cells and row heights, relative to its own top-left.</param>
    /// <param name="from">The first row to place.</param>
    /// <param name="drawn">How far into that row an earlier page already reached; nought for a fresh row.</param>
    /// <param name="body">
    /// The column the table goes in, which the cells' coordinates end up relative to — the whole body area
    /// for single-column text, and one column of it otherwise.
    /// </param>
    /// <param name="top">How far below that area's top the placed part starts.</param>
    /// <param name="column">Which column of the page it is, recorded on the result.</param>
    /// <param name="room">How much of the column's height is left.</param>
    /// <param name="columnIsEmpty">
    /// True when nothing else is on the column yet, which is what makes overflowing the right answer: a
    /// row too tall for a column of its own has nowhere better to go, and moving it would not terminate.
    /// </param>
    private static TablePart PlaceTablePart(
        PageTable table,
        LaidBlock laid,
        int from,
        Length drawn,
        DocRect body,
        Length top,
        int column,
        Length room,
        bool columnIsEmpty)
    {
        List<Length> heights = laid.RowHeights;
        List<PlacedTableCell> cells = [];

        // The headings, moved from the top of the table to the top of this part. Only on a continuation:
        // on the table's own first part they are the rows about to be placed.
        int headings = Math.Min(Math.Max(table.HeaderRowCount, 0), from);
        Length placed = Length.Zero;

        for (int row = 0; row < headings; row++) placed += heights[row];

        if (headings > 0)
        {
            cells.AddRange(TableLayouter.Offset(
                laid.Cells.Where(cell => cell.Row < headings), body.X, body.Y + top));
        }

        int start = from;

        // The rest of a row the previous page broke. It cannot be moved — its first part is already drawn —
        // so a remainder too tall for the page is placed anyway and broken again further down.
        if (drawn > Length.Zero && from < heights.Count)
        {
            List<PlacedTableCell> rowCells = RowCells(laid, from);

            TableLayouter.RowSlice? tail =
                TableLayouter.SliceRow(table.Rows[from], rowCells, drawn, room - placed)
                ?? TableLayouter.SliceRow(table.Rows[from], rowCells, drawn, Length.FromEmu(long.MaxValue));

            // A remainder with nothing in it, which the cut said there was: the row is finished rather
            // than unfinished. Asking again is what would not terminate.
            if (tail is { } rest)
            {
                cells.AddRange(TableLayouter.Offset(rest.Cells, body.X, body.Y + top + placed));
                placed += rest.Height;

                if (!rest.IsComplete)
                {
                    return new TablePart(
                        Part(table, cells, body, top, column, placed, from, from + 1),
                        placed, from, rest.Cut);
                }
            }

            start = from + 1;
        }

        // Then the run of whole rows that fits in what is left.
        int end = start;
        Length whole = Length.Zero;
        while (end < heights.Count && placed + whole + heights[end] <= room)
        {
            whole += heights[end];
            end++;
        }

        if (end > start)
        {
            Length skipped = Length.Zero;
            for (int row = 0; row < start; row++) skipped += heights[row];

            cells.AddRange(TableLayouter.Offset(
                laid.Cells.Where(cell => cell.Row >= start && cell.Row < end),
                body.X,
                body.Y + top + placed - skipped));

            placed += whole;
        }

        // Finally the first part of the row that does not fit, when the document lets it break.
        if (end < heights.Count && MaySplit(table, end) && !IsCoveredByAMerge(laid, end))
        {
            List<PlacedTableCell> rowCells = RowCells(laid, end);

            if (TableLayouter.SliceRow(table.Rows[end], rowCells, Length.Zero, room - placed)
                is { } head)
            {
                cells.AddRange(TableLayouter.Offset(head.Cells, body.X, body.Y + top + placed));
                placed += head.Height;

                return new TablePart(
                    Part(table, cells, body, top, column, placed, from, end + 1),
                    placed, end, head.Cut);
            }
        }

        // Nothing at all fitted. An empty column would leave the same problem, so the first row is placed
        // anyway and allowed to overflow; otherwise the caller ends the page and tries again at its top.
        if (end == from && drawn <= Length.Zero)
        {
            if (!columnIsEmpty || from >= heights.Count) return new TablePart(null, Length.Zero, from, drawn);

            Length skipped = Length.Zero;
            for (int row = 0; row < from; row++) skipped += heights[row];

            cells.AddRange(TableLayouter.Offset(
                RowCells(laid, from), body.X, body.Y + top + placed - skipped));

            placed += heights[from];
            end = from + 1;
        }

        return new TablePart(
            Part(table, cells, body, top, column, placed, from, end), placed, end, Length.Zero);
    }

    /// <summary>The cells of one row, as the table's own layout left them.</summary>
    private static List<PlacedTableCell> RowCells(LaidBlock laid, int row)
        => [.. laid.Cells.Where(cell => cell.Row == row)];

    /// <summary>
    /// Whether a cell starting further up the table reaches into this row.
    /// </summary>
    /// <remarks>
    /// Such a row cannot be broken here: the merged cell is one rectangle drawn with the row it starts in,
    /// and a break inside it would leave half of it on a page it was never placed on. Writer keeps the
    /// same case out of its own split by re-formatting the line that a row span crosses
    /// (<c>lcl_AdjustRowSpanCells</c>); declining is the same answer with none of the machinery.
    /// </remarks>
    private static bool IsCoveredByAMerge(LaidBlock laid, int row)
    {
        foreach (PlacedTableCell cell in laid.Cells)
        {
            if (cell.Row < row && cell.Row + Math.Max(1, cell.Cell.RowSpan) > row) return true;
        }

        return false;
    }

    /// <summary>
    /// Whether a row's own content may be broken across a page.
    /// </summary>
    /// <remarks>
    /// The document's say first — <see cref="PageTableRow.CanSplit"/> — and then the two rules Writer
    /// applies whatever the document says: a repeated heading never splits, since it is drawn again on the
    /// next page anyway, and neither does a row of a stated exact height, which is a fixed size rather than
    /// a floor. Both are the first two tests in <c>SwRowFrame::IsRowSplitAllowed</c>.
    /// </remarks>
    private static bool MaySplit(PageTable table, int row)
        => row >= Math.Max(table.HeaderRowCount, 0)
           && table.Rows[row].CanSplit
           && !table.Rows[row].HasExactHeight;

    private static PlacedTable Part(
        PageTable table,
        List<PlacedTableCell> cells,
        DocRect body,
        Length top,
        int column,
        Length height,
        int firstRow,
        int rowEnd)
        => new()
        {
            Table = table,
            Area = new DocRect(
                body.X + table.LeftWithin(body.Width),
                body.Y + top,
                table.WidthWithin(body.Width),
                height),
            Cells = cells,
            FirstRow = firstRow,
            RowEnd = rowEnd,
            Column = column,
        };

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
