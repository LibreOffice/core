using System.Globalization;

namespace XRay.Office.Spreadsheets.Layout;

/// <summary>
/// The small expression language a conditional-format rule is written in, parsed once per rule
/// and evaluated once per cell.
/// </summary>
/// <remarks>
/// <para>
/// <see cref="SheetConditions.Comparison"/> reads the one shape a rule most often has —
/// <c>&lt;operand&gt; &lt;op&gt; &lt;operand&gt;</c> — and answers null for everything else, which
/// paints nothing. That is the right answer for a formula nothing here can evaluate and the wrong
/// one for a workbook whose whole chart is conditional formatting: a Gantt planner states ten
/// rules built out of defined names, <c>MEDIAN</c>, <c>INT</c>, <c>MOD</c> and <c>COLUMN</c>, and
/// holds no drawing part at all.
/// </para>
/// <para>
/// <strong>The grammar is the corpus's, not the format's.</strong> This is not a formula engine:
/// there is no iteration, no array, no reference arithmetic, no error propagation beyond what the
/// corpus needs, and a construct outside the set below makes the whole rule unparseable so that it
/// paints nothing — which is the behaviour it already had. A rule this cannot read must stay
/// unread rather than become a confident wrong answer.
/// </para>
/// </remarks>
internal static class SheetFormula
{
    /// <summary>How deep a defined name may expand before it is treated as circular.</summary>
    private const int MaxNameDepth = 16;

    /// <summary>A parsed expression.</summary>
    internal abstract record Node;

    /// <summary>A literal number, or a boolean written as one.</summary>
    /// <param name="Value">The number.</param>
    internal sealed record NumberNode(double Value) : Node;

    /// <summary>A literal string.</summary>
    /// <param name="Value">The text, with its doubled quotes undone.</param>
    internal sealed record TextNode(string Value) : Node;

    /// <summary>
    /// An A1 reference on the rule's own sheet, with the two <c>$</c> flags kept.
    /// </summary>
    /// <remarks>
    /// Which halves are absolute is the whole of what a rule over a range means: a relative half
    /// is shifted by the distance from the cell the formula was written for, and an absolute one
    /// is not. A reference naming a <em>different</em> sheet is refused at parse time — this
    /// evaluator holds one sheet's values and inventing a blank for another sheet's cell would
    /// make a rule fire where the reference's does not.
    /// </remarks>
    /// <param name="Row">The zero-based row.</param>
    /// <param name="Column">The zero-based column.</param>
    /// <param name="AbsoluteRow">Whether the row was written with a <c>$</c>.</param>
    /// <param name="AbsoluteColumn">Whether the column was.</param>
    internal sealed record ReferenceNode(int Row, int Column, bool AbsoluteRow, bool AbsoluteColumn)
        : Node;

    /// <summary>A unary <c>-</c> or <c>+</c>, or a trailing <c>%</c>.</summary>
    /// <param name="Operator">The character.</param>
    /// <param name="Operand">What it applies to.</param>
    internal sealed record UnaryNode(char Operator, Node Operand) : Node;

    /// <summary>A binary operator.</summary>
    /// <param name="Operator">One of <c>+ - * / ^ &amp;</c> or a comparison.</param>
    /// <param name="Left">The left side.</param>
    /// <param name="Right">The right side.</param>
    internal sealed record BinaryNode(string Operator, Node Left, Node Right) : Node;

    /// <summary>A function call.</summary>
    /// <param name="Name">The function's name, upper-cased.</param>
    /// <param name="Arguments">Its arguments, in order.</param>
    internal sealed record CallNode(string Name, IReadOnlyList<Node> Arguments) : Node;

    /// <summary>
    /// Parses a rule's formula, expanding the defined names it uses, or null when it states
    /// anything this does not read.
    /// </summary>
    /// <param name="formula">The rule's <c>&lt;formula&gt;</c> text.</param>
    /// <param name="sheetName">The name of the sheet the rule is on, for a qualified reference.</param>
    /// <param name="definedName">What a workbook-level name expands to, or null when it is not one.</param>
    /// <param name="anchorRow">The zero-based row the rule's formula is written for.</param>
    /// <param name="anchorColumn">The zero-based column it is written for.</param>
    public static Node? Parse(
        string? formula,
        string sheetName,
        Func<string, string?> definedName,
        int anchorRow = 0,
        int anchorColumn = 0)
    {
        ArgumentNullException.ThrowIfNull(definedName);

        if (string.IsNullOrWhiteSpace(formula)) return null;

        string text = formula.Trim();
        if (text.StartsWith('=')) text = text[1..];

        try
        {
            Parser parser = new(
                text, sheetName ?? string.Empty, definedName, depth: 0, anchorRow, anchorColumn);
            Node node = parser.Expression(0);
            return parser.AtEnd ? node : null;
        }
        catch (FormatException)
        {
            // The one exit for "this formula states something the grammar does not hold", thrown
            // from wherever the parser notices. A rule that cannot be read paints nothing, which
            // is what it did before this existed.
            return null;
        }
    }

    /// <summary>
    /// What an expression is worth at one cell, or null when it states something this cannot
    /// evaluate.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>The whole formula is evaluated as though it lived in the cell being tested.</strong>
    /// <c>ScConditionEntry::IsCellValid</c> calls <c>Interpret(rPos)</c> with the tested cell's
    /// address, and <c>Interpret</c> builds a temporary formula cell <em>at that address</em> for
    /// a condition holding relative references —
    /// <c>oTemp.emplace(mrDoc, rPos, *pFormula1)</c>, <c>sc/source/core/data/conditio.cxx</c>:
    /// 680-700. So a relative half shifts by the distance from the cell the formula was written
    /// for, and <c>COLUMN()</c> and <c>ROW()</c> answer the <em>tested</em> cell's position rather
    /// than the base's.
    /// </para>
    /// <para>
    /// Null is "this cell is not painted" rather than an error value: there is no error
    /// propagation here, and a formula that divides by zero or adds a string simply does not fire.
    /// The alternative — inventing a number — is what turns an absent answer into a wrong one.
    /// </para>
    /// </remarks>
    /// <param name="node">The parsed expression.</param>
    /// <param name="sheet">The sheet's values.</param>
    /// <param name="row">The zero-based row being tested.</param>
    /// <param name="column">The zero-based column being tested.</param>
    /// <param name="rowShift">Rows from the cell the formula was written for.</param>
    /// <param name="columnShift">Columns from it.</param>
    public static SheetConditions.Value? Evaluate(
        Node node, SheetConditions.Sheet sheet, int row, int column, int rowShift, int columnShift)
    {
        ArgumentNullException.ThrowIfNull(node);
        ArgumentNullException.ThrowIfNull(sheet);

        switch (node)
        {
            case NumberNode number:
                return SheetConditions.Value.OfNumber(number.Value);

            case TextNode text:
                return SheetConditions.Value.OfText(text.Value);

            case ReferenceNode reference:
            {
                int at = reference.AbsoluteRow ? reference.Row : reference.Row + rowShift;
                int across = reference.AbsoluteColumn ? reference.Column : reference.Column + columnShift;
                return at < 0 || across < 0 ? SheetConditions.Value.Blank : sheet.At(at, across);
            }

            case UnaryNode unary:
            {
                if (Evaluate(unary.Operand, sheet, row, column, rowShift, columnShift) is not { } inner)
                    return null;
                if (Number(inner) is not { } value) return null;

                return unary.Operator switch
                {
                    '-' => SheetConditions.Value.OfNumber(-value),
                    '+' => SheetConditions.Value.OfNumber(value),
                    '%' => SheetConditions.Value.OfNumber(value / 100.0),
                    _ => null,
                };
            }

            case BinaryNode binary:
                return Binary(binary, sheet, row, column, rowShift, columnShift);

            case CallNode call:
                return Call(call, sheet, row, column, rowShift, columnShift);

            default:
                return null;
        }
    }

    private static SheetConditions.Value? Binary(
        BinaryNode node, SheetConditions.Sheet sheet, int row, int column, int rowShift, int columnShift)
    {
        if (Evaluate(node.Left, sheet, row, column, rowShift, columnShift) is not { } left) return null;
        if (Evaluate(node.Right, sheet, row, column, rowShift, columnShift) is not { } right) return null;

        // A comparison is worth one or nothing, which is what lets a rule multiply two of them
        // together — and `Value.Compare` is the reference's own ordering, measured once for the
        // two-operand reader and shared with it rather than restated.
        if (node.Operator is "=" or "<>" or "<" or "<=" or ">" or ">=")
        {
            return SheetConditions.Value.OfNumber(
                SheetConditions.Value.Compare(left, right, node.Operator) ? 1 : 0);
        }

        if (node.Operator == "&") return SheetConditions.Value.OfText(Text(left) + Text(right));

        if (Number(left) is not { } a || Number(right) is not { } b) return null;

        double? result = node.Operator switch
        {
            "+" => a + b,
            "-" => a - b,
            "*" => a * b,
            "/" => b == 0 ? null : a / b,
            "^" => Math.Pow(a, b),
            _ => null,
        };

        return result is { } value && double.IsFinite(value)
            ? SheetConditions.Value.OfNumber(value)
            : null;
    }

    /// <summary>
    /// The functions the corpus's refused rules are built out of, and no others.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>MOD</c> is Calc's and not C#'s: it is <c>a - b × INT(a / b)</c>, so its sign follows the
    /// <em>divisor</em> and <c>MOD(-1, 2)</c> is 1. <c>INT</c> rounds towards negative infinity
    /// rather than truncating, which is the same difference seen from the other side.
    /// </para>
    /// <para>
    /// <strong><c>MEDIAN</c> drops an argument that is not numeric</strong> — an empty cell and a
    /// text cell alike — rather than counting it as zero, because
    /// <c>ScInterpreter::GetNumberSequenceArray</c> pushes a single reference only
    /// <c>if (aCell.hasNumeric())</c>. Measured at 26.2.4.2: <c>MEDIAN(H2,1,2)</c> with <c>H2</c>
    /// empty is <b>1.5</b>, so the argument is removed and not zeroed. An argument that is an
    /// <em>expression</em> always contributes, because arithmetic over a blank has already made it
    /// a number — so <c>$C1+$D1-1</c> over a blank <c>$C1</c> is present and counted while a bare
    /// <c>$C1</c> beside it is not.
    /// </para>
    /// <para>
    /// <c>COLUMN()</c> and <c>ROW()</c> take no argument here. With one they would answer a
    /// reference's own position, which no corpus rule asks for; refusing is what keeps a rule this
    /// cannot evaluate unpainted.
    /// </para>
    /// </remarks>
    private static SheetConditions.Value? Call(
        CallNode node, SheetConditions.Sheet sheet, int row, int column, int rowShift, int columnShift)
    {
        switch (node.Name)
        {
            case "COLUMN" when node.Arguments.Count == 0:
                return SheetConditions.Value.OfNumber(column + 1);

            case "ROW" when node.Arguments.Count == 0:
                return SheetConditions.Value.OfNumber(row + 1);

            case "INT" when node.Arguments.Count == 1:
                return One(node.Arguments[0]) is { } single
                    ? SheetConditions.Value.OfNumber(Math.Floor(single))
                    : null;

            case "MOD" when node.Arguments.Count == 2:
            {
                if (One(node.Arguments[0]) is not { } a) return null;
                if (One(node.Arguments[1]) is not { } b || b == 0) return null;
                return SheetConditions.Value.OfNumber(a - (b * Math.Floor(a / b)));
            }

            case "ABS" when node.Arguments.Count == 1:
                return One(node.Arguments[0]) is { } magnitude
                    ? SheetConditions.Value.OfNumber(Math.Abs(magnitude))
                    : null;

            case "LEN" when node.Arguments.Count == 1:
                return Evaluate(node.Arguments[0], sheet, row, column, rowShift, columnShift)
                    is { } counted
                    ? SheetConditions.Value.OfNumber(Text(counted).Length)
                    : null;

            case "ISBLANK" when node.Arguments.Count == 1:
                // The value model has one blank and an error cell is stored as one, so this reads
                // an error as blank. No corpus rule puts the two together; see `ISERROR`, which is
                // left out for the same reason from the other side.
                return Evaluate(node.Arguments[0], sheet, row, column, rowShift, columnShift)
                    is { } tested
                    ? SheetConditions.Value.OfNumber(tested.IsBlank ? 1 : 0)
                    : null;

            case "NOT" when node.Arguments.Count == 1:
                return One(node.Arguments[0]) is { } negated
                    ? SheetConditions.Value.OfNumber(negated == 0 ? 1 : 0)
                    : null;

            // `AND` is the commonest of the lot by a wide margin — 42 of the corpus's refused
            // rules and 34 916 of the cells they can paint — and `OR` is here beside it because
            // the pair is small and a missing one of them paints nothing, which is the failure
            // mode that leaves no trace. No corpus rule states `OR`.
            case "AND" when node.Arguments.Count > 0:
            case "OR" when node.Arguments.Count > 0:
            {
                bool all = node.Name == "AND";
                bool held = all;

                foreach (Node argument in node.Arguments)
                {
                    if (One(argument) is not { } value) return null;
                    if (all) held &= value != 0;
                    else held |= value != 0;
                }

                return SheetConditions.Value.OfNumber(held ? 1 : 0);
            }

            case "ROUNDDOWN" when node.Arguments.Count is 1 or 2:
            {
                if (One(node.Arguments[0]) is not { } value) return null;

                double digits = 0;
                if (node.Arguments.Count == 2)
                {
                    if (One(node.Arguments[1]) is not { } stated) return null;
                    digits = Math.Truncate(stated);
                }

                if (Math.Abs(digits) > 15) return null;

                // Towards zero, which is what distinguishes it from `INT`.
                double scale = Math.Pow(10, digits);
                return SheetConditions.Value.OfNumber(Math.Truncate(value * scale) / scale);
            }

            // The reference recalculates a volatile function when it opens the file, so a rule
            // built on one is right only on the day it is drawn. `SOURCE_DATE_EPOCH` is honoured
            // here for the same reason `xray-office render` honours it in a header: without it two
            // sweeps a day apart disagree on these rows for no reason connected to any change.
            case "TODAY" when node.Arguments.Count == 0:
                return SheetConditions.Value.OfNumber(Math.Floor(Today()));

            case "MEDIAN" when node.Arguments.Count > 0:
            {
                List<double> numbers = [];
                foreach (Node argument in node.Arguments)
                {
                    if (Evaluate(argument, sheet, row, column, rowShift, columnShift)
                        is not { } value) return null;

                    // A NON-NUMERIC argument contributes nothing rather than contributing zero.
                    // `GetNumberSequenceArray` pushes a single reference only
                    // `if (aCell.hasNumeric())` (`sc/source/core/tool/interpr3.cxx`:3985-3990), so
                    // `MEDIAN(H2,1,2)` with H2 empty is **1.5** and not 1 — the argument is
                    // removed, not zeroed. A *computed* argument has already become a number, so
                    // `$C1+$D1-1` over a blank `$C1` is still present and still counted; empty and
                    // zero are not interchangeable here.
                    if (value.Number is not { } number) continue;
                    numbers.Add(number);
                }

                if (numbers.Count == 0) return null;
                numbers.Sort();

                int middle = numbers.Count / 2;
                return SheetConditions.Value.OfNumber(
                    numbers.Count % 2 == 1
                        ? numbers[middle]
                        : (numbers[middle - 1] + numbers[middle]) / 2.0);
            }

            default:
                return null;
        }

        double? One(Node argument)
            => Evaluate(argument, sheet, row, column, rowShift, columnShift) is { } value
                ? Number(value)
                : null;
    }

    /// <summary>
    /// Today, as a 1900-system serial, pinned by <c>SOURCE_DATE_EPOCH</c> where one is set.
    /// </summary>
    private static double Today()
    {
        DateTime now = Environment.GetEnvironmentVariable("SOURCE_DATE_EPOCH") is { Length: > 0 } pinned
                       && long.TryParse(pinned, NumberStyles.Integer, CultureInfo.InvariantCulture,
                                        out long seconds)
            ? DateTime.UnixEpoch.AddSeconds(seconds)
            : DateTime.Now;

        // 30 December 1899, not the 31st: Excel treats 1900 as a leap year and the epoch is
        // shifted to match. `Core.Numbers.SpreadsheetDate` states the same constant.
        return (now - new DateTime(1899, 12, 30, 0, 0, 0, DateTimeKind.Unspecified)).TotalDays;
    }

    /// <summary>What a value is worth as a number: a blank is nought and text is not a number.</summary>
    private static double? Number(SheetConditions.Value value)
        => value.IsBlank ? 0 : value.Number;

    /// <summary>What a value is worth as text, for <c>&amp;</c>.</summary>
    private static string Text(SheetConditions.Value value)
        => value.Text
           ?? value.Number?.ToString("R", CultureInfo.InvariantCulture)
           ?? string.Empty;

    /// <summary>A recursive-descent parser over one formula's text.</summary>
    private sealed class Parser
    {
        private readonly string _text;
        private readonly string _sheet;
        private readonly Func<string, string?> _names;
        private readonly int _depth;
        private readonly int _anchorRow;
        private readonly int _anchorColumn;
        private int _at;

        public Parser(
            string text, string sheet, Func<string, string?> names, int depth,
            int anchorRow, int anchorColumn)
        {
            _text = text;
            _sheet = sheet;
            _names = names;
            _depth = depth;
            _anchorRow = anchorRow;
            _anchorColumn = anchorColumn;
        }

        /// <summary>True when everything but trailing space has been read.</summary>
        public bool AtEnd
        {
            get
            {
                SkipSpace();
                return _at >= _text.Length;
            }
        }

        /// <summary>
        /// One expression, stopping below the given binding power.
        /// </summary>
        /// <remarks>
        /// Precedence climbing, with Excel's own table: comparison binds loosest, then
        /// concatenation, then <c>+</c>/<c>-</c>, then <c>*</c>//, then <c>^</c>. Only <c>^</c>
        /// associates to the right.
        /// </remarks>
        /// <param name="minimum">The lowest binding power this call may consume.</param>
        public Node Expression(int minimum)
        {
            Node left = Unary();

            while (true)
            {
                SkipSpace();
                if (Operator() is not { } op) break;

                int power = BindingPower(op);
                if (power < minimum) break;

                _at += op.Length;
                Node right = Expression(op == "^" ? power : power + 1);
                left = new BinaryNode(op, left, right);
            }

            return left;
        }

        private Node Unary()
        {
            SkipSpace();
            if (_at >= _text.Length) throw new FormatException();

            char c = _text[_at];
            if (c is '-' or '+')
            {
                _at++;
                return new UnaryNode(c, Unary());
            }

            Node node = Primary();

            SkipSpace();
            if (_at < _text.Length && _text[_at] == '%')
            {
                _at++;
                node = new UnaryNode('%', node);
            }

            return node;
        }

        private Node Primary()
        {
            SkipSpace();
            if (_at >= _text.Length) throw new FormatException();

            char c = _text[_at];

            if (c == '(')
            {
                _at++;
                Node inner = Expression(0);
                Expect(')');
                return inner;
            }

            if (c == '"') return new TextNode(QuotedString());
            if (char.IsAsciiDigit(c) || c == '.') return new NumberNode(Number());

            // A single-quoted sheet name, which is how a reference to a sheet whose name holds a
            // space is written -- and the only form the corpus's defined names use.
            if (c == '\'' || char.IsAsciiLetter(c) || c == '_' || c == '$') return NameOrReference();

            throw new FormatException();
        }

        /// <summary>
        /// A reference, a function call, a defined name, or a boolean.
        /// </summary>
        /// <remarks>
        /// The four are not distinguishable until the word has been read and what follows it
        /// looked at: <c>MOD(</c> is a call, <c>A1</c> is a reference, <c>Plan</c> is a name and
        /// <c>TRUE</c> is a number.
        /// </remarks>
        private Node NameOrReference()
        {
            string? sheet = null;

            if (_text[_at] == '\'')
            {
                sheet = QuotedSheetName();
                Expect('!');
            }

            int start = _at;
            while (_at < _text.Length
                   && (char.IsAsciiLetterOrDigit(_text[_at]) || _text[_at] is '_' or '.' or '$'))
            {
                _at++;
            }

            if (_at == start) throw new FormatException();
            string word = _text[start.._at];

            // An unquoted sheet name, which is what a name with no space in it is written as.
            if (sheet is null && _at < _text.Length && _text[_at] == '!')
            {
                _at++;
                return Qualified(word, ReadWord());
            }

            if (sheet is not null) return Qualified(sheet, word);

            SkipSpace();
            if (_at < _text.Length && _text[_at] == '(')
            {
                _at++;
                return new CallNode(word.ToUpperInvariant(), Arguments());
            }

            if (string.Equals(word, "TRUE", StringComparison.OrdinalIgnoreCase))
                return new NumberNode(1);
            if (string.Equals(word, "FALSE", StringComparison.OrdinalIgnoreCase))
                return new NumberNode(0);

            if (ReferenceOf(word) is { } reference) return reference;

            return Defined(word);
        }

        /// <summary>A reference qualified by a sheet name, which must be the rule's own sheet.</summary>
        private ReferenceNode Qualified(string sheet, string word)
        {
            if (!string.Equals(sheet, _sheet, StringComparison.OrdinalIgnoreCase))
                throw new FormatException();

            return ReferenceOf(word) ?? throw new FormatException();
        }

        /// <summary>A defined name, expanded and parsed in this formula's own place.</summary>
        /// <remarks>
        /// Expanded rather than resolved to a value, because a name is written relative to its own
        /// base and carries <c>$</c> flags that decide how it shifts — <c>PeriodInPlan</c> names
        /// <c>$C1</c>, whose row moves with the cell being tested. Substituting the text and
        /// parsing it here is what keeps those flags.
        /// </remarks>
        private Node Defined(string word)
        {
            if (_depth >= MaxNameDepth) throw new FormatException();

            string expanded = _names(word) ?? throw new FormatException();

            Parser inner = new(
                expanded.Trim(), _sheet, _names, _depth + 1, _anchorRow, _anchorColumn);
            Node node = inner.Expression(0);
            if (!inner.AtEnd) throw new FormatException();

            // Only the OUTERMOST expansion rebases, and that is not a detail: a name that refers
            // to another name would otherwise be moved onto the rule's base once per level, so
            // `Plan` -- which is `PeriodInPlan * (...)` -- read its `$C1` four rows below where
            // 26.2.4.2 reads it and evaluated to nothing on every cell of the sheet. A deeper
            // expansion returns its references as the name wrote them and is rebased with the
            // tree it is part of.
            return _depth == 0 ? Rebased(node, _anchorRow, _anchorColumn) : node;
        }

        /// <summary>
        /// A name's subtree, moved from the name's own base onto the rule's.
        /// </summary>
        /// <remarks>
        /// <para>
        /// <strong>A defined name's relative references are written against <c>A1</c>, not against
        /// the rule that uses it</strong>, and getting this wrong moves every such reference by
        /// the whole distance from the sheet's corner to the rule's first cell. 26.2.4.2 says so
        /// in its own export of the witness: each of the Gantt planner's eight names comes back as
        /// <c>&lt;table:named-expression … table:base-cell-address="$'Project
        /// Planner'.$A$1"/&gt;</c>, while the conditional format beside them carries
        /// <c>calcext:base-cell-address="'Project Planner'.H5"</c>. Two different bases in one
        /// file.
        /// </para>
        /// <para>
        /// Rather than carry two shifts through the evaluator, the name's relative halves are
        /// moved onto the rule's base here — once, at parse time — so that the single shift the
        /// evaluator applies lands them where <c>A1</c> would have put them. A reference the name
        /// wrote absolutely is already where it belongs and is left alone.
        /// </para>
        /// </remarks>
        private static Node Rebased(Node node, int anchorRow, int anchorColumn) => node switch
        {
            ReferenceNode reference => reference with
            {
                Row = reference.AbsoluteRow ? reference.Row : reference.Row + anchorRow,
                Column = reference.AbsoluteColumn ? reference.Column : reference.Column + anchorColumn,
            },
            UnaryNode unary => unary with
            {
                Operand = Rebased(unary.Operand, anchorRow, anchorColumn),
            },
            BinaryNode binary => binary with
            {
                Left = Rebased(binary.Left, anchorRow, anchorColumn),
                Right = Rebased(binary.Right, anchorRow, anchorColumn),
            },
            CallNode call => call with
            {
                Arguments = [.. call.Arguments.Select(one => Rebased(one, anchorRow, anchorColumn))],
            },
            _ => node,
        };

        private List<Node> Arguments()
        {
            List<Node> arguments = [];

            SkipSpace();
            if (_at < _text.Length && _text[_at] == ')')
            {
                _at++;
                return arguments;
            }

            while (true)
            {
                arguments.Add(Expression(0));
                SkipSpace();

                if (_at >= _text.Length) throw new FormatException();
                if (_text[_at] == ',') { _at++; continue; }
                if (_text[_at] == ')') { _at++; return arguments; }

                throw new FormatException();
            }
        }

        private string ReadWord()
        {
            int start = _at;
            while (_at < _text.Length
                   && (char.IsAsciiLetterOrDigit(_text[_at]) || _text[_at] is '_' or '.' or '$'))
            {
                _at++;
            }

            if (_at == start) throw new FormatException();
            return _text[start.._at];
        }

        private string QuotedSheetName()
        {
            _at++;   // the opening quote
            int start = _at;

            while (_at < _text.Length && _text[_at] != '\'') _at++;
            if (_at >= _text.Length) throw new FormatException();

            string name = _text[start.._at];
            _at++;
            return name;
        }

        private string QuotedString()
        {
            _at++;   // the opening quote
            System.Text.StringBuilder body = new();

            while (_at < _text.Length)
            {
                char c = _text[_at++];
                if (c != '"') { body.Append(c); continue; }

                // A doubled quote inside a string is one quote; a single one ends it.
                if (_at < _text.Length && _text[_at] == '"') { body.Append('"'); _at++; continue; }
                return body.ToString();
            }

            throw new FormatException();
        }

        private double Number()
        {
            int start = _at;
            while (_at < _text.Length && (char.IsAsciiDigit(_text[_at]) || _text[_at] == '.')) _at++;

            if (_at < _text.Length && (_text[_at] is 'e' or 'E'))
            {
                int save = _at;
                _at++;
                if (_at < _text.Length && _text[_at] is '+' or '-') _at++;
                if (_at < _text.Length && char.IsAsciiDigit(_text[_at]))
                {
                    while (_at < _text.Length && char.IsAsciiDigit(_text[_at])) _at++;
                }
                else
                {
                    _at = save;
                }
            }

            return double.TryParse(_text[start.._at], NumberStyles.Float, CultureInfo.InvariantCulture,
                                   out double value)
                ? value
                : throw new FormatException();
        }

        /// <summary>An A1 reference, or null when the word is not one.</summary>
        private static ReferenceNode? ReferenceOf(string word)
        {
            int at = 0;
            bool absoluteColumn = false;
            if (at < word.Length && word[at] == '$') { absoluteColumn = true; at++; }

            int letters = at;
            while (letters < word.Length && char.IsAsciiLetter(word[letters])) letters++;
            if (letters == at || letters - at > 3) return null;

            int digits = letters;
            bool absoluteRow = false;
            if (digits < word.Length && word[digits] == '$') { absoluteRow = true; digits++; }

            if (digits >= word.Length) return null;
            for (int i = digits; i < word.Length; i++)
            {
                if (!char.IsAsciiDigit(word[i])) return null;
            }

            int column = 0;
            for (int i = at; i < letters; i++)
                column = (column * 26) + (char.ToUpperInvariant(word[i]) - 'A' + 1);

            return int.TryParse(word[digits..], NumberStyles.Integer, CultureInfo.InvariantCulture,
                                out int row) && row >= 1
                ? new ReferenceNode(row - 1, column - 1, absoluteRow, absoluteColumn)
                : null;
        }

        private string? Operator()
        {
            if (_at >= _text.Length) return null;

            foreach (string op in Operators)
            {
                if (_at + op.Length <= _text.Length
                    && string.CompareOrdinal(_text, _at, op, 0, op.Length) == 0)
                {
                    return op;
                }
            }

            return null;
        }

        /// <summary>Longest first, so <c>&lt;=</c> is not read as <c>&lt;</c>.</summary>
        private static readonly string[] Operators =
            ["<>", "<=", ">=", "=", "<", ">", "&", "+", "-", "*", "/", "^"];

        /// <summary>Excel's precedence, loosest first.</summary>
        private static int BindingPower(string op) => op switch
        {
            "=" or "<>" or "<" or "<=" or ">" or ">=" => 1,
            "&" => 2,
            "+" or "-" => 3,
            "*" or "/" => 4,
            "^" => 5,
            _ => 0,
        };

        private void Expect(char c)
        {
            SkipSpace();
            if (_at >= _text.Length || _text[_at] != c) throw new FormatException();
            _at++;
        }

        private void SkipSpace()
        {
            while (_at < _text.Length && char.IsWhiteSpace(_text[_at])) _at++;
        }
    }
}
