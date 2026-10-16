using System.Globalization;
using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using CellIs = XRay.Office.Spreadsheets.Layout.SheetConditions.CellIs;
using Comparison = XRay.Office.Spreadsheets.Layout.SheetConditions.Comparison;
using ICondition = XRay.Office.Spreadsheets.Layout.SheetConditions.ICondition;
using Operand = XRay.Office.Spreadsheets.Layout.SheetConditions.Operand;
using Expression = XRay.Office.Spreadsheets.Layout.SheetConditions.Expression;
using Sheet = XRay.Office.Spreadsheets.Layout.SheetConditions.Sheet;
using Value = XRay.Office.Spreadsheets.Layout.SheetConditions.Value;

namespace XRay.Office.Spreadsheets.Ooxml;

/// <summary>
/// Reads the conditional formatting rules that name a <c>dxf</c>, and works out which cells each
/// one paints.
/// </summary>
/// <remarks>
/// <para>
/// Beside <see cref="XlsxConditionalFormats"/> rather than inside it because the two answer
/// different questions from opposite ends. A <c>colorScale</c> states no format at all — its
/// colour is computed from the numbers inside its own range — while every rule here names a
/// <c>&lt;dxf&gt;</c> in <c>styles.xml</c> and the whole of the work is deciding, per cell,
/// whether the rule is true. Calc keeps the same separation: a scale is an
/// <c>ScColorScaleFormat</c> that <c>fillinfo.cxx</c> applies last and on its own, and a
/// condition is an <c>ScCondFormatEntry</c> resolved to a cell <em>style</em> through
/// <c>ScDocument::GetCondResult</c>.
/// </para>
/// <para>
/// <strong>Exactly one rule wins a cell.</strong> <c>GetCondResult</c>
/// (<c>sc/source/core/data/documen4.cxx</c>) walks the formats covering the cell and returns the
/// first non-empty style name's item set; <c>ScConditionalFormat::GetCellStyle</c>
/// (<c>conditio.cxx</c>) returns the first matching entry within a format. So the properties of
/// two matching rules are never merged, and a cell takes one <c>dxf</c> or none.
/// </para>
/// <para>
/// <strong>What a <c>dxf</c> is silent about is not overridden.</strong> A conditional style's
/// item set holds only the items the rule states, and everything else falls through to the cell's
/// own pattern — which is why the result is a differential
/// <see cref="SheetConditionalText"/> rather than a whole format.
/// </para>
/// <para>
/// <strong>A <c>dxf</c>'s fill states its colour in <c>bgColor</c>, which is the opposite of a
/// cell's.</strong> <c>Fill::finalizeImport</c> (<c>sc/source/filter/oox/stylesbuffer.cxx</c>)
/// begins <c>if (mbDxf)</c> and, when the fill colour is used and the pattern is absent or solid,
/// moves <c>maFillColor</c> — the <c>bgColor</c> — into <c>maPatternColor</c> and forces the
/// pattern to solid. Reading <c>fgColor</c> here, as a cell's <c>patternFill</c> needs, finds
/// nothing at all on the 2734 corpus <c>dxf</c> fills that state only a background.
/// </para>
/// <para>
/// <strong>What is evaluated is deliberately narrow, and it is where the corpus is.</strong>
/// Censused over the 243 corpus <c>.xlsx</c>/<c>.xlsm</c> by parsing every worksheet:
/// <c>containsText</c> 927 rules in 5 documents, <c>expression</c> 601 in 34, <c>cellIs</c> 123
/// in 18, <c>containsBlanks</c> 99 in 1, <c>duplicateValues</c> 20 in 5, <c>endsWith</c> 5 in 1
/// and <c>notContainsBlanks</c> 3 in 2 — all of which are read. The 601 <c>expression</c> rules
/// reduce to 461 of the single shape <c>&lt;reference&gt; = "&lt;text&gt;"</c> and a further two
/// dozen of the same shape against a number or a second reference; the rest are <c>AND</c>,
/// <c>MOD(ROW())</c>, <c>ISERROR</c>, <c>TODAY</c>, defined names and <c>#REF!</c>, which need a
/// formula interpreter. A rule this cannot evaluate paints nothing, which is the same answer the
/// reader gave before it existed.
/// </para>
/// <para>
/// <strong>Eight further families the specification defines have no corpus witness and are
/// deliberately not modelled</strong> — <c>beginsWith</c>, <c>notContainsText</c>,
/// <c>uniqueValues</c>, <c>containsErrors</c>, <c>notContainsErrors</c>, <c>top10</c>,
/// <c>aboveAverage</c> and <c>timePeriod</c>, zero rules each. The first three are one arm apiece
/// of the predicates already here and the rest are <c>ScConditionMode::Error</c>,
/// <c>Top10</c>/<c>Bottom10</c>/<c>TopPercent</c>/<c>BottomPercent</c>,
/// <c>AboveAverage</c> and a date band; implementing any of them would be code no document
/// exercises.
/// </para>
/// </remarks>
internal static class XlsxConditionalStyles
{
    /// <summary>
    /// How many cell positions all of a sheet's rules together may be evaluated over.
    /// </summary>
    /// <remarks>
    /// A rule's <c>sqref</c> is routinely a whole column — <c>TK-Syllabus-Comparison-Document-v2</c>
    /// states 413 of them over <c>S1:S1048576</c> — so the ranges are clamped to the rows and
    /// columns the sheet actually states before anything is walked. This is the second guard, for
    /// the sheet that genuinely holds a hundred thousand rows under a hundred rules: past it the
    /// remaining rules are skipped rather than the reader stalling.
    /// </remarks>
    private const long PositionBudget = 8_000_000;

    /// <summary>
    /// Resolves every <c>dxf</c>-naming rule a worksheet states.
    /// </summary>
    /// <param name="formatting">The sheet's decoration, which takes the conditional fills.</param>
    /// <param name="styles">The <c>styleSheet</c> root, or null when the workbook has none.</param>
    /// <param name="theme">The <c>theme</c> root, for colours named by slot.</param>
    /// <param name="worksheet">The sheet's own root.</param>
    /// <param name="shared">The workbook's shared strings, for the cells a rule compares.</param>
    /// <param name="sheetName">The sheet's name, so a rule may qualify a reference with it.</param>
    /// <param name="workbook">The workbook root, for the defined names a rule's formula uses.</param>
    /// <returns>What each matching cell's winning rule changes about its text.</returns>
    public static Dictionary<(int Row, int Column), SheetConditionalText> Apply(
        SheetFormatting formatting,
        XElement? styles,
        XElement? theme,
        XElement? worksheet,
        XlsxSharedStrings shared,
        string? sheetName = null,
        XElement? workbook = null)
    {
        Dictionary<(int Row, int Column), SheetConditionalText> text = [];
        if (worksheet is null || styles is null) return text;

        XlsxPalette palette = XlsxPalette.Read(styles, theme);
        List<Difference> differences = ReadDifferences(styles, palette);
        if (differences.Count == 0) return text;

        List<Rule> rules = ReadRules(worksheet, differences, sheetName ?? string.Empty,
                                    DefinedNames(workbook));
        if (rules.Count == 0) return text;

        Sheet sheet = ReadSheet(worksheet, shared);
        if (sheet.LastRow < 0) return text;

        // Blocks in the order the sheet states them, and rules within one block by priority —
        // which is the reference's order and not a global sort on `priority`. A cell's pattern
        // carries an `ScCondFormatIndexes`, an `o3tl::sorted_vector<sal_uInt32>` of *format*
        // indices (`sc/inc/attrib.hxx`:271), and `ScDocument::GetCondResult`
        // (`sc/source/core/data/documen4.cxx`:1119-1143) walks it in that order; the index is
        // handed out by `AddCondFormat` as each `<conditionalFormatting>` block finalises, so it
        // is document order. Within a block `CondFormat::insertRule` keys `maRules` by priority
        // (`condformatbuffer.cxx`:1173-1180), a `std::map`, so those are ascending.
        rules.Sort(static (a, b) => a.Block != b.Block
            ? a.Block.CompareTo(b.Block)
            : a.Priority.CompareTo(b.Priority));

        HashSet<(int Row, int Column)> decided = [];
        long budget = PositionBudget;

        foreach (Rule rule in rules)
        {
            foreach (SheetRange range in rule.Ranges)
            {
                int firstRow = Math.Max(0, range.FirstRow);
                int lastRow = Math.Min(sheet.LastRow, range.LastRow);
                int firstColumn = Math.Max(0, range.FirstColumn);
                int lastColumn = Math.Min(sheet.LastColumn, range.LastColumn);
                if (lastRow < firstRow || lastColumn < firstColumn) continue;

                long span = (long)(lastRow - firstRow + 1) * (lastColumn - firstColumn + 1);
                if (span > budget) return text;
                budget -= span;

                for (int row = firstRow; row <= lastRow; row++)
                {
                    for (int column = firstColumn; column <= lastColumn; column++)
                    {
                        if (!decided.Add((row, column))) continue;
                        if (!rule.Test.Holds(sheet, row, column, rule.AnchorRow, rule.AnchorColumn))
                        {
                            decided.Remove((row, column));
                            continue;
                        }

                        Difference difference = rule.Difference;
                        if (difference.Background is { } fill)
                            formatting.SetConditionalBackground(row, column, fill);
                        if (!difference.Text.IsNone) text[(row, column)] = difference.Text;
                    }
                }
            }
        }

        return text;
    }

    /// <summary>One <c>&lt;dxf&gt;</c>: what it changes about the text, and its fill if it has one.</summary>
    private readonly record struct Difference(SheetConditionalText Text, Colour? Background);

    /// <summary>One <c>cfRule</c> that names a <c>dxf</c>.</summary>
    private sealed record Rule(
        int Block,
        int Priority,
        List<SheetRange> Ranges,
        int AnchorRow,
        int AnchorColumn,
        Difference Difference,
        ICondition Test);

    /// <summary>The <c>dxfs</c> table, in the order <c>dxfId</c> indexes it.</summary>
    private static List<Difference> ReadDifferences(XElement styles, XlsxPalette palette)
    {
        List<Difference> differences = [];

        foreach (XElement dxf in Xlsx.Children(Xlsx.Child(styles, "dxfs"), "dxf"))
        {
            XElement? font = Xlsx.Child(dxf, "font");
            SheetConditionalText text = font is null ? default : new SheetConditionalText
            {
                Colour = palette.Read(Xlsx.Child(font, "color")),
                IsStruckThrough = Toggle(Xlsx.Child(font, "strike")),
                FontWeight = Toggle(Xlsx.Child(font, "b")) switch
                {
                    true => 700,
                    false => 400,
                    null => null,
                },
                IsItalic = Toggle(Xlsx.Child(font, "i")),
                Underline = Xlsx.Child(font, "u") is { } underline
                    ? XlsxCellFormats.UnderlineOf(underline)
                    : null,
                FontSize = Number(Xlsx.Child(font, "sz")) is > 0 and { } points
                    ? Length.FromPoints(points)
                    : null,
                FontFamily = Xlsx.Attribute(Xlsx.Child(font, "name"), "val"),
            };

            differences.Add(new Difference(
                text, XlsxPatternFill.Resolve(Xlsx.Child(dxf, "fill"), palette, differential: true)));
        }

        return differences;
    }

    /// <summary>Every rule that names a <c>dxf</c> and whose condition can be evaluated.</summary>
    private static List<Rule> ReadRules(
        XElement worksheet,
        List<Difference> differences,
        string sheetName,
        Dictionary<string, string> names)
    {
        List<Rule> rules = [];
        int index = -1;

        foreach (XElement block in Xlsx.Children(worksheet, "conditionalFormatting"))
        {
            index++;
            List<SheetRange> ranges = ParseSqref(Xlsx.Attribute(block, "sqref"));
            if (ranges.Count == 0) continue;

            // The formula is written for the top-left cell of the whole `sqref` and is shifted
            // for every other cell in it, which is what `calcext:base-cell-address` states in
            // 26.2.4.2's own view of the file. Which cell that is, for a `sqref` naming several
            // ranges, is `ScRangeList::GetTopLeftCorner` — see `TopLeftCorner`.
            (int anchorRow, int anchorColumn) = TopLeftCorner(ranges);

            foreach (XElement rule in Xlsx.Children(block, "cfRule"))
            {
                if (Xlsx.Integer(rule, "dxfId") is not { } id) continue;
                if (id < 0 || id >= differences.Count) continue;

                Difference difference = differences[id];
                if (difference.Text.IsNone && difference.Background is null) continue;

                if (ConditionOf(rule, ranges, sheetName, names, anchorRow, anchorColumn)
                    is not { } test) continue;

                rules.Add(new Rule(
                    index,
                    Xlsx.Integer(rule, "priority") ?? int.MaxValue,
                    ranges, anchorRow, anchorColumn, difference, test));
            }
        }

        return rules;
    }

    /// <summary>
    /// The workbook-level defined names, by name, as the text they expand to.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A conditional-format rule may be written entirely in terms of names — the corpus's Gantt
    /// planner states <c>Plan</c>, <c>Actual</c> and six more, each of which refers to others —
    /// so a reader that stops at a bare identifier evaluates the whole chart as nothing.
    /// </para>
    /// <para>
    /// <strong>Sheet-scoped names are skipped.</strong> A <c>localSheetId</c> name resolves
    /// against the sheet that declares it, which this does not model, and taking one for a
    /// workbook name would resolve a rule against the wrong sheet's cells. The corpus's are all
    /// workbook scope.
    /// </para>
    /// </remarks>
    private static Dictionary<string, string> DefinedNames(XElement? workbook)
    {
        Dictionary<string, string> names = new(StringComparer.OrdinalIgnoreCase);
        if (workbook is null) return names;

        foreach (XElement element in Xlsx.Children(
                     Xlsx.Child(workbook, "definedNames"), "definedName"))
        {
            if (Xlsx.Attribute(element, "localSheetId") is not null) continue;
            if (Xlsx.Attribute(element, "name") is not { Length: > 0 } name) continue;

            // `_xlnm.Print_Area` and its siblings are ranges rather than expressions and are read
            // by `XlsxPrintNames`; letting one through here would make a rule naming it parse.
            if (name.StartsWith("_xlnm.", StringComparison.Ordinal)) continue;

            if (element.Value.Trim() is { Length: > 0 } body) names[name] = body;
        }

        return names;
    }

    /// <summary>The condition one <c>cfRule</c> states, or null when it cannot be evaluated.</summary>
    /// <remarks>
    /// The arms are <c>CondFormatRule::finalizeImport</c>'s
    /// (<c>sc/source/filter/oox/condformatbuffer.cxx</c>:799-905), which is where a
    /// <c>cfRule</c>'s <c>type</c> becomes an <c>ScConditionMode</c>. Two of them are
    /// not the reading the specification suggests and both are measured against 26.2.4.2's own
    /// view of the corpus documents that state them — see <see cref="TextPredicate"/> and
    /// <see cref="BlankPredicate"/>.
    /// </remarks>
    private static ICondition? ConditionOf(
        XElement rule,
        List<SheetRange> ranges,
        string sheetName,
        Dictionary<string, string> names,
        int anchorRow,
        int anchorColumn)
    {
        List<string> formulas = [.. Xlsx.Children(rule, "formula").Select(static f => f.Value)];

        switch (Xlsx.Attribute(rule, "type"))
        {
            case "expression":
                if (formulas.Count != 1) return null;

                // The two-operand reader first, and deliberately: it is measured, it is what the
                // great majority of these rules are, and keeping it in front means the evaluator
                // below can only reach a formula that used to paint nothing at all. That is this
                // round's confinement guarantee, and it is structural rather than measured.
                return (ICondition?)Comparison.Parse(formulas[0])
                       ?? Expression.Parse(formulas[0], sheetName, names, anchorRow, anchorColumn);

            case "cellIs":
                return CellIs.Parse(Xlsx.Attribute(rule, "operator"), formulas);

            case "containsText":
                return TextPredicate.Of(TextMatch.Contains, Xlsx.Attribute(rule, "text"));

            case "endsWith":
                return TextPredicate.Of(TextMatch.EndsWith, Xlsx.Attribute(rule, "text"));

            case "containsBlanks":
                return BlankPredicate.Blank;

            case "notContainsBlanks":
                return BlankPredicate.NotBlank;

            case "duplicateValues":
                return new DuplicatePredicate(ranges);

            default:
                return null;
        }
    }

    /// <summary>
    /// The cell a <c>sqref</c>'s formulas are written for.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>ScRangeList::GetTopLeftCorner</c> (<c>sc/source/core/tool/rangelst.cxx</c>:1142-1155)
    /// is the smallest range <em>start</em> under <c>ScAddress</c>'s own ordering, and that
    /// ordering is <c>(tab, col, row)</c> — <c>std::make_tuple(nTab, nCol, nRow) &lt;=&gt; …</c>,
    /// <c>sc/inc/address.hxx</c>:396. So it is the start with the smallest column, ties broken by
    /// the smallest row, and <strong>not</strong> the componentwise minimum.
    /// </para>
    /// <para>
    /// The two differ only for a <c>sqref</c> naming several ranges, and they differ from Excel:
    /// Excel writes the formula against the componentwise minimum. On
    /// <c>Application_Compliance_Checklist_5_Apr_2021.xlsx</c>, <c>$G376="N/A"</c> is stated on
    /// <c>G443:G444 D491 G446:G490 G377</c>, whose componentwise minimum is <c>D377</c> and whose
    /// top-left corner is <c>D491</c>; 26.2.4.2's own <c>--convert-to fods</c> writes that rule
    /// out with <c>calcext:base-cell-address="…D491"</c>, and gives the sibling block anchored at
    /// <c>A377</c> a resolved <c>formula-is(#ref!="N/A")</c> because the shift takes the row
    /// negative. 15 blocks in 3 corpus documents are affected. The reference is what this
    /// reproduces.
    /// </para>
    /// </remarks>
    private static (int Row, int Column) TopLeftCorner(List<SheetRange> ranges)
    {
        int row = ranges[0].FirstRow;
        int column = ranges[0].FirstColumn;

        foreach (SheetRange range in ranges)
        {
            if (range.FirstColumn < column || (range.FirstColumn == column && range.FirstRow < row))
            {
                row = range.FirstRow;
                column = range.FirstColumn;
            }
        }

        return (row, column);
    }

    /// <summary>Which end of a cell's text a <see cref="TextPredicate"/> looks at.</summary>
    private enum TextMatch
    {
        /// <summary>Anywhere in it — a <c>containsText</c> rule.</summary>
        Contains,

        /// <summary>At the end of it — an <c>endsWith</c> rule.</summary>
        EndsWith,
    }

    /// <summary>
    /// A <c>containsText</c> or <c>endsWith</c> rule, which tests the cell's own text against the
    /// rule's <c>text</c> attribute.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>The <c>&lt;formula&gt;</c> such a rule states is not read at all.</strong> Excel
    /// writes <c>NOT(ISERROR(SEARCH("xxx",A373)))</c> beside the <c>text</c> attribute and
    /// <c>finalizeImport</c> ignores it: the <c>ContainsText</c>/<c>EndsWith</c> branch builds a
    /// token array holding <em>only</em> <c>maModel.maText</c>, interned
    /// (<c>condformatbuffer.cxx</c>:938-947), which is why <c>--convert-to fods</c> writes these
    /// out as <c>contains-text("xxx")</c> with no reference in them. Reading the formula instead
    /// would need a <c>SEARCH</c> and an <c>ISERROR</c> to answer what one attribute states.
    /// </para>
    /// <para>
    /// <strong>A numeric cell is matched case-sensitively and a text cell is not.</strong>
    /// <c>ScConditionEntry::IsValidStr</c> (<c>conditio.cxx</c>:1219-1229) lowercases both sides
    /// through <c>ScGlobal::getCharClass()</c> unless the document is case-sensitive, which it is
    /// not by default; <c>IsValid</c>, the numeric arm (<c>:1130-1143</c>), stringifies the cell
    /// with <c>OUString::number(nArg)</c> and calls a bare <c>indexOf</c> with no lowercasing at
    /// all. The asymmetry is the reference's.
    /// </para>
    /// <para>
    /// <strong>And a numeric cell is matched against the number, not against what is drawn.</strong>
    /// <c>OUString::number</c> knows nothing of the cell's format, so <c>1234.5</c> displayed as
    /// a currency is searched as <c>1234.5</c>.
    /// </para>
    /// <para>
    /// An empty cell reaches <c>IsValidStr("")</c>, because <c>lcl_GetCellContent</c> answers
    /// <c>!bIsStr1</c> for one (<c>:770-771</c>) and a text rule's operand is a string — so it
    /// contains nothing and ends with nothing, and would match a <c>notContainsText</c> rule.
    /// </para>
    /// </remarks>
    private sealed record TextPredicate(TextMatch Match, string Needle, string LowerNeedle) : ICondition
    {
        /// <summary>Reads one, or null when the rule states no text to look for.</summary>
        /// <param name="match">Which end of the cell's text to look at.</param>
        /// <param name="text">The rule's <c>text</c> attribute.</param>
        /// <remarks>
        /// An absent or empty <c>text</c> is refused rather than modelled. The reference does
        /// something with it — an empty <c>aStrVal1</c> sends <c>IsValid</c> down a branch that
        /// searches for <c>OUString::number(nVal1)</c>, which is the string <c>0</c> — and no
        /// corpus rule states one, so reproducing that corner would be untested by construction.
        /// </remarks>
        public static TextPredicate? Of(TextMatch match, string? text)
            => string.IsNullOrEmpty(text)
                ? null
                : new TextPredicate(match, text, text.ToLowerInvariant());

        /// <inheritdoc/>
        public bool Holds(Sheet sheet, int row, int column, int anchorRow, int anchorColumn)
        {
            Value cell = sheet.At(row, column);

            if (cell.Number is { } number)
            {
                string drawn = NumberText(number);
                return Match is TextMatch.Contains
                    ? drawn.Contains(Needle, StringComparison.Ordinal)
                    : drawn.EndsWith(Needle, StringComparison.Ordinal);
            }

            string value = (cell.Text ?? string.Empty).ToLowerInvariant();
            return Match is TextMatch.Contains
                ? value.Contains(LowerNeedle, StringComparison.Ordinal)
                : value.EndsWith(LowerNeedle, StringComparison.Ordinal);
        }
    }

    /// <summary>
    /// A <c>containsBlanks</c> or <c>notContainsBlanks</c> rule.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>Neither is a condition mode at all.</strong> <c>finalizeImport</c>
    /// (<c>condformatbuffer.cxx</c>:860-866) gives them a <em>replacement formula</em> —
    /// <c>LEN(TRIM(#B))=0</c> and <c>LEN(TRIM(#B))&gt;0</c>, where <c>#B</c> is the relative
    /// address of the range list's top-left corner — and then sets
    /// <c>ScConditionMode::Direct</c>. 26.2.4.2's own view of
    /// <c>Application_Compliance_Checklist_5_Apr_2021.xlsx</c>, which states 99 of them, writes
    /// each out as <c>formula-is(LEN(TRIM([.M270]))=0)</c>.
    /// </para>
    /// <para>
    /// <strong>So a cell holding only spaces is blank here, and the specification's rule is that
    /// it is not.</strong> <c>TRIM</c> strips the leading and trailing spaces before <c>LEN</c>
    /// measures what is left. A number is never blank — <c>TRIM</c> stringifies it first, and
    /// even a zero is one character.
    /// </para>
    /// <para>
    /// Because the reference's replacement formula names the base cell itself and is then shifted
    /// by <c>rPos - base</c> like any relative reference, it resolves to the cell being tested;
    /// so this is a predicate on that cell and the anchor does not enter it.
    /// </para>
    /// </remarks>
    private sealed record BlankPredicate(bool Wanted) : ICondition
    {
        /// <summary>The <c>containsBlanks</c> spelling.</summary>
        public static BlankPredicate Blank { get; } = new(true);

        /// <summary>The <c>notContainsBlanks</c> spelling.</summary>
        public static BlankPredicate NotBlank { get; } = new(false);

        /// <inheritdoc/>
        public bool Holds(Sheet sheet, int row, int column, int anchorRow, int anchorColumn)
        {
            Value cell = sheet.At(row, column);
            bool blank = cell.Number is null
                         && (cell.Text is null || cell.Text.AsSpan().Trim(' ').Length == 0);
            return blank == Wanted;
        }
    }

    /// <summary>
    /// A <c>duplicateValues</c> rule, which holds where the cell's value occurs more than once
    /// inside the rule's own range.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>ScConditionEntry::FillCache</c> (<c>conditio.cxx</c>:804-859) walks
    /// <c>pCondFormat->GetRange()</c> — the whole <c>sqref</c>, not the sheet — skipping every
    /// empty cell, and counts numbers in one map keyed by value and strings in another keyed by
    /// <c>ScGlobal::getCharClass().lowercase(…)</c>. <c>IsDuplicate</c> (<c>:861-885</c>) then
    /// answers whether that cell's own key was counted more than once, so the two kinds never
    /// collide: the number 1 and the string <c>1</c> are not duplicates of each other.
    /// </para>
    /// <para>
    /// A cell holding the empty string matches nothing: <c>IsValidStr</c> guards the duplicate
    /// arm with <c>!rArg.isEmpty()</c> (<c>:1163</c>) and then falls through to
    /// <c>if (!bIsStr1) return eOp == NotEqual</c>, which is false for a rule with no operand.
    /// </para>
    /// <para>
    /// The reference clamps its own walk too, and for the same reason: a range reaching
    /// <c>MaxRow</c> is shrunk to the used data area under a comment calling it a temporary fix
    /// for slow duplicate conditions (<c>:820-828</c>).
    /// </para>
    /// </remarks>
    private sealed class DuplicatePredicate(List<SheetRange> ranges) : ICondition
    {
        private readonly List<SheetRange> _ranges = ranges;
        private Dictionary<double, int>? _numbers;
        private Dictionary<string, int>? _strings;

        /// <inheritdoc/>
        public bool Holds(Sheet sheet, int row, int column, int anchorRow, int anchorColumn)
        {
            Fill(sheet);

            Value cell = sheet.At(row, column);
            if (cell.Number is { } number)
                return _numbers!.TryGetValue(number, out int seen) && seen > 1;

            if (cell.Text is not { Length: > 0 } text) return false;
            return _strings!.TryGetValue(text.ToLowerInvariant(), out int times) && times > 1;
        }

        private void Fill(Sheet sheet)
        {
            if (_numbers is not null) return;

            _numbers = [];
            _strings = [];

            foreach (SheetRange range in _ranges)
            {
                int firstRow = Math.Max(0, range.FirstRow);
                int lastRow = Math.Min(sheet.LastRow, range.LastRow);
                int firstColumn = Math.Max(0, range.FirstColumn);
                int lastColumn = Math.Min(sheet.LastColumn, range.LastColumn);

                for (int row = firstRow; row <= lastRow; row++)
                {
                    for (int column = firstColumn; column <= lastColumn; column++)
                    {
                        Value cell = sheet.At(row, column);
                        if (cell.Number is { } number)
                        {
                            _numbers[number] = _numbers.TryGetValue(number, out int seen) ? seen + 1 : 1;
                        }
                        else if (cell.Text is { } text)
                        {
                            string key = text.ToLowerInvariant();
                            _strings[key] = _strings.TryGetValue(key, out int times) ? times + 1 : 1;
                        }
                    }
                }
            }
        }
    }

    /// <summary>
    /// What <c>OUString::number</c> makes of a cell's value, which is what a text rule searches
    /// when the cell holds a number.
    /// </summary>
    /// <remarks>
    /// <c>rtl::math::doubleToUString</c> at <c>rtl_math_StringFormat_Automatic</c> — the shortest
    /// decimal that round-trips, with no thousands separator, no exponent below the range where
    /// one is needed and a <c>.</c> for the point. The round-trip <c>R</c> format agrees with it
    /// over every value a corpus cell holds; where the two could part company is a magnitude at
    /// which the reference switches to an exponent, and no rule in the corpus searches a number
    /// at all.
    /// </remarks>
    private static string NumberText(double number)
        => number.ToString("R", CultureInfo.InvariantCulture);

    /// <summary>Reads every stated cell of one worksheet into the shape the rules ask against.</summary>
    /// <param name="worksheet">The sheet's own root.</param>
    /// <param name="shared">The workbook's shared strings.</param>
    private static Sheet ReadSheet(XElement worksheet, XlsxSharedStrings shared)
    {
        Sheet sheet = new();

        // A bounded `<col>` run as well as the cells the sheet writes. A column that holds no
        // cell at all is still printed when a run formats it, and a rule covering it would
        // otherwise be clamped away — while the sheet's own `dimension` is not used, because a
        // file stating `A1:XFD1048576` would put every rule past the budget below and turn the
        // whole feature off on it.
        foreach (XElement run in Xlsx.Children(Xlsx.Child(worksheet, "cols"), "col"))
        {
            int last = (Xlsx.Integer(run, "max") ?? 0) - 1;
            if (last >= 0 && last < SheetAddress.MaxColumn) sheet.Extend(-1, last);
        }

        int expectedRow = 0;
        foreach (XElement row in Xlsx.Children(Xlsx.Child(worksheet, "sheetData"), "row"))
        {
            int index = (Xlsx.Integer(row, "r") - 1) ?? expectedRow;
            if (index < 0) index = expectedRow;
            expectedRow = index + 1;

            sheet.Extend(index, -1);

            int expectedColumn = 0;
            foreach (XElement cell in Xlsx.Children(row, "c"))
            {
                int at = index;
                int column;
                if (Xlsx.TryParseCellReference(Xlsx.Attribute(cell, "r"), out int parsed, out int parsedRow))
                {
                    column = parsed;
                    at = parsedRow;
                }
                else
                {
                    column = expectedColumn;
                }

                expectedColumn = column + 1;

                sheet.Set(at, column, ValueOf(cell, shared) ?? Value.Blank);
            }
        }

        return sheet;
    }

    private static Value? ValueOf(XElement cell, XlsxSharedStrings shared)
    {
        string type = Xlsx.Attribute(cell, "t") ?? "n";

        switch (type)
        {
            case "s":
                return Xlsx.Child(cell, "v")?.Value is { } key
                       && int.TryParse(key, NumberStyles.Integer, CultureInfo.InvariantCulture,
                                       out int index)
                    ? Value.OfText(shared.StoredAt(index) ?? string.Empty)
                    : null;

            case "str":
                return Xlsx.Child(cell, "v")?.Value is { } literal ? Value.OfText(literal) : null;

            case "inlineStr":
                XlsxSharedStrings.ReadRichString(Xlsx.Child(cell, "is"), out string inline);
                return Value.OfText(inline);

            case "e":
                return null;

            default:
                return Xlsx.Child(cell, "v")?.Value is { } text
                       && double.TryParse(text, NumberStyles.Float, CultureInfo.InvariantCulture,
                                          out double number)
                    ? Value.OfNumber(number)
                    : null;
        }
    }

    /// <summary>A <c>dxf</c> toggle, which is absent when the rule does not change it.</summary>
    /// <remarks>
    /// Three states rather than two, which is what makes a <c>dxf</c> differential: no element
    /// leaves the cell's own answer alone, a bare element turns the property on, and an explicit
    /// zero turns it off. 287 of one corpus document's 413 rules turn a strikethrough off.
    /// </remarks>
    private static bool? Toggle(XElement? element)
    {
        if (element is null) return null;

        string? value = Xlsx.Attribute(element, "val");
        return value is null || (value is not "0" && !string.Equals(value, "false", StringComparison.OrdinalIgnoreCase));
    }

    private static double? Number(XElement? element)
        => double.TryParse(Xlsx.Attribute(element, "val"), NumberStyles.Float,
                           CultureInfo.InvariantCulture, out double parsed)
            ? parsed
            : null;

    /// <summary>The ranges an <c>sqref</c> names, which is a space-separated list of A1 ranges.</summary>
    private static List<SheetRange> ParseSqref(string? sqref)
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
}
