namespace Paperless.Spreadsheets.Layout;

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
/// <strong>Only a cell's own format extends anything here.</strong> A run of columns, a row style
/// or the sheet default covers every row to the sheet's end, which is a run of far more than
/// eighty-four equal rows and therefore the first thing Calc's scan stops at. Reading them would
/// reproduce the behaviour the constant exists to prevent, so they are left out — which is the
/// same answer Calc arrives at and reaches it without materialising a million rows.
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
    public static SheetRange Extend(
        SheetRange used,
        SheetFormatting formatting,
        IReadOnlyDictionary<int, int>? lastDataRowByColumn = null)
    {
        ArgumentNullException.ThrowIfNull(formatting);
        if (formatting.IsEmpty) return used;

        // The last row holding data, which is where the attribute scan starts. An invalid used
        // range means no data at all, and Calc then scans from the top of the sheet.
        int lastData = used.IsValid ? used.LastRow : -1;

        Dictionary<int, SortedList<int, SheetCellDecoration>> columns = [];
        Dictionary<int, SortedList<int, SheetCellDecoration>> whole = [];
        Dictionary<int, int> columnStart = [];
        foreach ((int row, int column, SheetCellDecoration format) in formatting.Cells)
        {
            if (!columnStart.TryGetValue(column, out int start))
                columnStart[column] = start = StartOf(column, lastData, lastDataRowByColumn);

            // The whole column, unfiltered, for the column-run comparison below: Calc's
            // `IsVisibleAttrEqual` asks about rows 0 to MaxRow and not about the scan's window.
            if (!whole.TryGetValue(column, out SortedList<int, SheetCellDecoration>? all))
                whole[column] = all = [];
            all[row] = format;

            // Calc processes the attribute run *containing* the column's last data row, so the
            // row itself is inside the scan and only the rows above it are out of it.
            if (row < start) continue;

            if (!columns.TryGetValue(column, out SortedList<int, SheetCellDecoration>? rows))
                columns[column] = rows = [];

            rows[row] = format;
        }

        SortedList<int, SheetCellDecoration> wholeRows = [];
        foreach ((int row, SheetCellDecoration format) in formatting.Rows)
        {
            if (row > lastData) wholeRows[row] = format;
        }

        int lastRow = lastData;
        int lastColumn = used.IsValid ? used.LastColumn : -1;

        foreach ((int column, SortedList<int, SheetCellDecoration> rows) in columns)
        {
            if (LastVisible(rows, columnStart[column]) is not { } reached) continue;

            // Calc widens the block to the column only when that column's own scan found
            // something inside the run limit — `bFound` gates both `nMaxX` and `nMaxY`
            // together (table1.cxx:717-722).
            if (reached > lastRow) lastRow = reached;
            if (column > lastColumn) lastColumn = column;
        }

        if (LastVisible(wholeRows, lastData) is { } byRow && byRow > lastRow) lastRow = byRow;

        int lastDataColumn = used.IsValid ? used.LastColumn : -1;
        lastColumn = StopAtEqualColumns(lastColumn, lastDataColumn, whole, columns, columnStart);

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
        Dictionary<int, SortedList<int, SheetCellDecoration>> whole,
        Dictionary<int, SortedList<int, SheetCellDecoration>> columns,
        Dictionary<int, int> columnStart)
    {
        if (lastColumn <= lastDataColumn) return lastColumn;

        for (int start = lastDataColumn + 1; start <= lastColumn;)
        {
            int end = start;
            while (end < lastColumn && SameColumn(whole, start, end + 1)) end++;

            if (end + 1 - start < EqualColumnsStop)
            {
                start = end + 1;
                continue;
            }

            int cut = start - 1;
            while (cut > lastDataColumn
                   && (!columns.TryGetValue(cut, out SortedList<int, SheetCellDecoration>? rows)
                       || LastVisible(rows, columnStart[cut]) is null))
            {
                cut--;
            }

            return cut;
        }

        // The columns past the last formatted one are equal to one another for ever, so a walk
        // that reaches the end has found a run without end and stops there.
        return lastColumn;
    }

    /// <summary>Whether two columns paint the same thing on every row.</summary>
    /// <remarks><c>ScAttrArray::IsVisibleEqual</c> over rows 0 to <c>MaxRow</c>.</remarks>
    private static bool SameColumn(
        Dictionary<int, SortedList<int, SheetCellDecoration>> whole, int left, int right)
    {
        whole.TryGetValue(left, out SortedList<int, SheetCellDecoration>? a);
        whole.TryGetValue(right, out SortedList<int, SheetCellDecoration>? b);

        if (a is null || b is null) return a is null && b is null;
        if (a.Count != b.Count) return false;

        for (int at = 0; at < a.Count; at++)
        {
            if (a.Keys[at] != b.Keys[at] || a.Values[at] != b.Values[at]) return false;
        }

        return true;
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
    /// Rows past the last stated one are unformatted for ever, which is a run without end and is
    /// what terminates the walk.
    /// </para>
    /// </remarks>
    private static int? LastVisible(SortedList<int, SheetCellDecoration> rows, int lastData)
    {
        if (rows.Count == 0) return null;

        int? found = null;
        int at = 0;

        while (at < rows.Count)
        {
            int start = rows.Keys[at];

            // The unformatted stretch between the previous run and this one is a run of its own.
            int gapFrom = at == 0 ? lastData + 1 : rows.Keys[at - 1] + 1;
            if (start - gapFrom >= VisibleAttributeStop) return found;

            // How far this run of visually equal, consecutive rows reaches.
            int end = at;
            while (end + 1 < rows.Count
                   && rows.Keys[end + 1] == rows.Keys[end] + 1
                   && rows.Values[end + 1] == rows.Values[end])
            {
                end++;
            }

            // Calc measures a run from the row after the last data row, not from where the run
            // itself begins: `if (nAttrStartRow <= nLastData) nAttrStartRow = nLastData + 1`
            // (attarray.cxx:1961-1962). Only the first run can start that high up, and for a
            // run that is nothing but the last data row the sum is zero — which is how a column
            // whose formatting begins on the row Calc calls its last data row is kept at all.
            if (rows.Keys[end] + 1 - Math.Max(start, lastData + 1) >= VisibleAttributeStop)
                return found;

            if (!rows.Values[end].IsNone) found = rows.Keys[end];

            at = end + 1;
        }

        return found;
    }
}
