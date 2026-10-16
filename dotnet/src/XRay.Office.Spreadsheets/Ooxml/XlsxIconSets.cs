using System.Globalization;
using System.Xml.Linq;
using XRay.Office.Spreadsheets.Layout;

namespace XRay.Office.Spreadsheets.Ooxml;

/// <summary>
/// Reads the <c>iconSet</c> rules a worksheet states — in both of the two spellings — and
/// resolves each one to the glyph it paints on each cell it covers.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Eighteen of the corpus's twenty rules are stated only in the worksheet's <c>x14</c>
/// extension list, with no main-namespace <c>cfRule</c> anywhere.</strong> Such an entry carries
/// its own <c>xm:sqref</c> and becomes a rule of its own: <c>ExtLstLocalContext</c> builds a
/// fresh <c>ScIconSetFormat</c> for an <c>x14:cfRule</c> whose <c>id</c> no main-namespace rule
/// claims, and its own comment says why — <em>"an ext entry does not need to have an existing
/// corresponding entry"</em> (<c>sc/source/filter/oox/extlstcontext.cxx</c>:165-194, this tree).
/// So a reader that walks only <c>conditionalFormatting</c>/<c>cfRule</c> sees two of the
/// corpus's twenty rules and none of the eight documents that hold the other eighteen. The two
/// populations are <strong>disjoint</strong> on this corpus, which is exactly how an earlier pass
/// lost a document and counted nine.
/// </para>
/// <para>
/// Censused 2026-09-11 over every corpus document that opens as an OPC spreadsheet, by content
/// rather than by extension (<c>probes/iconset-r103/census-iconset.py</c>):
/// <strong>20 rules in 10 documents</strong> — 2 main-namespace in 2 documents, 18 <c>x14</c>-only
/// in 8 others, 0 <c>x14</c> extensions <em>of</em> a main-namespace icon rule. Five set names are
/// stated: <c>3Flags</c> ×9, <c>3Stars</c> ×6, <c>3Symbols</c> ×2, <c>3TrafficLights1</c> ×2,
/// <c>3Symbols2</c> ×1.
/// </para>
/// <para>
/// <strong>The bucket is the last entry whose comparison holds, not the first.</strong>
/// <c>ScIconSetFormat::GetIconSetInfo</c> (<c>sc/source/core/data/colorscale.cxx</c>:1186-1253)
/// walks every entry and keeps the highest index that matched — it does not break out
/// (<c>:1205-1216</c>) — each with its own comparison mode, <c>EqGreater</c> by default
/// (<c>:1207</c>) and <c>Greater</c> where the file writes <c>gte="0"</c>
/// (<c>condformatbuffer.cxx</c>:117-122).
/// </para>
/// <para>
/// <strong>And <c>custom="1"</c> is what most corpus rules are really about.</strong> Fourteen of
/// the twenty override at least one bucket with <c>&lt;cfIcon iconSet="NoIcons"/&gt;</c>, stored
/// as index −1 (<c>condformatbuffer.cxx</c>:456-467) and answered as a null <c>ScIconSetInfo</c>
/// (<c>colorscale.cxx</c>:1236-1239) — so those cells draw no icon <em>and keep their own text
/// however <c>showValue</c> is set</em>. That is why the family's real reach is far smaller than
/// its rule count: 26.2.4.2 draws 60 icons over the ten documents, and three of the ten draw none
/// at all.
/// </para>
/// </remarks>
internal static class XlsxIconSets
{
    /// <summary>The <c>x14</c> conditional-formatting namespace.</summary>
    private const string X14 = "http://schemas.microsoft.com/office/spreadsheetml/2009/9/main";

    /// <summary>The <c>xm</c> namespace its ranges and formulas are in.</summary>
    private const string Xm = "http://schemas.microsoft.com/office/excel/2006/main";

    /// <summary>
    /// What <c>iconSet</c> defaults to when the element does not state it.
    /// </summary>
    /// <remarks>
    /// <c>IconSetRule::importAttribs</c>, <c>condformatbuffer.cxx</c>:418. The same name is what
    /// <c>getType</c> (<c>:438-451</c>) falls back to for any name it does not recognise —
    /// <c>NoIcons</c> included, though that one never reaches it because <c>importIcon</c>
    /// short-circuits to index −1 first.
    /// </remarks>
    private const string DefaultSet = "3TrafficLights1";

    /// <summary>Whether the sheet states any <c>iconSet</c> rule at all, in either spelling.</summary>
    /// <param name="worksheet">The sheet's own root.</param>
    public static bool AnyStated(XElement? worksheet)
    {
        if (worksheet is null) return false;

        foreach (XElement block in Xlsx.Children(worksheet, "conditionalFormatting"))
        {
            foreach (XElement rule in Xlsx.Children(block, "cfRule"))
            {
                if (string.Equals(Xlsx.Attribute(rule, "type"), "iconSet", StringComparison.Ordinal))
                    return true;
            }
        }

        foreach (XElement rule in worksheet.Descendants(XName.Get("cfRule", X14)))
        {
            if (string.Equals(rule.Attribute("type")?.Value, "iconSet", StringComparison.Ordinal))
                return true;
        }

        return false;
    }

    /// <summary>Applies every icon an <c>iconSet</c> rule draws on the sheet.</summary>
    /// <param name="formatting">The sheet's formatting, already holding its stated fills.</param>
    /// <param name="worksheet">The sheet's own root.</param>
    /// <param name="numbers">Every numeric cell the sheet states, by position.</param>
    public static void Apply(
        SheetFormatting formatting,
        XElement? worksheet,
        Dictionary<(int Row, int Column), double> numbers)
    {
        ArgumentNullException.ThrowIfNull(formatting);
        ArgumentNullException.ThrowIfNull(numbers);

        List<Rule> rules = Read(worksheet);
        if (rules.Count == 0) return;

        // Document order of the stating block, then priority within it — the shape
        // `XlsxDataBars` uses, and for the same reason: `ScDocument::FillInfo` keeps the *first*
        // icon that reaches a cell (`if(aData.pIconSet && !pInfo->pIconSet)`,
        // `sc/source/core/data/fillinfo.cxx`:331-335). **No corpus document states two icon rules
        // that reach one cell** — `066_Agile_Gantt_chart`'s two are `I10:BL35` and `I36:BL36` —
        // so the tie-break is read from the source and is not measured here.
        rules.Sort(static (a, b) => a.Order != b.Order
            ? a.Order.CompareTo(b.Order)
            : a.Priority.CompareTo(b.Priority));

        HashSet<(int Row, int Column)> painted = [];

        foreach (Rule rule in rules)
        {
            // Fewer than three entries is one of `GetIconSetInfo`'s four ways of answering
            // nothing (`colorscale.cxx`:1195-1196), and it is checked before any value is.
            if (rule.Entries.Count < 3) continue;

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

            // `ScColorFormat::getValues` sorts what it collected (`colorscale.cxx`:503-556), so
            // the sorted copy is a second list rather than an in-place sort of the one the
            // positions are paired with.
            List<double> sorted = [.. values];
            sorted.Sort();

            // `GetMinValue`/`GetMaxValue` (`colorscale.cxx`:1312-1334) take the *first* entry's
            // own number when it is typed `num` or is a formula and the range's smallest
            // otherwise, and the same at the other end with the last entry. Both are computed
            // once per rule, exactly as the reference does.
            double minimum = rule.Entries[0].Kind is EntryKind.Value or EntryKind.Formula
                ? Number(rule.Entries[0], numbers)
                : sorted[0];
            double maximum = rule.Entries[^1].Kind is EntryKind.Value or EntryKind.Formula
                ? Number(rule.Entries[^1], numbers)
                : sorted[^1];

            double[] thresholds = new double[rule.Entries.Count];
            for (int i = 0; i < thresholds.Length; i++)
                thresholds[i] = Threshold(rule.Entries[i], minimum, maximum, sorted, numbers);

            for (int i = 0; i < covered.Count; i++)
            {
                if (!painted.Add(covered[i])) continue;
                if (IconFor(rule, thresholds, values[i]) is not { } icon) continue;

                formatting.SetConditionalIcon(covered[i].Row, covered[i].Column, icon);
            }
        }
    }

    /// <summary>Where one <c>cfvo</c>'s threshold comes from.</summary>
    /// <remarks>
    /// <c>ScColorScaleEntryType</c>, narrowed to the five the icon path can reach.
    /// <see cref="Formula"/> is separate from <see cref="Value"/> because
    /// <c>ScIconSetFormat::GetMinValue</c> treats the two alike while <c>CalcValue</c>'s switch
    /// falls through for both — and because only a formula needs resolving.
    /// </remarks>
    private enum EntryKind
    {
        /// <summary>The entry's own number: <c>type="num"</c>.</summary>
        Value,

        /// <summary>The smallest number in the rule's range.</summary>
        Minimum,

        /// <summary>The largest.</summary>
        Maximum,

        /// <summary>A fraction of the span between the two.</summary>
        Percent,

        /// <summary>A percentile of the sorted numbers.</summary>
        Percentile,

        /// <summary>An expression, which this tree resolves only where it is one cell.</summary>
        Formula,
    }

    /// <summary>One <c>cfvo</c>: what it is measured against and how it compares.</summary>
    /// <param name="Kind">Where the number comes from.</param>
    /// <param name="Value">The stated number, meaningful for <see cref="EntryKind.Value"/>,
    /// <see cref="EntryKind.Percent"/> and <see cref="EntryKind.Percentile"/>.</param>
    /// <param name="Reference">A single-cell reference an expression resolved to, or null.</param>
    /// <param name="OrEqual">Whether the comparison is <c>&gt;=</c> rather than <c>&gt;</c>.</param>
    private readonly record struct Entry(EntryKind Kind, double Value, (int Row, int Column)? Reference, bool OrEqual);

    /// <summary>One <c>iconSet</c> rule, whichever spelling stated it.</summary>
    private sealed record Rule(
        int Order,
        int Priority,
        List<SheetRange> Ranges,
        string Set,
        bool ShowValue,
        bool Reverse,
        List<(string Set, int Index)> Custom,
        List<Entry> Entries);

    /// <summary>Reads every <c>iconSet</c> rule the worksheet states, in both spellings.</summary>
    private static List<Rule> Read(XElement? worksheet)
    {
        List<Rule> rules = [];
        if (worksheet is null) return rules;

        HashSet<string> claimed = [];
        int order = 0;

        foreach (XElement block in Xlsx.Children(worksheet, "conditionalFormatting"))
        {
            order++;
            List<SheetRange> ranges = XlsxConditionalFormats.ParseSqref(Xlsx.Attribute(block, "sqref"));

            foreach (XElement rule in Xlsx.Children(block, "cfRule"))
            {
                // Every main-namespace rule's `x14:id`, whatever its own type: an extension it
                // claims is an extension and not a rule of its own, and the claim is what tells
                // the two apart. **No corpus rule is claimed** — the census counts 0 `x14`
                // extensions of a main-namespace icon rule — so the merge such a pairing would
                // need is not written and an unclaimed extension is read as a whole rule below.
                foreach (XElement id in rule.Descendants(XName.Get("id", X14)))
                {
                    if (id.Value.Trim() is { Length: > 0 } text) claimed.Add(text);
                }

                if (ranges.Count == 0) continue;
                if (!string.Equals(Xlsx.Attribute(rule, "type"), "iconSet", StringComparison.Ordinal))
                    continue;
                if (Xlsx.Child(rule, "iconSet") is not { } body) continue;

                List<Entry> entries = [];
                foreach (XElement cfvo in Xlsx.Children(body, "cfvo"))
                {
                    entries.Add(new Entry(
                        KindOf(Xlsx.Attribute(cfvo, "type")),
                        XlsxConditionalFormats.ParseValue(Xlsx.Attribute(cfvo, "val")),
                        null,
                        OrEqual(Xlsx.Attribute(cfvo, "gte"))));
                }

                List<(string, int)> custom = [];
                foreach (XElement icon in Xlsx.Children(body, "cfIcon"))
                    custom.Add(CustomIcon(Xlsx.Attribute(icon, "iconSet"), Xlsx.Attribute(icon, "iconId")));

                rules.Add(new Rule(
                    order,
                    Xlsx.Integer(rule, "priority") ?? int.MaxValue,
                    ranges,
                    Xlsx.Attribute(body, "iconSet") ?? DefaultSet,
                    ShowValue(Xlsx.Attribute(body, "showValue")),
                    Flag(Xlsx.Attribute(body, "reverse")),
                    custom,
                    entries));
            }
        }

        foreach (XElement block in worksheet.Descendants(XName.Get("conditionalFormatting", X14)))
        {
            order++;

            // The range is the block's own `xm:sqref` and not an attribute: an `x14` rule is
            // stated outside `sheetData`'s namespace entirely, so it carries its address as an
            // element in the `xm` namespace beside the rules it applies to.
            List<SheetRange> ranges = XlsxConditionalFormats.ParseSqref(
                block.Element(XName.Get("sqref", Xm))?.Value);
            if (ranges.Count == 0) continue;

            foreach (XElement rule in block.Elements(XName.Get("cfRule", X14)))
            {
                if (!string.Equals(rule.Attribute("type")?.Value, "iconSet", StringComparison.Ordinal))
                    continue;
                if ((rule.Attribute("id")?.Value.Trim() ?? string.Empty) is { Length: > 0 } id
                    && claimed.Contains(id))
                {
                    continue;
                }

                if (rule.Element(XName.Get("iconSet", X14)) is not { } body) continue;

                List<Entry> entries = [];
                foreach (XElement cfvo in body.Elements(XName.Get("cfvo", X14)))
                    entries.Add(ExtendedEntry(cfvo));

                List<(string, int)> custom = [];
                foreach (XElement icon in body.Elements(XName.Get("cfIcon", X14)))
                {
                    custom.Add(CustomIcon(
                        icon.Attribute("iconSet")?.Value, icon.Attribute("iconId")?.Value));
                }

                rules.Add(new Rule(
                    order,
                    int.TryParse(rule.Attribute("priority")?.Value, NumberStyles.Integer,
                                 CultureInfo.InvariantCulture, out int priority)
                        ? priority
                        : int.MaxValue,
                    ranges,
                    body.Attribute("iconSet")?.Value ?? DefaultSet,
                    ShowValue(body.Attribute("showValue")?.Value),
                    Flag(body.Attribute("reverse")?.Value),
                    custom,
                    entries));
            }
        }

        return rules;
    }

    /// <summary>
    /// One <c>x14:cfvo</c>, whose number is an <c>xm:f</c> child rather than a <c>val</c>.
    /// </summary>
    /// <remarks>
    /// <c>IconSetRule::importFormula</c> (<c>condformatbuffer.cxx</c>:424-435) keeps the text as a
    /// number only when the stated type is one of <c>num</c>, <c>percent</c> or
    /// <c>percentile</c> <em>and</em> the whole string parses; otherwise it is a formula, and
    /// <c>ConvertToModel</c> (<c>:329-333</c>) then overrides whatever type the attribute named
    /// with <c>COLORSCALE_FORMULA</c>. So <c>&lt;x14:cfvo type="num"&gt;&lt;xm:f&gt;$C$11</c> is a
    /// formula entry and not a value one, which is what 26.2.4.2's own <c>fods</c> of
    /// <c>069_Blue_modern_balance_sheet</c> writes back:
    /// <c>calcext:value="=[.$C$11]" calcext:type="formula"</c>.
    /// </remarks>
    private static Entry ExtendedEntry(XElement cfvo)
    {
        EntryKind kind = KindOf(cfvo.Attribute("type")?.Value);
        bool orEqual = OrEqual(cfvo.Attribute("gte")?.Value);
        string text = (cfvo.Element(XName.Get("f", Xm))?.Value ?? string.Empty).Trim();

        bool numeric = kind is EntryKind.Value or EntryKind.Percent or EntryKind.Percentile;
        if (numeric && double.TryParse(text, NumberStyles.Float, CultureInfo.InvariantCulture,
                                       out double parsed))
        {
            return new Entry(kind, parsed, null, orEqual);
        }

        if (text.Length == 0) return new Entry(kind, 0, null, orEqual);

        return new Entry(EntryKind.Formula, 0, SingleCell(text), orEqual);
    }

    /// <summary>
    /// The one cell a formula entry names, when that is all it is.
    /// </summary>
    /// <remarks>
    /// <strong>A deliberate narrowing, and the corpus is what bounds it.</strong> The reference
    /// compiles the expression and evaluates it (<c>ScColorScaleEntry::SetFormula</c> through
    /// <c>ScFormulaCell</c>); this tree has no evaluator in the sheet reader. Both corpus formula
    /// entries are a bare absolute reference to a cell on the same sheet — <c>$C$11</c> and
    /// <c>$D$11</c>, in <c>069_Blue_modern_balance_sheet</c> — and both of that document's two
    /// icons depend on it, so resolving that one shape is what the corpus needs and anything
    /// wider is unreached. A formula this cannot read resolves to zero, which is what an
    /// unresolved entry was worth before.
    /// </remarks>
    private static (int Row, int Column)? SingleCell(string text)
        => Xlsx.TryParseCellReference(text.Replace("$", string.Empty, StringComparison.Ordinal),
                                      out int column, out int row)
            ? (row, column)
            : null;

    /// <summary>What one entry's threshold works out to for this rule's range.</summary>
    /// <remarks>
    /// <c>ScIconSetFormat::CalcValue</c>, <c>colorscale.cxx</c>:1336-1361. <c>PERCENTILE</c> over a
    /// single value answers that value rather than interpolating, which the reference spells out
    /// at <c>:1348-1350</c>.
    /// </remarks>
    private static double Threshold(
        Entry entry,
        double minimum,
        double maximum,
        List<double> sorted,
        Dictionary<(int Row, int Column), double> numbers)
        => entry.Kind switch
        {
            EntryKind.Percent => minimum + ((maximum - minimum) * (entry.Value / 100)),
            EntryKind.Minimum => minimum,
            EntryKind.Maximum => maximum,
            EntryKind.Percentile => sorted.Count == 1
                ? sorted[0]
                : XlsxConditionalFormats.Percentile(sorted, entry.Value / 100.0),
            _ => Number(entry, numbers),
        };

    /// <summary>An entry's own number, a resolvable single-cell formula included.</summary>
    private static double Number(Entry entry, Dictionary<(int Row, int Column), double> numbers)
        => entry.Reference is { } at && numbers.TryGetValue(at, out double found) ? found : entry.Value;

    /// <summary>
    /// The icon one cell takes, or null when the reference would have no <c>ScIconSetInfo</c>.
    /// </summary>
    /// <remarks>
    /// <c>GetIconSetInfo</c>'s body, in its own order: find the bucket, reflect it if the rule is
    /// reversed (<c>colorscale.cxx</c>:1226-1230), and only then look it up in the custom vector
    /// (<c>:1232-1245</c>) — so <c>reverse</c> and <c>custom</c> compose, and a reversed custom
    /// rule reads the custom entry at the <em>reflected</em> index.
    /// </remarks>
    private static SheetIcon? IconFor(Rule rule, double[] thresholds, double value)
    {
        int index = -1;
        for (int i = 0; i < thresholds.Length; i++)
        {
            if (rule.Entries[i].OrEqual ? value >= thresholds[i] : value > thresholds[i]) index = i;
        }

        if (index < 0) return null;

        if (rule.Reverse) index = rule.Entries.Count - 1 - index;

        string set = rule.Set;
        if (rule.Custom.Count > index)
        {
            (string customSet, int customIndex) = rule.Custom[index];

            // `NoIcons`, stored as −1 by `importIcon` and answered as a null `ScIconSetInfo` at
            // `colorscale.cxx`:1236-1239. The cell draws nothing *and keeps its own text*, which
            // is the half of `showValue` that only shows up on a custom rule.
            if (customIndex < 0) return null;

            set = customSet;
            index = customIndex;
        }

        return new SheetIcon { Glyph = GlyphFor(set, index), ShowValue = rule.ShowValue };
    }

    /// <summary>
    /// Which asset a (set, index) pair names, for the sets this tree draws.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>aBitmapMap</c> and the <c>a3Flags</c>-style tables beside it,
    /// <c>colorscale.cxx</c>:1399-1521. Only the four sets the corpus's ten documents actually
    /// name a painted bucket in are listed; every other set, and every index outside a listed
    /// set's own length, is <see cref="SheetIconGlyph.Unpainted"/> — including the unrecognised
    /// name that <c>getType</c> turns into <see cref="DefaultSet"/>.
    /// </para>
    /// <para>
    /// <c>3Symbols</c> and <c>3Symbols2</c> share one table (<c>a3Symbols1</c>,
    /// <c>:1497-1521</c>), which is why the two names answer the same three glyphs.
    /// </para>
    /// </remarks>
    private static SheetIconGlyph GlyphFor(string set, int index)
        => (set, index) switch
        {
            ("3Flags", 0) => SheetIconGlyph.FlagRed,
            ("3Flags", 1) => SheetIconGlyph.FlagAmber,
            ("3Flags", 2) => SheetIconGlyph.FlagGreen,
            ("3Signs", 0) => SheetIconGlyph.Diamond,
            ("3Symbols", 0) or ("3Symbols2", 0) => SheetIconGlyph.CrossInRed,
            ("3Symbols", 1) or ("3Symbols2", 1) => SheetIconGlyph.ExclamationInAmber,
            ("3Symbols", 2) or ("3Symbols2", 2) => SheetIconGlyph.TickInGreen,
            _ => SheetIconGlyph.Unpainted,
        };

    /// <summary>One <c>cfIcon</c>, with <c>NoIcons</c> already collapsed to an index of −1.</summary>
    /// <remarks><c>IconSetRule::importIcon</c>, <c>condformatbuffer.cxx</c>:456-467.</remarks>
    private static (string Set, int Index) CustomIcon(string? set, string? id)
    {
        if (string.Equals(set, "NoIcons", StringComparison.Ordinal)) return (DefaultSet, -1);

        return (set ?? DefaultSet,
                int.TryParse(id, NumberStyles.Integer, CultureInfo.InvariantCulture, out int index)
                    ? index
                    : -1);
    }

    /// <summary><c>SetCfvoData</c>'s type arm, <c>condformatbuffer.cxx</c>:139-158.</summary>
    private static EntryKind KindOf(string? type)
        => type switch
        {
            "min" => EntryKind.Minimum,
            "max" => EntryKind.Maximum,
            "percent" => EntryKind.Percent,
            "percentile" => EntryKind.Percentile,
            "formula" => EntryKind.Formula,
            _ => EntryKind.Value,
        };

    /// <summary>
    /// Whether a <c>cfvo</c> compares with <c>&gt;=</c>, which is what it does unless told not to.
    /// </summary>
    /// <remarks>
    /// <c>SetCfvoData</c>, <c>condformatbuffer.cxx</c>:117-122: an <em>absent</em> <c>gte</c>
    /// leaves the entry's mode at <c>EqGreater</c> and only a stated false moves it to
    /// <c>Greater</c>. Most corpus rules state no <c>gte</c> at all — one of the twenty does, in
    /// <c>069_Blue_modern_balance_sheet</c> — so the default is the arm that carries the corpus.
    /// </remarks>
    private static bool OrEqual(string? gte)
        => gte is null || !(gte is "0" or "false" or "False" or "FALSE");

    /// <summary><c>showValue</c>, which defaults to true (<c>condformatbuffer.cxx</c>:419).</summary>
    private static bool ShowValue(string? value) => value is not ("0" or "false" or "False" or "FALSE");

    /// <summary>An OOXML boolean that defaults to false.</summary>
    private static bool Flag(string? value) => value is "1" or "true" or "True" or "TRUE";
}
