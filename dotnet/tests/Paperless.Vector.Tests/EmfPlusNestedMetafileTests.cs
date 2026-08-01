using System.Buffers.Binary;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Shouldly;

namespace Paperless.Vector.Tests;

/// <summary>
/// A metafile carried inside another metafile, as an EMF+ image object.
/// </summary>
/// <remarks>
/// <para>
/// An <c>EmfPlusImage</c> may hold a whole WMF, EMF or EMF+ where a bitmap would go, and
/// <c>DrawImage</c> then places it under a transform. Replaying it means re-entering the
/// decoder from inside itself, which is why the bound is
/// <see cref="VectorLimits.MaxNestingDepth"/> rather than the shared budget: a budget is spent
/// as work is done, and a picture nested a thousand deep that draws almost nothing at each
/// level never spends any of it.
/// </para>
/// <para>
/// This is the same seam a reader uses to put a vector picture in a frame — one decode, one
/// display list, drawn into a destination rectangle — exercised from the one caller inside the
/// library, which is what makes it more than a hypothesis.
/// </para>
/// </remarks>
public sealed class EmfPlusNestedMetafileTests
{
    private const uint Red = 0xFFFF0000;

    [Fact]
    public void AMetafileCarriedAsAnImageIsReplayedRatherThanReported()
    {
        Recorder sink = Draw(Outer(Inner()));

        // The nested picture's own rectangle, drawn through the outer picture's placement.
        sink.Fills.ShouldHaveSingleItem().Paint.ShouldBeOfType<SolidPaint>()
            .Colour.ShouldBe(new Colour(255, 0, 0));
    }

    [Fact]
    public void TheNestedPictureLandsInsideTheDestinationItWasGiven()
    {
        Recorder sink = Draw(Outer(Inner()));
        DocRect bounds = sink.Fills.ShouldHaveSingleItem().Bounds;

        // The destination is a 20 mm square at 20 mm across, and it is the nested picture's whole
        // *frame* that is stretched onto it — not its ink. The inner picture's frame is 80 mm and
        // its red square 10 mm of that, so the square lands at an eighth of 20 mm. Getting this
        // the other way round is the classic wrongly-scaled nested metafile: the picture would
        // come out four times too large and clipped rather than merely small.
        bounds.X.Millimetres.ShouldBe(20, 0.5);
        bounds.Y.Millimetres.ShouldBe(20, 0.5);
        bounds.Width.Millimetres.ShouldBe(2.5, 0.5);
    }

    [Fact]
    public void NestingIsBoundedByDepthRatherThanByTheSharedBudget()
    {
        VectorImage image = VectorImages.Decode(
            Outer(Inner()).Build(),
            VectorLimits.Default with { MaxNestingDepth = 0 });

        // A budget cannot see this coming — the outer picture is tiny and does almost nothing —
        // so the depth is what has to be counted, and refusing says so.
        image.Diagnostics.ShouldContain(diagnostic => diagnostic.Code == "PL6039");
    }

    // ---------------------------------------------------------------- fixtures

    /// <summary>A whole EMF+ whose only content is a 10 mm red square at the origin.</summary>
    private static byte[] Inner() => new EmfBuilder()
        .Plus(new EmfPlusBuilder()
            .Header()
            .FillRects(Red, (0, 0, 1000, 1000))
            .End())
        .Build();

    /// <summary>An EMF+ that carries <paramref name="nested"/> as an image and draws it.</summary>
    private static EmfBuilder Outer(byte[] nested) => new EmfBuilder()
        .Plus(new EmfPlusBuilder()
            .Header()
            .Object(1, 0x500, MetafileImage(nested))
            .Raw(DrawImage(1, (2000, 2000, 2000, 2000)))
            .End());

    /// <summary>An <c>EmfPlusImage</c> of type 2, whose payload is a whole further metafile.</summary>
    private static byte[] MetafileImage(byte[] nested)
    {
        byte[] data = new byte[16 + nested.Length];

        BinaryPrimitives.WriteUInt32LittleEndian(data, 0xDBC01002);
        BinaryPrimitives.WriteInt32LittleEndian(data.AsSpan(4), 2);         // a metafile
        BinaryPrimitives.WriteInt32LittleEndian(data.AsSpan(8), 5);         // EMF+ only
        BinaryPrimitives.WriteInt32LittleEndian(data.AsSpan(12), nested.Length);
        nested.CopyTo(data.AsSpan(16));

        return data;
    }

    /// <summary>A <c>DrawImage</c> record naming a slot, a whole source and a destination.</summary>
    private static byte[] DrawImage(int slot, (float X, float Y, float Width, float Height) destination)
    {
        byte[] data = new byte[40];

        BinaryPrimitives.WriteUInt32LittleEndian(data, 0);                  // no image attributes
        BinaryPrimitives.WriteInt32LittleEndian(data.AsSpan(4), 2);         // a pixel source unit
        BinaryPrimitives.WriteSingleLittleEndian(data.AsSpan(8), 0);
        BinaryPrimitives.WriteSingleLittleEndian(data.AsSpan(12), 0);
        BinaryPrimitives.WriteSingleLittleEndian(data.AsSpan(16), 0);
        BinaryPrimitives.WriteSingleLittleEndian(data.AsSpan(20), 0);
        BinaryPrimitives.WriteSingleLittleEndian(data.AsSpan(24), destination.X);
        BinaryPrimitives.WriteSingleLittleEndian(data.AsSpan(28), destination.Y);
        BinaryPrimitives.WriteSingleLittleEndian(data.AsSpan(32), destination.Width);
        BinaryPrimitives.WriteSingleLittleEndian(data.AsSpan(36), destination.Height);

        byte[] record = new byte[12 + data.Length];
        BinaryPrimitives.WriteUInt16LittleEndian(record, 0x401A);
        BinaryPrimitives.WriteUInt16LittleEndian(record.AsSpan(2), (ushort)(slot & 0xFF));
        BinaryPrimitives.WriteUInt32LittleEndian(record.AsSpan(4), (uint)record.Length);
        BinaryPrimitives.WriteUInt32LittleEndian(record.AsSpan(8), (uint)data.Length);
        data.CopyTo(record.AsSpan(12));

        return record;
    }

    private static Recorder Draw(EmfBuilder emf)
    {
        VectorImage image = emf.Decode();
        Recorder sink = new();

        image.Draw(sink, new DocRect(DocPoint.Origin, image.IntrinsicSize));
        return sink;
    }
}
