using XRay.Office.MsBinary.Records;
using XRay.Office.Presentations.MsBinary;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// Tests that a deck's hyperlink list is capped by what its <c>_PID_HLINKS</c> blob yields
/// <em>through the reference's own truncating property reader</em>, and not by what the blob
/// declares or by how many <c>ExHyperlink</c> records the document holds.
/// </summary>
/// <remarks>
/// <para>
/// <c>Section::Read</c> clamps every property's buffer to <c>nSecSize - nSecOfs</c> — a declared
/// <em>length</em> less an absolute <em>position</em> (<c>sd/source/filter/ppt/propread.cxx</c>
/// :443-447) — so a user-defined section that starts late in the stream hands
/// <c>ImplSdPPTImport::Import</c> a blob cut short, its entry loop stops at the first string it
/// cannot read, and only the entries it built are given an index from the <c>ExObjList</c>
/// (<c>pptin.cxx</c>:392-549). A text range naming any later identifier is then not a field at
/// all.
/// </para>
/// <para>
/// Measured at 26.2.4.2 rather than taken from the source, which declares a different version:
/// on <c>RESPA_-_Section_8_Webinar.ppt</c> the rule predicts two entries where the blob declares
/// ten, and patching one text range's <c>exHyperlinkId</c> to each of the file's ten
/// <c>ExHyperlinkAtom</c> values in turn shows exactly the first two drawn as links
/// (<c>probes/slides-r103/hyperid.py</c>, <c>results.md</c> §1).
/// </para>
/// <para>
/// Synthetic property sets throughout: the corpus documents that carry this live in
/// <c>sample-files</c>, which the unit suite does not read.
/// </para>
/// </remarks>
public class PptHyperlinkBlobTests
{
    /// <summary>The offset the section list puts the one section at: 28 of header plus 20.</summary>
    private const int SectionOffset = 48;

    [Fact]
    public void EveryDeclaredHyperlinkIsReadWhenTheSectionsSizeDoesNotCutTheBlobShort()
    {
        byte[] summary = Summary(entries: 5, target: 100, clamp: null);

        PptHyperlinkBlob.Count(summary).ShouldBe(5);
    }

    [Fact]
    public void ThePropertyReadersClampStopsTheListAtTheLastWholeEntryItCanReach()
    {
        // Each entry is 32 bytes of typed integers, 8 + 200 of target and 12 of sub-address, so
        // the second ends at 12 + 2 x 252 = 516 and the third would end at 768.
        byte[] summary = Summary(entries: 5, target: 100, clamp: 700);

        PptHyperlinkBlob.Count(summary).ShouldBe(2);
    }

    [Fact]
    public void AClampThatFallsInsideAnEntrysFirstStringStillKeepsTheEntriesBeforeIt()
    {
        // 12 + 252 = 264 is the end of the first entry; 300 lands in the second's target.
        byte[] summary = Summary(entries: 5, target: 100, clamp: 300);

        PptHyperlinkBlob.Count(summary).ShouldBe(1);
    }

    [Fact]
    public void ADocumentWithNoSummaryStreamCapsNothing()
    {
        PptHyperlinkBlob.Count([]).ShouldBeNull();
    }

    [Fact]
    public void OnlyTheHyperlinksTheBlobYieldsAreGivenAnIdentifierFromTheExObjList()
    {
        (DffRecordBuffer stream, DffRecordHeader document) = PptHyperlinkFieldTests.Document(
            PptHyperlinkFieldTests.ExObjList(13, 15, 17, 20, 22));

        PptHyperlinks.Read(stream, document, Summary(entries: 5, target: 100, clamp: 700))
            .ShouldBe([13u, 15u], ignoreOrder: true);
    }

    [Fact]
    public void AnUncutBlobLeavesEveryIdentifierResolvable()
    {
        (DffRecordBuffer stream, DffRecordHeader document) = PptHyperlinkFieldTests.Document(
            PptHyperlinkFieldTests.ExObjList(13, 15, 17, 20, 22));

        PptHyperlinks.Read(stream, document, Summary(entries: 5, target: 100, clamp: null))
            .ShouldBe([13u, 15u, 17u, 20u, 22u], ignoreOrder: true);
    }

    [Fact]
    public void ABlobThatYieldsNothingFallsBackToOneEntryPerExHyperlinkRecord()
    {
        // pptin.cxx:551-575: with the list still empty the walk builds an entry per record, so a
        // blob cut before its first entry is the same as no blob at all.
        (DffRecordBuffer stream, DffRecordHeader document) = PptHyperlinkFieldTests.Document(
            PptHyperlinkFieldTests.ExObjList(13, 15, 17));

        PptHyperlinks.Read(stream, document, Summary(entries: 5, target: 100, clamp: 40))
            .ShouldBe([13u, 15u, 17u], ignoreOrder: true);
    }

    /// <summary>
    /// A <c>\u0005DocumentSummaryInformation</c> stream holding one user-defined section, a
    /// dictionary naming <c>_PID_HLINKS</c>, and a blob of the given shape.
    /// </summary>
    /// <param name="entries">How many hyperlinks the blob declares and carries.</param>
    /// <param name="target">The length, in characters, of each entry's target string.</param>
    /// <param name="clamp">
    /// The value <c>nSecSize - nSecOfs</c> is to take, which is what the reference cuts the
    /// property to; null for a section whose declared size cuts nothing.
    /// </param>
    private static byte[] Summary(int entries, int target, int? clamp)
    {
        byte[] blob = Blob(entries, target);
        byte[] dictionary = Dictionary();

        // The section's own layout: size, property count, two (id, offset) pairs, then the two
        // property values.
        int header = 8 + (2 * 8);
        int dictionaryOffset = header;
        int blobOffset = header + dictionary.Length;

        List<byte> section = [];
        section.AddRange(UInt32(clamp is { } cut ? (uint)(SectionOffset + cut) : 0xFFFFFFFF));
        section.AddRange(UInt32(2));
        section.AddRange(UInt32(0));
        section.AddRange(UInt32((uint)dictionaryOffset));
        section.AddRange(UInt32(2));
        section.AddRange(UInt32((uint)blobOffset));
        section.AddRange(dictionary);
        section.AddRange(blob);

        List<byte> stream = [];
        stream.AddRange([0xFE, 0xFF, 0x00, 0x00]);          // byte order, format
        stream.AddRange(UInt32(0x000A0000));                // OS version
        stream.AddRange(new byte[16]);                      // class id
        stream.AddRange(UInt32(1));                         // one section
        stream.AddRange(UserDefined);
        stream.AddRange(UInt32(SectionOffset));
        stream.AddRange(section);
        return [.. stream];
    }

    /// <summary>The <c>_PID_HLINKS</c> blob: six OLE properties per hyperlink.</summary>
    private static byte[] Blob(int entries, int target)
    {
        List<byte> body = [];
        for (int i = 0; i < entries; i++)
        {
            for (int field = 0; field < 4; field++)
            {
                body.AddRange(UInt32(3));                   // VT_I4
                body.AddRange(UInt32((uint)field));
            }

            body.AddRange(String(new string('a', target - 1)));
            body.AddRange(String(string.Empty));
        }

        List<byte> blob = [];
        blob.AddRange(UInt32(65));                          // VT_BLOB
        blob.AddRange(UInt32((uint)(body.Count + 4)));
        blob.AddRange(UInt32((uint)(entries * 6)));
        blob.AddRange(body);
        return [.. blob];
    }

    /// <summary>A <c>VT_LPWSTR</c> as the blob writes it: a character count, then the terminator.</summary>
    private static byte[] String(string value)
    {
        int count = value.Length + 1;
        List<byte> bytes = [];
        bytes.AddRange(UInt32(31));                         // VT_LPWSTR
        bytes.AddRange(UInt32((uint)count));
        foreach (char c in value)
        {
            bytes.Add((byte)c);
            bytes.Add(0);
        }

        bytes.AddRange([0, 0]);
        if ((count & 1) != 0) bytes.AddRange([0, 0]);       // dword alignment
        return [.. bytes];
    }

    /// <summary>Property zero: one name, <c>_PID_HLINKS</c>, against identifier two.</summary>
    private static byte[] Dictionary()
    {
        const string name = "_PID_HLINKS\0";
        List<byte> bytes = [];
        bytes.AddRange(UInt32(1));
        bytes.AddRange(UInt32(2));
        bytes.AddRange(UInt32((uint)name.Length));
        foreach (char c in name) bytes.Add((byte)c);
        while (bytes.Count % 4 != 0) bytes.Add(0);
        return [.. bytes];
    }

    private static byte[] UserDefined =>
    [
        0x05, 0xD5, 0xCD, 0xD5, 0x9C, 0x2E, 0x1B, 0x10,
        0x93, 0x97, 0x08, 0x00, 0x2B, 0x2C, 0xF9, 0xAE,
    ];

    private static byte[] UInt32(uint value)
        => [(byte)value, (byte)(value >> 8), (byte)(value >> 16), (byte)(value >> 24)];
}
