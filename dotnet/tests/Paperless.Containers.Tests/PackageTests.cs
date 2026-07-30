using System.IO.Compression;
using System.Text;
using Paperless.Containers.OpenDocument;
using Paperless.Containers.Ooxml;
using Paperless.Containers.Zip;
using Paperless.Core;
using Paperless.TestKit;

namespace Paperless.Containers.Tests;

public class OpcPackageTests
{
    [Theory]
    [InlineData("prose-docx.docx",
        "application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml")]
    [InlineData("sheet-xlsx.xlsx",
        "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml")]
    [InlineData("slides-pptx.pptx",
        "application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml")]
    public void ResolvesTheMainDocumentPartAndItsContentType(string fileName, string expectedContentType)
    {
        using FileStream stream = File.OpenRead(Corpus.Require(fileName));
        using OpcPackage package = OpcPackage.Open(stream, leaveOpen: true);

        IPackagePart? main = package.GetMainDocumentPart();
        Assert.NotNull(main);
        Assert.Equal(expectedContentType, package.GetMainDocumentContentType());
    }

    [Fact]
    public void FindsTheMainPartByFollowingTheRelationshipNotByGuessingThePath()
    {
        using FileStream stream = File.OpenRead(Corpus.Require("prose-docx.docx"));
        using OpcPackage package = OpcPackage.Open(stream, leaveOpen: true);

        IReadOnlyList<OpcXml.Relationship> rootRels = package.GetRelationships(null);
        Assert.NotEmpty(rootRels);
        Assert.Contains(rootRels, r => r.Type.EndsWith("/officeDocument", StringComparison.Ordinal));

        // Whatever the producer named it, the part the relationship points at must exist.
        IPackagePart main = package.GetMainDocumentPart()!;
        Assert.NotNull(package.GetPart(main.Name));
    }

    [Fact]
    public void AssignsMediaTypesToPartsFromTheContentTypeMap()
    {
        using FileStream stream = File.OpenRead(Corpus.Require("slides-pptx.pptx"));
        using OpcPackage package = OpcPackage.Open(stream, leaveOpen: true);

        // Every XML part should resolve, whether by override or by the '.xml' default.
        List<IPackagePart> xmlParts = [.. package.Parts.Where(p => p.Name.EndsWith(".xml", StringComparison.Ordinal))];
        Assert.NotEmpty(xmlParts);
        Assert.All(xmlParts, p => Assert.False(string.IsNullOrEmpty(p.MediaType), $"{p.Name} has no media type"));
    }

    [Fact]
    public void ReadsPartContent()
    {
        using FileStream stream = File.OpenRead(Corpus.Require("prose-docx.docx"));
        using OpcPackage package = OpcPackage.Open(stream, leaveOpen: true);

        IPackagePart main = package.GetMainDocumentPart()!;
        using Stream content = main.Open();
        using StreamReader reader = new(content);
        string xml = reader.ReadToEnd();

        Assert.Contains("<w:document", xml, StringComparison.Ordinal);
        // The prose source wraps across lines, so the body text must be in there.
        Assert.Contains("Paperless", xml, StringComparison.Ordinal);
    }

    [Fact]
    public void RejectsSomethingThatIsNotAZip()
    {
        using FileStream stream = File.OpenRead(Corpus.Require("prose-doc.doc"));
        Assert.Throws<MalformedDocumentException>(() => OpcPackage.Open(stream, leaveOpen: true));
    }

    [Theory]
    [InlineData("word/document.xml", "word/_rels/document.xml.rels")]
    [InlineData("xl/workbook.xml", "xl/_rels/workbook.xml.rels")]
    [InlineData("ppt/presentation.xml", "ppt/_rels/presentation.xml.rels")]
    [InlineData(null, "_rels/.rels")]
    public void DerivesTheRelationshipsPartName(string? partName, string expected)
        => Assert.Equal(expected, OpcXml.GetRelationshipsPartName(partName));

    [Theory]
    // A relative target resolves against the source part's directory...
    [InlineData("word/document.xml", "styles.xml", "word/styles.xml")]
    [InlineData("word/document.xml", "../customXml/item1.xml", "customXml/item1.xml")]
    // ...while an absolute one is package-rooted. Getting these backwards is a classic bug.
    [InlineData("word/document.xml", "/word/theme/theme1.xml", "word/theme/theme1.xml")]
    [InlineData(null, "word/document.xml", "word/document.xml")]
    public void ResolvesRelationshipTargetsRelativeToTheirSourcePart(
        string? sourcePart, string target, string expected)
    {
        string xml = $"""
            <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
              <Relationship Id="rId1" Type="http://example.com/rel" Target="{target}"/>
            </Relationships>
            """;
        List<OpcXml.Relationship> rels = OpcXml.ParseRelationships(xml, sourcePart);
        Assert.Single(rels);
        Assert.Equal(expected, rels[0].Target);
    }

    [Fact]
    public void MarksExternalTargetsAndLeavesThemUnresolved()
    {
        const string xml = """
            <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
              <Relationship Id="rId1" Type="http://example.com/rel"
                            Target="https://example.com/thing" TargetMode="External"/>
            </Relationships>
            """;
        List<OpcXml.Relationship> rels = OpcXml.ParseRelationships(xml, "word/document.xml");

        Assert.Single(rels);
        Assert.True(rels[0].IsExternal);
        Assert.Equal("https://example.com/thing", rels[0].Target);
    }

    [Fact]
    public void ContentTypeOverridesBeatExtensionDefaults()
    {
        const string xml = """
            <Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
              <Default Extension="xml" ContentType="application/xml"/>
              <Override PartName="/word/document.xml" ContentType="application/special+xml"/>
            </Types>
            """;
        OpcXml.ContentTypeMap map = OpcXml.ContentTypeMap.Parse(xml);

        Assert.Equal("application/special+xml", map.Resolve("word/document.xml"));
        Assert.Equal("application/xml", map.Resolve("word/styles.xml"));
        Assert.Null(map.Resolve("media/image1.png"));
    }

    [Fact]
    public void MalformedXmlYieldsWhatParsedRatherThanThrowing()
    {
        // Truncated mid-document. Partial results beat none: the entries before the break
        // are still valid and may be all the caller needs.
        const string truncated = """
            <Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
              <Default Extension="xml" ContentType="application/xml"/>
            """;
        OpcXml.ContentTypeMap map = OpcXml.ContentTypeMap.Parse(truncated);
        Assert.Equal("application/xml", map.Resolve("word/document.xml"));
    }

    /// <summary>
    /// XML from a document is untrusted, so an external entity must never be resolved --
    /// that would be an XXE and, with a remote system id, an SSRF.
    /// </summary>
    [Fact]
    public void RefusesToProcessADoctypeDeclaration()
    {
        const string withDtd = """
            <?xml version="1.0"?>
            <!DOCTYPE Types [<!ENTITY xxe SYSTEM "file:///etc/passwd">]>
            <Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
              <Default Extension="xml" ContentType="&xxe;"/>
            </Types>
            """;
        // Parsing stops at the prohibited DTD, so nothing is resolved and nothing leaks.
        OpcXml.ContentTypeMap map = OpcXml.ContentTypeMap.Parse(withDtd);
        Assert.Null(map.Resolve("word/document.xml"));
    }
}

public class OdfPackageTests
{
    [Theory]
    [InlineData("prose-odt.odt", "application/vnd.oasis.opendocument.text")]
    [InlineData("prose-ott.ott", "application/vnd.oasis.opendocument.text-template")]
    [InlineData("sheet-ods.ods", "application/vnd.oasis.opendocument.spreadsheet")]
    [InlineData("slides-odp.odp", "application/vnd.oasis.opendocument.presentation")]
    public void ReadsTheMimetypeEntry(string fileName, string expected)
    {
        using FileStream stream = File.OpenRead(Corpus.Require(fileName));
        using OdfPackage package = OdfPackage.Open(stream, leaveOpen: true);

        Assert.Equal(expected, package.MimeType);
        Assert.False(package.IsEncrypted);
    }

    [Fact]
    public void MimetypeAndManifestAgreeOnAWellFormedPackage()
    {
        using FileStream stream = File.OpenRead(Corpus.Require("prose-odt.odt"));
        using OdfPackage package = OdfPackage.Open(stream, leaveOpen: true);

        Assert.Equal(package.MimeType, package.ManifestRootMediaType);
        Assert.DoesNotContain(package.Diagnostics, d => d.Code == "PL1221");
    }

    [Fact]
    public void ExposesTheStandardParts()
    {
        using FileStream stream = File.OpenRead(Corpus.Require("prose-odt.odt"));
        using OdfPackage package = OdfPackage.Open(stream, leaveOpen: true);

        Assert.NotNull(package.GetPart(OdfPackage.PartNames.Content));
        Assert.NotNull(package.GetPart(OdfPackage.PartNames.Styles));
        Assert.NotNull(package.GetPart(OdfPackage.PartNames.Meta));

        using Stream? content = package.OpenContent();
        Assert.NotNull(content);
        using StreamReader reader = new(content);
        string xml = reader.ReadToEnd();
        Assert.Contains("office:document-content", xml, StringComparison.Ordinal);
        Assert.Contains("Paperless", xml, StringComparison.Ordinal);
    }

    [Fact]
    public void AssignsMediaTypesFromTheManifest()
    {
        using FileStream stream = File.OpenRead(Corpus.Require("prose-odt.odt"));
        using OdfPackage package = OdfPackage.Open(stream, leaveOpen: true);

        IPackagePart content = package.GetPart(OdfPackage.PartNames.Content)!;
        Assert.Equal("text/xml", content.MediaType);
    }

    /// <summary>
    /// A mimetype/manifest mismatch is a diagnostic, not a failure: LibreOffice throws, but
    /// there is no upside to refusing a document whose two self-descriptions merely disagree.
    /// </summary>
    [Fact]
    public void ReportsAMimetypeManifestMismatchWithoutFailing()
    {
        byte[] package = BuildOdfPackage(
            mimetype: "application/vnd.oasis.opendocument.text",
            manifestRootMediaType: "application/vnd.oasis.opendocument.spreadsheet");

        using MemoryStream stream = new(package);
        using OdfPackage odf = OdfPackage.Open(stream, leaveOpen: true);

        // 'mimetype' is authoritative, and the disagreement is recorded.
        Assert.Equal("application/vnd.oasis.opendocument.text", odf.MimeType);
        Assert.Contains(odf.Diagnostics, d => d.Code == "PL1221");
    }

    [Fact]
    public void DetectsEncryptionFromTheManifest()
    {
        byte[] package = BuildOdfPackage(
            mimetype: "application/vnd.oasis.opendocument.text",
            manifestRootMediaType: "application/vnd.oasis.opendocument.text",
            encrypted: true);

        using MemoryStream stream = new(package);
        using OdfPackage odf = OdfPackage.Open(stream, leaveOpen: true);

        Assert.True(odf.IsEncrypted);
    }

    private static byte[] BuildOdfPackage(
        string mimetype, string manifestRootMediaType, bool encrypted = false)
    {
        string encryptionData = encrypted
            ? """<manifest:encryption-data manifest:checksum-type="SHA1/1K" manifest:checksum="x"/>"""
            : "";

        MemoryStream buffer = new();
        using (ZipArchive archive = new(buffer, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "mimetype", mimetype);
            Write(archive, "META-INF/manifest.xml", $"""
                <manifest:manifest xmlns:manifest="urn:oasis:names:tc:opendocument:xmlns:manifest:1.0">
                  <manifest:file-entry manifest:full-path="/" manifest:media-type="{manifestRootMediaType}"/>
                  <manifest:file-entry manifest:full-path="content.xml" manifest:media-type="text/xml">
                    {encryptionData}
                  </manifest:file-entry>
                </manifest:manifest>
                """);
            Write(archive, "content.xml",
                  "<office:document-content xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\"/>");
        }
        return buffer.ToArray();

        static void Write(ZipArchive archive, string name, string content)
        {
            using Stream s = archive.CreateEntry(name).Open();
            byte[] bytes = Encoding.UTF8.GetBytes(content);
            s.Write(bytes, 0, bytes.Length);
        }
    }
}

public class ZipPackageSafetyTests
{
    [Theory]
    [InlineData("word/document.xml", "word/document.xml")]
    [InlineData("/word/document.xml", "word/document.xml")]
    [InlineData("word\\document.xml", "word/document.xml")]
    [InlineData("word/./document.xml", "word/document.xml")]
    [InlineData("word/sub/../document.xml", "word/document.xml")]
    // A traversal attempt cannot climb above the package root.
    [InlineData("../../../etc/passwd", "etc/passwd")]
    [InlineData("word/../../../../etc/passwd", "etc/passwd")]
    public void NormalisesPartNamesAndContainsTraversalAttempts(string input, string expected)
        => Assert.Equal(expected, ZipPackageBase.NormalisePartName(input));

    /// <summary>
    /// The per-part size cap is the guard that actually bounds the work, so it is tested
    /// against a genuine bomb: 8 MiB of zeroes, which deflate squeezes to a few kB.
    /// </summary>
    [Fact]
    public void RefusesToExpandAZipBombViaThePartSizeCap()
    {
        using MemoryStream buffer = BuildBomb(8 * 1024 * 1024);

        // 1 MiB cap: well above any real content.xml, well below the bomb.
        ZipPackageBase.Limits limits = new() { MaxPartBytes = 1024 * 1024 };
        using OdfPackage package = OdfPackage.Open(buffer, leaveOpen: true, limits);

        MalformedDocumentException ex =
            Assert.Throws<MalformedDocumentException>(() => package.GetPart("content.xml")!.Open());
        Assert.Contains("above the", ex.Message, StringComparison.OrdinalIgnoreCase);
    }

    /// <summary>
    /// The compression-ratio guard is narrow by necessity: DEFLATE tops out near 1028:1 while
    /// legitimate repetitive office XML reaches ~342:1, so only near-maximal payloads are
    /// distinguishable. This asserts it fires where it can, at the default threshold.
    /// </summary>
    [Fact]
    public void RefusesAPartCompressedAtNearlyDeflatesMaximumRatio()
    {
        // All-zero content reaches ~1028:1, just above the 1024:1 default.
        using MemoryStream buffer = BuildBomb(8 * 1024 * 1024);
        using OdfPackage package = OdfPackage.Open(buffer, leaveOpen: true);

        MalformedDocumentException ex =
            Assert.Throws<MalformedDocumentException>(() => package.GetPart("content.xml")!.Open());
        Assert.Contains("compression ratio", ex.Message, StringComparison.OrdinalIgnoreCase);
    }

    /// <summary>
    /// Realistic office XML compresses well -- around 342:1 for repetitive markup -- and must
    /// not trip the ratio guard. This is the false-positive side of the same threshold.
    /// </summary>
    [Fact]
    public void AllowsRealisticallyCompressibleOfficeXml()
    {
        byte[] xml = Encoding.UTF8.GetBytes(
            string.Concat(Enumerable.Repeat("<w:p><w:r><w:t>Hello world</w:t></w:r></w:p>\n", 20_000)));

        MemoryStream buffer = new();
        using (ZipArchive archive = new(buffer, ZipArchiveMode.Create, leaveOpen: true))
        {
            WriteText(archive, "mimetype", "application/vnd.oasis.opendocument.text");
            using Stream s = archive.CreateEntry("content.xml", CompressionLevel.SmallestSize).Open();
            s.Write(xml, 0, xml.Length);
        }
        buffer.Position = 0;

        using OdfPackage package = OdfPackage.Open(buffer, leaveOpen: true);
        using Stream content = package.GetPart("content.xml")!.Open();
        Assert.True(content.CanRead);
    }

    /// <summary>
    /// Many individually-innocuous parts can still exhaust memory together, so the total is
    /// capped as well as each part.
    /// </summary>
    [Fact]
    public void RefusesOnceTheTotalExtractionBudgetIsSpent()
    {
        MemoryStream buffer = new();
        using (ZipArchive archive = new(buffer, ZipArchiveMode.Create, leaveOpen: true))
        {
            WriteText(archive, "mimetype", "application/vnd.oasis.opendocument.text");
            for (int i = 0; i < 8; i++)
            {
                using Stream s = archive.CreateEntry($"part{i}.bin", CompressionLevel.SmallestSize).Open();
                byte[] chunk = new byte[256 * 1024];
                s.Write(chunk, 0, chunk.Length);
            }
        }
        buffer.Position = 0;

        // Each part is fine on its own; together they exceed the budget.
        ZipPackageBase.Limits limits = new()
        {
            MaxPartBytes = 1024 * 1024,
            MaxTotalUncompressedBytes = 600 * 1024,
            MaxCompressionRatio = int.MaxValue,
        };
        using OdfPackage package = OdfPackage.Open(buffer, leaveOpen: true, limits);

        Assert.Throws<MalformedDocumentException>(() =>
        {
            foreach (IPackagePart part in package.Parts.OrderBy(p => p.Name, StringComparer.Ordinal))
            {
                using Stream s = part.Open();
            }
        });
    }

    private static MemoryStream BuildBomb(int uncompressedBytes)
    {
        MemoryStream buffer = new();
        using (ZipArchive archive = new(buffer, ZipArchiveMode.Create, leaveOpen: true))
        {
            WriteText(archive, "mimetype", "application/vnd.oasis.opendocument.text");
            using Stream s = archive.CreateEntry("content.xml", CompressionLevel.SmallestSize).Open();
            byte[] zeroes = new byte[uncompressedBytes];
            s.Write(zeroes, 0, zeroes.Length);
        }
        buffer.Position = 0;
        return buffer;
    }

    private static void WriteText(ZipArchive archive, string name, string content)
    {
        using Stream s = archive.CreateEntry(name).Open();
        byte[] bytes = Encoding.UTF8.GetBytes(content);
        s.Write(bytes, 0, bytes.Length);
    }

    [Fact]
    public void RefusesAPartLargerThanTheConfiguredLimit()
    {
        MemoryStream buffer = new();
        using (ZipArchive archive = new(buffer, ZipArchiveMode.Create, leaveOpen: true))
        {
            using Stream s = archive.CreateEntry("mimetype", CompressionLevel.NoCompression).Open();
            byte[] bytes = Encoding.UTF8.GetBytes(new string('x', 4096));
            s.Write(bytes, 0, bytes.Length);
        }
        buffer.Position = 0;

        // A deliberately tiny limit, so the guard is what fails rather than memory.
        ZipPackageBase.Limits tight = new() { MaxPartBytes = 1024 };
        using OdfPackage package = OdfPackage.Open(buffer, leaveOpen: true, tight);

        IPackagePart part = package.GetPart("mimetype")!;
        Assert.Throws<MalformedDocumentException>(() => part.Open());
    }
}
