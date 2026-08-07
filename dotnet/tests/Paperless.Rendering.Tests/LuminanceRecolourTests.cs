using Paperless.Core.Graphics;
using Paperless.Rendering.Images;
using Shouldly;

namespace Paperless.Rendering.Tests;

/// <summary>
/// A blip's brightness and contrast, against LibreOffice 24.2.7.2's own rendering of them.
/// </summary>
/// <remarks>
/// <para>
/// The expectations are not derived from the formula, for the same reason
/// <see cref="DuotoneFilterTests"/>'s are not: they are levels read out of <c>soffice</c>'s
/// rendering of <c>slide-picture-washout.pptx</c>, which draws one generated ramp four times —
/// untouched, at <c>bright="70000" contrast="-70000"</c>, at <c>bright="40000"</c> alone, and
/// at <c>bright="20000" contrast="20000"</c>. Each row below is one column of that page: what
/// the untouched picture came out as, and what each of the other three came out as beside it.
/// </para>
/// <para>
/// <strong>The three columns are three different arithmetics, and the first is not the one the
/// file asks for.</strong> 70 and −70 is PowerPoint's washout, which the reference maps to its
/// own watermark mode and applies as <em>+50 and −70</em>; a brightness alone goes through the
/// colour modifier, whose offset carries the contrast's compensation; both together are baked
/// with MSO's formula, which spends half the brightness either side of the contrast. See
/// <see cref="LuminanceRecolour"/> for the citations.
/// </para>
/// <para>
/// The samples are read after both writers have scaled the picture, which is sound because
/// every one of these transforms is affine: averaging neighbours commutes with it, so a
/// resampled input and a resampled output still lie on the curve. It stops being true once a
/// channel saturates, which is why the rows above 136 are only asserted where they clamp to
/// 255 on both sides.
/// </para>
/// </remarks>
public class LuminanceRecolourTests
{
    [Theory]
    // untouched   washout   bright 40   bright 20 + contrast 20
    [InlineData(8, 219, 110, 36)]
    [InlineData(34, 227, 136, 68)]
    [InlineData(65, 236, 167, 107)]
    [InlineData(101, 247, 203, 152)]
    [InlineData(136, 255, 238, 195)]
    [InlineData(172, 255, 255, 240)]
    [InlineData(208, 255, 255, 255)]
    [InlineData(243, 255, 255, 255)]
    public void AChannelLandsWhereTheReferencePutsIt(
        int source, int washout, int brightnessAlone, int both)
    {
        Level(source, 70, -70).ShouldBe(washout, "washout");
        Level(source, 40, 0).ShouldBe(brightnessAlone, "brightness alone");
        Level(source, 20, 20).ShouldBe(both, "both stated");
    }

    /// <summary>
    /// A pair that states nothing changes nothing, and keeps the picture's encoded bytes.
    /// </summary>
    /// <remarks>
    /// 32 of the corpus's blip <c>a:lum</c> elements state neither attribute
    /// (<c>research/probes/slides-r19/count-bliplum.py</c>), so this is the commonest case in
    /// the corpus rather than a degenerate one — and it must not cost the JPEG pass-through,
    /// which is what recolouring a picture spends.
    /// </remarks>
    [Fact]
    public void AnEmptyPairChangesNothingAndKeepsThePassThrough()
    {
        RasterImage image = Pixel(10, 20, 30) with { Luminance = new LuminanceRecolour(0, 0) };
        RasterImage same = RasterImageDecoder.Ensure(image).ShouldNotBeNull();

        same.Pixels.Span[0].ShouldBe((byte)10);
        same.Pixels.Span[1].ShouldBe((byte)20);
        same.Pixels.Span[2].ShouldBe((byte)30);
        same.EncodedBytes.Length.ShouldBe(3);
    }

    /// <summary>Alpha is a coverage, not a colour, and no channel map touches it.</summary>
    [Fact]
    public void AlphaSurvivesTheTransform()
    {
        RasterImage recoloured = RasterImageDecoder
            .Ensure(Pixel(10, 20, 30, alpha: 64) with { Luminance = new LuminanceRecolour(70, -70) })
            .ShouldNotBeNull();

        recoloured.Pixels.Span[3].ShouldBe((byte)64);
    }

    /// <summary>
    /// The recolouring survives the decode, which is where it was first lost.
    /// </summary>
    /// <remarks>
    /// <c>Ensure</c> builds a fresh image from the encoded bytes and copies across only what it
    /// is told to, so a reader's pending transform reaches the page only if it is named in that
    /// list. It was not, and the symptom was a picture that decoded perfectly and drew
    /// completely unrecoloured — on <em>every</em> picture, since one that is already decoded
    /// never takes this branch at all. Caught by rendering a deck rather than by reading the
    /// code, which is why it is asserted here.
    /// </remarks>
    [Fact]
    public void TheRecolouringSurvivesADecode()
    {
        RasterImage encoded = RasterImage.Encoded(OnePixelPng, "image/png") with
        {
            Luminance = new LuminanceRecolour(70, -70),
        };

        RasterImage decoded = RasterImageDecoder.Ensure(encoded).ShouldNotBeNull();

        decoded.IsDecoded.ShouldBeTrue();

        // 64 is 236 under the watermark pair; untransformed it would still be 64. Mid grey
        // would not do: 128 saturates to 255 there, which a white pixel also reaches.
        decoded.Pixels.Span[0].ShouldBe((byte)236);
        decoded.Luminance.ShouldBeNull();
        decoded.EncodedBytes.IsEmpty.ShouldBeTrue();
    }

    /// <summary>One channel through the decoder, at a stated brightness and contrast.</summary>
    private static int Level(int source, int brightness, int contrast)
    {
        RasterImage image = Pixel((byte)source, (byte)source, (byte)source) with
        {
            Luminance = new LuminanceRecolour(brightness, contrast),
        };

        return RasterImageDecoder.Ensure(image).ShouldNotBeNull().Pixels.Span[0];
    }

    private static RasterImage Pixel(byte red, byte green, byte blue, byte alpha = 255)
        => new()
        {
            Width = 1,
            Height = 1,
            Pixels = new[] { red, green, blue, alpha },
            EncodedBytes = new byte[] { 1, 2, 3 },
            EncodedMediaType = "image/jpeg",
        };

    /// <summary>A one-pixel dark-grey PNG, written out rather than decoded from a fixture.</summary>
    private static byte[] OnePixelPng => Png(64, 64, 64);

    private static byte[] Png(byte red, byte green, byte blue)
    {
        byte[] header = [0x89, (byte)'P', (byte)'N', (byte)'G', 0x0D, 0x0A, 0x1A, 0x0A];
        byte[] ihdr = [0, 0, 0, 1, 0, 0, 0, 1, 8, 2, 0, 0, 0];
        byte[] raw = [0, red, green, blue];

        using MemoryStream stream = new();
        stream.Write(header);
        WriteChunk(stream, "IHDR"u8, ihdr);
        WriteChunk(stream, "IDAT"u8, Deflate(raw));
        WriteChunk(stream, "IEND"u8, []);
        return stream.ToArray();
    }

    private static byte[] Deflate(byte[] data)
    {
        using MemoryStream stream = new();
        using (System.IO.Compression.ZLibStream zlib =
               new(stream, System.IO.Compression.CompressionLevel.Optimal, leaveOpen: true))
        {
            zlib.Write(data);
        }

        return stream.ToArray();
    }

    private static void WriteChunk(Stream stream, ReadOnlySpan<byte> kind, ReadOnlySpan<byte> body)
    {
        Span<byte> length = stackalloc byte[4];
        System.Buffers.Binary.BinaryPrimitives.WriteUInt32BigEndian(length, (uint)body.Length);
        stream.Write(length);
        stream.Write(kind);
        stream.Write(body);

        byte[] covered = [.. kind, .. body];
        Span<byte> crc = stackalloc byte[4];
        System.Buffers.Binary.BinaryPrimitives.WriteUInt32BigEndian(crc, Crc32(covered));
        stream.Write(crc);
    }

    /// <summary>PNG's CRC-32, written out because the framework's lives in a package.</summary>
    private static uint Crc32(ReadOnlySpan<byte> data)
    {
        uint crc = 0xFFFFFFFF;
        foreach (byte value in data)
        {
            crc ^= value;
            for (int bit = 0; bit < 8; bit++)
            {
                crc = (crc >> 1) ^ (0xEDB88320u & (uint)-(int)(crc & 1));
            }
        }

        return crc ^ 0xFFFFFFFF;
    }
}
