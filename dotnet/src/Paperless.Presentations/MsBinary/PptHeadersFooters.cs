using Paperless.MsBinary.Records;

namespace Paperless.Presentations.MsBinary;

/// <summary>
/// Which of the four running placeholders a page shows, and the text each of them stands for.
/// </summary>
/// <remarks>
/// <para>
/// The date, the header, the footer and the slide number are not four properties of a page: they
/// are four <em>shapes on the master</em>, and the page only says which of them to show and what
/// string to put in them. So a deck states the shape once and the visibility once per slide,
/// which is why drawing the master's shapes without consulting this puts a footer on the title
/// slide of nearly every real deck.
/// </para>
/// <para>
/// A slide starts from its master's settings and overrides whatever its own container states
/// (<c>HeaderFooterEntry</c>'s copying constructor, <c>svdfppt.cxx:3089-3105</c>, then
/// <c>ImportHeaderFooterContainer</c> at <c>svdfppt.cxx:3141</c>). Nothing is visible by default:
/// the entry's atom starts at zero, so a deck that never writes the record shows none of the four
/// even where its master carries the placeholders.
/// </para>
/// </remarks>
/// <param name="Atom">
/// The <c>HeadersFootersAtom</c>'s flags word. Kept whole rather than split into booleans because
/// it also carries the date format in its low byte, which the same record has to answer for.
/// </param>
/// <param name="Date">The fixed date string, when the atom does not ask for today's.</param>
/// <param name="Header">The header string.</param>
/// <param name="Footer">The footer string.</param>
internal readonly record struct PptHeadersFooters(
    uint Atom, string? Date, string? Header, string? Footer)
{
    /// <summary>Nothing shown, which is what a deck stating no record gets.</summary>
    public static PptHeadersFooters None => default;

    /// <summary>Whether the page shows its master's date placeholder.</summary>
    public bool ShowsDate => (Atom & 0x010000) != 0;

    /// <summary>Whether the page shows its master's header placeholder.</summary>
    public bool ShowsHeader => (Atom & 0x100000) != 0;

    /// <summary>Whether the page shows its master's footer placeholder.</summary>
    public bool ShowsFooter => (Atom & 0x200000) != 0;

    /// <summary>Whether the page shows its master's slide-number placeholder.</summary>
    public bool ShowsSlideNumber => (Atom & 0x080000) != 0;

    /// <summary>
    /// Whether the date placeholder shows today's date rather than <see cref="Date"/>.
    /// </summary>
    /// <remarks>
    /// A rendering has to choose one, and choosing today's would make every comparison against a
    /// stored reference fail on the day after it was taken. Callers substitute
    /// <see cref="Date"/> and leave an automatic date empty, which is also what a deck written
    /// with a fixed date renders as.
    /// </remarks>
    public bool DateIsAutomatic => (Atom & 0x020000) != 0;

    /// <summary>Whether this page shows the instance a placeholder id names.</summary>
    /// <param name="placeholder">A master placeholder id, as <see cref="PptPlaceholders"/> numbers them.</param>
    public bool Shows(int placeholder) => placeholder switch
    {
        PptPlaceholders.MasterDate => ShowsDate,
        PptPlaceholders.MasterSlideNumber => ShowsSlideNumber,
        PptPlaceholders.MasterFooter => ShowsFooter,
        PptPlaceholders.MasterHeader => ShowsHeader,
        _ => true,
    };

    /// <summary>
    /// Reads a <c>HeadersFooters</c> container over <paramref name="inherited"/>.
    /// </summary>
    /// <remarks>
    /// A merge rather than a replacement: a slide that states only a footer string keeps its
    /// master's visibility word, and one that states only the word keeps the master's strings.
    /// </remarks>
    /// <param name="stream">The document stream.</param>
    /// <param name="container">The <c>HeadersFooters</c> container.</param>
    /// <param name="inherited">What the page inherits, which is its master's settings.</param>
    public static PptHeadersFooters Read(
        DffRecordBuffer stream, DffRecordHeader container, PptHeadersFooters inherited)
    {
        ArgumentNullException.ThrowIfNull(stream);

        PptHeadersFooters result = inherited;

        foreach (DffRecordHeader child in stream.Children(container))
        {
            ReadOnlySpan<byte> content = stream.Content(child);

            switch (child.Type)
            {
                case PptRecordTypes.HeadersFootersAtom when content.Length >= 4:
                    result = result with { Atom = DffRecordBuffer.ReadUInt32(content) };
                    break;

                // The instance is the discriminator: 0 date, 1 header, 2 footer. There is no
                // fourth string — a slide number is a number, not a caption.
                case PptRecordTypes.CString when child.Instance == 0:
                    result = result with { Date = Text(content) };
                    break;

                case PptRecordTypes.CString when child.Instance == 1:
                    result = result with { Header = Text(content) };
                    break;

                case PptRecordTypes.CString when child.Instance == 2:
                    result = result with { Footer = Text(content) };
                    break;

                default:
                    break;
            }
        }

        return result;
    }

    private static string Text(ReadOnlySpan<byte> content)
        => System.Text.Encoding.Unicode.GetString(content[..(content.Length & ~1)]).TrimEnd('\0');
}

/// <summary>
/// The placeholder ids a <c>PPT_PST_OEPlaceholderAtom</c> uses, as far as slide rendering cares.
/// </summary>
/// <remarks>
/// Numbering follows <c>PptPlaceholder</c> in <c>include/filter/msfilter/svdfppt.hxx:1455</c>,
/// which is <em>not</em> the numbering the specification's prose uses — the file's ids and the
/// documentation's list differ from 4 onwards, and the file's are the ones that matter.
/// </remarks>
internal static class PptPlaceholders
{
    /// <summary>No placeholder; an ordinary shape.</summary>
    public const int None = 0;

    /// <summary>The master's title prompt.</summary>
    public const int MasterTitle = 1;

    /// <summary>The master's outline body prompt, the last of the ids drawn only in master view.</summary>
    public const int MasterNotesBodyImage = 6;

    /// <summary>The master's date placeholder.</summary>
    public const int MasterDate = 7;

    /// <summary>The master's slide-number placeholder.</summary>
    public const int MasterSlideNumber = 8;

    /// <summary>The master's footer placeholder.</summary>
    public const int MasterFooter = 9;

    /// <summary>The master's header placeholder.</summary>
    public const int MasterHeader = 10;

    /// <summary>
    /// Whether an id names one of the prompts a master shows only in master view.
    /// </summary>
    /// <remarks>
    /// Ids 1 to 6 are the master's title, body, centred title, subtitle and the two notes images.
    /// Impress marks each of them <c>SetNotVisibleAsMaster</c> on import
    /// (<c>sd/source/filter/ppt/pptin.cxx:2341</c>) and then refuses to draw any presentation
    /// object of a master page while a slide is shown
    /// (<c>SdPage::checkVisibility</c>, <c>sd/source/core/sdpage.cxx:2987-2991</c>). Without this
    /// every slide of a French deck carries "Cliquez pour modifier le style du titre".
    /// </remarks>
    public static bool IsMasterPrompt(int id) => id is >= MasterTitle and <= MasterNotesBodyImage;

    /// <summary>
    /// Whether an id names one of the four running placeholders — date, slide number, footer,
    /// header — that a slide switches on and off and can recolour.
    /// </summary>
    /// <remarks>
    /// These are the four <c>HeaderFooterOfs</c> records exactly
    /// (<c>filter/source/msfilter/svdfppt.cxx:756-772</c>).
    /// </remarks>
    public static bool IsRunning(int id) => id is >= MasterDate and <= MasterHeader;
}
