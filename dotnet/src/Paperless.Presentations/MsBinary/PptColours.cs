using Paperless.Core.Graphics;
using Paperless.MsBinary.Escher;
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

    /// <summary>Whether two schemes name the same eight colours.</summary>
    /// <remarks>
    /// LibreOffice compares the thirty-two raw bytes (<c>HeaderFooterEntry::NeedToImportInstance</c>,
    /// <c>svdfppt.cxx:3131</c>). The fourth byte of each slot is ignored on read, so comparing the
    /// eight colours is the same test on any file whose padding is what the writer put there.
    /// </remarks>
    /// <param name="other">The scheme to compare against.</param>
    public bool SameColoursAs(PptColourScheme? other)
    {
        if (other is null) return false;
        if (ReferenceEquals(this, other)) return true;

        for (int slot = 0; slot < SlotCount; slot++)
        {
            if (_slots[slot] != other._slots[slot]) return false;
        }
        return true;
    }

    /// <summary>
    /// The <c>ColorSchemeAtom</c> instance that marks a page's own scheme —
    /// <c>SlideSchemeColorSchemeAtom</c>.
    /// </summary>
    public const ushort PageSchemeInstance = 1;

    /// <summary>
    /// A page's own colour scheme: the <c>ColorSchemeAtom</c> whose instance says it is one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A slide master carries several, and only one of them is its scheme.</strong> The
    /// header's instance tells them apart: 1 is the page's <c>SlideSchemeColorSchemeAtom</c>, and
    /// 6 is a <c>SchemeListElementColorSchemeAtom</c> — a member of the palette PowerPoint offers
    /// in its scheme picker, which the page does not use. LibreOffice writes exactly that pair of
    /// instances in its own export (<c>eppt.cxx:337</c> and <c>eppt.cxx:396-408</c>).
    /// </para>
    /// <para>
    /// <strong>Taking the first atom in the container reads a palette entry as the page's
    /// scheme</strong>, because the list entries are written before the drawing and the real one
    /// after it. LibreOffice never notices: its <c>SeekToRec</c> for the scheme runs after it has
    /// already sought past the <c>PPDrawing</c> record (<c>svdfppt.cxx:1590-1622</c>), so the scan
    /// starts beyond the list and lands on the right atom by position rather than by instance.
    /// Measured on a corpus deck whose master lists twelve: the first is white paper with black
    /// text and the real one is a navy page with white text, so every placeholder filling with
    /// "text and lines" came out black on black.
    /// </para>
    /// <para>
    /// The first atom of any instance is still the fallback, for a page that states one without
    /// saying which kind it is.
    /// </para>
    /// </remarks>
    /// <param name="stream">The stream the page lives in.</param>
    /// <param name="page">The <c>Slide</c>, <c>MainMaster</c> or <c>Notes</c> container.</param>
    public static PptColourScheme? OfPage(DffRecordBuffer stream, DffRecordHeader page)
    {
        ArgumentNullException.ThrowIfNull(stream);

        DffRecordHeader? fallback = null;

        foreach (DffRecordHeader child in stream.Children(page))
        {
            if (child.Type != ColorSchemeAtom) continue;
            if (child.Instance == PageSchemeInstance) return Read(stream.Content(child));
            fallback ??= child;
        }

        return fallback is { } atom ? Read(stream.Content(atom)) : null;
    }

    /// <summary>The <c>ColorSchemeAtom</c> record type, <c>PPT_PST_ColorSchemeAtom</c>.</summary>
    private const ushort ColorSchemeAtom = 2032;

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
    /// <param name="properties">
    /// The shape's property table, for the <c>0x10</c> family that derives its colour from
    /// another property of the same shape. Null answers null for those, which is what a caller
    /// with no table to hand should do.
    /// </param>
    /// <param name="within">
    /// Which property the word was read from, so a recursive reference back to it can be stopped
    /// and the right fallback chosen. One of <see cref="EscherPropertyIds"/>' colour identifiers.
    /// </param>
    public static Colour? Resolve(
        uint raw,
        PptColourScheme scheme,
        EscherPropertyTable? properties = null,
        ushort within = 0)
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

            return System(raw, scheme, properties, within);
        }

        // PowerPoint's other spelling: a top byte of 4 with nothing else set is a scheme index
        // too (msdffimp.cxx:3635).
        if ((upper & 0x04) != 0 && (raw & 0xFFFFF8) == 0) return scheme[upper];

        return Literal(raw);
    }

    /// <summary>
    /// The <c>0x10</c> family: a colour named indirectly, then put through a function.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The family holds two quite different things behind one flag bit, and only the first is
    /// unresolvable. Indices below <c>0xF0</c> name a <em>desktop</em> colour — the window
    /// background, the menu text — and a headless renderer has no desktop to ask, so inventing
    /// one would put a colour in the picture that no file states; those still come back null.
    /// From <c>0xF0</c> up the index names <em>another property of the same shape</em>
    /// (<c>include/svx/msdffdef.hxx:818-826</c>), which is entirely resolvable and has nothing
    /// to do with a theme.
    /// </para>
    /// <para>
    /// Treating the whole family as unresolvable is what this replaces, and the corpus says the
    /// distinction is the whole of it: of the 161 such words across the 51 <c>ppt</c> decks,
    /// <strong>every one</strong> names a property and none names a desktop colour. The clearest
    /// is <c>slides/batch-014/ppt/ws_prod-g-doc-Events-2008-February-5-NATO-activities.ppt</c>,
    /// whose master background is a shade whose second colour is <c>0x104301F0</c> — "take
    /// <c>fillColor</c>, darken it by 67/256" — over a <c>fillColor</c> of <c>#311577</c>. That
    /// is <c>#0C051F</c>, which is what LibreOffice's flat-ODF export of the deck states; falling
    /// back to white instead made every one of its fourteen pages a pale gradient where the
    /// reference draws a near-black one.
    /// </para>
    /// <para>
    /// The parameter, the function and the two flag bits are read exactly as
    /// <c>MSO_CLR_ToColor</c> reads them (<c>msdffimp.cxx:3456-3633</c>), including its shift of
    /// the additional flags by eight rather than twelve, which leaves them in the high nibble of
    /// a byte and is why they are tested against <c>0x80</c>, <c>0x40</c> and <c>0x20</c>.
    /// </para>
    /// </remarks>
    private static Colour? System(
        uint raw, PptColourScheme scheme, EscherPropertyTable? properties, ushort within)
    {
        if (properties is null) return null;

        (ushort source, uint fallback) = Source((int)(raw & 0xFF), properties);
        if (source == 0) return null;

        // The same guard LibreOffice uses: a property whose own word carries 0x10000000 would
        // send this straight back where it came from.
        uint referenced = properties.Value(source, fallback);
        if (source == within || (referenced & 0x10000000) != 0) return null;
        if (Resolve(referenced, scheme, properties, source) is not { } resolved) return null;

        int parameter = (int)((raw >> 16) & 0xFF);
        int function = (int)((raw & 0x00000F00) >> 8);
        int flags = (int)((raw & 0x0000F000) >> 8);

        byte r = resolved.R, g = resolved.G, b = resolved.B;

        if ((flags & 0x80) != 0)
        {
            // Color::GetLuminance, include/tools/color.hxx:274.
            byte grey = (byte)(((b * 29) + (g * 151) + (r * 76)) >> 8);
            r = g = b = grey;
        }

        switch (function)
        {
            case 1: // darken by the parameter
                r = (byte)((parameter * r) >> 8);
                g = (byte)((parameter * g) >> 8);
                b = (byte)((parameter * b) >> 8);
                break;

            case 2: // lighten by the parameter
                int inverse = (0xFF - parameter) * 0xFF;
                r = (byte)((inverse + (parameter * r)) >> 8);
                g = (byte)((inverse + (parameter * g)) >> 8);
                b = (byte)((inverse + (parameter * b)) >> 8);
                break;

            case 3: // add a grey level
                r = Clamp(r + parameter);
                g = Clamp(g + parameter);
                b = Clamp(b + parameter);
                break;

            case 4: // subtract a grey level
                r = Clamp(r - parameter);
                g = Clamp(g - parameter);
                b = Clamp(b - parameter);
                break;

            case 5: // subtract from a grey level
                r = Clamp(parameter - r);
                g = Clamp(parameter - g);
                b = Clamp(parameter - b);
                break;

            case 6: // per component, black below the parameter and white at or above it
                r = r < parameter ? (byte)0 : (byte)0xFF;
                g = g < parameter ? (byte)0 : (byte)0xFF;
                b = b < parameter ? (byte)0 : (byte)0xFF;
                break;

            default:
                break;
        }

        if ((flags & 0x40) != 0) (r, g, b) = ((byte)(r ^ 0x80), (byte)(g ^ 0x80), (byte)(b ^ 0x80));
        if ((flags & 0x20) != 0) (r, g, b) = ((byte)(0xFF - r), (byte)(0xFF - g), (byte)(0xFF - b));

        return new Colour(r, g, b);
    }

    /// <summary>
    /// Which property an index from <c>0xF0</c> up names, and the value it takes when unstated.
    /// </summary>
    /// <remarks>
    /// The defaults are LibreOffice's own and are not uniform — a missing line colour is black
    /// and a missing fill colour is white — so they are carried here rather than left to the
    /// property table's caller. <c>0xF1</c> is the only one that asks a second question: it takes
    /// the line colour when <c>fNoLineDrawDash</c> says the shape is outlined and the fill colour
    /// when it does not.
    /// </remarks>
    private static (ushort Property, uint Fallback) Source(int index, EscherPropertyTable properties)
        => index switch
        {
            0xF0 or 0xF4 or 0xF7 or 0xFF => (EscherPropertyIds.FillColour, 0xFFFFFF),
            0xF1 => (properties.Value(NoLineDrawDash) & 8) != 0
                ? (EscherPropertyIds.LineColour, 0u)
                : (EscherPropertyIds.FillColour, 0xFFFFFF),
            0xF2 => (EscherPropertyIds.LineColour, 0u),
            0xF3 => (EscherPropertyIds.ShadowColour, 0x808080),
            0xF5 => (FillBackColour, 0xFFFFFF),
            0xF6 => (LineBackColour, 0xFFFFFF),
            _ => (0, 0),
        };

    /// <summary><c>DFF_Prop_fillBackColor</c>, the second colour of a shade or pattern.</summary>
    private const ushort FillBackColour = 387;

    /// <summary><c>DFF_Prop_lineBackColor</c>.</summary>
    private const ushort LineBackColour = 450;

    /// <summary><c>DFF_Prop_fNoLineDrawDash</c>, whose bit 3 is "the shape has a line".</summary>
    private const ushort NoLineDrawDash = 511;

    private static byte Clamp(int value) => (byte)Math.Clamp(value, 0, 0xFF);

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
