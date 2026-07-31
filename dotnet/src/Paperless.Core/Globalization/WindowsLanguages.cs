namespace Paperless.Core.Globalization;

/// <summary>
/// Maps a Windows language identifier to a BCP 47 language tag.
/// </summary>
/// <remarks>
/// <para>
/// Every Microsoft format records a language as a 16-bit <c>LANGID</c> rather than as a tag: DOC and
/// RTF on runs and paragraphs, XLS on cells, PPT on text, and OOXML in the few places it kept a
/// numeric field. Reporting the tag instead means one table serves all of them, and the readers stay
/// free of locale data they would otherwise each carry a fragment of.
/// </para>
/// <para>
/// The table is generated from LibreOffice's own <c>i18nlangtag</c> data, in the order LibreOffice
/// consults its three tables and honouring its override marks — so Paperless answers what LibreOffice
/// answers, which is what makes a comparison against it meaningful. Getting the mapping only mostly
/// right is worse than it sounds: a wrong tag silently mislabels a document's language for spell
/// checking and locale-dependent number formatting, and nothing about the output looks wrong
/// (<c>research/05-infrastructure.md</c> section F.3).
/// </para>
/// <para>
/// An unknown identifier returns null rather than a guess. There is no sensible default: a wrong
/// language tag is a claim about the text, while no tag is merely the absence of one.
/// </para>
/// </remarks>
public static partial class WindowsLanguages
{
    /// <summary>
    /// The identifier meaning "no language" — text explicitly marked as having none.
    /// </summary>
    public const ushort None = 0x0000;

    /// <summary>
    /// The identifier meaning "the process's own default", which says nothing about the text.
    /// </summary>
    public const ushort SystemDefault = 0x0400;

    /// <summary>
    /// The mask selecting the primary language from an identifier, discarding the sublanguage.
    /// </summary>
    /// <remarks>
    /// The low ten bits are the language and the rest is the region — so a Swiss German document and
    /// an Austrian one share a primary language and differ only above the mask. This is how an
    /// unrecognised regional variant can still yield the right language.
    /// </remarks>
    public const ushort PrimaryLanguageMask = 0x03FF;

    /// <summary>How many identifiers the table maps.</summary>
    public static int Count => Table.Length;

    /// <summary>
    /// The BCP 47 tag for a Windows language identifier, or null when there is none.
    /// </summary>
    /// <remarks>
    /// An identifier the table does not list falls back to its primary language, because an
    /// unrecognised regional variant is far more likely than an unrecognised language — and a
    /// document tagged <c>de-DE</c> when it meant an unassigned German sublanguage is right about
    /// everything the tag is used for. When the primary language has no entry of its own, the
    /// language's default sublanguage supplies the tag; nothing outside the language resolves, so the
    /// fallback can widen a claim but never change it.
    /// </remarks>
    public static string? TagOf(ushort identifier)
    {
        if (identifier is None or SystemDefault) return null;

        string? exact = Lookup(identifier);
        if (exact is not null) return exact;

        ushort primary = (ushort)(identifier & PrimaryLanguageMask);
        if (primary == identifier) return null;

        return Lookup(primary) ?? DefaultSublanguage(primary);
    }

    /// <summary>
    /// The tag of the lowest-numbered identifier sharing a primary language, or null when none does.
    /// </summary>
    /// <remarks>
    /// The lowest is the language's default: Windows numbers sublanguages from one, so German's
    /// entries run 0x0407, 0x0807, 0x0C07 and the first is <c>de-DE</c>. Scanned rather than indexed
    /// because this is only reached for an identifier the table does not list at all, which is rare
    /// enough that a second table to make it fast would cost more than it saves.
    /// </remarks>
    private static string? DefaultSublanguage(ushort primary)
    {
        foreach ((ushort candidate, string tag) in Table)
        {
            if ((candidate & PrimaryLanguageMask) == primary) return tag;
        }
        return null;
    }

    /// <summary>
    /// The language subtag alone — the part before the first hyphen — or null when the identifier is
    /// unknown.
    /// </summary>
    /// <remarks>
    /// Separate because the two callers want different things: labelling a run of text wants the full
    /// tag, while choosing a code page for eight-bit text wants only the language, since a code page
    /// is shared across every region a language is written in.
    /// </remarks>
    public static string? LanguageOf(ushort identifier)
    {
        if (TagOf(identifier) is not { } tag) return null;

        int hyphen = tag.IndexOf('-', StringComparison.Ordinal);
        return hyphen < 0 ? tag : tag[..hyphen];
    }

    private static string? Lookup(ushort identifier)
    {
        int low = 0;
        int high = Table.Length - 1;
        while (low <= high)
        {
            int middle = low + ((high - low) / 2);
            ushort candidate = Table[middle].Identifier;
            if (identifier < candidate) high = middle - 1;
            else if (identifier > candidate) low = middle + 1;
            else return Table[middle].Tag;
        }
        return null;
    }
}
