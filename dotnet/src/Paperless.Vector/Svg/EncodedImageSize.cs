using System.Buffers.Binary;

namespace Paperless.Vector.Svg;

/// <summary>
/// Reads a raster image's pixel dimensions out of its header, without decoding it.
/// </summary>
/// <remarks>
/// <para>
/// An <c>&lt;image&gt;</c> in an SVG has to be placed before it can be drawn, and placing it
/// needs its aspect ratio — <c>preserveAspectRatio</c> fits the picture inside the element's
/// box, so a wrong ratio puts it in the wrong place, not merely at the wrong size.
/// </para>
/// <para>
/// Reading it from the header rather than from a codec is the same decision
/// <c>RasterImage.Encoded</c> records: <c>Paperless.Vector</c> must not depend on the
/// rasteriser, or every reader that touches a picture would inherit SkiaSharp. Five header
/// layouts cover every format an office document embeds, and a format not recognised here
/// simply does not draw — which is the same outcome as a codec that cannot decode it.
/// </para>
/// </remarks>
internal static class EncodedImageSize
{
    /// <summary>The image's pixel dimensions, or null when the header is not recognised.</summary>
    public static (int Width, int Height)? Read(ReadOnlySpan<byte> data)
    {
        if (Png(data) is { } png) return png;
        if (Gif(data) is { } gif) return gif;
        if (Bmp(data) is { } bmp) return bmp;
        if (WebP(data) is { } webp) return webp;
        if (Jpeg(data) is { } jpeg) return jpeg;

        return null;
    }

    private static (int, int)? Png(ReadOnlySpan<byte> data)
    {
        // Spelled as bytes rather than as a u8 literal: the first byte is 0x89, which a u8
        // literal would encode as the two bytes UTF-8 uses for U+0089.
        ReadOnlySpan<byte> signature = [0x89, (byte)'P', (byte)'N', (byte)'G', 0x0D, 0x0A, 0x1A, 0x0A];

        if (data.Length < 24 || !data[..8].SequenceEqual(signature)) return null;
        if (!data[12..16].SequenceEqual("IHDR"u8)) return null;

        return (
            BinaryPrimitives.ReadInt32BigEndian(data[16..20]),
            BinaryPrimitives.ReadInt32BigEndian(data[20..24]));
    }

    private static (int, int)? Gif(ReadOnlySpan<byte> data)
    {
        if (data.Length < 10) return null;
        if (!data[..6].SequenceEqual("GIF87a"u8) && !data[..6].SequenceEqual("GIF89a"u8)) return null;

        return (
            BinaryPrimitives.ReadUInt16LittleEndian(data[6..8]),
            BinaryPrimitives.ReadUInt16LittleEndian(data[8..10]));
    }

    private static (int, int)? Bmp(ReadOnlySpan<byte> data)
    {
        if (data.Length < 26 || data[0] != 'B' || data[1] != 'M') return null;

        uint headerSize = BinaryPrimitives.ReadUInt32LittleEndian(data[14..18]);

        // The 12-byte BITMAPCOREHEADER states its extent as two signed 16-bit values; every
        // later header states it as two 32-bit ones, with a negative height meaning the rows
        // are stored top-down rather than bottom-up.
        if (headerSize == 12)
        {
            return (
                BinaryPrimitives.ReadInt16LittleEndian(data[18..20]),
                Math.Abs((int)BinaryPrimitives.ReadInt16LittleEndian(data[20..22])));
        }

        return (
            BinaryPrimitives.ReadInt32LittleEndian(data[18..22]),
            Math.Abs(BinaryPrimitives.ReadInt32LittleEndian(data[22..26])));
    }

    private static (int, int)? WebP(ReadOnlySpan<byte> data)
    {
        if (data.Length < 30 || !data[..4].SequenceEqual("RIFF"u8) || !data[8..12].SequenceEqual("WEBP"u8))
        {
            return null;
        }

        ReadOnlySpan<byte> chunk = data[12..16];

        if (chunk.SequenceEqual("VP8X"u8))
        {
            // The extended header stores canvas size minus one, as two 24-bit values.
            int width = data[24] | (data[25] << 8) | (data[26] << 16);
            int height = data[27] | (data[28] << 8) | (data[29] << 16);
            return (width + 1, height + 1);
        }

        if (chunk.SequenceEqual("VP8 "u8) && data.Length >= 30)
        {
            return (
                BinaryPrimitives.ReadUInt16LittleEndian(data[26..28]) & 0x3FFF,
                BinaryPrimitives.ReadUInt16LittleEndian(data[28..30]) & 0x3FFF);
        }

        if (chunk.SequenceEqual("VP8L"u8) && data.Length >= 25 && data[20] == 0x2F)
        {
            uint bits = BinaryPrimitives.ReadUInt32LittleEndian(data[21..25]);
            return ((int)((bits & 0x3FFF) + 1), (int)(((bits >> 14) & 0x3FFF) + 1));
        }

        return null;
    }

    private static (int, int)? Jpeg(ReadOnlySpan<byte> data)
    {
        if (data.Length < 4 || data[0] != 0xFF || data[1] != 0xD8) return null;

        int offset = 2;
        while (offset + 9 < data.Length)
        {
            if (data[offset] != 0xFF) { offset++; continue; }

            byte marker = data[offset + 1];
            offset += 2;

            // Standalone markers: padding, restart intervals and the two image delimiters
            // carry no length field, so stepping over a length here would desynchronise.
            if (marker == 0xFF || marker == 0x01 || (marker >= 0xD0 && marker <= 0xD9)) continue;
            if (offset + 1 >= data.Length) return null;

            int length = BinaryPrimitives.ReadUInt16BigEndian(data[offset..(offset + 2)]);
            if (length < 2) return null;

            // SOF0..SOF15, less the three markers that share that range but are not frame
            // headers: DHT (C4), DAC (CC) and the reserved JPG (C8).
            bool isFrameHeader = marker >= 0xC0 && marker <= 0xCF
                && marker != 0xC4 && marker != 0xC8 && marker != 0xCC;

            if (isFrameHeader && offset + 7 <= data.Length)
            {
                return (
                    BinaryPrimitives.ReadUInt16BigEndian(data[(offset + 5)..(offset + 7)]),
                    BinaryPrimitives.ReadUInt16BigEndian(data[(offset + 3)..(offset + 5)]));
            }

            offset += length;
        }

        return null;
    }
}
