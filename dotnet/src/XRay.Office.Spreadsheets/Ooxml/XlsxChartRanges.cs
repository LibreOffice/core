using System.Xml.Linq;
using XRay.Office.Core.Extraction;
using XRay.Office.Core.Numbers;
using XRay.Office.Ooxml.DrawingML;
using XRay.Office.Spreadsheets.Layout;

namespace XRay.Office.Spreadsheets.Ooxml;

/// <summary>
/// Resolves a chart's <c>c:f</c> against the workbook the chart is anchored in.
/// </summary>
/// <remarks>
/// <para>
/// This is the Calc half of the split LibreOffice keeps between two chart data providers.
/// <c>ExcelChartConverter::createDataSequence</c>
/// (<c>sc/source/filter/oox/excelchartconverter.cxx:65-105</c>) parses the formula and asks the
/// sheet; only when there is no formula does it fall back to the cached points, which is all the
/// base <c>ChartConverter</c> ever does. See <see cref="ChartRangeResolver"/> for why the two
/// differ and what it costs to read the cache when a workbook is open.
/// </para>
/// <para>
/// <strong>Sheets are read once and kept, whoever asks first.</strong> A chart on sheet 3 may name
/// a range on sheet 12, so the resolver cannot wait for the reader's own loop to reach it — but
/// parsing a worksheet is the expensive half of reading a workbook, so the reader's loop feeds the
/// same cache rather than parsing beside it. A workbook holding no chart never loads a second
/// sheet on this account and never builds an index.
/// </para>
/// <para>
/// <strong>What is deliberately not resolved.</strong> A multi-area reference
/// (<c>(A1:A3,A5:A7)</c>), a defined name, an external workbook and a whole-column reference all
/// return null and leave the cache in place. LibreOffice's formula parser handles the first two
/// and this does not; answering null is the same outcome as its <c>createDataSequence</c> throwing
/// — the cache is what gets drawn — whereas guessing at them would substitute wrong numbers for
/// stale ones, which is worse.
/// </para>
/// </remarks>
internal sealed class XlsxChartRanges(XlsxFile file, XlsxSheetReader reader)
{
    /// <summary>How many cells one sequence is read from.</summary>
    /// <remarks>
    /// The ceiling <see cref="XRay.Office.Ooxml.DrawingML.DrawingChartPlot"/> applies to a cache,
    /// applied to a range for the same reason: a chart over a whole column names a million cells
    /// and plots none of them.
    /// </remarks>
    private const int MaximumCells = 65536;

    private readonly Dictionary<string, ContentTable> _tables =
        new(StringComparer.OrdinalIgnoreCase);

    private readonly Dictionary<string, Dictionary<(int Row, int Column), ContentTableCell>> _cells =
        new(StringComparer.OrdinalIgnoreCase);

    private readonly Dictionary<string, XlsxChartTotalsRows> _totals =
        new(StringComparer.OrdinalIgnoreCase);

    private readonly Dictionary<string, XlsxChartHiddenCells> _hidden =
        new(StringComparer.OrdinalIgnoreCase);

    /// <summary>
    /// A sheet's cells, read once however many times they are asked for.
    /// </summary>
    /// <param name="sheet">The sheet.</param>
    /// <param name="worksheet">
    /// Its already-loaded <c>worksheet</c> root, when the caller has one. Null makes this load the
    /// part itself, which is what a forward reference from a chart needs.
    /// </param>
    public ContentTable TableFor(XlsxSheetEntry sheet, XElement? worksheet)
    {
        ArgumentNullException.ThrowIfNull(sheet);

        if (_tables.TryGetValue(sheet.Name, out ContentTable? known)) return known;

        XElement? root = worksheet ?? file.LoadSheet(sheet);
        ContentTable table = root is null ? new ContentTable() : reader.ReadSheet(root, sheet);
        _tables[sheet.Name] = table;
        return table;
    }

    /// <summary>
    /// A resolver bound to one chart's <c>c:plotVisOnly</c>.
    /// </summary>
    /// <param name="plotVisibleOnly">
    /// What the chart's <c>c:plotVisOnly</c> resolves to. True — the default for anything Excel
    /// 2010 or later wrote — means a cell in a hidden row or column is not chart data. See
    /// <see cref="XlsxChartHiddenCells"/>.
    /// </param>
    public ChartRangeResolver Resolver(bool plotVisibleOnly)
        => formula => Resolve(formula, plotVisibleOnly);

    /// <summary>The cells a <c>c:f</c> names, or null when it names nothing this can reach.</summary>
    /// <param name="formula">The <c>c:f</c> text.</param>
    /// <param name="plotVisibleOnly">
    /// True when the chart plots visible cells only, which is <c>c:plotVisOnly</c>'s own default.
    /// </param>
    public ChartRangeValues? Resolve(string formula, bool plotVisibleOnly = true)
    {
        if (string.IsNullOrWhiteSpace(formula)) return null;

        string text = DefinedName(formula.Trim()) ?? formula.Trim();

        // A union, an intersection or a list of areas. Excel writes them parenthesised and
        // comma-separated; a single area never contains either character.
        if (text.Contains('(', StringComparison.Ordinal)
            || text.Contains(',', StringComparison.Ordinal)
            || text.Contains('[', StringComparison.Ordinal))
        {
            return null;
        }

        if (SplitSheet(text) is not (string sheetName, string reference)) return null;
        if (!SheetAddress.TryParseRange(reference, out SheetRange range)) return null;
        if (!range.IsValid) return null;

        // A whole-column or whole-row reference. TryParseRange fills the missing coordinate with
        // the sheet's own maximum, so this is the one shape that has to be told apart by size
        // rather than rejected by syntax.
        long cells = (long)range.RowCount * range.ColumnCount;
        if (cells is <= 0 or > MaximumCells) return null;

        XlsxSheetEntry? sheet = null;
        foreach (XlsxSheetEntry candidate in file.Sheets)
        {
            if (!string.Equals(candidate.Name, sheetName, StringComparison.OrdinalIgnoreCase))
                continue;
            sheet = candidate;
            break;
        }

        if (sheet is null) return null;

        Dictionary<(int Row, int Column), ContentTableCell> index = IndexFor(sheet);
        XlsxChartTotalsRows totals = TotalsFor(sheet);
        XlsxChartHiddenCells hidden = plotVisibleOnly ? HiddenFor(sheet) : XlsxChartHiddenCells.None;

        List<string?> labels = new((int)cells);
        List<double?> numbers = new((int)cells);
        bool any = false;
        bool skippedHidden = false;

        for (int row = range.FirstRow; row <= range.LastRow; row++)
        {
            for (int column = range.FirstColumn; column <= range.LastColumn; column++)
            {
                // An Excel table's totals row is not chart data. The cell is dropped from the
                // sequence rather than blanked, because LibreOffice's loop `break`s past it and
                // the sequence it builds is genuinely one shorter.
                if (totals.Skips(range, row, column)) continue;

                // A hidden row or column is not chart data unless the chart says otherwise. As
                // above, the cell is dropped rather than blanked: LibreOffice's own loop
                // `continue`s past it (chart2uno.cxx:2636-2646).
                if (hidden.Hides(row, column)) { skippedHidden = true; continue; }

                labels.Add(null);
                numbers.Add(null);

                if (!index.TryGetValue((row, column), out ContentTableCell? cell)) continue;

                string shown = cell.GetOwnText();
                if (shown.Length > 0) { labels[^1] = shown; any = true; }

                if (NumberOf(cell.Value) is { } number) { numbers[^1] = number; any = true; }
            }
        }

        // A range every cell of which is hidden leaves the *cache* standing, which is the
        // opposite of what an all-totals range does and is measured rather than reasoned.
        // 053_Personal_asset_inventory names two ranges wholly inside hidden columns H and I:
        // 26.2.4.2 draws its six cached points, and stripping the <c:pt> from the chart part —
        // changing nothing else — makes it draw nothing at all. So the cached points are what
        // it is drawing there. probes/chart-resid-r75/results.md §2.
        if (labels.Count == 0 && skippedHidden) return null;

        // Every cell of the range was an Excel table's totals row. That is a *resolved* sequence
        // with no points, not a failure to resolve, and the difference decides whether the chart
        // draws nothing (Calc's answer) or draws the cache (which would be the whole plot).
        if (labels.Count == 0) return new ChartRangeValues(labels, numbers);

        // Nothing at all in the range this names. That is what a reference to a sheet whose part
        // is missing looks like, and it is the case LibreOffice's own converter reaches by
        // throwing — so the cache stands rather than being replaced by a column of blanks.
        return any ? new ChartRangeValues(labels, numbers) : null;
    }

    /// <summary>The numeric value a cell contributes to a value sequence, or null.</summary>
    /// <remarks>
    /// A date is a serial number in every spreadsheet and a chart plots the number, so the two
    /// temporal types are converted back rather than dropped. Text, an error and an empty cell all
    /// contribute nothing — the missing point a plotter skips, not a zero.
    /// </remarks>
    private double? NumberOf(object? value) => value switch
    {
        double number => number,
        bool flag => flag ? 1.0 : 0.0,
        DateTime date => Serial(date),
        TimeSpan span => span.TotalDays,
        _ => null,
    };

    /// <summary>A date back to its serial number, the inverse of <see cref="SpreadsheetDate"/>.</summary>
    private double Serial(DateTime date)
    {
        if (file.DateSystem == SpreadsheetDateSystem.Date1904)
            return (date - new DateTime(1904, 1, 1)).TotalDays;

        double days = (date - new DateTime(1899, 12, 30)).TotalDays;

        // The phantom 29 February 1900 again: FromSerial adds a day below serial 61, so the
        // inverse takes it back off. 1900-03-01 is serial 61 and is where the two agree.
        return days < 61 ? days - 1 : days;
    }

    /// <summary>A sheet's totals-row tables, read once however many charts ask for them.</summary>
    private XlsxChartTotalsRows TotalsFor(XlsxSheetEntry sheet)
    {
        if (_totals.TryGetValue(sheet.Name, out XlsxChartTotalsRows? known)) return known;

        XlsxChartTotalsRows totals = XlsxChartTotalsRows.Read(file, sheet);
        _totals[sheet.Name] = totals;
        return totals;
    }

    /// <summary>A sheet's hidden rows and columns, read once however many charts ask.</summary>
    private XlsxChartHiddenCells HiddenFor(XlsxSheetEntry sheet)
    {
        if (_hidden.TryGetValue(sheet.Name, out XlsxChartHiddenCells? known)) return known;

        XlsxChartHiddenCells cells = XlsxChartHiddenCells.Read(file.LoadSheet(sheet));
        _hidden[sheet.Name] = cells;
        return cells;
    }

    private Dictionary<(int Row, int Column), ContentTableCell> IndexFor(XlsxSheetEntry sheet)
    {
        if (_cells.TryGetValue(sheet.Name, out Dictionary<(int, int), ContentTableCell>? known))
            return known;

        Dictionary<(int Row, int Column), ContentTableCell> index = [];
        foreach (ContentNode row in TableFor(sheet, null).Children)
        {
            foreach (ContentNode node in row.Children)
            {
                if (node is ContentTableCell cell) index[(cell.Row, cell.Column)] = cell;
            }
        }

        _cells[sheet.Name] = index;
        return index;
    }

    /// <summary>
    /// Splits <c>'Literature Mapping'!$A$4:$A$16</c> into its sheet and its reference.
    /// </summary>
    /// <remarks>
    /// The separator is searched from the right and outside quotes, because a sheet name may
    /// contain an exclamation mark and is then written quoted. A quoted name doubles its own
    /// apostrophes, which is undone here — <c>'O''Brien'!A1</c> is a sheet called
    /// <c>O'Brien</c>.
    /// </remarks>
    /// <summary>What a workbook-level <c>definedName</c> of this name expands to, or null.</summary>
    /// <remarks>
    /// <para>
    /// The remarks above say a defined name is deliberately not resolved, and for a <c>c:f</c> that
    /// is still the right answer: LibreOffice's own <c>ExcelChartConverter</c> parses the formula
    /// with the document's grammar and a name that does not resolve leaves the cache standing,
    /// which is what returning null here does.
    /// </para>
    /// <para>
    /// <strong>A chartex <c>cx:f</c> has no cache to fall back on and names nothing else.</strong>
    /// Both corpus witnesses state six hidden <c>_xlchart.v1.n</c> names and reference their chart
    /// data entirely through them, so declining here would leave the chart with no numbers at all.
    /// Only the simple shape is taken — a workbook-scope name (no <c>localSheetId</c>) whose value
    /// is one sheet-qualified area — because the loop below then re-parses it exactly as it would a
    /// stated reference. A name whose value is a formula, a union or another name resolves to null,
    /// which is the same outcome as before.
    /// </para>
    /// </remarks>
    private string? DefinedName(string text)
    {
        if (text.Length == 0 || text.Contains('!', StringComparison.Ordinal)) return null;

        _names ??= ReadNames();
        return _names.GetValueOrDefault(text);
    }

    private Dictionary<string, string>? _names;

    private Dictionary<string, string> ReadNames()
    {
        Dictionary<string, string> names = new(StringComparer.OrdinalIgnoreCase);

        foreach (XElement element in Xlsx.Children(
                     Xlsx.Child(file.Workbook, "definedNames"), "definedName"))
        {
            // A sheet-scoped name is resolved against the sheet that declares it, which this does
            // not model; the chartex names are all workbook-scope.
            if (Xlsx.Attribute(element, "localSheetId") is not null) continue;
            if (Xlsx.Attribute(element, "name") is not { Length: > 0 } name) continue;

            string value = element.Value.Trim();
            if (value.Length == 0) continue;

            names.TryAdd(name, value);
        }

        return names;
    }

    private static (string Sheet, string Reference)? SplitSheet(string text)
    {
        bool quoted = false;
        int separator = -1;

        for (int at = 0; at < text.Length; at++)
        {
            if (text[at] == '\'') quoted = !quoted;
            else if (text[at] == '!' && !quoted) separator = at;
        }

        if (separator <= 0 || separator == text.Length - 1) return null;

        string name = text[..separator].Trim();
        if (name.Length >= 2 && name[0] == '\'' && name[^1] == '\'')
            name = name[1..^1].Replace("''", "'", StringComparison.Ordinal);

        return name.Length == 0 ? null : (name, text[(separator + 1)..]);
    }
}
