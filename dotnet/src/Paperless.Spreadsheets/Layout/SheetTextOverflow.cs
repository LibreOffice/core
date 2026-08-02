using Paperless.Core.Extraction;
using Paperless.Core.Units;

namespace Paperless.Spreadsheets.Layout;

/// <summary>
/// Widens a sheet's print area to cover text that overflows its column.
/// </summary>
/// <remarks>
/// <para>
/// A string too wide for its cell spills into the empty cells to its right, and Calc prints all
/// of it — so the printed area is wider than the area that holds cells. <c>AdjustPrintArea</c>
/// calls <c>ScTable::ExtendPrintArea</c> for exactly this
/// (<c>sc/source/core/data/table1.cxx:2127</c>, and the per-cell rule in
/// <c>MaybeAddExtraColumn</c> at <c>:2217</c>), and without it a sheet of long strings comes out
/// a page narrower than the reference.
/// </para>
/// <para>
/// It is worth being clear that this is the one place pagination depends on measuring text. Row
/// heights and column widths are stated in the file, so nothing else about where the pages fall
/// needs a font — which is why an inexact measurement here is survivable: the extension is by
/// <em>whole columns</em>, so being within one column's width of LibreOffice's answer gives the
/// same page. Measured on <c>xls-features.xls</c>, whose Strings sheet holds 183-character
/// strings in a 64-point column: the extension runs to column N either way, and the sheet takes
/// two pages.
/// </para>
/// <para>
/// Four of Calc's conditions are reproduced and matter. Only a cell holding <em>text</em>
/// overflows — a number too wide shows <c>###</c> instead, which takes no extra room. A cell that
/// <em>wraps</em> never overflows at all, so it never widens anything. Overflow
/// stops at the first non-empty cell to the right, so a value in the next column truncates the
/// string rather than being written over. And a right-aligned cell overflows to the left, which
/// costs no columns at the right-hand end.
/// </para>
/// <para>
/// The wrap condition is the one that dominated the corpus and it is worth stating why. A
/// wrapping column is usually a <em>wide</em> column of long prose, so the strings in it measure
/// to thousands of points; extending the print area by that much adds column after column of
/// empty sheet, each of which becomes a band of pages that hold nothing. Measured on
/// <c>AFS-400_Contacts.xlsx</c>, seven columns of contacts with three wrapping ones: 340 pages
/// against LibreOffice's 48, of which 289 of ours were blank and none of LibreOffice's were.
/// </para>
/// </remarks>
internal static class SheetTextOverflow
{
    /// <summary>
    /// How many cells will be measured before the walk gives up.
    /// </summary>
    /// <remarks>
    /// A guard against a hostile sheet rather than a tuning knob: measuring is shaping, and a
    /// million-row sheet of distinct strings would otherwise shape a million of them to decide a
    /// page boundary. Reaching the limit leaves the print area as wide as it had got, which
    /// under-reports rather than mis-reports.
    /// </remarks>
    private const int MeasurementBudget = 20_000;

    /// <summary>The cell text margin either side, which counts towards the width needed.</summary>
    /// <remarks><c>ATTR_MARGIN</c>'s default of 20 twips, left and right.</remarks>
    private static readonly Length CellMargins = SheetTextLayout.CellMargin * 2;

    /// <summary>The last column a sheet's contents reach, overflow included.</summary>
    /// <param name="sheet">The sheet.</param>
    /// <param name="used">The block of cells the sheet holds.</param>
    public static int ExtendedLastColumn(SheetLayout sheet, SheetRange used)
    {
        ArgumentNullException.ThrowIfNull(sheet);
        if (!used.IsValid) return used.LastColumn;

        Dictionary<(string Text, SheetCellFormat Format), Length> widths = [];
        int last = used.LastColumn;
        int measured = 0;

        foreach (ContentTableRow row in (sheet.Cells?.Children ?? []).OfType<ContentTableRow>())
        {
            if (row.Index < used.FirstRow || row.Index > used.LastRow) continue;

            foreach (ContentTableCell cell in row.Children.OfType<ContentTableCell>())
            {
                if (cell.Column < used.FirstColumn || cell.Column > used.LastColumn) continue;
                if (cell.Value is not null and not string) continue;
                if (sheet.Grid.Columns.IsHidden(cell.Column)) continue;

                string text = cell.GetText();
                if (text.Length == 0) continue;

                // Nothing overflows into an occupied cell, and checking first is what keeps a
                // dense sheet from being measured at all — the same short-circuit Calc added in
                // tdf#128873.
                if (!SheetTextLayout.IsAvailable(sheet.CellAt(row.Index, cell.Column + 1))) continue;
                if (measured++ >= MeasurementBudget) return last;

                SheetCellFormat format = sheet.Formats.At(row.Index, cell.Column);

                // A cell that breaks its text into lines needs no width beyond its own column, so
                // it never widens the print area. `ScColumn::GetNeededSize` says it in one line —
                // `if ( bWidth && bBreak ) return 0;` (sc/source/core/data/column2.cxx:226) —
                // and it is the difference between a page and a hundred: a wrapping column of
                // long strings measures to thousands of points, and extending the print area by
                // that much manufactures column band after column band of blank paper.
                // Never a value here: the walk above kept only cells holding text.
                if (SheetTextLayout.Breaks(format, isValue: false)) continue;

                if (!widths.TryGetValue((text, format), out Length width))
                {
                    width = SheetText.Measure(text, SheetFonts.For(format), format.FontSize)
                            + CellMargins + format.Indent;
                    widths[(text, format)] = width;
                }

                Length missing = width - sheet.Grid.Columns.SizeAt(cell.Column);

                // Alignment decides which way the overflow goes, and only the rightward part
                // costs columns. Calc looks at exactly this and no more: a centred cell spills
                // both ways so half is missing to the right, and a right-aligned one spills only
                // to the left, which costs nothing at the right-hand end
                // (`ScTable::MaybeAddExtraColumn`, sc/source/core/data/table1.cxx:2264-2276).
                if (missing > Length.Zero)
                {
                    if (format.Horizontal == SheetHorizontalAlignment.Centre) missing /= 2;
                    else if (format.Horizontal == SheetHorizontalAlignment.Right) missing = Length.Zero;
                }

                int at = cell.Column;
                while (missing > Length.Zero && at < SheetAddress.MaxColumn)
                {
                    if (!SheetTextLayout.IsAvailable(sheet.CellAt(row.Index, at + 1))) break;
                    at++;
                    missing -= sheet.Grid.Columns.PrintedSizeAt(at);
                }

                if (at > last) last = at;
            }
        }

        return last;
    }
}
