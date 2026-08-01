using System.Buffers.Binary;
using Paperless.Core.Graphics;

namespace Paperless.Vector.Metafiles;

/// <summary>
/// Reads the device-independent bitmaps a metafile's raster records carry.
/// </summary>
/// <remarks>
/// <para>
/// <b>Nothing here decodes a pixel.</b> A DIB is a Windows BMP without its fourteen-byte file
/// header: the header is the only thing a BMP has that a DIB does not. So the whole job is to
/// measure the DIB — how long the header is, how big the colour table is, where the bits start
/// — and hand back the same bytes with a file header in front, as
/// <c>RasterImage.Encoded</c>. That is exactly what <c>RasterImage.Encoded</c> exists for, and
/// it keeps <c>Paperless.Vector</c> free of a codec, in the same way <c>Svg/</c> hands out
/// embedded PNGs undecoded.
/// </para>
/// <para>
/// It also means a malformed DIB costs nothing: the measurement is a dozen integer reads, and
/// the decoder that eventually looks at the pixels is the rasteriser's, which is hardened and
/// already handles everything else in the document.
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

    private static void WritePaletteEntry(Span<byte> destination, Colour colour)
    {
        destination[0] = colour.B;
        destination[1] = colour.G;
        destination[2] = colour.R;
        destination[3] = 0;
    }
}
