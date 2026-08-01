using System.Buffers.Binary;
using Paperless.Core.Graphics;
using Paperless.Vector.Metafiles;
using Shouldly;

namespace Paperless.Vector.Tests;

/// <summary>
/// The transparent bitmap idiom, in both formats, and the pixels it needs.
/// </summary>
/// <remarks>
/// <para>
/// <b>This is the open question the metafile work left, answered.</b> Neither WMF nor EMF has a
/// record that says "transparent". A producer says it by blitting a monochrome mask with
/// <c>SRCAND</c> and then the colour image with <c>SRCPAINT</c> to the same rectangle, and the
/// two together are one bitmap with an alpha channel. Before this, both were drawn in order and
/// the transparent area came out black.
/// </para>
/// <para>
/// Resolving it needs the values of the pixels — and an uncompressed DIB is not an encoded
/// format, so reading them is arithmetic rather than a decode. The ordinary opaque blit is
/// unaffected and still goes through as <c>RasterImage.Encoded</c>, which is what these tests
/// assert alongside the merge.
/// </para>
/// </remarks>
public class MetafileTransparencyTests
{
    private const int Mm = 100;

    [Fact]
    public void AnUncompressedDibIsReadableAsStraightRgbaWithoutACodec()
    {
        byte[] dib = Dib24(2, 1, [(0xFF, 0x00, 0x00), (0x00, 0x80, 0xFF)]);

        DeviceIndependentBitmap.Pixels pixels = DeviceIndependentBitmap.ReadPixels(dib).ShouldNotBeNull();

        pixels.Width.ShouldBe(2);
        pixels.Height.ShouldBe(1);
        pixels.Rgba.ShouldBe([0xFF, 0x00, 0x00, 0xFF, 0x00, 0x80, 0xFF, 0xFF]);
    }

    [Fact]
    public void ACompressedDibAnswersNothingRatherThanGuessing()
    {
        // Run-length and JPEG or PNG payloads genuinely do need a codec. They have never been
        // seen as the mask half of a blit pair, which is the case the pixel path exists for.
        byte[] dib = Dib24(2, 1, [(0, 0, 0), (0, 0, 0)]);
        BinaryPrimitives.WriteUInt32LittleEndian(dib.AsSpan(16), 1);        // BI_RLE8

        DeviceIndependentBitmap.ReadPixels(dib).ShouldBeNull();
    }

    [Fact]
    public void AThirtyTwoBitDibIsOpaqueUnlessTheRecordSaysOtherwise()
    {
        // The fourth byte is normally unused and a great many producers leave it zero. Reading
        // it as alpha by default would make every such bitmap invisible.
        byte[] dib = Dib32(1, 1, [(0x10, 0x20, 0x30, 0x00)]);

        DeviceIndependentBitmap.ReadPixels(dib).ShouldNotBeNull().Rgba[3].ShouldBe((byte)0xFF);
        DeviceIndependentBitmap.ReadPixels(dib, alphaFromUnusedByte: true).ShouldNotBeNull()
            .Rgba[3].ShouldBe((byte)0x00);
    }

    [Fact]
    public void AWmfMaskAndImageBecomeOneBitmapWithAnAlphaChannel()
    {
        const uint SourceAnd = 0x008800C6;
        const uint SourcePaint = 0x00EE0086;

        // White in the AND mask is where the destination shows through, because ANDing with
        // white leaves the destination alone. Getting that sense backwards makes the picture a
        // silhouette of itself.
        byte[] mask = Dib24(2, 1, [(0xFF, 0xFF, 0xFF), (0x00, 0x00, 0x00)]);
        byte[] image = Dib24(2, 1, [(0x00, 0x00, 0x00), (0x11, 0x22, 0x33)]);

        Recorder recorder = Draw(new WmfBuilder()
            .Raw(WmfFunction.StretchDib, StretchDib(mask, 2, 1, SourceAnd))
            .Raw(WmfFunction.StretchDib, StretchDib(image, 2, 1, SourcePaint)));

        RasterImage merged = recorder.Images.ShouldHaveSingleItem().Image;

        merged.IsDecoded.ShouldBeTrue();
        merged.Pixels.Span[3].ShouldBe((byte)0);
        merged.Pixels.Span[7].ShouldBe((byte)255);
        merged.Pixels.Span[4].ShouldBe((byte)0x11);
    }

    [Fact]
    public void TheOtherOrderOfTheIdiomIsRecognisedToo()
    {
        // i20085. The first record is still the mask and the second still the colour, but the
        // operations are the other way round — so the mask's sense inverts and white becomes
        // opaque rather than transparent.
        const uint SourceAnd = 0x008800C6;
        const uint SourcePaint = 0x00EE0086;

        byte[] mask = Dib24(2, 1, [(0xFF, 0xFF, 0xFF), (0x00, 0x00, 0x00)]);
        byte[] image = Dib24(2, 1, [(0x44, 0x55, 0x66), (0x77, 0x88, 0x99)]);

        Recorder recorder = Draw(new WmfBuilder()
            .Raw(WmfFunction.StretchDib, StretchDib(mask, 2, 1, SourcePaint))
            .Raw(WmfFunction.StretchDib, StretchDib(image, 2, 1, SourceAnd)));

        RasterImage merged = recorder.Images.ShouldHaveSingleItem().Image;

        merged.IsDecoded.ShouldBeTrue();
        merged.Pixels.Span[0].ShouldBe((byte)0x44);
        merged.Pixels.Span[3].ShouldBe((byte)255);
        merged.Pixels.Span[7].ShouldBe((byte)0);
    }

    [Fact]
    public void AnUnpairedMaskIsStillDrawnRatherThanSwallowed()
    {
        const uint SourceAnd = 0x008800C6;

        byte[] mask = Dib24(1, 1, [(0xFF, 0xFF, 0xFF)]);

        VectorImage image = VectorImages.Decode(new WmfBuilder()
            .Raw(WmfFunction.StretchDib, StretchDib(mask, 1, 1, SourceAnd))
            .Build());

        Recorder recorder = new();
        image.Draw(recorder, new Core.Geometry.DocRect(
            Core.Geometry.DocPoint.Origin, image.IntrinsicSize));

        // Deferring a blit must not lose it: the record stream ends and the pending one is drawn.
        recorder.Images.ShouldHaveSingleItem().Image.IsDecoded.ShouldBeFalse();
        image.Diagnostics.ShouldContain(d => d.Code == "PL6033");
    }

    [Fact]
    public void AnOpaqueBlitStillGoesThroughUndecoded()
    {
        byte[] dib = Dib24(2, 1, [(1, 2, 3), (4, 5, 6)]);

        Recorder recorder = Draw(new WmfBuilder()
            .Raw(WmfFunction.StretchDib, StretchDib(dib, 2, 1, 0x00CC0020)));

        // The common path costs nothing: no pixel is looked at, and the rasteriser's own decoder
        // is what eventually reads the bytes.
        recorder.Images.ShouldHaveSingleItem().Image.IsDecoded.ShouldBeFalse();
    }

    private static byte[] Dib24(int width, int height, (byte R, byte G, byte B)[] pixels)
    {
        int stride = ((width * 24) + 31) / 32 * 4;
        byte[] dib = new byte[40 + (stride * height)];

        BinaryPrimitives.WriteUInt32LittleEndian(dib, 40);
        BinaryPrimitives.WriteInt32LittleEndian(dib.AsSpan(4), width);
        BinaryPrimitives.WriteInt32LittleEndian(dib.AsSpan(8), height);
        BinaryPrimitives.WriteUInt16LittleEndian(dib.AsSpan(12), 1);
        BinaryPrimitives.WriteUInt16LittleEndian(dib.AsSpan(14), 24);

        // Rows run bottom-up, so the last row of the picture is the first in the file.
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                (byte r, byte g, byte b) = pixels[((height - 1 - y) * width) + x];
                int at = 40 + (y * stride) + (x * 3);
                dib[at] = b;
                dib[at + 1] = g;
                dib[at + 2] = r;
            }
        }

        return dib;
    }

    private static byte[] Dib32(int width, int height, (byte R, byte G, byte B, byte A)[] pixels)
    {
        byte[] dib = new byte[40 + (width * height * 4)];

        BinaryPrimitives.WriteUInt32LittleEndian(dib, 40);
        BinaryPrimitives.WriteInt32LittleEndian(dib.AsSpan(4), width);
        BinaryPrimitives.WriteInt32LittleEndian(dib.AsSpan(8), height);
        BinaryPrimitives.WriteUInt16LittleEndian(dib.AsSpan(12), 1);
        BinaryPrimitives.WriteUInt16LittleEndian(dib.AsSpan(14), 32);

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                (byte r, byte g, byte b, byte a) = pixels[((height - 1 - y) * width) + x];
                int at = 40 + (((y * width) + x) * 4);
                dib[at] = b;
                dib[at + 1] = g;
                dib[at + 2] = r;
                dib[at + 3] = a;
            }
        }

        return dib;
    }

    private static byte[] StretchDib(byte[] dib, short width, short height, uint rop)
    {
        byte[] payload = new byte[22 + dib.Length];

        BinaryPrimitives.WriteUInt32LittleEndian(payload, rop);
        BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(4), 0);
        BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(6), height);
        BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(8), width);
        BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(10), 0);
        BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(12), 0);
        BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(14), 10 * Mm);
        BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(16), 10 * Mm);
        BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(18), 0);
        BinaryPrimitives.WriteInt16LittleEndian(payload.AsSpan(20), 0);
        dib.CopyTo(payload.AsSpan(22));

        return payload;
    }

    private static Recorder Draw(WmfBuilder builder)
    {
        VectorImage image = VectorImages.Decode(builder.Build());

        Recorder recorder = new();
        image.Draw(recorder, new Core.Geometry.DocRect(
            Core.Geometry.DocPoint.Origin, image.IntrinsicSize));
        return recorder;
    }
}
