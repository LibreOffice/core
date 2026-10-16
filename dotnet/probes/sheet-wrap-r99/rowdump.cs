using XRay.Office;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Extraction;
using XRay.Office.Spreadsheets.Layout;
using System.Text.Json;

string mode = args[0];

if (mode == "break")
{
    // Does the layouter emit a line for a trailing hard break?
    var face = XRay.Office.Spreadsheets.Layout.SheetFonts.For(new SheetCellFormat());
    Console.WriteLine($"face={face?.Reference.FaceKey}");
    if (face is null) return;
    var lay = new XRay.Office.Text.Layout.ParagraphLayouter(face.Value.Face, shaper: null, breaksOverflowingBlanks: true);
    foreach (string probe in new[] { "a", "a\n", "a\nb", "a\nb\n", "a\n\n", "a\nb\n\n" })
    {
        var laid = lay.Layout(probe, emSize: XRay.Office.Core.Units.Length.FromPoints(11),
            textAreaWidth: XRay.Office.Core.Units.Length.FromPoints(200));
        Console.WriteLine($"  {System.Text.Json.JsonSerializer.Serialize(probe),-14} lines={laid.Lines.Count}");
    }
    return;
}

string path = args[1];

using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(path);
SpreadsheetPages pages = (SpreadsheetPages)document.Layout();

if (mode == "rows")
{
    string? want = args.Length > 2 ? args[2] : null;
    foreach (SheetLayout sheet in pages.Sheets)
    {
        if (want is not null && sheet.Name != want) continue;
        Console.WriteLine($"== sheet {sheet.Name}");
        foreach (var run in sheet.Grid.Rows.Runs)
            Console.WriteLine($"  rows {run.First,6}-{run.Last,-6} {run.Size.Twips,8:F1} hidden={run.IsHidden} opt={run.IsOptimalSize}");
    }
}
else if (mode == "lines")
{
    string want = args[2];
    int lo = int.Parse(args[3]), hi = int.Parse(args[4]);
    foreach (SheetLayout sheet in pages.Sheets)
    {
        if (sheet.Name != want) continue;
        for (int r = lo; r <= hi; r++)
            for (int c = 0; c < 40; c++)
            {
                ContentTableCell? cell = sheet.CellAt(r, c);
                if (cell is null) continue;
                string text = cell.GetOwnText();
                if (text.Length == 0) continue;
                var portions = sheet.RichText.At(r, c, text);
                
                Console.WriteLine($"  r{r} c{c} portions={(portions?.Count ?? -1)} paras={text.Split('\n').Length}");
            }
    }
}
else if (mode == "fill")
{
    string want = args[2];
    int lo = int.Parse(args[3]), hi = int.Parse(args[4]);
    int cmax = args.Length > 5 ? int.Parse(args[5]) : 8;
    foreach (SheetLayout sheet in pages.Sheets)
    {
        if (sheet.Name != want) continue;
        for (int r = lo; r <= hi; r++)
            for (int c = 0; c < cmax; c++)
            {
                var dec = sheet.Formatting.At(r, c);
                Console.WriteLine($"  r{r} c{c} back={dec.Background}");
            }
    }
}
else if (mode == "cells")
{
    string want = args[2];
    int lo = int.Parse(args[3]), hi = int.Parse(args[4]);
    foreach (SheetLayout sheet in pages.Sheets)
    {
        if (sheet.Name != want) continue;
        for (int r = lo; r <= hi; r++)
            for (int c = 0; c < 40; c++)
            {
                ContentTableCell? cell = sheet.CellAt(r, c);
                if (cell is null) continue;
                string text = cell.GetOwnText();
                var portions = sheet.RichText.At(r, c, text);
                Console.WriteLine($"  r{r} c{c} len={text.Length} portions={(portions?.Count ?? -1)} text={JsonSerializer.Serialize(text)}");
            }
    }
}

// -- probe: line counts for a cell, through the same two entry points the height uses.
