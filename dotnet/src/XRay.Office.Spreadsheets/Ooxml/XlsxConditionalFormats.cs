using System.Globalization;
using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.Spreadsheets.Layout;

namespace XRay.Office.Spreadsheets.Ooxml;

/// <summary>
/// Reads the <c>cfRule type="colorScale"</c> rules a worksheet states, and resolves each one to
/// the colour it paints on each cell it covers.
/// </summary>
/// <remarks>
/// <para>
/// Resolved here rather than at layout time because a colour scale's stops are defined over the
/// values in its <em>own</em> range — <c>min</c>, <c>max</c> and every percentile are computed
/// from the numbers inside the <c>sqref</c>, not from the sheet — and this is the only place that
/// has both the rule and the cells in front of it. The result is a flat map from position to
/// colour, which is what <see cref="SheetFormatting"/> stores.
/// </para>
/// <para>
/// <strong>What a rule covers and what it paints are different numbers.</strong>
/// <c>ScColorScaleFormat::GetColor</c> begins <c>if(!rCell.hasNumeric()) return {}</c>
/// (<c>sc/source/core/data/colorscale.cxx:679</c>), so a text or empty cell inside the range takes
/// no colour at all and contributes nothing to the range's minimum and maximum. Measured over the
/// corpus, that is the difference between 500 cells and 423: one document declares eight scales
/// over 488 cells of graded answers like <c>"3 (Neutral)"</c> and paints none of them.
/// </para>
/// <para>
/// <strong>The interpolation truncates the delta, not the result.</strong> A channel is
/// <c>c1 + (int)((v − v1)/(v2 − v1) × (c2 − c1))</c> — <c>GetColorValue</c>,
/// <c>colorscale.cxx:591-601</c> — so a rising channel floors and a falling one appears to round
/// up. Read off <c>003_advanced_excel_pie</c>'s own reference PDF before the source was consulted
/// and exact on all 36 channel values of its twelve fills; the authored cases in
/// <c>probes/sheets-r58/probe-colorscale.py</c> reproduce it on every stop type.
/// </para>
/// <para>
/// <strong>The C++ in this checkout is 27.2.0.0.alpha0+ and disagrees with the running binary
/// about whether any of this is drawn.</strong> <c>fillinfo.cxx:776</c> applies a colour scale
/// only when some <em>other</em>, style-named condition also matched on the sheet
/// (<c>if (bAnyCondition &amp;&amp; pInfo-&gt;mxColorScale)</c>). On 26.2.4.2 a workbook whose only
/// conditional formatting is one colour scale draws it: eleven interpolated fills on the authored
/// <c>02-two-minmax</c>, and twelve on <c>003_advanced_excel_pie</c>, which states nothing else.
/// The binary is the ground truth and the tree is reference material.
/// </para>
/// <para>
/// <c>colorScale</c> is read here and <c>dataBar</c> beside it, in
/// <see cref="XlsxDataBars"/>, which this calls with the same walk of the sheet's numbers.
/// Every rule naming a <c>dxfId</c> — <c>expression</c>, <c>cellIs</c>, <c>containsText</c>,
/// <c>endsWith</c>, <c>containsBlanks</c>, <c>notContainsBlanks</c> and
/// <c>duplicateValues</c> — is <see cref="XlsxConditionalStyles"/>'s. The division is the
/// reference's: these two are <c>ScColorScaleFormat</c> and <c>ScDataBarFormat</c> in
/// <c>sc/source/core/data/colorscale.cxx</c>, they state no format, and they compute their
/// answer from the numbers in their own range.
/// </para>
/// <para>
/// <c>iconSet</c> is read beside them, in <see cref="XlsxIconSets"/>, and it is the one of the
/// three whose glyph is not in the file: <c>drawIconSets</c>
/// (<c>sc/source/ui/view/output.cxx</c>:960-989) paints an icon-theme asset into the cell's
/// bottom-left corner at <strong>the cell's own font height</strong>. Which asset is entirely
/// decided by the file, and this tree draws the seven the corpus reaches as vector paths
/// (<see cref="SheetIconArtwork"/>). Censused 2026-09-11 over every corpus document that opens as
/// an OPC spreadsheet: <strong>20 rules in 10 documents</strong> — 2 in the main namespace and 18
/// stated only in the <c>x14</c> extension list, in two disjoint sets of documents — and
/// 26.2.4.2's own renderings of those ten draw <strong>60 icons over 1549 pt²</strong> in seven of
/// them.
/// </para>
/// </remarks>
internal static class XlsxConditionalFormats
{
    /// <summary>
    /// Applies every colour scale a worksheet states to the formatting being built.
    /// </summary>
    /// <remarks>
    /// The conditional fills go in through
    /// <see cref="SheetFormatting.SetConditionalBackground"/>, which is deliberately *not* one of
    /// the stated-format setters: a conditional fill must not reach
    /// <see cref="SheetDecorationArea"/>, because a rule declared over <c>N18:Q1048576</c> would
    /// then extend how far the sheet prints and move page counts. 26.2.4.2 does not extend it
    /// either — an authored scale over <c>B2:B40</c> with data to <c>B12</c> still prints one page.
    /// </remarks>
    /// <param name="formatting">The sheet's formatting, already holding its stated fills.</param>
    /// <param name="styles">The <c>styleSheet</c> root, or null when the workbook has none.</param>
    /// <param name="theme">The <c>theme</c> root, for stops named by theme slot.</param>
    /// <param name="worksheet">The sheet's own root.</param>
    public static void Apply(
        SheetFormatting formatting, XElement? styles, XElement? theme, XElement? worksheet)
    {
        if (worksheet is null) return;

        List<Rule> rules = ReadRules(worksheet, styles, theme);
        bool bars = XlsxDataBars.AnyStated(worksheet);
        bool icons = XlsxIconSets.AnyStated(worksheet);
        if (rules.Count == 0 && !bars && !icons) return;

        // One walk of the sheet's numbers for all three families: a scale, a bar and an icon set
        // each resolve their stops over the same numbers in their own range, and the walk is the
        // expensive half.
        Dictionary<(int Row, int Column), double> numbers = ReadNumbers(worksheet);
        if (numbers.Count == 0) return;

        if (bars) XlsxDataBars.Apply(formatting, worksheet, styles, theme, numbers);
        if (icons) XlsxIconSets.Apply(formatting, worksheet, numbers);
        if (rules.Count == 0) return;

        // Highest priority first, and a lower `priority` attribute is the higher priority. It is
        // priority rather than document order that decides, measured with a discriminating pair:
        // the same two overlapping scales with the document order reversed paint the same colours
        // both times, and the loser's ramp appears in neither rendering.
        rules.Sort(static (a, b) => a.Priority.CompareTo(b.Priority));

        // A cell covered by two scales takes the higher-priority one, so the first rule to reach
        // a position keeps it.
        HashSet<(int Row, int Column)> painted = [];

        foreach (Rule rule in rules)
        {
            List<(int Row, int Column)> covered = [];
            List<double> values = [];

            foreach (SheetRange range in rule.Ranges)
            {
                foreach (KeyValuePair<(int Row, int Column), double> cell in numbers)
                {
                    (int row, int column) = cell.Key;
                    if (row < range.FirstRow || row > range.LastRow
                        || column < range.FirstColumn || column > range.LastColumn)
                    {
                        continue;
                    }

                    covered.Add(cell.Key);
                    values.Add(cell.Value);
                }
            }

            if (covered.Count == 0) continue;

            // The stops are defined over the *sorted* numbers inside the rule's own range —
            // `min`, `max` and every percentile come from `getValues()`, which sorts what it
            // collected (`colorscale.cxx:503-556`) — so the sorted copy is a second list rather
            // than an in-place sort of the one the positions are paired with.
            List<double> sorted = [.. values];
            sorted.Sort();

            double[] stops = new double[rule.Stops.Count];
            for (int i = 0; i < stops.Length; i++) stops[i] = StopValue(rule.Stops[i], sorted);

            for (int i = 0; i < covered.Count; i++)
            {
                if (!painted.Add(covered[i])) continue;

                formatting.SetConditionalBackground(
                    covered[i].Row, covered[i].Column, ColourAt(values[i], stops, rule));
            }
        }
    }

    /// <summary>One colour-scale rule: where it applies, in what order, and its stops.</summary>
    private sealed record Rule(int Priority, List<SheetRange> Ranges, List<Stop> Stops);

    /// <summary>One <c>cfvo</c> and the colour beside it.</summary>
    private readonly record struct Stop(string Kind, double Value, Colour Colour);

    private static List<Rule> ReadRules(XElement worksheet, XElement? styles, XElement? theme)
    {
        List<Rule> rules = [];
        XlsxPalette? palette = null;

        foreach (XElement block in Xlsx.Children(worksheet, "conditionalFormatting"))
        {
            List<SheetRange> ranges = ParseSqref(Xlsx.Attribute(block, "sqref"));
            if (ranges.Count == 0) continue;

            foreach (XElement rule in Xlsx.Children(block, "cfRule"))
            {
                if (!string.Equals(Xlsx.Attribute(rule, "type"), "colorScale", StringComparison.Ordinal))
                    continue;

                XElement? scale = Xlsx.Child(rule, "colorScale");
                if (scale is null) continue;

                palette ??= XlsxPalette.Read(styles, theme);

                List<XElement> cfvos = [.. Xlsx.Children(scale, "cfvo")];
                List<XElement> colours = [.. Xlsx.Children(scale, "color")];
                if (cfvos.Count < 2 || cfvos.Count != colours.Count) continue;

                List<Stop> stops = [];
                for (int i = 0; i < cfvos.Count; i++)
                {
                    if (palette.Read(colours[i]) is not { } colour) break;

                    stops.Add(new Stop(
                        Xlsx.Attribute(cfvos[i], "type") ?? "num",
                        ParseValue(Xlsx.Attribute(cfvos[i], "val")),
                        colour));
                }

                if (stops.Count != cfvos.Count) continue;

                // A missing priority sorts last, which is what an unnumbered rule deserves
                // against numbered ones; 1 is the highest priority SpreadsheetML can state.
                rules.Add(new Rule(Xlsx.Integer(rule, "priority") ?? int.MaxValue, ranges, stops));
            }
        }

        return rules;
    }

    /// <summary>
    /// The value a <c>cfvo</c> resolves to, given the sorted numbers in the rule's range.
    /// </summary>
    /// <remarks>
    /// <c>ScColorScaleFormat::CalcValue</c>, <c>colorscale.cxx:648-672</c>. <c>formula</c> is not
    /// evaluated: the corpus's only formula stops are string literals, on a range that holds no
    /// numbers at all, and a stop whose <c>val</c> is a bare number reads correctly here — the
    /// authored <c>11-formula-cfvo</c> and <c>04-num-2-8</c> render identically on 26.2.4.2.
    /// </remarks>
    private static double StopValue(Stop stop, List<double> sorted) => stop.Kind switch
    {
        "min" => sorted[0],
        "max" => sorted[^1],
        "percent" => sorted[0] + ((sorted[^1] - sorted[0]) * (stop.Value / 100.0)),
        "percentile" => sorted.Count == 1 ? sorted[0] : Percentile(sorted, stop.Value / 100.0),
        _ => stop.Value,
    };

    /// <summary>
    /// The <c>fPercentile</c> of a sorted array, interpolating between the two neighbours.
    /// </summary>
    /// <remarks>
    /// <c>GetPercentile</c>, <c>colorscale.cxx:613-643</c>: the index is
    /// <c>p × (n − 1)</c> and the fraction is what is left of it, so <c>percentile 50</c> over an
    /// even count is the mean of the middle pair. Twelve values 93…170 give 131.5, which is what
    /// puts <c>003_advanced_excel_pie</c>'s seventh cell just past the yellow stop.
    /// </remarks>
    internal static double Percentile(List<double> sorted, double fraction)
    {
        fraction = Math.Min(1.0, fraction);
        if (fraction < 0) return sorted[0];

        double position = fraction * (sorted.Count - 1);
        int index = (int)Math.Floor(position);
        double rest = position - index;

        if (index >= sorted.Count - 1) return sorted[^1];
        return rest == 0.0 ? sorted[index] : sorted[index] + (rest * (sorted[index + 1] - sorted[index]));
    }

    /// <summary>The colour one value takes on a scale whose stops have been resolved.</summary>
    /// <remarks>
    /// <c>ScColorScaleFormat::GetColor</c> walks the stops until the value is no longer past the
    /// upper one, so a three-stop scale is two legs and the value's leg is found by comparison
    /// rather than by index. Values below the first stop and above the last take the end colours
    /// outright, which is what <c>GetColorValue</c>'s two guards do.
    /// </remarks>
    private static Colour ColourAt(double value, double[] stopValues, Rule rule)
    {
        int upper = 1;
        while (upper < stopValues.Length - 1 && value > stopValues[upper]) upper++;

        double lowValue = stopValues[upper - 1];
        double highValue = stopValues[upper];
        Colour low = rule.Stops[upper - 1].Colour;
        Colour high = rule.Stops[upper].Colour;

        return new Colour(
            Channel(value, lowValue, low.R, highValue, high.R),
            Channel(value, lowValue, low.G, highValue, high.G),
            Channel(value, lowValue, low.B, highValue, high.B));
    }

    /// <summary>
    /// One channel of the interpolation.
    /// </summary>
    /// <remarks>
    /// The cast is on the <em>delta</em> and not on the sum, which is the whole of it:
    /// <c>static_cast&lt;int&gt;((nVal - nVal1)/(nVal2-nVal1)*(nColVal2-nColVal1)) + nColVal1</c>.
    /// A rising channel therefore floors and a falling one truncates towards zero, so it appears
    /// to round up. Rounding the sum instead misses 003_advanced_excel_pie's twelve fills on six
    /// of their thirty-six channels.
    /// </remarks>
    private static byte Channel(double value, double low, byte from, double high, byte to)
    {
        if (value <= low) return from;
        if (value >= high) return to;
        if (high <= low) return from;

        int shifted = from + (int)((value - low) / (high - low) * (to - from));
        return (byte)Math.Clamp(shifted, 0, 255);
    }

    /// <summary>Every numeric cell the sheet states, by position.</summary>
    /// <remarks>
    /// A cell counts as numeric when it has a <c>v</c> that parses and its <c>t</c> is not one of
    /// the string, error or boolean spellings. A formula cell's cached <c>v</c> is what is read;
    /// the reference recalculates, so a scale over a volatile formula is a known divergence and is
    /// recorded in this round's prediction file rather than worked around here.
    /// </remarks>
    private static Dictionary<(int Row, int Column), double> ReadNumbers(XElement worksheet)
    {
        Dictionary<(int Row, int Column), double> values = [];

        int expectedRow = 0;
        foreach (XElement row in Xlsx.Children(Xlsx.Child(worksheet, "sheetData"), "row"))
        {
            int index = (Xlsx.Integer(row, "r") - 1) ?? expectedRow;
            if (index < 0) index = expectedRow;
            expectedRow = index + 1;

            foreach (XElement cell in Xlsx.Children(row, "c"))
            {
                string type = Xlsx.Attribute(cell, "t") ?? "n";
                if (type is "s" or "str" or "inlineStr" or "e" or "b") continue;

                if (Xlsx.Child(cell, "v")?.Value is not { } text) continue;
                if (!double.TryParse(text, NumberStyles.Float, CultureInfo.InvariantCulture,
                                     out double number))
                {
                    continue;
                }

                if (Xlsx.TryParseCellReference(Xlsx.Attribute(cell, "r"), out int column, out int at))
                    values[(at, column)] = number;
                else
                    values[(index, values.Count)] = number;
            }
        }

        return values;
    }

    /// <summary>
    /// The ranges an <c>sqref</c> names, which is a space-separated list of A1 ranges.
    /// </summary>
    /// <remarks>
    /// A single cell is a range of one, and a reference may be absolute — <c>$C$5</c> — which
    /// means nothing here because nothing is being copied.
    /// </remarks>
    internal static List<SheetRange> ParseSqref(string? sqref)
    {
        List<SheetRange> ranges = [];
        if (string.IsNullOrWhiteSpace(sqref)) return ranges;

        foreach (string part in sqref.Split(' ', StringSplitOptions.RemoveEmptyEntries))
        {
            string one = part.Replace("$", string.Empty, StringComparison.Ordinal);
            int colon = one.IndexOf(':', StringComparison.Ordinal);

            if (colon < 0)
            {
                if (Xlsx.TryParseCellReference(one, out int column, out int row))
                    ranges.Add(new SheetRange(column, row, column, row));
                continue;
            }

            if (Xlsx.TryParseCellReference(one[..colon], out int firstColumn, out int firstRow)
                && Xlsx.TryParseCellReference(one[(colon + 1)..], out int lastColumn, out int lastRow))
            {
                ranges.Add(new SheetRange(
                    Math.Min(firstColumn, lastColumn), Math.Min(firstRow, lastRow),
                    Math.Max(firstColumn, lastColumn), Math.Max(firstRow, lastRow)));
            }
        }

        return ranges;
    }

    internal static double ParseValue(string? value)
        => double.TryParse(value, NumberStyles.Float, CultureInfo.InvariantCulture, out double parsed)
            ? parsed
            : 0;
}
