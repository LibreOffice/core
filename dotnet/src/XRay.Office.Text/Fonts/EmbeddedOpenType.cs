using System.Buffers.Binary;
using System.Text;

namespace XRay.Office.Text.Fonts;

/// <summary>
/// A font as a document carries it: an Embedded OpenType (EOT) wrapper around an sfnt.
/// </summary>
/// <remarks>
/// <para>
/// This is what a <c>.pptx</c> stores in <c>ppt/fonts/*.fntdata</c> and points at from
/// <c>p:embeddedFontLst</c>. <strong>It is not an obfuscated TrueType file</strong>, and the two
/// are easy to confuse because both live under "embedded fonts" in OOXML. The obfuscated form is
/// WordprocessingML's: <c>w:embedRegular</c> carries a <c>w:fontKey</c> GUID and the first 32
/// bytes of the part are XORed with bytes derived from it. PresentationML carries no such key —
/// there is no GUID anywhere in <c>p:embeddedFontLst</c> — and its part is an EOT container,
/// byte for byte unaltered.
/// </para>
/// <para>
/// LibreOffice agrees, and the two paths meet in one function:
/// <c>oox::ppt::EmbeddedFontListContext</c> (<c>oox/source/ppt/EmbeddedFontListContext.cxx</c>)
/// calls <c>EmbeddedFontsManager::addEmbeddedFont(stream, typeface, style, key, eot, subsetted)</c>
/// with an <em>empty</em> <c>key</c> and <c>eot = true</c>, so the XOR loop at the top of that
/// function (<c>vcl/source/gdi/embeddedfontsmanager.cxx</c>) runs zero iterations for a deck and
/// is only ever exercised by the Word path, which passes the GUID and <c>eot = false</c>.
/// </para>
/// <para>
/// <strong>Compressed EOT is declined rather than decoded.</strong> The <c>TTEMBED_TTCOMPRESSED</c>
/// flag means the sfnt has been through MicroType Express, a whole codec that LibreOffice delegates
/// to <c>libeot</c> and that has no C# prior art. Measured over the slides track: of 28 embedded
/// font parts across 6 decks, 10 are uncompressed and 18 are MicroType-Express-compressed. A
/// compressed part is reported as read — <see cref="IsCompressed"/> — with no
/// <see cref="FontData"/>, so a caller can tell "a face this reader cannot decompress" from "not a
/// font at all" and fall back to substitution deliberately.
/// </para>
/// </remarks>
/// <param name="Version">
/// The container version, <c>0x00010000</c>, <c>0x00020001</c> or <c>0x00020002</c>. It decides
/// how many variable-length blocks the header carries, so it has to be read before the header can
/// be walked at all.
/// </param>
/// <param name="Flags">The <c>TTEMBED_*</c> bits, whole, so a caller can report an unknown one.</param>
/// <param name="Weight">The weight the wrapper declares, on the usual 1-1000 scale.</param>
/// <param name="IsItalic">Whether the wrapper declares the face italic.</param>
/// <param name="EmbeddingRights">
/// The <c>fsType</c> the wrapper repeats from the face's <c>OS/2</c> table. Read and reported, and
/// deliberately not acted on: LibreOffice defers a restricted face rather than dropping it, and
/// re-admits it when the same family turns out to be installed anyway
/// (<c>EmbeddedFontsManager::addEmbeddedFont</c>), which is a policy no document in the corpus
/// exercises — every embedded face the slides track actually draws with declares <c>0</c>.
/// </param>
/// <param name="FamilyName">The family name the wrapper declares, which need not match the face's own.</param>
/// <param name="StyleName">The style name the wrapper declares.</param>
/// <param name="FontData">
/// The sfnt inside, or empty when the container is compressed. Not itself validated as a font:
/// <see cref="OpenTypeFace.Read(byte[], int)"/> is the reader that decides that.
/// </param>
public readonly record struct EmbeddedOpenTypeFont(
    uint Version,
    uint Flags,
    int Weight,
    bool IsItalic,
    int EmbeddingRights,
    string? FamilyName,
    string? StyleName,
    ReadOnlyMemory<byte> FontData)
{
    /// <summary>The <c>MagicNumber</c> every EOT header carries, at a fixed offset.</summary>
    /// <remarks>
    /// The only cheap way to recognise the format. The first field is a length rather than a
    /// signature, so a container is identified by this plus the length agreeing with the part.
    /// </remarks>
    public const ushort Magic = 0x504C;

    /// <summary>The face inside has been subsetted to the glyphs the document uses.</summary>
    public const uint SubsetFlag = 0x0000_0001;

    /// <summary>The face inside is MicroType Express compressed.</summary>
    public const uint CompressedFlag = 0x0000_0004;

    /// <summary>The face inside is XOR-encrypted with <see cref="XorKey"/>.</summary>
    public const uint XorEncryptedFlag = 0x1000_0000;

    /// <summary>The one byte an <see cref="XorEncryptedFlag"/> container is masked with.</summary>
    public const byte XorKey = 0x50;

    /// <summary>The shortest header any version can have, used to reject a truncated part early.</summary>
    private const int MinimumHeaderLength = 82;

    /// <summary>True when the wrapper holds a face this reader cannot decompress.</summary>
    public bool IsCompressed => (Flags & CompressedFlag) != 0;

    /// <summary>True when the face inside covers only the glyphs the document uses.</summary>
    public bool IsSubset => (Flags & SubsetFlag) != 0;

    /// <summary>True when there is an sfnt to hand to <see cref="OpenTypeFace"/>.</summary>
    public bool HasFontData => !FontData.IsEmpty;

    /// <summary>
    /// True when the bytes open as an EOT container.
    /// </summary>
    /// <remarks>
    /// Cheaper than <see cref="Read"/> and used for the same job a magic number usually does:
    /// deciding whether a part is worth parsing. A part whose declared size disagrees with its
    /// actual length is rejected here, because that is the one thing a stray <c>0x504C</c> at
    /// offset 34 cannot fake.
    /// </remarks>
    public static bool Looks(ReadOnlySpan<byte> data)
        => data.Length >= MinimumHeaderLength
           && BinaryPrimitives.ReadUInt16LittleEndian(data[34..]) == Magic
           && BinaryPrimitives.ReadUInt32LittleEndian(data) == (uint)data.Length;

    /// <summary>
    /// Opens an EOT container, or null when the bytes are not one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The header is walked in full rather than skipped over, for two reasons. The block after the
    /// fixed fields is a run of length-prefixed UTF-16 strings whose count depends on the version,
    /// so its end cannot be computed without reading it; and the names it holds are the only
    /// statement of what the face is that does not require parsing the face. Validated against the
    /// whole slides track: the walk's end offset equals <c>EOTSize - FontDataSize</c> on 28 of 28
    /// parts, so the two independent ways of finding the font data agree everywhere it has been
    /// measured.
    /// </para>
    /// <para>
    /// Where they do <em>not</em> agree, the trailing slice wins and the names are dropped. A
    /// producer that miscounts a padding field costs a caller a diagnostic string; taking the walk's
    /// answer instead would cost it the font.
    /// </para>
    /// </remarks>
    /// <param name="data">The whole part.</param>
    public static EmbeddedOpenTypeFont? Read(ReadOnlyMemory<byte> data)
    {
        ReadOnlySpan<byte> span = data.Span;
        if (!Looks(span)) return null;

        uint fontDataSize = BinaryPrimitives.ReadUInt32LittleEndian(span[4..]);
        uint version = BinaryPrimitives.ReadUInt32LittleEndian(span[8..]);
        uint flags = BinaryPrimitives.ReadUInt32LittleEndian(span[12..]);

        if (fontDataSize > (uint)span.Length - MinimumHeaderLength) return null;

        int weight = (int)BinaryPrimitives.ReadUInt32LittleEndian(span[28..]);
        bool italic = span[27] != 0;
        int rights = BinaryPrimitives.ReadUInt16LittleEndian(span[32..]);

        int start = span.Length - (int)fontDataSize;
        Walk(span, version, start, out string? family, out string? style);

        // A compressed container is opened and reported rather than refused: the caller wants to
        // know the face was there and could not be used, which is a different fact from the part
        // being unreadable, and a different one again from the deck embedding nothing.
        ReadOnlyMemory<byte> font = (flags & CompressedFlag) != 0
            ? ReadOnlyMemory<byte>.Empty
            : Deciphered(data[start..], flags);

        return new EmbeddedOpenTypeFont(
            version, flags, weight, italic, rights, family, style, font);
    }

    /// <summary>Undoes the one-byte mask an <see cref="XorEncryptedFlag"/> container applies.</summary>
    /// <remarks>
    /// A copy rather than an in-place edit, because the input is a package part a caller may hold
    /// for other reasons. No document in the corpus sets the flag; it is implemented because the
    /// alternative to four lines here is a face that reads as garbage with no explanation.
    /// </remarks>
    private static ReadOnlyMemory<byte> Deciphered(ReadOnlyMemory<byte> font, uint flags)
    {
        if ((flags & XorEncryptedFlag) == 0) return font;

        byte[] plain = font.ToArray();
        for (int i = 0; i < plain.Length; i++) plain[i] ^= XorKey;

        return plain;
    }

    /// <summary>
    /// Walks the variable-length half of the header for the names, stopping at the first
    /// disagreement with where the font data actually starts.
    /// </summary>
    private static void Walk(
        ReadOnlySpan<byte> span, uint version, int fontDataStart, out string? family, out string? style)
    {
        family = null;
        style = null;

        // The fixed fields: through MagicNumber at 34, then four Unicode ranges, two code-page
        // ranges, the checksum adjustment and four reserved longs.
        int at = 36 + 16 + 8 + 4 + 16;

        string?[] names = new string?[4];
        for (int i = 0; i < 4; i++)
        {
            if (!Block(span, ref at, out ReadOnlySpan<byte> name)) return;
            names[i] = Encoding.Unicode.GetString(name).TrimEnd('\0');
        }

        // 0x00020001 adds the root string, and 0x00020002 a signature and an EUDC font after it.
        // Neither is read; both have to be stepped over to know the header ended where it should.
        if (version >= 0x0002_0001 && !Block(span, ref at, out _)) return;

        if (version >= 0x0002_0002)
        {
            at += 8;                                                  // root-string checksum, EUDC code page
            if (!Block(span, ref at, out _)) return;                  // signature
            at += 4;                                                  // EUDC flags
            if (at + 4 > span.Length) return;

            uint eudc = BinaryPrimitives.ReadUInt32LittleEndian(span[at..]);
            at += 4;
            if (eudc > (uint)(span.Length - at)) return;

            at += (int)eudc;
        }

        if (at != fontDataStart) return;

        family = names[0];
        style = names[1];
    }

    /// <summary>A padding word, a length word and that many bytes; false when it runs off the end.</summary>
    private static bool Block(ReadOnlySpan<byte> span, ref int at, out ReadOnlySpan<byte> value)
    {
        value = default;
        if (at + 4 > span.Length) return false;

        int length = BinaryPrimitives.ReadUInt16LittleEndian(span[(at + 2)..]);
        at += 4;
        if (length > span.Length - at) return false;

        value = span.Slice(at, length);
        at += length;
        return true;
    }
}
