namespace XRay.Office.Spreadsheets.Layout;

/// <summary>
/// Widens a sheet's print area to cover the cells that are formatted but empty.
/// </summary>
/// <remarks>
/// <para>
/// A ruled-off row of blank cells prints, and a workbook of forms is mostly that. Calc reaches it
/// in a second pass over the same columns: <c>ScTable::GetPrintArea</c> finds the last row and
/// column holding <em>data</em>, and then runs the loop again headed <c>// Test attribute</c>
/// asking each column for its last <em>visible</em> attribute
/// (<c>sc/source/core/data/table1.cxx:710-724</c>). A cell counts as visibly attributed when it
/// states a background that is not transparent, any of the four border edges, a diagonal or a
/// shadow — <c>ScPatternAttr::CalcVisible</c> (<c>patattr.cxx:1584-1612</c>), which is the same
/// pair of properties <see cref="SheetCellDecoration"/> carries.
/// </para>
/// <para>
/// <strong>The scan has to stop, and where it stops is the whole difficulty.</strong> Formatting
/// runs to the end of the sheet far more often than data does — a column style, a banded fill, a
/// default cell style — so a scan that simply took the furthest formatted cell would put the print
/// area at row 1048576 on ordinary workbooks. Calc's rule is <c>SC_VISATTR_STOP</c>: below the last
/// row holding data, attribute runs are followed only while each run of visually equal rows is
/// shorter than <strong>84</strong> rows, and the first run that long ends the scan
/// (<c>ScAttrArray::GetLastVisibleAttr</c>, <c>attarray.cxx:1922-1975</c>, and its <c>#i30830#</c>
/// note). Eighty-four is two default pages' worth and the comment says as much: "as good as any
/// number".
/// </para>
/// <para>
/// <strong>A whole-column format is not exempt from the scan, and reading it as exempt cost a
/// page.</strong> The reasoning that used to stand here was that a run of columns, a row style or
/// the sheet default covers every row to the sheet's end, so it is one run far longer than
/// eighty-four and the first thing the scan stops at. That is true only while nothing
/// <em>splits</em> it. A single <c>&lt;row&gt;</c> with <c>customFormat</c> cuts the column's
/// attribute array in three, and the piece above the split is then a short, visible run that the
/// scan takes. Measured on <c>CSJU List of Recipients of funds 2013-2020.xlsx</c>, whose columns
/// E and F carry a solid white <c>&lt;col style&gt;</c> and whose row 13 states a format: Calc's
/// print area reaches F, ours reached D, and the two extra columns are 21724 twips against 19615
/// — enough that the fit-to-width search settles on 46% where we settled on 52%, which is nine
/// pages against eight.
/// </para>
/// <para>
/// <strong>What bounds the scan sideways is not the formatting but which columns exist.</strong>
/// <c>ScTable::GetPrintArea</c> loops <c>for (i = 0; i &lt; aCol.size(); i++)</c>, and
/// <c>aCol</c> holds only the columns that have been <em>allocated</em>. A format applied to a
/// range that reaches the sheet's last column allocates nothing: <c>ScTable::ApplyPatternArea</c>
/// takes <c>maxCol = max(nStartCol, aCol.size()) - 1</c> and writes the rest into
/// <c>aDefaultColData</c> (<c>sc/source/core/data/table2.cxx:2988-2998</c>). So the closing
/// <c>&lt;col min="7" max="16384"&gt;</c> that Excel writes on nearly every sheet materialises no
/// column at all, while <c>&lt;col min="6" max="6"&gt;</c> materialises one — which is why the
/// same workbook stops at F and not at XFD. That is reproduced here as
/// <see cref="AllocatedLastColumn"/>, and it is what keeps a whole-column fill from widening
/// every sheet that states one by sixteen thousand columns.
/// </para>
/// <para>
/// <strong>The scan is asked per column and starts per column.</strong>
/// <c>ScColumn::GetLastVisibleAttr</c> passes that column's <em>own</em> <c>GetLastDataPos()</c>,
/// "0 if none" (<c>sc/inc/column.hxx:892-897</c>), so a column holding no data is scanned from
/// the top of the sheet rather than from wherever the sheet's data ends. Starting every column
/// at the sheet's last data row instead loses the columns whose only formatting is above it —
/// measured on <c>Computer and Software Services_50 State Comparison.xlsx</c>, whose columns I
/// to O carry a fill on all 129 rows and no data at all: the sheet's data stops at row 42, the
/// fill below it is one run of 112 equal rows and stops the scan, and the one run short enough
/// to be taken is the header row above the data. The print area therefore ended at column H and
/// Calc's reaches O, which is a whole third column band — 24 pages against 26.
/// </para>
/// <para>
/// Measured on <c>e-pass-contact-details-template.xlsx</c>, a form whose only values are its nine
/// column headings and whose row 14 is a ruled box across two of them: the print area stopped at
/// row 1, so the box was never placed on a page and never drawn, and the second page differed from
/// LibreOffice's by 0.21% of its ink with no page-count or word-count difference to explain it.
/// </para>
/// </remarks>
internal static class SheetDecorationArea
{
    /// <summary>
    /// How far past the last visible thing the scan looks before giving up.
    /// </summary>
    /// <remarks><c>SC_VISATTR_STOP</c>, <c>sc/source/core/data/attarray.cxx:1921</c>.</remarks>
    public const int VisibleAttributeStop = 84;

    /// <summary>
    /// The used range, widened to cover the formatted cells beyond it.
    /// </summary>
    /// <param name="used">The block of cells the sheet holds, which may be invalid.</param>
    /// <param name="formatting">The sheet's fills and borders.</param>
    /// <param name="lastDataRowByColumn">
    /// The last row holding data in each column that holds any, as
    /// <see cref="SheetLayout.LastDataRowByColumn"/> supplies it. Null falls back to the sheet's
    /// own last data row for every column, which is the narrower answer and the one this scan
    /// gave before the per-column start was implemented.
    /// </param>
    /// <param name="allocatedLastColumn">
    /// The last column the file materialises, from <see cref="AllocatedLastColumn"/>. Negative
    /// leaves the scan bounded by the columns that state a format of their own, which is what it
    /// was bounded by before the sideways limit was measured.
    /// </param>
    public static SheetRange Extend(
        SheetRange used,
        SheetFormatting formatting,
        IReadOnlyDictionary<int, int>? lastDataRowByColumn = null,
        int allocatedLastColumn = -1)
    {
        ArgumentNullException.ThrowIfNull(formatting);
        if (formatting.IsEmpty) return used;

        // The last row holding data, which is where the attribute scan starts. An invalid used
        // range means no data at all, and Calc then scans from the top of the sheet.
        int lastData = used.IsValid ? used.LastRow : -1;
        int lastDataColumn = used.IsValid ? used.LastColumn : -1;

        // The whole column, unfiltered: Calc's `IsVisibleAttrEqual` asks about rows 0 to MaxRow
        // and not about the scan's window, and the run walk below starts at the run *containing*
        // the column's last data row rather than at the first entry below it.
        Dictionary<int, SortedList<int, SheetCellDecoration>> points = [];
        foreach ((int row, int column, SheetCellDecoration format) in formatting.Cells)
        {
            if (row < 0 || column < 0) continue;
            if (!points.TryGetValue(column, out SortedList<int, SheetCellDecoration>? all))
                points[column] = all = [];
            all[row] = format;
        }

        // A repeat is held as a rectangle rather than as its cells, and it reaches this scan as
        // one. That is the shape the scan wants in any case: Calc measures the *length* of each
        // run of visually equal rows, so a block of a million identical ones is one measurement
        // rather than a million entries to re-derive it from.
        List<(int FirstRow, int LastRow, int FirstColumn, int LastColumn, SheetCellDecoration Format)>
            blocks = [];
        int blockAllocated = -1;
        foreach ((int blockFirstRow, int blockLastRow, int blockFirstColumn, int blockLastColumn,
                  SheetCellDecoration blockFormat) in formatting.CellBlocks)
        {
            if (blockFirstRow < 0 || blockFirstColumn < 0) continue;
            if (blockLastRow < blockFirstRow || blockLastColumn < blockFirstColumn) continue;

            blocks.Add((blockFirstRow, blockLastRow, blockFirstColumn, blockLastColumn, blockFormat));

            // The same clause `AllocatedLastColumn` reads for a column run: a range that stops
            // short of the sheet's last column materialises the whole of it, and one that reaches
            // it materialises nothing past where it starts (`ScTable::ApplyPatternArea`,
            // `sc/source/core/data/table2.cxx:2988-2998`). A sheet's padding always reaches it.
            int reaches = blockLastColumn < SheetAddress.MaxColumn
                ? blockLastColumn
                : blockFirstColumn - 1;
            if (reaches > blockAllocated) blockAllocated = reaches;
        }

        SortedList<int, SheetCellDecoration> rowDefaults = [];
        SortedList<int, SheetCellDecoration> wholeRows = [];
        foreach ((int row, SheetCellDecoration format) in formatting.Rows)
        {
            if (row < 0) continue;
            rowDefaults[row] = format;
            if (row > lastData) wholeRows[row] = format;
        }

        int lastRow = lastData;
        int lastColumn = used.IsValid ? used.LastColumn : -1;
        int allocated = Math.Max(
            allocatedLastColumn,
            Math.Max(LastStatedColumn(lastDataColumn, points), blockAllocated));

        Dictionary<int, List<(int Start, int End, SheetCellDecoration Format)>> whole =
            Spans(points, blocks, allocated);

        // A column with no entries of its own is the same scan for every column sharing a
        // background, and a sheet can carry thousands of them.
        Dictionary<(SheetCellDecoration Base, int Start), int?> byBase = [];

        int? Scan(int column)
        {
            int start = StartOf(column, lastData, lastDataRowByColumn);
            SheetCellDecoration background = formatting.ColumnDefault(column);
            whole.TryGetValue(column, out List<(int Start, int End, SheetCellDecoration Format)>? cells);

            if (cells is not null) return LastVisible(Runs(cells, rowDefaults, background), start);

            if (byBase.TryGetValue((background, start), out int? cached)) return cached;

            int? answer = LastVisible(Runs(null, rowDefaults, background), start);
            byBase[(background, start)] = answer;
            return answer;
        }

        for (int column = 0; column <= allocated; column++)
        {
            if (Scan(column) is not { } reached) continue;

            // Calc widens the block to the column only when that column's own scan found
            // something inside the run limit — `bFound` gates both `nMaxX` and `nMaxY`
            // together (table1.cxx:717-722).
            if (reached > lastRow) lastRow = reached;
            if (column > lastColumn) lastColumn = column;
        }

        List<(int Start, int End, SheetCellDecoration Format)> belowData = [];
        for (int at = 0; at < wholeRows.Count; at++)
            belowData.Add((wholeRows.Keys[at], wholeRows.Keys[at], wholeRows.Values[at]));

        if (LastVisible(Runs(belowData, [], SheetCellDecoration.None), lastData) is { } byRow
            && byRow > lastRow)
        {
            lastRow = byRow;
        }

        // Deliberately *not* `lastColumn = max(lastColumn, allocated)`. Being materialised is
        // what lets a column be looked at; it is not what puts it in the block — `bFound` in
        // `ScTable::GetPrintArea` is set by the data loop and by `GetLastVisibleAttr`, and by
        // nothing else. Extending to the allocated column outright was measured over the whole
        // track and is refuted: it fixes `CSJU` and breaks `fy20-may20-sep20.xlsx` (a sheet with
        // no data at all, allocated to column E, which then prints a page it should not) and
        // `fm-provider-service-measures.xlsx` (a sheet whose data stops at column C and whose
        // closed run reaches T, which then fits to a smaller zoom and loses two pages).
        lastColumn = StopAtEqualColumns(lastColumn, lastDataColumn, whole, formatting, Scan);

        if (lastRow <= lastData && lastColumn <= lastDataColumn) return used;

        return used.IsValid
            ? used with
            {
                LastRow = Math.Max(used.LastRow, lastRow),
                LastColumn = Math.Max(used.LastColumn, lastColumn),
            }
            : new SheetRange(0, 0, Math.Max(lastColumn, 0), Math.Max(lastRow, 0));
    }

    /// <summary>
    /// How many equally-formatted columns behind the data end the sideways scan.
    /// </summary>
    /// <remarks><c>SC_COLUMNS_STOP</c>, <c>sc/source/core/data/table1.cxx:655</c>.</remarks>
    public const int EqualColumnsStop = 30;

    /// <summary>
    /// Cuts the block back before the first run of equally-formatted columns behind the data.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The sideways twin of <see cref="VisibleAttributeStop"/> and needed for the same reason.
    /// A ruled grid does not stop where the data does, so the attribute pass can widen a sheet
    /// by a hundred columns of identical empty ruling; Calc walks right from the last data
    /// column grouping columns that are <em>visually equal over every row</em>, and the first
    /// group of <see cref="EqualColumnsStop"/> or more ends the block before it
    /// (<c>table1.cxx:737-757</c>). It then walks back over any column whose own scan found
    /// nothing, so the block ends on a column that actually paints something.
    /// </para>
    /// <para>
    /// The run past the last formatted column is unbounded and equal to itself, which is what
    /// stops the walk on an ordinary sheet — so on those this changes nothing, and only a sheet
    /// with thirty or more identically ruled empty columns feels it. Measured on
    /// <c>environment-edb-docs-edb-emissions-databank.xls</c>, whose <c>ICAO databank</c> sheet
    /// holds data to column 104 and formatting to column 228: without this the block keeps all
    /// 124 of them, which is nine extra column bands and nine extra pages.
    /// </para>
    /// <para>
    /// The cut is sideways only. <c>nMaxY</c> was set by the pass that found those columns and
    /// Calc does not undo it, so a row a dropped column reached stays inside the block.
    /// </para>
    /// </remarks>
    private static int StopAtEqualColumns(
        int lastColumn,
        int lastDataColumn,
        Dictionary<int, List<(int Start, int End, SheetCellDecoration Format)>> whole,
        SheetFormatting formatting,
        Func<int, int?> scan)
    {
        if (lastColumn <= lastDataColumn) return lastColumn;

        for (int start = lastDataColumn + 1; start <= lastColumn;)
        {
            int end = start;
            while (end < lastColumn && SameColumn(whole, formatting, start, end + 1)) end++;

            if (end + 1 - start < EqualColumnsStop)
            {
                start = end + 1;
                continue;
            }

            int cut = start - 1;
            while (cut > lastDataColumn && scan(cut) is null) cut--;

            return cut;
        }

        // The columns past the last formatted one are equal to one another for ever, so a walk
        // that reaches the end has found a run without end and stops there.
        return lastColumn;
    }

    /// <summary>Whether two columns paint the same thing on every row.</summary>
    /// <remarks>
    /// <c>ScAttrArray::IsVisibleEqual</c> over rows 0 to <c>MaxRow</c>. The column's own
    /// background counts: two columns with no cells of their own still differ when one carries a
    /// <c>&lt;col style&gt;</c> that paints and the other does not.
    /// </remarks>
    private static bool SameColumn(
        Dictionary<int, List<(int Start, int End, SheetCellDecoration Format)>> whole,
        SheetFormatting formatting,
        int left,
        int right)
    {
        if (formatting.ColumnDefault(left) != formatting.ColumnDefault(right)) return false;

        whole.TryGetValue(left, out List<(int Start, int End, SheetCellDecoration Format)>? a);
        whole.TryGetValue(right, out List<(int Start, int End, SheetCellDecoration Format)>? b);

        if (a is null || b is null) return a is null && b is null;
        if (a.Count != b.Count) return false;

        for (int at = 0; at < a.Count; at++)
        {
            if (a[at] != b[at]) return false;
        }

        return true;
    }

    /// <summary>
    /// The last column a sheet's stated column runs cause to be materialised.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>ScTable::GetPrintArea</c> loops over <c>aCol</c>, which holds the sheet's
    /// <em>allocated</em> columns, so this is the right-hand limit of the whole scan and it has
    /// nothing to do with what those columns paint. A width or a format applied to a range that
    /// stops short of the sheet's last column materialises the whole range; one that reaches the
    /// last column materialises only up to <c>max(nStartCol, aCol.size()) - 1</c> and writes the
    /// rest into the unallocated-column default (<c>ScTable::ApplyPatternArea</c>,
    /// <c>sc/source/core/data/table2.cxx:2988-2998</c>), so the closing
    /// <c>&lt;col min="7" max="16384"&gt;</c> Excel writes on nearly every sheet materialises
    /// nothing beyond the column before it.
    /// </para>
    /// <para>
    /// <strong>Five variants of one workbook fix this rule and nothing weaker fits all five.</strong>
    /// On <c>CSJU List of Recipients of funds 2013-2020.xlsx</c>, whose sheet <c>2020</c> holds
    /// data to column D and whose <c>&lt;cols&gt;</c> are A–D, E, F and then G–XFD, LibreOffice
    /// prints eight pages where we printed nine. Rendering the reference against edited copies:
    /// merging F into the open G–XFD run gives nine pages (the print band loses F); widening F
    /// to fifty characters gives seven; adding a closed <c>&lt;col&gt;</c> at J gives seven;
    /// widening E with F merged away gives seven; and removing F's <c>style</c> while keeping
    /// its own run changes nothing. So the band's right edge follows the last <em>closed</em>
    /// run and not the formatting — the fourth of those separates E from D, and the fifth rules
    /// the column's own fill out as the cause.
    /// </para>
    /// <para>
    /// Each of those page counts is the one the fit-to-width search produces from the widened
    /// band, computed before the render: 21724 twips of columns against 22173 of page at 46%
    /// rather than 19615 against 19615 at 52%.
    /// </para>
    /// </remarks>
    /// <param name="columns">The column runs the file states, or null when it states none.</param>
    public static int AllocatedLastColumn(IReadOnlyList<SheetDigitRun>? columns)
    {
        if (columns is null) return -1;

        int last = -1;
        foreach (SheetDigitRun run in columns)
        {
            // The `first - 1` for an open run is the same clause's other half rather than a
            // guess: the call materialises up to the column before the range starts.
            int allocated = run.Last < SheetAddress.MaxColumn ? run.Last : run.First - 1;
            if (allocated > last) last = allocated;
        }

        return Math.Min(last, SheetAddress.MaxColumn);
    }

    /// <summary>
    /// One sheet's stated formats as a span list per column: the cells written one at a time,
    /// and the rectangles a repeat count wrote, folded into one sorted, disjoint list.
    /// </summary>
    /// <remarks>
    /// Spans rather than rows, because a rectangle is what ODF states and what Calc stores. The
    /// producers write disjoint runs — each cell element of each row element covers its own
    /// columns of its own rows — so the clip below is a guard rather than a rule, and where it
    /// does fire the span written first keeps the overlap.
    /// </remarks>
    /// <param name="points">The cells stated one at a time, keyed by column then row.</param>
    /// <param name="blocks">The rectangles, in the order the file wrote them.</param>
    /// <param name="allocated">The last column the scan will look at; blocks stop there.</param>
    private static Dictionary<int, List<(int Start, int End, SheetCellDecoration Format)>> Spans(
        Dictionary<int, SortedList<int, SheetCellDecoration>> points,
        List<(int FirstRow, int LastRow, int FirstColumn, int LastColumn, SheetCellDecoration Format)> blocks,
        int allocated)
    {
        Dictionary<int, List<(int Start, int End, SheetCellDecoration Format)>> whole = [];

        foreach ((int column, SortedList<int, SheetCellDecoration> rows) in points)
        {
            List<(int Start, int End, SheetCellDecoration Format)> spans = new(rows.Count);
            for (int at = 0; at < rows.Count; at++) spans.Add((rows.Keys[at], rows.Keys[at], rows.Values[at]));
            whole[column] = spans;
        }

        foreach ((int firstRow, int lastRow, int firstColumn, int lastColumn, SheetCellDecoration format)
                 in blocks)
        {
            int last = Math.Min(lastColumn, allocated);
            for (int column = firstColumn; column <= last; column++)
            {
                if (!whole.TryGetValue(column, out List<(int Start, int End, SheetCellDecoration Format)>? spans))
                    whole[column] = spans = [];
                spans.Add((firstRow, lastRow, format));
            }
        }

        foreach (List<(int Start, int End, SheetCellDecoration Format)> spans in whole.Values)
        {
            if (spans.Count < 2) continue;

            spans.Sort(static (left, right) => left.Start.CompareTo(right.Start));

            int keep = 0;
            int reached = int.MinValue;
            for (int at = 0; at < spans.Count; at++)
            {
                (int start, int end, SheetCellDecoration format) = spans[at];
                if (start <= reached) start = reached + 1;
                if (start > end) continue;

                spans[keep++] = (start, end, format);
                reached = end;
            }

            spans.RemoveRange(keep, spans.Count - keep);
        }

        return whole;
    }

    /// <summary>The last column anything states a format for, or the last holding data.</summary>
    private static int LastStatedColumn(
        int lastDataColumn, Dictionary<int, SortedList<int, SheetCellDecoration>> whole)
    {
        int last = lastDataColumn;

        foreach (int column in whole.Keys)
        {
            if (column > last) last = column;
        }

        return last;
    }

    /// <summary>
    /// One column's attribute array, as runs of rows that paint the same thing.
    /// </summary>
    /// <remarks>
    /// The whole column, from row zero to the sheet's last row, because that is what Calc holds:
    /// the column's background fills every row no cell and no row format states, and the runs it
    /// is cut into by the ones that do are what the scan measures.
    /// </remarks>
    /// <param name="cells">The spans this column states a format for, or null when it states none.</param>
    /// <param name="rowDefaults">The rows the whole sheet states a format for.</param>
    /// <param name="background">What the column itself states, or the sheet's own default.</param>
    private static List<(int Start, int End, SheetCellDecoration Format)> Runs(
        List<(int Start, int End, SheetCellDecoration Format)>? cells,
        SortedList<int, SheetCellDecoration> rowDefaults,
        SheetCellDecoration background)
    {
        List<(int Start, int End, SheetCellDecoration Format)> runs = [];

        void Add(int start, int end, SheetCellDecoration format)
        {
            if (end < start) return;

            if (runs.Count > 0 && runs[^1].Format == format && runs[^1].End + 1 == start)
                runs[^1] = (runs[^1].Start, end, format);
            else
                runs.Add((start, end, format));
        }

        int at = 0;
        int cellAt = 0;
        int rowAt = 0;

        while (at <= SheetAddress.MaxRow)
        {
            while (cells is not null && cellAt < cells.Count && cells[cellAt].End < at) cellAt++;
            while (rowAt < rowDefaults.Count && rowDefaults.Keys[rowAt] < at) rowAt++;

            int nextCell = cells is not null && cellAt < cells.Count
                ? Math.Max(cells[cellAt].Start, at)
                : int.MaxValue;
            int nextRow = rowAt < rowDefaults.Count ? rowDefaults.Keys[rowAt] : int.MaxValue;

            if (nextCell == int.MaxValue && nextRow == int.MaxValue)
            {
                Add(at, SheetAddress.MaxRow, background);
                return runs;
            }

            // A cell's own format beats its row's, which beats its column's — the order
            // SheetFormatting.At resolves in and the order all three formats write. A span wins
            // every row it covers, so a row default inside one never surfaces.
            if (nextCell <= nextRow)
            {
                Add(at, nextCell - 1, background);
                Add(nextCell, cells![cellAt].End, cells[cellAt].Format);
                at = cells[cellAt].End + 1;
            }
            else
            {
                Add(at, nextRow - 1, background);
                Add(nextRow, nextRow, rowDefaults.Values[rowAt]);
                at = nextRow + 1;
            }
        }

        return runs;
    }

    /// <summary>
    /// The row one column's attribute scan is measured from — Calc's <c>nLastData</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>ScColumn::GetLastVisibleAttr</c> passes that column's own <c>GetLastDataPos()</c>,
    /// documented as "always including notes, <strong>0 if none</strong>"
    /// (<c>sc/inc/column.hxx:892-897</c>). So a column holding no data is measured from row zero
    /// and not from the sheet's last data row, which is what lets an empty but filled column to
    /// the right of the data keep the sheet's print area — the run above the sheet's data is
    /// what the scan reads, and it never reached it before.
    /// </para>
    /// <para>
    /// Without a per-column map the sheet's own last data row stands in for every column, which
    /// is the narrower answer: every scan then starts lower down and finds less.
    /// </para>
    /// </remarks>
    private static int StartOf(
        int column, int sheetLastData, IReadOnlyDictionary<int, int>? lastDataRowByColumn)
    {
        if (lastDataRowByColumn is null) return sheetLastData;

        return lastDataRowByColumn.TryGetValue(column, out int last) ? last : 0;
    }

    /// <summary>
    /// The last visibly attributed row of one column below the data, or null when the scan found
    /// none before it stopped.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The runs are walked upwards from the row after the last data row, each run being a stretch
    /// of rows that look the same, and the first run of <see cref="VisibleAttributeStop"/> rows or
    /// more ends the scan for that column. Both kinds of run count, which is the half that decides
    /// whether this rule is usable at all: a gap of eighty-four unformatted rows stops it, and so
    /// does a block of eighty-four identically ruled ones.
    /// </para>
    /// <para>
    /// The second is the common case and the expensive one to get wrong. A sheet whose whole grid
    /// is ruled to row 1001 — <c>edb-emissions-databank v27</c>'s third sheet rules 46172 cells
    /// down to it — is one run far longer than the limit, so Calc takes nothing from it and prints
    /// 368 pages; a scan that only broke on gaps takes all of it and prints 460.
    /// </para>
    /// <para>
    /// The last run reaches the sheet's last row, so it is always far longer than the limit and is
    /// what terminates the walk — whether it paints nothing, which is the ordinary sheet, or
    /// carries a whole-column fill, which is the case this scan used to be unable to see.
    /// </para>
    /// </remarks>
    private static int? LastVisible(
        List<(int Start, int End, SheetCellDecoration Format)> runs, int lastData)
    {
        int? found = null;

        // `Search(nLastData, nPos)` — the walk starts at the run holding the column's last data
        // row, so the runs entirely above it are never measured.
        int at = 0;
        while (at < runs.Count && runs[at].End < lastData) at++;

        for (; at < runs.Count; at++)
        {
            // Calc measures a run from the row after the last data row, not from where the run
            // itself begins: `if (nAttrStartRow <= nLastData) nAttrStartRow = nLastData + 1`
            // (attarray.cxx:1961-1962). Only the first run can start that high up, and for a
            // run that is nothing but the last data row the sum is zero — which is how a column
            // whose formatting begins on the row Calc calls its last data row is kept at all.
            int start = Math.Max(runs[at].Start, lastData + 1);
            if ((long)runs[at].End + 1 - start >= VisibleAttributeStop) return found;

            if (!runs[at].Format.IsNone) found = runs[at].End;
        }

        return found;
    }
}
