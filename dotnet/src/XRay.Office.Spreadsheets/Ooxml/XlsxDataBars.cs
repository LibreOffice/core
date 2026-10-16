using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.Spreadsheets.Layout;

namespace XRay.Office.Spreadsheets.Ooxml;

/// <summary>
/// Reads the <c>cfRule type="dataBar"</c> rules a worksheet states and resolves each one to the
/// bar it draws on each cell it covers.
/// </summary>
/// <remarks>
/// <para>
/// Beside <see cref="XlsxConditionalFormats"/>'s colour scale rather than beside
/// <see cref="XlsxConditionalStyles"/>'s predicates, and that is the reference's own division:
/// a bar is an <c>ScDataBarFormat</c> in <c>sc/source/core/data/colorscale.cxx</c> and not an
/// <c>ScConditionEntry</c>. Like a scale it states no <c>&lt;dxf&gt;</c> and computes its answer
/// from the numbers in its own range; unlike one it draws geometry over the cell rather than
/// giving the cell a colour.
/// </para>
/// <para>
/// <strong>Half of a <c>dataBar</c>'s meaning is in the <c>x14</c> extension, and it is not the
/// half the specification would lead you to expect.</strong> <c>ExtCfDataBarRule::importDataBar</c>
/// (<c>sc/source/filter/oox/condformatbuffer.cxx</c>:1710-1715) reads exactly two attributes off
/// <c>x14:dataBar</c> — <c>gradient</c> and <c>axisPosition</c> — and the extension's own
/// <c>minLength</c> and <c>maxLength</c> are <em>never read at all</em>. Its <c>x14:cfvo</c>
/// children then overwrite the main-namespace <c>cfvo</c> types (<c>:1665-1703</c>), which is how
/// <c>autoMin</c>/<c>autoMax</c> reach a rule whose visible markup says <c>min</c>/<c>max</c>.
/// Measured on <c>tests/corpus/features/sheet-cf-data-bar-auto.xlsx</c>, whose
/// <c>--convert-to fods</c> at 26.2.4.2 writes <c>auto-minimum</c>/<c>auto-maximum</c> and
/// <c>min-length="10" max-length="90"</c> against the extension's own 0 and 100.
/// </para>
/// <para>
/// <strong>And the axis position decides whether those two lengths mean anything.</strong>
/// <c>DataBarRule</c>'s constructor sets <c>databar::NONE</c> (<c>:351-356</c>) and only the
/// extension moves it — anything but <c>none</c> and <c>middle</c> becomes <c>AUTOMATIC</c>,
/// including the <c>automatic</c> default of a <c>x14:dataBar</c> that states no
/// <c>axisPosition</c>. <c>ScDataBarFormat::GetDataBarInfo</c>
/// (<c>colorscale.cxx</c>:968-1094) uses <c>mnMinLength</c>/<c>mnMaxLength</c> in the
/// <c>NONE</c> and <c>MIDDLE</c> arms and <strong>ignores both in the <c>AUTOMATIC</c> arm</strong>,
/// where the bar is a plain percentage of the range. So a rule with an extension and one without
/// draw different bars from identical main-namespace markup, and it is the extension-bearing
/// spelling that every corpus rule uses: 26.2.4.2 paints
/// <c>sheet-cf-data-bar-lengths.xlsx</c>'s five cells at 10, 30, 50, 70 and 90 % of the cell and
/// <c>sheet-cf-data-bar-auto.xlsx</c>'s at nothing, 25, 50, 75 and 100 %.
/// </para>
/// <para>
/// Censused 2026-09-11 over every corpus document that opens as an OPC spreadsheet, by content
/// rather than by extension: <strong>9 rules in 6 documents</strong>, every one of them with an
/// extension stating <c>gradient="0"</c> and no <c>axisPosition</c>, so all nine are the
/// <c>AUTOMATIC</c> arm with a solid bar. See <c>probes/cond-format-r97/census-drawrules.py</c>.
/// </para>
/// </remarks>
internal static class XlsxDataBars
{
    /// <summary>The <c>x14</c> conditional-formatting namespace.</summary>
    private const string X14 = "http://schemas.microsoft.com/office/spreadsheetml/2009/9/main";

    /// <summary>
    /// What a negative value takes when the rule states no negative colour of its own.
    /// </summary>
    /// <remarks>
    /// <c>COL_LIGHTRED</c>, the fallback beside <c>mxNegativeColor</c> in
    /// <c>ScDataBarFormat::GetDataBarInfo</c> (<c>colorscale.cxx</c>:1082). It is reachable from an
    /// OOXML import — indeed it is the *usual* answer there — because <c>mbNeg</c> defaults to true
    /// and no SpreadsheetML path clears it.
    /// </remarks>
    private static readonly Colour DefaultNegative = new(0xFF, 0x00, 0x00);

    /// <summary>The <c>xm</c> namespace its formulas and ranges are in.</summary>
    private const string Xm = "http://schemas.microsoft.com/office/excel/2006/main";

    /// <summary>Whether the sheet states any <c>dataBar</c> rule at all.</summary>
    /// <remarks>
    /// Asked before the sheet's numbers are walked, so a workbook with no bar pays one scan of
    /// the <c>conditionalFormatting</c> blocks and nothing else.
    /// </remarks>
    /// <param name="worksheet">The sheet's own root.</param>
    public static bool AnyStated(XElement? worksheet)
    {
        foreach (XElement block in Xlsx.Children(worksheet, "conditionalFormatting"))
        {
            foreach (XElement rule in Xlsx.Children(block, "cfRule"))
            {
                if (string.Equals(Xlsx.Attribute(rule, "type"), "dataBar", StringComparison.Ordinal))
                    return true;
            }
        }

        return false;
    }

    /// <summary>Applies every data bar a worksheet states to the formatting being built.</summary>
    /// <param name="formatting">The sheet's formatting, already holding its stated fills.</param>
    /// <param name="worksheet">The sheet's own root.</param>
    /// <param name="styles">The <c>styleSheet</c> root, or null when the workbook has none.</param>
    /// <param name="theme">The <c>theme</c> root, for colours named by slot.</param>
    /// <param name="numbers">Every numeric cell the sheet states, by position.</param>
    public static void Apply(
        SheetFormatting formatting,
        XElement? worksheet,
        XElement? styles,
        XElement? theme,
        Dictionary<(int Row, int Column), double> numbers)
    {
        ArgumentNullException.ThrowIfNull(formatting);
        ArgumentNullException.ThrowIfNull(numbers);

        List<Rule> rules = Read(worksheet, styles, theme);
        if (rules.Count == 0) return;

        // Blocks in document order and rules within a block by priority, which is what
        // `ScConditionalFormat::GetData` walking `maEntries` gives — and then the *first* bar to
        // reach a cell keeps it, because `ScDocument::FillInfo` takes a bar only
        // `if(aData.pDataBar && !pInfo->pDataBar)` (`fillinfo.cxx`:326-330). The corpus states
        // the discriminating case for the priority half: `076_Inventory_list_accessibility_guide`
        // declares two bars over `J6:J16` **inside one block**, priorities 21 and 22, resolving
        // to #d9d9d9 and #989494 — and 26.2.4.2's own PDF holds eleven #d9d9d9 bars and no
        // #989494 anywhere. No corpus document states two bars in two different blocks, so the
        // block half is the reference's rule read from the source rather than measured here.
        rules.Sort(static (a, b) => a.Block != b.Block
            ? a.Block.CompareTo(b.Block)
            : a.Priority.CompareTo(b.Priority));

        HashSet<(int Row, int Column)> painted = [];

        foreach (Rule rule in rules)
        {
            // The stated numbers filtered by the range rather than a walk of the range, which is
            // the shape `XlsxConditionalFormats` uses for a colour scale and needs no position
            // budget: a rule declared over `A1:XFD1048576` costs one pass of the sheet's own
            // cells. The reference bounds it the same way — `ScColorFormat::getValues` shrinks a
            // range reaching `MaxRow` to the used data area (`colorscale.cxx`:532-537).
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

            List<double> sorted = [.. values];
            sorted.Sort();

            for (int i = 0; i < covered.Count; i++)
            {
                if (!painted.Add(covered[i])) continue;

                formatting.SetConditionalBar(covered[i].Row, covered[i].Column, BarFor(rule, values[i], sorted));
            }
        }
    }

    /// <summary>Where a <c>cfvo</c>'s value comes from, in the reference's own vocabulary.</summary>
    /// <remarks>
    /// <c>ScColorScaleEntryType</c>. <c>Auto</c> is reachable only from an <c>x14:cfvo</c> whose
    /// type is <c>autoMin</c> or <c>autoMax</c>; the main namespace has no spelling for it.
    /// </remarks>
    private enum Limit
    {
        /// <summary>The rule's own number, which is also where <c>formula</c> lands.</summary>
        Value,

        /// <summary>The smallest number in the rule's range.</summary>
        Minimum,

        /// <summary>The largest.</summary>
        Maximum,

        /// <summary>Clamped against zero: <c>min(0, …)</c> below and <c>max(0, …)</c> above.</summary>
        Auto,

        /// <summary>A fraction of the range's span.</summary>
        Percent,

        /// <summary>A percentile of the sorted numbers.</summary>
        Percentile,
    }

    /// <summary>Where the bar's origin sits, which decides which arm computes its length.</summary>
    private enum Axis
    {
        /// <summary>At the left edge, and the only arm besides <see cref="Middle"/> that uses the two lengths.</summary>
        None,

        /// <summary>Wherever the range's own sign change falls.</summary>
        Automatic,

        /// <summary>At the half-way mark, whatever the numbers are.</summary>
        Middle,
    }

    /// <summary>One <c>dataBar</c> rule, both halves of it folded together.</summary>
    private sealed record Rule(
        int Block,
        int Priority,
        List<SheetRange> Ranges,
        Limit LowerKind,
        double LowerValue,
        Limit UpperKind,
        double UpperValue,
        Colour Positive,
        Colour? Negative,
        Colour AxisColour,
        Axis AxisPosition,
        double MinLength,
        double MaxLength,
        bool Gradient,
        bool ShowValue);

    /// <summary>Reads every <c>dataBar</c> rule the worksheet states, extension included.</summary>
    private static List<Rule> Read(XElement? worksheet, XElement? styles, XElement? theme)
    {
        List<Rule> rules = [];
        if (worksheet is null) return rules;

        Dictionary<string, XElement> extensions = Extensions(worksheet);
        XlsxPalette? palette = null;
        int block = 0;

        foreach (XElement group in Xlsx.Children(worksheet, "conditionalFormatting"))
        {
            block++;
            List<SheetRange> ranges = XlsxConditionalFormats.ParseSqref(Xlsx.Attribute(group, "sqref"));
            if (ranges.Count == 0) continue;

            foreach (XElement rule in Xlsx.Children(group, "cfRule"))
            {
                if (!string.Equals(Xlsx.Attribute(rule, "type"), "dataBar", StringComparison.Ordinal))
                    continue;

                XElement? bar = Xlsx.Child(rule, "dataBar");
                if (bar is null) continue;

                List<XElement> cfvos = [.. Xlsx.Children(bar, "cfvo")];
                if (cfvos.Count < 2) continue;

                palette ??= XlsxPalette.Read(styles, theme);
                if (palette.Read(Xlsx.Child(bar, "color")) is not { } positive) continue;

                (Limit lowerKind, double lowerValue) = LimitOf(cfvos[0]);
                (Limit upperKind, double upperValue) = LimitOf(cfvos[1]);

                // `importAttribs`, condformatbuffer.cxx:384-389: both default and both are
                // percentages of the cell, and `showValue` defaults to true.
                double minLength = Length(bar, "minLength", 10);
                double maxLength = Length(bar, "maxLength", 90);
                bool showValue = !string.Equals(Xlsx.Attribute(bar, "showValue"), "0", StringComparison.Ordinal)
                                 && !string.Equals(Xlsx.Attribute(bar, "showValue"), "false",
                                                   StringComparison.OrdinalIgnoreCase);

                Axis axis = Axis.None;
                Colour? negative = null;
                Colour axisColour = new(0, 0, 0);

                // `ScDataBarFormatData`'s own default, which only the extension changes.
                bool gradient = true;

                if (Extension(rule, extensions) is { } extended)
                {
                    XElement? body = extended.Element(XName.Get("dataBar", X14));
                    if (body is not null)
                    {
                        axis = (body.Attribute("axisPosition")?.Value) switch
                        {
                            "none" => Axis.None,
                            "middle" => Axis.Middle,
                            _ => Axis.Automatic,
                        };
                        gradient = body.Attribute("gradient")?.Value is not ("0" or "false");

                        List<XElement> extendedCfvos = [.. body.Elements(XName.Get("cfvo", X14))];
                        if (extendedCfvos.Count > 0)
                            (lowerKind, lowerValue) = ExtendedLimit(extendedCfvos[0], lowerKind, lowerValue);
                        if (extendedCfvos.Count > 1)
                            (upperKind, upperValue) = ExtendedLimit(extendedCfvos[1], upperKind, upperValue);

                        // The element supplies `mxNegativeColor`. It does not decide *whether* a
                        // negative value is treated as negative — `mbNeg` is already true — so a
                        // rule stating none falls back to <see cref="DefaultNegative"/>.
                        if (palette.Read(body.Element(XName.Get("negativeFillColor", X14))) is { } n)
                            negative = n;
                        if (palette.Read(body.Element(XName.Get("axisColor", X14))) is { } a)
                            axisColour = a;
                    }
                }

                rules.Add(new Rule(
                    block,
                    Xlsx.Integer(rule, "priority") ?? int.MaxValue,
                    ranges,
                    lowerKind,
                    lowerValue,
                    upperKind,
                    upperValue,
                    positive,
                    negative,
                    axisColour,
                    axis,
                    minLength,
                    maxLength,
                    gradient,
                    showValue));
            }
        }

        return rules;
    }

    /// <summary>Every <c>x14:cfRule</c> of type <c>dataBar</c> on the sheet, by its <c>id</c>.</summary>
    /// <remarks>
    /// Found by descendant search rather than by walking the <c>extLst</c>/<c>ext</c>/
    /// <c>x14:conditionalFormattings</c> chain: the chain is four elements deep, two of its links
    /// are identified by a GUID in a <c>uri</c> attribute, and the rules are keyed by <c>id</c>
    /// anyway, so the shape of the wrapper carries no information this needs.
    /// </remarks>
    private static Dictionary<string, XElement> Extensions(XElement worksheet)
    {
        Dictionary<string, XElement> found = [];

        foreach (XElement rule in worksheet.Descendants(XName.Get("cfRule", X14)))
        {
            if (!string.Equals(rule.Attribute("type")?.Value, "dataBar", StringComparison.Ordinal))
                continue;

            if (rule.Attribute("id")?.Value is { Length: > 0 } id) found[id] = rule;
        }

        return found;
    }

    /// <summary>The extension a main-namespace rule claims through its <c>x14:id</c>, if any.</summary>
    private static XElement? Extension(XElement rule, Dictionary<string, XElement> extensions)
    {
        if (extensions.Count == 0) return null;

        foreach (XElement id in rule.Descendants(XName.Get("id", X14)))
        {
            if (extensions.TryGetValue(id.Value.Trim(), out XElement? found)) return found;
        }

        return null;
    }

    /// <summary>What a main-namespace <c>cfvo</c> resolves to.</summary>
    /// <remarks>
    /// <c>SetCfvoData</c> through <c>ConvertToModel</c>, <c>condformatbuffer.cxx</c>:313-337.
    /// <c>formula</c> keeps the <c>val</c> it parsed, which is what this tree does with a
    /// colour scale's formula stop for the same reason: no corpus stop is an expression.
    /// </remarks>
    private static (Limit Kind, double Value) LimitOf(XElement cfvo)
    {
        double value = XlsxConditionalFormats.ParseValue(Xlsx.Attribute(cfvo, "val"));
        return (Xlsx.Attribute(cfvo, "type") switch
        {
            "min" => Limit.Minimum,
            "max" => Limit.Maximum,
            "percent" => Limit.Percent,
            "percentile" => Limit.Percentile,
            _ => Limit.Value,
        }, value);
    }

    /// <summary>What an <c>x14:cfvo</c> changes about the limit the main namespace stated.</summary>
    /// <remarks>
    /// The type is replaced outright and the value only when the <c>xm:f</c> beside it parses
    /// whole as a number — <c>ExtCfDataBarRule::finalizeImport</c>'s <c>num</c> arm insists on
    /// <c>nSize == msScaleTypeValue.getLength()</c> (<c>condformatbuffer.cxx</c>:1688-1701), so a
    /// real formula such as <c>$C$11</c> leaves the main-namespace number in place.
    /// </remarks>
    private static (Limit Kind, double Value) ExtendedLimit(XElement cfvo, Limit kind, double value)
    {
        Limit found = cfvo.Attribute("type")?.Value switch
        {
            "min" => Limit.Minimum,
            "max" => Limit.Maximum,
            "autoMin" => Limit.Auto,
            "autoMax" => Limit.Auto,
            "percentile" => Limit.Percentile,
            "percent" => Limit.Percent,
            "num" => Limit.Value,
            _ => kind,
        };

        if (found == Limit.Value
            && cfvo.Element(XName.Get("f", Xm))?.Value.Trim() is { Length: > 0 } text
            && double.TryParse(text, System.Globalization.NumberStyles.Float,
                               System.Globalization.CultureInfo.InvariantCulture, out double parsed))
        {
            value = parsed;
        }

        return (found, value);
    }

    private static double Length(XElement bar, string name, double fallback)
        => double.TryParse(Xlsx.Attribute(bar, name), System.Globalization.NumberStyles.Float,
                           System.Globalization.CultureInfo.InvariantCulture, out double parsed)
            ? parsed
            : fallback;

    /// <summary>
    /// The bar one cell takes, which is <c>ScDataBarFormat::GetDataBarInfo</c> line for line.
    /// </summary>
    /// <remarks>
    /// <c>colorscale.cxx</c>:968-1094. The three axis arms are genuinely three different
    /// formulas rather than one with a shifted origin, and only two of them read
    /// <see cref="Rule.MinLength"/> and <see cref="Rule.MaxLength"/>.
    /// </remarks>
    /// <param name="rule">The rule, both halves folded together.</param>
    /// <param name="value">The cell's own number.</param>
    /// <param name="sorted">The rule's range's numbers, ascending.</param>
    private static SheetDataBar BarFor(Rule rule, double value, List<double> sorted)
    {
        double rangeMinimum = sorted[0];
        double rangeMaximum = sorted[^1];

        double minimum = Resolve(rule.LowerKind, rule.LowerValue, rangeMinimum, rangeMaximum, sorted, lower: true);
        double maximum = Resolve(rule.UpperKind, rule.UpperValue, rangeMinimum, rangeMaximum, sorted, lower: false);

        double length;
        double zero;

        if (rule.AxisPosition == Axis.None)
        {
            if (value <= minimum) length = rule.MinLength;
            else if (value >= maximum) length = rule.MaxLength;
            else
            {
                length = rule.MinLength
                         + ((value - minimum) / (maximum - minimum) * (rule.MaxLength - rule.MinLength));
            }

            zero = 0;
        }
        else if (rule.AxisPosition == Axis.Automatic)
        {
            // The two nudges before the arithmetic, and they are asymmetric in the source: the
            // lower one fires for `COLORSCALE_AUTO` and the upper one for `COLORSCALE_MAX`.
            if (rule.LowerKind == Limit.Auto && minimum > 0) minimum = 0;
            if (rule.UpperKind == Limit.Maximum && maximum < 0) maximum = 0;

            if (minimum < 0) zero = maximum < 0 ? 100 : -100 * minimum / (maximum - minimum);
            else zero = 0;

            double nonNegativeMinimum = Math.Max(0.0, minimum);
            double nonPositiveMaximum = Math.Min(0.0, maximum);

            if (value < 0 && minimum < 0)
            {
                length = value < minimum
                    ? -100
                    : -100 * (value - nonPositiveMaximum) / (minimum - nonPositiveMaximum);
            }
            else if (value > maximum) length = 100;
            else if (value <= minimum) length = 0;
            else length = 100 * (value - nonNegativeMinimum) / (maximum - nonNegativeMinimum);
        }
        else
        {
            zero = 50;
            double furthest = Math.Max(Math.Abs(minimum), Math.Abs(maximum));

            // The reference divides by this unguarded; a range whose limits are both zero would
            // give it 0/0. Nothing states the `middle` axis in the corpus and nothing can reach
            // this, so the guard changes no measured answer.
            if (furthest == 0) furthest = 1;

            if (value < 0 && minimum < 0)
                length = rule.MaxLength * ((value < minimum ? minimum : value) / furthest);
            else
                length = rule.MaxLength * ((value > maximum ? maximum : Math.Max(value, minimum)) / furthest);
        }

        // Same reason: a limit pair a file states rather than one the numbers imply can make any
        // of the three arms divide by zero, and a non-finite percentage would reach the geometry.
        if (!double.IsFinite(length)) length = 0;
        if (!double.IsFinite(zero)) zero = 0;

        return new SheetDataBar
        {
            // `mbNeg` is `ScDataBarFormatData`'s own default of **true** (`colorscale.hxx`:107) and
            // the OOXML importer only ever sets it true again (`condformatbuffer.cxx`:1662); the one
            // place it is cleared is the *ODF* importer (`xmlcondformat.cxx`:483). So every bar read
            // from OOXML treats a negative value as negative, and one whose extension states no
            // `x14:negativeFillColor` paints it in the source's own `COL_LIGHTRED`
            // (`colorscale.cxx`:1073-1085, the colour at :1082) rather than in its positive colour.
            // Measured: 26.2.4.2's PDF of an extension stating no negative colour over -100…100
            // paints the two negative cells `#ff0000`. `probes/cond-format-r97` §2 arm (4).
            Colour = value < 0 ? rule.Negative ?? DefaultNegative : rule.Positive,
            Length = length,
            Zero = zero,
            AxisColour = rule.AxisColour,
            ShowValue = rule.ShowValue,
            Gradient = rule.Gradient,
        };
    }

    /// <summary>
    /// One limit's number, which is <c>getMin</c> or <c>getMax</c> depending on which end it is.
    /// </summary>
    /// <remarks>
    /// The two are not one function with a flag in the source and are not here either: <c>getMin</c>
    /// answers <c>COLORSCALE_MIN</c> and clamps <c>AUTO</c> with <c>min(0, …)</c>, <c>getMax</c>
    /// answers <c>COLORSCALE_MAX</c> and clamps it with <c>max(0, …)</c>, and **neither answers
    /// the other's** — a lower limit typed <c>max</c> falls through to its own stated number
    /// (<c>colorscale.cxx</c>:917-966).
    /// </remarks>
    private static double Resolve(
        Limit kind, double stated, double rangeMinimum, double rangeMaximum, List<double> sorted, bool lower)
        => kind switch
        {
            Limit.Minimum when lower => rangeMinimum,
            Limit.Maximum when !lower => rangeMaximum,
            Limit.Auto => lower ? Math.Min(0, rangeMinimum) : Math.Max(0, rangeMaximum),
            Limit.Percent => rangeMinimum + ((rangeMaximum - rangeMinimum) / 100 * stated),
            Limit.Percentile => XlsxConditionalFormats.Percentile(sorted, stated / 100.0),
            _ => stated,
        };
}
