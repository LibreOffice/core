using System.Xml.Linq;
using XRay.Office.Spreadsheets.Layout;

namespace XRay.Office.Spreadsheets.Ooxml;

/// <summary>
/// The row-label cells a pivot table draws blank because they repeat the row above.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Calc lays a pivot table out itself; it does not print the cells Excel left behind.</strong>
/// A SpreadsheetML pivot table part states its output range in <c>location/@ref</c>, and Excel
/// writes the laid-out result into those cells as ordinary values. Calc imports the
/// <em>definition</em> and regenerates the output through <c>ScDPOutput</c>, which writes a row
/// field's label only where that field's group starts — every later row of the same group is
/// empty. Excel's "Repeat All Item Labels" (<c>x14:pivotField/@fillDownLabels</c>) fills those
/// cells in; Calc has no such option and ignores the attribute.
/// </para>
/// <para>
/// <strong>Established by an authored variant, not by reading the C++.</strong> Rendering
/// <c>DynamicBubbleChart.xlsx</c> through LibreOffice 26.2.4.2 draws each department name
/// <em>once</em>; flipping <c>fillDownLabels</c> to <c>"0"</c> changes nothing; and removing the
/// pivot table part draws all three copies of each — which is what our renderer does with the
/// part present. See <c>dotnet/probes/sheets-r53-totalsrow/</c>.
/// </para>
/// <para>
/// <strong>Why the test is on the whole prefix and not on the cell above.</strong> A group is
/// keyed by every row field from the outermost down to this one, so a label is repeated only when
/// the labels to its left repeat as well. Testing the single cell would blank
/// <c>DynamicBubbleChart</c>'s <c>Cost</c> column, which holds <c>150</c> twice in a row under two
/// different risk values and which the reference prints both times.
/// </para>
/// <para>
/// <strong>Reach.</strong> Eleven of the corpus's 802 zip documents carry a pivot table part and
/// exactly one states <c>fillDownLabels="1"</c>. On the other ten Excel itself wrote the repeats
/// blank, so there is nothing for this to blank and it is a no-op — which is why the rule is
/// stated as "the cells that repeat" rather than as "the documents that ask for it".
/// </para>
/// </remarks>
internal sealed class XlsxPivotLabels
{
    /// <summary>A sheet holding no pivot table, which is almost every sheet.</summary>
    public static XlsxPivotLabels None { get; } = new([]);

    private readonly HashSet<(int Row, int Column)> _blank;

    private XlsxPivotLabels(HashSet<(int Row, int Column)> blank) => _blank = blank;

    /// <summary>True when the sheet blanks nothing.</summary>
    public bool IsEmpty => _blank.Count == 0;

    /// <summary>True when this cell is a repeated pivot row label and draws nothing.</summary>
    public bool Blanks(int row, int column) => _blank.Contains((row, column));

    /// <summary>
    /// Works out which of a sheet's cells are repeated pivot row labels.
    /// </summary>
    /// <param name="file">The open package, for the sheet's pivot table relationships.</param>
    /// <param name="sheet">The sheet.</param>
    /// <param name="worksheet">Its already-loaded root.</param>
    public static XlsxPivotLabels Read(XlsxFile file, XlsxSheetEntry? sheet, XElement? worksheet)
    {
        ArgumentNullException.ThrowIfNull(file);
        if (sheet is null || worksheet is null) return None;

        IReadOnlyList<XlsxPivotTable> pivots = file.LoadPivotTables(sheet);
        if (pivots.Count == 0) return None;

        List<(SheetRange Area, int FirstDataRow, int LabelColumns)> layouts = [];
        foreach (XlsxPivotTable pivot in pivots)
        {
            if (Xlsx.Child(pivot.Root, "location") is not { } location) continue;
            if (!SheetAddress.TryParseRange(Xlsx.Attribute(location, "ref"), out SheetRange area)) continue;
            if (!area.IsValid) continue;

            // Both offsets are counted from the location's own first row and first column.
            int firstDataRow = Xlsx.Integer(location, "firstDataRow") ?? 1;
            int firstDataCol = Xlsx.Integer(location, "firstDataCol") ?? 0;
            if (firstDataRow < 0) firstDataRow = 0;

            int labelColumns = Math.Clamp(firstDataCol, 0, area.ColumnCount);
            if (labelColumns == 0) continue;

            layouts.Add((area, area.FirstRow + firstDataRow, labelColumns));
        }

        if (layouts.Count == 0) return None;

        Dictionary<(int Row, int Column), string> keys = ReadKeys(worksheet);
        HashSet<(int Row, int Column)> blank = [];

        foreach ((SheetRange area, int firstDataRow, int labelColumns) in layouts)
        {
            for (int row = firstDataRow + 1; row <= area.LastRow; row++)
            {
                // The prefix has to match from the outermost field inwards; the first column that
                // differs ends the run, because everything to its right starts a new group.
                for (int offset = 0; offset < labelColumns; offset++)
                {
                    int column = area.FirstColumn + offset;
                    string here = keys.GetValueOrDefault((row, column), string.Empty);
                    if (here.Length == 0) break;
                    if (!string.Equals(
                            here,
                            keys.GetValueOrDefault((row - 1, column), string.Empty),
                            StringComparison.Ordinal))
                    {
                        break;
                    }

                    blank.Add((row, column));
                }
            }
        }

        return blank.Count == 0 ? None : new XlsxPivotLabels(blank);
    }

    /// <summary>
    /// Every cell's stated content, keyed the way the repeat test compares them.
    /// </summary>
    /// <remarks>
    /// The comparison is on what the file states — a shared-string index, an inline string, a
    /// stored number — not on the displayed text, because two cells showing the same formatted
    /// string are not the same pivot item and a pivot's own items are distinct by construction.
    /// </remarks>
    private static Dictionary<(int Row, int Column), string> ReadKeys(XElement worksheet)
    {
        Dictionary<(int Row, int Column), string> keys = [];
        int expectedRow = 0;

        foreach (XElement rowElement in Xlsx.Children(Xlsx.Child(worksheet, "sheetData"), "row"))
        {
            int rowIndex = (Xlsx.Integer(rowElement, "r") - 1) ?? expectedRow;
            if (rowIndex < 0) rowIndex = expectedRow;
            expectedRow = rowIndex + 1;

            int expectedColumn = 0;
            foreach (XElement cellElement in Xlsx.Children(rowElement, "c"))
            {
                int column = expectedColumn;
                if (Xlsx.Attribute(cellElement, "r") is { } reference
                    && Xlsx.TryParseCellReference(reference, out int parsed, out _))
                {
                    column = parsed;
                }
                if (column < 0) column = expectedColumn;
                expectedColumn = column + 1;

                string type = Xlsx.Attribute(cellElement, "t") ?? "n";
                string body = type == "inlineStr"
                    ? Xlsx.Child(cellElement, "is")?.Value ?? string.Empty
                    : Xlsx.Child(cellElement, "v")?.Value ?? string.Empty;

                if (body.Length == 0) continue;
                keys[(rowIndex, column)] = type + ":" + body;
            }
        }

        return keys;
    }
}
