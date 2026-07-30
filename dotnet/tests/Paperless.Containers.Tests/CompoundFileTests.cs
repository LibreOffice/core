using Paperless.Containers;
using Paperless.Containers.Ole2;
using Paperless.Core;
using Paperless.TestKit;

namespace Paperless.Containers.Tests;

public class CompoundFileTests
{
    [Theory]
    [InlineData("prose-doc.doc", "WordDocument")]
    [InlineData("sheet-xls.xls", "Workbook")]
    [InlineData("slides-ppt.ppt", "PowerPoint Document")]
    public void FindsTheFormatDefiningRootStream(string fileName, string expectedStream)
    {
        using FileStream stream = File.OpenRead(Corpus.Require(fileName));
        using CompoundFile file = CompoundFile.Open(stream, leaveOpen: true);

        Assert.Contains(expectedStream, file.RootStreamNames);
    }

    [Fact]
    public void ReadsTheWordDocumentStreamContents()
    {
        using FileStream stream = File.OpenRead(Corpus.Require("prose-doc.doc"));
        using CompoundFile file = CompoundFile.Open(stream, leaveOpen: true);

        IPackagePart? part = file.GetPart("WordDocument");
        Assert.NotNull(part);

        using Stream content = part.Open();
        byte[] buffer = new byte[8];
        Assert.Equal(8, content.Read(buffer, 0, 8));

        // The WW8 FIB starts with wIdent 0xA5EC for Word 8 documents.
        ushort wIdent = (ushort)(buffer[0] | (buffer[1] << 8));
        Assert.Equal(0xA5EC, wIdent);
    }

    [Fact]
    public void ReadsSmallStreamsThroughTheMiniFat()
    {
        // The summary-information streams are small, so they exercise the mini-stream path
        // rather than the main FAT -- a genuinely different code path worth covering.
        using FileStream stream = File.OpenRead(Corpus.Require("prose-doc.doc"));
        using CompoundFile file = CompoundFile.Open(stream, leaveOpen: true);

        IPackagePart? summary = file.GetPart("SummaryInformation");
        Assert.NotNull(summary);
        Assert.InRange(summary.Length, 1, 4095);   // below the 4096 mini-stream cutoff

        using Stream content = summary.Open();
        byte[] bytes = new byte[summary.Length];
        int read = content.Read(bytes, 0, bytes.Length);
        Assert.Equal(bytes.Length, read);

        // An OLE property set begins with byte order mark 0xFFFE.
        Assert.Equal(0xFE, bytes[0]);
        Assert.Equal(0xFF, bytes[1]);
    }

    [Fact]
    public void EnumeratesPartsIncludingNestedStorages()
    {
        using FileStream stream = File.OpenRead(Corpus.Require("slides-ppt.ppt"));
        using CompoundFile file = CompoundFile.Open(stream, leaveOpen: true);

        List<IPackagePart> parts = [.. file.Parts];
        Assert.NotEmpty(parts);
        // Every part must have a usable name and a non-negative length.
        Assert.All(parts, p =>
        {
            Assert.False(string.IsNullOrEmpty(p.Name));
            Assert.True(p.Length >= 0);
        });
    }

    [Fact]
    public void PartsCanBeOpenedMoreThanOnce()
    {
        using FileStream stream = File.OpenRead(Corpus.Require("sheet-xls.xls"));
        using CompoundFile file = CompoundFile.Open(stream, leaveOpen: true);

        IPackagePart part = file.GetPart("Workbook")!;
        using Stream first = part.Open();
        using Stream second = part.Open();

        Assert.Equal(part.Length, first.Length);
        Assert.Equal(part.Length, second.Length);
    }

    [Fact]
    public void HasSignatureRecognisesCompoundFilesAndRestoresPosition()
    {
        using FileStream doc = File.OpenRead(Corpus.Require("prose-doc.doc"));
        doc.Position = 17;
        Assert.True(CompoundFile.HasSignature(doc));
        Assert.Equal(17, doc.Position);

        using FileStream docx = File.OpenRead(Corpus.Require("prose-docx.docx"));
        Assert.False(CompoundFile.HasSignature(docx));
    }

    [Fact]
    public void GetPartReturnsNullForAnAbsentStream()
    {
        using FileStream stream = File.OpenRead(Corpus.Require("prose-doc.doc"));
        using CompoundFile file = CompoundFile.Open(stream, leaveOpen: true);

        Assert.Null(file.GetPart("NoSuchStream"));
    }

    [Fact]
    public void RejectsNonCompoundFiles()
    {
        using FileStream stream = File.OpenRead(Corpus.Require("prose-docx.docx"));
        Assert.Throws<MalformedDocumentException>(() => CompoundFile.Open(stream, leaveOpen: true));
    }

    [Fact]
    public void RejectsAnEmptyStream()
    {
        using MemoryStream empty = new();
        Assert.Throws<MalformedDocumentException>(() => CompoundFile.Open(empty, leaveOpen: true));
    }

    /// <summary>
    /// When truncation takes the directory with it, nothing is recoverable and the reader
    /// says so. Callers that only need the format still get an answer, because
    /// <c>FormatIdentifier</c> catches this and reports the container it did recognise.
    /// </summary>
    [Fact]
    public void ThrowsWhenTruncationRemovesTheDirectory()
    {
        byte[] full = File.ReadAllBytes(Corpus.Require("slides-ppt.ppt"));
        using MemoryStream truncated = new(full.AsSpan(0, full.Length / 2).ToArray());

        Assert.Throws<MalformedDocumentException>(() => CompoundFile.Open(truncated, leaveOpen: true));
    }

    /// <summary>
    /// The commoner case: the directory survives but a stream's sectors do not. That must
    /// degrade to a short stream plus a diagnostic, never an exception -- the rest of the
    /// document is still perfectly readable.
    /// </summary>
    [Fact]
    public void ReportsAStreamWhoseSectorsAreMissing()
    {
        byte[] file = BuildMinimalCompoundFile(streamStartSector: 2, streamLength: 1 << 20,
                                               cyclic: false);
        using MemoryStream stream = new(file);
        using CompoundFile cfb = CompoundFile.Open(stream, leaveOpen: true);

        Assert.Contains("Truncated", cfb.RootStreamNames);

        IPackagePart part = cfb.GetPart("Truncated")!;
        using Stream content = part.Open();

        // The stream declared 1 MiB; the file holds far less.
        Assert.True(content.Length < 1 << 20);
        Assert.NotEmpty(cfb.Diagnostics);
    }

    /// <summary>
    /// A cyclic FAT chain is the classic hang, so it gets an explicit test: the guard must
    /// stop it. Built by hand because no legitimate producer emits one.
    /// </summary>
    [Fact]
    public void DoesNotHangOnACyclicSectorChain()
    {
        byte[] file = BuildMinimalCompoundFile(streamStartSector: 2, streamLength: 1 << 20,
                                               cyclic: true);
        using MemoryStream stream = new(file);

        // The assertion is simply that this returns at all.
        using CompoundFile cfb = CompoundFile.Open(stream, leaveOpen: true);
        foreach (IPackagePart part in cfb.Parts)
        {
            using Stream s = part.Open();
            _ = s.Length;
        }
    }

    /// <summary>
    /// Builds a minimal 512-byte-sector compound file containing one stream, used to
    /// exercise the damage paths that no legitimate producer emits.
    /// </summary>
    /// <param name="streamStartSector">The stream's first sector.</param>
    /// <param name="streamLength">The length the directory entry declares.</param>
    /// <param name="cyclic">
    /// When true the stream's FAT chain points at itself, which would loop forever without
    /// a guard. When false the chain simply runs off the end of the file.
    /// </param>
    private static byte[] BuildMinimalCompoundFile(uint streamStartSector, ulong streamLength, bool cyclic)
    {
        const int sector = 512;
        byte[] file = new byte[sector * 4];

        // Header.
        ReadOnlySpan<byte> signature = [0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1, 0x1A, 0xE1];
        signature.CopyTo(file);
        BitConverter.TryWriteBytes(file.AsSpan(26), (ushort)0x003E);  // minor version
        BitConverter.TryWriteBytes(file.AsSpan(28), (ushort)0xFFFE);  // byte order
        BitConverter.TryWriteBytes(file.AsSpan(30), (ushort)9);       // 512-byte sectors
        BitConverter.TryWriteBytes(file.AsSpan(32), (ushort)6);       // 64-byte mini sectors
        BitConverter.TryWriteBytes(file.AsSpan(44), 1u);              // one FAT sector
        BitConverter.TryWriteBytes(file.AsSpan(48), 1u);              // directory at sector 1
        BitConverter.TryWriteBytes(file.AsSpan(56), 4096u);           // mini-stream cutoff
        BitConverter.TryWriteBytes(file.AsSpan(60), 0xFFFFFFFEu);     // no mini-FAT
        BitConverter.TryWriteBytes(file.AsSpan(68), 0xFFFFFFFEu);     // no DIFAT chain
        BitConverter.TryWriteBytes(file.AsSpan(76), 0u);              // DIFAT[0] = sector 0

        int fat = sector;
        BitConverter.TryWriteBytes(file.AsSpan(fat + 0 * 4), 0xFFFFFFFDu);  // sector 0 = FAT
        BitConverter.TryWriteBytes(file.AsSpan(fat + 1 * 4), 0xFFFFFFFEu);  // sector 1 = dir end
        // Sector 2 either loops back on itself, or continues into sectors the file lacks.
        BitConverter.TryWriteBytes(file.AsSpan(fat + 2 * 4), cyclic ? 2u : 99u);
        for (int i = 3; i < sector / 4; i++)
            BitConverter.TryWriteBytes(file.AsSpan(fat + i * 4), 0xFFFFFFFFu);

        // Directory in sector 1: root entry, then one stream entry using the cyclic chain.
        int dir = sector * 2;
        WriteDirEntry(file.AsSpan(dir), "Root Entry", type: 5, child: 1, start: 0xFFFFFFFE, length: 0);
        WriteDirEntry(file.AsSpan(dir + 128), cyclic ? "Cyclic" : "Truncated", type: 2, child: -1,
                      start: streamStartSector, length: streamLength);
        return file;

        static void WriteDirEntry(Span<byte> e, string name, byte type, int child, uint start, ulong length)
        {
            System.Text.Encoding.Unicode.GetBytes(name).CopyTo(e);
            BitConverter.TryWriteBytes(e[64..], (ushort)((name.Length + 1) * 2));
            e[66] = type;
            BitConverter.TryWriteBytes(e[68..], 0xFFFFFFFFu);            // no left sibling
            BitConverter.TryWriteBytes(e[72..], 0xFFFFFFFFu);            // no right sibling
            BitConverter.TryWriteBytes(e[76..], child < 0 ? 0xFFFFFFFFu : (uint)child);
            BitConverter.TryWriteBytes(e[116..], start);
            BitConverter.TryWriteBytes(e[120..], length);
        }
    }
}
