using System.Buffers.Binary;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Shouldly;

namespace Paperless.Vector.Tests;

/// <summary>
/// EMF+ image attributes, which turned out to need no pixels at all.
/// </summary>
/// <remarks>
/// <para>
/// This gap was recorded as "a colour matrix, a gamma, a chroma key and a colour remap table,
/// all of which need the pixels of a JPEG or a PNG — the one remaining case that a codec here
/// would buy". That describes GDI+'s <c>ImageAttributes</c> API class. The object a metafile
/// carries is [MS-EMFPLUS] 2.2.1.5 and is twenty-four bytes: version, reserved, wrap mode, clamp
/// colour, object clamp, reserved. The colour adjustments are applied by the producer before the
/// bitmap is written, so they are already in the pixels and never in the file.
/// </para>
/// <para>
/// So what the attributes actually decide is the <em>edge</em>: what fills the part of a
/// destination whose source rectangle reaches outside the bitmap. These tests pin that, and pin
/// that an attributes slot no longer silently discards its object — which is what let the gap be
/// mis-described for as long as it was.
/// </para>
/// </remarks>
public class EmfPlusImageAttributeTests
{
    [Fact]
    public void AClampingWrapModePaintsItsColourWhereTheSourceRectangleLeavesTheBitmap()
    {
        Recorder recorder = Draw(Attributes(wrapMode: 4, clamp: 0xFF00FF00), (-4, -4, 16, 16));

        // The clamp colour lands under the image, in the image's own placement, so a destination
        // whose source rectangle overhangs is green outside the bitmap rather than transparent.
        recorder.Fills.ShouldHaveSingleItem();
        recorder.Fills[0].Paint.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(new Colour(0, 255, 0));
        recorder.Images.ShouldHaveSingleItem();
    }

    [Fact]
    public void ASourceRectangleInsideTheBitmapNeedsNoEdgeColourEvenWhenTheModeClamps()
    {
        Recorder recorder = Draw(Attributes(wrapMode: 4, clamp: 0xFF00FF00), (1, 1, 6, 6));

        recorder.Fills.ShouldBeEmpty();
        recorder.Images.ShouldHaveSingleItem();
    }

    [Fact]
    public void ATilingWrapModePaintsNoEdgeColourBecauseTilingIsNotAFlatColour()
    {
        // Every image-attributes object in the reference corpus states a tiling mode, and every
        // source rectangle that uses one overhangs by half a pixel. Painting the clamp colour
        // there — which is transparent black in all of them — would be a visible black frame
        // around a picture that GDI+ draws with its own edge pixels repeated.
        Recorder recorder = Draw(Attributes(wrapMode: 0, clamp: 0xFF00FF00), (-4, -4, 16, 16));

        recorder.Fills.ShouldBeEmpty();
        recorder.Images.ShouldHaveSingleItem();
    }

    private static byte[] Attributes(uint wrapMode, uint clamp)
    {
        byte[] data = new byte[24];
        BinaryPrimitives.WriteUInt32LittleEndian(data.AsSpan(0), 0xDBC01002);
        BinaryPrimitives.WriteUInt32LittleEndian(data.AsSpan(8), wrapMode);
        BinaryPrimitives.WriteUInt32LittleEndian(data.AsSpan(12), clamp);
        return data;
    }

    private static Recorder Draw(byte[] attributes, (float X, float Y, float W, float H) source)
    {
        VectorImage image = new EmfBuilder()
            .Plus(new EmfPlusBuilder()
                .Header()
                .Object(1, 0x500, Bitmap(8, 8))
                .Object(2, 0x800, attributes)
                .Raw(DrawImage(image: 1, attributes: 2, source, (0, 0, 40, 40)))
                .End())
            .Decode();

        Recorder sink = new();
        image.Draw(sink, new DocRect(DocPoint.Origin, image.IntrinsicSize));
        return sink;
    }

    /// <summary>A GDI+ native 32-bit bitmap, so nothing in the test needs a codec either.</summary>
    private static byte[] Bitmap(int width, int height)
    {
        byte[] data = new byte[24 + (width * height * 4)];

        BinaryPrimitives.WriteUInt32LittleEndian(data.AsSpan(0), 0xDBC01002);
        BinaryPrimitives.WriteInt32LittleEndian(data.AsSpan(4), 1);         // a bitmap
        BinaryPrimitives.WriteInt32LittleEndian(data.AsSpan(8), width);
        BinaryPrimitives.WriteInt32LittleEndian(data.AsSpan(12), height);
        BinaryPrimitives.WriteInt32LittleEndian(data.AsSpan(16), width * 4);
        BinaryPrimitives.WriteUInt32LittleEndian(data.AsSpan(20), 0x0026200A);   // 32bppARGB
        BinaryPrimitives.WriteUInt32LittleEndian(data.AsSpan(24), 0);            // pixels follow

        for (int i = 28; i < data.Length; i += 4)
        {
            data[i] = 0x40;
            data[i + 1] = 0x80;
            data[i + 2] = 0xC0;
            data[i + 3] = 0xFF;
        }

        return data;
    }

    private static byte[] DrawImage(
        int image,
        int attributes,
        (float X, float Y, float W, float H) source,
        (float X, float Y, float W, float H) destination)
    {
        byte[] data = new byte[40];

        BinaryPrimitives.WriteUInt32LittleEndian(data, (uint)attributes);
        BinaryPrimitives.WriteInt32LittleEndian(data.AsSpan(4), 2);         // a pixel source unit
        BinaryPrimitives.WriteSingleLittleEndian(data.AsSpan(8), source.X);
        BinaryPrimitives.WriteSingleLittleEndian(data.AsSpan(12), source.Y);
        BinaryPrimitives.WriteSingleLittleEndian(data.AsSpan(16), source.W);
        BinaryPrimitives.WriteSingleLittleEndian(data.AsSpan(20), source.H);
        BinaryPrimitives.WriteSingleLittleEndian(data.AsSpan(24), destination.X);
        BinaryPrimitives.WriteSingleLittleEndian(data.AsSpan(28), destination.Y);
        BinaryPrimitives.WriteSingleLittleEndian(data.AsSpan(32), destination.W);
        BinaryPrimitives.WriteSingleLittleEndian(data.AsSpan(36), destination.H);

        byte[] record = new byte[12 + data.Length];
        BinaryPrimitives.WriteUInt16LittleEndian(record, 0x401A);
        BinaryPrimitives.WriteUInt16LittleEndian(record.AsSpan(2), (ushort)(image & 0xFF));
        BinaryPrimitives.WriteUInt32LittleEndian(record.AsSpan(4), (uint)record.Length);
        BinaryPrimitives.WriteUInt32LittleEndian(record.AsSpan(8), (uint)data.Length);
        data.CopyTo(record.AsSpan(12));

        return record;
    }
}
