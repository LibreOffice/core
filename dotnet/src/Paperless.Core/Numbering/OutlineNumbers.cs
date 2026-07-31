using System.Globalization;
using System.Text;

namespace Paperless.Core.Numbering;

/// <summary>
/// Renders list and outline counter values, and normalises bullet characters.
/// </summary>
/// <remarks>
/// <para>
/// Every format Paperless reads generates list labels the same way — a counter per level, run
/// through a per-level format — and they agree on which formats exist: decimal, upper and lower
/// roman, upper and lower alphabetic. ODF spells them <c>1 i I a A</c> and
/// WordprocessingML spells them <c>decimal lowerRoman upperRoman lowerLetter upperLetter</c>,
/// but the arithmetic is identical, so it lives here rather than being written once per format.
/// </para>
/// <para>
/// This is in <c>Paperless.Core</c> deliberately: it is shared by the ODF, OOXML and legacy
/// binary readers, none of which may depend on each other.
/// </para>
/// </remarks>
public static class OutlineNumbers
{
    /// <summary>
    /// Formats a counter value as plain decimal digits.
    /// </summary>
    /// <remarks>
    /// Named <c>Digits</c> rather than <c>Decimal</c> so it does not read as a conversion to
    /// <see cref="decimal"/>, which is what a method called <c>Decimal</c> would suggest.
    /// </remarks>
    public static string Digits(int value) => value.ToString(CultureInfo.InvariantCulture);

    /// <summary>
    /// Formats a counter value as decimal with a leading zero below ten, which is
    /// WordprocessingML's <c>decimalZero</c>.
    /// </summary>
    public static string DigitsWithLeadingZero(int value)
        => value is >= 0 and < 10
            ? "0" + value.ToString(CultureInfo.InvariantCulture)
            : Digits(value);

    /// <summary>
    /// Formats a counter value as a roman numeral.
    /// </summary>
    /// <param name="value">The value. Zero, negatives and values above 3999 come back as digits.</param>
    /// <param name="upperCase">True for <c>IV</c>, false for <c>iv</c>.</param>
    /// <remarks>
    /// Roman numerals have no representation for zero or for negatives, and no conventional one
    /// above 3999. LibreOffice falls back to the bare number in those cases, so matching it
    /// keeps a document with a peculiar start value rendering the same in both.
    /// </remarks>
    public static string Roman(int value, bool upperCase)
    {
        if (value is < 1 or > 3999) return Digits(value);

        ReadOnlySpan<int> magnitudes = [1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1];
        string[] numerals = ["M", "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX", "V", "IV", "I"];

        StringBuilder result = new();
        int remaining = value;
        for (int i = 0; i < magnitudes.Length; i++)
        {
            while (remaining >= magnitudes[i])
            {
                result.Append(numerals[i]);
                remaining -= magnitudes[i];
            }
        }
        return upperCase ? result.ToString() : result.ToString().ToLowerInvariant();
    }

    /// <summary>
    /// Formats a counter value as letters.
    /// </summary>
    /// <param name="value">The value; below one it comes back as digits.</param>
    /// <param name="upperCase">True for <c>A</c>, false for <c>a</c>.</param>
    /// <param name="synchronised">
    /// True to repeat the letter past twenty-six (<c>aa</c>, <c>bb</c>) rather than counting in
    /// bijective base 26 (<c>aa</c>, <c>ab</c>). ODF selects between the two with
    /// <c>style:num-letter-sync</c>; Word's <c>upperLetter</c> and <c>lowerLetter</c> are the
    /// repeating kind.
    /// </param>
    public static string Alphabetic(int value, bool upperCase, bool synchronised = false)
    {
        if (value < 1) return Digits(value);
        char first = upperCase ? 'A' : 'a';

        if (synchronised)
        {
            int repeats = ((value - 1) / 26) + 1;
            char letter = (char)(first + ((value - 1) % 26));
            return new string(letter, repeats);
        }

        StringBuilder result = new();
        int remaining = value;
        while (remaining > 0)
        {
            int digit = (remaining - 1) % 26;
            result.Insert(0, (char)(first + digit));
            remaining = (remaining - 1) / 26;
        }
        return result.ToString();
    }

    /// <summary>
    /// Formats a counter value as an English ordinal: <c>1st</c>, <c>2nd</c>, <c>11th</c>.
    /// </summary>
    /// <remarks>
    /// English only, which is what WordprocessingML's <c>ordinal</c> means — the format is
    /// defined against the document's language, and every other language would need its own
    /// rules. A document using it in another language is mis-numbered by Word too.
    /// </remarks>
    public static string Ordinal(int value)
    {
        if (value < 1) return Digits(value);

        // 11th, 12th and 13th are the exceptions the naive last-digit rule gets wrong.
        string suffix = (value % 100) is >= 11 and <= 13
            ? "th"
            : (value % 10) switch { 1 => "st", 2 => "nd", 3 => "rd", _ => "th" };
        return Digits(value) + suffix;
    }

    /// <summary>
    /// Replaces a Private Use Area bullet character with U+2022 BULLET.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Both families do this. Impress writes its default bullet as a code point from the
    /// <c>starbats</c> symbol font, and Word writes its from <c>Symbol</c> or
    /// <c>Wingdings</c> — in each case a Private Use Area code point, which means nothing at
    /// all outside the font that defines it.
    /// </para>
    /// <para>
    /// Passing it through produces text no consumer can interpret. LibreOffice's own HTML
    /// export substitutes a bullet in exactly this case, so this agrees with the reference
    /// rather than departing from it.
    /// </para>
    /// </remarks>
    public static string NormaliseBullet(string bullet)
    {
        ArgumentNullException.ThrowIfNull(bullet);
        if (bullet.Length != 1) return bullet;

        // The Basic Multilingual Plane's Private Use Area. The two supplementary areas are not
        // used by these fonts.
        return bullet[0] is >= '\uE000' and <= '\uF8FF' ? "\u2022" : bullet;
    }
}
