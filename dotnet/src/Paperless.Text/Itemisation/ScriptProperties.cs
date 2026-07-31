namespace Paperless.Text.Itemisation;

/// <summary>
/// The UAX #24 <c>Script</c> property, as the ISO 15924 codes a shaper wants.
/// </summary>
/// <remarks>
/// <para>
/// Codes rather than an enum, because the only consumer is HarfBuzz and HarfBuzz takes a four-letter
/// tag. An enum would have to be translated back into one at every call, and the translation table
/// would be a second place for a script to be missing from.
/// </para>
/// <para>
/// Two of LibreOffice's own adjustments are baked into the generated table rather than applied here,
/// so that the itemiser cannot forget them: a non-spacing mark reports <c>Zinh</c> whatever its own
/// script says (tdf#154549), and Katakana reports <c>Hira</c> because the three Japanese script codes
/// share one OpenType script tag and splitting them would cost a shaping boundary for nothing. Both
/// are <c>getScript</c> in <c>vcl/source/gdi/scrptrun.cxx</c>.
/// </para>
/// </remarks>
public static partial class ScriptProperties
{
    /// <summary>The code for characters that belong to no script in particular, such as a space.</summary>
    public const string Common = "Zyyy";

    /// <summary>The code for characters that take their script from what precedes them.</summary>
    public const string Inherited = "Zinh";

    /// <summary>The code for a character with no script assigned, including the unassigned ones.</summary>
    public const string Unknown = "Zzzz";

    /// <summary>The ISO 15924 code of a code point.</summary>
    public static string ScriptOf(int codePoint)
    {
        if (codePoint is < 0 or >= 0x110000) return Unknown;

        int low = 0;
        int high = ScriptStarts.Length - 1;
        while (low <= high)
        {
            int middle = (low + high) >> 1;
            if (codePoint < ScriptStarts[middle]) high = middle - 1;
            else if (codePoint >= ScriptEnds[middle]) low = middle + 1;
            else return Codes[ScriptIndices[middle]];
        }

        return Unknown;
    }

    /// <summary>
    /// True for the two scripts that take their identity from their neighbours.
    /// </summary>
    /// <remarks>
    /// <c>Common</c> and <c>Inherited</c> are what UAX #24's resolution rule is about: a full stop
    /// between two Greek words is Greek, and the same full stop between two Arabic words is Arabic.
    /// A run boundary at every punctuation mark would be both wrong and expensive — it would cost the
    /// shaping context across the mark, and the measured width with it.
    /// </remarks>
    public static bool IsWeak(string script)
        => string.Equals(script, Common, StringComparison.Ordinal)
           || string.Equals(script, Inherited, StringComparison.Ordinal);
}
