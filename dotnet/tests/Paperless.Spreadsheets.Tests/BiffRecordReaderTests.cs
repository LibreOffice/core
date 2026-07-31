using System.Buffers.Binary;
using Paperless.Core.Diagnostics;
using Paperless.Spreadsheets.MsBinary;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// Tests the BIFF record stream against hand-built byte sequences.
/// </summary>
/// <remarks>
/// Synthetic rather than corpus-driven on purpose: the cases that matter here are the ones a
/// well-formed file never contains — a record whose declared length overruns the stream, a
/// string that changes width halfway through, a directory offset that points at nothing. A
/// corpus of files LibreOffice wrote cannot exercise any of them.
/// </remarks>
public class BiffRecordReaderTests
{
    /// <summary>Builds one record: a 16-bit id, a 16-bit length, and the payload.</summary>
    private static byte[] Record(ushort id, params byte[] payload)
    {
        byte[] record = new byte[4 + payload.Length];
        BinaryPrimitives.WriteUInt16LittleEndian(record, id);
        BinaryPrimitives.WriteUInt16LittleEndian(record.AsSpan(2), (ushort)payload.Length);
        payload.CopyTo(record.AsSpan(4));
        return record;
    }

    private static byte[] Concat(params byte[][] parts)
        => [.. parts.SelectMany(part => part)];

    [Theory]
    // The four combinations of the two flag bits, each producing a plausible number from the
    // same thirty bits — which is why every one is asserted.
    [InlineData(0x00000000, 0.0)]
    [InlineData(0x3FF00000, 1.0)]           // A double's high word: 1.0.
    [InlineData(0x3FF00001, 0.01)]          // The same, divided by a hundred.
    [InlineData(0x00000032, 12.0)]          // An integer: 0x32 >> 2.
    [InlineData(0x0000070B, 4.5)]           // An integer of hundredths: 1803 >> 2 = 450.
    [InlineData(unchecked((int)0xFFFFFFFE), -1.0)]  // A negative integer, sign-extended.
    public void RkNumbersDecodeByTheirTwoFlagBits(int encoded, double expected)
        => BiffRecordReader.RkValue(encoded).ShouldBe(expected);

    [Fact]
    public void ARecordWhoseLengthOverrunsTheStreamIsClampedAndReported()
    {
        List<Diagnostic> diagnostics = [];

        // Declares eight bytes of payload and supplies two.
        byte[] data = [0x0A, 0x02, 0x08, 0x00, 0x01, 0x02];
        BiffRecordReader stream = new(data, diagnostics);

        stream.MoveNext().ShouldBeTrue();
        stream.RecordLeft.ShouldBe(2);
        stream.ReadUInt16().ShouldBe((ushort)0x0201);

        diagnostics.ShouldHaveSingleItem();
        diagnostics[0].Code.ShouldBe("PL2320");
    }

    [Fact]
    public void ReadingPastARecordsEndYieldsZeroesRatherThanThrowing()
    {
        BiffRecordReader stream = new(Record(0x0201, 1, 0), []);
        stream.MoveNext().ShouldBeTrue();

        stream.ReadUInt16().ShouldBe((ushort)1);
        stream.ReadUInt16().ShouldBe((ushort)0);
        stream.IsValid.ShouldBeFalse();
    }

    [Fact]
    public void ZeroRecordsAreSteppedOverButNotIndefinitely()
    {
        // A handful of id-and-length-zero records between real ones is padding some writers
        // emit; a stream of them means the position is not a record boundary at all.
        byte[] padded = Concat([0, 0, 0, 0], [0, 0, 0, 0], Record(0x0201, 7, 0));
        BiffRecordReader padding = new(padded, []);
        padding.MoveNext().ShouldBeTrue();
        padding.RecordId.ShouldBe((ushort)0x0201);

        byte[] noise = new byte[64];
        BiffRecordReader all = new(noise, []);
        all.MoveNext().ShouldBeFalse();
    }

    [Fact]
    public void ARecordAndItsContinuationsAreOneLogicalPayload()
    {
        byte[] data = Concat(
            Record(0x00FC, 1, 2, 3),
            Record(0x003C, 4, 5, 6),
            Record(0x0201, 9, 9));

        BiffRecordReader stream = new(data, []);
        stream.MoveNext().ShouldBeTrue();
        stream.RecordId.ShouldBe((ushort)0x00FC);
        stream.RecordLeft.ShouldBe(6);
        stream.ReadBytes(6).ShouldBe([1, 2, 3, 4, 5, 6]);

        // And the record after the continuations is the next one the caller sees.
        stream.MoveNext().ShouldBeTrue();
        stream.RecordId.ShouldBe((ushort)0x0201);
    }

    [Fact]
    public void AStringMayChangeWidthAtAContinuationBoundary()
    {
        // Six characters declared, three of them compressed in the first record; the
        // continuation restates the width and supplies the rest as 16-bit characters. This
        // is the trap the whole class exists for: reading straight through gives text that
        // is right up to the boundary and mojibake afterwards.
        byte[] data = Concat(
            Record(0x0207, 6, 0, 0, (byte)'a', (byte)'b', (byte)'c'),
            Record(0x003C, 0x01, (byte)'d', 0, (byte)'e', 0, (byte)'f', 0));

        BiffRecordReader stream = new(data, []);
        stream.MoveNext().ShouldBeTrue();
        stream.ReadString(eightBitLength: false).ShouldBe("abcdef");
    }

    [Fact]
    public void AWideStringDecodesAsUtf16AndACompressedOneAsLatin1()
    {
        // The compressed form is not the code page: its bytes are Unicode code points below
        // 256, so an e-acute is 0xE9 whatever CODEPAGE said.
        BiffRecordReader compressed = new(Record(0x0207, 2, 0, 0, 0xE9, (byte)'t'), []);
        compressed.MoveNext();
        compressed.ReadString(eightBitLength: false).ShouldBe("ét");

        BiffRecordReader wide = new(Record(0x0207, 2, 0, 0x01, 0x14, 0x20, 0x41, 0x00), []);
        wide.MoveNext();
        wide.ReadString(eightBitLength: false).ShouldBe("—A");
    }

    [Fact]
    public void RichTextAndPhoneticTrailersAreSkippedRatherThanReadAsCharacters()
    {
        // The flags say the string carries two formatting runs and four bytes of Asian
        // phonetic data. Both sit after the characters, and their sizes are declared before
        // them, so a reader that ignores the flags reads the trailers as text.
        byte[] payload =
        [
            3, 0,               // Three characters.
            0x0C,               // Rich, and far-east.
            2, 0,               // Two formatting runs.
            4, 0, 0, 0,         // Four bytes of phonetic data.
            (byte)'a', (byte)'b', (byte)'c',
            0, 0, 0, 0, 0, 0, 0, 0,   // The runs.
            9, 9, 9, 9,               // The phonetic block.
        ];

        BiffRecordReader stream = new(Concat(Record(0x00FD, payload), Record(0x0201, 0, 0)), []);
        stream.MoveNext();
        stream.ReadString(eightBitLength: false).ShouldBe("abc");
        stream.RecordLeft.ShouldBe(0);
    }

    [Fact]
    public void ARecordIdentifierCanBeCheckedWithoutMoving()
    {
        byte[] data = Concat(Record(0x0201, 0, 0), Record(0x0809, 0, 6));
        BiffRecordReader stream = new(data, []);

        // What the sheet directory's offset check needs: does this offset hold a BOF?
        stream.PeekRecordId(6).ShouldBe((ushort)0x0809);
        stream.PeekRecordId(9999).ShouldBe((ushort)0);
        BiffRecords.IsBof(stream.PeekRecordId(6)).ShouldBeTrue();
    }

    [Fact]
    public void TheStreamCanBeRestartedAtAnAbsoluteOffset()
    {
        byte[] data = Concat(Record(0x0201, 0, 0), Record(0x0203, 1, 0));
        BiffRecordReader stream = new(data, []);

        stream.MoveNext(6).ShouldBeTrue();
        stream.RecordId.ShouldBe((ushort)0x0203);
        stream.MoveNext(-4).ShouldBeFalse();
    }
}
