// Prints, per sheet, the used range, the printed range and what the first few columns state,
// so a print-area difference can be attributed to the scan rather than inferred from a page count.
using XRay.Office;
using XRay.Office.Core.Documents;
using XRay.Office.Spreadsheets.Layout;

string path = args[0];
string? only = args.Length > 1 && args[1].Length > 0 ? args[1] : null;
int cols = args.Length > 2 ? int.Parse(args[2]) : 10;

using IDocument doc = OfficeDocument.Open(path);
var pages = (SpreadsheetPages)((IPaginatedDocument)doc).Layout();

foreach (SheetLayout sheet in pages.Sheets)
{
    if (only is not null && sheet.Name != only) continue;
    Console.WriteLine($"# '{sheet.Name}' used {sheet.UsedRange.LastColumn},{sheet.UsedRange.LastRow}"
        + $"  printed {sheet.PrintedRange.LastColumn},{sheet.PrintedRange.LastRow}");
    for (int c = 0; c <= cols; c++)
    {
        var top = sheet.Formatting.At(0, c);
        var mid = sheet.Formatting.At(12, c);
        Console.WriteLine($"    col {c}: row0 bg={top.Background} borders={!top.Borders.IsNone}"
            + $"   row12 bg={mid.Background} borders={!mid.Borders.IsNone}");
    }
}
