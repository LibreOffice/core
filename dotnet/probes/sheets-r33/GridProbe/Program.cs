// Dumps the geometry a sheet is paginated from: the resolved column widths, the printable
// page size, and the column/row bands the paginator produced. Written to be diffed against
// LibreOffice's own answer, read out of a flat-ODF export.
using Paperless;
using Paperless.Core.Documents;
using Paperless.Spreadsheets.Layout;

string path = args[0];
string? only = args.Length > 1 && args[1].Length > 0 ? args[1] : null;
int maxCols = args.Length > 2 ? int.Parse(args[2]) : 80;
int maxRows = args.Length > 3 ? int.Parse(args[3]) : 400;

using IDocument doc = PaperlessDocument.Open(path);
var pages = (SpreadsheetPages)((IPaginatedDocument)doc).Layout();

foreach (SheetLayout sheet in pages.Sheets)
{
    if (only is not null && sheet.Name != only) continue;
    SheetGrid grid = sheet.Grid;
    SheetPrintSetup s = sheet.Setup;
    Console.WriteLine($"# sheet '{sheet.Name}' hidden={sheet.IsHidden}");
    Console.WriteLine($"  font={grid.ColumnDigits?.Font}  impliedDigitTwips={ImpliedDigit(grid)}");
    Console.WriteLine($"  page={s.PageSize.Width.Twips}x{s.PageSize.Height.Twips} "
        + $"L{s.LeftMargin.Twips} R{s.RightMargin.Twips} T{s.TopMargin.Twips} B{s.BottomMargin.Twips} "
        + $"hdr{s.HeaderHeight.Twips} ftr{s.FooterHeight.Twips} "
        + $"scale={s.ScaleMode}/{s.ScalePercentage} wide={s.FitToPagesWide} tall={s.FitToPagesTall} "
        + $"headings={s.PrintsHeadings} order={s.PageOrder}");
    foreach (SheetRange a in s.PrintAreas)
        Console.WriteLine($"  printArea {a.FirstColumn},{a.FirstRow}..{a.LastColumn},{a.LastRow}");
    if (s.RepeatColumns is { } rc) Console.WriteLine($"  repeatCols {rc.FirstColumn}..{rc.LastColumn}");
    if (s.RepeatRows is { } rr) Console.WriteLine($"  repeatRows {rr.FirstRow}..{rr.LastRow}");

    List<string> cw = [];
    for (int c = 0; c <= maxCols; c++)
        cw.Add($"{c}:{grid.Columns.PrintedSizeAt(c).Twips}{(grid.Columns.IsHidden(c) ? "H" : "")}");
    Console.WriteLine("  cols " + string.Join(" ", cw));

    List<string> rh = [];
    int maxRow = Math.Min(maxRows, sheet.UsedRange.LastRow);
    for (int r = 0; r <= maxRow; r++)
        rh.Add($"{r}:{grid.Rows.PrintedSizeAt(r).Twips}{(grid.Rows.IsHidden(r) ? "H" : "")}");
    Console.WriteLine("  rows " + string.Join(" ", rh));
}

Console.WriteLine($"# {pages.Count} pages");
foreach (SheetPage p in pages.Pages)
{
    if (only is not null && p.Sheet.Name != only) continue;
    SheetPagePlacement pl = p.Placement;
    Console.WriteLine($"  page {p.Index + 1} '{p.Sheet.Name}' band {pl.ColumnBand}/{pl.RowBand} "
        + $"cells {pl.Cells.FirstColumn},{pl.Cells.FirstRow}..{pl.Cells.LastColumn},{pl.Cells.LastRow} zoom {pl.ZoomPercentage}");
}

static string ImpliedDigit(SheetGrid grid)
{
    if (grid.ColumnDigits is null) return "n/a";
    foreach (SheetDigitRun run in grid.ColumnDigits.Runs)
        if (run.Width.Digits > 0)
            return ((grid.Columns.SizeAt(run.First).Twips - run.Width.BiasTwips) / run.Width.Digits)
                .ToString("0.###");
    SheetDigitWidth d = grid.ColumnDigits.Default;
    return d.Digits > 0
        ? ((grid.Columns.SizeAt(0).Twips - d.BiasTwips) / d.Digits).ToString("0.###")
        : "n/a";
}
