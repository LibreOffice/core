// Dumps what a row's cells look like to the row-height computation: the text exactly as the
// reader produced it, the cell's wrap/rotation flags, its face and size, and the column width.
// Written so a hypothesis about a row height can be checked against the input rather than
// inferred from the output.
using Paperless;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Spreadsheets.Layout;

string path = args[0];
string sheetName = args.Length > 1 ? args[1] : "";
HashSet<int> rows = [.. (args.Length > 2 ? args[2] : "").Split(',', StringSplitOptions.RemoveEmptyEntries).Select(int.Parse)];

using IDocument doc = PaperlessDocument.Open(path);
var pages = (SpreadsheetPages)((IPaginatedDocument)doc).Layout();

foreach (SheetLayout sheet in pages.Sheets)
{
    if (sheetName.Length > 0 && sheet.Name != sheetName) continue;
    Console.WriteLine($"# sheet '{sheet.Name}'");
    foreach (ContentTableRow tr in (sheet.Cells?.Children ?? []).OfType<ContentTableRow>())
    {
        foreach (ContentTableCell cell in tr.Children.OfType<ContentTableCell>())
        {
            if (rows.Count > 0 && !rows.Contains(cell.Row)) continue;
            string text = cell.GetText();
            if (text.Length == 0) continue;
            var f = sheet.Formats.At(cell.Row, cell.Column);
            var portions = sheet.RichText.At(cell.Row, cell.Column, text);
            Console.WriteLine(
                $"  r{cell.Row} c{cell.Column} colw={sheet.Grid.Columns.SizeAt(cell.Column).Twips} "
                + $"rowh={sheet.Grid.Rows.SizeAt(cell.Row).Twips} wrap={f.Wraps} rot={f.RotationDegrees} "
                + $"stack={f.IsStacked} margin={f.Margin.Twips} size={f.FontSize.Twips} font='{f.FontFamily}' "
                + $"portions={(portions?.Count ?? -1)} field={sheet.HoldsField(cell.Row, cell.Column)} "
                + $"text={System.Text.Json.JsonSerializer.Serialize(text)}");
        }
    }
}
