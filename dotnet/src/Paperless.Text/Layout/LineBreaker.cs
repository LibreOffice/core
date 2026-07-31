using System.Globalization;

namespace Paperless.Text.Layout;

/// <summary>
/// Finds where a line may be broken, per Unicode UAX #14 with LibreOffice's own two changes to it.
/// </summary>
/// <remarks>
/// <para>
/// Hand-rolled because there is nothing to call. The BCL exposes grapheme clusters, normalisation and
/// collation but no line-break iterator and not even the <c>Line_Break</c> property — and although
/// .NET's globalisation is ICU-backed on Linux, ICU's <c>BreakIterator</c> is not surfaced, so being
/// "on ICU" buys nothing here (<c>Paperless.Text/TODO.md</c>).
/// </para>
/// <para>
/// Written as the rules in order rather than as a pair table. A pair table is faster and completely
/// opaque: when a break comes out wrong the only way to find out why is to reverse-engineer which
/// cell decided it. Rules in order can be read against the specification, which is the only way to
/// check them — and a paragraph is a few hundred characters, so the speed is irrelevant.
/// </para>
/// <para>
/// Three deliberate differences from current UAX #14, all copied from LibreOffice's own rule file
/// (<c>i18npool/source/breakiterator/data/line.txt</c>), because matching LibreOffice's line breaks
/// is the point:
/// </para>
/// <list type="bullet">
/// <item>
/// <description>
/// <strong>LB15 is disabled</strong> (LibreOffice issue i#83649): a break <em>is</em> allowed between
/// a quotation mark and a following opening bracket.
/// </description>
/// </item>
/// <item>
/// <description>
/// <strong>A number range breaks after its hyphen</strong> (i#83229): <c>100-199</c> may break after
/// the hyphen, where plain LB25 keeps the whole range together.
/// </description>
/// </item>
/// <item>
/// <description>
/// <strong>LB21a is the Unicode 15.0 form</strong>, without the <c>[^HL]</c> that 15.1 added: a break
/// after a Hebrew letter's hyphen is forbidden whatever follows, not only before a non-Hebrew letter.
/// This is the rule LibreOffice's ICU implements, and following the newer text would split Hebrew
/// compounds LibreOffice keeps whole.
/// </description>
/// </item>
/// </list>
/// <para>
/// Also strict rather than normal breaking: LibreOffice folds CJ into NS, which is CSS
/// <c>line-break: strict</c>. Under normal breaking CJ would behave like ID and small kana could
/// start a line.
/// </para>
/// </remarks>
public sealed class LineBreaker : ILineBreaker
{
    /// <summary>A shared instance; the breaker holds no state between calls.</summary>
    public static LineBreaker Instance { get; } = new();

    /// <inheritdoc/>
    public IReadOnlyList<int> FindBreakOpportunities(
        ReadOnlySpan<char> text, string? language = null)
    {
        List<int> breaks = [];
        if (text.Length == 0) return breaks;

        Analysis analysis = Analyse(text);
        bool[] insideNumber = MarkNumbers(analysis);

        // LB2: never break at the start of text. The loop therefore considers only the boundaries
        // between two characters, and adds the end of the text afterwards for LB3.
        for (int i = 1; i < analysis.Count; i++)
        {
            if (Allows(analysis, insideNumber, i)) breaks.Add(analysis.Offsets[i]);
        }

        // LB3: always break at the end of text.
        breaks.Add(text.Length);
        return breaks;
    }

    /// <inheritdoc/>
    public IReadOnlyList<int> FindMandatoryBreaks(ReadOnlySpan<char> text, string? language = null)
    {
        List<int> breaks = [];
        if (text.Length == 0) return breaks;

        Analysis analysis = Analyse(text);

        // LB4 and LB5: break after BK, and after CR, LF or NL — with CR LF counting as one break, which
        // is why the boundary after a CR followed by an LF is skipped.
        for (int i = 0; i < analysis.Count; i++)
        {
            LineBreakClass here = analysis.Resolved[i];
            if (here is not (LineBreakClass.BK or LineBreakClass.CR
                             or LineBreakClass.LF or LineBreakClass.NL))
            {
                continue;
            }

            if (here == LineBreakClass.CR
                && i + 1 < analysis.Count
                && analysis.Resolved[i + 1] == LineBreakClass.LF)
            {
                continue;
            }

            int after = i + 1 < analysis.Count ? analysis.Offsets[i + 1] : text.Length;
            if (after < text.Length) breaks.Add(after);
        }

        return breaks;
    }

    /// <summary>
    /// The classes of a text's characters, with the resolving rules already applied.
    /// </summary>
    /// <remarks>
    /// One entry per <em>code point</em>, not per UTF-16 unit: a surrogate pair is one character to
    /// every rule here, and treating its halves separately would put a break inside it. The offsets
    /// array maps back to UTF-16 indices, which is what a caller slicing the string needs.
    /// </remarks>
    private static Analysis Analyse(ReadOnlySpan<char> text)
    {
        List<int> offsets = [];
        List<LineBreakClass> original = [];
        List<int> codePoints = [];

        for (int i = 0; i < text.Length;)
        {
            int codePoint = char.ConvertToUtf32(text[i..].ToString(), 0);
            int width = char.IsSurrogatePair(text[i], i + 1 < text.Length ? text[i + 1] : '\0')
                ? 2
                : 1;

            offsets.Add(i);
            codePoints.Add(codePoint);
            original.Add(LineBreakProperties.ClassOf(codePoint));
            i += width;
        }

        LineBreakClass[] resolved = new LineBreakClass[original.Count];
        for (int i = 0; i < original.Count; i++)
        {
            resolved[i] = Resolve(original[i], codePoints[i]);
        }

        // LB9: a combining mark takes the class of the character it attaches to, so CM and ZWJ are
        // treated as part of the base. LB10 then makes a mark with no base behave like a letter.
        for (int i = 0; i < resolved.Length; i++)
        {
            if (resolved[i] is not (LineBreakClass.CM or LineBreakClass.ZWJ)) continue;

            LineBreakClass? baseClass = null;
            for (int back = i - 1; back >= 0; back--)
            {
                if (resolved[back] is LineBreakClass.CM or LineBreakClass.ZWJ) continue;

                // A mark cannot attach to any of these: they end the run it would have joined.
                baseClass = resolved[back] is LineBreakClass.BK or LineBreakClass.CR
                                           or LineBreakClass.LF or LineBreakClass.NL
                                           or LineBreakClass.SP or LineBreakClass.ZW
                    ? null
                    : resolved[back];
                break;
            }

            resolved[i] = baseClass ?? LineBreakClass.AL;
        }

        return new Analysis(offsets, resolved, original, codePoints);
    }

    /// <summary>
    /// LB1: gives the classes that carry no break behaviour of their own a concrete one.
    /// </summary>
    /// <remarks>
    /// <c>SA</c> is the interesting case. It marks scripts that need a dictionary to break — Thai,
    /// Lao, Khmer — and Paperless has none, so its letters become AL and its combining marks CM. The
    /// consequence is honest and worth stating: Thai text gets no intra-word breaks, exactly as any
    /// implementation without a Thai dictionary produces, rather than breaking in the wrong places.
    /// </remarks>
    private static LineBreakClass Resolve(LineBreakClass original, int codePoint) => original switch
    {
        LineBreakClass.AI or LineBreakClass.SG or LineBreakClass.XX => LineBreakClass.AL,

        LineBreakClass.SA => CharUnicodeInfo.GetUnicodeCategory(codePoint)
                is UnicodeCategory.NonSpacingMark or UnicodeCategory.SpacingCombiningMark
            ? LineBreakClass.CM
            : LineBreakClass.AL,

        // Strict breaking, as LibreOffice sets it: a conditional Japanese starter may not begin a
        // line. Under normal breaking this would be ID instead.
        LineBreakClass.CJ => LineBreakClass.NS,

        _ => original,
    };

    /// <summary>
    /// Whether a break is allowed before the character at an index.
    /// </summary>
    /// <remarks>
    /// The rules are tried in the order the standard numbers them, and the first that applies decides
    /// — which is what makes the order load-bearing rather than incidental. LB1 to LB3 and LB9 to LB10
    /// were handled while analysing; the rest are here.
    /// </remarks>
    private static bool Allows(Analysis analysis, bool[] insideNumber, int index)
    {
        LineBreakClass before = analysis.Resolved[index - 1];
        LineBreakClass after = analysis.Resolved[index];

        // LB4: always break after a hard line break.
        if (before == LineBreakClass.BK) return true;

        // LB5: treat CR LF as one break, and break after any of CR, LF and NL.
        if (before == LineBreakClass.CR && after == LineBreakClass.LF) return false;
        if (before is LineBreakClass.CR or LineBreakClass.LF or LineBreakClass.NL) return true;

        // LB6: do not break before a hard line break.
        if (after is LineBreakClass.BK or LineBreakClass.CR or LineBreakClass.LF or LineBreakClass.NL)
            return false;

        // LB7: do not break before a space or a zero-width space.
        if (after is LineBreakClass.SP or LineBreakClass.ZW) return false;

        // LB8: break after a zero-width space, even across the spaces that follow it.
        int beforeSpaces = SkipSpacesBack(analysis, index);
        if (beforeSpaces >= 0 && analysis.Resolved[beforeSpaces] == LineBreakClass.ZW) return true;

        // LB8a: do not break after a zero-width joiner.
        if (analysis.Original[index - 1] == LineBreakClass.ZWJ) return false;

        // LB9: a combining mark or a zero-width joiner attaches to the character before it, so there
        // is no boundary between them. Analysing gave the mark its base's class, which is the half of
        // the rule the later rules need; this is the other half, and without it the boundary is still
        // offered — which is what splits an emoji from the joiner holding its sequence together.
        // A mark after a space has no base, and LB10 has already made it behave like a letter.
        if (analysis.Original[index] is LineBreakClass.CM or LineBreakClass.ZWJ
            && before != LineBreakClass.SP)
            return false;

        // LB11: do not break before or after a word joiner.
        if (after == LineBreakClass.WJ || before == LineBreakClass.WJ) return false;

        // LB12: do not break after glue.
        if (before == LineBreakClass.GL) return false;

        // LB12a: do not break before glue, unless what precedes it already allows a break.
        if (after == LineBreakClass.GL
            && before is not (LineBreakClass.SP or LineBreakClass.BA or LineBreakClass.HY))
            return false;

        // LB13: do not break before closing punctuation, a closing parenthesis, an exclamation, an
        // infix separator or a break symbol.
        if (after is LineBreakClass.CL or LineBreakClass.CP or LineBreakClass.EX
                  or LineBreakClass.IS or LineBreakClass.SY)
            return false;

        // LB14: do not break after opening punctuation, however many spaces follow it.
        if (beforeSpaces >= 0 && analysis.Resolved[beforeSpaces] == LineBreakClass.OP) return false;

        // LB15 is deliberately absent: LibreOffice disables it, so a quotation mark followed by an
        // opening bracket may break between them (i#83649).

        // LB16: do not break between a closing bracket and a non-starter.
        if (after == LineBreakClass.NS && beforeSpaces >= 0
            && analysis.Resolved[beforeSpaces] is LineBreakClass.CL or LineBreakClass.CP)
            return false;

        // LB17: do not break inside a break-both pair, such as two em dashes.
        if (after == LineBreakClass.B2 && beforeSpaces >= 0
            && analysis.Resolved[beforeSpaces] == LineBreakClass.B2)
            return false;

        // LB18: break after a space. Every rule that needed to see across spaces has now run.
        if (before == LineBreakClass.SP) return true;

        // LB19: do not break before or after a quotation mark.
        if (after == LineBreakClass.QU || before == LineBreakClass.QU) return false;

        // LB20: break before and after a contingent break.
        if (before == LineBreakClass.CB || after == LineBreakClass.CB) return true;

        // LB20a: do not break after a hyphen that itself follows a break opportunity. "-ated" at the
        // start of a line is one token, because there is nowhere to put the hyphen otherwise.
        //
        // U+2010 HYPHEN is named alongside the hyphen class because its own class is BA, so the class
        // alone would not catch it. ICU added this as a Finnish tailoring and Unicode 15.1 promoted it
        // to the default, which is why LibreOffice's rule file carries it as a numbered rule.
        if (after == LineBreakClass.AL
            && (before == LineBreakClass.HY || analysis.CodePoints[index - 1] == 0x2010)
            && (index == 1
                || analysis.Resolved[index - 2] is LineBreakClass.BK or LineBreakClass.CR
                                                or LineBreakClass.LF or LineBreakClass.NL
                                                or LineBreakClass.SP or LineBreakClass.ZW
                                                or LineBreakClass.CB or LineBreakClass.GL))
            return false;

        // LB21: do not break before a hyphen, a break-after, a non-starter or an inseparable, and do
        // not break after a break-before.
        if (after is LineBreakClass.BA or LineBreakClass.HY or LineBreakClass.NS or LineBreakClass.IN)
            return false;
        if (before == LineBreakClass.BB) return false;

        // LB21a, in the Unicode 15.0 form LibreOffice's ICU implements: no break after a Hebrew
        // letter's hyphen, whatever follows it. Unicode 15.1 narrowed this to "before a non-Hebrew
        // letter"; following the newer text would split Hebrew compounds LibreOffice keeps whole.
        if (index >= 2
            && analysis.Resolved[index - 2] == LineBreakClass.HL
            && before is LineBreakClass.HY or LineBreakClass.BA)
            return false;

        // LB21b: do not break between a break symbol and a Hebrew letter.
        if (before == LineBreakClass.SY && after == LineBreakClass.HL) return false;

        // LB22: do not break before an inseparable.
        if (after == LineBreakClass.IN) return false;

        // LB23: do not break between a letter and a digit either way.
        if ((before is LineBreakClass.AL or LineBreakClass.HL) && after == LineBreakClass.NU)
            return false;
        if (before == LineBreakClass.NU && after is LineBreakClass.AL or LineBreakClass.HL)
            return false;

        // LB23a: do not break between a numeric prefix and an ideograph or emoji, or between an
        // ideograph or emoji and a numeric postfix.
        if (before == LineBreakClass.PR
            && after is LineBreakClass.ID or LineBreakClass.EB or LineBreakClass.EM)
            return false;
        if (before is LineBreakClass.ID or LineBreakClass.EB or LineBreakClass.EM
            && after == LineBreakClass.PO)
            return false;

        // LB24: do not break between numeric prefixes and suffixes and letters.
        if (before is LineBreakClass.PR or LineBreakClass.PO
            && after is LineBreakClass.AL or LineBreakClass.HL)
            return false;
        if (before is LineBreakClass.AL or LineBreakClass.HL
            && after is LineBreakClass.PR or LineBreakClass.PO)
            return false;

        // LB25: do not break inside a number. LibreOffice's own addition comes first: a hyphen
        // between two numbers *is* a break opportunity, so "100-199" may break after the hyphen
        // (i#83229).
        if (before == LineBreakClass.HY && after == LineBreakClass.NU
            && index >= 2 && analysis.Resolved[index - 2] == LineBreakClass.NU)
            return true;
        if (insideNumber[index]) return false;

        // LB26: do not break inside a Hangul syllable.
        if (before == LineBreakClass.JL
            && after is LineBreakClass.JL or LineBreakClass.JV or LineBreakClass.H2 or LineBreakClass.H3)
            return false;
        if (before is LineBreakClass.JV or LineBreakClass.H2
            && after is LineBreakClass.JV or LineBreakClass.JT)
            return false;
        if (before is LineBreakClass.JT or LineBreakClass.H3 && after == LineBreakClass.JT)
            return false;

        // LB27: treat a Hangul syllable like an ideograph beside numeric affixes.
        if (before is LineBreakClass.JL or LineBreakClass.JV or LineBreakClass.JT
                    or LineBreakClass.H2 or LineBreakClass.H3
            && after == LineBreakClass.PO)
            return false;
        if (before == LineBreakClass.PR
            && after is LineBreakClass.JL or LineBreakClass.JV or LineBreakClass.JT
                      or LineBreakClass.H2 or LineBreakClass.H3)
            return false;

        // LB28: do not break between letters.
        if (before is LineBreakClass.AL or LineBreakClass.HL
            && after is LineBreakClass.AL or LineBreakClass.HL)
            return false;

        // LB28a: do not break inside an orthographic syllable of a Brahmic script.
        if (IsInsideAksara(analysis, index)) return false;

        // LB29: do not break between an infix separator and a letter.
        if (before == LineBreakClass.IS && after is LineBreakClass.AL or LineBreakClass.HL)
            return false;

        // LB30: do not break between a letter or digit and a *narrow* opening bracket, or between a
        // narrow closing parenthesis and a letter or digit. A wide bracket is a CJK bracket, and CJK
        // text breaks between characters — applying the rule to it runs a clause off the line.
        if (before is LineBreakClass.AL or LineBreakClass.HL or LineBreakClass.NU
            && after == LineBreakClass.OP
            && !LineBreakProperties.IsWide(analysis.CodePoints[index]))
            return false;
        if (before == LineBreakClass.CP
            && !LineBreakProperties.IsWide(analysis.CodePoints[index - 1])
            && after is LineBreakClass.AL or LineBreakClass.HL or LineBreakClass.NU)
            return false;

        // LB30a: break between regional indicators only after an even number of them, so a flag
        // stays whole and two flags may be separated.
        if (before == LineBreakClass.RI && after == LineBreakClass.RI)
            return CountRegionalIndicatorsBack(analysis, index) % 2 == 0;

        // LB30b: do not break between an emoji base and its modifier.
        if (before == LineBreakClass.EB && after == LineBreakClass.EM) return false;
        if (after == LineBreakClass.EM
            && analysis.Original[index - 1] == LineBreakClass.XX
            && LineBreakProperties.IsExtendedPictographic(analysis.CodePoints[index - 1]))
            return false;

        // LB31: break everywhere else.
        return true;
    }

    /// <summary>
    /// The index of the last character before a position that is not a space, or -1.
    /// </summary>
    /// <remarks>
    /// Several rules are written "X SP* Y", meaning they look through any spaces between the two —
    /// which is why they all have to run before LB18 breaks after a space. Getting that order wrong
    /// puts a break after the space inside "( word", where the opening bracket should have held it.
    /// </remarks>
    private static int SkipSpacesBack(Analysis analysis, int index)
    {
        int at = index - 1;
        while (at >= 0 && analysis.Resolved[at] == LineBreakClass.SP) at--;
        return at;
    }

    /// <summary>
    /// Marks every boundary that falls inside a number, per LB25.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A number is a small grammar rather than a character class — an optional sign, an optional
    /// bracket or hyphen, an optional separator, digits with separators among them, then an optional
    /// closing bracket and an optional trailing sign. A pair of adjacent classes cannot decide whether
    /// a boundary is inside one: whether a comma sits inside a number or ends a clause depends on what
    /// follows it, and whether a currency sign begins a number depends on what follows that.
    /// </para>
    /// <para>
    /// So the numbers are found by one forward scan that matches the grammar greedily and marks the
    /// boundaries strictly inside each match. Scanning forwards rather than asking at each boundary
    /// also means each number is recognised once instead of re-parsed from both sides of every
    /// boundary in it.
    /// </para>
    /// </remarks>
    private static bool[] MarkNumbers(Analysis analysis)
    {
        bool[] inside = new bool[analysis.Count];

        for (int at = 0; at < analysis.Count;)
        {
            int end = MatchNumber(analysis, at);
            if (end <= at + 1)
            {
                at++;
                continue;
            }

            for (int boundary = at + 1; boundary < end; boundary++) inside[boundary] = true;
            at = end;
        }

        return inside;
    }

    /// <summary>
    /// One past the end of the number starting at an index, or the index itself when none does.
    /// </summary>
    private static int MatchNumber(Analysis analysis, int start)
    {
        int at = start;
        LineBreakClass ClassAt(int index)
            => index < analysis.Count ? analysis.Resolved[index] : LineBreakClass.XX;

        // An optional leading sign, then an optional bracket or hyphen, then an optional separator.
        if (ClassAt(at) is LineBreakClass.PR or LineBreakClass.PO) at++;
        if (ClassAt(at) is LineBreakClass.OP or LineBreakClass.HY) at++;
        if (ClassAt(at) == LineBreakClass.IS) at++;

        // The digits are what make it a number: without one, whatever was consumed was not a prefix.
        if (ClassAt(at) != LineBreakClass.NU) return start;
        at++;

        while (ClassAt(at) is LineBreakClass.NU or LineBreakClass.SY or LineBreakClass.IS) at++;

        // A trailing separator or symbol belongs to the number only if a digit follows it, so any
        // that end the run are given back — "1." ends a sentence rather than continuing a number.
        while (at > start && ClassAt(at - 1) is LineBreakClass.SY or LineBreakClass.IS) at--;

        if (ClassAt(at) is LineBreakClass.CL or LineBreakClass.CP) at++;
        if (ClassAt(at) is LineBreakClass.PR or LineBreakClass.PO) at++;

        return at;
    }

    /// <summary>
    /// True when a break before an index would fall inside a Brahmic orthographic syllable, per
    /// LB28a.
    /// </summary>
    private static bool IsInsideAksara(Analysis analysis, int index)
    {
        LineBreakClass before = analysis.Resolved[index - 1];
        LineBreakClass after = analysis.Resolved[index];

        // AP × (AK | ◌ | AS)
        if (before == LineBreakClass.AP
            && after is LineBreakClass.AK or LineBreakClass.AS or LineBreakClass.AL)
            return true;

        // (AK | ◌ | AS) × (VF | VI)
        if (before is LineBreakClass.AK or LineBreakClass.AS or LineBreakClass.AL
            && after is LineBreakClass.VF or LineBreakClass.VI)
            return true;

        // (AK | ◌ | AS) VI × (AK | ◌)
        if (index >= 2
            && analysis.Resolved[index - 2] is LineBreakClass.AK or LineBreakClass.AS or LineBreakClass.AL
            && before == LineBreakClass.VI
            && after is LineBreakClass.AK or LineBreakClass.AL)
            return true;

        // (AK | ◌ | AS) × (AK | ◌ | AS) VF
        if (before is LineBreakClass.AK or LineBreakClass.AS or LineBreakClass.AL
            && after is LineBreakClass.AK or LineBreakClass.AS or LineBreakClass.AL
            && index + 1 < analysis.Count
            && analysis.Resolved[index + 1] == LineBreakClass.VF)
            return true;

        return false;
    }

    /// <summary>How many regional indicators immediately precede an index.</summary>
    private static int CountRegionalIndicatorsBack(Analysis analysis, int index)
    {
        int count = 0;
        for (int at = index - 1; at >= 0 && analysis.Resolved[at] == LineBreakClass.RI; at--) count++;
        return count;
    }

    /// <summary>
    /// One text's characters as the rules see them.
    /// </summary>
    /// <param name="Offsets">Each character's UTF-16 index, so a break maps back to the string.</param>
    /// <param name="Resolved">The class after LB1, LB9 and LB10 have been applied.</param>
    /// <param name="Original">
    /// The class before resolution. Two rules need it: LB8a asks whether a character really was a
    /// zero-width joiner, and LB30b whether one really was unassigned — both of which LB9 and LB1
    /// would otherwise have hidden.
    /// </param>
    /// <param name="CodePoints">The code points, for the rules that ask about a character directly.</param>
    private readonly record struct Analysis(
        List<int> Offsets,
        LineBreakClass[] Resolved,
        List<LineBreakClass> Original,
        List<int> CodePoints)
    {
        /// <summary>How many characters the text holds, counting a surrogate pair as one.</summary>
        public int Count => Resolved.Length;
    }
}
