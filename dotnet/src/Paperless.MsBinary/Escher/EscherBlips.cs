using System.Buffers.Binary;
using System.IO.Compression;
using Paperless.MsBinary.Records;

namespace Paperless.MsBinary.Escher;

/// <summary>
/// An Office Art blip store: the pictures a drawing's shapes index by number.
/// </summary>
/// <remarks>
/// <para>
/// Escher separates a picture's <em>bytes</em> from every reference to them. The bytes live once in an
/// <c>OfficeArtBStoreContainer</c> at the head of the drawing group, and a shape that wants one carries
/// a <c>pib</c> property holding its <strong>one-based</strong> position in that container. That is what
/// makes the same logo used on forty pages cost one copy, and it is also why a shape alone can never
/// answer what it is a picture of.
/// </para>
/// <para>
/// One <c>msofbtBSE</c> per entry: thirty-six bytes of bookkeeping, then <em>either</em> the blip
/// record inline <em>or</em> a <c>foDelay</c> offset into a stream of its own. Word uses the second, and
/// its delay stream is the <c>Data</c> stream — the same one <c>sprmCPicLocation</c> indexes for an
/// inline picture, which is why the two arrive together. LibreOffice wires the same pair up in
/// <c>SwMSDffManager</c>'s constructor (<c>ww8par.cxx</c>), which hands <c>SvxMSDffManager</c> the
/// reader's <c>m_pDataStream</c> as the BLIP stream.
/// </para>
/// <para>
/// <strong>The store belongs to Escher, not to any one front end.</strong> PowerPoint keeps the same
/// structure with a different delay stream — the compound file's <c>Pictures</c> stream — and Excel
/// keeps it inline in the workbook stream, so all three delegate here rather than each carrying its own
/// copy of the UID rule and the metafile inflate. <c>SvxMSDffManager</c> is the single class that serves
/// all three in LibreOffice for the same reason.
/// </para>
/// <para>
/// Measured on LibreOffice's own DOC export of <c>picture-flow.fodt</c>: the <c>msofbtBSE</c> in the
/// table stream is exactly thirty-six bytes with nothing after it, and the <c>Data</c> stream is a
/// hundred and eighteen — an eight-byte record header, a sixteen-byte checksum, one tag byte, and the
/// ninety-three-byte PNG. A reader that looked only inline would find a blip store with one entry and no
/// pictures in it, which is precisely what it looks like when it is wrong.
/// </para>
/// <para>
/// <strong>The trap is the UID count.</strong> A blip record begins with either one or two sixteen-byte
/// checksums and nothing in the record says which except the low bit of its instance field — odd means
/// two. Reading one where there are two puts sixteen bytes of MD4 in front of the picture, which for a
/// PNG shifts its signature past the sniffer and for a JPEG produces bytes that decode to noise. The
/// rule is <c>SvxMSDffManager::GetBLIPDirect</c>'s
/// (<c>filter/source/msfilter/msdffimp.cxx</c>), which reads <c>nUidBytes = (nInst &amp; 1) ? 32 : 16</c>
/// before anything else.
/// </para>
/// </remarks>
public static class EscherBlips
{
    /// <summary>The fixed part of an <c>msofbtBSE</c>, before any inline blip record.</summary>
    /// <remarks>
    /// Two byte-order-mark bytes, a sixteen-byte UID, a tag, the size, the reference count, the delay
    /// offset, the usage, the name length and two unused bytes — <c>MSOF_BSE</c> in MS-ODRAW §2.2.32.
    /// </remarks>
    private const int BlipStoreEntrySize = 36;

    /// <summary>The size of one checksum at the head of a blip record.</summary>
    private const int UidSize = 16;

    /// <summary>The <c>msofbtBlip</c> record types that hold a raster.</summary>
    private static bool IsRaster(ushort type) => type is 0xF01D or 0xF01E or 0xF01F or 0xF02A;

    /// <summary>The ones that hold a metafile: EMF, WMF and Macintosh PICT.</summary>
    private static bool IsVector(ushort type) => type is 0xF01A or 0xF01B or 0xF01C;

    /// <summary>
    /// The two whose bytes are worth extracting, which is the two Paperless has a decoder for.
    /// </summary>
    /// <remarks>
    /// PICT shares the metafile header and is left with no bytes on purpose: nothing here reads one,
    /// and a blip with no bytes keeps the named <c>PL2370</c> diagnostic — "a PICT picture was found"
    /// — where handing on bytes nothing can sniff would degrade it to "in no format this library
    /// recognises", which is true of every corrupt blob and says less.
    /// </remarks>
    private static bool IsReadableMetafile(ushort type) => type is 0xF01A or 0xF01B;

    /// <summary>
    /// The <c>OfficeArtMetafileHeader</c> that precedes a metafile blip's bytes.
    /// </summary>
    /// <remarks>
    /// Thirty-four bytes — <c>cbSize</c>, an <c>rcBounds</c>, a <c>ptSize</c> in EMUs, <c>cbSave</c>,
    /// and the compression and filter bytes — where a raster blip has a single one-byte tag. Getting
    /// the two confused is not a subtle failure: thirty-three bytes of header in front of a metafile
    /// puts a <c>D7 CD C6 9A</c> where nothing looks for one.
    /// </remarks>
    private const int MetafileHeaderSize = 34;

    /// <summary>Where the <c>compression</c> byte sits inside that header.</summary>
    private const int MetafileCompressionAt = 32;

    /// <summary>Whether a record type is a blip of any kind, raster or vector.</summary>
    /// <remarks>
    /// Asked before a record is believed to be the picture, because the alternative — trusting that
    /// whatever follows the entry's thirty-six bytes is a blip — reads the <em>next entry</em> as this
    /// one's picture whenever the store keeps its blips elsewhere.
    /// </remarks>
    private static bool IsBlipRecord(ushort type)
        => IsRaster(type) || IsVector(type) || type is 0xF029;

    /// <summary>
    /// Every picture in a drawing's blip store, in the order the store lists them.
    /// </summary>
    /// <remarks>
    /// Indexed from one on the way out, to match what a <c>pib</c> holds; index zero means "no picture"
    /// and is what a picture frame with no content carries.
    /// </remarks>
    /// <param name="officeArt">The bytes at <c>fcDggInfo</c>, whose first record is the drawing group.</param>
    /// <param name="delay">
    /// The <c>Data</c> stream: the first place a <c>foDelay</c> is looked for. Empty for a document
    /// without one.
    /// </param>
    /// <param name="fallback">
    /// The <c>WordDocument</c> stream, where the same offset is tried when the <c>Data</c> stream does
    /// not hold a blip there. Both are needed and neither is redundant — LibreOffice passes the pair to
    /// <c>SvxMSDffManager</c> as <c>pStData</c> and <c>pStData2</c> and tries them in that order
    /// (<c>msdffimp.cxx:6465</c>, "there is a second chance"). Its own DOC export takes the second
    /// route: for <c>picture-flow.doc</c> the <c>foDelay</c> is 6717, the <c>Data</c> stream is 118
    /// bytes long, and the <c>WordDocument</c> stream is 6835 — the picture is the last thing in it.
    /// </param>
    public static Dictionary<int, EscherBlip> Read(byte[] officeArt, byte[] delay, byte[] fallback)
    {
        ArgumentNullException.ThrowIfNull(officeArt);
        ArgumentNullException.ThrowIfNull(delay);
        ArgumentNullException.ThrowIfNull(fallback);

        if (officeArt.Length < DffRecordHeader.HeaderSize) return [];

        DffRecordBuffer buffer = new(officeArt);

        return buffer.TryReadHeader(0, out DffRecordHeader group)
               && group.Type == EscherRecordTypes.DrawingGroupContainer
            ? Read(buffer, group, delay, fallback)
            : [];
    }

    /// <summary>
    /// The same, for a drawing group already located inside a larger stream.
    /// </summary>
    /// <remarks>
    /// PowerPoint keeps its group inside a <c>PPDrawingGroup</c> record rather than at the head of a
    /// blob of its own, so it arrives as a header into the document stream. Taking the pair rather
    /// than a copied byte array also keeps a <c>foDelay</c> honest: the offset is into the delay
    /// stream and is unaffected by where the group sits, but slicing the group out and re-reading it
    /// from zero is the sort of rebasing that invites the mistake.
    /// </remarks>
    /// <param name="buffer">The stream the group was found in.</param>
    /// <param name="group">The <c>msofbtDggContainer</c>.</param>
    /// <param name="delay">The blip delay stream — <c>Data</c> for a DOC, <c>Pictures</c> for a PPT.</param>
    /// <param name="fallback">A second stream to try the same offset in; empty when there is none.</param>
    public static Dictionary<int, EscherBlip> Read(
        DffRecordBuffer buffer, DffRecordHeader group, byte[] delay, byte[] fallback)
    {
        ArgumentNullException.ThrowIfNull(buffer);
        ArgumentNullException.ThrowIfNull(delay);
        ArgumentNullException.ThrowIfNull(fallback);

        Dictionary<int, EscherBlip> blips = [];

        foreach (DffRecordHeader child in buffer.Children(group))
        {
            if (child.Type != EscherRecordTypes.BlipStoreContainer) continue;

            int index = 1;

            foreach (DffRecordHeader entry in buffer.Children(child))
            {
                if (entry.Type != EscherRecordTypes.BlipStoreEntry) continue;

                if (Blip(buffer, entry, delay, fallback) is { } blip) blips[index] = blip;
                index++;
            }
        }

        return blips;
    }

    /// <summary>
    /// The picture in a lone <c>msofbtBSE</c> at an offset, which is how Word stores an inline one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The document-wide blip store is not the only place a picture can be. An inline picture's shape
    /// container is followed, in the same stream, by a blip store entry of its own holding just that
    /// picture — and the drawing group then has no <c>OfficeArtBStoreContainer</c> at all, so the
    /// <c>pib</c> the shape carries indexes nothing. Measured on <c>picture-anchor.doc</c>: its drawing
    /// group holds an <c>msofbtDgg</c> and one <c>0xF11E</c> and no <c>0xF001</c>, while the shape's
    /// <c>pib</c> is 1.
    /// </para>
    /// <para>
    /// <c>SvxMSDffManager::ImportGraphic</c> does the same and says why in a comment worth keeping —
    /// "still no luck, let's look at the end of this record for a FBSE pool, this fallback is a
    /// specific case for how word does it sometimes" (<c>msdffimp.cxx:3859</c>). Its skip of
    /// <c>20 + 4 + 4 + 4 + 4</c> is the thirty-six bytes of the entry, written as the fields it steps
    /// over rather than as a total.
    /// </para>
    /// </remarks>
    /// <param name="buffer">The stream the shape was read from.</param>
    /// <param name="offset">One past the shape container, where the entry begins.</param>
    public static EscherBlip? Inline(DffRecordBuffer buffer, int offset)
    {
        ArgumentNullException.ThrowIfNull(buffer);

        if (!buffer.TryReadHeader(offset, out DffRecordHeader entry)) return null;
        if (entry.Type != EscherRecordTypes.BlipStoreEntry) return null;

        return Blip(buffer, entry, [], []);
    }

    /// <summary>The picture one <c>msofbtBSE</c> names, wherever it keeps it.</summary>
    /// <remarks>
    /// Inline first and the delay stream second, which is the order that copes with both: an entry with
    /// a record after its thirty-six bytes has the picture there, and one without has a
    /// <c>foDelay</c>. The offset is checked against the entry's own length rather than believed,
    /// because <c>0xFFFFFFFF</c> is how "nowhere" is written and reading at it would fault.
    /// </remarks>
    private static EscherBlip? Blip(
        DffRecordBuffer buffer, DffRecordHeader entry, byte[] delay, byte[] fallback)
    {
        int at = entry.ContentStart + BlipStoreEntrySize;
        DffRecordHeader blip;
        DffRecordBuffer source;

        if (entry.Length > BlipStoreEntrySize
            && buffer.TryReadHeader(at, out blip)
            && IsBlipRecord(blip.Type))
        {
            source = buffer;
        }
        else
        {
            ReadOnlySpan<byte> bse = buffer.Content(entry);
            if (bse.Length < BlipStoreEntrySize) return null;

            // Zero is a real offset, not "nowhere". LibreOffice reads a `foDelay` of zero as
            // "the picture is inside this entry" only when the entry is big enough to hold it —
            // `if ( (!nBLIPPos) && (nBLIPLen < nLenFBSE) )`, msdffimp.cxx:6084 — and the branch
            // above has already taken that case by finding the blip record inline. What is left
            // is a store whose first picture begins at offset zero of the delay stream, which is
            // where PowerPoint puts a deck's only picture: the `Pictures` stream starts with it.
            uint offset = BinaryPrimitives.ReadUInt32LittleEndian(bse[28..]);

            if (!At(delay, offset, out source, out blip) && !At(fallback, offset, out source, out blip))
            {
                return null;
            }
        }

        ReadOnlySpan<byte> content = source.Content(blip);

        // One checksum, or two when the instance is odd. Nothing else distinguishes them.
        int skip = (blip.Instance & 1) != 0 ? UidSize * 2 : UidSize;
        if (content.Length <= skip) return null;

        if (IsReadableMetafile(blip.Type)) return new EscherBlip(blip.Type, Metafile(content[skip..]));
        if (!IsRaster(blip.Type)) return new EscherBlip(blip.Type, default);

        // A one-byte tag follows the checksums on every raster blip. MS-ODRAW calls it `bTag` and gives
        // it no meaning; LibreOffice skips it the same way, and reading the picture from one byte early
        // costs a JPEG its `FF D8` and a PNG its signature byte.
        ReadOnlySpan<byte> data = content[(skip + 1)..];

        // A DIB blip is the *inside* of a `.bmp`: a BITMAPINFOHEADER, its palette and its pixels, with
        // the fourteen-byte file header the format's own signature lives in stripped off. Nothing can
        // sniff it and no decoder will take it, so the header is put back — which is exactly what
        // LibreOffice does in `SvxMSDffManager::GetBLIPDirect`, where a DIB is written into a stream
        // behind a synthesised `BITMAPFILEHEADER` before the graphic filter sees it.
        return blip.Type == 0xF01F
            ? new EscherBlip(blip.Type, WithFileHeader(data))
            : new EscherBlip(blip.Type, data.ToArray());
    }

    /// <summary>
    /// The metafile inside a blip, past its header and inflated where the header says it is deflated.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A metafile blip is compressed and a raster blip is not</strong>, which is the whole of
    /// why this exists. <c>SvxMSDffManager::GetBLIPDirect</c> sets <c>bZCodecCompression</c> for the
    /// EMF, WMF and PICT cases and for no other (<c>msdffimp.cxx:6518-6549</c>), and it does so
    /// <em>unconditionally</em> — it never reads the <c>compression</c> byte the header carries. The
    /// byte is honoured here because the format states it and <c>0xFE</c> legitimately means "stored";
    /// a stream that will not inflate falls back to its own bytes, so a producer that writes the byte
    /// wrongly still draws.
    /// </para>
    /// <para>
    /// Measured on LibreOffice's own DOC export of a WMF drawing: a 426-byte metafile arrives as 262
    /// bytes of deflate behind the thirty-four-byte header, so a reader that skipped the header and
    /// not the compression finds neither a placeable magic nor a <c>METAHEADER</c> and declines the
    /// picture as an unrecognised blob.
    /// </para>
    /// </remarks>
    private static ReadOnlyMemory<byte> Metafile(ReadOnlySpan<byte> content)
    {
        if (content.Length <= MetafileHeaderSize) return default;

        byte compression = content[MetafileCompressionAt];
        ReadOnlySpan<byte> data = content[MetafileHeaderSize..];

        // 0x00 is deflate and 0xFE is stored; nothing else is defined, and an undefined value is
        // treated as deflate because that is what every producer writes.
        if (compression == 0xFE) return data.ToArray();

        try
        {
            using MemoryStream source = new(data.ToArray(), writable: false);
            using ZLibStream inflate = new(source, CompressionMode.Decompress);
            using MemoryStream buffer = new();
            inflate.CopyTo(buffer);

            return buffer.Length == 0 ? data.ToArray() : buffer.ToArray();
        }
        catch (InvalidDataException)
        {
            // A blip whose bytes are not deflate after all. Leniency rule: hand on what is there and
            // let the sniffer decline it, rather than losing the picture and the diagnostic with it.
            return data.ToArray();
        }
    }

    /// <summary>The blip record at an offset in a stream, when that stream holds one there.</summary>
    private static bool At(byte[] stream, uint offset, out DffRecordBuffer buffer, out DffRecordHeader blip)
    {
        buffer = new DffRecordBuffer(stream);
        return buffer.TryReadHeader((int)Math.Min(offset, int.MaxValue), out blip)
               && IsBlipRecord(blip.Type);
    }

    /// <summary>
    /// A device-independent bitmap with the <c>BITMAPFILEHEADER</c> a <c>.bmp</c> begins with.
    /// </summary>
    /// <remarks>
    /// The only field that takes thought is the offset to the pixels, which is the two headers plus the
    /// palette — and the palette's size is not stated anywhere: it is <c>biClrUsed</c> entries, or
    /// <c>2 ^ biBitCount</c> when that is zero, and only for a bitmap of eight bits or fewer. A
    /// <c>BI_BITFIELDS</c> bitmap has three colour masks in the same place instead. Getting the offset
    /// wrong does not fail — it produces an image shifted by a few rows, which is far harder to spot.
    /// </remarks>
    private static byte[] WithFileHeader(ReadOnlySpan<byte> dib)
    {
        const int FileHeaderSize = 14;
        if (dib.Length < 40) return dib.ToArray();

        int headerSize = BinaryPrimitives.ReadInt32LittleEndian(dib);
        if (headerSize < 12 || headerSize > dib.Length) return dib.ToArray();

        int bits = BinaryPrimitives.ReadUInt16LittleEndian(dib[14..]);
        int compression = BinaryPrimitives.ReadInt32LittleEndian(dib[16..]);
        int used = BinaryPrimitives.ReadInt32LittleEndian(dib[32..]);

        int palette = bits <= 8 ? (used > 0 ? used : 1 << bits) * 4 : 0;
        if (compression == 3) palette += 12;

        byte[] bytes = new byte[FileHeaderSize + dib.Length];
        bytes[0] = (byte)'B';
        bytes[1] = (byte)'M';
        BinaryPrimitives.WriteInt32LittleEndian(bytes.AsSpan(2), bytes.Length);
        BinaryPrimitives.WriteInt32LittleEndian(
            bytes.AsSpan(10), FileHeaderSize + headerSize + palette);
        dib.CopyTo(bytes.AsSpan(FileHeaderSize));

        return bytes;
    }
}

/// <summary>
/// One entry of a blip store: what kind of picture it is, and its bytes when they are a raster.
/// </summary>
/// <param name="RecordType">
/// The <c>msofbtBlip</c> record type, which is the only honest statement of the picture's format any of
/// the four front ends gets — <c>0xF01E</c> for PNG, <c>0xF01D</c> for JPEG, <c>0xF01F</c> for a DIB,
/// <c>0xF01A</c> and <c>0xF01B</c> for EMF and WMF.
/// </param>
/// <param name="Bytes">
/// The picture, ready for a decoder, or empty for a blip whose bytes could not be reached. A metafile
/// arrives inflated and with its <c>OfficeArtMetafileHeader</c> already stripped, so what is here is
/// always a whole file of its own format — which is what lets one sniffer serve all four front ends.
/// </param>
public readonly record struct EscherBlip(ushort RecordType, ReadOnlyMemory<byte> Bytes)
{
    /// <summary>The document's own name for the format, for a diagnostic about one that will not draw.</summary>
    public string Kind => RecordType switch
    {
        0xF01A => "EMF",
        0xF01B => "WMF",
        0xF01C => "PICT",
        0xF01D or 0xF02A => "JPEG",
        0xF01E => "PNG",
        0xF01F => "DIB",
        0xF029 => "TIFF",
        _ => "unknown",
    };
}
