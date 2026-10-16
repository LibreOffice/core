using System.Globalization;
using System.Xml.Linq;

namespace XRay.Office.Spreadsheets.Ooxml;

/// <summary>
/// The rows and columns a sheet hides, which a chart plotting visible cells only must skip.
/// </summary>
/// <remarks>
/// <para>
/// <strong>The rule.</strong> <c>ScChart2DataSequence::BuildDataCache</c> asks
/// <c>ColHidden</c> and <c>RowHidden</c> for every cell of a chart's range and
/// <c>continue</c>s past it — the cell is dropped from the sequence, not blanked — unless the
/// diagram says <c>IncludeHiddenCells</c> (<c>sc/source/ui/unoobj/chart2uno.cxx</c>:2636-2646).
/// The OOXML importer sets that property to <c>!c:plotVisOnly</c>
/// (<c>oox/source/drawingml/chart/chartspaceconverter.cxx</c>:264), and <c>plotVisOnly</c>
/// itself defaults to <c>!bMSO2007Document</c> — true for anything Excel 2010 or later wrote
/// (<c>chartspacefragment.cxx</c>:130-131, <c>chartspacemodel.cxx</c>:30). So the default is
/// that a hidden cell is not chart data.
/// </para>
/// <para>
/// <strong>The BIFF reader has had this since round 62</strong> —
/// <c>XlsChartSource.IsHidden</c> — and the OOXML one did not, which is the whole of
/// <c>053_Personal_asset_inventory</c>'s seventh bar.
/// </para>
/// <para>
/// <strong>Measured on that document</strong>, whose chart names <c>Assets!$H$24:$H$30</c> and
/// <c>Assets!$I$24:$I$30</c> — a pivot table's output, in columns H and I, both
/// <c>hidden="1"</c>. 26.2.4.2 draws six categories to a $300,000 axis; unhide the two columns
/// and it draws seven, <c>Grand Total</c> included, to a $400,000 axis, which is what this tree
/// drew from the file as it stands. See <c>probes/chart-resid-r75/results.md</c>.
/// </para>
/// <para>
/// <strong>Reach is four documents of the 947 and one of them is a control.</strong> Censused by
/// <c>probes/chart-resid-r75/census-hidden.py</c>: 28 sequences in 4 workbooks name a hidden
/// cell, and in every one of them <em>every</em> cell of the sequence is hidden.
/// <c>055_Project_timeline_with_milestones</c> is the fourth and states
/// <c>&lt;c:plotVisOnly val="0"/&gt;</c>, so its seventeen hidden cells are chart data and must
/// stay so.
/// </para>
/// </remarks>
internal sealed class XlsxChartHiddenCells
{
    /// <summary>A sheet that hides nothing.</summary>
    public static XlsxChartHiddenCells None { get; } = new([], []);

    private readonly IReadOnlyList<(int First, int Last)> _columns;
    private readonly HashSet<int> _rows;

    private XlsxChartHiddenCells(IReadOnlyList<(int First, int Last)> columns, HashSet<int> rows)
    {
        _columns = columns;
        _rows = rows;
    }

    /// <summary>Reads one sheet's hidden rows and columns, in zero-based indices.</summary>
    /// <param name="worksheet">The <c>worksheet</c> root, or null when the part is missing.</param>
    public static XlsxChartHiddenCells Read(XElement? worksheet)
    {
        if (worksheet is null) return None;

        List<(int First, int Last)>? columns = null;
        foreach (XElement column in Xlsx.Children(Xlsx.Child(worksheet, "cols"), "col"))
        {
            if (!Xlsx.Flag(column, "hidden")) continue;

            int first = Xlsx.Integer(column, "min") ?? 1;
            int last = Xlsx.Integer(column, "max") ?? first;
            if (last < first) continue;

            (columns ??= []).Add((first - 1, last - 1));
        }

        HashSet<int>? rows = null;
        foreach (XElement row in Xlsx.Children(Xlsx.Child(worksheet, "sheetData"), "row"))
        {
            if (!Xlsx.Flag(row, "hidden")) continue;

            string? index = row.Attribute("r")?.Value;
            if (!int.TryParse(index, NumberStyles.Integer, CultureInfo.InvariantCulture, out int at)
                || at <= 0)
            {
                continue;
            }

            (rows ??= []).Add(at - 1);
        }

        return columns is null && rows is null
            ? None
            : new XlsxChartHiddenCells(columns ?? [], rows ?? []);
    }

    /// <summary>True when the sheet hides no row and no column.</summary>
    public bool IsEmpty => _columns.Count == 0 && _rows.Count == 0;

    /// <summary>True when the cell at this row and column is hidden either way.</summary>
    /// <param name="row">Zero-based row.</param>
    /// <param name="column">Zero-based column.</param>
    public bool Hides(int row, int column)
    {
        if (_rows.Contains(row)) return true;

        foreach ((int first, int last) in _columns)
        {
            if (column >= first && column <= last) return true;
        }

        return false;
    }
}
