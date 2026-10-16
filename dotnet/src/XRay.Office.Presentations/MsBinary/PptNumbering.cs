using System.Globalization;
using System.Xml.Linq;
using XRay.Office.Ooxml.DrawingML;

namespace XRay.Office.Presentations.MsBinary;

/// <summary>
/// The number an automatically numbered binary PowerPoint paragraph draws.
/// </summary>
/// <remarks>
/// <para>
/// <strong>The arithmetic is not restated here, it is delegated.</strong> A number's value is a
/// property of the run of paragraphs it sits in — where the run starts, what breaks it, and how a
/// nested level restarts when its parent advances — and that is the same question in a
/// <c>.pptx</c>, a <c>.ppt</c> and a diagram. <c>DrawingTextBody.AutoNumber</c> already answers it
/// and says in its own remarks that it is public so a second walk cannot drift from it; this maps
/// PowerPoint's scheme <em>number</em> onto the <c>ST_TextAutonumberScheme</c> <em>name</em> that
/// function takes, and hands the counters straight through.
/// </para>
/// <para>
/// The mapping is <c>PPTNumberFormatCreator::ImplGetExtNumberFormat</c>'s switch,
/// <c>filter/source/msfilter/svdfppt.cxx:3466-3630</c>, read as the alphabet-plus-punctuation pair
/// it is: scheme 8 is a lower-case letter in parentheses, which DrawingML spells
/// <c>alphaLcParenBoth</c>, and scheme 7 is an upper-case roman numeral with a full stop, which it
/// spells <c>romanUcPeriod</c>. The schemes LibreOffice leaves to a CJK, Hebrew or full-width
/// numbering type are not mapped: DrawingML has no name for them either, and inventing one would
/// draw a Latin number where the reference draws a Chinese one — they fall to
/// <see cref="Unmapped"/>, which draws no marker at all rather than the wrong one.
/// </para>
/// </remarks>
internal static class PptNumbering
{
    /// <summary>How many outline levels a binary PowerPoint body has.</summary>
    /// <remarks>
    /// Five in the file — <c>nMaxPPTLevels</c> — and the arrays are the nine
    /// <c>DrawingTextBody.AutoNumber</c> indexes, so a depth the record should not hold cannot
    /// walk off the end of one.
    /// </remarks>
    public const int Levels = 9;

    /// <summary>The scheme this maps to nothing, so nothing is drawn.</summary>
    private const string Unmapped = "";

    /// <summary>
    /// The next number at a level, advancing the counters the body owns, or null for no number.
    /// </summary>
    /// <remarks>
    /// Null for an empty paragraph and for a scheme with no DrawingML name, and in neither case is
    /// a counter touched: an empty item does not consume a number (the blank line an author leaves
    /// between two items would otherwise make the second jump from 2 to 4), and a scheme this
    /// cannot spell must leave the paragraph's own bullet in place rather than relabel it.
    /// </remarks>
    /// <param name="extended">The paragraph's extension entry, which states it is numbered.</param>
    /// <param name="depth">The outline level, zero for the first.</param>
    /// <param name="hasText">Whether the paragraph has any characters.</param>
    /// <param name="counters">The current number at each level.</param>
    /// <param name="counting">Whether each level is inside a run of numbering.</param>
    public static string? Next(
        PptExtendedParagraph extended, int depth, bool hasText, int[] counters, bool[] counting)
    {
        ArgumentNullException.ThrowIfNull(counters);
        ArgumentNullException.ThrowIfNull(counting);

        if (!hasText) return null;
        if (SchemeOf(extended.SchemeKind) is not { Length: > 0 } scheme) return null;

        int level = Math.Clamp(depth, 0, Levels - 1);
        XElement stated = new(Drawing.Name("buAutoNum"), new XAttribute("type", scheme));

        if (extended.Start is { } start)
        {
            stated.Add(new XAttribute("startAt", start.ToString(CultureInfo.InvariantCulture)));
        }

        return DrawingTextBody.AutoNumber(stated, level, counters, counting);
    }

    /// <summary>Ends the run of numbering at a level, so the next number there restarts.</summary>
    /// <param name="depth">The outline level, zero for the first.</param>
    /// <param name="counting">Whether each level is inside a run of numbering.</param>
    public static void Break(int depth, bool[] counting)
    {
        ArgumentNullException.ThrowIfNull(counting);

        int level = Math.Clamp(depth, 0, Levels - 1);
        for (int deeper = level; deeper < counting.Length; deeper++) counting[deeper] = false;
    }

    /// <summary>
    /// PowerPoint's numbering scheme as the <c>ST_TextAutonumberScheme</c> name that spells it.
    /// </summary>
    private static string SchemeOf(int scheme) => scheme switch
    {
        0 => "alphaLcPeriod",
        1 => "alphaUcPeriod",
        2 => "arabicParenR",
        3 => "arabicPeriod",
        4 => "romanLcParenBoth",
        5 => "romanLcParenR",
        6 => "romanLcPeriod",
        7 => "romanUcPeriod",
        8 => "alphaLcParenBoth",
        9 => "alphaLcParenR",
        10 => "alphaUcParenBoth",
        11 => "alphaUcParenR",
        12 => "arabicParenBoth",
        13 => "arabicPlain",
        14 => "romanUcParenBoth",
        15 => "romanUcParenR",

        // Everything else is one of the CJK, Hebrew, Arabic-double-byte or circled families.
        // LibreOffice draws them with a numbering type DrawingML has no name for, so they are
        // left undrawn rather than substituted with a Latin numeral.
        _ => Unmapped,
    };
}
