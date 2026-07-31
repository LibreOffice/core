using System.Globalization;
using System.Text;

namespace Paperless.Spreadsheets.Numbers;

/// <summary>
/// A spreadsheet number-format code, parsed far enough to say what a cell displays.
/// </summary>
/// <remarks>
/// <para>
/// A spreadsheet stores a date as a serial number and a percentage as a fraction. Without the
/// number format, extraction reports 46598 where the file shows 2027-07-30, so a reader that
/// stops at the value has not finished the job. ODF sidesteps this by caching the displayed
/// text next to the value; BIFF does not, so the format has to be applied.
/// </para>
/// <para>
/// The code language is the one Excel, OOXML and BIFF share: up to four semicolon-separated
/// sections (positive, negative, zero, text), each a mix of digit placeholders, date and time
/// parts, quoted literals and bracketed directives. This implements the constructs that occur
/// in real files rather than the whole language — see <see cref="IsUnderstood"/> for what is
/// not covered, which is fractions and the locale-dependent calendar modifiers.
/// </para>
/// <para>
/// Two deliberate approximations. Month and weekday names come out in English, because the
/// name a spreadsheet shows depends on the reading application's locale rather than on
/// anything in the file. And <c>_x</c>, which reserves the width of a character, becomes one
/// space: it exists to align columns and there is no width in extracted text.
/// </para>
/// </remarks>
public sealed class NumberFormatCode
{
    private readonly Section[] _sections;

    private NumberFormatCode(string code, Section[] sections)
    {
        Code = code;
        _sections = sections;

        Section first = sections.Length > 0 ? sections[0] : Section.Empty;
        HasDate = sections.Any(s => s.HasDate);
        HasTime = sections.Any(s => s.HasTime);
        HasElapsedTime = sections.Any(s => s.HasElapsedTime);
        IsTextOnly = first.IsTextOnly;
        IsUnderstood = sections.All(s => s.IsUnderstood);
    }

    /// <summary>The code as the file records it.</summary>
    public string Code { get; }

    /// <summary>True when the format displays a calendar date.</summary>
    public bool HasDate { get; }

    /// <summary>True when the format displays a time of day or an elapsed time.</summary>
    public bool HasTime { get; }

    /// <summary>
    /// True when the format displays an elapsed time — <c>[h]:mm</c> and friends.
    /// </summary>
    /// <remarks>
    /// The brackets are what distinguish "14 hours 30 minutes of work" from "half past two":
    /// the same stored number, and only the format says which was meant.
    /// </remarks>
    public bool HasElapsedTime { get; }

    /// <summary>True when the format shows text and nothing else.</summary>
    public bool IsTextOnly { get; }

    /// <summary>
    /// False when the code uses something this implementation does not reproduce, so a caller
    /// can record a diagnostic rather than presenting a guess as the displayed text.
    /// </summary>
    public bool IsUnderstood { get; }

    /// <summary>The General format, which every workbook has whether or not it says so.</summary>
    public static NumberFormatCode General { get; } = Parse("General");

    /// <summary>Parses a format code. Never throws; an unparseable code formats as General.</summary>
    public static NumberFormatCode Parse(string? code)
    {
        code ??= string.Empty;
        List<Section> sections = [];
        foreach (string part in SplitSections(code)) sections.Add(Section.Parse(part));
        if (sections.Count == 0) sections.Add(Section.Parse("General"));
        return new NumberFormatCode(code, [.. sections]);
    }

    /// <summary>
    /// Splits a code into its sections, ignoring semicolons that are part of a literal.
    /// </summary>
    private static List<string> SplitSections(string code)
    {
        List<string> parts = [];
        StringBuilder current = new();
        bool quoted = false;
        bool bracketed = false;

        for (int i = 0; i < code.Length; i++)
        {
            char c = code[i];

            if (c == '\\' && i + 1 < code.Length)
            {
                current.Append(c).Append(code[i + 1]);
                i++;
                continue;
            }

            if (c == '"') quoted = !quoted;
            else if (!quoted && c == '[') bracketed = true;
            else if (!quoted && c == ']') bracketed = false;
            else if (c == ';' && !quoted && !bracketed)
            {
                parts.Add(current.ToString());
                current.Clear();
                continue;
            }

            current.Append(c);
        }

        if (current.Length > 0 || parts.Count > 0) parts.Add(current.ToString());
        return parts;
    }

    /// <summary>
    /// The text a cell holding <paramref name="value"/> displays.
    /// </summary>
    /// <param name="value">The stored number.</param>
    /// <param name="epoch">
    /// The workbook's day zero, needed only when the format shows a date. BIFF workbooks use
    /// one of two epochs and the file says which.
    /// </param>
    public string Format(double value, DateTime epoch)
    {
        // Excel's section rules: with all four present the third is used for zero and the
        // fourth for text; with two, the second covers every negative; with one, negatives
        // get a sign and the single section formats their magnitude.
        Section section;
        bool signed = false;

        if (_sections.Length >= 2 && value < 0)
        {
            section = _sections[1];
            value = -value;
        }
        else if (_sections.Length >= 3 && value == 0)
        {
            section = _sections[2];
        }
        else
        {
            section = _sections[0];
            if (value < 0)
            {
                signed = true;
                value = -value;
            }
        }

        string text = section.Format(value, epoch);
        return signed ? "-" + text : text;
    }

    /// <summary>
    /// The text a cell holding a string displays, which is the fourth section when there is
    /// one and the string itself otherwise.
    /// </summary>
    public string FormatText(string text)
        => _sections.Length >= 4 ? _sections[3].FormatText(text) : text;

    /// <summary>
    /// Whether a stored number should be reported as a date, a duration, or a plain number.
    /// </summary>
    public CellValueKind ValueKind
        => HasElapsedTime ? CellValueKind.Duration
            : HasDate ? CellValueKind.DateTime
            : HasTime ? CellValueKind.Duration
            : CellValueKind.Number;

    /// <inheritdoc/>
    public override string ToString() => Code;

    /// <summary>What kind of value a number format says a cell holds.</summary>
    public enum CellValueKind
    {
        /// <summary>A plain number.</summary>
        Number,

        /// <summary>A point in time.</summary>
        DateTime,

        /// <summary>A duration: an elapsed time, or a time of day, which the file cannot tell apart.</summary>
        Duration,
    }

    /// <summary>One semicolon-separated section of a format code.</summary>
    private sealed class Section
    {
        private readonly Token[] _tokens;
        private readonly int _integerPlaces;
        private readonly int _integerZeros;
        private readonly string _decimalPattern = string.Empty;
        private readonly bool _grouped;
        private readonly int _scaleDivisions;
        private readonly int _percents;
        private readonly int _exponentDigits;
        private readonly bool _exponentSigned;
        private readonly bool _general;
        private readonly bool _twelveHour;

        private Section(Token[] tokens)
        {
            _tokens = tokens;

            bool afterDecimal = false;
            int lastIntegerDigit = -1;
            List<int> commas = [];
            StringBuilder decimalPattern = new();

            for (int i = 0; i < tokens.Length; i++)
            {
                Token token = tokens[i];
                switch (token.Kind)
                {
                    case TokenKind.Digit:
                        if (afterDecimal)
                        {
                            decimalPattern.Append(token.Symbol);
                        }
                        else
                        {
                            _integerPlaces++;
                            if (token.Symbol == '0') _integerZeros++;
                            lastIntegerDigit = i;
                        }

                        break;

                    case TokenKind.Decimal:
                        afterDecimal = true;
                        break;

                    case TokenKind.Group:
                        if (!afterDecimal) commas.Add(i);
                        break;

                    case TokenKind.Percent:
                        _percents++;
                        break;

                    case TokenKind.Exponent:
                        _exponentDigits = token.Count;
                        _exponentSigned = token.Symbol == '+';
                        break;

                    case TokenKind.General:
                        _general = true;
                        break;

                    case TokenKind.AmPm:
                        _twelveHour = true;
                        break;

                    case TokenKind.Date:
                        HasDate = HasDate || token.Symbol is 'y' or 'd' or 'M';
                        HasTime = HasTime || token.Symbol is 'h' or 'm' or 's';
                        HasElapsedTime = HasElapsedTime || token.Elapsed;
                        break;

                    case TokenKind.Text:
                        IsText = true;
                        break;

                    case TokenKind.Unsupported:
                        IsUnderstood = false;
                        break;

                    default:
                        break;
                }
            }

            _decimalPattern = decimalPattern.ToString();

            // A comma with a digit placeholder still to come groups thousands; one after the
            // last placeholder divides the value by a thousand instead. Both spellings are the
            // same character, and only its position separates "#,##0" from "#,##0,".
            foreach (int at in commas)
            {
                if (at < lastIntegerDigit) _grouped = true;
                else _scaleDivisions++;
            }
        }

        public static Section Empty { get; } = new([]);

        public bool HasDate { get; }
        public bool HasTime { get; }
        public bool HasElapsedTime { get; }
        public bool IsText { get; }
        public bool IsUnderstood { get; } = true;

        /// <summary>True when the section shows text and no number at all.</summary>
        public bool IsTextOnly
            => IsText && _integerPlaces == 0 && _decimalPattern.Length == 0 && !HasDate && !HasTime;

        public static Section Parse(string code)
        {
            List<Token> tokens = [];

            for (int i = 0; i < code.Length;)
            {
                char c = code[i];

                switch (c)
                {
                    case '"':
                    {
                        int end = code.IndexOf('"', i + 1);
                        string literal = end < 0 ? code[(i + 1)..] : code[(i + 1)..end];
                        tokens.Add(Token.Literal(literal));
                        i = end < 0 ? code.Length : end + 1;
                        continue;
                    }

                    case '\\':
                        if (i + 1 < code.Length) tokens.Add(Token.Literal(code[i + 1].ToString()));
                        i += 2;
                        continue;

                    case '[':
                    {
                        int end = code.IndexOf(']', i + 1);
                        string inside = end < 0 ? code[(i + 1)..] : code[(i + 1)..end];
                        tokens.AddRange(Bracketed(inside));
                        i = end < 0 ? code.Length : end + 1;
                        continue;
                    }

                    case '_':
                        // Reserves the width of the next character. There is no width in text,
                        // so a single space keeps the shape of accounting formats.
                        tokens.Add(Token.Literal(" "));
                        i += 2;
                        continue;

                    case '*':
                        // Repeat-to-fill. Nothing to fill in extracted text.
                        i += 2;
                        continue;

                    case '0' or '#' or '?':
                        tokens.Add(Token.Digit(c));
                        i++;
                        continue;

                    case '.':
                        tokens.Add(new Token(TokenKind.Decimal, '.', 0, null, false));
                        i++;
                        continue;

                    case ',':
                        tokens.Add(new Token(TokenKind.Group, ',', 0, null, false));
                        i++;
                        continue;

                    case '%':
                        tokens.Add(new Token(TokenKind.Percent, '%', 0, null, false));
                        tokens.Add(Token.Literal("%"));
                        i++;
                        continue;

                    case '@':
                        tokens.Add(new Token(TokenKind.Text, '@', 0, null, false));
                        i++;
                        continue;

                    case '/':
                        // A slash between digit placeholders is a fraction, which needs a
                        // continued-fraction search rather than a digit walk.
                        if (tokens.Any(t => t.Kind == TokenKind.Digit))
                        {
                            tokens.Add(new Token(TokenKind.Unsupported, '/', 0, null, false));
                            i++;
                            continue;
                        }

                        tokens.Add(Token.Literal("/"));
                        i++;
                        continue;

                    default:
                        break;
                }

                if (Matches(code, i, "General"))
                {
                    tokens.Add(new Token(TokenKind.General, 'G', 0, null, false));
                    i += "General".Length;
                    continue;
                }

                if (Matches(code, i, "AM/PM") || Matches(code, i, "A/P"))
                {
                    bool full = Matches(code, i, "AM/PM");
                    tokens.Add(new Token(TokenKind.AmPm, full ? 'A' : 'a', 0, null, false));
                    i += full ? 5 : 3;
                    continue;
                }

                if ((c is 'E' or 'e') && i + 1 < code.Length && code[i + 1] is '+' or '-')
                {
                    int digits = 0;
                    int at = i + 2;
                    while (at < code.Length && code[at] is '0' or '#' or '?')
                    {
                        digits++;
                        at++;
                    }

                    tokens.Add(new Token(TokenKind.Exponent, code[i + 1], Math.Max(digits, 1), null, false));
                    i = at;
                    continue;
                }

                if (IsDateLetter(c))
                {
                    char lower = char.ToLowerInvariant(c);
                    int count = 0;
                    while (i + count < code.Length && char.ToLowerInvariant(code[i + count]) == lower) count++;
                    tokens.Add(new Token(TokenKind.Date, lower, count, null, false));
                    i += count;
                    continue;
                }

                tokens.Add(Token.Literal(c.ToString()));
                i++;
            }

            return new Section([.. Disambiguate(tokens)]);
        }

        /// <summary>
        /// Decides which <c>m</c> means month and which means minute.
        /// </summary>
        /// <remarks>
        /// The same letter means both, and only the neighbours say which: an <c>m</c> directly
        /// after an hour part or directly before a seconds part is minutes, and everything else
        /// is a month. Getting this wrong turns 30/07 into 30/30 in exactly the formats people
        /// use most, which is why it is a pass of its own rather than a guess made inline.
        /// </remarks>
        private static List<Token> Disambiguate(List<Token> tokens)
        {
            for (int i = 0; i < tokens.Count; i++)
            {
                Token token = tokens[i];
                if (token.Kind != TokenKind.Date || token.Symbol != 'm') continue;

                bool minutes = PreviousTimePart(tokens, i) == 'h' || NextTimePart(tokens, i) == 's';
                if (!minutes) tokens[i] = token with { Symbol = 'M' };
            }

            return tokens;
        }

        private static char PreviousTimePart(List<Token> tokens, int index)
        {
            for (int i = index - 1; i >= 0; i--)
            {
                if (tokens[i].Kind == TokenKind.Date) return tokens[i].Symbol;
                if (tokens[i].Kind != TokenKind.Literal) break;
                if (tokens[i].Text is { Length: > 0 } text && text.Any(char.IsLetterOrDigit)) break;
            }

            return '\0';
        }

        private static char NextTimePart(List<Token> tokens, int index)
        {
            for (int i = index + 1; i < tokens.Count; i++)
            {
                if (tokens[i].Kind == TokenKind.Date) return tokens[i].Symbol;
                if (tokens[i].Kind != TokenKind.Literal) break;
                if (tokens[i].Text is { Length: > 0 } text && text.Any(char.IsLetterOrDigit)) break;
            }

            return '\0';
        }

        private static IEnumerable<Token> Bracketed(string inside)
        {
            if (inside.Length == 0) yield break;

            // [$£-809] is a currency symbol and a locale; the symbol is everything before the
            // dash and is the only part that shows.
            if (inside[0] == '$')
            {
                int dash = inside.IndexOf('-');
                string symbol = dash < 0 ? inside[1..] : inside[1..dash];
                if (symbol.Length > 0) yield return Token.Literal(symbol);
                yield break;
            }

            // [h], [mm], [ss]: elapsed time rather than a clock reading.
            char first = char.ToLowerInvariant(inside[0]);
            if (first is 'h' or 'm' or 's' && inside.All(c => char.ToLowerInvariant(c) == first))
            {
                yield return new Token(TokenKind.Date, first, inside.Length, null, true);
                yield break;
            }

            // Colours and conditions. A colour changes nothing about the text; a condition
            // selects between sections and is not reproduced, so it is reported instead.
            if (inside.StartsWith('>') || inside.StartsWith('<') || inside.StartsWith('='))
                yield return new Token(TokenKind.Unsupported, '[', 0, inside, false);
        }

        private static bool IsDateLetter(char c)
            => char.ToLowerInvariant(c) is 'y' or 'm' or 'd' or 'h' or 's';

        private static bool Matches(string code, int at, string word)
            => at + word.Length <= code.Length
               && code.AsSpan(at, word.Length).Equals(word, StringComparison.OrdinalIgnoreCase);

        public string FormatText(string text)
        {
            StringBuilder builder = new();
            foreach (Token token in _tokens)
            {
                if (token.Kind == TokenKind.Text) builder.Append(text);
                else if (token.Kind == TokenKind.Literal) builder.Append(token.Text);
            }

            return builder.ToString();
        }

        public string Format(double value, DateTime epoch)
        {
            if (_general || (_integerPlaces == 0 && _decimalPattern.Length == 0 && !HasDate && !HasTime))
                return FormatGeneral(value);

            return HasDate || HasTime ? FormatDateTime(value, epoch) : FormatNumber(value);
        }

        /// <summary>
        /// The General format: as many significant digits as the value needs, up to the fifteen
        /// a double carries reliably.
        /// </summary>
        /// <remarks>
        /// Calc's General is width-dependent on screen — it shortens a number to fit its column
        /// and shows <c>###</c> when it cannot — but its text and CSV exports use the full form,
        /// which is what this reproduces.
        /// </remarks>
        private string FormatGeneral(double value)
        {
            StringBuilder builder = new();
            foreach (Token token in _tokens)
            {
                if (token.Kind == TokenKind.General) builder.Append(GeneralText(value));
                else if (token.Kind == TokenKind.Literal) builder.Append(token.Text);
            }

            return builder.Length == 0 ? GeneralText(value) : builder.ToString();
        }

        private static string GeneralText(double value)
        {
            if (double.IsNaN(value) || double.IsInfinity(value)) return "#VALUE!";
            if (value == 0) return "0";

            string text = value.ToString("G15", CultureInfo.InvariantCulture);

            // .NET writes E+20 where a spreadsheet writes E+20 as well, but it writes E-05
            // with two exponent digits and a spreadsheet writes E-05 too, so the only
            // difference left is .NET's "1E+20" versus Calc's "1E+20" — none. Round-tripping
            // through decimal notation when the exponent is small keeps 0.000123 readable.
            return text;
        }

        private string FormatNumber(double value)
        {
            double scaled = value;
            for (int i = 0; i < _percents; i++) scaled *= 100;
            for (int i = 0; i < _scaleDivisions; i++) scaled /= 1000;

            string integerDigits;
            string decimalDigits;
            string exponent = string.Empty;

            if (_exponentDigits > 0)
            {
                int power = 0;
                double mantissa = scaled;
                if (mantissa != 0)
                {
                    power = (int)Math.Floor(Math.Log10(Math.Abs(mantissa)));
                    // The mantissa's integer part is as wide as the pattern asks for, so
                    // "##0.0E+0" shows 12.3E+3 where "0.00E+00" shows 1.23E+04.
                    power -= Math.Max(_integerPlaces, 1) - 1;
                    mantissa /= Math.Pow(10, power);
                }

                Split(mantissa, out integerDigits, out decimalDigits);
                string sign = power < 0 ? "-" : _exponentSigned ? "+" : string.Empty;
                exponent = sign + Math.Abs(power).ToString(CultureInfo.InvariantCulture)
                                      .PadLeft(_exponentDigits, '0');
            }
            else
            {
                Split(scaled, out integerDigits, out decimalDigits);
            }

            if (_grouped) integerDigits = Group(integerDigits);

            return Emit(integerDigits, decimalDigits, exponent);
        }

        /// <summary>
        /// Rounds to the pattern's decimal places and splits the result into its two halves.
        /// </summary>
        /// <remarks>
        /// The integer half is padded out to the <c>0</c> placeholders and the fractional half
        /// trimmed back where its placeholders are <c>#</c>, which is what makes <c>0.##</c>
        /// show 1.5 where <c>0.00</c> shows 1.50.
        /// </remarks>
        private void Split(double value, out string integerDigits, out string decimalDigits)
        {
            string text = Math.Abs(value).ToString(
                "F" + _decimalPattern.Length.ToString(CultureInfo.InvariantCulture),
                CultureInfo.InvariantCulture);

            int dot = text.IndexOf('.');
            integerDigits = dot < 0 ? text : text[..dot];
            decimalDigits = dot < 0 ? string.Empty : text[(dot + 1)..];

            if (integerDigits == "0" && _integerZeros == 0) integerDigits = string.Empty;
            else if (integerDigits.Length < _integerZeros) integerDigits = integerDigits.PadLeft(_integerZeros, '0');

            int keep = decimalDigits.Length;
            while (keep > 0 && decimalDigits[keep - 1] == '0' && _decimalPattern[keep - 1] != '0') keep--;
            decimalDigits = decimalDigits[..keep];
        }

        private static string Group(string digits)
        {
            if (digits.Length <= 3) return digits;

            StringBuilder builder = new(digits.Length + digits.Length / 3);
            int lead = digits.Length % 3;
            if (lead > 0) builder.Append(digits, 0, lead);
            for (int i = lead; i < digits.Length; i += 3)
            {
                if (builder.Length > 0) builder.Append(',');
                builder.Append(digits, i, 3);
            }

            return builder.ToString();
        }

        /// <summary>
        /// Interleaves the digits with the section's literals.
        /// </summary>
        /// <remarks>
        /// The whole integer part is emitted where the first integer placeholder stands rather
        /// than one digit per placeholder, because the number of digits a value has and the
        /// number of placeholders a pattern has are unrelated — <c>#,##0</c> has four and must
        /// still show seven digits — and because the grouping commas the digits already carry
        /// would otherwise be counted as digit positions.
        /// </remarks>
        private string Emit(string integerDigits, string decimalDigits, string exponent)
        {
            StringBuilder builder = new();
            bool integerEmitted = false;
            bool decimalEmitted = false;
            bool afterDecimal = false;

            foreach (Token token in _tokens)
            {
                switch (token.Kind)
                {
                    case TokenKind.Literal:
                        builder.Append(token.Text);
                        break;

                    case TokenKind.Digit when !afterDecimal:
                        if (!integerEmitted)
                        {
                            builder.Append(integerDigits);
                            integerEmitted = true;
                        }

                        break;

                    case TokenKind.Digit:
                        if (!decimalEmitted)
                        {
                            builder.Append(decimalDigits);
                            decimalEmitted = true;
                        }

                        break;

                    case TokenKind.Decimal:
                        afterDecimal = true;
                        if (decimalDigits.Length > 0) builder.Append('.');
                        break;

                    case TokenKind.Exponent:
                        builder.Append('E').Append(exponent);
                        break;

                    default:
                        break;
                }
            }

            return builder.ToString();
        }

        private string FormatDateTime(double value, DateTime epoch)
        {
            // An elapsed time is a count of days, not a point on the calendar, so it is
            // decomposed before the epoch is ever consulted.
            double days = value;
            TimeSpan elapsed = TimeSpan.FromDays(days);
            DateTime moment;
            try
            {
                moment = epoch.AddDays(days);
            }
            catch (ArgumentOutOfRangeException)
            {
                return GeneralText(value);
            }

            // Rounding to the second before rendering avoids 14:29:59.9999 printing as 14:29.
            double secondsOfDay = (days - Math.Floor(days)) * 86400;
            int totalSeconds = (int)Math.Round(secondsOfDay, MidpointRounding.AwayFromZero);
            if (totalSeconds >= 86400)
            {
                totalSeconds = 0;
                moment = moment.Date.AddDays(1);
            }
            else
            {
                moment = moment.Date.AddSeconds(totalSeconds);
            }

            int hour = totalSeconds / 3600;
            int minute = totalSeconds / 60 % 60;
            int second = totalSeconds % 60;
            bool pm = hour >= 12;
            int clockHour = _twelveHour ? (hour % 12 == 0 ? 12 : hour % 12) : hour;

            StringBuilder builder = new();
            foreach (Token token in _tokens)
            {
                switch (token.Kind)
                {
                    case TokenKind.Literal:
                        builder.Append(token.Text);
                        break;

                    case TokenKind.AmPm:
                        builder.Append(token.Symbol == 'A' ? (pm ? "PM" : "AM") : pm ? "P" : "A");
                        break;

                    case TokenKind.Date when token.Elapsed:
                        builder.Append(Elapsed(token.Symbol, elapsed, token.Count));
                        break;

                    case TokenKind.Date:
                        builder.Append(DatePart(token, moment, clockHour, minute, second));
                        break;

                    default:
                        break;
                }
            }

            return builder.ToString();
        }

        private static string Elapsed(char symbol, TimeSpan elapsed, int width)
        {
            double total = symbol switch
            {
                'h' => elapsed.TotalHours,
                'm' => elapsed.TotalMinutes,
                _ => elapsed.TotalSeconds,
            };

            return ((long)Math.Floor(total)).ToString(CultureInfo.InvariantCulture)
                                            .PadLeft(width, '0');
        }

        private static string DatePart(Token token, DateTime moment, int hour, int minute, int second)
        {
            CultureInfo culture = CultureInfo.InvariantCulture;
            return token.Symbol switch
            {
                'y' => token.Count <= 2
                    ? (moment.Year % 100).ToString("00", culture)
                    : moment.Year.ToString("0000", culture),
                'M' => token.Count switch
                {
                    1 => moment.Month.ToString(culture),
                    2 => moment.Month.ToString("00", culture),
                    3 => culture.DateTimeFormat.AbbreviatedMonthNames[moment.Month - 1],
                    4 => culture.DateTimeFormat.MonthNames[moment.Month - 1],
                    _ => culture.DateTimeFormat.MonthNames[moment.Month - 1][..1],
                },
                'd' => token.Count switch
                {
                    1 => moment.Day.ToString(culture),
                    2 => moment.Day.ToString("00", culture),
                    3 => culture.DateTimeFormat.AbbreviatedDayNames[(int)moment.DayOfWeek],
                    _ => culture.DateTimeFormat.DayNames[(int)moment.DayOfWeek],
                },
                'h' => token.Count <= 1 ? hour.ToString(culture) : hour.ToString("00", culture),
                'm' => token.Count <= 1 ? minute.ToString(culture) : minute.ToString("00", culture),
                's' => token.Count <= 1 ? second.ToString(culture) : second.ToString("00", culture),
                _ => string.Empty,
            };
        }
    }

    private enum TokenKind
    {
        Literal,
        Digit,
        Decimal,
        Group,
        Percent,
        Exponent,
        Text,
        General,
        Date,
        AmPm,
        Unsupported,
    }

    private readonly record struct Token(TokenKind Kind, char Symbol, int Count, string? Text, bool Elapsed)
    {
        public static Token Literal(string text) => new(TokenKind.Literal, '\0', 0, text, false);

        public static Token Digit(char symbol) => new(TokenKind.Digit, symbol, 0, null, false);
    }
}
