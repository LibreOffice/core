using System.Buffers.Binary;
using System.Diagnostics;
using Paperless.Vector.Emf;
using Shouldly;

namespace Paperless.Vector.Tests;

/// <summary>
/// What a hostile or broken EMF does.
/// </summary>
/// <remarks>
/// <para>
/// EMF widens every field WMF bounded by accident: a point count is 32 bits rather than 16, a
/// record's length is 32 bits, and a bitmap's header and bits are named by offsets into the
/// record rather than found where they lie. Each of those is a place where a file can claim
/// more than it holds, and none of them is checked by the format.
/// </para>
/// <para>
/// Every test asserts the same contract: never throw, never hang, and answer an empty picture
/// with a diagnostic rather than an exception. A picture that cannot be read is a document to
/// draw <em>without</em> that picture.
/// </para>
/// </remarks>
public class EmfSafetyTests
{
    [Fact]
    public void NoBytesAreClaimedThatAreNotAnEmf()
    {
        EmfImageDecoder decoder = new();

        decoder.CanDecode([]).ShouldBeFalse();
        decoder.CanDecode("<svg xmlns='http://www.w3.org/2000/svg'/>"u8).ShouldBeFalse();
        decoder.CanDecode(new byte[200]).ShouldBeFalse();

        // A record type of 1 alone is one byte of signal; without the " EMF" signature at
        // offset 40 it must not be claimed, or the sniff would swallow half the binary formats
        // in a package.
        byte[] pretender = new byte[200];
        BinaryPrimitives.WriteUInt32LittleEndian(pretender, 1);
        BinaryPrimitives.WriteUInt32LittleEndian(pretender.AsSpan(4), 108);
        decoder.CanDecode(pretender).ShouldBeFalse();

        decoder.CanDecode(new WmfBuilder().Build()).ShouldBeFalse();
    }

    [Fact]
    public void ATruncatedFileIsSkippedRatherThanThrown()
    {
        byte[] whole = new EmfBuilder()
            .SolidBrush(1, 0, 0, 0)
            .Select(1)
            .Rectangle(0, 0, 1000, 1000)
            .Build();

        for (int length = 1; length < whole.Length; length++)
        {
            VectorImage image = VectorImages.Decode(whole.AsMemory(0, length));
            image.ShouldNotBeNull();
        }
    }

    [Fact]
    public void AHeaderThatOverstatesTheFileSizeIsBelievedOnlyAsFarAsTheFileGoes()
    {
        byte[] file = new EmfBuilder()
            .SolidBrush(1, 0x10, 0x20, 0x30)
            .Select(1)
            .Rectangle(0, 0, 1000, 1000)
            .Build();

        // A truncated EMF is common, and the records that are there are still worth drawing.
        BinaryPrimitives.WriteUInt32LittleEndian(file.AsSpan(48), 0x7FFFFFFF);

        VectorImage image = VectorImages.Decode(file);
        image.IsEmpty.ShouldBeFalse();
    }

    [Fact]
    public void ARecordThatClaimsMorePointsThanTheFileHoldsAllocatesNothing()
    {
        byte[] file = new EmfBuilder()
            .SolidBrush(1, 0, 0, 0)
            .Select(1)
            .Poly16(EmfFunction.Polygon16, (0, 0), (100, 0), (100, 100))
            .Build();

        // The count sits after the sixteen-byte bounds rectangle in the record's payload.
        int at = IndexOfRecord(file, (uint)EmfFunction.Polygon16) + 8 + 16;
        BinaryPrimitives.WriteInt32LittleEndian(file.AsSpan(at), 400_000);

        Stopwatch clock = Stopwatch.StartNew();
        VectorImage image = VectorImages.Decode(file);
        clock.Stop();

        // The cheapest guard there is: a stated count is checked against the bytes that remain
        // before anything is allocated, so the allocation never happens rather than happening
        // and then failing.
        image.ShouldNotBeNull();
        clock.Elapsed.ShouldBeLessThan(TimeSpan.FromSeconds(5));
    }

    [Fact]
    public void ABitmapOffsetOutsideTheRecordReadsNothing()
    {
        byte[] pixels = new byte[3];
        byte[] file = new EmfBuilder()
            .Bitmap((0, 0, 1000, 1000), 1, 1, pixels)
            .Build();

        // offBmiSrc is at the record's payload offset 32; a value past the end of the file must
        // answer an empty span rather than an out-of-range read.
        int at = IndexOfRecord(file, (uint)EmfFunction.StretchDIBits) + 8 + 32;
        BinaryPrimitives.WriteUInt32LittleEndian(file.AsSpan(at), 0xFFFFFF00);

        VectorImage image = VectorImages.Decode(file);
        image.ShouldNotBeNull();
    }

    [Fact]
    public void ARecordSizeOfZeroDoesNotLoop()
    {
        byte[] file = new EmfBuilder()
            .SolidBrush(1, 0, 0, 0)
            .Select(1)
            .Rectangle(0, 0, 1000, 1000)
            .Build();

        int at = IndexOfRecord(file, (uint)EmfFunction.CreateBrushIndirect);
        BinaryPrimitives.WriteUInt32LittleEndian(file.AsSpan(at + 4), 0);

        Stopwatch clock = Stopwatch.StartNew();
        VectorImages.Decode(file).ShouldNotBeNull();
        clock.Stop();

        clock.Elapsed.ShouldBeLessThan(TimeSpan.FromSeconds(5));
    }

    [Fact]
    public void AStatedRecordCountFarBeyondTheFileStopsAtTheFilesEnd()
    {
        byte[] file = new EmfBuilder()
            .SolidBrush(1, 0, 0, 0)
            .Select(1)
            .Rectangle(0, 0, 1000, 1000)
            .Build();

        BinaryPrimitives.WriteUInt32LittleEndian(file.AsSpan(52), 0x7FFFFFFF);

        Stopwatch clock = Stopwatch.StartNew();
        VectorImages.Decode(file).IsEmpty.ShouldBeFalse();
        clock.Stop();

        clock.Elapsed.ShouldBeLessThan(TimeSpan.FromSeconds(5));
    }

    [Fact]
    public void AMillionSaveRecordsAreCappedByTheRecordBudgetRatherThanTheCommandBudget()
    {
        // Most records emit no drawing command at all, so a command cap alone would let this
        // spin: a record cap and a command cap bound different things.
        EmfBuilder builder = new();
        for (int i = 0; i < 200_000; i++) builder.Record(EmfFunction.SaveDc);

        VectorLimits limits = new() { MaxRecords = 1000 };

        Stopwatch clock = Stopwatch.StartNew();
        VectorImage image = VectorImages.Decode(builder.Build(), limits);
        clock.Stop();

        image.IsTruncated.ShouldBeTrue();
        image.Diagnostics.ShouldContain(d => d.Code == "PL6032");
        clock.Elapsed.ShouldBeLessThan(TimeSpan.FromSeconds(5));
    }

    [Fact]
    public void ExcludingHundredsOfRectanglesDoesNotMultiplyWithoutBound()
    {
        // Each exclusion can quadruple the rectangle count, so an adversarial file of a few
        // hundred of them would allocate without bound if nothing capped the result.
        EmfBuilder builder = new();
        builder.Record(EmfFunction.IntersectClipRect, 0, 0, 8000, 6000);

        for (int i = 0; i < 400; i++)
        {
            builder.Record(EmfFunction.ExcludeClipRect, i * 7, i * 5, (i * 7) + 3, (i * 5) + 3);
        }

        builder.SolidBrush(1, 0, 0, 0).Select(1).Rectangle(0, 0, 8000, 6000);

        Stopwatch clock = Stopwatch.StartNew();
        VectorImage image = VectorImages.Decode(builder.Build());
        clock.Stop();

        image.IsEmpty.ShouldBeFalse();
        clock.Elapsed.ShouldBeLessThan(TimeSpan.FromSeconds(10));
    }

    [Fact]
    public void ADeeplyRepeatedRestoreDoesNotUnderflowTheSaveStack()
    {
        EmfBuilder builder = new();
        for (int i = 0; i < 500; i++) builder.Record(EmfFunction.RestoreDc, -1);
        builder.Record(EmfFunction.RestoreDc, 1000);
        builder.SolidBrush(1, 0, 0, 0).Select(1).Rectangle(0, 0, 1000, 1000);

        VectorImages.Decode(builder.Build()).IsEmpty.ShouldBeFalse();
    }

    [Fact]
    public void AWorldTransformOfInfinitiesIsIgnoredRatherThanPropagated()
    {
        byte[] transform = new byte[24];
        for (int i = 0; i < 6; i++)
        {
            BinaryPrimitives.WriteSingleLittleEndian(transform.AsSpan(i * 4), float.PositiveInfinity);
        }

        VectorImage image = new EmfBuilder()
            .SolidBrush(1, 0, 0, 0)
            .Select(1)
            .Raw(EmfFunction.SetWorldTransform, transform)
            .Rectangle(0, 0, 1000, 1000)
            .Decode();

        image.IsEmpty.ShouldBeFalse();
    }

    [Fact]
    public void AnEmbeddedEmfInAWmfIsBoundedByTheWmfsOwnBytes()
    {
        // The escape payload is the EMF analogue of the SVG href: an open extension point in an
        // attacker-supplied file. It names only bytes inside its own record, and a chunk that
        // claims more than the file holds reassembles to nothing.
        byte[] escape = new byte[38];
        BinaryPrimitives.WriteUInt32LittleEndian(escape, 0x43464D57);       // "WMFC"
        BinaryPrimitives.WriteUInt32LittleEndian(escape.AsSpan(4), 1);      // comment type
        BinaryPrimitives.WriteUInt32LittleEndian(escape.AsSpan(8), 0x00010000);
        BinaryPrimitives.WriteUInt32LittleEndian(escape.AsSpan(18), 1);     // chunk count
        BinaryPrimitives.WriteUInt32LittleEndian(escape.AsSpan(22), 0x7FFFFFFF);

        byte[] payload = new byte[4 + escape.Length];
        BinaryPrimitives.WriteUInt16LittleEndian(payload, 15);              // METAFILE_COMMENT
        BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(2), (ushort)escape.Length);
        escape.CopyTo(payload.AsSpan(4));

        byte[] file = new WmfBuilder()
            .Raw(WmfFunction.Escape, payload)
            .Rectangle(0, 0, 500, 500)
            .Build();

        VectorImages.Decode(file).ShouldNotBeNull();
    }

    private static int IndexOfRecord(byte[] file, uint type)
    {
        int at = BinaryPrimitives.ReadInt32LittleEndian(file.AsSpan(4));

        while (at + 8 <= file.Length)
        {
            uint found = BinaryPrimitives.ReadUInt32LittleEndian(file.AsSpan(at));
            uint size = BinaryPrimitives.ReadUInt32LittleEndian(file.AsSpan(at + 4));

            if (found == type) return at;
            if (size < 8) break;

            at += (int)size;
        }

        throw new InvalidOperationException($"No record of type {type} in the file.");
    }
}
