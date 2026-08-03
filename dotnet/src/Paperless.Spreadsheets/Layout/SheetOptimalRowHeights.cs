using Paperless.Core.Extraction;
using Paperless.Core.Units;

namespace Paperless.Spreadsheets.Layout;

/// <summary>
/// Recomputes the height of every row whose file states one only as a hint.
/// </summary>
/// <remarks>
/// <para>
/// A row height in a spreadsheet file is two different things wearing one attribute. When the
/// file also says the user chose it — SpreadsheetML's <c>customHeight</c>, BIFF's <c>fUnsynced</c>,
/// ODF's <c>style:use-optimal-row-height="false"</c> — it is a statement and is honoured. When it
/// does not, it is the writing application's own measurement of the row's content, and Calc
/// discards it and measures the content again before anything is drawn. All three import filters
/// do this and each does it in its own place:
/// <c>WorkbookGlobals::finalize</c> for SpreadsheetML
/// (<c>sc/source/filter/oox/workbookhelper.cxx:659</c>), <c>ImportExcel::AdjustRowHeight</c> for
/// BIFF (<c>sc/source/filter/excel/impop.cxx:1285</c>) and <c>ScXMLImport</c>'s recalc ranges for
/// ODF (<c>sc/source/filter/xml/xmlimprt.cxx:1438</c>). The stated height survives only as the
/// starting value.
/// </para>
/// <para>
/// <strong>The common case is arithmetic, not measurement, and that is what makes this shippable.</strong>
/// <c>ScColumn::GetOptimalHeight</c> (<c>sc/source/core/data/column2.cxx:898-1100</c>) splits every
/// cell into two kinds. A cell that neither wraps, rotates, stacks nor holds more than one line is
/// <em>standard only</em>, and its row asks for <c>lcl_GetAttribHeight</c>
/// (<c>column2.cxx:866-892</c>): the font's <em>size</em> times 1.18, plus the cell's top and
/// bottom margins, less 23 twips, floored at the sheet's minimum. No glyph is measured and no
/// device is involved, so it can be reproduced exactly. Anything else is measured through
/// <c>ScColumn::GetNeededSize</c> against a reference device, and that measurement is
/// demonstrably coarser than the one Calc draws with — the module's TODO records five probe
/// documents where reproducing the formula with an accurate measurement lands 5.8% out.
/// </para>
/// <para>
/// So the split here is Calc's own: a row of standard-only cells is recomputed, and a row holding
/// anything that would go through <c>GetNeededSize</c> takes the larger of the arithmetic answer
/// and the height its file already states. That second rule is not a fudge — the arithmetic answer
/// really is a lower bound in Calc too, because <c>bStdAllowed</c> stays true for a wrapping cell
/// and its attribute height is written into the array before the per-cell measurement is compared
/// against it. It means a row we cannot measure never gets shorter than the writer said, which is
/// the direction that cannot lose text.
/// </para>
/// <para>
/// Measured on <c>National-Reports.xlsx</c>, whose 117 rows state <c>ht</c> and none states
/// <c>customHeight</c>: LibreOffice's own flat-ODF export gives <c>0.2083in</c> — 300 twips — for
/// every single-line row, and this computes 300 twips for each of them from a 12 pt font
/// (<c>trunc(240 × 1.18) = 283</c>, plus 40 twips of margin, less 23).
/// </para>
/// </remarks>
internal static class SheetOptimalRowHeights
{
    /// <summary>
    /// Calc's own fudge between a font's size and the row it needs.
    /// </summary>
    /// <remarks><c>nHeight *= 1.18</c>, <c>sc/source/core/data/column2.cxx:869</c>.</remarks>
    private const double FontHeightFactor = 1.18;

    /// <summary>
    /// The twips a row is allowed to be shorter than its font plus its margins.
    /// </summary>
    /// <remarks>
    /// <c>STD_ROWHEIGHT_DIFF</c>, <c>sc/inc/global.hxx:116</c>, whose comment states the whole
    /// formula: "standard row height: text + margin - STD_ROWHEIGHT_DIFF".
    /// </remarks>
    private const int StandardRowHeightDifference = 23;

    /// <summary>
    /// A cell's top and bottom margins, together, in twips.
    /// </summary>
    /// <remarks>
    /// Calc's default <c>ATTR_MARGIN</c> is 20 twips on all four sides
    /// (<c>SvxMarginItem</c>'s default constructor, <c>svx/source/items/algitem.cxx:123-132</c>),
    /// and none of the three readers reads a cell's vertical margins — no format states one that
    /// Paperless keeps, and the indent it does keep is horizontal. So this is a constant rather
    /// than a lookup, and it is the constant that turns a 12 pt font's 283 twips into the 300 Calc
    /// writes.
    /// </remarks>
    private const int VerticalMarginTwips = 40;

    /// <summary>
    /// The twips a pixel is worth on the device Calc measures rows against.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>ScSizeDeviceProvider</c> builds a plain <c>VirtualDevice</c> and derives its
    /// pixels-per-twip from it (<c>sc/source/ui/docshell/sizedev.cxx:43-50</c>); headless, that is
    /// 96 dpi, so a pixel is 15 twips.
    /// </para>
    /// <para>
    /// It matters because <c>lcl_pixelSizeChanged</c> (<c>sc/source/core/data/table2.cxx:3388</c>)
    /// refuses to replace a stated height with a computed one that rounds to the same pixel, so a
    /// file whose heights are already Calc's own answers keeps them exactly. That guard is applied
    /// here to all three formats although Calc applies it to two: the BIFF filter reaches rows
    /// through <c>SetOptimalHeightOnly</c>, which has no guard. Keeping it is the conservative
    /// difference — it can only leave a file's own height in place — and it is what makes a
    /// document LibreOffice wrote round-trip unchanged when the arithmetic here lands a twip or
    /// two away from Calc's.
    /// </para>
    /// </remarks>
    private const int TwipsPerPixel = 15;

    /// <summary>
    /// The grid a sheet is laid out on, with its hinted row heights re-derived from its content.
    /// </summary>
    /// <param name="sheet">The sheet, for its cells, formats and merges.</param>
    /// <param name="grid">Its geometry as the file states it.</param>
    internal static SheetGrid Apply(SheetLayout sheet, SheetGrid grid)
    {
        SheetRange range = SheetDecorationArea.Extend(sheet.UsedRange, sheet.Formatting);
        if (!range.IsValid) return grid;

        int lastRow = range.LastRow;
        int firstColumn = range.FirstColumn;
        int lastColumn = range.LastColumn;
        if (lastRow < 0 || lastColumn < firstColumn) return grid;

        SheetCellFormats formats = sheet.Formats;

        // The columns a cell states nothing about resolve to the column's format, then to the
        // sheet's. Neither depends on the row, so both are folded once rather than per row.
        int baseline = AttributeHeight(formats.SheetDefault);
        foreach (SheetCellFormat column in formats.ColumnDefaults(firstColumn, lastColumn))
            baseline = Math.Max(baseline, AttributeHeight(column));

        Dictionary<int, RowState> rows = CollectRows(sheet, formats, range);

        SheetAxis axis = grid.Rows;
        int minimum = (int)grid.OptimalMinimumRowHeight.Twips;
        Dictionary<int, int> changes = [];
        int index = 0;

        while (index <= lastRow)
        {
            // A height the user chose is honoured, and it also ends the run Calc would have
            // grouped: `SetOptimalHeightsToRows` flushes its pending range at a manual row
            // (`sc/source/core/data/table1.cxx:221-226`).
            if (!axis.IsOptimalSize(index)) { index++; continue; }

            int height = Optimal(rows, index, baseline, minimum, axis);
            int last = index;
            while (last + 1 <= lastRow && axis.IsOptimalSize(last + 1)
                   && Optimal(rows, last + 1, baseline, minimum, axis) == height)
            {
                last++;
            }

            // Calc replaces a run's heights only when the new one is at least a pixel away from
            // one of them, and then replaces all of them.
            bool changed = false;
            for (int row = index; row <= last && !changed; row++)
            {
                long stated = axis.SizeAt(row).Twips;
                changed = stated != height && stated / TwipsPerPixel != height / TwipsPerPixel;
            }

            if (changed)
                for (int row = index; row <= last; row++) changes[row] = height;

            index = last + 1;
        }

        return changes.Count == 0 ? grid : grid with { Rows = Rebuild(axis, changes, lastRow) };
    }

    /// <summary>
    /// The axis again with the recomputed rows replaced and everything else exactly as stated.
    /// </summary>
    /// <remarks>
    /// Built in one ordered pass and handed to <see cref="SheetAxis.FromOrdered"/> rather than to
    /// the normalising constructor, whose cost is quadratic in the run count — and a sheet stating
    /// a height for each of ten thousand rows has ten thousand runs before this touches it.
    /// </remarks>
    private static SheetAxis Rebuild(SheetAxis axis, Dictionary<int, int> changes, int lastRow)
    {
        List<SheetSizeRun> runs = [];

        for (int row = 0; row <= lastRow; row++)
        {
            bool hidden = axis.IsHidden(row);
            bool optimal = changes.ContainsKey(row) || axis.IsOptimalSize(row);
            Length size = changes.TryGetValue(row, out int height)
                ? Length.FromTwips(height)
                : axis.SizeAt(row);

            // A row that is in every way the axis default needs no run of its own.
            if (!hidden && optimal && size == axis.DefaultSize) continue;

            if (runs.Count > 0 && runs[^1].Last + 1 == row && runs[^1].Size == size
                && runs[^1].IsHidden == hidden && runs[^1].IsOptimalSize == optimal)
            {
                runs[^1] = runs[^1] with { Last = row };
                continue;
            }

            runs.Add(new SheetSizeRun(row, row, size, hidden, optimal));
        }

        // Everything past the sheet's content keeps whatever the file said, which is also where
        // Calc stops: its SpreadsheetML path recomputes only the used rows
        // (`UpdateAllRowHeights(/*bOnlyUsedRows=*/true)`, `workbookhelper.cxx:659`).
        foreach (SheetSizeRun run in axis.Runs)
        {
            if (run.Last <= lastRow) continue;
            runs.Add(run.First <= lastRow ? run with { First = lastRow + 1 } : run);
        }

        return SheetAxis.FromOrdered(axis.DefaultSize, runs);
    }

    /// <summary>The height one row asks for, in twips.</summary>
    private static int Optimal(
        Dictionary<int, RowState> rows, int row, int baseline, int minimum, SheetAxis axis)
    {
        int height = baseline;
        bool measured = false;

        if (rows.TryGetValue(row, out RowState state))
        {
            height = Math.Max(state.Attribute, state.CoversEveryColumn ? 0 : baseline);
            measured = state.NeedsMeasurement;
        }

        // A row Calc would have measured takes the larger of the arithmetic lower bound and what
        // its file already states, so it can only be at least as tall as the writer made it.
        if (measured) height = Math.Max(height, (int)axis.SizeAt(row).Twips);

        return Math.Max(height, minimum);
    }

    /// <summary>What one row's own cells and row format contribute.</summary>
    /// <param name="Attribute">The tallest arithmetic height any of them asks for.</param>
    /// <param name="CoversEveryColumn">
    /// True when the row states a format for every column in range, so nothing falls through to a
    /// column or sheet default.
    /// </param>
    /// <param name="NeedsMeasurement">
    /// True when one of its cells would have gone through <c>ScColumn::GetNeededSize</c>.
    /// </param>
    private readonly record struct RowState(int Attribute, bool CoversEveryColumn, bool NeedsMeasurement);

    private static Dictionary<int, RowState> CollectRows(
        SheetLayout sheet, SheetCellFormats formats, SheetRange range)
    {
        Dictionary<int, RowState> rows = [];
        Dictionary<int, int> stated = [];

        void Contribute(int row, SheetCellFormat format, bool needsMeasurement)
        {
            rows.TryGetValue(row, out RowState state);
            rows[row] = state with
            {
                Attribute = Math.Max(state.Attribute, format.IsStacked ? 0 : AttributeHeight(format)),
                NeedsMeasurement = state.NeedsMeasurement || needsMeasurement,
            };
        }

        IReadOnlyList<SheetRange> merges = sheet.MergedRanges;

        foreach ((int row, int column, SheetCellFormat format) in formats.Cells)
        {
            if (row < 0 || row > range.LastRow) continue;
            if (column < range.FirstColumn || column > range.LastColumn) continue;

            // Calc skips a cell that a merge covers and a merge anchor that spans rows, and takes
            // only the anchor of a purely horizontal one — `ScColumn::GetOptimalHeight`,
            // `sc/source/core/data/column2.cxx:917-925`.
            if (IsExcludedByMerge(merges, row, column)) continue;

            Contribute(row, format, false);
            stated[row] = stated.GetValueOrDefault(row) + 1;
        }

        int columns = range.LastColumn - range.FirstColumn + 1;
        foreach ((int row, int count) in stated)
        {
            if (count < columns) continue;
            rows[row] = rows[row] with { CoversEveryColumn = true };
        }

        for (int row = 0; row <= range.LastRow; row++)
        {
            if (formats.RowDefault(row) is { } format) Contribute(row, format, false);
        }

        // Only a cell that holds something is measured: `GetNeededSize` returns zero for an empty
        // one before it looks at the pattern (`column2.cxx:100-103`), so a wrap-formatted blank
        // does not make its row unmeasurable.
        foreach (ContentTableRow tableRow in (sheet.Cells?.Children ?? []).OfType<ContentTableRow>())
        {
            foreach (ContentTableCell cell in tableRow.Children.OfType<ContentTableCell>())
            {
                if (cell.Row < 0 || cell.Row > range.LastRow) continue;
                if (cell.Column < range.FirstColumn || cell.Column > range.LastColumn) continue;
                if (IsExcludedByMerge(merges, cell.Row, cell.Column)) continue;

                string text = cell.GetText();
                if (text.Length == 0) continue;

                SheetCellFormat format = formats.At(cell.Row, cell.Column);

                // The same wrap decision the drawing path makes, so that a row is measured exactly
                // when its text will be broken — including Calc's rule that a plain number never
                // breaks however the cell is formatted.
                bool needsMeasurement =
                    format.IsStacked
                    || format.RotationDegrees != 0
                    || SheetTextLayout.Breaks(format, cell.Value is not null and not string)
                    || text.AsSpan().IndexOfAny('\n', '\r') >= 0
                    || sheet.RichText.At(cell.Row, cell.Column, text) is not null;

                if (needsMeasurement) Contribute(cell.Row, format, true);
            }
        }

        return rows;
    }

    private static bool IsExcludedByMerge(IReadOnlyList<SheetRange> merges, int row, int column)
    {
        foreach (SheetRange merge in merges)
        {
            if (row < merge.FirstRow || row > merge.LastRow) continue;
            if (column < merge.FirstColumn || column > merge.LastColumn) continue;

            // The anchor of a merge one row tall is the one cell Calc still considers.
            bool anchor = row == merge.FirstRow && column == merge.FirstColumn;
            if (anchor && merge.FirstRow == merge.LastRow) continue;
            return true;
        }

        return false;
    }

    /// <summary>
    /// The height a cell's font asks for without measuring anything.
    /// </summary>
    /// <remarks>
    /// <c>lcl_GetAttribHeight</c>, <c>sc/source/core/data/column2.cxx:866-892</c>. The truncation
    /// is Calc's: the running value is a <c>sal_uInt16</c>, so <c>nHeight *= 1.18</c> throws the
    /// fraction away before the margins are added — 240 twips becomes 283 rather than 283.2, and a
    /// 12 pt row lands on 300 twips exactly.
    /// </remarks>
    private static int AttributeHeight(SheetCellFormat format)
    {
        int height = (int)(format.FontSize.Twips * FontHeightFactor);
        height += VerticalMarginTwips;
        if (height > StandardRowHeightDifference) height -= StandardRowHeightDifference;
        return height;
    }
}
