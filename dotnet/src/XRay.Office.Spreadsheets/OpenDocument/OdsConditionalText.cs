using System.Globalization;
using System.Xml.Linq;
using XRay.Office.OpenDocument;
using XRay.Office.OpenDocument.Styles;
using XRay.Office.Spreadsheets.Layout;

namespace XRay.Office.Spreadsheets.OpenDocument;

/// <summary>
/// The font a cell is drawn and measured in while one of its conditions holds.
/// </summary>
/// <remarks>
/// <para>
/// <strong>An ODF conditional format replaces the whole of a cell's font, not the part of it the
/// rule states.</strong> A conditional format applies a cell <em>style sheet</em>, and Calc reads
/// the font out of that style's item set through <c>lcl_populateresult</c>
/// (<c>sc/source/core/data/patattr.cxx:608-637</c> in the reference C++ checkout), which asks
/// <c>pCondSet-&gt;GetItemIfSet(nWhich)</c> — and <c>GetItemIfSet</c>'s <c>bSrchInParent</c>
/// defaults to <c>true</c> (<c>include/svl/itemset.hxx:201</c>). So the search does not stop at
/// the conditional style: it walks the style's parents, and every parentless Calc cell style is
/// re-parented to <c>Default</c>. A conditional style stating only a colour therefore hands back
/// <c>Default</c>'s face, size and weight, and those beat the cell's own.
/// </para>
/// <para>
/// <strong>SpreadsheetML is not affected, and the difference is one line of the reference.</strong>
/// <c>StylesBuffer::createDxfStyle</c> calls <c>rStyleSheet.ResetParent()</c> before filling the
/// style from the <c>&lt;dxf&gt;</c> (<c>sc/source/filter/oox/stylesbuffer.cxx:3225-3234</c>), so
/// an <c>.xlsx</c> conditional style has no parent to search and a silent rule really does leave
/// the cell's font alone — which is what <see cref="SheetConditionalText"/>'s nulls already model
/// for that reader.
/// </para>
/// <para>
/// <strong>Measured at 26.2.4.2 on one workbook in both spellings.</strong>
/// <c>sistem-rekod-markah-srm-_-rekod-master</c> states <c>C4:T43</c> conditional over cells that
/// are Arial 9 throughout, under a <c>Default</c> of Calibri 11, and its six <c>#N/A</c> columns
/// fire. Read as <c>.ods</c> the reference draws those cells in <strong>Carlito-Regular 11.0</strong>
/// — Calibri's substitute, at <c>Default</c>'s size — and gives their rows 298.2 twips; read as
/// <c>.xlsx</c> it draws the same cells in <strong>LiberationSans 9.01</strong>. The neighbouring
/// <c>E</c> cells, whose condition does not hold, are LiberationSans 9.01 in both.
/// <c>probes/sheet-rows-r98</c>.
/// </para>
/// <para>
/// <strong>Only a condition that actually holds does any of this</strong>, and that is the half an
/// earlier attempt at this got wrong. <c>ScColumn::GetNeededSize</c> is handed
/// <c>pCondSet = rDocument.GetCondResult(...)</c>, which is null unless an entry matched
/// (<c>column2.cxx:283-292</c>); with no match the font comes from the cell as usual. A rule
/// applied to a range whose cells do not satisfy it changes nothing but the row's <em>measurement
/// path</em>, which <see cref="SheetLayout.ConditionalRanges"/> already models.
/// </para>
/// <para>
/// Conditions this cannot decide leave the cell alone, which is what the tree did before this
/// existed. <c>contains-text</c> and <c>formula-is</c> are the corpus's two commonest forms and
/// neither is evaluated here; a cell whose first undecided condition comes before any decided one
/// is skipped outright, because Calc applies the <em>first</em> matching entry
/// (<c>ScConditionalFormat::GetCellStyle</c>) and a later match must not be applied over an
/// earlier one that might have held.
/// </para>
/// </remarks>
internal static class OdsConditionalText
{
    /// <summary>Reads the font each cell of one sheet takes while a condition of its holds.</summary>
    /// <param name="styles">The document's styles, for the applied styles' own fonts.</param>
    /// <param name="table">The <c>table:table</c> element.</param>
    public static Dictionary<(int Row, int Column), SheetConditionalText> Read(
        OdfStyles styles, XElement table)
    {
        ArgumentNullException.ThrowIfNull(styles);
        ArgumentNullException.ThrowIfNull(table);

        Dictionary<(int, int), SheetConditionalText> text = [];

        List<Rule> rules = ReadRules(table);
        if (rules.Count == 0) return text;

        // `calcext:apply-style-name` names the style by its DISPLAY name while the style element
        // is keyed by its encoded `style:name` — `ConditionalStyle_2` against
        // `ConditionalStyle_5f_2`, since `SvXMLExport::EncodeStyleName` escapes the underscore.
        // The importer resolves through the same map, so the lookup has to as well or every
        // conditional style in a LibreOffice-written file misses.
        Dictionary<string, string> byDisplayName = new(StringComparer.Ordinal);
        foreach (OdfStyle style in styles.NamedStyles)
        {
            if (style.Family == OdfStyleFamily.TableCell && style.DisplayName is { } shown)
            {
                byDisplayName.TryAdd(shown, style.Name);
            }
        }

        foreach (Rule rule in rules)
        {
            for (int i = 0; i < rule.Conditions.Count; i++)
            {
                if (byDisplayName.TryGetValue(rule.Conditions[i].Style, out string? encoded))
                {
                    rule.Conditions[i] = rule.Conditions[i] with { Style = encoded };
                }
            }
        }

        Dictionary<string, SheetCellFormat> applied = OdsCellFormats.ResolveNamed(
            styles, rules.SelectMany(rule => rule.Conditions).Select(entry => entry.Style));

        int row = 0;
        foreach (XElement element in table.Descendants(XName.Get("table-row", OdfNamespaces.Table)))
        {
            int repeat = Repeat(element, "number-rows-repeated");
            int lastRow = row + repeat - 1;

            // Only the rules whose ranges reach this row go into the per-cell walk. A workbook
            // stating four hundred blocks over a sheet of eight thousand rows would otherwise
            // cost the product of the two on every cell it holds.
            List<Rule> here = rules.FindAll(rule => rule.Touches(row, lastRow));
            if (here.Count > 0)
            {
                // Clamped to what the rules actually cover, because a sheet's padding is stated
                // as repeats: a run of a million rows or sixteen thousand columns is one element,
                // and only the part of it inside a range can answer anything.
                ReadRow(element, Math.Max(row, here.Min(rule => rule.FirstRow)),
                        Math.Min(lastRow, here.Max(rule => rule.LastRow)),
                        here, applied, text);
            }

            row = lastRow + 1;
        }

        return text;
    }

    private static void ReadRow(
        XElement element, int firstRow, int lastRow, List<Rule> rules,
        Dictionary<string, SheetCellFormat> applied,
        Dictionary<(int, int), SheetConditionalText> text)
    {
        int column = 0;
        foreach (XElement cell in element.Elements())
        {
            if (cell.Name != XName.Get("table-cell", OdfNamespaces.Table))
            {
                if (cell.Name == XName.Get("covered-table-cell", OdfNamespaces.Table))
                {
                    column += Repeat(cell, "number-columns-repeated");
                }

                continue;
            }

            int repeat = Repeat(cell, "number-columns-repeated");
            int lastColumn = column + repeat - 1;
            int firstCovered = Math.Max(column, rules.Min(rule => rule.FirstColumn));
            int lastCovered = Math.Min(lastColumn, rules.Max(rule => rule.LastColumn));

            // An empty cell has no text to draw and is not scanned for a row height, so its
            // condition cannot move anything this reader answers. Skipping it is also what keeps
            // a rule stated over a whole column from costing a million lookups.
            Value? value = ValueOf(cell);
            if (value is { } cellValue)
            {
                for (int r = firstRow; r <= lastRow; r++)
                {
                    for (int c = firstCovered; c <= lastCovered; c++)
                    {
                        if (Applied(rules, applied, r, c, cellValue) is { } rule)
                        {
                            text[(r, c)] = rule;
                        }
                    }
                }
            }

            column = lastColumn + 1;
        }
    }

    /// <summary>The first matching condition's font, or null when none matches or one is undecided.</summary>
    private static SheetConditionalText? Applied(
        List<Rule> rules, Dictionary<string, SheetCellFormat> applied,
        int row, int column, Value value)
    {
        foreach (Rule rule in rules)
        {
            if (!rule.Covers(row, column)) continue;

            foreach (Condition condition in rule.Conditions)
            {
                bool? holds = condition.Holds(value);
                if (holds is null) return null;
                if (holds is false) continue;

                return applied.TryGetValue(condition.Style, out SheetCellFormat? format)
                    ? new SheetConditionalText
                    {
                        Colour = format.Colour,
                        IsStruckThrough = format.IsStruckThrough,
                        FontWeight = format.FontWeight,
                        IsItalic = format.IsItalic,
                        Underline = format.Underline,
                        FontSize = format.FontSize,
                        FontFamily = format.FontFamily,
                        DeclaredFontClass = format.DeclaredFontClass,
                    }
                    : null;
            }

            // Calc's own order: the first format covering the cell that names a style wins, and a
            // format none of whose entries match contributes nothing rather than ending the walk
            // (`ScDocument::GetCondResult`, `sc/source/core/data/documen4.cxx`).
        }

        return null;
    }

    private static List<Rule> ReadRules(XElement table)
    {
        List<Rule> rules = [];

        XElement? formats =
            table.Element(XName.Get("conditional-formats", OdfNamespaces.CalcExt));
        if (formats is null) return rules;

        foreach (XElement format in
                 formats.Elements(XName.Get("conditional-format", OdfNamespaces.CalcExt)))
        {
            string? stated =
                format.Attribute(XName.Get("target-range-address", OdfNamespaces.CalcExt))?.Value;
            if (string.IsNullOrEmpty(stated)) continue;

            List<SheetRange> ranges = [];
            foreach (string part in SheetAddress.SplitList(stated, ' '))
            {
                if (SheetAddress.TryParseRange(part, out SheetRange range)) ranges.Add(range);
            }

            if (ranges.Count == 0) continue;

            List<Condition> conditions = [];
            foreach (XElement entry in
                     format.Elements(XName.Get("condition", OdfNamespaces.CalcExt)))
            {
                string? style =
                    entry.Attribute(XName.Get("apply-style-name", OdfNamespaces.CalcExt))?.Value;
                if (string.IsNullOrEmpty(style)) continue;

                conditions.Add(Condition.Parse(
                    entry.Attribute(XName.Get("value", OdfNamespaces.CalcExt))?.Value, style));
            }

            // A colour scale, data bar or icon set states no `calcext:condition` at all and
            // changes no font; a format holding only those contributes nothing here.
            if (conditions.Count > 0) rules.Add(new Rule(ranges, conditions));
        }

        return rules;
    }

    private static int Repeat(XElement element, string name)
    {
        string? stated = element.Attribute(XName.Get(name, OdfNamespaces.Table))?.Value;

        return int.TryParse(stated, NumberStyles.Integer, CultureInfo.InvariantCulture, out int repeat)
               && repeat > 0
            ? repeat
            : 1;
    }

    /// <summary>What a condition is compared against, as Calc reads it out of the cell.</summary>
    /// <remarks>
    /// <c>lcl_GetCellContent</c> (<c>sc/source/core/data/conditio.cxx:766-800</c>) yields a number
    /// for a value cell and for a formula cell whose result <c>IsValue()</c>, and a string
    /// otherwise. <strong>An error result is a number, and it is zero</strong> — which is why the
    /// six <c>#N/A</c> columns of <c>sistem-rekod-markah-srm</c> satisfy <c>&lt; 40</c> and their
    /// neighbours holding <c>E</c> do not.
    /// </remarks>
    private readonly record struct Value(double? Number, string? Text);

    private static Value? ValueOf(XElement cell)
    {
        string? kind = cell.Attribute(XName.Get("value-type", OdfNamespaces.CalcExt))?.Value
                       ?? cell.Attribute(XName.Get("value-type", OdfNamespaces.Office))?.Value;

        switch (kind)
        {
            case "error":
                return new Value(0, null);

            case "float":
            case "percentage":
            case "currency":
                return Number(cell, "value") is { } number ? new Value(number, null) : null;

            case "boolean":
                return cell.Attribute(XName.Get("boolean-value", OdfNamespaces.Office))?.Value switch
                {
                    "true" => new Value(1, null),
                    "false" => new Value(0, null),
                    _ => null,
                };

            case "string":
                return new Value(null, Str(cell));

            // A date or time is a number in Calc and a formatted string in the file; converting
            // one back to its serial is a second reader's worth of rules, so it stays undecided.
            default:
                return null;
        }
    }

    private static string Str(XElement cell)
    {
        string? stated = cell.Attribute(XName.Get("string-value", OdfNamespaces.Office))?.Value;

        return stated ?? string.Concat(
            cell.Elements(XName.Get("p", OdfNamespaces.Text)).Select(p => p.Value));
    }

    private static double? Number(XElement cell, string name)
        => double.TryParse(
            cell.Attribute(XName.Get(name, OdfNamespaces.Office))?.Value,
            NumberStyles.Float, CultureInfo.InvariantCulture, out double value)
            ? value
            : null;

    private sealed class Rule(List<SheetRange> ranges, List<Condition> conditions)
    {
        public List<Condition> Conditions { get; } = conditions;

        public int FirstRow { get; } = ranges.Min(range => range.FirstRow);

        public int LastRow { get; } = ranges.Max(range => range.LastRow);

        public int FirstColumn { get; } = ranges.Min(range => range.FirstColumn);

        public int LastColumn { get; } = ranges.Max(range => range.LastColumn);

        public bool Touches(int firstRow, int lastRow)
            => ranges.Exists(range => range.FirstRow <= lastRow && range.LastRow >= firstRow);

        public bool Covers(int row, int column)
            => ranges.Exists(range =>
                row >= range.FirstRow && row <= range.LastRow
                && column >= range.FirstColumn && column <= range.LastColumn);
    }

    /// <summary>One <c>calcext:condition</c>, decided where it can be.</summary>
    private readonly record struct Condition(Comparison Operator, double Number, string? Text, string Style)
    {
        public static Condition Parse(string? value, string style)
        {
            string stated = (value ?? string.Empty).Trim();

            (Comparison op, int length) = stated switch
            {
                _ when stated.StartsWith("<=", StringComparison.Ordinal) => (Comparison.AtMost, 2),
                _ when stated.StartsWith(">=", StringComparison.Ordinal) => (Comparison.AtLeast, 2),
                _ when stated.StartsWith("!=", StringComparison.Ordinal) => (Comparison.Unequal, 2),
                _ when stated.StartsWith("<>", StringComparison.Ordinal) => (Comparison.Unequal, 2),
                _ when stated.StartsWith('<') => (Comparison.Less, 1),
                _ when stated.StartsWith('>') => (Comparison.Greater, 1),
                _ when stated.StartsWith('=') => (Comparison.Equal, 1),
                _ => (Comparison.Undecided, 0),
            };

            if (op == Comparison.Undecided) return new Condition(op, 0, null, style);

            string operand = stated[length..].Trim();

            if (operand.Length >= 2 && operand[0] == '"' && operand[^1] == '"')
            {
                return new Condition(op, 0, operand[1..^1], style);
            }

            return double.TryParse(
                operand, NumberStyles.Float, CultureInfo.InvariantCulture, out double number)
                ? new Condition(op, number, null, style)
                : new Condition(Comparison.Undecided, 0, null, style);
        }

        /// <summary>Whether the condition holds, or null when this cannot decide.</summary>
        public bool? Holds(Value value)
        {
            if (Operator == Comparison.Undecided) return null;

            if (value.Number is { } number)
            {
                // A numeric cell against a text operand is `IsValid`'s string branch, which this
                // does not model; leave it undecided rather than guess.
                if (Text is not null) return null;

                return Operator switch
                {
                    Comparison.Less => number < Number,
                    Comparison.Greater => number > Number,
                    Comparison.AtMost => number <= Number,
                    Comparison.AtLeast => number >= Number,
                    Comparison.Equal => number == Number,
                    Comparison.Unequal => number != Number,
                    _ => null,
                };
            }

            // `IsValidStr` (`conditio.cxx:1156-1183`): a numeric operand against a string cell is
            // false outright, whatever the comparison, except for "not equal".
            if (Text is null) return Operator == Comparison.Unequal;

            // Calc compares strings through the transliteration its case-sensitivity flag picks,
            // and a conditional format is case-insensitive unless the document says otherwise;
            // only equality and inequality are modelled here.
            bool equal = string.Equals(value.Text, Text, StringComparison.OrdinalIgnoreCase);

            return Operator switch
            {
                Comparison.Equal => equal,
                Comparison.Unequal => !equal,
                _ => null,
            };
        }
    }

    private enum Comparison
    {
        Undecided,
        Less,
        Greater,
        AtMost,
        AtLeast,
        Equal,
        Unequal,
    }
}
