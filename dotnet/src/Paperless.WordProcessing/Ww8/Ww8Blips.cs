using System.Buffers.Binary;
using Paperless.MsBinary.Escher;
using Paperless.MsBinary.Records;

namespace Paperless.WordProcessing.Ww8;

/// <summary>
/// A DOC's blip store: the pictures its shapes index by number.
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
internal static class Ww8Blips
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

    /// <summary>The ones that hold a vector picture this library cannot yet draw.</summary>
    private static bool IsVector(ushort type) => type is 0xF01A or 0xF01B or 0xF01C;

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
    public static Dictionary<int, Ww8Blip> Read(byte[] officeArt, byte[] delay, byte[] fallback)
    {
        ArgumentNullException.ThrowIfNull(officeArt);
        ArgumentNullException.ThrowIfNull(delay);
        ArgumentNullException.ThrowIfNull(fallback);

        Dictionary<int, Ww8Blip> blips = [];
        if (officeArt.Length < DffRecordHeader.HeaderSize) return blips;

        DffRecordBuffer buffer = new(officeArt);

        if (!buffer.TryReadHeader(0, out DffRecordHeader group)
            || group.Type != EscherRecordTypes.DrawingGroupContainer)
        {
            return blips;
        }

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
    public static Ww8Blip? Inline(DffRecordBuffer buffer, int offset)
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
    private static Ww8Blip? Blip(
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

            uint offset = BinaryPrimitives.ReadUInt32LittleEndian(bse[28..]);
            if (offset == 0) return null;

            if (!At(delay, offset, out source, out blip) && !At(fallback, offset, out source, out blip))
            {
                return null;
            }
        }

        ReadOnlySpan<byte> content = source.Content(blip);

        // One checksum, or two when the instance is odd. Nothing else distinguishes them.
        int skip = (blip.Instance & 1) != 0 ? UidSize * 2 : UidSize;
        if (content.Length <= skip) return null;

        if (IsVector(blip.Type)) return new Ww8Blip(blip.Type, default);
        if (!IsRaster(blip.Type)) return new Ww8Blip(blip.Type, default);

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
            ? new Ww8Blip(blip.Type, WithFileHeader(data))
            : new Ww8Blip(blip.Type, data.ToArray());
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
/// The picture, ready for a decoder, or empty for a blip whose format this library does not draw. Empty
/// is not the same as absent: the entry exists, the shape that indexes it is a picture, and the frame
/// reserves its room — see the diagnostic <see cref="Ww8DocumentReader"/> raises for one.
/// </param>
internal readonly record struct Ww8Blip(ushort RecordType, ReadOnlyMemory<byte> Bytes)
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
