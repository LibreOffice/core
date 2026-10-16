using System.Globalization;

namespace XRay.Office.Spreadsheets.Layout;

/// <summary>
/// The part of a conditional-format rule that does not depend on how the rule was spelled: the
/// values a sheet holds, the operands a condition names, and the comparison the reference makes
/// between them.
/// </summary>
/// <remarks>
/// <para>
/// Every one of these was read out of Calc's own evaluator for the OOXML reader and is cited
/// there against <c>sc/source/core/tool/conditio.cxx</c> and
/// <c>sc/source/filter/oox/condformatbuffer.cxx</c>. They live here rather than in
/// <c>XlsxConditionalStyles</c> because a BIFF <c>CF</c> record states the same conditions in
/// a different notation — an RPN token array instead of an infix formula and an
/// <c>operator</c> attribute — and reaches exactly the same
/// <c>ScCondFormatEntry</c>/<c>ScConditionMode</c> pair
/// (<c>XclImpCondFormat::ReadCF</c>, <c>sc/source/filter/excel/xicontent.cxx</c>:526-713).
/// Two spellings of one predicate must not be two implementations of it: the comparison rules
/// below are subtle — a blank equals zero and equals the empty string, text is compared without
/// regard to case, a number never equals a string — and were measured once.
/// </para>
/// <para>
/// The container is a static class rather than a namespace so that the members keep the short
/// names the OOXML reader uses for them, which are aliased back in at its top.
/// </para>
/// </remarks>
internal static class SheetConditions
{
    /// <summary>A condition that can be asked about one cell.</summary>
    internal interface ICondition
    {
        /// <summary>Whether the rule holds at one position.</summary>
        /// <param name="sheet">The sheet's values.</param>
        /// <param name="row">The zero-based row being tested.</param>
        /// <param name="column">The zero-based column being tested.</param>
        /// <param name="anchorRow">The row the formula was written for.</param>
        /// <param name="anchorColumn">The column the formula was written for.</param>
        bool Holds(Sheet sheet, int row, int column, int anchorRow, int anchorColumn);
    }

    /// <summary>What a cell or a literal is worth: a number, a string, or nothing at all.</summary>
    internal readonly record struct Value(double? Number, string? Text)
    {
        /// <summary>An empty cell, which equals neither a number nor a non-empty string.</summary>
        public static Value Blank { get; }

        /// <summary>A numeric value.</summary>
        /// <param name="number">The number.</param>
        public static Value OfNumber(double number) => new(number, null);

        /// <summary>A text value.</summary>
        /// <param name="text">The string.</param>
        public static Value OfText(string text) => new(null, text);

        /// <summary>True when the cell holds nothing.</summary>
        public bool IsBlank => Number is null && Text is null;

        /// <summary>
        /// Compares two values the way a condition does.
        /// </summary>
        /// <remarks>
        /// <para>
        /// Text against text is compared without regard to case, which is Calc's default: the
        /// document option <c>IsIgnoreCase</c> is on unless a workbook turns it off
        /// (<c>ScDocOptions</c>), and every corpus rule of this shape tests a one-letter marker.
        /// </para>
        /// <para>
        /// A number and a string are never equal and a number always sorts first, which is the
        /// ordering <c>ScInterpreter::CompareFunc</c> uses. A blank equals a blank, equals the
        /// number zero and equals the empty string, and is otherwise less than anything.
        /// </para>
        /// </remarks>
        /// <param name="left">The left side.</param>
        /// <param name="right">The right side.</param>
        /// <param name="op">One of <c>=</c>, <c>&lt;&gt;</c>, <c>&lt;</c>, <c>&lt;=</c>, <c>&gt;</c>, <c>&gt;=</c>.</param>
        public static bool Compare(Value left, Value right, string op)
        {
            int order = Order(left, right);

            return op switch
            {
                "=" => order == 0,
                "<>" => order != 0,
                "<" => order < 0,
                "<=" => order <= 0,
                ">" => order > 0,
                ">=" => order >= 0,
                _ => false,
            };
        }

        private static int Order(Value left, Value right)
        {
            if (left.IsBlank && right.IsBlank) return 0;
            if (left.IsBlank) return right.Number is { } n ? 0.0.CompareTo(n) : right.Text!.Length == 0 ? 0 : -1;
            if (right.IsBlank) return left.Number is { } m ? m.CompareTo(0.0) : left.Text!.Length == 0 ? 0 : 1;

            if (left.Number is { } a && right.Number is { } b) return a.CompareTo(b);
            if (left.Text is { } s && right.Text is { } t)
                return string.Compare(s, t, StringComparison.OrdinalIgnoreCase);

            return left.Number is not null ? -1 : 1;
        }
    }

    /// <summary>One side of a comparison: a cell reference, a number, a string or a boolean.</summary>
    internal sealed record Operand(
        bool IsReference,
        int Row,
        int Column,
        bool AbsoluteRow,
        bool AbsoluteColumn,
        Value Literal)
    {
        /// <summary>Reads one, or null when it is anything more than a reference or a literal.</summary>
        /// <param name="text">The operand's text, with any surrounding space.</param>
        public static Operand? Parse(string? text)
        {
            if (string.IsNullOrWhiteSpace(text)) return null;

            string one = text.Trim();
            if (one.Length == 0) return null;

            if (one[0] == '"')
            {
                if (one.Length < 2 || one[^1] != '"') return null;
                string body = one[1..^1].Replace("\"\"", "\"", StringComparison.Ordinal);
                if (body.Contains('"', StringComparison.Ordinal)) return null;
                return OfLiteral(Value.OfText(body));
            }

            if (double.TryParse(one, NumberStyles.Float, CultureInfo.InvariantCulture, out double number))
                return OfLiteral(Value.OfNumber(number));

            if (string.Equals(one, "TRUE", StringComparison.OrdinalIgnoreCase))
                return OfLiteral(Value.OfNumber(1));
            if (string.Equals(one, "FALSE", StringComparison.OrdinalIgnoreCase))
                return OfLiteral(Value.OfNumber(0));

            return Reference(one);
        }

        /// <summary>What this side is worth at a position, given the shift from the anchor.</summary>
        /// <param name="sheet">The sheet's values.</param>
        /// <param name="rowShift">Rows from the cell the formula was written for.</param>
        /// <param name="columnShift">Columns from it.</param>
        public Value Resolve(Sheet sheet, int rowShift, int columnShift)
        {
            if (!IsReference) return Literal;

            int row = AbsoluteRow ? Row : Row + rowShift;
            int column = AbsoluteColumn ? Column : Column + columnShift;
            return row < 0 || column < 0 ? Value.Blank : sheet.At(row, column);
        }

        private static Operand OfLiteral(Value value) => new(false, 0, 0, false, false, value);

        /// <summary>
        /// An A1 reference, with the two <c>$</c> flags kept.
        /// </summary>
        /// <remarks>
        /// A rule's formula is stated for the top-left cell of its range and shifted for the
        /// rest, so which halves are absolute is the whole of what a rule over a column means:
        /// <c>H2="x"</c> on <c>S2:S900</c> tests a different <c>H</c> on every row, and
        /// <c>$H$2="x"</c> would test one cell for all of them. A reference naming a sheet is
        /// refused — nothing here can reach another sheet's values.
        /// </remarks>
        private static Operand? Reference(string text)
        {
            if (text.Contains('!', StringComparison.Ordinal)) return null;

            int at = 0;
            bool absoluteColumn = false;
            if (text[at] == '$') { absoluteColumn = true; at++; }

            int letters = at;
            while (letters < text.Length && char.IsAsciiLetter(text[letters])) letters++;
            if (letters == at || letters - at > 3) return null;

            int digitsStart = letters;
            bool absoluteRow = false;
            if (digitsStart < text.Length && text[digitsStart] == '$') { absoluteRow = true; digitsStart++; }

            if (digitsStart >= text.Length) return null;
            for (int i = digitsStart; i < text.Length; i++)
            {
                if (!char.IsAsciiDigit(text[i])) return null;
            }

            int column = 0;
            for (int i = at; i < letters; i++)
                column = (column * 26) + (char.ToUpperInvariant(text[i]) - 'A' + 1);

            if (!int.TryParse(text[digitsStart..], NumberStyles.Integer, CultureInfo.InvariantCulture,
                              out int row) || row < 1)
            {
                return null;
            }

            return new Operand(true, row - 1, column - 1, absoluteRow, absoluteColumn, Value.Blank);
        }
    }

    /// <summary>
    /// An <c>expression</c> rule whose formula is more than two operands and a comparison.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Reached only where <see cref="Comparison"/> refuses, which is the confinement guarantee of
    /// the round that added it: every formula this evaluates used to paint nothing at all.
    /// </para>
    /// <para>
    /// <strong>The rule fires when the formula's value is a non-zero NUMBER</strong>, not when it
    /// is exactly true. <c>ScConditionEntry::IsCellValid</c> is
    /// <c>if (eOp == ScConditionMode::Direct) return nVal1 != 0.0;</c>
    /// (<c>sc/source/core/data/conditio.cxx</c>:1272-1278), and <c>Interpret</c> sets
    /// <c>nVal1 = 0.0</c> whenever the result is a string (<c>:714-719</c>) — so a text-valued
    /// formula never fires, and <c>MOD(COLUMN(),2)</c> fires on every odd column without ever
    /// being a boolean. It is also what lets a rule multiply comparisons together, which is how
    /// a Gantt planner states its bars.
    /// </para>
    /// </remarks>
    /// <param name="Root">The parsed formula.</param>
    internal sealed record Expression(SheetFormula.Node Root) : ICondition
    {
        /// <summary>Reads one, or null when the formula is outside the grammar.</summary>
        /// <param name="formula">The rule's <c>&lt;formula&gt;</c> text.</param>
        /// <param name="sheetName">The sheet the rule is on, for a qualified reference.</param>
        /// <param name="names">The workbook's defined names, by name.</param>
        /// <param name="anchorRow">The zero-based row the rule's formula is written for.</param>
        /// <param name="anchorColumn">The zero-based column it is written for.</param>
        public static Expression? Parse(
            string? formula, string sheetName, Dictionary<string, string> names,
            int anchorRow, int anchorColumn)
        {
            ArgumentNullException.ThrowIfNull(names);

            return SheetFormula.Parse(
                       formula, sheetName, name => names.GetValueOrDefault(name),
                       anchorRow, anchorColumn)
                is { } root
                ? new Expression(root)
                : null;
        }

        /// <inheritdoc/>
        public bool Holds(Sheet sheet, int row, int column, int anchorRow, int anchorColumn)
            => SheetFormula.Evaluate(
                   Root, sheet, row, column, row - anchorRow, column - anchorColumn)
               is { Number: { } value } && value != 0.0;
    }

    /// <summary>An <c>expression</c> rule of the shape <c>&lt;operand&gt; &lt;op&gt; &lt;operand&gt;</c>.</summary>
    internal sealed record Comparison(Operand Left, string Operator, Operand Right) : ICondition
    {
        /// <summary>Reads one, or null when the formula is anything else.</summary>
        /// <param name="formula">The rule's <c>&lt;formula&gt;</c> text.</param>
        public static Comparison? Parse(string? formula)
        {
            if (string.IsNullOrWhiteSpace(formula)) return null;

            string text = formula.Trim();
            if (text.StartsWith('=')) text = text[1..];

            foreach (string op in Operators)
            {
                int at = IndexOfOperator(text, op);
                if (at < 0) continue;

                if (Operand.Parse(text[..at]) is not { } left) return null;
                if (Operand.Parse(text[(at + op.Length)..]) is not { } right) return null;
                return new Comparison(left, op, right);
            }

            return null;
        }

        /// <inheritdoc/>
        public bool Holds(Sheet sheet, int row, int column, int anchorRow, int anchorColumn)
        {
            int rowShift = row - anchorRow;
            int columnShift = column - anchorColumn;

            Value left = Left.Resolve(sheet, rowShift, columnShift);
            Value right = Right.Resolve(sheet, rowShift, columnShift);
            return Value.Compare(left, right, Operator);
        }

        /// <summary>Longest first, so <c>&lt;=</c> is not read as <c>&lt;</c>.</summary>
        private static readonly string[] Operators = ["<>", "<=", ">=", "=", "<", ">"];

        /// <summary>
        /// Where an operator sits, ignoring the ones inside a quoted string.
        /// </summary>
        /// <remarks>
        /// A single pass rather than a split, because <c>A1="&lt;"</c> is legal and a naive
        /// <c>IndexOf</c> finds the wrong character in it. Parentheses are counted as well: a
        /// formula whose top level is inside a call is not a comparison this understands, and
        /// finding an operator inside one would evaluate half a function.
        /// </remarks>
        private static int IndexOfOperator(string text, string op)
        {
            int depth = 0;
            bool quoted = false;

            for (int i = 0; i < text.Length; i++)
            {
                char c = text[i];
                if (c == '"') { quoted = !quoted; continue; }
                if (quoted) continue;
                if (c == '(') { depth++; continue; }
                if (c == ')') { depth--; continue; }
                if (depth != 0) continue;

                if (i + op.Length <= text.Length
                    && string.CompareOrdinal(text, i, op, 0, op.Length) == 0)
                {
                    return i;
                }
            }

            return -1;
        }
    }

    /// <summary>
    /// A <c>cellIs</c> rule, which compares the cell being formatted against one or two operands.
    /// </summary>
    internal sealed record CellIs(string Operator, Operand First, Operand? Second) : ICondition
    {
        /// <summary>Reads one, or null when the operator or the operands are not understood.</summary>
        /// <param name="op">The rule's <c>operator</c> attribute.</param>
        /// <param name="formulas">Its <c>&lt;formula&gt;</c> children, one or two of them.</param>
        public static CellIs? Parse(string? op, List<string> formulas)
        {
            if (op is null || formulas.Count == 0) return null;

            bool two = op is "between" or "notBetween";
            if (formulas.Count < (two ? 2 : 1)) return null;
            if (Operand.Parse(formulas[0]) is not { } first) return null;

            Operand? second = null;
            if (two)
            {
                if (Operand.Parse(formulas[1]) is not { } parsed) return null;
                second = parsed;
            }

            return op is "equal" or "notEqual" or "greaterThan" or "lessThan"
                       or "greaterThanOrEqual" or "lessThanOrEqual" or "between" or "notBetween"
                ? new CellIs(op, first, second)
                : null;
        }

        /// <inheritdoc/>
        public bool Holds(Sheet sheet, int row, int column, int anchorRow, int anchorColumn)
        {
            int rowShift = row - anchorRow;
            int columnShift = column - anchorColumn;

            Value cell = sheet.At(row, column);
            Value first = First.Resolve(sheet, rowShift, columnShift);

            switch (Operator)
            {
                case "equal": return Value.Compare(cell, first, "=");
                case "notEqual": return Value.Compare(cell, first, "<>");
                case "greaterThan": return Value.Compare(cell, first, ">");
                case "lessThan": return Value.Compare(cell, first, "<");
                case "greaterThanOrEqual": return Value.Compare(cell, first, ">=");
                case "lessThanOrEqual": return Value.Compare(cell, first, "<=");
                default:
                    if (Second is not { } upper) return false;
                    Value second = upper.Resolve(sheet, rowShift, columnShift);
                    bool inside = Value.Compare(cell, first, ">=") && Value.Compare(cell, second, "<=");
                    return Operator is "between" ? inside : !inside;
            }
        }
    }

    /// <summary>A sheet's values, by position, for the rules to be asked against.</summary>
    /// <remarks>
    /// <para>
    /// A formula cell's cached <c>v</c> is what is read, on the same terms as every other reader
    /// here: the reference recalculates, so a rule testing a volatile formula is the divergence
    /// this project already records rather than one this introduces.
    /// </para>
    /// <para>
    /// <strong>A string cell holds the text Calc stores, not the text it draws.</strong>
    /// <c>RichStringPortion::setText</c> decodes the <c>_xHHHH_</c> escapes and stops
    /// (<c>sc/source/filter/oox/richstring.cxx</c>:60-63), so a control character the drawing
    /// layer elides is still in the string every condition here compares — see
    /// <c>XlsxCellText.Stored</c>. The <c>str</c> arm is the one exception, and only
    /// because it was never decoded either way and no corpus cell reaches it.
    /// </para>
    /// </remarks>
    internal sealed class Sheet
    {
        private readonly Dictionary<(int Row, int Column), Value> _values = [];

        /// <summary>The last row the sheet states anything on, or -1 for an empty sheet.</summary>
        public int LastRow { get; private set; } = -1;

        /// <summary>The last column it states anything on, or -1.</summary>
        public int LastColumn { get; private set; } = -1;

        /// <summary>Widens the walked extent to a rectangle the sheet declares.</summary>
        /// <param name="row">A row the sheet reaches.</param>
        /// <param name="column">A column it reaches.</param>
        private void Reaches(int row, int column)
        {
            if (row > LastRow) LastRow = row;
            if (column > LastColumn) LastColumn = column;
        }

        /// <summary>What one cell holds.</summary>
        /// <param name="row">The zero-based row.</param>
        /// <param name="column">The zero-based column.</param>
        public Value At(int row, int column)
            => _values.TryGetValue((row, column), out Value value) ? value : Value.Blank;

        /// <summary>Reads every stated cell of one worksheet.</summary>
        /// <summary>Records what one cell holds, and widens the walked extent to reach it.</summary>
        /// <param name="row">The zero-based row.</param>
        /// <param name="column">The zero-based column.</param>
        /// <param name="value">What the cell holds; a blank one is not stored.</param>
        public void Set(int row, int column, Value value)
        {
            if (row < 0 || column < 0) return;
            if (!value.IsBlank) _values[(row, column)] = value;
            Reaches(row, column);
        }

        /// <summary>Widens the walked extent without stating a value there.</summary>
        /// <param name="row">A row the sheet reaches, or -1 for none.</param>
        /// <param name="column">A column it reaches, or -1 for none.</param>
        public void Extend(int row, int column) => Reaches(row, column);
    }

}
