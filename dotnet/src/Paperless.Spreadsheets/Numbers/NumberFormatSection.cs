using System.Globalization;

namespace Paperless.Spreadsheets.Numbers;

/// <summary>
/// One semicolon-separated subformat of a <see cref="NumberFormatCode"/>, parsed into the
/// tokens that render it.
/// </summary>
/// <remarks>
/// Tokenising once and rendering from tokens — rather than interpreting the code character by
/// character at every cell — matters because a sheet applies the same handful of formats to
/// tens of thousands of cells.
/// </remarks>
public sealed class NumberFormatSection
{
    private readonly List<FormatToken> _tokens;

    private NumberFormatSection(
        string code,
        List<FormatToken> tokens,
        NumberFormatKind kind,
        NumberFormatCondition? condition,
        int scaleByPercent,
        int scaleByThousand,
        bool hasDatePart,
        bool hasTimePart,
        bool twelveHour,
        bool hasElapsed)
    {
        Code = code;
        _tokens = tokens;
        Kind = kind;
        Condition = condition;
        PercentCount = scaleByPercent;
        ThousandScale = scaleByThousand;
        HasDatePart = hasDatePart;
        HasTimePart = hasTimePart;
        TwelveHour = twelveHour;
        HasElapsed = hasElapsed;
    }

    /// <summary>The subformat as written.</summary>
    public string Code { get; }

    /// <summary>What this subformat produces.</summary>
    public NumberFormatKind Kind { get; }

    /// <summary>The <c>[&gt;=100]</c>-style condition guarding this subformat, if any.</summary>
    public NumberFormatCondition? Condition { get; }

    /// <summary>How many <c>%</c> signs the subformat contains; each multiplies by 100.</summary>
    public int PercentCount { get; }

    /// <summary>How many trailing commas scale the value down by a thousand each.</summary>
    public int ThousandScale { get; }

    /// <summary>True when the subformat shows a year, month or day.</summary>
    public bool HasDatePart { get; }

    /// <summary>True when the subformat shows an hour, minute or second.</summary>
    public bool HasTimePart { get; }

    /// <summary>True when an AM/PM marker makes the hours run 1–12.</summary>
    public bool TwelveHour { get; }

    /// <summary>True when the subformat uses a bracketed elapsed unit such as <c>[h]</c>.</summary>
    public bool HasElapsed { get; }

    /// <summary>True when this is the bare <c>General</c> subformat.</summary>
    public bool IsGeneral => Kind == NumberFormatKind.General;

    /// <summary>The tokens, for the renderer.</summary>
    internal IReadOnlyList<FormatToken> Tokens => _tokens;

    /// <summary>
    /// Parses one subformat.
    /// </summary>
    /// <remarks>
    /// The month-versus-minute ambiguity is resolved here rather than at render time, because
    /// it depends on neighbouring tokens: <c>m</c> is a minute when it follows an hour or
    /// precedes a second, and a month otherwise. A renderer that decided per token would need
    /// the same lookaround at every cell.
    /// </remarks>
    public static NumberFormatSection Parse(string code)
    {
        ArgumentNullException.ThrowIfNull(code);

        List<FormatToken> tokens = [];
        NumberFormatCondition? condition = null;
        int percents = 0;
        bool sawGeneral = false;
        bool sawDigits = false;
        bool sawDateTime = false;
        bool twelveHour = false;
        bool hasElapsed = false;

        for (int i = 0; i < code.Length;)
        {
            char c = code[i];

            switch (c)
            {
                case '"':
                {
                    int end = code.IndexOf('"', i + 1);
                    string literal = end < 0 ? code[(i + 1)..] : code[(i + 1)..end];
                    tokens.Add(FormatToken.Literal(literal));
                    i = end < 0 ? code.Length : end + 1;
                    continue;
                }

                case '\\':
                    if (i + 1 < code.Length) tokens.Add(FormatToken.Literal(code[i + 1].ToString()));
                    i += 2;
                    continue;

                // "Reserve the width of the next character." There is no column to align to
                // during extraction, so a single space stands in — dropping it entirely would
                // run an accounting format's currency symbol into its digits.
                case '_':
                    tokens.Add(FormatToken.Literal(" "));
                    i += 2;
                    continue;

                // "Repeat the next character to fill the column." Column-width dependent, so
                // it contributes nothing to extracted text.
                case '*':
                    i += 2;
                    continue;

                case '[':
                {
                    int end = code.IndexOf(']', i + 1);
                    string body = end < 0 ? code[(i + 1)..] : code[(i + 1)..end];
                    i = end < 0 ? code.Length : end + 1;

                    if (NumberFormatCondition.TryParse(body) is { } parsed)
                    {
                        condition ??= parsed;
                    }
                    else if (body.StartsWith('$'))
                    {
                        // [$symbol-locale]: the symbol is real text, the locale tag is not.
                        string symbol = body[1..];
                        int dash = symbol.IndexOf('-', StringComparison.Ordinal);
                        if (dash >= 0) symbol = symbol[..dash];
                        if (symbol.Length > 0) tokens.Add(FormatToken.Literal(symbol));
                    }
                    else if (IsElapsedUnit(body, out char unit))
                    {
                        tokens.Add(FormatToken.Elapsed(unit, body.Length));
                        hasElapsed = true;
                        sawDateTime = true;
                    }
                    // Anything else — a colour name, [ENG], [NatNum1] — changes appearance or
                    // numeral system, not the text this extracts.
                    continue;
                }

                case '@':
                    tokens.Add(FormatToken.TextPlaceholder());
                    i++;
                    continue;

                case '%':
                    percents++;
                    tokens.Add(FormatToken.Literal("%"));
                    i++;
                    continue;

                case '0' or '#' or '?':
                {
                    System.Text.StringBuilder run = new();
                    bool grouping = false;
                    while (i < code.Length)
                    {
                        char d = code[i];
                        if (d is '0' or '#' or '?') { run.Append(d); i++; continue; }
                        // A comma between placeholders groups thousands; one at the end of the
                        // run scales instead, and that is decided in a later pass.
                        if (d == ',' && i + 1 < code.Length && code[i + 1] is '0' or '#' or '?')
                        {
                            grouping = true;
                            i++;
                            continue;
                        }
                        break;
                    }
                    tokens.Add(FormatToken.Digits(run.ToString(), grouping));
                    sawDigits = true;
                    continue;
                }

                case '.':
                    tokens.Add(FormatToken.DecimalPoint());
                    i++;
                    continue;

                case ',':
                    // A comma not between placeholders is a scaling comma when it trails a
                    // digit run, and a plain literal otherwise.
                    tokens.Add(FormatToken.ScaleComma());
                    i++;
                    continue;

                case '/':
                    tokens.Add(FormatToken.Slash());
                    i++;
                    continue;

                // "E+"/"E-" is the exponent marker; a lone "e" is the Far Eastern era field,
                // so it falls through to the date-letter handling below.
                case 'E' or 'e' when i + 1 < code.Length && code[i + 1] is '+' or '-':
                    tokens.Add(FormatToken.Exponent(code[i + 1] == '+'));
                    i += 2;
                    continue;

                default:
                    break;
            }

            if (MatchesWord(code, i, "General"))
            {
                tokens.Add(FormatToken.GeneralPlaceholder());
                sawGeneral = true;
                i += "General".Length;
                continue;
            }

            if (MatchesWord(code, i, "AM/PM") || MatchesWord(code, i, "A/P"))
            {
                int length = MatchesWord(code, i, "AM/PM") ? 5 : 3;
                tokens.Add(FormatToken.AmPm(length == 5));
                twelveHour = true;
                sawDateTime = true;
                i += length;
                continue;
            }

            if (IsDateTimeLetter(c))
            {
                char lower = char.ToLowerInvariant(c);
                int count = 0;
                while (i + count < code.Length && char.ToLowerInvariant(code[i + count]) == lower) count++;
                tokens.Add(FormatToken.DateTime(lower, count));
                sawDateTime = true;
                i += count;
                continue;
            }

            tokens.Add(FormatToken.Literal(c.ToString()));
            i++;
        }

        int thousandScale = ResolveCommas(tokens);
        bool hasDate = false;
        bool hasTime = false;
        if (sawDateTime) ResolveMinutes(tokens, out hasDate, out hasTime);

        // A date format wins over stray digits, because "yyyy" and "0" can coexist only in a
        // format whose author meant a date; General wins over bare literals for the same
        // reason.
        NumberFormatKind kind =
            sawDateTime ? NumberFormatKind.DateTime
            : sawDigits ? NumberFormatKind.Number
            : sawGeneral ? NumberFormatKind.General
            : NumberFormatKind.Text;

        return new NumberFormatSection(
            code, tokens, kind, condition, percents, thousandScale,
            hasDate, hasTime, twelveHour, hasElapsed);
    }

    private static bool MatchesWord(string code, int index, string word)
        => index + word.Length <= code.Length
           && string.Compare(code, index, word, 0, word.Length, StringComparison.OrdinalIgnoreCase) == 0;

    private static bool IsDateTimeLetter(char c)
        => char.ToLowerInvariant(c) is 'y' or 'm' or 'd' or 'h' or 's' or 'g' or 'e' or 'b';

    private static bool IsElapsedUnit(string body, out char unit)
    {
        unit = '\0';
        if (body.Length is < 1 or > 4) return false;
        char first = char.ToLowerInvariant(body[0]);
        if (first is not ('h' or 'm' or 's')) return false;
        foreach (char c in body)
        {
            if (char.ToLowerInvariant(c) != first) return false;
        }
        unit = first;
        return true;
    }

    /// <summary>
    /// Decides which commas scale by a thousand, and drops them; the rest become literals.
    /// </summary>
    /// <remarks>
    /// A comma only scales when it trails the integer digits — <c>#,##0,,</c> means millions.
    /// One elsewhere is a literal separator, which is what a format like <c>0,0</c> in some
    /// locales means.
    /// </remarks>
    private static int ResolveCommas(List<FormatToken> tokens)
    {
        int scale = 0;
        for (int i = tokens.Count - 1; i >= 0; i--)
        {
            if (tokens[i].Kind != FormatTokenKind.ScaleComma) continue;

            // Scaling commas form a run directly after the integer digit run.
            int previous = i - 1;
            while (previous >= 0 && tokens[previous].Kind == FormatTokenKind.ScaleComma) previous--;

            if (previous >= 0 && tokens[previous].Kind == FormatTokenKind.Digits)
            {
                scale++;
                tokens.RemoveAt(i);
            }
            else
            {
                tokens[i] = FormatToken.Literal(",");
            }
        }
        return scale;
    }

    /// <summary>
    /// Rewrites each <c>m</c> run as a month or a minute, and reports which parts the
    /// subformat shows.
    /// </summary>
    private static void ResolveMinutes(
        List<FormatToken> tokens, out bool hasDate, out bool hasTime)
    {
        hasDate = false;
        hasTime = false;

        for (int i = 0; i < tokens.Count; i++)
        {
            FormatToken token = tokens[i];
            if (token.Kind == FormatTokenKind.Elapsed)
            {
                hasTime = true;
                continue;
            }
            if (token.Kind == FormatTokenKind.AmPm) { hasTime = true; continue; }
            if (token.Kind != FormatTokenKind.DateTime) continue;

            switch (token.Symbol)
            {
                case 'y' or 'd' or 'g' or 'e' or 'b':
                    hasDate = true;
                    break;
                case 'h' or 's':
                    hasTime = true;
                    break;
                case 'm':
                {
                    bool minute = PrecededByHour(tokens, i) || FollowedBySecond(tokens, i);
                    tokens[i] = FormatToken.DateTime(minute ? 'n' : 'm', token.Count);
                    if (minute) hasTime = true; else hasDate = true;
                    break;
                }
                default:
                    break;
            }
        }
    }

    private static bool PrecededByHour(List<FormatToken> tokens, int index)
    {
        for (int i = index - 1; i >= 0; i--)
        {
            FormatToken token = tokens[i];
            if (token.Kind == FormatTokenKind.Elapsed && token.Symbol == 'h') return true;
            if (token.Kind != FormatTokenKind.DateTime) continue;
            return token.Symbol == 'h';
        }
        return false;
    }

    private static bool FollowedBySecond(List<FormatToken> tokens, int index)
    {
        for (int i = index + 1; i < tokens.Count; i++)
        {
            FormatToken token = tokens[i];
            if (token.Kind == FormatTokenKind.Elapsed && token.Symbol == 's') return true;
            if (token.Kind != FormatTokenKind.DateTime) continue;
            return token.Symbol == 's';
        }
        return false;
    }

    /// <inheritdoc/>
    public override string ToString() => Code;

}

/// <summary>The kinds of token a subformat parses into.</summary>
internal enum FormatTokenKind
{
    Literal,
    Digits,
    DecimalPoint,
    ScaleComma,
    Slash,
    Exponent,
    DateTime,
    Elapsed,
    AmPm,
    TextPlaceholder,
    GeneralPlaceholder,
}

/// <summary>One token of a parsed subformat.</summary>
internal readonly record struct FormatToken(
    FormatTokenKind Kind,
    string Text,
    char Symbol,
    int Count,
    bool Flag)
{
    public static FormatToken Literal(string text)
        => new(FormatTokenKind.Literal, text, '\0', 0, false);

    public static FormatToken Digits(string placeholders, bool grouping)
        => new(FormatTokenKind.Digits, placeholders, '\0', placeholders.Length, grouping);

    public static FormatToken DecimalPoint()
        => new(FormatTokenKind.DecimalPoint, ".", '\0', 0, false);

    public static FormatToken ScaleComma()
        => new(FormatTokenKind.ScaleComma, ",", '\0', 0, false);

    public static FormatToken Slash()
        => new(FormatTokenKind.Slash, "/", '\0', 0, false);

    public static FormatToken Exponent(bool explicitPlus)
        => new(FormatTokenKind.Exponent, explicitPlus ? "E+" : "E-", '\0', 0, explicitPlus);

    public static FormatToken DateTime(char symbol, int count)
        => new(FormatTokenKind.DateTime, string.Empty, symbol, count, false);

    public static FormatToken Elapsed(char symbol, int count)
        => new(FormatTokenKind.Elapsed, string.Empty, symbol, count, false);

    public static FormatToken AmPm(bool longForm)
        => new(FormatTokenKind.AmPm, string.Empty, '\0', 0, longForm);

    public static FormatToken TextPlaceholder()
        => new(FormatTokenKind.TextPlaceholder, "@", '\0', 0, false);

    public static FormatToken GeneralPlaceholder()
        => new(FormatTokenKind.GeneralPlaceholder, "General", '\0', 0, false);
}
