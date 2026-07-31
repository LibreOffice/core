using System.Xml.Linq;
using Paperless.Spreadsheets.Layout;

namespace Paperless.Spreadsheets.Ooxml;

/// <summary>
/// A sheet's print area and repeated header bands, as the workbook's defined names state them.
/// </summary>
/// <param name="PrintAreas">The print ranges, in the order the name lists them.</param>
/// <param name="RepeatColumns">The repeated columns, or null.</param>
/// <param name="RepeatRows">The repeated rows, or null.</param>
public readonly record struct XlsxSheetPrintNames(
    IReadOnlyList<SheetRange> PrintAreas, SheetRange? RepeatColumns, SheetRange? RepeatRows)
{
    /// <summary>A sheet that declares neither.</summary>
    public static XlsxSheetPrintNames None { get; } = new([], null, null);
}

/// <summary>
/// Reads the two built-in defined names that carry a sheet's print setup.
/// </summary>
/// <remarks>
/// <para>
/// SpreadsheetML keeps neither the print area nor the repeated headers on the worksheet. Both
/// are workbook-level <c>definedName</c> entries with reserved names — <c>_xlnm.Print_Area</c>
/// and <c>_xlnm.Print_Titles</c> — scoped to a sheet by <c>localSheetId</c>, which is an index
/// into the workbook's own sheet list rather than a <c>sheetId</c>. Getting that wrong on a
/// workbook with a deleted sheet attaches the print area to the wrong sheet, which is the same
/// class of mistake as assuming <c>sheet1.xml</c> is the first sheet.
/// </para>
/// <para>
/// <c>Print_Titles</c> carries both bands in one value, comma-separated, and tells them apart by
/// shape rather than by order: the column band is a whole-column reference with no row digits
/// (<c>Sheet1!$A:$B</c>) and the row band a whole-row reference with no column letters
/// (<c>Sheet1!$1:$2</c>). LibreOffice separates them the same way in
/// <c>DefinedNameBase::getAbsoluteRange</c>'s callers
/// (<c>sc/source/filter/oox/defnamesbuffer.cxx</c>).
/// </para>
/// </remarks>
internal static class XlsxPrintNames
{
    private const string PrintArea = "_xlnm.Print_Area";
    private const string PrintTitles = "_xlnm.Print_Titles";

    /// <summary>Reads the print names of every sheet, keyed by sheet index.</summary>
    /// <param name="workbook">The <c>workbook</c> element.</param>
    public static Dictionary<int, XlsxSheetPrintNames> Read(XElement workbook)
    {
        ArgumentNullException.ThrowIfNull(workbook);

        Dictionary<int, List<SheetRange>> areas = [];
        Dictionary<int, SheetRange?> repeatColumns = [];
        Dictionary<int, SheetRange?> repeatRows = [];

        foreach (XElement name in Xlsx.Children(Xlsx.Child(workbook, "definedNames"), "definedName"))
        {
            int sheet = Xlsx.Integer(name, "localSheetId") ?? -1;
            if (sheet < 0) continue;

            string? which = Xlsx.Attribute(name, "name");
            if (string.Equals(which, PrintArea, StringComparison.OrdinalIgnoreCase))
            {
                foreach (SheetRange range in Ranges(name.Value))
                {
                    if (!areas.TryGetValue(sheet, out List<SheetRange>? list))
                        areas[sheet] = list = [];
                    list.Add(range);
                }
            }
            else if (string.Equals(which, PrintTitles, StringComparison.OrdinalIgnoreCase))
            {
                foreach (string part in Split(name.Value))
                {
                    // Which band a part is comes from its shape: a reference with no row digits
                    // is a column band and one with no column letters is a row band.
                    if (!SheetAddress.TryParseRange(part, out SheetRange range)) continue;

                    if (range.LastRow >= SheetAddress.MaxRow) repeatColumns[sheet] = range;
                    else if (range.LastColumn >= SheetAddress.MaxColumn) repeatRows[sheet] = range;
                }
            }
        }

        Dictionary<int, XlsxSheetPrintNames> names = [];
        foreach (int sheet in areas.Keys.Concat(repeatColumns.Keys).Concat(repeatRows.Keys).Distinct())
        {
            names[sheet] = new XlsxSheetPrintNames(
                areas.GetValueOrDefault(sheet) ?? (IReadOnlyList<SheetRange>)[],
                repeatColumns.GetValueOrDefault(sheet),
                repeatRows.GetValueOrDefault(sheet));
        }

        return names;
    }

    private static List<SheetRange> Ranges(string value)
    {
        List<SheetRange> ranges = [];
        foreach (string part in Split(value))
        {
            if (SheetAddress.TryParseRange(part, out SheetRange range)) ranges.Add(range);
        }
        return ranges;
    }

    /// <summary>
    /// Splits a defined name's value on commas that are not inside a quoted sheet name.
    /// </summary>
    /// <remarks>
    /// A sheet called <c>Q1, Q2</c> is legal and is written quoted, so splitting on every comma
    /// tears its name in half and loses the range.
    /// </remarks>
    private static List<string> Split(string value)
    {
        List<string> parts = [];
        bool quoted = false;
        int start = 0;

        for (int at = 0; at < value.Length; at++)
        {
            if (value[at] == '\'') quoted = !quoted;
            else if (value[at] == ',' && !quoted)
            {
                parts.Add(value[start..at]);
                start = at + 1;
            }
        }

        parts.Add(value[start..]);
        return [.. parts.Select(part => part.Trim()).Where(part => part.Length > 0)];
    }
}
