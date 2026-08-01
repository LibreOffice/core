using System.Buffers.Binary;
using System.Numerics;
using Paperless.Core.Graphics;

namespace Paperless.Vector.Metafiles;

/// <summary>
/// Reads the device-independent bitmaps a metafile's raster records carry.
/// </summary>
/// <remarks>
/// <para>
/// <b>The ordinary path decodes no pixel.</b> A DIB is a Windows BMP without its fourteen-byte
/// file header: the header is the only thing a BMP has that a DIB does not. So the whole job is
/// to measure the DIB — how long the header is, how big the colour table is, where the bits
/// start — and hand back the same bytes with a file header in front, as
/// <c>RasterImage.Encoded</c>. That is exactly what <c>RasterImage.Encoded</c> exists for, and
/// it keeps <c>Paperless.Vector</c> free of a codec, in the same way <c>Svg/</c> hands out
/// embedded PNGs undecoded.
/// </para>
/// <para>
/// It also means a malformed DIB costs nothing: the measurement is a dozen integer reads, and
/// the decoder that eventually looks at the pixels is the rasteriser's, which is hardened and
/// already handles everything else in the document.
/// </para>
/// <para>
/// <b><see cref="ReadPixels"/> is the one exception, and it needs no codec either.</b> Two
/// bitmaps have to be combined for the records that express transparency — a mask and its image,
/// or a DIB with a real alpha channel — and combining needs the values. An uncompressed DIB is
/// not an encoded format: <c>BI_RGB</c> and <c>BI_BITFIELDS</c> are a stride, a channel order
/// and a palette, so answering straight RGBA is arithmetic over a byte array rather than a
/// decode. The compressed forms — run-length, and a DIB wrapping a whole JPEG or PNG — answer
/// null, because those genuinely would need one.
/// </para>
/// </remarks>
public static class DeviceIndependentBitmap
{
    /// <summary>The size of a BMP file header, which is the only thing a DIB is missing.</summary>
    public const int FileHeaderSize = 14;

    private const int CoreHeaderSize = 12;
    private const int InfoHeaderSize = 40;
    private const uint CompressionRgb = 0;
    private const uint CompressionRle8 = 1;
    private const uint CompressionRle4 = 2;
    private const uint CompressionBitFields = 3;
    private const uint CompressionJpeg = 4;
    private const uint CompressionPng = 5;

    /// <summary>The largest side a bitmap may have, so a malformed header cannot ask for a terabyte.</summary>
    private const int MaxPixelSide = 1 << 16;

    /// <summary>The largest pixel count decoded for a merge: 64 megapixels, 256 MB of RGBA.</summary>
    private const long MaxPixelCount = 64L * 1024 * 1024;

    /// <summary>A DIB measured, and turned back into a stand-alone bitmap.</summary>
    /// <param name="Image">The bitmap, still encoded.</param>
    /// <param name="Width">Width in pixels.</param>
    /// <param name="Height">Height in pixels, always positive whichever way the rows ran.</param>
    /// <param name="Length">How many bytes of the input the DIB occupied.</param>
    public readonly record struct Result(RasterImage Image, int Width, int Height, int Length);

    /// <summary>
    /// Reads a DIB from the start of a span.
    /// </summary>
    /// <remarks>
    /// A DIB carrying a JPEG or PNG payload — legal since Windows 98, and what
    /// <c>emfio</c> refuses outright (<c>wmfreader.cxx:1027</c>) — is unwrapped and handed on as
    /// that format instead, since the payload is already a stand-alone image.
    /// </remarks>
    /// <param name="data">The bytes, beginning at the DIB header.</param>
    /// <returns>The bitmap, or null when the header is not one this can measure.</returns>
    public static Result? Read(ReadOnlySpan<byte> data)
    {
        if (data.Length < 4) return null;

        uint headerSize = BinaryPrimitives.ReadUInt32LittleEndian(data);

        int width;
        int height;
        int bitCount;
        uint compression = CompressionRgb;
        uint paletteEntries = 0;
        uint imageSize = 0;
        int paletteEntryBytes;

        if (headerSize == CoreHeaderSize)
        {
            if (data.Length < CoreHeaderSize) return null;

            width = BinaryPrimitives.ReadInt16LittleEndian(data[4..]);
            height = BinaryPrimitives.ReadInt16LittleEndian(data[6..]);
            bitCount = BinaryPrimitives.ReadUInt16LittleEndian(data[10..]);
            paletteEntryBytes = 3;
        }
        else if (headerSize is >= InfoHeaderSize and <= 124)
        {
            if (data.Length < headerSize) return null;

            width = BinaryPrimitives.ReadInt32LittleEndian(data[4..]);
            height = BinaryPrimitives.ReadInt32LittleEndian(data[8..]);
            bitCount = BinaryPrimitives.ReadUInt16LittleEndian(data[14..]);
            compression = BinaryPrimitives.ReadUInt32LittleEndian(data[16..]);
            imageSize = BinaryPrimitives.ReadUInt32LittleEndian(data[20..]);
            paletteEntries = BinaryPrimitives.ReadUInt32LittleEndian(data[32..]);
            paletteEntryBytes = 4;
        }
        else
        {
            return null;
        }

        if (width <= 0 || height == 0 || width > 1 << 16 || Math.Abs((long)height) > 1 << 16) return null;

        int rows = Math.Abs(height);

        // A DIB may wrap a whole JPEG or PNG, in which case the bits are that file and nothing
        // about the DIB's own geometry applies to them.
        if (compression is CompressionJpeg or CompressionPng)
        {
            int payloadStart = (int)headerSize;
            if (imageSize == 0 || payloadStart + imageSize > (uint)data.Length) return null;

            string type = compression == CompressionJpeg ? "image/jpeg" : "image/png";
            byte[] payload = data.Slice(payloadStart, (int)imageSize).ToArray();
            return new Result(RasterImage.Encoded(payload, type), width, rows, payloadStart + (int)imageSize);
        }

        if (bitCount is not (1 or 2 or 4 or 8 or 16 or 24 or 32)) return null;

        // Bit-field masks sit between the header and the colour table, but only when the header
        // is too short to have declared them itself.
        int maskBytes = compression == CompressionBitFields && headerSize == InfoHeaderSize ? 12 : 0;

        if (paletteEntries == 0 && bitCount <= 8) paletteEntries = 1u << bitCount;
        if (paletteEntries > 1 << 16) return null;

        long paletteBytes = (long)paletteEntries * paletteEntryBytes;

        long stride = ((long)width * bitCount + 31) / 32 * 4;
        long bits = compression is CompressionRle4 or CompressionRle8 ? imageSize : stride * rows;

        // A run-length DIB that states no size is unreadable: the length is the only thing that
        // says where it ends.
        if (bits <= 0) return null;

        long offset = headerSize + maskBytes + paletteBytes;
        long total = offset + bits;

        if (total > data.Length)
        {
            // Real files truncate the last scan lines. Keeping what is there beats dropping the
            // picture, and every BMP decoder tolerates a short final row.
            bits = data.Length - offset;
            if (bits <= 0) return null;
            total = data.Length;
        }

        byte[] bmp = new byte[FileHeaderSize + total];
        bmp[0] = (byte)'B';
        bmp[1] = (byte)'M';
        BinaryPrimitives.WriteUInt32LittleEndian(bmp.AsSpan(2), (uint)bmp.Length);
        BinaryPrimitives.WriteUInt32LittleEndian(bmp.AsSpan(10), (uint)(FileHeaderSize + offset));
        data[..(int)total].CopyTo(bmp.AsSpan(FileHeaderSize));

        return new Result(RasterImage.Encoded(bmp, "image/bmp"), width, rows, (int)total);
    }

    /// <summary>
    /// Reads a DIB whose header and bits are in two places, as every EMF bitmap record states
    /// them.
    /// </summary>
    /// <remarks>
    /// <b>This is the one structural difference between the two formats' bitmaps.</b> A WMF
    /// carries the DIB whole and contiguous; an EMF states four offsets — <c>offBmiSrc</c>,
    /// <c>cbBmiSrc</c>, <c>offBitsSrc</c>, <c>cbBitsSrc</c> — relative to the start of the
    /// record, and the two pieces need be neither adjacent nor in order. Splicing them back
    /// together is what <c>emfio</c> does too (<c>emfreader.cxx:1804-1832</c>), and it is still
    /// no decode: the result is a BMP made of the file's own bytes with a fourteen-byte header
    /// in front.
    /// </remarks>
    /// <param name="header">The <c>BITMAPINFO</c>: header and colour table.</param>
    /// <param name="bits">The pixel data.</param>
    public static Result? ReadSplit(ReadOnlySpan<byte> header, ReadOnlySpan<byte> bits)
    {
        if (header.Length < 12) return null;

        byte[] joined = new byte[header.Length + bits.Length];
        header.CopyTo(joined);
        bits.CopyTo(joined.AsSpan(header.Length));

        // The header states how long it and its colour table are; if that disagrees with what
        // the record allotted, the bits are not where the concatenation puts them and the
        // picture would come out as noise. Reading the joined buffer catches it, because the
        // measurement is done from the header's own fields.
        return Read(joined);
    }

    /// <summary>
    /// Straight RGBA pixels from an uncompressed DIB, for the records that have to combine two
    /// bitmaps.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <b>This is the only place in <c>Paperless.Vector</c> that looks at a pixel, and it needs
    /// no codec to do it.</b> A DIB is not a compressed format: <c>BI_RGB</c> and
    /// <c>BI_BITFIELDS</c> are a row stride, a channel order and a palette, so turning one into
    /// the straight RGBA <c>RasterImage.Pixels</c> wants is arithmetic over a byte array. That
    /// is the same argument that let a DIB become <c>RasterImage.Encoded</c> for the price of a
    /// fourteen-byte header, carried one step further, and it is what makes the transparent
    /// bitmap idiom and <c>AlphaBlend</c> expressible without <c>Paperless.Vector</c> depending
    /// on <c>Paperless.Rendering</c>'s decoder.
    /// </para>
    /// <para>
    /// It answers null for the compressed forms — run-length, and a DIB wrapping a whole JPEG
    /// or PNG — because those genuinely do need a codec. Neither has ever been seen as the mask
    /// half of a blit pair, which is the case this exists for; the ordinary opaque blit still
    /// goes through undecoded.
    /// </para>
    /// </remarks>
    /// <param name="data">The bytes, beginning at the DIB header.</param>
    /// <param name="alphaFromUnusedByte">
    /// True to read the fourth byte of a 32-bit pixel as alpha. It is normally unused and must
    /// be taken as opaque — a great many producers leave it zero, and honouring it would make
    /// every such bitmap invisible. Only <c>EMR_ALPHABLEND</c> with <c>AC_SRC_ALPHA</c> says
    /// otherwise, and it says so in its own blend function rather than in the DIB header.
    /// </param>
    /// <returns>The pixels and their dimensions, or null when the DIB is compressed.</returns>
    public static Pixels? ReadPixels(ReadOnlySpan<byte> data, bool alphaFromUnusedByte = false)
    {
        if (data.Length < 12) return null;

        uint headerSize = BinaryPrimitives.ReadUInt32LittleEndian(data);

        int width;
        int height;
        int bitCount;
        uint compression = CompressionRgb;
        uint paletteEntries = 0;
        int paletteEntryBytes;

        if (headerSize == CoreHeaderSize)
        {
            width = BinaryPrimitives.ReadInt16LittleEndian(data[4..]);
            height = BinaryPrimitives.ReadInt16LittleEndian(data[6..]);
            bitCount = BinaryPrimitives.ReadUInt16LittleEndian(data[10..]);
            paletteEntryBytes = 3;
        }
        else if (headerSize is >= InfoHeaderSize and <= 124 && data.Length >= headerSize)
        {
            width = BinaryPrimitives.ReadInt32LittleEndian(data[4..]);
            height = BinaryPrimitives.ReadInt32LittleEndian(data[8..]);
            bitCount = BinaryPrimitives.ReadUInt16LittleEndian(data[14..]);
            compression = BinaryPrimitives.ReadUInt32LittleEndian(data[16..]);
            paletteEntries = BinaryPrimitives.ReadUInt32LittleEndian(data[32..]);
            paletteEntryBytes = 4;
        }
        else
        {
            return null;
        }

        if (compression is not (CompressionRgb or CompressionBitFields)) return null;
        if (bitCount is not (1 or 2 or 4 or 8 or 16 or 24 or 32)) return null;
        if (width <= 0 || height == 0 || width > MaxPixelSide || Math.Abs((long)height) > MaxPixelSide) return null;

        int rows = Math.Abs(height);
        if ((long)width * rows > MaxPixelCount) return null;

        // A negative height means the rows run top-down; the usual positive one means the first
        // row in the file is the bottom row of the picture.
        bool bottomUp = height > 0;

        int maskBytes = compression == CompressionBitFields && headerSize == InfoHeaderSize ? 12 : 0;

        (uint red, uint green, uint blue, uint alpha) masks = Masks(data, headerSize, compression, bitCount);
        if (alphaFromUnusedByte && bitCount == 32 && masks.alpha == 0) masks.alpha = 0xFF000000;

        if (paletteEntries == 0 && bitCount <= 8) paletteEntries = 1u << bitCount;
        if (paletteEntries > 1 << 16) return null;

        long paletteOffset = headerSize + maskBytes;
        long bitsOffset = paletteOffset + ((long)paletteEntries * paletteEntryBytes);
        long stride = (((long)width * bitCount) + 31) / 32 * 4;

        if (bitsOffset > data.Length) return null;

        byte[] rgba = new byte[width * rows * 4];

        for (int y = 0; y < rows; y++)
        {
            long rowStart = bitsOffset + ((long)y * stride);
            int destinationRow = bottomUp ? rows - 1 - y : y;

            if (rowStart + stride > data.Length) break;

            ReadOnlySpan<byte> row = data.Slice((int)rowStart, (int)stride);
            Span<byte> destination = rgba.AsSpan(destinationRow * width * 4, width * 4);

            for (int x = 0; x < width; x++)
            {
                (byte r, byte g, byte b, byte a) = bitCount switch
                {
                    <= 8 => Indexed(row, x, bitCount, data, (int)paletteOffset, (int)paletteEntries, paletteEntryBytes),
                    16 => Packed(BinaryPrimitives.ReadUInt16LittleEndian(row[(x * 2)..]), masks),
                    24 => (row[(x * 3) + 2], row[(x * 3) + 1], row[x * 3], (byte)255),
                    _ => Packed(BinaryPrimitives.ReadUInt32LittleEndian(row[(x * 4)..]), masks),
                };

                destination[x * 4] = r;
                destination[(x * 4) + 1] = g;
                destination[(x * 4) + 2] = b;
                destination[(x * 4) + 3] = a;
            }
        }

        return new Pixels(rgba, width, rows);
    }

    /// <summary>An uncompressed DIB turned into straight RGBA.</summary>
    /// <param name="Rgba">Row-major, four bytes per pixel, no row padding, top row first.</param>
    /// <param name="Width">Width in pixels.</param>
    /// <param name="Height">Height in pixels.</param>
    public readonly record struct Pixels(byte[] Rgba, int Width, int Height)
    {
        /// <summary>The image as a decoded <see cref="RasterImage"/>.</summary>
        public RasterImage Image => new() { Width = Width, Height = Height, Pixels = Rgba };
    }

    /// <summary>
    /// Reads the 16-bit device-dependent bitmap that <c>META_CREATEPATTERNBRUSH</c> and the
    /// bitmap-carrying <c>BitBlt</c> records use.
    /// </summary>
    /// <remarks>
    /// A device-dependent bitmap has no colour table and no compression: it is a width, a
    /// height, a stride and the bits, in whatever the device's format was. Only the monochrome
    /// form is worth supporting — it is what pattern brushes are, and what
    /// <c>emfio</c>'s <c>CreateBitmap16</c> supports (<c>wmfreader.cxx:259-310</c>) — and it is
    /// turned into a two-colour BMP so that nothing here has to touch a pixel either.
    /// </remarks>
    /// <param name="data">The bytes, beginning at the bitmap structure.</param>
    /// <param name="headerBytes">
    /// How long the structure before the bits is: 10 for [MS-WMF] 2.2.2.1's Bitmap16, 32 for
    /// the legacy BITMAP struct plus padding that <c>META_CREATEPATTERNBRUSH</c> wraps it in.
    /// </param>
    /// <param name="foreground">The colour a set bit takes.</param>
    /// <param name="background">The colour a clear bit takes.</param>
    public static Result? ReadDeviceDependent(
        ReadOnlySpan<byte> data,
        int headerBytes,
        Colour foreground,
        Colour background)
    {
        if (data.Length < Math.Max(headerBytes, 10)) return null;

        int type = BinaryPrimitives.ReadInt16LittleEndian(data);
        int width = BinaryPrimitives.ReadInt16LittleEndian(data[2..]);
        int height = BinaryPrimitives.ReadInt16LittleEndian(data[4..]);
        int widthBytes = BinaryPrimitives.ReadInt16LittleEndian(data[6..]);
        int planes = data[8];
        int bitsPerPixel = data[9];

        if (type != 0 || planes != 1 || bitsPerPixel != 1) return null;
        if (width <= 0 || height <= 0 || widthBytes <= 0) return null;
        if (widthBytes < (width + 7) / 8) return null;

        long bits = (long)widthBytes * height;
        if (headerBytes + bits > data.Length) return null;

        // A DIB with a two-entry colour table, rows bottom-up as BMP wants them. The source rows
        // run top-down, so they are written in reverse.
        long stride = (width + 31) / 32 * 4;
        int offset = FileHeaderSize + InfoHeaderSize + 8;
        byte[] bmp = new byte[offset + (stride * height)];

        bmp[0] = (byte)'B';
        bmp[1] = (byte)'M';
        BinaryPrimitives.WriteUInt32LittleEndian(bmp.AsSpan(2), (uint)bmp.Length);
        BinaryPrimitives.WriteUInt32LittleEndian(bmp.AsSpan(10), (uint)offset);
        BinaryPrimitives.WriteUInt32LittleEndian(bmp.AsSpan(14), InfoHeaderSize);
        BinaryPrimitives.WriteInt32LittleEndian(bmp.AsSpan(18), width);
        BinaryPrimitives.WriteInt32LittleEndian(bmp.AsSpan(22), height);
        BinaryPrimitives.WriteUInt16LittleEndian(bmp.AsSpan(26), 1);
        BinaryPrimitives.WriteUInt16LittleEndian(bmp.AsSpan(28), 1);
        BinaryPrimitives.WriteUInt32LittleEndian(bmp.AsSpan(46), 2);

        // Bit 0 of a Windows monochrome bitmap is the background, bit 1 the foreground.
        WritePaletteEntry(bmp.AsSpan(FileHeaderSize + InfoHeaderSize), background);
        WritePaletteEntry(bmp.AsSpan(FileHeaderSize + InfoHeaderSize + 4), foreground);

        for (int y = 0; y < height; y++)
        {
            ReadOnlySpan<byte> source = data.Slice(headerBytes + (y * widthBytes), Math.Min(widthBytes, (int)stride));
            source.CopyTo(bmp.AsSpan(offset + (int)((height - 1 - y) * stride)));
        }

        return new Result(RasterImage.Encoded(bmp, "image/bmp"), width, height, headerBytes + (int)bits);
    }

    /// <summary>The channel masks in force, which only the bit-field forms state.</summary>
    private static (uint Red, uint Green, uint Blue, uint Alpha) Masks(
        ReadOnlySpan<byte> data,
        uint headerSize,
        uint compression,
        int bitCount)
    {
        if (compression == CompressionBitFields)
        {
            // A 40-byte header states the masks after itself; a longer one has fields for them.
            int at = headerSize == InfoHeaderSize ? (int)headerSize : 40;
            if (at + 12 <= data.Length)
            {
                uint red = BinaryPrimitives.ReadUInt32LittleEndian(data[at..]);
                uint green = BinaryPrimitives.ReadUInt32LittleEndian(data[(at + 4)..]);
                uint blue = BinaryPrimitives.ReadUInt32LittleEndian(data[(at + 8)..]);
                uint alpha = headerSize >= 56 && at + 16 <= data.Length
                    ? BinaryPrimitives.ReadUInt32LittleEndian(data[(at + 12)..])
                    : 0;

                if (red != 0 && green != 0 && blue != 0) return (red, green, blue, alpha);
            }
        }

        // The defaults GDI uses when no masks are stated: 5-5-5 at sixteen bits and 8-8-8 at
        // thirty-two, with the top byte unused rather than alpha. AlphaBlend is the one record
        // that says otherwise, and it says so in its own flag rather than in the DIB.
        return bitCount == 16
            ? (0x7C00u, 0x03E0u, 0x001Fu, 0u)
            : (0x00FF0000u, 0x0000FF00u, 0x000000FFu, 0u);
    }

    private static (byte R, byte G, byte B, byte A) Packed(uint value, (uint Red, uint Green, uint Blue, uint Alpha) masks)
        => (Channel(value, masks.Red), Channel(value, masks.Green), Channel(value, masks.Blue),
            masks.Alpha == 0 ? (byte)255 : Channel(value, masks.Alpha));

    /// <summary>One channel scaled to eight bits, whatever width the mask gave it.</summary>
    private static byte Channel(uint value, uint mask)
    {
        if (mask == 0) return 0;

        int shift = BitOperations.TrailingZeroCount(mask);
        uint field = (value & mask) >> shift;
        int bits = BitOperations.PopCount(mask);

        // Replicating the high bits rather than shifting left keeps white white: a 5-bit 31
        // must become 255, not 248.
        return bits >= 8
            ? (byte)(field >> (bits - 8))
            : (byte)((field * 255) / ((1u << bits) - 1));
    }

    private static (byte R, byte G, byte B, byte A) Indexed(
        ReadOnlySpan<byte> row,
        int x,
        int bitCount,
        ReadOnlySpan<byte> data,
        int paletteOffset,
        int paletteEntries,
        int paletteEntryBytes)
    {
        int perByte = 8 / bitCount;
        int index = (row[x / perByte] >> ((perByte - 1 - (x % perByte)) * bitCount)) & ((1 << bitCount) - 1);

        int at = paletteOffset + (index * paletteEntryBytes);
        if (index >= paletteEntries || at + 3 > data.Length) return (0, 0, 0, 255);

        return (data[at + 2], data[at + 1], data[at], 255);
    }

    private static void WritePaletteEntry(Span<byte> destination, Colour colour)
    {
        destination[0] = colour.B;
        destination[1] = colour.G;
        destination[2] = colour.R;
        destination[3] = 0;
    }
}
