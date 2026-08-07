using Paperless.Core.Units;

namespace Paperless.Spreadsheets.Layout;

/// <summary>
/// One printed page of a sheet: which cells fall on it, and at what scale.
/// </summary>
/// <param name="Cells">The block of cells the page's body holds.</param>
/// <param name="RepeatColumns">
/// The columns repeated down this page's left edge, or null when it does not repeat any.
/// </param>
/// <param name="RepeatRows">The rows repeated across this page's top, or null.</param>
/// <param name="ZoomPercentage">
/// The scale the page is drawn at, which is 100 unless the sheet fits to a page count.
/// </param>
/// <param name="ColumnBand">Which band of columns the page is, counting from zero.</param>
/// <param name="RowBand">Which band of rows it is.</param>
/// <param name="AreaIndex">Which of the sheet's print ranges it came from.</param>
public readonly record struct SheetPagePlacement(
    SheetRange Cells,
    SheetRange? RepeatColumns,
    SheetRange? RepeatRows,
    int ZoomPercentage,
    int ColumnBand,
    int RowBand,
    int AreaIndex);

/// <summary>
/// Splits a sheet into printed pages, the way <c>ScPrintFunc</c> does.
/// </summary>
/// <remarks>
/// <para>
/// This is the routine the module's TODO singles out, and the reason it is singled out is that
/// a spreadsheet has no intrinsic pagination at all: there is no page in the file, only a print
/// setup from which pages follow. Getting it wrong does not shift a line, it changes how many
/// pages there are.
/// </para>
/// <para>
/// It is a port of two routines rather than one, because LibreOffice splits the work in two and
/// the split is load-bearing. <c>ScTable::UpdatePageBreaks</c>
/// (<c>sc/source/core/data/table5.cxx:57</c>) walks the columns and rows accumulating sizes
/// against the page and <em>records a break</em> wherever the running total would overflow;
/// <c>sc::PrintPageRanges::calculate</c> (<c>sc/source/ui/view/printfun.cxx:3082</c>) then turns
/// that set of breaks into page ranges. They are separate because the breaks are also what the
/// page-break preview draws, and because the second pass is where hidden columns and rows are
/// collapsed — several breaks inside one hidden block count as one.
/// </para>
/// <para>
/// The arithmetic is done in whole twips, deliberately. Calc accumulates
/// <c>sal_uInt16</c> twips and compares against a page size derived by integer division
/// (<c>… * 100 / nZoom</c>), so a page that fills exactly is decided by those roundings and not
/// by the exact measure. Working in EMUs and converting at the end would put a page boundary in
/// a different place on a sheet whose columns happen to fill the width.
/// </para>
/// </remarks>
public static class SheetPagination
{
    /// <summary>The smallest scale a fit-to-pages search will settle on.</summary>
    /// <remarks><c>ZOOM_MIN</c>, <c>sc/source/ui/view/printfun.cxx:70</c>.</remarks>
    public const int MinimumZoom = 10;

    /// <summary>The width the printed row headings take, when they are printed.</summary>
    /// <remarks>One centimetre — <c>PRINT_HEADER_WIDTH</c>, <c>sc/source/ui/inc/printfun.hxx:45</c>.</remarks>
    private const long HeadingWidthTwips = 567;

    /// <summary>The height the printed column headings take.</summary>
    /// <remarks>12.8 points — <c>PRINT_HEADER_HEIGHT</c>, the same header.</remarks>
    private const long HeadingHeightTwips = 256;

    /// <summary>
    /// Splits a sheet into pages.
    /// </summary>
    /// <param name="setup">The sheet's print setup, which is its page geometry.</param>
    /// <param name="grid">Its column widths and row heights.</param>
    /// <param name="used">
    /// The block of cells the sheet actually holds, used when it declares no print range.
    /// </param>
    /// <param name="isPrintEmpty">
    /// Calc's <c>IsPrintEmpty</c> over a block, or <see langword="null"/> when the caller cannot
    /// answer it. The zoom search needs it because the page count it bisects on counts only the
    /// row bands that are <em>not</em> empty; with no way to ask, every band is counted, which is
    /// what this class did before the question could be reached.
    /// </param>
    /// <returns>The pages, in the order the sheet's page order prints them.</returns>
    public static IReadOnlyList<SheetPagePlacement> Paginate(
        SheetPrintSetup setup, SheetGrid grid, SheetRange used,
        Func<SheetRange, bool>? isPrintEmpty = null)
    {
        ArgumentNullException.ThrowIfNull(setup);
        ArgumentNullException.ThrowIfNull(grid);

        // Several print ranges are paginated one after another, each with a zoom of its own —
        // CalcZoom is called per range in ScPrintFunc::DoPrint (printfun.cxx:2718), so a
        // fit-to-width setting fits each range separately rather than all of them together.
        IReadOnlyList<SheetRange> areas = setup.PrintAreas.Count > 0
            ? setup.PrintAreas
            : (used.IsValid ? [used] : []);

        List<SheetPagePlacement> pages = [];
        for (int index = 0; index < areas.Count; index++)
        {
            SheetRange area = Limit(areas[index], used);
            if (!area.IsValid) continue;

            pages.AddRange(PaginateArea(setup, grid, area, index, isPrintEmpty));
        }

        return pages;
    }

    /// <summary>
    /// Cuts a whole-column or whole-row print range back to what the sheet holds.
    /// </summary>
    /// <remarks>
    /// A print range of <c>A:D</c> covers a million rows, and paginating it literally would give
    /// a sheet of four columns twenty thousand blank pages. Calc does the same cutting for the
    /// same reason: <c>AdjustPrintArea(false)</c> re-searches an axis, and only that axis, when
    /// the range it was given spans the whole sheet along it (<c>printfun.cxx:707-712</c>).
    /// </remarks>
    private static SheetRange Limit(SheetRange area, SheetRange used)
    {
        if (!used.IsValid) return area;

        return area with
        {
            LastColumn = area.FirstColumn == 0 && area.LastColumn >= SheetAddress.MaxColumn
                ? used.LastColumn
                : area.LastColumn,
            LastRow = area.FirstRow == 0 && area.LastRow >= SheetAddress.MaxRow
                ? used.LastRow
                : area.LastRow,
        };
    }

    private static List<SheetPagePlacement> PaginateArea(
        SheetPrintSetup setup, SheetGrid grid, SheetRange area, int areaIndex,
        Func<SheetRange, bool>? isPrintEmpty)
    {
        (int zoom, List<int> columnEnds, List<(int First, int Last)> rowBands) =
            CalcZoom(setup, grid, area, isPrintEmpty);

        List<SheetPagePlacement> pages = [];
        if (columnEnds.Count == 0 || rowBands.Count == 0) return pages;

        void Add(int bandX, int bandY)
        {
            int firstColumn = bandX == 0 ? area.FirstColumn : columnEnds[bandX - 1] + 1;
            int lastColumn = columnEnds[bandX];
            (int firstRow, int lastRow) = rowBands[bandY];

            // A page starting inside the repeated band skips past it rather than printing
            // those columns twice; a page starting before it repeats nothing, because the band
            // is already there. ScPrintFunc::PrintPage does exactly this at printfun.cxx:2108.
            SheetRange? repeatColumns = null;
            if (setup.RepeatColumns is { } columns && columns.IsValid)
            {
                if (firstColumn > columns.FirstColumn && firstColumn <= columns.LastColumn)
                    firstColumn = columns.LastColumn + 1;
                if (firstColumn > columns.LastColumn) repeatColumns = columns;
            }

            SheetRange? repeatRows = null;
            if (setup.RepeatRows is { } rows && rows.IsValid)
            {
                if (firstRow > rows.FirstRow && firstRow <= rows.LastRow)
                    firstRow = rows.LastRow + 1;
                if (firstRow > rows.LastRow) repeatRows = rows;
            }

            pages.Add(new SheetPagePlacement(
                new SheetRange(firstColumn, firstRow, lastColumn, lastRow),
                repeatColumns,
                repeatRows,
                zoom,
                bandX,
                bandY,
                areaIndex));
        }

        if (setup.PageOrder == PagePrintOrder.DownThenAcross)
        {
            for (int x = 0; x < columnEnds.Count; x++)
                for (int y = 0; y < rowBands.Count; y++)
                    Add(x, y);
        }
        else
        {
            for (int y = 0; y < rowBands.Count; y++)
                for (int x = 0; x < columnEnds.Count; x++)
                    Add(x, y);
        }

        return pages;
    }

    /// <summary>
    /// Finds the scale the sheet prints at, searching when it has to fit a page count.
    /// </summary>
    /// <remarks>
    /// A bisection rather than a formula, and necessarily so: halving the scale does not halve
    /// the page count, because a column either fits on a page or starts a new one. LibreOffice
    /// bisects on the integer percentage between the last scale that fitted and the last that
    /// did not (<c>ScPrintFunc::CalcZoom</c>, <c>printfun.cxx:2816</c>), which converges in
    /// about seven repaginations.
    /// </remarks>
    private static (int Zoom, List<int> ColumnEnds, List<(int, int)> RowBands) CalcZoom(
        SheetPrintSetup setup, SheetGrid grid, SheetRange area,
        Func<SheetRange, bool>? isPrintEmpty)
    {
        int zoom;

        switch (setup.ScaleMode)
        {
            case PrintScaleMode.FitToPageCount:
            {
                int budget = Math.Max(1, setup.FitToPageCount);
                zoom = Search(candidate =>
                {
                    (int columns, int rows) = Count(candidate);
                    return columns * rows <= budget;
                });
                break;
            }

            case PrintScaleMode.FitToPages:
            {
                int wide = Math.Max(0, setup.FitToPagesWide);
                int tall = Math.Max(0, setup.FitToPagesTall);
                if (wide == 0 && tall == 0)
                {
                    zoom = 100;
                    break;
                }

                zoom = Search(candidate =>
                {
                    (int columns, int rows) = Count(candidate);
                    return (wide == 0 || columns <= wide) && (tall == 0 || rows <= tall);
                });

                // tdf#103516: fitting to width alone regularly leaves a last page holding one
                // row, because the search stops at the first scale that fits horizontally and
                // never looks at the vertical at all. A two-per-cent nudge removes that page
                // when it can, and is abandoned when it cannot — printfun.cxx:2988.
                if (wide > 0 && tall == 0)
                {
                    int before = Count(zoom).Rows;
                    int nudged = (int)(zoom * 0.98);
                    if (before > 1 && nudged < zoom && nudged >= MinimumZoom
                        && Count(nudged).Rows != before)
                    {
                        zoom = nudged;
                    }
                }
                break;
            }

            default:
                zoom = Math.Max(MinimumZoom, setup.ScalePercentage);
                break;
        }

        (List<int> columnEnds, List<(int, int)> rowBands) = Split(setup, grid, area, zoom);
        return (zoom, columnEnds, rowBands);

        (int Columns, int Rows) Count(int candidate)
        {
            (List<int> ends, List<(int, int)> bands) = Split(setup, grid, area, candidate);
            return (ends.Count, CountedRowBands(bands));
        }

        // Calc's `m_nPagesY`, which is *not* the number of row bands the geometry produces.
        // `PrintPageRanges::calculate` increments it only for a band `IsPrintEmpty` says false
        // for (`printfun.cxx:3176` for the bands the break iterator ends, `:3220` for the last
        // one), so a band of trailing empty rows never reaches the count the zoom search — and
        // the tdf#103516 nudge in particular — compares.
        //
        // Measured on `Company_Seniority_Date_Calculator.xlsx`, whose `Bulletin Clarification`
        // sheet states a print area of `A1:Y49` over a sheet whose last `<row>` is 48. At the
        // fitting zoom of 80 the rows split 2 bands with data in both; at the nudged 78 they
        // split rows 1-48 and the empty row 49, which Calc counts as **one** page and we counted
        // as two — so the nudge saw an unchanged count, was abandoned, and the sheet printed at
        // 80 where Calc prints it at 78, spilling a thirteenth page holding one row.
        int CountedRowBands(List<(int First, int Last)> bands)
        {
            if (isPrintEmpty is null) return bands.Count;

            int counted = 0;
            foreach ((int first, int last) in bands)
            {
                // The whole width of the print range, not the page's column band: Calc passes
                // `getStartColumn()`/`getEndColumn()` (`printfun.cxx:3174`).
                if (!isPrintEmpty(new SheetRange(area.FirstColumn, first, area.LastColumn, last)))
                    counted++;
            }

            return counted;
        }

        // The bisection itself, kept apart from what it is bisecting on because the two
        // fit-to modes differ only in the predicate.
        static int Search(Func<int, bool> fits)
        {
            int zoom = 100;
            int lastFit = 0;
            int lastNonFit = 0;

            while (zoom > MinimumZoom)
            {
                if (fits(zoom))
                {
                    if (zoom == 100) return 100;

                    lastFit = zoom;
                    int next = (lastNonFit + zoom) / 2;
                    if (next == lastFit) return lastFit;
                    zoom = next;
                }
                else
                {
                    if (zoom - lastFit <= 1) return Math.Max(MinimumZoom, lastFit);

                    lastNonFit = zoom;
                    zoom = (lastFit + zoom) / 2;
                }
            }

            // LibreOffice leaves the loop here with the page split from the *previous* scale
            // and the zoom from this one, which is self-inconsistent; clamping and letting the
            // caller re-split at the clamped value is the one deliberate deviation from the
            // port, and it only shows on a sheet with a column too wide to fit at any scale.
            return Math.Max(MinimumZoom, zoom);
        }
    }

    /// <summary>
    /// The printable area of one page, in document twips at a given scale.
    /// </summary>
    /// <remarks>
    /// A literal port of <c>ScPrintFunc::GetDocPageSize</c>
    /// (<c>printfun.cxx:2987</c>), including two details that look like noise and are not.
    /// The page rectangle is built from a VCL <c>tools::Rectangle</c>, whose right and bottom
    /// edges are inclusive — hence the <c>- 1</c> and the <c>+ 1</c> when the size is taken
    /// back off it. And each edge is divided by the zoom <em>separately</em> before the
    /// subtraction, so scaling a page is not the same as scaling its width.
    /// </remarks>
    private static (long Width, long Height) DocPageSize(SheetPrintSetup setup, int zoom)
    {
        long width = setup.PageSize.Width.Twips;
        long height = setup.PageSize.Height.Twips;
        int scale = Math.Max(1, zoom);

        long left = setup.LeftMargin.Twips * 100 / scale;
        long right = (width - 1 - setup.RightMargin.Twips) * 100 / scale;

        // The header and footer bands are subtracted unscaled, because Calc prints them at
        // full size whatever the sheet's scale: they are page furniture rather than content.
        long top = (setup.TopMargin.Twips * 100 / scale) + setup.HeaderHeight.Twips;
        long bottom = ((height - 1 - setup.BottomMargin.Twips) * 100 / scale)
                      - setup.FooterHeight.Twips;

        long pageWidth = right - left + 1;
        long pageHeight = bottom - top + 1;

        if (setup.PrintsHeadings)
        {
            pageWidth -= HeadingWidthTwips;
            pageHeight -= HeadingHeightTwips;
        }

        return (pageWidth, pageHeight);
    }

    /// <summary>
    /// Breaks a print range into bands of columns and rows at one scale.
    /// </summary>
    private static (List<int> ColumnEnds, List<(int First, int Last)> RowBands) Split(
        SheetPrintSetup setup, SheetGrid grid, SheetRange area, int zoom)
    {
        (long pageWidth, long pageHeight) = DocPageSize(setup, zoom);

        // A page with no room at all would divide the sheet into one band per column, which on
        // a full sheet is sixteen thousand pages. One twip is the floor Calc's own comparison
        // implies: a column of width w always fits a page of width w.
        pageWidth = Math.Max(1, pageWidth);
        pageHeight = Math.Max(1, pageHeight);

        // fdo#40788: a scale-to-pages setting overrides the author's manual breaks, because
        // honouring both is contradictory — the breaks fix a page count that the scaling is
        // trying to choose. Fitting to width alone suppresses only the column breaks (i#54993).
        bool skipColumnBreaks = setup.ScaleMode == PrintScaleMode.FitToPageCount
                                || (setup.ScaleMode == PrintScaleMode.FitToPages
                                    && setup.FitToPagesWide > 0);
        bool skipRowBreaks = setup.ScaleMode == PrintScaleMode.FitToPageCount
                             || (setup.ScaleMode == PrintScaleMode.FitToPages
                                 && setup.FitToPagesTall > 0);

        HashSet<int> columnBreaks = AxisBreaks(
            grid.Columns, area.FirstColumn, area.LastColumn, pageWidth,
            skipColumnBreaks ? [] : setup.ManualColumnBreaks,
            setup.RepeatColumns is { IsValid: true } rc ? (rc.FirstColumn, rc.LastColumn) : null);

        HashSet<int> rowBreaks = AxisBreaks(
            grid.Rows, area.FirstRow, area.LastRow, pageHeight,
            skipRowBreaks ? [] : setup.ManualRowBreaks,
            setup.RepeatRows is { IsValid: true } rr ? (rr.FirstRow, rr.LastRow) : null);

        List<int> columnEnds = Bands(grid.Columns, area.FirstColumn, area.LastColumn, columnBreaks);
        List<int> rowEnds = Bands(grid.Rows, area.FirstRow, area.LastRow, rowBreaks);

        List<(int, int)> rowBands = new(rowEnds.Count);
        int start = area.FirstRow;
        foreach (int end in rowEnds)
        {
            rowBands.Add((start, end));
            start = end + 1;
        }

        return (columnEnds, rowBands);
    }

    /// <summary>
    /// Where the pages start along one axis: the port of <c>ScTable::UpdatePageBreaks</c>.
    /// </summary>
    /// <remarks>
    /// The comparison is <c>running + this &gt; page</c> rather than <c>&gt;=</c>, so a set of
    /// columns whose widths sum to exactly the page width all fit on it — which is why the
    /// twip rounding above has to match Calc's.
    /// </remarks>
    private static HashSet<int> AxisBreaks(
        SheetAxis axis,
        int first,
        int last,
        long pageSize,
        IReadOnlyList<int> manualBreaks,
        (int First, int Last)? repeat)
    {
        HashSet<int> breaks = [];
        HashSet<int> manual = [.. manualBreaks];

        // A print range that does not start at the sheet's own origin begins with a break, so
        // that the first band starts where the range does (table5.cxx:155, "AREABREAK").
        if (first > 0) breaks.Add(first);

        long running = 0;
        bool repeatSubtracted = false;

        for (int index = first; index <= last; index++)
        {
            bool startsPage;
            long size = axis.PrintedSizeAt(index).Twips;

            if (running + size > pageSize || manual.Contains(index))
            {
                breaks.Add(index);
                running = 0;
                startsPage = true;
            }
            else if (index != first)
            {
                breaks.Remove(index);
                startsPage = false;
            }
            else
            {
                startsPage = true;
            }

            if (startsPage && repeat is { } band && index > band.First && !repeatSubtracted)
            {
                // Every page after the one the repeated band starts on loses the band's size
                // from its capacity, once and for all: the band is reprinted on each of them.
                pageSize -= axis.TotalPrintedSize(band.First, band.Last).Twips;
                pageSize = Math.Max(1, pageSize);

                // Calc's own loop steps *past* the repeated band while clearing the breaks
                // inside it, so those entries are never measured against the page either. It
                // reads like an off-by-one and is what the reference renderer does, so it is
                // reproduced rather than corrected (table5.cxx:180).
                while (index <= band.Last) breaks.Remove(++index);

                repeatSubtracted = true;
            }

            running += size;
        }

        return breaks;
    }

    /// <summary>
    /// Turns a set of breaks into the last index of each band.
    /// </summary>
    /// <remarks>
    /// The visibility bookkeeping is the whole point of this being a second pass. A break in
    /// the middle of a hidden block must not open a page — the page would print nothing — so a
    /// band closes only once something visible has been seen since the last one. Several breaks
    /// inside one hidden block therefore collapse to none, which is what the comment in
    /// <c>PrintPageRanges::calculate</c> means by "of several breaks in a hidden area, only one
    /// counts".
    /// </remarks>
    private static List<int> Bands(SheetAxis axis, int first, int last, HashSet<int> breaks)
    {
        List<int> ends = [];
        bool sawVisible = false;

        for (int index = first; index <= last; index++)
        {
            if (index > first && sawVisible && breaks.Contains(index))
            {
                ends.Add(index - 1);
                sawVisible = false;
            }

            if (!axis.IsHidden(index)) sawVisible = true;
        }

        if (sawVisible) ends.Add(last);
        return ends;
    }
}
