using System.Globalization;
using System.Xml.Linq;
using XRay.Office.Spreadsheets.Layout;

namespace XRay.Office.Spreadsheets.Ooxml;

/// <summary>
/// The cells a chart data range must not read because they are an Excel table's totals row.
/// </summary>
/// <remarks>
/// <para>
/// <strong>The rule, and it is Excel's rather than Calc's.</strong>
/// <c>ScChart2DataSequence::BuildDataCache</c>
/// (<c>sc/source/ui/unoobj/chart2uno.cxx:2616-2632</c>) skips a cell when it is the
/// <em>last row of the range being read</em>, a database range covers it, that database range has
/// a totals row, and the database range <em>ends</em> on that row. Its own comment says why:
/// "Excel behavior: if the last row is the totals row, the data is not added to the chart. If
/// it's not the last row, the data is added like normal."
/// </para>
/// <para>
/// <strong>Only a table produces one.</strong> A SpreadsheetML <c>table</c> part becomes a
/// <em>named</em> database range with <c>TotalsRow</c> set from <c>totalsRowCount</c>
/// (<c>sc/source/filter/oox/tablebuffer.cxx:133-137</c>), and
/// <c>ScDBCollection::GetDBAtCursor</c> searches the named ranges first
/// (<c>sc/source/core/tool/dbdata.cxx:2160-2182</c>). The other two kinds it would find — the
/// sheet-local and the global anonymous ranges, which is what a plain <c>autoFilter</c> becomes —
/// never carry totals, so a table is the only thing that can hide a cell from a chart.
/// </para>
/// <para>
/// <strong>Two conditions the import bails on.</strong> <c>Table::finalizeImport</c> returns
/// before creating the database range when the table's <c>id</c> is not positive or its
/// <c>displayName</c> is empty, so such a table has no totals row as far as a chart is concerned.
/// Both are checked here, because a table part that names no range is exactly the shape a
/// hand-written or repaired file has.
/// </para>
/// <para>
/// <strong>Why this is keyed per column and not per range.</strong> LibreOffice asks
/// <c>GetDBAtCursor(nCol, nRow, …)</c> inside the column loop, so a range wider than the table
/// loses the last cell of the columns the table covers and keeps the last cell of the ones it
/// does not. No corpus document has that shape — all four hits over 946 documents are a single
/// column or a single row — but reproducing the per-column test costs nothing and guessing at it
/// would be a claim about a case nothing measures.
/// </para>
/// </remarks>
internal sealed class XlsxChartTotalsRows
{
    /// <summary>A workbook that states no totals-row table anywhere.</summary>
    public static XlsxChartTotalsRows None { get; } = new([]);

    private readonly IReadOnlyList<SheetRange> _tables;

    private XlsxChartTotalsRows(IReadOnlyList<SheetRange> tables) => _tables = tables;

    /// <summary>Reads a sheet's table parts, or <see cref="None"/> when none of them has totals.</summary>
    /// <param name="file">The open package.</param>
    /// <param name="sheet">The sheet whose relationships name the table parts.</param>
    public static XlsxChartTotalsRows Read(XlsxFile file, XlsxSheetEntry sheet)
    {
        ArgumentNullException.ThrowIfNull(file);
        ArgumentNullException.ThrowIfNull(sheet);

        List<SheetRange>? tables = null;
        foreach (XElement table in file.LoadTables(sheet))
        {
            if (!string.Equals(table.Name.LocalName, "table", StringComparison.Ordinal)) continue;

            string? totals = table.Attribute("totalsRowCount")?.Value;
            if (totals is null) continue;
            if (!int.TryParse(totals, NumberStyles.Integer, CultureInfo.InvariantCulture,
                    out int rows) || rows <= 0)
            {
                continue;
            }

            string? id = table.Attribute("id")?.Value;
            if (!int.TryParse(id, NumberStyles.Integer, CultureInfo.InvariantCulture, out int number)
                || number <= 0)
            {
                continue;
            }

            if (string.IsNullOrEmpty(table.Attribute("displayName")?.Value)) continue;
            if (!SheetAddress.TryParseRange(table.Attribute("ref")?.Value, out SheetRange area)) continue;
            if (!area.IsValid) continue;

            (tables ??= []).Add(area);
        }

        return tables is null ? None : new XlsxChartTotalsRows(tables);
    }

    /// <summary>True when the workbook's sheet states no totals-row table at all.</summary>
    public bool IsEmpty => _tables.Count == 0;

    /// <summary>
    /// True when a chart reading <paramref name="range"/> must skip the cell at
    /// (<paramref name="row"/>, <paramref name="column"/>).
    /// </summary>
    /// <param name="range">The range being read, in zero-based indices.</param>
    /// <param name="row">The cell's row.</param>
    /// <param name="column">The cell's column.</param>
    public bool Skips(SheetRange range, int row, int column)
    {
        // Only the range's own last row is ever tested. A totals row in the middle of a range
        // is read like any other, which is the half of the comment that is easy to drop.
        if (_tables.Count == 0 || row != range.LastRow) return false;

        foreach (SheetRange area in _tables)
        {
            if (area.LastRow != row) continue;
            if (column < area.FirstColumn || column > area.LastColumn) continue;
            if (row < area.FirstRow) continue;
            return true;
        }
        return false;
    }
}
