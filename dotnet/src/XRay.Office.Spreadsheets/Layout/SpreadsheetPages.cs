using XRay.Office.Core.Threading;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Extraction;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;

namespace XRay.Office.Spreadsheets.Layout;

/// <summary>
/// One printed page of a workbook: which sheet it came from, and which cells are on it.
/// </summary>
public sealed class SheetPage : IPage, IPageRenderLimits
{
    private readonly SheetPageDrawing? _drawing;
    private readonly SheetNotePageDrawing? _notes;
    private readonly long _maxImagePixels = OfficeOperation.MaxImagePixels;
    private readonly OversizedImagePolicy _oversizedImages = OfficeOperation.OversizedImages;

    internal SheetPage(int index, int number, SheetLayout sheet, SheetPagePlacement placement)
    {
        Index = index;
        Number = number;
        Sheet = sheet;
        Placement = placement;
        _drawing = new SheetPageDrawing(sheet, placement);
    }

    /// <summary>A page listing the sheet's notes rather than its cells.</summary>
    /// <remarks>
    /// A separate constructor rather than a flag on <see cref="SheetPagePlacement"/>, because a
    /// note page holds no block of cells and no scale: <c>ScPrintFunc::PrintNotes</c> shares only
    /// the paper, the margins and the furniture with <c>PrintPage</c> and draws everything else
    /// itself (<c>sc/source/ui/view/printfun.cxx:2004-2066</c>).
    /// </remarks>
    internal SheetPage(int index, int number, SheetLayout sheet, IReadOnlyList<PlacedNote> notes)
    {
        Index = index;
        Number = number;
        Sheet = sheet;
        Placement = default;
        _notes = new SheetNotePageDrawing(sheet, notes);
    }

    /// <summary>True when the page lists the sheet's notes rather than its cells.</summary>
    public bool IsNotePage => _notes is not null;

    /// <inheritdoc/>
    long IPageRenderLimits.MaxImagePixels => _maxImagePixels;

    /// <inheritdoc/>
    OversizedImagePolicy IPageRenderLimits.OversizedImages => _oversizedImages;

    /// <summary>
    /// True on the first page of the whole printout, which is what
    /// <c>headerFooter/@differentFirst</c> selects on.
    /// </summary>
    /// <remarks>
    /// <strong>The printout's first page, not each sheet's.</strong> Calc decides with
    /// <c>bFirst = 0 == nPageNo</c> (<c>sc/source/ui/view/printfun.cxx:1796</c>), and that
    /// <c>nPageNo</c> counts across the whole print job rather than restarting per table — the
    /// per-table value is <c>aTableParam.nFirstPageNo</c>, which is added to it a few lines later
    /// to make the number that gets *printed* (<c>:1828</c>). Reading it as per-sheet is the
    /// natural mistake and it is measurable: <c>042_Business_monthly_budget_4e4d092f.xlsx</c> has
    /// four sheets of one page each, all four declaring <c>differentFirst</c>, and the reference
    /// leaves page 1 bare while drawing the footer on pages 2, 3 and 4 — each of which is its own
    /// sheet's first page. Suppressing per sheet costs that workbook 16 words instead of 4.
    /// </remarks>
    internal bool IsFirstPrintedPage { get; init; }

    /// <summary>
    /// True for the first page a sheet contributes, which is where the sheet's entry in a PDF's outline
    /// points.
    /// </summary>
    internal bool StartsSheet { get; init; }

    /// <inheritdoc/>
    public int Index { get; }

    /// <summary>The printed page number, which restarts per sheet when the sheet says so.</summary>
    public int Number { get; }

    /// <summary>The sheet this page came from.</summary>
    public SheetLayout Sheet { get; }

    /// <summary>Which cells the page holds, at what scale, with which repeated bands.</summary>
    public SheetPagePlacement Placement { get; }

    /// <inheritdoc/>
    public DocSize Size => Sheet.Setup.PageSize;

    /// <inheritdoc/>
    /// <remarks>
    /// The sheet's name, which is what Calc puts in the default header and what a reader uses to
    /// tell one page of a workbook from another. The printed page number is
    /// <see cref="Number"/>; a spreadsheet's pages are identified by sheet far more usefully
    /// than by ordinal, since a workbook's page seven means nothing without knowing which sheet
    /// it is the seventh page of.
    /// </remarks>
    public string? Label => Sheet.Name;

    /// <summary>
    /// How many pages the whole printout has, which is what a header's <c>&amp;N</c> stands for.
    /// </summary>
    /// <remarks>
    /// A property of the job rather than of the page, and not knowable until every sheet has
    /// been paginated — which is why it is set afterwards rather than passed to the constructor.
    /// Calc keeps it in <c>ScHeaderFieldData::nTotalPages</c> and fills it in the same way.
    /// </remarks>
    public int PageCount { get; internal set; } = 1;

    /// <summary>
    /// The instant the printout dates itself at, which is what <c>&amp;D</c> and <c>&amp;T</c>
    /// print.
    /// </summary>
    /// <remarks>
    /// A property of the job like <see cref="PageCount"/>, and stamped on for the same reason:
    /// every page of one printout prints the same instant, so reading the clock in each page's
    /// header context would let a slow render disagree with itself. <c>ScHeaderFieldData</c>
    /// holds one <c>Date</c> and one <c>Time</c> for the whole job.
    /// </remarks>
    public DateTime Printed { get; internal set; } = SheetPrintInstant.Now();

    /// <inheritdoc/>
    public void Draw(IDrawingSink sink)
    {
        if (_notes is { } notes) notes.Draw(sink, HeaderContext());
        else if (_drawing is not null)
        {
            _drawing.StartsSheet = StartsSheet;
            _drawing.Draw(sink, HeaderContext());
        }
    }

    /// <summary>
    /// What this page's header and footer fields stand for.
    /// </summary>
    /// <remarks>
    /// <c>&amp;Z</c> gets the file's name rather than its path, because that is all a document
    /// read from a stream ever knows: <c>DocumentSource</c> carries a name hint and not a
    /// location. The name is nearer than nothing, and a caller with a path can say so by
    /// building the context itself.
    /// </remarks>
    private SheetHeaderContext HeaderContext() => new()
    {
        PageNumber = Number,
        PageCount = PageCount,
        IsFirstPageOfSheet = IsFirstPrintedPage,
        SheetName = Sheet.Name,
        FileName = Sheet.FileName,
        FilePath = Sheet.FileName,
        Printed = Printed,
    };
}

/// <summary>
/// The pages a workbook printed to.
/// </summary>
/// <remarks>
/// <para>
/// The adapter between the paginator's own answer and the family-neutral
/// <see cref="IPageSequence"/>, in the same shape <c>WordProcessingPages</c> takes for the same
/// reason: a spreadsheet's page carries a block of cells and a scale, and a word-processing
/// page carries line boxes, so there is nothing worth abstracting over the two beyond a count
/// and a size.
/// </para>
/// <para>
/// Which sheets get pages is decided here rather than by the paginator, because the rule is
/// workbook-wide and slightly surprising. Calc prints every visible sheet — but as soon as
/// <em>any</em> sheet in the workbook declares a print range, the sheets that declare none are
/// printed only if their own "print entire sheet" flag is still set
/// (<c>ScPrintFunc::InitParam</c>, <c>sc/source/ui/view/printfun.cxx:1022-1046</c>). A hidden
/// sheet is never printed, which is the one place layout and extraction deliberately disagree:
/// extraction keeps hidden sheets and flags them.
/// </para>
/// </remarks>
public sealed class SpreadsheetPages : IPageSequence
{
    private readonly List<SheetPage> _pages;

    /// <summary>Lays a workbook's sheets out into pages.</summary>
    /// <param name="sheets">The sheets, in workbook order.</param>
    /// <param name="options">Layout options; null for the defaults.</param>
    public SpreadsheetPages(IReadOnlyList<SheetLayout> sheets, LayoutOptions? options = null)
    {
        ArgumentNullException.ThrowIfNull(sheets);

        Sheets = sheets;

        // Built under the options' own scope when the caller is not already inside a layout, so that
        // the pages pick up its picture limits and a direct construction honours its time limit.
        _pages = OfficeOperation.Layout(options, () => Build(sheets, options ?? LayoutOptions.Default));

        // The total is only knowable once every sheet has been paginated, and a header holding
        // "&N" needs it, so it is stamped on afterwards rather than threaded through the loop.
        DateTime printed = SheetPrintInstant.Now();
        foreach (SheetPage page in _pages)
        {
            page.PageCount = _pages.Count;
            page.Printed = printed;
        }
    }

    /// <summary>The sheets the pages were laid out from, printed or not.</summary>
    public IReadOnlyList<SheetLayout> Sheets { get; }

    /// <inheritdoc/>
    public int Count => _pages.Count;

    /// <inheritdoc/>
    public IPage this[int index] => _pages[index];

    /// <summary>The pages, with the cell block and scale a renderer needs.</summary>
    public IReadOnlyList<SheetPage> Pages => _pages;

    /// <summary>True when a sheet contributes pages to a printout of the workbook.</summary>
    /// <param name="sheet">The sheet to test.</param>
    /// <param name="anySheetHasPrintRange">
    /// Whether any sheet in the workbook declares a print range, which is what makes the
    /// per-sheet flag matter at all.
    /// </param>
    public static bool IsPrinted(SheetLayout sheet, bool anySheetHasPrintRange)
    {
        ArgumentNullException.ThrowIfNull(sheet);

        if (sheet.IsHidden) return false;
        if (!anySheetHasPrintRange) return true;

        return sheet.Setup.PrintAreas.Count > 0 || sheet.Setup.PrintsEntireSheet;
    }

    private static List<SheetPage> Build(IReadOnlyList<SheetLayout> sheets, LayoutOptions options)
    {
        bool anyPrintRange = sheets.Any(sheet => sheet.Setup.PrintAreas.Count > 0);
        List<SheetPage> pages = [];

        // The page number continues across sheets unless a sheet asks to restart, which is what
        // Calc's nPageStart carries between tables (printfun.cxx:958). Note that this is the
        // number *printed*; which page counts as the printout's first, for `differentFirst`, is
        // `pages.Count == 0` below and has nothing to do with it.
        int number = 1;

        foreach (SheetLayout sheet in sheets)
        {
            OfficeOperation.ThrowIfCancelled();
            if (!IsPrinted(sheet, anyPrintRange)) continue;

            if (sheet.Setup.FirstPageNumber > 0) number = sheet.Setup.FirstPageNumber;
            bool startedSheet = false;

            // Paginated, then thinned. The grid a page lands on is geometry, but the page *count*
            // is not: Calc's `m_nPagesY` skips a row band `IsPrintEmpty` is true across
            // (printfun.cxx:3176), and the fit-to-page zoom search bisects on that count — so the
            // emptiness test has to be reachable from inside pagination as well as after it.
            // Numbering follows the thinning: a dropped page takes its number with it.
            foreach (SheetPagePlacement placement in SheetEmptyPages.Occupied(
                         sheet,
                         SheetPagination.Paginate(
                             sheet.Setup, sheet.Grid, sheet.PrintedRange,
                             block => SheetEmptyPages.IsPrintEmpty(sheet, block))))
            {
                OfficeOperation.ThrowIfCancelled();
                if (options.MaxPages > 0 && pages.Count >= options.MaxPages) return pages;

                pages.Add(new SheetPage(pages.Count, number, sheet, placement)
                {
                    IsFirstPrintedPage = pages.Count == 0,
                    StartsSheet = !startedSheet,
                });
                startedSheet = true;
                number++;
            }

            // The sheet's notes are listed after its cells and before the next sheet, which is
            // where `ScPrintFunc::CountNotePages` puts them: the note pages are counted onto the
            // end of the table's own page count (`printfun.cxx:2557`) and numbered with them.
            foreach (IReadOnlyList<PlacedNote> notes in SheetNotePages.Paginate(sheet))
            {
                OfficeOperation.ThrowIfCancelled();
                if (options.MaxPages > 0 && pages.Count >= options.MaxPages) return pages;

                pages.Add(new SheetPage(pages.Count, number, sheet, notes)
                {
                    IsFirstPrintedPage = pages.Count == 0,
                });
                number++;
            }
        }

        return pages;
    }
}

/// <summary>One run of columns placed together, and where it starts.</summary>
/// <remarks>
/// A page has one or two: the repeated columns, when the sheet declares any, and its own. They
/// are separate because Calc prints them as separate blocks, so each has its own last column —
/// and it is the last column of a block that decides whose spill reaches back into it.
/// </remarks>
/// <param name="Last">
/// The band's last column, hidden or not — Calc's <c>mnX2</c>. Only the cell past <em>this</em>
/// column can be pulled onto the page, because the lookahead that finds it runs at <c>nX ==
/// mnX2</c> and the mirror-image walk to the left of <c>mnX1</c> is behind <c>!bTaggedPDF</c>.
/// See <see cref="SheetPageDrawing"/>.
/// </param>
/// <param name="Left">Where the band starts, scaled.</param>
/// <param name="Right">
/// Where it ends, scaled — Calc's <c>mnScrX + mnScrW</c> for the block it prints. A cell whose
/// output area falls entirely outside those two edges is not drawn at all, which is the whole of
/// what the pair is for; see <see cref="SheetTextContext"/>.
/// </param>
/// <param name="FirstVisible">
/// The band's first column that is actually placed — Calc's <c>mnVisX1</c>, which is
/// <c>mnX1</c> after <c>ScDocument::StripHidden</c> has walked it past any leading hidden
/// columns (<c>sc/source/ui/view/output.cxx:198-202</c>). One rule turns on it: a covered cell
/// may reach back to a merge's origin outside the band only when it is this column. Minus one
/// when the band places nothing.
/// </param>
internal readonly record struct ColumnBand(
    int Last, Length Left, Length Right, int FirstVisible);

/// <summary>One run of rows placed together, and where it starts.</summary>
/// <remarks>
/// The other axis of <see cref="ColumnBand"/>, and it exists for the same reason: a page's
/// repeated rows and its own rows are two separate <c>ScPrintFunc::PrintArea</c> calls
/// (<c>printfun.cxx:2321</c> and <c>:2331</c>), so each is its own <c>ScOutputData</c> with its own
/// <c>mnVisY1</c>, and only a band's own first placed row may reach back to a merge's origin above
/// it. It carries no right-hand edge because the page clips vertically on its own; the pair on
/// <see cref="ColumnBand"/> exists for the horizontal overflow rules, which have no vertical twin.
/// </remarks>
/// <param name="Top">Where the band starts, scaled.</param>
/// <param name="FirstVisible">
/// The band's first row that is actually placed — Calc's <c>mnVisY1</c>, which is <c>mnY1</c>
/// after the leading hidden rows have been walked past. Minus one when the band places nothing.
/// </param>
internal readonly record struct RowBand(Length Top, int FirstVisible);

/// <summary>
/// Places and draws the cells of one page.
/// </summary>
/// <remarks>
/// <para>
/// The geometry is <c>ScPrintFunc::PrintPage</c>'s (<c>printfun.cxx:2070</c>): the printable
/// rectangle is the paper less the margins and the header and footer bands, the row and column
/// headings take a fixed strip off its top and left when they are printed, the repeated bands
/// come next, and the page's own cells fill what is left. Everything but the headings is scaled
/// by the print zoom, which is why the offsets are accumulated in document twips and multiplied
/// once at the end.
/// </para>
/// <para>
/// Where the text goes inside a cell is <see cref="SheetTextLayout"/>'s business, not this
/// class's: alignment, wrapping, shrink-to-fit, indent, rotation, the overflow of a long string
/// into empty neighbours and the <c>###</c> a too-narrow numeric cell shows are all one body of
/// rules ported from Calc's own text output, and none of them needs to know what a page is. What
/// this class supplies is the cell's rectangle, the print zoom and a way to ask whether the
/// neighbours are free.
/// </para>
/// <para>
/// <strong>A run that overflows rightwards is painted on the page holding its anchor cell and on
/// no other, however far its text reaches.</strong> This is the one rule about overflow that a
/// page decides rather than <see cref="SheetTextLayout"/>, and it is <em>not</em> what reading
/// <c>ScOutputData::LayoutStrings</c> suggests. That loop starts one column before the block —
/// <c>if (mnX1 &gt; 0 &amp;&amp; !bTaggedPDF) --nLoopStartX; // start before mnX1 for rest of long
/// text to the left</c> (<c>sc/source/ui/view/output2.cxx:1541-1543</c>) — and the extra iteration
/// resolves to the nearest cell with text at or left of <c>mnX1</c>, however many empty columns
/// away (<c>:1638-1656</c>). The guard is the whole of it: <c>UseTaggedPDF</c>'s default is
/// <c>true</c> (<c>officecfg/registry/schema/org/openoffice/Office/Common.xcs:4318-4323</c>), so
/// every PDF <c>soffice --convert-to pdf</c> writes is tagged, <c>bTaggedPDF</c> holds, and the
/// branch that draws the lead-in is never taken.
/// </para>
/// <para>
/// Measured rather than reasoned, because the C++ tree here is 27.2-alpha and the reference binary
/// is 26.2.4.2. Rendering <c>essd-16-3433-2024-t02.xlsx</c> twice through the <em>same</em>
/// 26.2.4.2 with nothing changed but that one filter option gives words per page
/// <c>439 / 0 / 0 / 0</c> tagged and <c>439 / 315 / 152 / 49</c> untagged — and the untagged
/// figures are ours to within a word, so the port was faithful to a branch the reference never
/// takes. Drawing the lead-in anyway put 617 words of another column's spill on five pages of
/// <c>RCO_VOR_Master_List_082824.xlsx</c> that LibreOffice leaves blank.
/// </para>
/// <para>
/// <strong>The rule is asymmetric, and the other half is still drawn.</strong> Calc's rightward
/// lookahead — at <c>nX == mnX2</c>, scan right over empty cells for the first cell with content
/// so that a <em>right-aligned</em> run anchored past the block spills leftwards into it
/// (<c>output2.cxx:1660-1678</c>) — carries no <c>bTaggedPDF</c> guard, and the probe confirms it
/// on the binary: a right-aligned string anchored in the last column is painted on all three of
/// the pages its width covers.
/// </para>
/// <para>
/// None of this touches which pages <em>exist</em>. That is <c>IsPrintEmpty</c>'s question, it is
/// answered from the print area's overflow extension rather than from the paint, and it is
/// genuinely independent of tagging — which is why a page whose only content would have been a
/// lead-in is still printed, and printed blank. See <see cref="SheetEmptyPages"/> and
/// <see cref="SheetTextOverflow.ExtendedLastColumn"/>.
/// </para>
/// </remarks>
internal sealed class SheetPageDrawing(SheetLayout sheet, SheetPagePlacement placement)
{
    private readonly double _scale = placement.Scale;
    private readonly SheetPageDecoration _decoration = new(sheet, placement);
    private readonly SheetPageGraphics _graphics =
        new(sheet, placement.Scale);

    /// <summary>True when this is the first page the sheet contributes; see <see cref="SheetPage.StartsSheet"/>.</summary>
    internal bool StartsSheet { get; set; }

    /// <summary>
    /// Draws the page: what is painted behind the cells, their text, and the page's furniture.
    /// </summary>
    /// <remarks>
    /// The order is <c>ScPrintFunc</c>'s (<c>printfun.cxx:1679-1713</c> and <c>:2344-2404</c>):
    /// backgrounds, borders, cell text, the grid, the drawing layer, the headings and the frame
    /// round them. Each step covers part of the one before it, so the order is correctness rather
    /// than taste — a background painted after a border would erase half of it, and a picture
    /// painted before the grid would be crossed by it.
    /// </remarks>
    /// <param name="sink">Receives the drawing commands.</param>
    /// <param name="context">
    /// What a header's fields stand for on this page, or null when nothing needs them.
    /// </param>
    public void Draw(IDrawingSink sink, SheetHeaderContext? context = null)
    {
        ArgumentNullException.ThrowIfNull(sink);

        sink.BeginPage(sheet.Setup.PageSize);
        try
        {
            // One outline entry per sheet, at the top of the sheet's first page: the bookmarks panel of a
            // PDF reads as the workbook's tab strip.
            if (StartsSheet && sheet.Name is { Length: > 0 } sheetName)
            {
                sink.AddOutlineEntry(1, sheetName, DocPoint.Origin);
            }

            OfficeOperation.ThrowIfCancelled();
            DocPoint origin = BodyOrigin;
            List<PlacedColumn> columns = Columns(origin.X, out List<ColumnBand> bands);
            List<PlacedRow> rows = Rows(origin.Y, out List<RowBand> rowBands);

            _decoration.DrawBackgrounds(columns, rows, sink);

            // Between the fills and the borders, which is where `drawCells` puts them: a bar is
            // painted inside the same `DrawBackground` pass as the cell's own colour and over it
            // (`sc/source/ui/view/output.cxx`:1000-1035), long before any border or text.
            _decoration.DrawDataBars(columns, rows, sink);

            // And an icon in the same pass and after the bar: `drawCells` calls `drawDataBars`
            // and then `drawIconSets` on the same rectangle (`output.cxx`:1028-1031), so a cell
            // carrying both draws the icon over the bar.
            _decoration.DrawIconSets(columns, rows, sink);
            _decoration.DrawBorders(columns, rows, sink);

            foreach (PlacedRow row in rows)
            {
                OfficeOperation.Poll();
                // No lead-in from the left: a rightward overflow belongs to its anchor's page
                // alone, because the loop that would draw it here is behind `!bTaggedPDF` and
                // every reference PDF is tagged. The class remarks carry the measurement.
                foreach (ColumnBand band in bands) DrawTrailIn(band, row, sink);

                RowBand rowBand = BandOf(rowBands, row);

                foreach (PlacedColumn column in columns)
                {
                    ContentTableCell? cell = sheet.CellAt(row.Row, column.Column);
                    if (cell is null)
                    {
                        DrawCoveredMerge(column, row, sink, BandOf(bands, column), rowBand);
                        continue;
                    }

                    string text = cell.GetOwnText();
                    if (text.Length == 0) continue;

                    DrawCell(text, cell, column, row, sink, BandOf(bands, column));
                }
            }

            _decoration.DrawGrid(columns, rows, sink);

            // After the grid and before the headings, which is Calc's own order: a picture is on
            // the front drawing layer and covers the gridlines under it (printfun.cxx:1695-1703).
            _graphics.Draw(sink, columns, rows);

            _decoration.DrawHeadings(HeadingOrigin, columns, rows, sink);
            _decoration.DrawHeaderAndFooter(context ?? new SheetHeaderContext(), sink);
        }
        finally
        {
            // Always closed, even when a sink throws: a page left open would make the next one
            // nest inside it, turning one bad page into a broken document.
            sink.EndPage();
        }
    }

    /// <summary>
    /// How much of the page the row and column heading strips take, at the print scale.
    /// </summary>
    /// <remarks>
    /// Zero on both axes when the sheet does not print them. Snapped, and snapped before the
    /// zoom, for the same reason a column is (<see cref="SheetDeviceUnits"/>) and with the same
    /// citation — <c>nHeaderWidth = PRINT_HEADER_WIDTH * nScaleX</c> (<c>printfun.cxx:2204</c>)
    /// is computed in unscaled hundredths of a millimetre and the map mode's fraction scales it
    /// afterwards. The strip's own error is small, a twentieth of a hundredth; what matters is
    /// that it leaves the block's origin on a whole device unit, so the snapped column widths
    /// accumulate onto the boundaries LibreOffice puts them on. Measured on
    /// <c>sheet-decor-ods.ods</c>: the first column starts at exactly 3000 hundredths — 85.039 pt
    /// — in both renderings.
    /// </remarks>
    private DocPoint HeadingStrip => sheet.Setup.PrintsHeadings
        ? new DocPoint(
            SheetDeviceUnits.Snap(SheetPageDecoration.HeadingWidth) * _scale,
            SheetDeviceUnits.Snap(SheetPageDecoration.HeadingHeight) * _scale)
        : new DocPoint(Length.Zero, Length.Zero);

    /// <summary>Where the block starts including its heading strips, which the frame encloses.</summary>
    private DocPoint HeadingOrigin
    {
        get
        {
            DocPoint body = BodyOrigin;
            if (!sheet.Setup.PrintsHeadings) return body;

            return new DocPoint(body.X - HeadingStrip.X, body.Y - HeadingStrip.Y);
        }
    }

    /// <summary>
    /// Where the page's cell block starts, centring included.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Centring is measured against what this page holds rather than against the sheet, which is
    /// what makes the last page of a sheet sit differently from the ones before it: Calc sums the
    /// widths of the columns on <em>this</em> page and halves the remainder
    /// (<c>ScPrintFunc::PrintPage</c>, <c>printfun.cxx:2150</c>).
    /// </para>
    /// <para>
    /// <strong>The remainder is halved whatever its sign, so a block wider than the page is
    /// centred <em>off both edges</em> rather than left alone.</strong> Calc writes
    /// <c>nLeftSpace += ( aPageRect.GetWidth() - nDataWidth ) / 2</c> with no clamp
    /// (<c>printfun.cxx</c>:2165) and <c>nTopSpace</c> the same way (<c>:2188</c>), so the offset
    /// goes negative exactly when the columns do not fit and the block loses text at the left
    /// margin as well as at the right. Guarding the addition on a positive remainder — which this
    /// did — turns centring into left-alignment on precisely the sheets where the flag is most
    /// visible. Measured on <c>048_Expense_trends_budget</c>, whose <c>tips</c> sheet is one
    /// 152-character column under <c>printOptions/@horizontalCentered</c>: every line of page 1
    /// sits <strong>167.3 pt</strong> left of ours in 26.2.4.2, the same constant on all nineteen.
    /// </para>
    /// <para>
    /// The heading strip belongs to the block being centred, not to the margin: Calc adds
    /// <c>PRINT_HEADER_WIDTH</c> to <c>nDataWidth</c> before halving (<c>printfun.cxx</c>:2156)
    /// and only then lays the strip at <c>nStartX</c> with the body at
    /// <c>nStartX + nHeaderWidth</c> (<c>:2219</c>, <c>:2252</c>). Adding the strip to the origin
    /// and leaving it out of the extent, as this did, puts the body half a strip too far in. No
    /// corpus sheet prints headings <em>and</em> centres, so this half is transcribed from the
    /// source rather than measured.
    /// </para>
    /// </remarks>
    private DocPoint BodyOrigin
    {
        get
        {
            SheetPrintSetup setup = sheet.Setup;
            // At the page's own print scale, because the two furniture bands reach the paper
            // scaled and the four margins do not — see `SheetPrintSetup.PrintableAreaAt`. This
            // is the whole of round 56's 18.46 pt body offset.
            DocRect area = setup.PrintableAreaAt(_scale);

            // Two roundings and one scaling, in Calc's own order. The margin and the heading
            // strip each reach the device separately — nStartX = nLeftSpace * nScaleX and
            // nHeaderWidth = PRINT_HEADER_WIDTH * nScaleX, both truncated into hundredths of a
            // millimetre (printfun.cxx:2204 and :2220) — and only then does the map mode's
            // fraction apply the print zoom, which is why the strip is snapped before it is
            // scaled and the margin is not scaled at all (aPageRect divides it by the zoom
            // first, printfun.cxx:2104, so the device's multiplication gives it back).
            Length x = SheetDeviceUnits.Snap(area.X) + HeadingStrip.X;
            Length y = SheetDeviceUnits.Snap(area.Y) + HeadingStrip.Y;

            if (setup.CentresHorizontally)
            {
                Length block = Extent(Columns(x).Select(c => c.Width)) + HeadingStrip.X;
                x += (area.Width - block) / 2;
            }

            if (setup.CentresVertically)
            {
                Length block = Extent(Rows(y).Select(r => r.Height)) + HeadingStrip.Y;
                y += (area.Height - block) / 2;
            }

            return new DocPoint(x, y);
        }
    }

    private static Length Extent(IEnumerable<Length> sizes)
    {
        Length total = Length.Zero;
        foreach (Length size in sizes) total += size;
        return total;
    }

    /// <summary>The columns on the page, repeated band first, each with its position.</summary>
    /// <remarks>
    /// Snapped to the device unit <em>before</em> the print zoom is applied, not after, and the
    /// difference is measurable on any scaled sheet: Calc hands its output device coordinates in
    /// unscaled hundredths of a millimetre and lets the map mode's fraction do the scaling, so a
    /// 72 pt column at 66% comes out at exactly 47.52 pt rather than at the 47.5087 that snapping
    /// the scaled value gives. See <see cref="SheetDeviceUnits"/>.
    /// </remarks>
    private List<PlacedColumn> Columns(Length left) => Columns(left, out _);

    /// <inheritdoc cref="Columns(Length)"/>
    /// <param name="left">Where the block starts.</param>
    /// <param name="bands">
    /// Receives where each band of columns falls, which is what a trail-in needs: Calc prints a
    /// page's repeated columns and its own columns as two separate <c>ScPrintFunc::PrintArea</c>
    /// calls (<c>printfun.cxx:2312</c> and <c>:2330</c>), each with its own <c>mnX2</c>, and each
    /// therefore with its own right-hand neighbour to look ahead to.
    /// </param>
    private List<PlacedColumn> Columns(Length left, out List<ColumnBand> bands)
    {
        List<PlacedColumn> placed = [];
        List<ColumnBand> starts = [];
        Length offset = Length.Zero;

        if (placement.RepeatColumns is { IsValid: true } repeat)
            Append(repeat.FirstColumn, repeat.LastColumn);

        Append(placement.Cells.FirstColumn, placement.Cells.LastColumn);
        bands = starts;
        return placed;

        void Append(int first, int last)
        {
            Length start = left + (offset * _scale);
            int visible = -1;

            for (int column = first; column <= last; column++)
            {
                if (sheet.Grid.Columns.IsHidden(column)) continue;
                if (visible < 0) visible = column;

                Length width = SheetDeviceUnits.Snap(sheet.Grid.Columns.SizeAt(column));
                placed.Add(new PlacedColumn(column, left + (offset * _scale), width * _scale));
                offset += width;
            }

            starts.Add(new ColumnBand(last, start, left + (offset * _scale), visible));
        }
    }

    /// <summary>The rows on the page, repeated band first.</summary>
    private List<PlacedRow> Rows(Length top) => Rows(top, out _);

    /// <inheritdoc cref="Rows(Length)"/>
    /// <param name="top">Where the block starts.</param>
    /// <param name="bands">
    /// Receives where each band of rows begins and which row it really starts at, which is what a
    /// merge reaching back above the page needs: only a band's own first placed row may do so.
    /// </param>
    private List<PlacedRow> Rows(Length top, out List<RowBand> bands)
    {
        List<PlacedRow> placed = [];
        List<RowBand> starts = [];
        Length offset = Length.Zero;

        if (placement.RepeatRows is { IsValid: true } repeat)
            Append(repeat.FirstRow, repeat.LastRow);

        Append(placement.Cells.FirstRow, placement.Cells.LastRow);
        bands = starts;
        return placed;

        void Append(int first, int last)
        {
            Length start = top + (offset * _scale);
            int visible = -1;

            for (int row = first; row <= last; row++)
            {
                if (sheet.Grid.Rows.IsHidden(row)) continue;
                if (visible < 0) visible = row;

                Length height = SheetDeviceUnits.Snap(sheet.Grid.Rows.SizeAt(row));
                placed.Add(new PlacedRow(row, top + (offset * _scale), height * _scale));
                offset += height;
            }

            starts.Add(new RowBand(start, visible));
        }
    }

    /// <summary>
    /// The context a cell's text is laid out in: the zoom, the neighbours, the column widths.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The widths come from the sheet's grid rather than from the page's placed columns, because a
    /// string may spill past the last column on the page and Calc measures the spill against the
    /// document (<c>mpDoc-&gt;GetColWidth</c> in <c>GetOutputArea</c>). Using the page's columns
    /// would stop the overflow at the page boundary and draw a shorter string on the last column
    /// of every page.
    /// </para>
    /// <para>
    /// A neighbour is free only when it is both empty <em>and</em> outside every merge.
    /// <c>ScOutputData::IsAvailable</c> (<c>sc/source/ui/view/output2.cxx:1178-1191</c>) asks two
    /// questions and the second is the one a content tree cannot answer on its own: the cells a
    /// merge covers are dropped by every reader, so they look exactly like empty ones. Overflowing
    /// through them draws a string that Calc cuts short — measured on <c>sheet-features.ods</c>,
    /// where the reference shortens "Second row of pair" to "Second row of p" at the edge of the
    /// two-row merge beside it and X-Ray.Office drew all of it.
    /// </para>
    /// </remarks>
    private SheetTextContext ContextFor(ColumnBand band) => new(
        _scale,
        (row, column) => SheetTextLayout.IsAvailable(sheet.CellAt(row, column))
                         && !sheet.IsMerged(row, column),
        column => SheetDeviceUnits.Snap(sheet.Grid.Columns.PrintedSizeAt(column)) * _scale,
        band.Left,
        band.Right,
        sheet.CellBreaksStartLines);

    /// <summary>Which band a placed column belongs to, by where it sits on the paper.</summary>
    /// <remarks>
    /// The bands do not overlap and are appended left to right, so the last one starting at or
    /// left of the column is the one that placed it. Asking by column <em>number</em> would be
    /// ambiguous: a repeated column is in both ranges and drawn twice, in two different places.
    /// </remarks>
    private static ColumnBand BandOf(List<ColumnBand> bands, PlacedColumn column)
    {
        ColumnBand found = bands[0];
        foreach (ColumnBand band in bands)
        {
            if (band.Left <= column.X) found = band;
        }
        return found;
    }

    /// <inheritdoc cref="BandOf(List{ColumnBand}, PlacedColumn)"/>
    private static RowBand BandOf(List<RowBand> bands, PlacedRow row)
    {
        RowBand found = bands[0];
        foreach (RowBand band in bands)
        {
            if (band.Top <= row.Y) found = band;
        }
        return found;
    }

    /// <summary>
    /// Draws the cell right of a band whose text reaches back into it, at the place it really is.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The half of Calc's string output that a page's own columns cannot supply — and, since the
    /// reference is always a tagged PDF, the <em>only</em> half. At its last column
    /// <c>ScOutputData::LayoutStrings</c> looks ahead past the block for the first cell with
    /// content, so that a run anchored beyond the paper can spill leftwards onto it
    /// (<c>sc/source/ui/view/output2.cxx:1660-1678</c>):
    /// </para>
    /// <code>
    /// if ( bEmpty &amp;&amp; !bMergeEmpty &amp;&amp; nX == mnX2 &amp;&amp; !bOverlapped )
    /// {
    ///     SCCOL nTempX=nX;
    ///     while (nTempX &lt; nLastContentCol &amp;&amp; IsEmptyCellText( pThisRowInfo, nTempX, nY ))
    ///         ++nTempX;
    ///     ...
    /// }
    /// </code>
    /// <para>
    /// <strong>Unlike the mirror-image walk to the left, this one carries no <c>bTaggedPDF</c>
    /// guard</strong> (contrast <c>:1541-1543</c>), so it survives into every exported PDF. That
    /// asymmetry is the whole of the difference between the two directions and it is measured, not
    /// inferred: an authored probe with a right-aligned string anchored in the last of twenty-one
    /// one-inch columns is drawn by 26.2.4.2 on all three of the pages its width covers — 15, 16
    /// and 14 of its tokens — while a left-anchored string in the same workbook appears on its own
    /// page and nowhere else.
    /// </para>
    /// <para>
    /// The cell is placed at its true position, which is off the right of the block, and the text
    /// then overflows leftwards under the ordinary rules — <see cref="ContextFor"/> already
    /// measures that against the document grid rather than against the page, which is what makes
    /// the two halves of one string line up across the break. A left-aligned cell found this way
    /// spills the other way, so its area never reaches the block and
    /// <c>SheetTextLayout</c>'s <c>bOutside</c> test drops it — which is why this needs no
    /// alignment condition of its own, exactly as Calc needs none.
    /// </para>
    /// <para>
    /// It cannot draw a cell twice on one page. The walk starts at the band's own last column, so
    /// a band whose last column holds text yields that column and the <c>&gt; last</c> test
    /// rejects it; only a column strictly right of the band — and therefore not among the band's
    /// placed columns — is ever drawn this way.
    /// </para>
    /// </remarks>
    private void DrawTrailIn(ColumnBand band, PlacedRow row, IDrawingSink sink)
    {
        // `nLastContentCol` (output2.cxx:1534-1539): the walk stops at the sheet's last column
        // holding anything rather than at the last column that could exist, so an empty row costs
        // a bounded scan instead of sixteen thousand lookups.
        int limit = sheet.UsedRange.LastColumn;
        if (band.Last >= limit) return;

        // Calc walks forward from mnX2 itself, so a band whose own last column holds text stops
        // there and no trail-in is drawn (output2.cxx:1666-1668).
        int at = band.Last;
        while (at < limit && SheetTextLayout.IsAvailable(sheet.CellAt(row.Row, at))) at++;
        if (at <= band.Last) return;

        ContentTableCell? cell = sheet.CellAt(row.Row, at);
        if (cell is null || SheetTextLayout.IsAvailable(cell)) return;
        if (sheet.Grid.Columns.IsHidden(at)) return;

        string text = cell.GetOwnText();
        if (text.Length == 0) return;

        // A merge anywhere between the two suppresses the trail-in: Calc asks
        // HasAttrib(Merged | Overlapped) over exactly that span (output2.cxx:1675), because a
        // merged block's own origin is what draws its text and it may be neither of these cells.
        for (int between = band.Last; between <= at; between++)
        {
            if (sheet.IsMerged(row.Row, between)) return;
        }

        Length ahead = Length.Zero;
        for (int column = band.Last + 1; column < at; column++)
            ahead += SheetDeviceUnits.Snap(sheet.Grid.Columns.PrintedSizeAt(column));

        DrawCell(
            text,
            cell,
            new PlacedColumn(
                at,
                band.Right + (ahead * _scale),
                SheetDeviceUnits.Snap(sheet.Grid.Columns.PrintedSizeAt(at)) * _scale),
            row,
            sink,
            band);
    }

    /// <summary>
    /// Draws a merged block from a covered cell, when the cell that anchors it is not on the
    /// page to draw it itself.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Four situations put a merge's origin out of reach and they are one rule, because a block is
    /// covered on two axes and both behave the same way. A hidden column or a hidden row is not
    /// placed on the page at all, so a merge anchored inside one is never reached; and a merge
    /// straddling a page break — across the paper or down it — has its origin on the *previous*
    /// page, so the page carrying its tail holds nothing but covered cells. Either way the whole
    /// block vanishes, however much of the page it goes on to cover.
    /// </para>
    /// <para>
    /// Calc reaches it from the other end: every covered cell asks
    /// <c>ScOutputData::GetMergeOrigin</c> (<c>output2.cxx:953</c>) for the block's origin, and the
    /// two walks — left while <c>bHOverlapped</c> (<c>:989</c>), then up while <c>bVOverlapped</c>
    /// (<c>:1008</c>) — are governed by one flag. <c>bDoMerge</c> is <c>bIsLeft = (nX == mnVisX1)</c>
    /// for a horizontally overlapped cell, <c>bIsTop = (nY == mnVisY1)</c> for a vertically
    /// overlapped one, and both together for a cell covered on both axes (<c>:958-983</c>): the
    /// block's first <em>visible</em> column and row on the page. When it is set the walk runs back
    /// to the origin through anything; when it is not, the walk gives up the moment it steps onto a
    /// column or row that is not hidden —
    /// <c>if (!bDoMerge &amp;&amp; !bHidden) return false;</c> (<c>:993</c>, <c>:1012</c>) — because
    /// that position is either the origin itself or a nearer covered cell, and one of them will draw
    /// the block instead.
    /// </para>
    /// <para>
    /// So exactly one cell of a block ever draws it on a given page: the block's first visible
    /// column and row when the origin is off the page above or to the left, and otherwise the
    /// nearest cell whose path back to the origin is entirely hidden. Every page a straddling merge
    /// touches draws it, each from the origin's true position, and each shows the part that lands on
    /// the paper — which is Calc's behaviour and the reason a long merged heading reads continuously
    /// across a column break and a tall merged description carries on down the next sheet of paper.
    /// </para>
    /// <para>
    /// The origin is placed at its true position, reached by subtracting the printed size of every
    /// column and row between — which is <c>GetOutputArea</c>'s own two walks back from
    /// <c>nX</c>/<c>nArrY</c> to <c>nCellX</c>/<c>nCellY</c> (<c>output2.cxx:1216-1254</c>), and puts
    /// the origin above the top of the paper when that is where it is. A hidden column or row
    /// contributes nothing, so a merge anchored in one still starts exactly where the first column
    /// or row it can be seen in starts.
    /// </para>
    /// <para>
    /// Measured on <c>RPD 155 REDAC SCHEDULE 2014-04-02.xls</c>, whose <c>Funds ($000)</c>
    /// heading is a four-column merge anchored in a collapsed column; on <c>P1636e.xls</c>, whose
    /// title and eight footnotes are each merged across all six columns while the page break falls
    /// after the third; and on <c>RegChangeReport.xlsx</c>, whose regulatory descriptions are merges
    /// up to thirty-five rows tall, so four of its twelve pages hold nothing but the continuation of
    /// a block anchored on the page before.
    /// </para>
    /// </remarks>
    private void DrawCoveredMerge(
        PlacedColumn column, PlacedRow row, IDrawingSink sink, ColumnBand band, RowBand rows)
    {
        // Only a covered position reaches back at all. An ordinary empty cell has no origin to
        // find, and walking back from one would drag an unrelated neighbour onto the page.
        if (MergeOver(row.Row, column.Column) is not { } merge) return;

        bool horizontal = column.Column > merge.FirstColumn;
        bool vertical = row.Row > merge.FirstRow;

        // The block's own anchor, which holds a cell this never sees: there is nowhere to walk to.
        if (!horizontal && !vertical) return;

        bool left = column.Column == band.FirstVisible;
        bool top = row.Row == rows.FirstVisible;
        bool doMerge = horizontal && vertical ? left && top : horizontal ? left : top;

        Length back = Length.Zero;
        for (int at = column.Column - 1; at >= merge.FirstColumn; at--)
        {
            if (!doMerge && !sheet.Grid.Columns.IsHidden(at)) return;
            back += SheetDeviceUnits.Snap(sheet.Grid.Columns.PrintedSizeAt(at)) * _scale;
        }

        Length up = Length.Zero;
        for (int at = row.Row - 1; at >= merge.FirstRow; at--)
        {
            if (!doMerge && !sheet.Grid.Rows.IsHidden(at)) return;
            up += SheetDeviceUnits.Snap(sheet.Grid.Rows.PrintedSizeAt(at)) * _scale;
        }

        // A merge the file states whose anchor holds nothing — an empty block, or one whose anchor
        // was dropped as trailing padding — has no text to draw and no cell to draw it as.
        if (sheet.CellAt(merge.FirstRow, merge.FirstColumn) is not { } origin) return;

        string text = origin.GetOwnText();
        if (text.Length == 0) return;

        DrawCell(
            text,
            origin,
            new PlacedColumn(
                merge.FirstColumn,
                column.X - back,
                SheetDeviceUnits.Snap(
                    sheet.Grid.Columns.PrintedSizeAt(merge.FirstColumn)) * _scale),
            new PlacedRow(
                merge.FirstRow,
                row.Y - up,
                SheetDeviceUnits.Snap(sheet.Grid.Rows.PrintedSizeAt(merge.FirstRow)) * _scale),
            sink,
            band);
    }

    /// <summary>The merged block covering a position, or null when nothing does.</summary>
    /// <remarks>
    /// The block itself rather than <see cref="SheetLayout.IsMerged"/>'s yes-or-no, because both
    /// walks back need to know where to stop and which axes are covered at all — Calc reads the
    /// same two facts off <c>ATTR_MERGE_FLAG</c>'s <c>ScMF::Hor</c> and <c>ScMF::Ver</c> bits.
    /// </remarks>
    private SheetRange? MergeOver(int row, int column)
    {
        foreach (SheetRange merge in sheet.MergedRanges)
        {
            if (row >= merge.FirstRow && row <= merge.LastRow
                && column >= merge.FirstColumn && column <= merge.LastColumn)
            {
                return merge;
            }
        }

        return null;
    }

    private void DrawCell(
        string text,
        ContentTableCell cell,
        PlacedColumn column,
        PlacedRow row,
        IDrawingSink sink,
        ColumnBand band)
    {
        // A `dataBar` stating `showValue="0"` takes the cell's own value off the page rather than
        // hiding it behind the bar, and it does so at drawing time: `ScOutputData::DrawStrings`
        // clears `bDoCell` after the row's height is already settled
        // (`sc/source/ui/view/output2.cxx`:1691-1698), so the layout keeps the cell and only the
        // paint drops it.
        if (sheet.Formatting.HidesValue(row.Row, column.Column)) return;

        SheetTextLayout.Draw(sink, ContextFor(band), new SheetCellText(
            text,
            cell.Value,
            sheet.Formats.At(row.Row, column.Column),
            row.Row,
            column.Column,
            new DocRect(column.X, row.Y, SpanWidth(cell, column), SpanHeight(cell, row)),
            sheet.RichText.At(row.Row, column.Column, text),
            sheet.HoldsField(row.Row, column.Column)));
    }

    /// <summary>How wide a cell is, a merge's further columns included.</summary>
    private Length SpanWidth(ContentTableCell cell, PlacedColumn column)
    {
        Length width = column.Width;
        for (int at = 1; at < Math.Max(1, cell.ColumnSpan); at++)
        {
            width += SheetDeviceUnits.Snap(
                sheet.Grid.Columns.PrintedSizeAt(column.Column + at)) * _scale;
        }
        return width;
    }

    /// <summary>How tall a cell is, a merge's further rows included.</summary>
    private Length SpanHeight(ContentTableCell cell, PlacedRow row)
    {
        Length height = row.Height;
        for (int at = 1; at < Math.Max(1, cell.RowSpan); at++)
        {
            height += SheetDeviceUnits.Snap(
                sheet.Grid.Rows.PrintedSizeAt(row.Row + at)) * _scale;
        }
        return height;
    }
}
