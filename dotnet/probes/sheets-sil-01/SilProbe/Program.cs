// Dumps, per sheet, the used/printed ranges and every page placement pagination produces.
// The optional third/fourth args extend the printed range by rows/columns so the effect of a
// wider print area can be measured without changing the reader.
using XRay.Office;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;

string path = args[0];
string? only = args.Length > 1 && args[1].Length > 0 ? args[1] : null;
int extraRows = args.Length > 2 ? int.Parse(args[2]) : 0;
int extraColumns = args.Length > 3 ? int.Parse(args[3]) : 0;

using IDocument doc = OfficeDocument.Open(path);
var pages = (SpreadsheetPages)((IPaginatedDocument)doc).Layout();

foreach (SheetLayout sheet in pages.Sheets)
{
    if (only is not null && sheet.Name != only) continue;
    SheetRange used = sheet.UsedRange;
    int forcedRow = -1;
    if (Environment.GetEnvironmentVariable("LASTROWS") is { } spec2)
    {
        foreach (string pair in spec2.Split(';'))
        {
            string[] kv = pair.Split('=');
            if (kv.Length == 2 && kv[0] == sheet.Name)
                forcedRow = int.Parse(kv[1], System.Globalization.CultureInfo.InvariantCulture);
        }
    }

    SheetRange printed = sheet.PrintedRange with
    {
        LastRow = forcedRow >= 0
            ? Math.Max(sheet.PrintedRange.LastRow, forcedRow)
            : sheet.PrintedRange.LastRow + extraRows,
        LastColumn = sheet.PrintedRange.LastColumn + extraColumns,
    };
    var mine = pages.Pages.Where(p => ReferenceEquals(p.Sheet, sheet)).ToList();
    Console.WriteLine($"# '{sheet.Name}' used ..{used.LastColumn},{used.LastRow}"
        + $"  printed ..{printed.LastColumn},{printed.LastRow}  livePages={mine.Count}");

    foreach (SheetDrawing d in sheet.Drawings.Items)
    {
        Console.WriteLine($"    drawing {d.Anchor} from {d.From.Column}+{d.From.ColumnOffset.Twips},"
            + $"{d.From.Row}+{d.From.RowOffset.Twips} to {d.To.Column}+{d.To.ColumnOffset.Twips},"
            + $"{d.To.Row}+{d.To.RowOffset.Twips}");
        long top = sheet.Grid.Rows.TotalPrintedSize(0, d.From.Row - 1).Twips + d.From.RowOffset.Twips;
        long bottom = sheet.Grid.Rows.TotalPrintedSize(0, d.To.Row - 1).Twips + d.To.RowOffset.Twips;
        long left = sheet.Grid.Columns.TotalPrintedSize(0, d.From.Column - 1).Twips + d.From.ColumnOffset.Twips;
        long right = sheet.Grid.Columns.TotalPrintedSize(0, d.To.Column - 1).Twips + d.To.ColumnOffset.Twips;
        Console.WriteLine($"      twips l={left} t={top} r={right} b={bottom}  w={right - left} h={bottom - top}");
    }

    if (Environment.GetEnvironmentVariable("DRAWROWS") is { } dspec)
    {
        foreach (string part in dspec.Split(','))
        {
            int r = int.Parse(part, System.Globalization.CultureInfo.InvariantCulture);
            Console.WriteLine($"    drawgrid row {r} h={sheet.Grid.Rows.PrintedSizeAt(r).Twips}"
                + $" start={sheet.Grid.Rows.TotalPrintedSize(0, r - 1).Twips}"
                + $"   live h={sheet.Grid.Rows.PrintedSizeAt(r).Twips}"
                + $" start={sheet.Grid.Rows.TotalPrintedSize(0, r - 1).Twips}");
        }
        foreach (SheetDrawing d in sheet.Drawings.Items)
        {
            Console.WriteLine($"    parts={d.Parts.Count} anchor={d.Anchor}");
            foreach (var pt in d.Parts.Take(3))
                Console.WriteLine($"      part {pt.X:F4},{pt.Y:F4} {pt.Width:F4}x{pt.Height:F4} rot {pt.Degrees}");
        }
    }

    if (Environment.GetEnvironmentVariable("ROWSTARTS") is not null)
    {
        for (int r = 0; r <= 260; r++)
        {
            Console.WriteLine($"ROWSTART\t{sheet.Name}\t{r}\t{sheet.Grid.Rows.TotalPrintedSize(0, r - 1).Twips}");
        }
        for (int c = 0; c <= 30; c++)
        {
            Console.WriteLine($"COLSTART\t{sheet.Name}\t{c}\t{sheet.Grid.Columns.TotalPrintedSize(0, c - 1).Twips}");
        }
    }

    if (Environment.GetEnvironmentVariable("ROWS") is { } spec)
    {
        foreach (string part in spec.Split(','))
        {
            int r = int.Parse(part, System.Globalization.CultureInfo.InvariantCulture);
            Console.WriteLine($"    row {r} height={sheet.Grid.Rows.PrintedSizeAt(r).Twips} tw"
                + $"  start={sheet.Grid.Rows.TotalPrintedSize(0, r - 1).Twips}");
        }
    }

    var all = SheetPagination.Paginate(sheet.Setup, sheet.Grid, printed);
    var keptSet = new HashSet<(int, int, int)>(
        mine.Where(p => !p.IsNotePage)
            .Select(p => (p.Placement.AreaIndex, p.Placement.ColumnBand, p.Placement.RowBand)));
    foreach (var p in all)
    {
        Console.WriteLine($"   [{(keptSet.Contains((p.AreaIndex, p.ColumnBand, p.RowBand)) ? "live" : "    ")}]"
            + $" x={p.ColumnBand} y={p.RowBand} cells {p.Cells.FirstColumn},{p.Cells.FirstRow}"
            + $"..{p.Cells.LastColumn},{p.Cells.LastRow}");
    }
}
