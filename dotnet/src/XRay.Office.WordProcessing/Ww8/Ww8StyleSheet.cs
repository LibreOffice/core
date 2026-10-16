using System.Buffers.Binary;
using System.Text;

namespace XRay.Office.WordProcessing.Ww8;

/// <summary>One style definition from the STSH.</summary>
/// <param name="Name">The style's name as the document records it.</param>
/// <param name="BaseIndex">
/// The style this one is based on, or 4095 for none — WW8's "no parent" is a sentinel rather than a
/// zero, and treating it as an index makes every root style inherit from style 4095.
/// </param>
/// <param name="Kind">
/// What the style applies to: 1 paragraph, 2 character, 3 table, 4 numbering.
/// </param>
/// <param name="Properties">
/// The style's own paragraph formatting, as a grpprl in exactly the same shape as direct formatting.
/// Empty for a character style, which has no paragraph half.
/// </param>
/// <param name="CharacterProperties">
/// The style's own character formatting. A paragraph style has one of these as well as a paragraph
/// grpprl — that is how a heading style makes its text bold without any run saying so — so reading
/// only the first half of a paragraph style's definition loses every character property it sets.
/// </param>
/// <param name="Sti">
/// Which of Word's built-in styles this one is, or <see cref="UserStyle"/> for a style Word does not
/// know. The name cannot answer that question — a document may rename a built-in style or give a
/// style of its own a built-in name — which is why LibreOffice keys on this number and not on the
/// name wherever a rule is about a particular built-in style (<c>SwWW8StyInf::GetWWStyleId</c>,
/// <c>sw/source/filter/ww8/ww8par.hxx</c>:344).
/// </param>
public readonly record struct Ww8Style(
    string Name,
    ushort BaseIndex,
    byte Kind,
    ReadOnlyMemory<byte> Properties,
    ReadOnlyMemory<byte> CharacterProperties,
    ushort Sti = Ww8Style.UserStyle)
{
    /// <summary>The <see cref="BaseIndex"/> value meaning "no parent style".</summary>
    public const ushort NoBaseStyle = 4095;

    /// <summary>
    /// The <see cref="Sti"/> of a style Word has no built-in identity for.
    /// </summary>
    /// <remarks>
    /// <c>ww::stiUser</c>, 0x0ffe (<c>sw/source/filter/inc/wwstyles.hxx</c>:133) — *"user styles are
    /// distinguished by name"*. 0x0fff beside it is <c>stiNil</c>, the twelve-bit maximum.
    /// </remarks>
    public const ushort UserStyle = 0x0FFE;

    /// <summary>
    /// The <see cref="Sti"/> of Word's built-in <c>Hyperlink</c> character style.
    /// </summary>
    /// <remarks>
    /// <c>ww::stiHyperlink</c>, 85 (<c>sw/source/filter/inc/wwstyles.hxx</c>:126). Its sibling
    /// <c>stiHyperlinkFollowed</c> is 86 and is deliberately not named here: the one rule that asks
    /// about this style tests for the unvisited one alone.
    /// </remarks>
    public const ushort HyperlinkStyle = 85;

    /// <summary>True for a paragraph style.</summary>
    public bool IsParagraphStyle => Kind == 1;

    /// <summary>True for a character style.</summary>
    public bool IsCharacterStyle => Kind == 2;
}

/// <summary>
/// The stylesheet: a WW8 document's named styles, with the same sprm-encoded formatting as direct
/// formatting.
/// </summary>
/// <remarks>
/// <para>
/// The STSH is an array of style definitions addressed by index — a paragraph names its style with
/// a number, not a name — so the names here are needed only to report something a caller
/// recognises, while the formatting is needed to resolve what a run actually looks like.
/// </para>
/// <para>
/// Style inheritance uses the same machinery as everything else: a style's grpprl is applied over
/// its parent's, which is applied over the document defaults. That is the same resolve-through-a-
/// chain shape as ODF's parent styles and DOCX's <c>w:basedOn</c>
/// (<c>research/02-writer.md</c> section C.2, point 5).
/// </para>
/// </remarks>
public sealed class Ww8StyleSheet
{
    /// <summary>
    /// How deep a base-style chain is followed before it is treated as circular.
    /// </summary>
    public const int MaxBaseChainDepth = 32;

    private readonly List<Ww8Style> _styles = [];

    /// <summary>The styles, in index order.</summary>
    public IReadOnlyList<Ww8Style> Styles => _styles;

    /// <summary>
    /// The font a style that names none of its own is set in: <c>Stshi.ftcAsci</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A WW8 document's default font is not in any style's CHPX and not in the DOP; it is a
    /// bare font-table index in the stylesheet's own header.</strong> <c>WW8Style</c> reads it as
    /// <c>m_ftcAsci</c>, twelve bytes into the <c>Stshi</c> and only when the header is long enough
    /// to hold it (<c>sw/source/filter/ww8/ww8scan.cxx</c>:6919-6921), and
    /// <c>WW8RStyle::Set1StyleDefaults</c> then applies it to every paragraph style that is based on
    /// nothing and set no font of its own — <c>if (!mbFontChanged) SetNewFontAttr(m_ftcAsci, true,
    /// RES_CHRATR_FONT)</c> (<c>ww8par2.cxx</c>:3724-3725, called from <c>PostStyle</c> at :3862).
    /// </para>
    /// <para>
    /// Word 2007 and later write .doc that way whenever the document's default is not the first
    /// entry of the font table. <c>AAC-AD-No-2021-01-Boeing-737-8-and-737-9-MAX.doc</c> is the
    /// corpus's plainest case: <c>Normal</c>'s CHPX is
    /// <c>5f48 0104 6d48 0904 7348 0904 7448 0904</c> and holds no <c>sprmCRgFtc0</c> at all, while
    /// <c>rgftcStandardChpStsh</c> is <c>(4, 4, 4)</c> and font 4 is <c>Calibri</c>. Without this
    /// every run that states no font of its own falls to the resolver's own default, so the body of
    /// the document is set in Liberation Serif where 26.2.4.2 sets it in Carlito — which is not a
    /// metric-compatible pair, so it reflows.
    /// </para>
    /// <para>
    /// Nought when the header is too short to state one, which is <c>WW8Style</c>'s own initial value
    /// and means the font table's first entry.
    /// </para>
    /// </remarks>
    public ushort DefaultFontIndex { get; private set; }

    /// <summary>An empty stylesheet, for a document that declares none.</summary>
    public static Ww8StyleSheet Empty { get; } = new();

    /// <summary>
    /// Parses the STSH.
    /// </summary>
    /// <remarks>
    /// The header states the size of a style's fixed part, and later Word versions made it bigger —
    /// so the offset of a style's name is read from the file rather than assumed, which is what lets
    /// one parser handle Word 97 through Word 2007.
    /// </remarks>
    public static Ww8StyleSheet Parse(ReadOnlySpan<byte> stsh)
    {
        Ww8StyleSheet sheet = new();
        if (stsh.Length < 4) return sheet;

        int headerLength = BinaryPrimitives.ReadUInt16LittleEndian(stsh);
        if (headerLength < 8 || headerLength > stsh.Length) return sheet;

        // Inside the header: the count of styles, and how big each style's fixed part is.
        ushort styleCount = BinaryPrimitives.ReadUInt16LittleEndian(stsh[2..]);
        ushort fixedPartSize = BinaryPrimitives.ReadUInt16LittleEndian(stsh[4..]);
        if (fixedPartSize is < 8 or > 64) fixedPartSize = 10;

        // `ftcAsci` sits twelve bytes into the Stshi, behind `cstd`, `cbSTDBaseInFile`, the flags,
        // `stiMaxWhenSaved`, `istdMaxFixedWhenSaved` and `nVerBuiltInNamesWhenSaved` -- six words.
        // Read only when the header declares it, exactly as `WW8Style` does.
        if (headerLength >= 14 && stsh.Length >= 16)
        {
            sheet.DefaultFontIndex = BinaryPrimitives.ReadUInt16LittleEndian(stsh[14..]);
        }

        int position = 2 + headerLength;
        for (int i = 0; i < styleCount && position + 2 <= stsh.Length; i++)
        {
            int definitionLength = BinaryPrimitives.ReadUInt16LittleEndian(stsh[position..]);
            position += 2;

            if (definitionLength == 0)
            {
                // An empty slot: the index is reserved but unused. It still occupies an index, so a
                // placeholder keeps every later style's index right.
                sheet._styles.Add(new Ww8Style(string.Empty, Ww8Style.NoBaseStyle, 0, default, default));
                continue;
            }

            if (position + definitionLength > stsh.Length) break;

            sheet._styles.Add(ReadStyle(stsh.Slice(position, definitionLength), fixedPartSize));
            position += definitionLength;

            // Definitions are word-aligned.
            if ((definitionLength & 1) != 0) position++;
        }

        return sheet;
    }

    /// <summary>The style at an index, or null when there is none.</summary>
    public Ww8Style? At(int index)
        => index >= 0 && index < _styles.Count ? _styles[index] : null;

    /// <summary>
    /// A style's name, or null when the index names nothing.
    /// </summary>
    public string? NameOf(int index)
        => At(index) is { Name.Length: > 0 } style ? style.Name : null;

    /// <summary>
    /// The paragraph formatting a style and its ancestors set, outermost first.
    /// </summary>
    public List<ReadOnlyMemory<byte>> ResolveChain(int index) => Chain(index, character: false);

    /// <summary>
    /// The character formatting a style and its ancestors set, outermost first.
    /// </summary>
    /// <remarks>
    /// Separate from <see cref="ResolveChain"/> because a paragraph style carries both halves and the
    /// two must not be mixed: a paragraph sprm id and a character sprm id can collide numerically, so
    /// applying a PAPX to a character format sets whatever property happens to share the number.
    /// </remarks>
    public List<ReadOnlyMemory<byte>> ResolveCharacterChain(int index) => Chain(index, character: true);

    /// <summary>
    /// Walks a base-style chain, collecting one half of each style's formatting.
    /// </summary>
    /// <remarks>
    /// Outermost first so a caller can apply them in order and let the nearest win, which is the same
    /// shape the other formats' resolvers use. Cycle-guarded, because a base-style loop is malformed
    /// but does occur and this walks a chain over untrusted input.
    /// </remarks>
    private List<ReadOnlyMemory<byte>> Chain(int index, bool character)
    {
        List<ReadOnlyMemory<byte>> chain = [];
        HashSet<int> visited = [];
        bool rootless = false;

        int current = index;
        for (int depth = 0; depth < MaxBaseChainDepth; depth++)
        {
            if (current < 0 || current >= _styles.Count || !visited.Add(current)) break;

            Ww8Style style = _styles[current];
            ReadOnlyMemory<byte> half = character ? style.CharacterProperties : style.Properties;
            if (!half.IsEmpty) chain.Add(half);

            // The chain's root is where `Set1StyleDefaults` fires, and its condition is
            // `rSI.m_nBase >= m_cstd` -- a base index the stylesheet has no style for, of which the
            // `NoBaseStyle` sentinel is the ordinary case and an out-of-range one the malformed
            // case that reaches the same arm.
            if (style.BaseIndex == Ww8Style.NoBaseStyle || style.BaseIndex >= _styles.Count)
            {
                rootless = style.IsParagraphStyle;
                break;
            }

            current = style.BaseIndex;
        }

        chain.Reverse();

        // The document's default font, behind everything the chain itself says. Prepending rather
        // than testing whether the chain set a font is what makes it a *default*: a style that does
        // state one states it later in the list and wins, which is `mbFontChanged` expressed as
        // ordering. See `DefaultFontIndex`.
        if (character && rootless) chain.Insert(0, DefaultFontGrpprl());

        return chain;
    }

    /// <summary>A one-sprm grpprl setting <c>sprmCRgFtc0</c> to the document's default font.</summary>
    /// <remarks>
    /// Synthesised rather than plumbed as a separate value so that it travels the one path every
    /// other character property travels — <c>ApplyLayoutSprms</c> — and so that anything that later
    /// changes how a font code is read changes this with it. Cached, because it is the same four
    /// bytes for every chain in a document.
    /// </remarks>
    private ReadOnlyMemory<byte> DefaultFontGrpprl()
    {
        if (_defaultFontGrpprl.IsEmpty)
        {
            byte[] grpprl = [0x4F, 0x4A, (byte)(DefaultFontIndex & 0xFF), (byte)(DefaultFontIndex >> 8)];
            _defaultFontGrpprl = grpprl;
        }

        return _defaultFontGrpprl;
    }

    private ReadOnlyMemory<byte> _defaultFontGrpprl;

    private static Ww8Style ReadStyle(ReadOnlySpan<byte> definition, int fixedPartSize)
    {
        if (definition.Length < 4) return new Ww8Style(string.Empty, Ww8Style.NoBaseStyle, 0, default, default);

        // The first 16-bit field is the built-in identity in its low twelve bits, with four flags above
        // it — `fScratch`, `fInvalHeight`, `fHasUpe` and `fMassCopy`, none of which a reader wants.
        ushort sti = (ushort)(BinaryPrimitives.ReadUInt16LittleEndian(definition) & 0x0FFF);

        // The second 16-bit field packs the kind into its low nibble and the base index into the
        // rest, which is why neither can be read as a whole byte.
        ushort kindAndBase = BinaryPrimitives.ReadUInt16LittleEndian(definition[2..]);
        byte kind = (byte)(kindAndBase & 0x000F);
        ushort baseIndex = (ushort)((kindAndBase >> 4) & 0x0FFF);

        int position = Math.Min(fixedPartSize, definition.Length);
        string name = ReadName(definition, ref position);

        // What follows the name is the style's own formatting, as one or two length-prefixed UPXs.
        // A paragraph style has both halves — a PAPX whose first two bytes are its own index, then a
        // bare CHPX — while a character style has only the CHPX.
        ReadOnlyMemory<byte> first = ReadUpx(definition, ref position);
        ReadOnlyMemory<byte> second = ReadUpx(definition, ref position);

        return kind == 1
            // The PAPX's leading istd is not a sprm; passing it to the walker would desynchronise it.
            ? new Ww8Style(name, baseIndex, kind, first.Length >= 2 ? first[2..] : default, second, sti)
            : new Ww8Style(name, baseIndex, kind, default, first, sti);
    }

    /// <summary>
    /// Reads one length-prefixed UPX, advancing past its word alignment padding.
    /// </summary>
    /// <remarks>
    /// The padding is why the two UPXs cannot be found by adding their declared lengths: an odd-length
    /// first half is followed by a filler byte, and skipping it reads the second half's length from
    /// the middle of a sprm.
    /// </remarks>
    private static ReadOnlyMemory<byte> ReadUpx(ReadOnlySpan<byte> definition, ref int position)
    {
        if (position + 2 > definition.Length)
        {
            position = definition.Length;
            return default;
        }

        int length = BinaryPrimitives.ReadUInt16LittleEndian(definition[position..]);
        position += 2;

        if (length <= 0 || position + length > definition.Length)
        {
            position = Math.Min(definition.Length, position + Math.Max(0, length));
            return default;
        }

        ReadOnlyMemory<byte> upx = definition.Slice(position, length).ToArray();
        position += length;
        if ((length & 1) != 0) position++;
        return upx;
    }

    /// <summary>
    /// Reads a style's name, which is a length-prefixed UTF-16 string followed by a terminator.
    /// </summary>
    /// <remarks>
    /// The count is two bytes, not one, and the string is UTF-16 — so the byte count is twice the
    /// character count. Reading the count as a single byte leaves the walk one byte out of step, which
    /// does not fail: every character of the name comes back shifted by eight bits, so "Heading 1"
    /// arrives as CJK, and the formatting that follows the name is read from the wrong offset too.
    /// The name is followed by a null character that is not part of it.
    /// </remarks>
    private static string ReadName(ReadOnlySpan<byte> definition, ref int position)
    {
        if (position + 2 > definition.Length)
        {
            position = definition.Length;
            return string.Empty;
        }

        int characters = BinaryPrimitives.ReadUInt16LittleEndian(definition[position..]);
        position += 2;

        int bytes = characters * 2;
        if (bytes < 0 || position + bytes > definition.Length)
        {
            position = definition.Length;
            return string.Empty;
        }

        string name = Encoding.Unicode.GetString(definition.Slice(position, bytes));
        position += bytes;

        // Skip the terminating null character.
        position += 2;

        return name;
    }
}
