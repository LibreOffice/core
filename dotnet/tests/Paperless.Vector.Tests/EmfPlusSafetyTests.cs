using System.Buffers.Binary;
using System.Diagnostics;
using Paperless.Core.Geometry;
using Shouldly;

namespace Paperless.Vector.Tests;

/// <summary>
/// What an EMF+ stream may not do, however malformed it is.
/// </summary>
/// <remarks>
/// <para>
/// An EMF comment is the most attacker-controllable part of a metafile, because it is
/// <em>opaque</em> to every consumer that does not understand it: a validator, a virus scanner and
/// a thumbnailer all walk straight past bytes that this reader then parses. So the invariants
/// asserted here are the same ones the SVG and WMF safety tests assert, applied to a payload
/// nothing else has looked at.
/// </para>
/// <para>
/// Every one of them is "the picture is drawn without that part", never an exception: a picture
/// that cannot be read is a document to draw without that picture.
/// </para>
/// </remarks>
public sealed class EmfPlusSafetyTests
{
    [Fact]
    public void ARecordWhoseSizeIsBelowTheHeaderEndsTheStream()
    {
        byte[] record = new byte[12];
        BinaryPrimitives.WriteUInt16LittleEndian(record, 0x400A);
        BinaryPrimitives.WriteUInt32LittleEndian(record.AsSpan(4), 4);      // impossible

        VectorImage image = Decode(
            new EmfPlusBuilder().Header().FillRects(0xFFFF0000, (0, 0, 1000, 1000)), record);

        // The record before it is still drawn: a stream that stops making sense part-way is a
        // picture to draw the beginning of.
        image.IsEmpty.ShouldBeFalse();
        image.IsTruncated.ShouldBeFalse();
    }

    [Fact]
    public void ARecordThatClaimsMoreBytesThanTheCommentHoldsIsRefused()
    {
        byte[] record = new byte[12];
        BinaryPrimitives.WriteUInt16LittleEndian(record, 0x400A);
        BinaryPrimitives.WriteUInt32LittleEndian(record.AsSpan(4), 0x7FFFFFF0);
        BinaryPrimitives.WriteUInt32LittleEndian(record.AsSpan(8), 8);

        Should.NotThrow(() => Decode(new EmfPlusBuilder().Header(), record));
    }

    [Fact]
    public void ADataSizeLargerThanTheRecordIsRefused()
    {
        byte[] record = new byte[20];
        BinaryPrimitives.WriteUInt16LittleEndian(record, 0x400A);
        BinaryPrimitives.WriteUInt32LittleEndian(record.AsSpan(4), 20);
        BinaryPrimitives.WriteUInt32LittleEndian(record.AsSpan(8), 0xFFFFFF00);

        Should.NotThrow(() => Decode(new EmfPlusBuilder().Header(), record));
    }

    [Fact]
    public void APointCountBeyondTheRecordIsClampedRatherThanAllocated()
    {
        // The cheapest guard there is, and the one that catches the commonest attack: a record
        // claiming four hundred thousand points inside a two-hundred-byte file.
        byte[] data = new byte[12];
        BinaryPrimitives.WriteUInt32LittleEndian(data, 0xDBC01002);
        BinaryPrimitives.WriteUInt32LittleEndian(data.AsSpan(4), 400_000);
        BinaryPrimitives.WriteUInt32LittleEndian(data.AsSpan(8), 0);

        Stopwatch clock = Stopwatch.StartNew();

        VectorImage image = Decode(new EmfPlusBuilder()
            .Header()
            .Object(1, 0x300, data)
            .FillPath(1, 0xFF000000)
            .End());

        clock.Elapsed.ShouldBeLessThan(TimeSpan.FromSeconds(5));
        image.ShouldNotBeNull();
    }

    [Fact]
    public void AnObjectSlotBeyondTheTableIsMaskedRatherThanIndexed()
    {
        // The slot is the flags word's low byte, so a table of 256 can be indexed exactly; a
        // reader that used the whole word would run off the end of its own array.
        Should.NotThrow(() => Decode(new EmfPlusBuilder()
            .Header()
            .Object(0x1FF, 0x100, [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0])
            .End()));
    }

    [Fact]
    public void ADeeplyNestedRegionIsRefusedRatherThanRecursedInto()
    {
        // A region is a binary tree of set operations, so nesting is unbounded in the format and
        // every level is one stack frame. Ten thousand of them is a two-hundred-byte file.
        List<byte> data = [0x02, 0x10, 0xC0, 0xDB, 0, 0, 0, 0];

        for (int i = 0; i < 10_000; i++)
        {
            data.AddRange([0x01, 0, 0, 0]);                 // an intersection node
        }

        Should.NotThrow(() => Decode(new EmfPlusBuilder()
            .Header()
            .Object(1, 0x400, [.. data])
            .End()));
    }

    [Fact]
    public void AnObjectSplitOverEndlessPartsCannotGrowWithoutBound()
    {
        EmfPlusBuilder plus = new EmfPlusBuilder().Header();

        byte[] part = new byte[1024];
        BinaryPrimitives.WriteUInt32LittleEndian(part, 0x7FFFFFFF);

        // Nothing in the format bounds how many parts an object may have, so the assembled buffer
        // is what has to be bounded.
        for (int i = 0; i < 4000; i++) plus.Record(0x4008, 0x8300, part);

        Stopwatch clock = Stopwatch.StartNew();
        Should.NotThrow(() => Decode(plus.End()));
        clock.Elapsed.ShouldBeLessThan(TimeSpan.FromSeconds(10));
    }

    [Fact]
    public void TheRecordCapStopsAStreamOfRecordsThatDrawNothing()
    {
        EmfPlusBuilder plus = new EmfPlusBuilder().Header();

        // Most EMF+ records emit no drawing command at all, so a command cap alone would never
        // fire however long the stream is — which is why the budget charges both.
        for (int i = 0; i < 5_000; i++) plus.Save(i & 0xFF);

        VectorImage image = VectorImages.Decode(
            new EmfBuilder().Plus(plus.End()).Build(), VectorLimits.Default with { MaxRecords = 1_000 });

        image.IsTruncated.ShouldBeTrue();
        image.Diagnostics.ShouldContain(diagnostic => diagnostic.Code == "PL6032");
    }

    [Fact]
    public void ACommentThatIsNotEmfPlusIsLeftAlone()
    {
        byte[] payload = new byte[16];
        BinaryPrimitives.WriteUInt32LittleEndian(payload, 12);
        BinaryPrimitives.WriteUInt32LittleEndian(payload.AsSpan(4), 0x43494447);     // "GDIC"

        VectorImage image = new EmfBuilder()
            .Raw(EmfFunction.Comment, payload)
            .SolidBrush(1, 255, 0, 0)
            .Select(1)
            .Rectangle(0, 0, 1000, 1000)
            .Decode();

        // A public GDI comment must not put the reader into EMF+ mode, or the rest of the file's
        // GDI records stop being drawn.
        Recorder sink = new();
        image.Draw(sink, new DocRect(DocPoint.Origin, image.IntrinsicSize));

        sink.Fills.ShouldHaveSingleItem();
    }

    [Fact]
    public void AnEmptyCommentIsHarmless()
    {
        byte[] payload = new byte[8];
        BinaryPrimitives.WriteUInt32LittleEndian(payload, 4);
        BinaryPrimitives.WriteUInt32LittleEndian(payload.AsSpan(4), 0x2B464D45);

        Should.NotThrow(() => new EmfBuilder().Raw(EmfFunction.Comment, payload).Decode());
    }

    [Fact]
    public void ANonFiniteFloatDoesNotPoisonTheMapping()
    {
        byte[] data = new byte[24];
        for (int i = 0; i < 6; i++)
        {
            BinaryPrimitives.WriteUInt32LittleEndian(data.AsSpan(i * 4), 0x7FC00000);    // NaN
        }

        VectorImage image = Decode(new EmfPlusBuilder()
            .Header()
            .Record(0x402A, 0, data)
            .FillRects(0xFFFF0000, (0, 0, 1000, 1000))
            .End());

        Recorder sink = new();
        image.Draw(sink, new DocRect(DocPoint.Origin, image.IntrinsicSize));

        // One NaN in a world transform would otherwise make the whole rest of the picture vanish
        // rather than one shape, which is why they are filtered at the read.
        foreach ((DocRect bounds, _, _) in sink.Fills)
        {
            double.IsFinite(bounds.X.Emu).ShouldBeTrue();
            double.IsFinite(bounds.Width.Emu).ShouldBeTrue();
        }
    }

    [Fact]
    public void ATruncatedBrushLeavesTheSlotUsableRatherThanThrowing()
    {
        byte[] data = new byte[6];
        BinaryPrimitives.WriteUInt32LittleEndian(data, 0xDBC01002);

        Should.NotThrow(() => Decode(new EmfPlusBuilder()
            .Header()
            .Object(1, 0x100, data)
            .FillRectsWithBrush(1, (0, 0, 1000, 1000))
            .End()));
    }

    [Fact]
    public void APenWhoseDashArrayRunsPastTheRecordIsRefused()
    {
        List<byte> data = [];

        void Add32(uint value)
        {
            data.Add((byte)value);
            data.Add((byte)(value >> 8));
            data.Add((byte)(value >> 16));
            data.Add((byte)(value >> 24));
        }

        Add32(0xDBC01002);
        Add32(0);
        Add32(0x0100);                                     // a custom dash array follows
        Add32(0);
        Add32(BitConverter.SingleToUInt32Bits(10f));
        Add32(1_000_000);                                  // and it claims a million entries

        Should.NotThrow(() => Decode(new EmfPlusBuilder()
            .Header()
            .Object(1, 0x200, [.. data])
            .DrawLines(1, [(0, 0), (1000, 1000)])
            .End()));
    }

    [Fact]
    public void ANativeBitmapWhoseStrideRunsPastTheRecordIsRefused()
    {
        List<byte> data = [];

        void Add32(uint value)
        {
            data.Add((byte)value);
            data.Add((byte)(value >> 8));
            data.Add((byte)(value >> 16));
            data.Add((byte)(value >> 24));
        }

        Add32(0xDBC01002);
        Add32(1);                                          // a bitmap
        Add32(4000);                                       // width
        Add32(4000);                                       // height
        Add32(16000);                                      // stride
        Add32(0x0026200A);                                 // 32 bits a pixel
        Add32(0);                                          // native pixels, and none follow

        Should.NotThrow(() => Decode(new EmfPlusBuilder()
            .Header()
            .Object(1, 0x500, [.. data])
            .End()));
    }

    private static VectorImage Decode(EmfPlusBuilder plus, params byte[][] malformed)
    {
        ArgumentNullException.ThrowIfNull(plus);
        ArgumentNullException.ThrowIfNull(malformed);

        foreach (byte[] record in malformed) plus.Raw(record);

        return new EmfBuilder().Plus(plus).Decode();
    }
}
