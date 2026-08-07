namespace Paperless.Text.Fonts;

/// <summary>
/// The shape LibreOffice files a family under, from its <c>FontType</c> in <c>VCL.xcu</c>.
/// </summary>
/// <remarks>
/// What a substitution falls back to once the named chain has failed. The distinction is coarse on
/// purpose — it is the same one the configuration draws, and it is the one that survives not having
/// the requested face: a grotesque stands in for a grotesque, and a monospaced request keeps its
/// columns.
/// </remarks>
public enum FontFamilyClass
{
    /// <summary>The table names no shape for this family, or has never heard of it.</summary>
    Unknown = 0,

    /// <summary>A grotesque: <c>Normal,SansSerif</c>.</summary>
    SansSerif,

    /// <summary>A roman: <c>Normal,Serif</c>.</summary>
    Serif,

    /// <summary>A monospaced face: <c>Normal,Fixed</c>.</summary>
    Fixed,

    /// <summary>A pi or dingbat face: <c>Symbol,Special</c>.</summary>
    Symbol,
}

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
    {
        string key = Normalise(familyName);
        return !FontconfigOverridesTheChain.Contains(key)
               && Chains.TryGetValue(key, out string[]? chain)
            ? chain
            : [];
    }

    /// <summary>
    /// Families whose <c>SubstFonts</c> chain the running binary demonstrably does not follow.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>The chain is not the first thing LibreOffice consults on this platform.</strong>
    /// <c>PhysicalFontCollection::FindFontFamily</c> calls the pre-match hook at
    /// <c>vcl/source/font/PhysicalFontCollection.cxx:1142</c> and returns whatever it names if that
    /// family is installed (<c>:1151</c>); <c>ImplFontSubstitute</c>, which is this table, is only
    /// reached in the *second* loop at <c>:1180</c>. On Linux the hook is
    /// <c>FcPreMatchSubstitution::FindFontSubstitute</c>
    /// (<c>vcl/unx/generic/font/fontsubst.cxx:98</c>), which asks fontconfig about every request
    /// that is not symbol-encoded — and fontconfig always answers, with its own default family when
    /// it has no rule for the name. So for an uninstalled, non-symbol family the chain never runs.
    /// </para>
    /// <para>
    /// <strong>Measured rather than reasoned.</strong> A flat-ODS probe naming all 296 families the
    /// sample corpus mentions, each row carrying <c>Hamburgefonstiv</c> and <c>0123456789</c> in
    /// that family, was rendered by LibreOffice 24.2.7.2 and read back with <c>pdftotext -bbox</c>:
    /// the two drawn widths identify the face exactly, since the eight installed faces' letter and
    /// digit runs are all more than a point apart. 270 of the 293 it could name agree with this
    /// resolver already. The two below are the ones where the chain reaches an installed face
    /// fontconfig would not have chosen and the family is Latin and not symbol-encoded:
    /// <c>Helv</c> and <c>SansSerif</c> both come back DejaVu Sans, 86.45 and 63.64 points against
    /// Liberation Sans's 75.61 and 55.63.
    /// </para>
    /// <para>
    /// <strong>What is deliberately not here, and why.</strong> The probe disagrees on twenty more
    /// families and none of them is safe to act on from it. Four are <c>Wingdings</c>,
    /// <c>Wingdings 2</c>, <c>Wingdings 3</c> and <c>Webdings</c>, where the probe is the wrong
    /// instrument: ODF states no charset, so the request was not symbol-encoded and the hook did not
    /// bail at <c>fontsubst.cxx:101</c> as it does for a DOCX or XLSX font carrying
    /// <c>charset="2"</c>. Two are <c>MS Gothic</c> and <c>MS PGothic</c>, where fontconfig's answer
    /// depends on the characters asked for and the probe asked in Latin. The rest —
    /// <c>Book Antiqua</c>, <c>Bookman Old Style</c>, <c>Century</c>, <c>Century Schoolbook</c>,
    /// <c>NewCenturySchlbk</c>, <c>CG Times</c>, <c>Times-Roman</c>, <c>Lucida Console</c> reading
    /// DejaVu Sans against our serif or fixed answer, and <c>Nimbus Sans L</c>,
    /// <c>Palatino Linotype</c>, <c>SimSun</c> and the <c>Times New Roman CE</c> family reading a
    /// face we do not reach — need <see cref="ClassOf"/> replaced by fontconfig's own classification
    /// rather than an entry here, which is a larger change than this one and reaches the words and
    /// slides tracks. The measurement is recorded in <c>dotnet/TODO.batches.md</c> so the next round
    /// has it.
    /// </para>
    /// </remarks>
    private static readonly HashSet<string> FontconfigOverridesTheChain =
        new(StringComparer.Ordinal) { "helv", "sansserif" };

    /// <summary>
    /// The shape LibreOffice files a family under, or <see cref="FontFamilyClass.Unknown"/>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The half of the table that decides what a substitution does when <em>nothing</em> in the
    /// chain turned out to be installed — which on a typical Linux box is the common case rather
    /// than the exception, because the chains are full of Microsoft and Agfa faces that are not
    /// there. Tahoma's chain, for instance, names fourteen faces and a machine carrying only the
    /// Liberation and DejaVu families has none of them.
    /// </para>
    /// <para>
    /// It is worth reading from the table rather than guessing from the name because the guess is
    /// wrong for precisely the families that matter. Nothing in the strings "Tahoma", "Verdana" or
    /// "Segoe UI" says grotesque, so a name-based heuristic files all three under roman and renders
    /// a sans-serif document in a serif face.
    /// </para>
    /// </remarks>
    public static FontFamilyClass ClassOf(string? familyName)
        => Classes.TryGetValue(Normalise(familyName), out FontFamilyClass kind)
            ? kind
            : FontFamilyClass.Unknown;

    /// <summary>
    /// The body-text faces LibreOffice falls back through when a document names no family at all.
    /// </summary>
    /// <remarks>
    /// From <c>DefaultFonts</c>/<c>LATIN_TEXT</c> rather than from <c>FontSubstitutions</c>, because
    /// they answer different questions. A request for a face nobody has installed is a substitution;
    /// a request for nothing is the default template speaking, and the two have different answers —
    /// the substitution path ends at fontconfig's generic sans, while this one is a serif list headed
    /// by Liberation Serif. Sending blank requests down the substitution path sets every document
    /// that specifies no font in the wrong shape at once.
    /// </remarks>
    public static IReadOnlyList<string> DefaultLatinTextChain => LatinTextDefaultChain;

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
