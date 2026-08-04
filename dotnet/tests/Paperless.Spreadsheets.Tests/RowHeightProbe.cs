using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Units;
using Paperless.Spreadsheets.Layout;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// A scratch probe: dumps every row's computed height for a workbook named by an environment
/// variable, so it can be joined against LibreOffice's own flat-ODF export of the same file.
/// </summary>
public class RowHeightProbe
{
    /// <summary>
    /// How much wider a string is when every glyph advance is rounded to a whole device pixel at a
    /// quantised em, which is what a 96 dpi reference device does to it.
    /// </summary>
    [Fact]
    public void PixelWidths()
    {
        string? path = Environment.GetEnvironmentVariable("PROBE_WIDTHS");
        if (string.IsNullOrEmpty(path)) return;
        int dpi = int.Parse(Environment.GetEnvironmentVariable("PROBE_DPI") ?? "96", System.Globalization.CultureInfo.InvariantCulture);

        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(path);
        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();

        Dictionary<(long, int), (double Exact, double Rounded, int Count)> byEm = [];

        foreach (SheetLayout sheet in pages.Sheets)
        {
            foreach (ContentTableRow tableRow in
                     (sheet.Cells?.Children ?? []).OfType<ContentTableRow>())
            {
                foreach (ContentTableCell cell in tableRow.Children.OfType<ContentTableCell>())
                {
                    string text = cell.GetText();
                    if (text.Length < 20) continue;

                    SheetCellFormat format = sheet.Formats.At(cell.Row, cell.Column);
                    if (!format.Wraps) continue;
                    if (Paperless.Spreadsheets.Layout.SheetFonts.For(format) is not { } face) continue;

                    long sizeTwips = format.FontSize.Twips;
                    double emPx = sizeTwips / (1440.0 / dpi);
                    double emQuantised = Math.Round(emPx);

                    Paperless.Text.Layout.TextMeasurer measurer = new(face.Face);
                    Paperless.Text.Shaping.ShapedText shaped =
                        measurer.Shape(text, Paperless.Spreadsheets.Layout.SheetText.NoKerning);

                    double exact = 0, rounded = 0;
                    foreach (Paperless.Text.Shaping.ShapedGlyph g in shaped.Glyphs)
                    {
                        double a = g.Advance * emQuantised / shaped.UnitsPerEm;
                        exact += g.Advance * emPx / shaped.UnitsPerEm;
                        rounded += Math.Round(a);
                    }

                    (long, int) key = (sizeTwips, face.Metrics.UnitsPerEm);
                    byEm.TryGetValue(key, out (double, double, int) acc);
                    byEm[key] = (acc.Item1 + exact, acc.Item2 + rounded, acc.Item3 + 1);
                }
            }
        }

        string outPath = Environment.GetEnvironmentVariable("PROBE_OUT") ?? "/tmp/widths.tsv";
        using StreamWriter w = new(outPath + ".widths");
        foreach (((long size, int upem), (double exact, double rounded, int n)) in byEm)
        {
            double emPx = size / (1440.0 / dpi);
            w.WriteLine(
                $"size={size}twips\tupem={upem}\tcells={n}\tempx={emPx:F3}\t"
                + $"emq={Math.Round(emPx)}\texact={exact:F0}\trounded={rounded:F0}\t"
                + $"ratio={rounded / exact:F5}");
        }
    }

    /// <summary>
    /// The line count a supplied string takes at a range of available widths, so our break points
    /// can be bisected the same way LibreOffice's are.
    /// </summary>
    [Fact]
    public void Strings()
    {
        string? file = Environment.GetEnvironmentVariable("PROBE_STRINGS");
        if (string.IsNullOrEmpty(file)) return;

        Length size = Length.FromPoints(double.Parse(
            Environment.GetEnvironmentVariable("PROBE_SIZE") ?? "11",
            System.Globalization.CultureInfo.InvariantCulture));
        string family = Environment.GetEnvironmentVariable("PROBE_FONT") ?? "Calibri";
        string[] widths = (Environment.GetEnvironmentVariable("PROBE_COLS") ?? "6200")
            .Split(',', StringSplitOptions.RemoveEmptyEntries);

        using StreamWriter w = new(
            Environment.GetEnvironmentVariable("PROBE_OUT") ?? "/tmp/strings.tsv");

        foreach (string raw in File.ReadAllLines(file))
        {
            if (raw.Trim().Length == 0) continue;
            bool bold = raw.StartsWith('*');
            string text = (bold ? raw[1..] : raw).Replace("\\n", "\n");

            SheetCellFormat format = new()
            {
                FontFamily = family,
                FontSize = size,
                FontWeight = bold ? 700 : 400,
                Wraps = true,
            };

            if (Paperless.Spreadsheets.Layout.SheetFonts.For(format) is not { } face) continue;

            System.Text.StringBuilder row = new();
            row.Append(bold ? "bold\t" : "reg\t").Append(text.Length).Append('\t');
            row.Append("width=").Append(
                new Paperless.Text.Layout.TextMeasurer(face.Face)
                    .Measure(text, size, Paperless.Spreadsheets.Layout.SheetText.NoKerning).Twips);

            foreach (string cw in widths)
            {
                long column = long.Parse(cw, System.Globalization.CultureInfo.InvariantCulture);
                long paperPx = (long)(column * 0.067) - 2 - 1;
                Length available = Length.FromTwips((long)(paperPx / 0.067));
                row.Append(System.Globalization.CultureInfo.InvariantCulture,
                    $"\t{cw}:{SheetTextLayout.LineCount(text, face, size, available)}"
                    + $"@{available.Twips}");
            }

            w.WriteLine(row.ToString());
        }
    }

    [Fact]
    public void Dump()
    {
        string? path = Environment.GetEnvironmentVariable("PROBE_WORKBOOK");
        if (string.IsNullOrEmpty(path)) return;
        string outPath = Environment.GetEnvironmentVariable("PROBE_OUT") ?? "/tmp/probe.tsv";
        string detail = Environment.GetEnvironmentVariable("PROBE_ROWS") ?? "";

        HashSet<string> wanted = [.. detail.Split(';', StringSplitOptions.RemoveEmptyEntries)];

        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(path);
        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();

        using StreamWriter w = new(outPath);
        foreach (SheetLayout sheet in pages.Sheets)
        {
            SheetRange range = SheetDecorationArea.Extend(sheet.UsedRange, sheet.Formatting);
            SheetGrid grid = sheet.Grid;
            int lastRow = range.IsValid ? range.LastRow : -1;
            for (int row = 0; row <= lastRow; row++)
            {
                w.WriteLine(
                    $"{sheet.Name}\t{row}\t{grid.Rows.SizeAt(row).Twips}\t"
                    + $"{(grid.Rows.IsOptimalSize(row) ? "opt" : "man")}");
            }

            if (wanted.Count == 0) continue;

            foreach (ContentTableRow tableRow in
                     (sheet.Cells?.Children ?? []).OfType<ContentTableRow>())
            {
                foreach (ContentTableCell cell in tableRow.Children.OfType<ContentTableCell>())
                {
                    if (!wanted.Contains($"{sheet.Name}:{cell.Row}")) continue;

                    string text = cell.GetText();
                    if (text.Length == 0) continue;

                    SheetCellFormat format = sheet.Formats.At(cell.Row, cell.Column);
                    IReadOnlyList<SheetTextPortion>? portions =
                        sheet.RichText.At(cell.Row, cell.Column, text);

                    w.WriteLine(
                        $"#CELL\t{sheet.Name}\t{cell.Row}\t{cell.Column}\t"
                        + $"width={grid.Columns.SizeAt(cell.Column).Twips}\t"
                        + $"span={cell.ColumnSpan}\t"
                        + $"wraps={format.Wraps}\th={format.Horizontal}\tv={format.Vertical}\t"
                        + $"size={format.FontSize.Twips}\tfont={format.FontFamily}\t"
                        + $"rot={format.RotationDegrees}\tstacked={format.IsStacked}\t"
                        + $"field={sheet.HoldsField(cell.Row, cell.Column)}\t"
                        + $"portions={portions?.Count ?? -1}\t"
                        + $"len={text.Length}\t"
                        + $"text={text.Replace("\n", "\\n").Replace("\r", "\\r").Replace("\t", "\\t")}");

                    long twips = 0;
                    int span = Math.Max(1, cell.ColumnSpan);
                    for (int c = cell.Column; c < cell.Column + span; c++)
                        twips += grid.Columns.SizeAt(c).Twips;
                    long paperPx = (long)(twips * 0.067) - 2 - 1;
                    Length available = Length.FromTwips((long)(paperPx / 0.067));

                    if (Paperless.Spreadsheets.Layout.SheetFonts.For(format) is { } face)
                    {
                        int plain = SheetTextLayout.LineCount(
                            text, face, format.FontSize, available);
                        int rich = portions is { Count: > 0 }
                            ? SheetTextLayout.RichLineRanges(text, portions, face, available).Count
                            : -1;
                        w.WriteLine(
                            $"#LINES\t{sheet.Name}\t{cell.Row}\t{cell.Column}\t"
                            + $"avail={available.Twips}\tplain={plain}\trich={rich}");
                    }

                    if (portions is null) continue;
                    foreach (SheetTextPortion p in portions)
                    {
                        w.WriteLine(
                            $"#PORT\t{sheet.Name}\t{cell.Row}\t{cell.Column}\t"
                            + $"{p.Start}\t{p.Length}\tsize={p.Format.FontSize.Twips}\t"
                            + $"font={p.Format.FontFamily}\tw={p.Format.FontWeight}");
                    }
                }
            }
        }
    }
}
