using Paperless.Core.Graphics;
using Paperless.MsBinary.Records;

namespace Paperless.Presentations.MsBinary;

/// <summary>
/// A page's <c>ColorSchemeAtom</c>: the eight slots every colour in a PowerPoint file may name
/// instead of stating red, green and blue.
/// </summary>
/// <remarks>
/// <para>
/// The binary format's answer to DrawingML's colour scheme, and the reason a colour read out of a
/// <c>TxMasterStyleAtom</c> or an Escher property table cannot be resolved without knowing which
/// <em>page</em> it was on: the atom hangs off the slide, not off the master
/// (<c>filter/source/msfilter/svdfppt.cxx:1622</c>), and a slide whose <c>SlideAtom</c> flags
/// carry bit 1 follows its master's instead (<c>svdfppt.cxx:2568</c>).
/// </para>
/// <para>
/// Thirty-two bytes, four per slot, red first and the fourth byte ignored
/// (<c>PptColorSchemeAtom::GetColor</c>, <c>svdfppt.cxx:362</c>). The slots are positional and
/// have fixed meanings — background, text and lines, shadows, title text, fills, accent, accent
/// and hyperlink, accent and followed hyperlink — which is why a deck's colours change wholesale
/// when its scheme is swapped.
/// </para>
/// </remarks>
public sealed class PptColourScheme
{
    /// <summary>How many slots the atom holds.</summary>
    public const int SlotCount = 8;

    /// <summary>The slot a run with no colour of its own falls back to.</summary>
    public const int BackgroundSlot = 0;

    /// <summary>The slot ordinary text uses.</summary>
    public const int TextSlot = 1;

    /// <summary>The slot title text uses.</summary>
    public const int TitleTextSlot = 3;

    private readonly Colour[] _slots;

    private PptColourScheme(Colour[] slots) => _slots = slots;

    /// <summary>
    /// The scheme a page with no atom of its own falls back to.
    /// </summary>
    /// <remarks>
    /// White paper, black text and lines, grey shadows, black titles, and four accents.
    /// Every writer emits the atom — both corpus decks do, on every page — so nothing measured
    /// reaches this; it exists because the alternative is what LibreOffice does, which is to warn
    /// (<c>svdfppt.cxx:1626</c>) and carry on with an all-zero scheme that draws black text on a
    /// black slide.
    /// </remarks>
    public static PptColourScheme Default { get; } = new(
    [
        Colour.White, Colour.Black,
        new Colour(0x80, 0x80, 0x80), Colour.Black,
        Colour.White, new Colour(0x33, 0x33, 0x99),
        new Colour(0x00, 0x00, 0x99), new Colour(0x99, 0xCC, 0x00),
    ]);

    /// <summary>The colour in a slot, or the default scheme's when the index is out of range.</summary>
    public Colour this[int slot]
        => slot >= 0 && slot < SlotCount ? _slots[slot] : Default._slots[BackgroundSlot];

    /// <summary>
    /// Reads a <c>ColorSchemeAtom</c> payload, or returns null when it is too short to be one.
    /// </summary>
    /// <param name="content">The atom's payload.</param>
    public static PptColourScheme? Read(ReadOnlySpan<byte> content)
    {
        if (content.Length < SlotCount * 4) return null;

        Colour[] slots = new Colour[SlotCount];
        for (int slot = 0; slot < SlotCount; slot++)
        {
            int at = slot * 4;
            slots[slot] = new Colour(content[at], content[at + 1], content[at + 2]);
        }

        return new PptColourScheme(slots);
    }
}

/// <summary>
/// Turns the drawing layer's packed colour words into real colours.
/// </summary>
/// <remarks>
/// <para>
/// A <c>MSO_CLR</c> is thirty-two bits whose <em>top byte</em> decides what the other three mean:
/// a literal <c>0x00BBGGRR</c>, an index into the page's colour scheme, or a system colour with a
/// function applied to it. Ported from <c>SvxMSDffManager::MSO_CLR_ToColor</c>
/// (<c>filter/source/msfilter/msdffimp.cxx:3420</c>) and its text-flavoured wrapper
/// <c>MSO_TEXT_CLR_ToColor</c> (<c>msdffimp.cxx:3403</c>).
/// </para>
/// <para>
/// <strong>Text and shape colours are packed differently, and the difference is silent.</strong>
/// A shape's fill states a scheme colour as <c>0x08000000 | index</c>; a text run states one as
/// <c>index &lt;&lt; 24</c>, so slot 1 is <c>0x01000000</c> — which read as a shape colour is the
/// literal black <c>0x000000</c> under a top byte nobody looked at. Both spellings do occur in
/// the same file, which is why this has two entry points rather than one.
/// </para>
/// </remarks>
public static class PptColour
{
    /// <summary>
    /// A shape property's colour: a literal, or a slot of the page's scheme.
    /// </summary>
    /// <param name="raw">The packed word, as the property table states it.</param>
    /// <param name="scheme">The page's colour scheme.</param>
    /// <remarks>
    /// System colours — the <c>0x10</c> family, which resolve against the desktop theme and then
    /// darken, lighten or invert by a parameter — are not resolved: a headless renderer has no
    /// desktop theme, and inventing one would put a colour in the picture that no file states.
    /// They come back null, which the caller treats as "no fill" rather than as black.
    /// </remarks>
    public static Colour? Resolve(uint raw, PptColourScheme scheme)
    {
        ArgumentNullException.ThrowIfNull(scheme);

        // 0xfe marks a colour PowerPoint wrote for text and something else read as a shape's;
        // the low three bytes are a literal either way.
        if ((raw & 0xFE000000) == 0xFE000000) return Literal(raw);

        byte upper = (byte)(raw >> 24);

        if ((upper & 0x19) != 0)
        {
            if ((upper & 0x08) != 0) return scheme[(int)(raw & 0xFFFF)];
            if ((upper & 0x10) == 0) return scheme[upper];

            // A system colour. Deliberately unresolved; see the remarks.
            return null;
        }

        // PowerPoint's other spelling: a top byte of 4 with nothing else set is a scheme index
        // too (msdffimp.cxx:3635).
        if ((upper & 0x04) != 0 && (raw & 0xFFFFF8) == 0) return scheme[upper];

        return Literal(raw);
    }

    /// <summary>
    /// A text run's colour, whose scheme indices are packed into the top byte.
    /// </summary>
    /// <param name="raw">The packed word, as a character property or style sheet states it.</param>
    /// <param name="scheme">The page's colour scheme.</param>
    public static Colour? ResolveText(uint raw, PptColourScheme scheme)
    {
        if ((raw & 0xFE000000) == 0xFE000000) return Literal(raw);

        // Anything below 0x08000000 is an index in the top byte rather than a colour at all.
        if ((raw & 0xF8000000) == 0) raw = (raw >> 24) | 0x08000000;

        return Resolve(raw, scheme);
    }

    /// <summary>The low three bytes, which the format orders blue-green-red.</summary>
    private static Colour Literal(uint raw)
        => new((byte)raw, (byte)(raw >> 8), (byte)(raw >> 16));
}

/// <summary>
/// The document's font collection: the names a run's <c>cfTypeface</c> index refers to.
/// </summary>
/// <remarks>
/// One <c>FontEntityAtom</c> per face inside the <c>Environment</c>'s <c>FontCollection</c>, each
/// beginning with a fixed sixty-four-byte UTF-16 name field padded with NULs
/// (<c>ReadPptFontEntityAtom</c>, <c>svdfppt.cxx:383</c>). The index is the atom's position in the
/// collection, not its record instance — the instance is the font's own id in some writers and
/// zero in others, so counting is the only reliable answer.
/// </remarks>
public sealed class PptFontTable
{
    /// <summary>The fixed width of the name field, in bytes.</summary>
    private const int NameBytes = 64;

    /// <summary>
    /// <c>lfCharSet</c>'s value for a symbol font, which changes what its characters mean.
    /// </summary>
    /// <remarks>
    /// The byte immediately after the name field (<c>ReadPptFontEntityAtom</c>,
    /// <c>svdfppt.cxx:390-401</c>). Two is Windows's <c>SYMBOL_CHARSET</c>.
    /// </remarks>
    private const byte SymbolCharSet = 2;

    private readonly List<string> _names;
    private readonly List<bool> _symbols;

    private PptFontTable(List<string> names, List<bool> symbols)
    {
        _names = names;
        _symbols = symbols;
    }

    /// <summary>A collection holding nothing, for a document with no environment.</summary>
    public static PptFontTable Empty { get; } = new([], []);

    /// <summary>How many faces the collection names.</summary>
    public int Count => _names.Count;

    /// <summary>The face at an index, or null when the collection does not reach that far.</summary>
    public string? this[int index]
        => index >= 0 && index < _names.Count ? _names[index] : null;

    /// <summary>
    /// Whether the face at an index declares the symbol character set.
    /// </summary>
    /// <remarks>
    /// It is not a property of the face so much as of how its code points must be read: a
    /// character shown in a symbol font means the glyph at that slot rather than the letter, so
    /// the same byte is <c>l</c> in Arial and a filled circle in Wingdings. LibreOffice moves such
    /// a character into the Private Use Area before doing anything with it
    /// (<c>svdfppt.cxx:3767-3771</c>), which is what stops a bullet being drawn as a letter.
    /// </remarks>
    public bool IsSymbol(int index)
        => index >= 0 && index < _symbols.Count && _symbols[index];

    /// <summary>Reads the collection out of a document's <c>Environment</c> container.</summary>
    /// <param name="stream">The document stream.</param>
    /// <param name="environment">The <c>Environment</c> container, or null when there is none.</param>
    public static PptFontTable Read(DffRecordBuffer stream, DffRecordHeader? environment)
    {
        ArgumentNullException.ThrowIfNull(stream);
        if (environment is not { } container) return Empty;

        List<string> names = [];
        List<bool> symbols = [];

        foreach (DffRecordHeader child in stream.Children(container))
        {
            if (child.Type != PptRecordTypes.FontCollection) continue;

            foreach (DffRecordHeader entity in stream.Children(child))
            {
                if (entity.Type != PptRecordTypes.FontEntityAtom) continue;

                ReadOnlySpan<byte> content = stream.Content(entity);
                names.Add(NameOf(content));
                symbols.Add(content.Length > NameBytes && content[NameBytes] == SymbolCharSet);
            }
        }

        return names.Count > 0 ? new PptFontTable(names, symbols) : Empty;
    }

    private static string NameOf(ReadOnlySpan<byte> content)
    {
        int usable = Math.Min(content.Length, NameBytes) & ~1;
        if (usable <= 0) return string.Empty;

        string name = System.Text.Encoding.Unicode.GetString(content[..usable]);
        int nul = name.IndexOf('\0', StringComparison.Ordinal);
        return nul >= 0 ? name[..nul] : name;
    }
}
