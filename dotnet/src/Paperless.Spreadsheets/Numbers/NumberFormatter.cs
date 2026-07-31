using System.Globalization;
using System.Text;

namespace Paperless.Spreadsheets.Numbers;

/// <summary>
/// Renders a stored cell value as the text a spreadsheet application would display for it.
/// </summary>
/// <remarks>
/// <para>
/// SpreadsheetML caches no display text — unlike ODF, which writes the rendered string beside
/// the value — so a date is nothing but a serial number until its format is applied. Producing
/// the same shape of content tree for both therefore means reimplementing the formatter, not
/// merely reading one.
/// </para>
/// <para>
/// Two things are deliberately out of scope, because both need a column width that extraction
/// does not have: the <c>###</c> a too-narrow numeric cell shows, and the way <c>General</c>
/// picks between fixed and scientific notation by how many characters fit. What is produced
/// here is what the cell shows at a width that fits it.
/// </para>
/// </remarks>
public static class NumberFormatter
{
    /// <summary>
    /// Formats a number through a format code.
    /// </summary>
    /// <param name="code">The parsed format code.</param>
    /// <param name="value">The stored value.</param>
    /// <param name="system">The workbook's date epoch, used only by date and time formats.</param>
    public static string Format(
        NumberFormatCode code, double value,
        SpreadsheetDateSystem system = SpreadsheetDateSystem.Date1900)
    {
        ArgumentNullException.ThrowIfNull(code);

        NumberFormatSection section = code.SelectFor(value);

        // When the code supplies a subformat of its own for negatives, the minus sign is that
        // subformat's business — usually a literal, sometimes parentheses. Emitting one here
        // as well is how "-1,234.50" becomes "--1,234.50".
        bool suppressSign = code.Sections.Count >= 2 && value < 0;

        return Render(section, value, suppressSign, system);
    }

    /// <summary>
    /// Formats a string through a format code: the fourth subformat when the code has one.
    /// </summary>
    public static string Format(NumberFormatCode code, string text)
    {
        ArgumentNullException.ThrowIfNull(code);
        ArgumentNullException.ThrowIfNull(text);

        NumberFormatSection? section = code.SelectForText();
        if (section is null) return text;

        StringBuilder output = new();
        foreach (FormatToken token in section.Tokens)
        {
            switch (token.Kind)
            {
                case FormatTokenKind.TextPlaceholder or FormatTokenKind.GeneralPlaceholder:
                    output.Append(text);
                    break;
                case FormatTokenKind.Literal:
                    output.Append(token.Text);
                    break;
                default:
                    break;
            }
        }
        return output.ToString();
    }

    /// <summary>
    /// The <c>General</c> rendering of a number: as short as round-trips, without notation.
    /// </summary>
    /// <remarks>
    /// Fifteen significant digits, which is where a <see cref="double"/> stops being exactly
    /// decimal and is also what Calc shows in a wide enough column. Round-tripping at
    /// seventeen digits would surface floating-point noise — 0.1 + 0.2 as 0.30000000000000004
    /// — that no spreadsheet displays.
    /// </remarks>
    public static string General(double value)
    {
        if (double.IsNaN(value)) return "NaN";
        if (double.IsInfinity(value)) return value > 0 ? "INF" : "-INF";
        if (value == 0) return "0";

        string text = value.ToString("G15", CultureInfo.InvariantCulture);

        // G15 writes "1.2345E+20"; a spreadsheet writes "1.2345E+20" too, but with at least
        // two exponent digits, which .NET already guarantees. What it does not do is drop the
        // "+", so the two agree.
        return text;
    }

    private static string Render(
        NumberFormatSection section, double value, bool suppressSign, SpreadsheetDateSystem system)
    {
        switch (section.Kind)
        {
            case NumberFormatKind.General:
                return RenderGeneralSection(section, value, suppressSign);

            case NumberFormatKind.DateTime:
                return RenderDateTime(section, value, system);

            case NumberFormatKind.Number:
                return RenderNumber(section, value, suppressSign);

            default:
                // A text-only subformat applied to a number shows its literals; that is what a
                // format such as "TRUE";"TRUE";"FALSE" relies on.
                return RenderLiteralsOnly(section);
        }
    }

    private static string RenderLiteralsOnly(NumberFormatSection section)
    {
        StringBuilder output = new();
        foreach (FormatToken token in section.Tokens)
        {
            if (token.Kind == FormatTokenKind.Literal) output.Append(token.Text);
        }
        return output.ToString();
    }

    private static string RenderGeneralSection(
        NumberFormatSection section, double value, bool suppressSign)
    {
        double effective = suppressSign ? Math.Abs(value) : value;
        StringBuilder output = new();
        foreach (FormatToken token in section.Tokens)
        {
            switch (token.Kind)
            {
                case FormatTokenKind.GeneralPlaceholder:
                    output.Append(General(effective));
                    break;
                case FormatTokenKind.Literal:
                    output.Append(token.Text);
                    break;
                default:
                    break;
            }
        }
        return output.Length == 0 ? General(effective) : output.ToString();
    }

    // ---- numbers -----------------------------------------------------------------------

    private static string RenderNumber(NumberFormatSection section, double value, bool suppressSign)
    {
        IReadOnlyList<FormatToken> tokens = section.Tokens;

        double scaled = value;
        for (int i = 0; i < section.PercentCount; i++) scaled *= 100.0;
        for (int i = 0; i < section.ThousandScale; i++) scaled /= 1000.0;

        bool negative = scaled < 0 || (scaled == 0 && double.IsNegative(scaled));
        double magnitude = Math.Abs(scaled);

        int slash = IndexOf(tokens, FormatTokenKind.Slash);
        int exponent = IndexOf(tokens, FormatTokenKind.Exponent);
        int point = IndexOf(tokens, FormatTokenKind.DecimalPoint);

        StringBuilder output = new();
        if (negative && !suppressSign) output.Append('-');

        if (slash >= 0) RenderFraction(tokens, slash, magnitude, output);
        else if (exponent >= 0) RenderScientific(tokens, exponent, point, magnitude, output);
        else RenderFixed(tokens, point, magnitude, output);

        return output.ToString();
    }

    private static void RenderFixed(
        IReadOnlyList<FormatToken> tokens, int point, double magnitude, StringBuilder output)
    {
        int decimals = CountPlaceholders(tokens, point < 0 ? tokens.Count : point + 1, tokens.Count);
        string rounded = magnitude.ToString(
            "F" + Math.Min(decimals, 15).ToString(CultureInfo.InvariantCulture),
            CultureInfo.InvariantCulture);

        int dot = rounded.IndexOf('.', StringComparison.Ordinal);
        string integerDigits = dot < 0 ? rounded : rounded[..dot];
        string decimalDigits = dot < 0 ? string.Empty : rounded[(dot + 1)..];

        bool afterPoint = false;
        int decimalCursor = 0;
        int pointPosition = -1;

        for (int i = 0; i < tokens.Count; i++)
        {
            FormatToken token = tokens[i];
            switch (token.Kind)
            {
                case FormatTokenKind.Literal:
                    output.Append(token.Text);
                    break;

                case FormatTokenKind.DecimalPoint:
                    afterPoint = true;
                    pointPosition = output.Length;
                    output.Append('.');
                    break;

                case FormatTokenKind.Digits when !afterPoint:
                    output.Append(FormatIntegerPart(token, integerDigits));
                    break;

                case FormatTokenKind.Digits:
                    output.Append(FormatDecimalPart(token, decimalDigits, ref decimalCursor));
                    break;

                default:
                    break;
            }
        }

        // A format such as "#.##" holding a whole number produces nothing after the point, and
        // a trailing separator with no decimals behind it is not what the cell shows.
        if (pointPosition >= 0 && pointPosition == output.Length - 1) output.Length = pointPosition;
    }

    private static void RenderScientific(
        IReadOnlyList<FormatToken> tokens, int exponentIndex, int point, double magnitude,
        StringBuilder output)
    {
        int mantissaIntegerDigits = Math.Max(1, CountPlaceholders(tokens, 0, point < 0 ? exponentIndex : point));
        int mantissaDecimals = point < 0
            ? 0
            : CountPlaceholders(tokens, point + 1, exponentIndex);

        int exponent = 0;
        double mantissa = magnitude;
        if (magnitude != 0)
        {
            exponent = (int)Math.Floor(Math.Log10(magnitude));
            // Engineering notation: "##0.0E+0" keeps the exponent a multiple of three by
            // letting the mantissa grow to three integer digits. With a single integer
            // placeholder this is a no-op and the mantissa stays in [1, 10).
            exponent -= Mod(exponent, mantissaIntegerDigits);
            mantissa = magnitude / Math.Pow(10, exponent);

            // Rounding the mantissa can carry it past its digit budget: 9.99 to two decimals
            // with one integer digit becomes 10.0, which needs the exponent bumped.
            double limit = Math.Pow(10, mantissaIntegerDigits);
            if (Math.Round(mantissa, mantissaDecimals) >= limit)
            {
                exponent += mantissaIntegerDigits;
                mantissa = magnitude / Math.Pow(10, exponent);
            }
        }

        string rounded = mantissa.ToString(
            "F" + mantissaDecimals.ToString(CultureInfo.InvariantCulture), CultureInfo.InvariantCulture);
        int dot = rounded.IndexOf('.', StringComparison.Ordinal);
        string integerDigits = dot < 0 ? rounded : rounded[..dot];
        string decimalDigits = dot < 0 ? string.Empty : rounded[(dot + 1)..];

        bool afterPoint = false;
        bool afterExponent = false;
        int decimalCursor = 0;

        for (int i = 0; i < tokens.Count; i++)
        {
            FormatToken token = tokens[i];
            switch (token.Kind)
            {
                case FormatTokenKind.Literal:
                    output.Append(token.Text);
                    break;

                case FormatTokenKind.DecimalPoint:
                    afterPoint = true;
                    output.Append('.');
                    break;

                case FormatTokenKind.Exponent:
                    afterExponent = true;
                    output.Append('E');
                    if (exponent < 0) output.Append('-');
                    else if (token.Flag) output.Append('+');
                    break;

                case FormatTokenKind.Digits when afterExponent:
                    output.Append(Math.Abs(exponent).ToString(
                        CultureInfo.InvariantCulture).PadLeft(CountZeros(token.Text), '0'));
                    break;

                case FormatTokenKind.Digits when afterPoint:
                    output.Append(FormatDecimalPart(token, decimalDigits, ref decimalCursor));
                    break;

                case FormatTokenKind.Digits:
                    output.Append(FormatIntegerPart(token, integerDigits));
                    break;

                default:
                    break;
            }
        }
    }

    private static void RenderFraction(
        IReadOnlyList<FormatToken> tokens, int slash, double magnitude, StringBuilder output)
    {
        int numeratorIndex = PreviousDigits(tokens, slash);
        int denominatorIndex = NextDigits(tokens, slash);
        int wholeIndex = numeratorIndex < 0 ? -1 : PreviousDigits(tokens, numeratorIndex);

        int fixedDenominator = denominatorIndex < 0 ? LiteralDenominator(tokens, slash) : 0;
        int denominatorDigits = denominatorIndex < 0
            ? 0
            : tokens[denominatorIndex].Text.Length;

        double whole = wholeIndex >= 0 ? Math.Truncate(magnitude) : 0;
        double remainder = magnitude - whole;

        long numerator;
        long denominator;
        if (fixedDenominator > 0)
        {
            denominator = fixedDenominator;
            numerator = (long)Math.Round(remainder * denominator);
        }
        else
        {
            long maximum = (long)Math.Pow(10, Math.Max(1, denominatorDigits)) - 1;
            (numerator, denominator) = BestFraction(remainder, maximum);
        }

        // Rounding can push the fraction to a whole unit: 0.99 over a maximum of 9 is 1/1.
        if (denominator > 0 && numerator == denominator)
        {
            whole += 1;
            numerator = 0;
        }

        bool afterSlash = false;
        for (int i = 0; i < tokens.Count; i++)
        {
            FormatToken token = tokens[i];
            switch (token.Kind)
            {
                case FormatTokenKind.Literal:
                    output.Append(token.Text);
                    break;

                case FormatTokenKind.Slash:
                    afterSlash = true;
                    // Excel blanks the whole fraction when there is nothing left over, so a
                    // "0 ?/?" cell holding 2 reads "2" rather than "2 0/1".
                    output.Append(numerator == 0 ? ' ' : '/');
                    break;

                case FormatTokenKind.Digits when i == wholeIndex:
                    output.Append(FormatIntegerPart(
                        token, whole.ToString("F0", CultureInfo.InvariantCulture)));
                    break;

                case FormatTokenKind.Digits when i == numeratorIndex:
                    output.Append(numerator == 0
                        ? new string(' ', token.Text.Length)
                        : FormatIntegerPart(token, numerator.ToString(CultureInfo.InvariantCulture)));
                    break;

                case FormatTokenKind.Digits when afterSlash:
                    output.Append(numerator == 0
                        ? new string(' ', token.Text.Length)
                        : FormatIntegerPart(token, denominator.ToString(CultureInfo.InvariantCulture)));
                    break;

                default:
                    break;
            }
        }
    }

    /// <summary>
    /// The closest fraction with a denominator no larger than <paramref name="maximum"/>.
    /// </summary>
    /// <remarks>
    /// A continued-fraction expansion rather than a search: it reaches the best approximation
    /// in a handful of steps where scanning every denominator would take up to 10^n of them,
    /// and it is the same convergent sequence Excel's own fraction formats land on.
    /// </remarks>
    private static (long Numerator, long Denominator) BestFraction(double value, long maximum)
    {
        if (maximum < 1) maximum = 1;
        if (value <= 0) return (0, 1);

        long previousNumerator = 0, previousDenominator = 1;
        long numerator = 1, denominator = 0;
        double remainder = value;

        for (int step = 0; step < 32; step++)
        {
            long whole = (long)Math.Floor(remainder);
            long nextNumerator = whole * numerator + previousNumerator;
            long nextDenominator = whole * denominator + previousDenominator;
            if (nextDenominator > maximum) break;

            previousNumerator = numerator;
            previousDenominator = denominator;
            numerator = nextNumerator;
            denominator = nextDenominator;

            double fraction = remainder - whole;
            if (fraction < 1e-12) break;
            remainder = 1.0 / fraction;
        }

        if (denominator == 0) return (0, 1);
        return (numerator, denominator);
    }

    private static int LiteralDenominator(IReadOnlyList<FormatToken> tokens, int slash)
    {
        StringBuilder digits = new();
        for (int i = slash + 1; i < tokens.Count; i++)
        {
            if (tokens[i].Kind != FormatTokenKind.Literal) break;
            foreach (char c in tokens[i].Text)
            {
                if (!char.IsAsciiDigit(c)) return Parse(digits);
                digits.Append(c);
            }
        }
        return Parse(digits);

        static int Parse(StringBuilder digits)
            => digits.Length > 0
               && int.TryParse(digits.ToString(), NumberStyles.Integer, CultureInfo.InvariantCulture,
                               out int value)
               && value > 0
                ? value
                : 0;
    }

    /// <summary>
    /// Lays a run of digits into a placeholder run, right-aligned.
    /// </summary>
    /// <remarks>
    /// The three placeholders differ only in what they do when there is no digit to show:
    /// <c>0</c> writes a zero, <c>?</c> writes a space so that columns of fractions line up,
    /// and <c>#</c> writes nothing. Digits beyond the run's width are never dropped — a format
    /// narrower than its value still shows the value.
    /// </remarks>
    private static string FormatIntegerPart(FormatToken token, string digits)
    {
        string placeholders = token.Text;

        int zeros = 0;
        foreach (char c in placeholders)
        {
            if (c == '0') zeros++;
        }

        string body = digits.TrimStart('0');
        if (body.Length < zeros) body = body.PadLeft(zeros, '0');

        StringBuilder output = new();
        for (int i = 0; i < placeholders.Length - body.Length; i++)
        {
            if (placeholders[i] == '?') output.Append(' ');
        }
        output.Append(body);

        string laid = output.ToString();
        return token.Flag ? Group(laid) : laid;
    }

    private static string FormatDecimalPart(FormatToken token, string digits, ref int cursor)
    {
        string placeholders = token.Text;
        StringBuilder output = new();

        for (int i = 0; i < placeholders.Length; i++, cursor++)
        {
            char digit = cursor < digits.Length ? digits[cursor] : '0';
            switch (placeholders[i])
            {
                case '0':
                    output.Append(digit);
                    break;
                case '?':
                    output.Append(RemainingIsZero(digits, cursor) ? ' ' : digit);
                    break;
                default:
                    if (RemainingIsZero(digits, cursor)) return output.ToString();
                    output.Append(digit);
                    break;
            }
        }
        return output.ToString();

        static bool RemainingIsZero(string digits, int from)
        {
            for (int i = from; i < digits.Length; i++)
            {
                if (digits[i] != '0') return false;
            }
            return true;
        }
    }

    /// <summary>Inserts thousands separators among the digits of an already laid-out run.</summary>
    private static string Group(string laid)
    {
        int lastDigit = laid.Length - 1;
        while (lastDigit >= 0 && !char.IsAsciiDigit(laid[lastDigit])) lastDigit--;
        if (lastDigit < 0) return laid;

        int firstDigit = 0;
        while (firstDigit <= lastDigit && !char.IsAsciiDigit(laid[firstDigit])) firstDigit++;

        StringBuilder output = new();
        output.Append(laid, 0, firstDigit);
        int count = lastDigit - firstDigit + 1;
        for (int i = 0; i < count; i++)
        {
            if (i > 0 && (count - i) % 3 == 0) output.Append(',');
            output.Append(laid[firstDigit + i]);
        }
        output.Append(laid, lastDigit + 1, laid.Length - lastDigit - 1);
        return output.ToString();
    }

    // ---- dates and times ---------------------------------------------------------------

    private static readonly string[] MonthNames =
    [
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December",
    ];

    private static readonly string[] MonthAbbreviations =
    [
        "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
    ];

    private static readonly string[] DayNames =
    [
        "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday",
    ];

    private static readonly string[] DayAbbreviations =
    [
        "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
    ];

    private static string RenderDateTime(
        NumberFormatSection section, double value, SpreadsheetDateSystem system)
    {
        DateTime? moment = SpreadsheetDate.FromSerial(value, system);
        if (moment is null) return General(value);

        DateTime when = moment.Value;
        // Excel rounds seconds up rather than truncating, and a format without a seconds
        // field rounds the minutes; without this a stored 14:29:59.6 shows as 14:29.
        when = RoundForPrecision(when, section);

        StringBuilder output = new();
        bool twelveHour = section.TwelveHour;

        foreach (FormatToken token in section.Tokens)
        {
            switch (token.Kind)
            {
                case FormatTokenKind.Literal:
                    output.Append(token.Text);
                    break;

                case FormatTokenKind.DecimalPoint:
                    output.Append('.');
                    break;

                case FormatTokenKind.Digits:
                    // Only sub-second digits can follow a decimal point in a time format.
                    output.Append(FractionalSeconds(when, token.Text.Length));
                    break;

                case FormatTokenKind.AmPm:
                    output.Append(when.Hour < 12
                        ? token.Flag ? "AM" : "A"
                        : token.Flag ? "PM" : "P");
                    break;

                case FormatTokenKind.Elapsed:
                    output.Append(Elapsed(value, token.Symbol, token.Count));
                    break;

                case FormatTokenKind.DateTime:
                    output.Append(DateField(when, token.Symbol, token.Count, twelveHour));
                    break;

                default:
                    break;
            }
        }
        return output.ToString();
    }

    private static DateTime RoundForPrecision(DateTime when, NumberFormatSection section)
    {
        bool hasSeconds = false;
        bool hasSubSeconds = false;
        bool sawPoint = false;

        foreach (FormatToken token in section.Tokens)
        {
            if (token.Kind == FormatTokenKind.DecimalPoint) { sawPoint = true; continue; }
            if (token.Kind == FormatTokenKind.Digits && sawPoint) hasSubSeconds = true;
            if ((token.Kind == FormatTokenKind.DateTime || token.Kind == FormatTokenKind.Elapsed)
                && token.Symbol == 's')
                hasSeconds = true;
        }

        if (hasSubSeconds) return when;
        if (hasSeconds)
        {
            return when.Millisecond >= 500 ? when.AddMilliseconds(1000 - when.Millisecond) : when.AddMilliseconds(-when.Millisecond);
        }

        // Round to the minute.
        double intoMinute = when.Second + (when.Millisecond / 1000.0);
        DateTime floor = when.AddSeconds(-intoMinute);
        return intoMinute >= 30 ? floor.AddMinutes(1) : floor;
    }

    private static string FractionalSeconds(DateTime when, int digits)
    {
        double fraction = when.Millisecond / 1000.0;
        string text = fraction.ToString(
            "F" + Math.Min(digits, 3).ToString(CultureInfo.InvariantCulture), CultureInfo.InvariantCulture);
        int dot = text.IndexOf('.', StringComparison.Ordinal);
        string decimals = dot < 0 ? string.Empty : text[(dot + 1)..];
        return decimals.PadRight(digits, '0')[..digits];
    }

    private static string Elapsed(double serial, char unit, int digits)
    {
        TimeSpan span = SpreadsheetDate.ToTimeOfDay(serial, keepWholeDays: true);
        double total = unit switch
        {
            'h' => Math.Floor(span.TotalHours),
            'm' => Math.Floor(span.TotalMinutes),
            _ => Math.Floor(span.TotalSeconds),
        };
        return total.ToString("F0", CultureInfo.InvariantCulture)
                    .PadLeft(digits, '0');
    }

    private static string DateField(DateTime when, char symbol, int count, bool twelveHour)
        => symbol switch
        {
            'y' => count <= 2
                ? (when.Year % 100).ToString("D2", CultureInfo.InvariantCulture)
                : when.Year.ToString("D4", CultureInfo.InvariantCulture),

            // 'm' is a month here; the parser has already rewritten minutes to 'n'.
            'm' => count switch
            {
                1 => when.Month.ToString(CultureInfo.InvariantCulture),
                2 => when.Month.ToString("D2", CultureInfo.InvariantCulture),
                3 => MonthAbbreviations[when.Month - 1],
                4 => MonthNames[when.Month - 1],
                _ => MonthNames[when.Month - 1][..1],
            },

            'n' => count <= 1
                ? when.Minute.ToString(CultureInfo.InvariantCulture)
                : when.Minute.ToString("D2", CultureInfo.InvariantCulture),

            'd' => count switch
            {
                1 => when.Day.ToString(CultureInfo.InvariantCulture),
                2 => when.Day.ToString("D2", CultureInfo.InvariantCulture),
                3 => DayAbbreviations[(int)when.DayOfWeek],
                _ => DayNames[(int)when.DayOfWeek],
            },

            'h' => Hour(when, twelveHour, count),

            's' => count <= 1
                ? when.Second.ToString(CultureInfo.InvariantCulture)
                : when.Second.ToString("D2", CultureInfo.InvariantCulture),

            // 'g' and 'e' are era and era-year in Far Eastern calendars; without calendar
            // support the Gregorian year is the honest answer.
            'e' => when.Year.ToString("D4", CultureInfo.InvariantCulture),

            _ => string.Empty,
        };

    private static string Hour(DateTime when, bool twelveHour, int count)
    {
        int hour = when.Hour;
        if (twelveHour)
        {
            hour %= 12;
            if (hour == 0) hour = 12;
        }
        return count <= 1
            ? hour.ToString(CultureInfo.InvariantCulture)
            : hour.ToString("D2", CultureInfo.InvariantCulture);
    }

    // ---- token helpers -----------------------------------------------------------------

    private static int IndexOf(IReadOnlyList<FormatToken> tokens, FormatTokenKind kind)
    {
        for (int i = 0; i < tokens.Count; i++)
        {
            if (tokens[i].Kind == kind) return i;
        }
        return -1;
    }

    private static int PreviousDigits(IReadOnlyList<FormatToken> tokens, int from)
    {
        for (int i = from - 1; i >= 0; i--)
        {
            if (tokens[i].Kind == FormatTokenKind.Digits) return i;
        }
        return -1;
    }

    private static int NextDigits(IReadOnlyList<FormatToken> tokens, int from)
    {
        for (int i = from + 1; i < tokens.Count; i++)
        {
            if (tokens[i].Kind == FormatTokenKind.Digits) return i;
            if (tokens[i].Kind != FormatTokenKind.Literal) break;
        }
        return -1;
    }

    private static int CountPlaceholders(IReadOnlyList<FormatToken> tokens, int start, int end)
    {
        int count = 0;
        for (int i = Math.Max(0, start); i < Math.Min(end, tokens.Count); i++)
        {
            if (tokens[i].Kind == FormatTokenKind.Digits) count += tokens[i].Text.Length;
        }
        return count;
    }

    private static int CountZeros(string placeholders)
    {
        int count = 0;
        foreach (char c in placeholders)
        {
            if (c == '0') count++;
        }
        return Math.Max(count, 1);
    }

    private static int Mod(int value, int modulus)
    {
        if (modulus <= 0) return 0;
        int remainder = value % modulus;
        return remainder < 0 ? remainder + modulus : remainder;
    }
}
