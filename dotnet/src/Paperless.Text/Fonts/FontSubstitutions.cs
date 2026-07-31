namespace Paperless.Text.Fonts;

/// <summary>
/// LibreOffice's own font substitution table: what it renders when a requested font is absent.
/// </summary>
/// <remarks>
/// <para>
/// Generated from <c>officecfg/registry/data/org/openoffice/VCL.xcu</c> rather than reimplemented.
/// The research notes call that file "the single most valuable, portable artifact for a C# port",
/// because it encodes what LibreOffice actually falls back to independently of any platform font API
/// — and a substitution that differs from LibreOffice's reflows the document, which is the one class
/// of difference that cannot be worked around downstream
/// (<c>research/06-rendering.md</c> section B.1).
/// </para>
/// <para>
/// The chains that matter most are the metric-compatible ones: Calibri to Carlito, Cambria to
/// Caladea, Arial to Liberation Sans, Times New Roman to Liberation Serif, Courier New to Liberation
/// Mono. Those substitutes are built to the originals' advance widths, so the text occupies the same
/// space and breaks in the same places. Every other entry in the table is a best effort that will
/// reflow, and the reader is told which it got.
/// </para>
/// </remarks>
public static partial class FontSubstitutions
{
    /// <summary>How many requested names the table knows a chain for.</summary>
    public static int Count => Chains.Count;

    /// <summary>
    /// Normalises a font name to the form the table is keyed on.
    /// </summary>
    /// <remarks>
    /// Lower case with every space and punctuation mark removed, which is what LibreOffice's own
    /// configuration stores: "Times New Roman" is <c>timesnewroman</c>. A document may spell the same
    /// font "Arial", "arial" or "Arial Unicode MS", so nothing matches without normalising both
    /// sides.
    /// </remarks>
    public static string Normalise(string? familyName)
    {
        if (string.IsNullOrWhiteSpace(familyName)) return string.Empty;

        Span<char> buffer = familyName.Length <= 128
            ? stackalloc char[familyName.Length]
            : new char[familyName.Length];

        int length = 0;
        foreach (char character in familyName)
        {
            if (char.IsAsciiLetterOrDigit(character))
            {
                buffer[length++] = char.ToLowerInvariant(character);
            }
            else if (character > 127 && char.IsLetterOrDigit(character))
            {
                // A non-ASCII family name — a CJK font naming itself in its own script — keeps its
                // letters, since dropping them would collapse every such name to the empty string.
                buffer[length++] = char.ToLowerInvariant(character);
            }
        }

        return new string(buffer[..length]);
    }

    /// <summary>
    /// The substitutes LibreOffice would try for a requested family, best first.
    /// </summary>
    /// <remarks>
    /// Normalised names, in the table's own order. The requested name itself appears in most chains
    /// and is not added when it does not: the table's order is LibreOffice's preference, and putting
    /// the request first regardless would defeat entries that deliberately prefer a substitute.
    /// </remarks>
    public static IReadOnlyList<string> ChainFor(string? familyName)
        => Chains.TryGetValue(Normalise(familyName), out string[]? chain) ? chain : [];

    /// <summary>
    /// The Microsoft font a free face stands in for, or null when the table names none.
    /// </summary>
    /// <remarks>
    /// The table's other direction: it is how a free font declares which commercial one it is
    /// metric-compatible with, and so which requests it can satisfy without reflowing the document.
    /// </remarks>
    public static string? MicrosoftEquivalentOf(string? familyName)
        => MicrosoftEquivalents.TryGetValue(Normalise(familyName), out string? equivalent)
            ? equivalent
            : null;

    /// <summary>
    /// True when two families are metric-compatible, so substituting one for the other preserves
    /// every line break.
    /// </summary>
    /// <remarks>
    /// Decided from the table rather than from a hardcoded list of pairs: a face is
    /// metric-compatible with the Microsoft font it declares itself an equivalent of, and with any
    /// other face declaring the same one. That makes Carlito compatible with Calibri, and Arimo
    /// compatible with Liberation Sans, without either pair being named here.
    /// </remarks>
    public static bool AreMetricCompatible(string? first, string? second)
    {
        string a = Normalise(first);
        string b = Normalise(second);
        if (a.Length == 0 || b.Length == 0) return false;
        if (string.Equals(a, b, StringComparison.Ordinal)) return true;

        string? equivalentOfA = MicrosoftEquivalentOf(a);
        string? equivalentOfB = MicrosoftEquivalentOf(b);

        // One declares itself the other's equivalent.
        if (Normalise(equivalentOfA) == b || Normalise(equivalentOfB) == a) return true;

        // Both declare the same equivalent, so they are compatible with each other through it.
        return equivalentOfA is not null
               && equivalentOfB is not null
               && Normalise(equivalentOfA) == Normalise(equivalentOfB);
    }
}
