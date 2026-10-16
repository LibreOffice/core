using System.Globalization;

namespace XRay.Office.Core.Numbers;

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
        bool hasElapsed,
        bool hasUnreproducedDirective,
        Graphics.Colour? colour)
    {
        Colour = colour;
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
        HasUnreproducedDirective = hasUnreproducedDirective;

        foreach (FormatToken token in tokens)
        {
            if (token.Kind != FormatTokenKind.Fill) continue;
            HasFillDirective = true;
            break;
        }
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

    /// <summary>
    /// True when the subformat carries a directive that changes the characters shown and is
    /// not reproduced — a numeral-system or calendar substitution.
    /// </summary>
    /// <remarks>
    /// <c>[NatNum1]</c> and <c>[DBNum2]</c> replace the Western digits with another numeral
    /// system, and <c>[~buddhist]</c> counts the years from another era; LibreOffice carries
    /// each as a modifier on the whole format (<c>svl/source/numbers/zforscan.cxx:215</c>).
    /// Ignoring one silently produces plausible digits that are not the ones the cell shows,
    /// which is worse than saying so — a caller can raise a diagnostic instead.
    /// </remarks>
    public bool HasUnreproducedDirective { get; }

    /// <summary>
    /// True when the subformat carries a <c>*c</c> fill directive, whose expansion needs a
    /// column width.
    /// </summary>
    /// <remarks>
    /// Worth asking before rendering rather than after: a caller that owns a width re-renders
    /// the value to find where the fill goes, and every other format would be re-rendered for
    /// nothing. Accounting formats — built-in ids 5–8, 41–44 and every <c>_("$"* …)</c> code
    /// Excel writes — are the ones that carry it.
    /// </remarks>
    public bool HasFillDirective { get; }

    /// <summary>
    /// The colour this subformat states, or null when it states none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>[Red]</c> in a subformat colours the cells that subformat formats, so it belongs to
    /// the <em>section</em> and not to the format: <c>#,##0 ;[Red](#,##0)</c> draws a positive
    /// in the cell's own colour and a negative in red, and which one a cell gets is decided by
    /// its value. It was dropped with the other bracketed bodies under the reasoning that a
    /// colour "changes appearance rather than the text this extracts" — true of the text, and
    /// false of the ink.
    /// </para>
    /// <para>
    /// <strong>The ten names and their values are LibreOffice's own</strong>, in its own order:
    /// <c>ImpSvNumberformatScan::StandardColor</c> is
    /// <c>{ BLACK, LIGHTBLUE, LIGHTGREEN, LIGHTCYAN, LIGHTRED, LIGHTMAGENTA, BROWN, GRAY,
    /// YELLOW, WHITE }</c> against the keywords <c>BLACK BLUE GREEN CYAN RED MAGENTA BROWN GREY
    /// YELLOW WHITE</c> (<c>svl/source/numbers/zforscan.cxx</c>:98-111, 145-147). So
    /// <c>[Blue]</c> is <c>#0000FF</c> and not <c>#000080</c>, and <c>[Grey]</c> is spelled the
    /// British way. Confirmed against 26.2.4.2's own output: <c>[Red]</c> draws <c>#ff0000</c>
    /// and <c>[Blue]</c> <c>#0000ff</c>.
    /// </para>
    /// <para>
    /// <strong><c>[COLOR n]</c> is deliberately not resolved</strong> and leaves this null.
    /// LibreOffice takes it from <c>GetUserDefColor(n-1)</c>, which Calc wires to
    /// <c>XColorList::CreateStdColorList()</c> (<c>sc/source/core/data/documen9.cxx</c>:246-259)
    /// — the palette loaded from <c>SvtPathOptions().GetPalettePath()</c>, i.e. the
    /// <c>share/palette/standard.soc</c> of whichever installation is running, 120 entries and
    /// user-configurable. Index 9 there is <c>#dddddd</c> "Light Gray 4", which is exactly what
    /// 26.2.4.2 paints for <c>[Color10]</c>. Reproducing that would mean vendoring one
    /// installation's UI palette into a document renderer and pinning to it, which is the same
    /// class of coupling as the bundled-font confound. Corpus reach of the gap is <b>18 cells in
    /// one document</b>; see <c>probes/numfmtcolour-r139/results.md</c>.
    /// </para>
    /// </remarks>
    public Graphics.Colour? Colour { get; }

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
        bool unreproduced = false;
        Graphics.Colour? colour = null;

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

                // "Reserve the width of the next character." LibreOffice does not measure the
                // character either: SvNumberformat::InsertBlanks
                // (svl/source/numbers/zformat.cxx:89-104) inserts one, two or three ordinary
                // spaces from a 96-entry table of coarse widths, so `_(` is one space and `_%`
                // would be three. One space stands in for all of them here because every `_x` in
                // the corpus is one of `(`, `)`, `-` and a blank — 2572 of them over the chart
                // parts and workbook styles of 947 documents, and all four are 1 in that table,
                // so the table has nothing to add and a wrong count would be invisible.
                // Dropping the blank entirely is what runs an accounting format's currency symbol
                // into its digits.
                case '_':
                    tokens.Add(FormatToken.Literal(" "));
                    i += 2;
                    continue;

                // "Repeat the next character to fill the column." How many copies is a function
                // of the column, which extraction has no width for — so the token records only
                // *where* the fill goes and which character fills it, and the renderer emits a
                // marker there for a caller that has a width. See FormatToken.Fill.
                case '*':
                    if (i + 1 < code.Length) tokens.Add(FormatToken.Fill(code[i + 1]));
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
                    else if (IsNumeralOrCalendarDirective(body))
                    {
                        unreproduced = true;
                    }
                    else if (StandardColour(body) is { } named)
                    {
                        // The first colour wins, as LibreOffice's scan does: it takes the colour
                        // at the head of the subformat and a second one is a syntax error there
                        // rather than an override.
                        colour ??= named;
                    }
                    // Anything else — [ENG], [COLOR n] — changes appearance rather than the
                    // text this extracts, or needs a palette this does not have.
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

            // The day-name keys, which are not letters of a date the way `d` and `m` are:
            // `AAA`/`AAAA` are Excel's (East Asian in origin, and every producer writes them
            // lower case), `NN`/`NNN`/`NNNN` are LibreOffice's own. They share one case in
            // `zformat.cxx`:3983-4008 and the difference is short name against long, plus the
            // locale's day-of-week separator on `NNNN` alone.
            if (DayNameRun(code, i, out int nameLength, out int nameCount, out bool otherCalendar))
            {
                tokens.Add(FormatToken.DateTime('w', nameCount));
                sawDateTime = true;
                if (otherCalendar) unreproduced = true;
                i += nameLength;
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
            hasDate, hasTime, twelveHour, hasElapsed, unreproduced, colour);
    }

    /// <summary>
    /// One of LibreOffice's ten colour keywords, or null for anything else.
    /// </summary>
    /// <remarks>
    /// The names and the values are both <c>svl/source/numbers/zforscan.cxx</c>'s, paired in its
    /// own order (:98-111 against :145-147). Matching is case-insensitive because
    /// <c>ImpSvNumberformatScan::GetColor</c> upper-cases the word first (:548), which is why a
    /// file may write <c>[Red]</c>, <c>[RED]</c> or <c>[red]</c> and all three are the same
    /// colour. <c>GREY</c> is the spelling LibreOffice keys on; <c>GRAY</c> is accepted beside
    /// it because Excel writes that and the two name one colour.
    /// </remarks>
    private static Graphics.Colour? StandardColour(string body) => body.ToUpperInvariant() switch
    {
        "BLACK" => Graphics.Colour.FromRgb(0x000000),
        "BLUE" => Graphics.Colour.FromRgb(0x0000FF),
        "GREEN" => Graphics.Colour.FromRgb(0x00FF00),
        "CYAN" => Graphics.Colour.FromRgb(0x00FFFF),
        "RED" => Graphics.Colour.FromRgb(0xFF0000),
        "MAGENTA" => Graphics.Colour.FromRgb(0xFF00FF),
        "BROWN" => Graphics.Colour.FromRgb(0x808000),
        "GREY" or "GRAY" => Graphics.Colour.FromRgb(0x808080),
        "YELLOW" => Graphics.Colour.FromRgb(0xFFFF00),
        "WHITE" => Graphics.Colour.FromRgb(0xFFFFFF),
        _ => null,
    };

    private static bool IsNumeralOrCalendarDirective(string body)
        => body.StartsWith("NatNum", StringComparison.OrdinalIgnoreCase)
           || body.StartsWith("DBNum", StringComparison.OrdinalIgnoreCase)
           || body.StartsWith('~');

    private static bool MatchesWord(string code, int index, string word)
        => index + word.Length <= code.Length
           && string.Compare(code, index, word, 0, word.Length, StringComparison.OrdinalIgnoreCase) == 0;

    /// <summary>
    /// Matches a day-name key at <paramref name="index"/> and says how it is written.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>The five keys do not pair up the way their lengths suggest, and the pairing was
    /// measured rather than counted.</strong> <c>zformat.cxx</c>:3983-4004 puts <c>NN</c> with
    /// <c>AAA</c> on <c>SHORT_DAY_NAME</c> and <c>NNN</c> with <c>AAAA</c> on
    /// <c>LONG_DAY_NAME</c>, so <c>nnn</c> is a <em>long</em> name and not a short one; only
    /// <c>NNNN</c> appends the locale's day-of-week separator (:4004,
    /// <c>getLongDateDayOfWeekSep</c>). Both binaries draw <c>Sun</c>, <c>Sunday</c> and
    /// <c>Sunday, </c> for the three. The count below is therefore 3 for a short name, 4 for a
    /// long one and 5 for a long one with the separator.
    /// </para>
    /// <para>
    /// Runs of one or two <c>a</c>, and a lone <c>n</c>, are not keywords and stay literal —
    /// <c>sKeyword</c> holds only <c>AAA</c>, <c>AAAA</c>, <c>NN</c>, <c>NNN</c> and
    /// <c>NNNN</c> (<c>svl/source/numbers/zforscan.cxx</c>:60-77).
    /// </para>
    /// <para>
    /// <strong>An <c>A</c> key drags a calendar in with it and an <c>N</c> key does not.</strong>
    /// <c>ImpIsOtherCalendar</c> (<c>zformat.cxx</c>:3453-3480) answers true for a subformat
    /// holding <c>AAA</c>, <c>AAAA</c>, <c>EC</c>, <c>EEC</c>, <c>R</c>, <c>RR</c>, <c>G</c>,
    /// <c>GG</c> or <c>GGG</c> — and for none of the <c>N</c> keys — after which
    /// <c>SwitchToOtherCalendar</c> (:3486-3512) renders the month and day fields in the
    /// <em>first non-Gregorian calendar the locale lists</em>, leaving the year Gregorian.
    /// Measured on both installed binaries
    /// (<c>dotnet/probes/numfmt-r68/make-codes.py</c>): under en-US that calendar is the Jewish
    /// one, so serial 44794 — 21 August 2022, a Sunday — draws <c>05/24/22 Sunday</c> under
    /// <c>mm/dd/yy aaaa</c> and <c>08/21/22 Sunday</c> under <c>mm/dd/yy nnn</c>. The day name
    /// itself is exact either way; the date beside an <c>A</c> key is not, which is why only
    /// those report <see cref="HasUnreproducedDirective"/>.
    /// </para>
    /// </remarks>
    private static bool DayNameRun(
        string code, int index, out int length, out int count, out bool otherCalendar)
    {
        length = 0;
        count = 0;
        otherCalendar = false;

        char first = char.ToLowerInvariant(code[index]);
        if (first is not ('a' or 'n')) return false;

        int run = 0;
        while (index + run < code.Length && char.ToLowerInvariant(code[index + run]) == first) run++;

        // Longest key first, left to right, and the remainder of an over-long run is whatever
        // it parses as next. Measured on both binaries: `aaaaa` draws `Sundaya` and `nnnnn`
        // draws `Sunday, n`, so the scanner takes AAAA/NNNN and leaves the tail — not the other
        // way round, which would draw `aSunday`.
        int shortest = first == 'a' ? 3 : 2;
        if (run < shortest) return false;

        length = Math.Min(run, 4);
        count = first == 'a'
            ? length                                        // AAA short, AAAA long
            : length switch { 2 => 3, 3 => 4, _ => 5 };     // NN short, NNN long, NNNN long+sep
        otherCalendar = first == 'a';
        return true;
    }

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
                // 'w' is a day name, which is a date part even though it draws no digits.
                case 'y' or 'd' or 'g' or 'e' or 'b' or 'w':
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
    Fill,
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

    /// <summary>
    /// A <c>*c</c> directive: repeat <paramref name="fill"/> until the column is full.
    /// </summary>
    /// <remarks>
    /// The token's text is the marker LibreOffice itself writes into the formatted string —
    /// <c>U+001B</c> followed by the fill character (<c>lcl_appendStarFillChar</c>,
    /// <c>svl/source/numbers/zformat.cxx:2200</c>) — so a fill renders exactly like a literal
    /// everywhere in the renderer, and the one caller that owns a column width finds it by its
    /// escape and expands it.
    /// </remarks>
    public static FormatToken Fill(char fill)
        => new(FormatTokenKind.Fill, string.Concat(NumberFormatter.FillMarker, fill), fill, 0, false);
}
