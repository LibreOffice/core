using System.Buffers.Binary;
using Paperless.Vector.Wmf;
using Shouldly;

namespace Paperless.Vector.Tests;

/// <summary>
/// What a hostile or broken WMF does.
/// </summary>
/// <remarks>
/// <para>
/// These streams arrive inside documents that arrive by e-mail, and the format bounds nothing:
/// the record loop is driven by a length the file supplies and a polygon's point count is a
/// 16-bit number the file also supplies. The SVG work found two live holes in a third-party
/// parser — a <c>file:///etc/passwd</c> read reachable from an emailed document, and a
/// 1 057-byte input that hung for two minutes — so the guards here exist before the first bug
/// report rather than after it.
/// </para>
/// <para>
/// Every test asserts the same contract: never throw, never hang, and answer an empty picture
/// with a diagnostic rather than an exception. A picture that cannot be read is a document to
/// draw <em>without</em> that picture.
/// </para>
/// </remarks>
public class WmfSafetyTests
{
    [Fact]
    public void NoBytesAreClaimedThatAreNotAWmf()
    {
        WmfImageDecoder decoder = new();

        decoder.CanDecode([]).ShouldBeFalse();
        decoder.CanDecode("<svg xmlns='http://www.w3.org/2000/svg'/>"u8).ShouldBeFalse();
        decoder.CanDecode([0x89, (byte)'P', (byte)'N', (byte)'G', 13, 10, 26, 10, 0, 0, 0, 13, 0, 0, 0, 0, 0, 0]).ShouldBeFalse();

        // An EMF: its own signature, which must not be mistaken for a bare metafile header.
        byte[] emf = new byte[18];
        BinaryPrimitives.WriteUInt32LittleEndian(emf, 1);
        emf[8] = (byte)' ';
        decoder.CanDecode(emf).ShouldBeFalse();
    }

    [Fact]
    public void BothHeaderShapesAreRecognised()
    {
        WmfImageDecoder decoder = new();

        decoder.CanDecode(new WmfBuilder().Build()).ShouldBeTrue();
        decoder.CanDecode(new WmfBuilder { Placeable = false }.Build()).ShouldBeTrue();
    }

    [Fact]
    public void ATruncatedFileIsSkippedRatherThanThrown()
    {
        byte[] whole = new WmfBuilder()
            .WindowOrigin(0, 0)
            .WindowExtent(1000, 1000)
            .Rectangle(0, 0, 500, 500)
            .Build();

        // Every prefix, including the ones that stop in the middle of a record's parameters.
        for (int length = 0; length < whole.Length; length++)
        {
            VectorImage image = new WmfImageDecoder().Decode(whole.AsMemory(0, length));
            image.ShouldNotBeNull();
        }
    }

    [Fact]
    public void EveryCorruptedByteIsSurvived()
    {
        byte[] whole = new WmfBuilder()
            .WindowOrigin(0, 0)
            .WindowExtent(1000, 1000)
            .SolidBrush(1, 2, 3)
            .Select(0)
            .Polygon((0, 0), (100, 0), (100, 100))
            .Rectangle(0, 0, 500, 500)
            .Build();

        for (int i = 0; i < whole.Length; i++)
        {
            byte[] corrupt = [.. whole];
            corrupt[i] ^= 0xFF;

            new WmfImageDecoder().Decode(corrupt).ShouldNotBeNull();
        }
    }

    [Fact]
    public void ARecordClaimingMorePointsThanTheFileHoldsIsRefused()
    {
        // 30 000 points in a record with room for three. Believing the count is how a decoder
        // allocates 240 kB from a 60-byte file, and how it walks off the end of the buffer.
        byte[] file = new WmfBuilder()
            .WindowOrigin(0, 0)
            .WindowExtent(1000, 1000)
            .Record(WmfFunction.Polygon, 30000, 0, 0, 100, 100, 200, 0)
            .Build();

        VectorImage image = new WmfImageDecoder().Decode(file);

        image.ShouldNotBeNull();
        image.Content.Count.ShouldBe(0);
    }

    [Fact]
    public void ARecordSizeReachingPastTheFileStopsTheReplay()
    {
        byte[] file = new WmfBuilder()
            .WindowOrigin(0, 0)
            .WindowExtent(1000, 1000)
            .Rectangle(0, 0, 500, 500)
            .Build();

        // Rewrite the last real record's size to claim a megabyte.
        int position = 22 + 18;
        while (position + 6 < file.Length)
        {
            uint size = BinaryPrimitives.ReadUInt32LittleEndian(file.AsSpan(position));
            ushort function = BinaryPrimitives.ReadUInt16LittleEndian(file.AsSpan(position + 4));
            if (function == 0x041B)
            {
                BinaryPrimitives.WriteUInt32LittleEndian(file.AsSpan(position), 500_000);
                break;
            }

            position += (int)size * 2;
        }

        new WmfImageDecoder().Decode(file).ShouldNotBeNull();
    }

    [Fact]
    public void AFileOfNothingButStateRecordsIsBoundedByTheRecordCap()
    {
        // The case a command cap cannot see: a hundred thousand records that draw nothing at all.
        // Each still costs a parse, so only a record cap bounds it.
        WmfBuilder builder = new WmfBuilder().WindowOrigin(0, 0).WindowExtent(1000, 1000);
        for (int i = 0; i < 5000; i++) builder.Record(WmfFunction.SaveDc);

        VectorImage image = new WmfImageDecoder().Decode(
            builder.Rectangle(0, 0, 100, 100).Build(),
            new VectorLimits { MaxRecords = 100 });

        image.IsTruncated.ShouldBeTrue();
        image.Diagnostics.ShouldContain(d => d.Code == "PL6032");
    }

    [Fact]
    public void TheCommandCapStopsAFileThatDrawsTooMuch()
    {
        WmfBuilder builder = new WmfBuilder().WindowOrigin(0, 0).WindowExtent(1000, 1000);
        for (int i = 0; i < 200; i++) builder.Rectangle(0, 0, 100, 100);

        VectorImage image = new WmfImageDecoder().Decode(
            builder.Build(), new VectorLimits { MaxCommands = 20 });

        image.IsTruncated.ShouldBeTrue();
        image.Content.Count.ShouldBeLessThan(60);
    }

    [Fact]
    public void AFileLargerThanTheByteCapIsNotReadAtAll()
    {
        VectorImage image = new WmfImageDecoder().Decode(
            new WmfBuilder().Rectangle(0, 0, 10, 10).Build(),
            new VectorLimits { MaxBytes = 8 });

        image.IsEmpty.ShouldBeTrue();
        image.Diagnostics.ShouldContain(d => d.Code == "PL6002");
    }

    [Fact]
    public void ARestoreThatNamesAStateThatWasNeverSavedIsIgnored()
    {
        // RestoreDC with a positive argument names a specific saved state, and files name ones
        // they never saved. Treating it as a count would pop the stack empty.
        VectorImage image = new WmfImageDecoder().Decode(new WmfBuilder()
            .WindowOrigin(0, 0)
            .WindowExtent(1000, 1000)
            .Record(WmfFunction.RestoreDc, 7)
            .Record(WmfFunction.RestoreDc, -3)
            .Rectangle(0, 0, 500, 500)
            .Build());

        image.Content.Count.ShouldBeGreaterThan(0);
    }

    [Fact]
    public void SelectingAHandleThatWasNeverCreatedKeepsTheCurrentSelection()
    {
        VectorImage image = new WmfImageDecoder().Decode(new WmfBuilder()
            .WindowOrigin(0, 0)
            .WindowExtent(1000, 1000)
            .Select(40)
            .Rectangle(0, 0, 500, 500)
            .Build());

        image.Content.Count.ShouldBeGreaterThan(0);
        image.Diagnostics.ShouldContain(d => d.Code == "PL6031");
    }

    [Fact]
    public void AnEscapeIsNeverAskedToResolveAnythingOutsideTheFile()
    {
        // An escape is WMF's open extension point, and the SVG work's lesson is that an
        // extension point in an attacker-supplied file is where the file-read hole lives. The
        // two payloads this reads name nothing outside their own bytes; everything else is
        // ignored. A comment holding a path must therefore have no effect at all.
        byte[] payload = new byte[64];
        BinaryPrimitives.WriteUInt16LittleEndian(payload, 15);
        BinaryPrimitives.WriteUInt16LittleEndian(payload.AsSpan(2), 60);
        System.Text.Encoding.ASCII.GetBytes("file:///etc/passwd\0").CopyTo(payload, 4);

        VectorImage image = new WmfImageDecoder().Decode(new WmfBuilder()
            .WindowOrigin(0, 0)
            .WindowExtent(1000, 1000)
            .Raw(WmfFunction.Escape, payload)
            .Rectangle(0, 0, 500, 500)
            .Build());

        image.Content.Count.ShouldBeGreaterThan(0);
        image.Diagnostics.ShouldNotContain(d => d.Code == "PL6030");
    }

    [Fact]
    public void AnObjectRecordWithAnImplausibleCountIsRefusedBeforeItAllocates()
    {
        // A palette claiming 60 000 entries inside a 20-byte record.
        VectorImage image = new WmfImageDecoder().Decode(new WmfBuilder()
            .WindowOrigin(0, 0)
            .WindowExtent(1000, 1000)
            .Record((WmfFunction)0x00F7, 0, unchecked((short)60000), 0, 0, 0, 0)
            .Rectangle(0, 0, 500, 500)
            .Build());

        image.Content.Count.ShouldBeGreaterThan(0);
    }

    [Fact]
    public void ARecordWhoseSizeIsTooSmallToBeLegalEndsTheReplay()
    {
        byte[] file = new WmfBuilder().WindowOrigin(0, 0).WindowExtent(1000, 1000).Build();

        // A record size below three cannot even hold its own header; GDI's own reader stops.
        BinaryPrimitives.WriteUInt32LittleEndian(file.AsSpan(22 + 18), 1);

        new WmfImageDecoder().Decode(file).ShouldNotBeNull();
    }
}
