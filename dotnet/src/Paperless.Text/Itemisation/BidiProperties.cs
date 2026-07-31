namespace Paperless.Text.Itemisation;

/// <summary>
/// The Unicode <c>Bidi_Class</c> of a character.
/// </summary>
/// <remarks>
/// Declared in ICU's <c>UCharDirection</c> order rather than in the order UAX #9 introduces them,
/// because the table generator writes ICU's own integer for each range and a differently ordered
/// enum would silently relabel every character. ICU's order is what LibreOffice reads too, so a
/// value here can be compared with one in a <c>u_charDirection</c> trace without translation.
/// </remarks>
public enum BidiClass
{
    /// <summary>Left-to-right; the default for anything the table omits.</summary>
    L = 0,

    /// <summary>Right-to-left — Hebrew and the other non-Arabic RTL scripts.</summary>
    R,

    /// <summary>European number.</summary>
    EN,

    /// <summary>European number separator.</summary>
    ES,

    /// <summary>European number terminator.</summary>
    ET,

    /// <summary>Arabic number.</summary>
    AN,

    /// <summary>Common number separator.</summary>
    CS,

    /// <summary>Paragraph separator.</summary>
    B,

    /// <summary>Segment separator — the tab.</summary>
    S,

    /// <summary>Whitespace.</summary>
    WS,

    /// <summary>Other neutral.</summary>
    ON,

    /// <summary>Left-to-right embedding.</summary>
    LRE,

    /// <summary>Left-to-right override.</summary>
    LRO,

    /// <summary>Right-to-left Arabic — the class that turns European digits into Arabic ones.</summary>
    AL,

    /// <summary>Right-to-left embedding.</summary>
    RLE,

    /// <summary>Right-to-left override.</summary>
    RLO,

    /// <summary>Pop directional format.</summary>
    PDF,

    /// <summary>Non-spacing mark.</summary>
    NSM,

    /// <summary>Boundary neutral.</summary>
    BN,

    /// <summary>First strong isolate.</summary>
    FSI,

    /// <summary>Left-to-right isolate.</summary>
    LRI,

    /// <summary>Right-to-left isolate.</summary>
    RLI,

    /// <summary>Pop directional isolate.</summary>
    PDI,
}

/// <summary>Whether a character opens a bracket pair, closes one, or is not a bracket.</summary>
public enum BracketType
{
    /// <summary>Not a paired bracket.</summary>
    None = 0,

    /// <summary>An opening bracket.</summary>
    Open,

    /// <summary>A closing bracket.</summary>
    Close,
}

/// <summary>
/// The Unicode properties the bidirectional algorithm reads.
/// </summary>
/// <remarks>
/// The tables in the generated half of this class come from ICU, which is what LibreOffice resolves
/// bidi with (<c>ubidi_setPara</c> in <c>vcl/source/text/ImplLayoutArgs.cxx</c>). Matching its answers
/// is the whole point, so taking the property data from the same place removes one way to disagree.
/// </remarks>
public static partial class BidiProperties
{
    /// <summary>
    /// The <c>Bidi_Class</c> of a code point.
    /// </summary>
    /// <remarks>
    /// <c>L</c> for anything the table omits, which is both the property's own default for most
    /// assigned characters and the right answer for an unassigned one outside the RTL blocks. The
    /// blocks that default to <c>R</c> or <c>AL</c> — Hebrew, Arabic, and the rest — are in the table
    /// as explicit ranges, because ICU applies those defaults and a reader that did not would put
    /// unassigned Arabic-block characters the wrong way round.
    /// </remarks>
    public static BidiClass ClassOf(int codePoint)
    {
        if (codePoint is < 0 or >= 0x110000) return BidiClass.L;

        int low = 0;
        int high = ClassStarts.Length - 1;
        while (low <= high)
        {
            int middle = (low + high) >> 1;
            if (codePoint < ClassStarts[middle]) high = middle - 1;
            else if (codePoint >= ClassEnds[middle]) low = middle + 1;
            else return Classes[middle];
        }

        return BidiClass.L;
    }

    /// <summary>Whether a code point opens a bracket pair, closes one, or is not a bracket.</summary>
    public static BracketType BracketTypeOf(int codePoint)
    {
        int at = Array.BinarySearch(BracketChars, codePoint);
        return at < 0 ? BracketType.None : BracketTypes[at];
    }

    /// <summary>
    /// The bracket a code point pairs with, or zero when it is not a bracket.
    /// </summary>
    /// <remarks>
    /// Canonicalised, which BD16 requires and which matters for exactly one pair in practice:
    /// U+2329 LEFT-POINTING ANGLE BRACKET is canonically equivalent to U+3008 LEFT ANGLE BRACKET, so
    /// a text that opens with one and closes with the other is still a pair. Without the fold, N0
    /// would not see it and the text inside would take the surrounding direction instead of the
    /// bracket's.
    /// </remarks>
    public static int PairedBracket(int codePoint)
    {
        int at = Array.BinarySearch(BracketChars, codePoint);
        return at < 0 ? 0 : Canonical(BracketPairs[at]);
    }

    /// <summary>
    /// Folds the one canonical singleton among the brackets onto its equivalent.
    /// </summary>
    /// <remarks>
    /// The angle brackets are the only bracket characters in Unicode with a canonical decomposition,
    /// so this is the whole of BD16's canonical-equivalence requirement rather than a shortcut around
    /// a normaliser.
    /// </remarks>
    public static int Canonical(int codePoint) => codePoint switch
    {
        0x2329 => 0x3008,
        0x232A => 0x3009,
        _ => codePoint,
    };
}
