using XRay.Office.Core.Units;

namespace XRay.Office.Text.Layout;

/// <summary>
/// Writer's extra space between East Asian and Western text.
/// </summary>
/// <remarks>
/// <para>
/// A fifth of the font size, inserted where the script changes with East Asian text on one side of
/// the change: <c>SwTextFormatter::BuildPortions</c>, <c>sw/source/core/text/itrform2.cxx</c>:707-734,
/// whose own comment is <em>"The distance between two different scripts is set to 20% of the
/// fontheight"</em> and whose value is <c>rInf.GetFont()-&gt;GetHeight()/5</c> — the size the document
/// asks for, in twips, not the line height. Measured on <c>手机免提系统TSB.doc</c>: 2.400 pt at 12 pt,
/// repeated at every one of that document's 38 script changes.
/// </para>
/// <para>
/// It is not a general script-change rule, and the two exclusions are the whole of why it is safe.
/// <c>fnRequireKerningAtPosition</c> (<c>:486-521</c>) refuses the gap unless one side is
/// <c>ScriptType::ASIAN</c> — tdf#89288, so Latin beside Arabic or Hebrew gets nothing — and refuses
/// it again when either side is Hangul, because tdf#136663 established the space is a Chinese and
/// Japanese convention rather than a Korean one. The caller adds a third: both characters across the
/// boundary must be a letter or a digit, so no gap opens beside punctuation.
/// </para>
/// <para>
/// <strong>Both the measurement and the drawing must apply it, and from this one place.</strong> A gap
/// the filler charges for and the pen does not — or the reverse — is the same class of defect as a
/// paragraph measured in a face it is not drawn in: the line breaks on one number and is painted with
/// another.
/// </para>
/// </remarks>
public static class ScriptSpacing
{
    /// <summary>The gap a script change opens at a font size.</summary>
    /// <remarks>
    /// Truncated to whole twips rather than rounded, which is what integer division of a twip count
    /// by five gives in the C++.
    /// </remarks>
    public static Length GapFor(Length emSize)
        => emSize <= Length.Zero ? Length.Zero : Length.FromTwips(emSize.Twips / 5);

    /// <summary>
    /// The positions in a text where a gap opens, as indices of the character *after* the change.
    /// </summary>
    /// <param name="text">The paragraph's text.</param>
    public static List<int> Boundaries(ReadOnlySpan<char> text)
    {
        List<int> at = [];

        for (int i = 1; i < text.Length; i++)
        {
            if (Opens(text, i)) at.Add(i);
        }

        return at;
    }

    /// <summary>True when a script change at an index earns the gap.</summary>
    /// <param name="text">The paragraph's text.</param>
    /// <param name="index">The index of the character after the change; never zero.</param>
    public static bool Opens(ReadOnlySpan<char> text, int index)
    {
        if (index <= 0 || index >= text.Length) return false;

        char before = text[index - 1];
        char here = text[index];

        // Both ends must be a letter or a digit — `CharClass::isLetterNumeric` on each side of the
        // boundary, guarding the comment "we do not want a kerning portion if any end would be a
        // punctuation character". A full stop between two scripts therefore opens nothing.
        if (!char.IsLetterOrDigit(before) || !char.IsLetterOrDigit(here)) return false;

        ScriptClass past = ClassOf(before);
        ScriptClass now = ClassOf(here);
        if (past == now) return false;

        // tdf#89288: one side must be East Asian. tdf#136663: and neither may be Hangul.
        if (past != ScriptClass.Asian && now != ScriptClass.Asian) return false;

        return past != ScriptClass.Hangul && now != ScriptClass.Hangul;
    }

    /// <summary>The three classes this rule distinguishes.</summary>
    /// <remarks>
    /// Hangul is split out of <see cref="Asian"/> rather than folded into it, because the rule needs
    /// it on both counts at once: a Hangul syllable <em>is</em> <c>ScriptType::ASIAN</c> for the
    /// script-change test and is excluded by name from the gap.
    /// </remarks>
    private enum ScriptClass
    {
        /// <summary>Anything that is neither of the others, which is most text.</summary>
        Other,

        /// <summary>Han, kana, bopomofo and the CJK compatibility and fullwidth blocks.</summary>
        Asian,

        /// <summary>Hangul, in all three of its blocks.</summary>
        Hangul,
    }

    /// <summary>
    /// Which class a character belongs to.
    /// </summary>
    /// <remarks>
    /// The blocks <c>unicode::getUnicodeScriptType</c> maps to <c>ScriptType::ASIAN</c>, which is a
    /// list of ranges rather than a property lookup — and deliberately not the <c>Line_Break</c>
    /// class <see cref="LineBreakProperties"/> already holds, which answers a different question and
    /// puts Hangul, kana and Han in four classes that do not line up with this one.
    /// </remarks>
    private static ScriptClass ClassOf(char character)
        => character switch
        {
            >= 'ᄀ' and <= 'ᇿ' => ScriptClass.Hangul,   // Hangul Jamo
            >= '㄰' and <= '㆏' => ScriptClass.Hangul,   // Hangul Compatibility Jamo
            >= 'ꥠ' and <= '꥿' => ScriptClass.Hangul,   // Hangul Jamo Extended-A
            >= '가' and <= '퟿' => ScriptClass.Hangul,   // Hangul Syllables, Extended-B
            >= '⺀' and <= '⿟' => ScriptClass.Asian,    // CJK radicals, Kangxi
            >= '　' and <= '〿' => ScriptClass.Asian,    // CJK symbols and punctuation
            >= '぀' and <= 'ヿ' => ScriptClass.Asian,    // Hiragana, Katakana
            >= '㄀' and <= 'ㄯ' => ScriptClass.Asian,    // Bopomofo
            >= '㆐' and <= '鿿' => ScriptClass.Asian,    // Kanbun, Yi, CJK Unified
            >= '豈' and <= '﫿' => ScriptClass.Asian,    // CJK Compatibility Ideographs
            >= '︰' and <= '﹏' => ScriptClass.Asian,    // CJK Compatibility Forms
            >= '＀' and <= '｠' => ScriptClass.Asian,    // Fullwidth forms
            >= '￠' and <= '￦' => ScriptClass.Asian,    // Fullwidth signs
            _ => ScriptClass.Other,
        };
}
