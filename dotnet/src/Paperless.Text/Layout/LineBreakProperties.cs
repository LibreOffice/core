namespace Paperless.Text.Layout;

/// <summary>
/// A character's Unicode <c>Line_Break</c> class, the input to UAX #14's rules.
/// </summary>
/// <remarks>
/// The two-letter names are the ones the standard and every implementation use, including
/// LibreOffice's own rule file. Renaming them to something more descriptive would make the rules
/// unreadable against the specification they come from, which is the only way to check them.
/// </remarks>
public enum LineBreakClass : byte
{
    /// <summary>Unknown: unassigned, or a code point the tables do not cover.</summary>
    XX,

    /// <summary>Ambiguous — an East Asian width that depends on context.</summary>
    AI,

    /// <summary>Alphabetic: ordinary letters.</summary>
    AL,

    /// <summary>Break opportunity before and after, such as an em dash.</summary>
    B2,

    /// <summary>Break after.</summary>
    BA,

    /// <summary>Break before.</summary>
    BB,

    /// <summary>A mandatory break.</summary>
    BK,

    /// <summary>Contingent break — an embedded object decides.</summary>
    CB,

    /// <summary>A conditional Japanese starter, small kana and the like.</summary>
    CJ,

    /// <summary>Closing punctuation.</summary>
    CL,

    /// <summary>A combining mark, which attaches to the character before it.</summary>
    CM,

    /// <summary>A closing parenthesis.</summary>
    CP,

    /// <summary>Carriage return.</summary>
    CR,

    /// <summary>An emoji base.</summary>
    EB,

    /// <summary>An emoji modifier.</summary>
    EM,

    /// <summary>An exclamation or question mark.</summary>
    EX,

    /// <summary>Glue: no break on either side, such as a non-breaking space.</summary>
    GL,

    /// <summary>A Hangul syllable of LV form.</summary>
    H2,

    /// <summary>A Hangul syllable of LVT form.</summary>
    H3,

    /// <summary>A Hebrew letter.</summary>
    HL,

    /// <summary>A hyphen.</summary>
    HY,

    /// <summary>Ideographic.</summary>
    ID,

    /// <summary>Inseparable, such as a run of leader dots.</summary>
    IN,

    /// <summary>Infix numeric separator, such as a decimal comma.</summary>
    IS,

    /// <summary>A Hangul leading consonant.</summary>
    JL,

    /// <summary>A Hangul trailing consonant.</summary>
    JT,

    /// <summary>A Hangul vowel.</summary>
    JV,

    /// <summary>Line feed.</summary>
    LF,

    /// <summary>Next line.</summary>
    NL,

    /// <summary>A non-starter, which may not begin a line.</summary>
    NS,

    /// <summary>A digit.</summary>
    NU,

    /// <summary>Opening punctuation.</summary>
    OP,

    /// <summary>A postfix numeric sign, such as a per-cent sign.</summary>
    PO,

    /// <summary>A prefix numeric sign, such as a currency symbol.</summary>
    PR,

    /// <summary>A quotation mark.</summary>
    QU,

    /// <summary>A regional indicator, two of which make a flag.</summary>
    RI,

    /// <summary>Complex context: a script needing dictionary-based breaking.</summary>
    SA,

    /// <summary>An unpaired surrogate.</summary>
    SG,

    /// <summary>A space.</summary>
    SP,

    /// <summary>A break symbol, such as a solidus.</summary>
    SY,

    /// <summary>A word joiner.</summary>
    WJ,

    /// <summary>A zero-width space.</summary>
    ZW,

    /// <summary>A zero-width joiner.</summary>
    ZWJ,

    /// <summary>An Aksara, in a Brahmic script.</summary>
    AK,

    /// <summary>An Aksara prebase.</summary>
    AP,

    /// <summary>An Aksara start.</summary>
    AS,

    /// <summary>A virama final.</summary>
    VF,

    /// <summary>A virama.</summary>
    VI,
}

/// <summary>
/// The Unicode properties UAX #14 reads, looked up by code point.
/// </summary>
/// <remarks>
/// Three properties rather than one, because the rules need all three: the <c>Line_Break</c> class
/// drives almost everything, <c>East_Asian_Width</c> distinguishes the narrow brackets rule LB30
/// applies to from the wide ones it does not, and <c>Extended_Pictographic</c> is what LB30b's emoji
/// rule is about. The tables are generated — see <c>LineBreakProperties.Tables.cs</c>.
/// </remarks>
public static partial class LineBreakProperties
{
    /// <summary>How many ranges the class table holds.</summary>
    public static int RangeCount => ClassStarts.Length;

    /// <summary>
    /// The <c>Line_Break</c> class of a code point.
    /// </summary>
    /// <remarks>
    /// An unassigned or out-of-range code point is <see cref="LineBreakClass.XX"/>, which LB1
    /// resolves to <see cref="LineBreakClass.AL"/> — so an unknown character behaves like a letter
    /// rather than stopping the walk. That is what the standard prescribes, and it is also what keeps
    /// a document using a code point newer than these tables laying out sensibly.
    /// </remarks>
    public static LineBreakClass ClassOf(int codePoint)
    {
        if (codePoint is < 0 or > 0x10FFFF) return LineBreakClass.XX;

        int low = 0;
        int high = ClassStarts.Length - 1;
        while (low <= high)
        {
            int middle = low + ((high - low) / 2);
            if (codePoint < ClassStarts[middle]) high = middle - 1;
            else if (codePoint >= ClassEnds[middle]) low = middle + 1;
            else return Classes[middle];
        }
        return LineBreakClass.XX;
    }

    /// <summary>True when the code point is <c>Extended_Pictographic</c>.</summary>
    public static bool IsExtendedPictographic(int codePoint)
        => InPairs(PictographicRanges, codePoint);

    /// <summary>
    /// True when the code point's East Asian width is full, wide or half.
    /// </summary>
    /// <remarks>
    /// LB30 forbids a break between a letter and a following opening bracket, but only for a
    /// <em>narrow</em> bracket: a wide one is a CJK bracket, and CJK text breaks between characters,
    /// so applying the rule to it would run a whole clause off the line.
    /// </remarks>
    public static bool IsWide(int codePoint) => InPairs(WideRanges, codePoint);

    private static bool InPairs(int[] pairs, int codePoint)
    {
        int low = 0;
        int high = (pairs.Length / 2) - 1;
        while (low <= high)
        {
            int middle = low + ((high - low) / 2);
            if (codePoint < pairs[middle * 2]) high = middle - 1;
            else if (codePoint >= pairs[(middle * 2) + 1]) low = middle + 1;
            else return true;
        }
        return false;
    }
}
