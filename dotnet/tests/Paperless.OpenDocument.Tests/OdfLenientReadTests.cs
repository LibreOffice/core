using System.IO.Compression;
using System.Text;
using Paperless.Core;
using Paperless.Core.Diagnostics;
using Paperless.OpenDocument;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.OpenDocument.Tests;

/// <summary>
/// Tests that damaged and hostile ODF is repaired where possible, skipped where not, and
/// never crashes the reader.
/// </summary>
/// <remarks>
/// Rule five of the project: real files violate their own specifications constantly, so an
/// exception is reserved for input that lenient parsing genuinely cannot recover. These are
/// also the cases a fuzzer would find first, and the readers are pure parsing code over
/// untrusted input.
/// </remarks>
public class OdfLenientReadTests
{
    /// <summary>Builds an ODF package from named parts, replacing or dropping any of them.</summary>
    private static MemoryStream BuildPackage(
        Dictionary<string, string?> replacements)
    {
        MemoryStream result = new();
        using (FileStream original = File.OpenRead(Corpus.Require("prose-odt.odt")))
        using (ZipArchive source = new(original, ZipArchiveMode.Read))
        using (ZipArchive target = new(result, ZipArchiveMode.Create, leaveOpen: true))
        {
            foreach (ZipArchiveEntry entry in source.Entries)
            {
                if (replacements.TryGetValue(entry.FullName, out string? replacement))
                {
                    // A null replacement means "leave this part out entirely".
                    if (replacement is null) continue;

                    using Stream created = target.CreateEntry(entry.FullName).Open();
                    created.Write(Encoding.UTF8.GetBytes(replacement));
                    continue;
                }

                using Stream input = entry.Open();
                using Stream output = target.CreateEntry(entry.FullName).Open();
                input.CopyTo(output);
            }
        }
        result.Position = 0;
        return result;
    }

    [Fact]
    public void AMalformedStylesPartIsSkippedAndTheContentStillReads()
    {
        // Losing styles.xml costs formatting, not text. Refusing the document over it would
        // throw away everything the user actually wants.
        using MemoryStream package = BuildPackage(new() { ["styles.xml"] = "<broken" });
        using OdfFile file = OdfFile.Open(package);

        file.Body.ShouldNotBeNull();
        file.Diagnostics.ShouldContain(d => d.Code == "PL2010" && d.Severity == DiagnosticSeverity.Error);
        file.Styles.NamedStyles.ShouldBeEmpty();
    }

    [Fact]
    public void AMissingStylesPartIsNotAnError()
    {
        using MemoryStream package = BuildPackage(new() { ["styles.xml"] = null });
        using OdfFile file = OdfFile.Open(package);

        file.Body.ShouldNotBeNull();
        file.Diagnostics.ShouldNotContain(d => d.Code == "PL2010");
    }

    [Fact]
    public void AMissingContentPartIsFatalBecauseThereIsNothingToRead()
    {
        using MemoryStream package = BuildPackage(new() { ["content.xml"] = null });

        MalformedDocumentException thrown =
            Should.Throw<MalformedDocumentException>(() => OdfFile.Open(package));
        thrown.Message.ShouldContain("content.xml");
    }

    [Fact]
    public void AMalformedContentPartReportsWhereTheXmlBroke()
    {
        using MemoryStream package = BuildPackage(new() { ["content.xml"] = "<a><unclosed>" });

        // The parser's position is included: "malformed" alone is not actionable, and a
        // truncated part and a namespace error need different responses.
        Should.Throw<MalformedDocumentException>(() => OdfFile.Open(package))
              .Message.ShouldContain("Line 1");
    }

    [Fact]
    public void AnEncryptedPackageAsksForAPasswordRatherThanReadingRubbish()
    {
        // ODF encrypts per entry and leaves 'mimetype' readable, so the manifest's
        // encryption-data is the only signal — and decrypting is not implemented yet, so the
        // honest answer is to say a password is needed rather than to return garbled bytes.
        string manifest = """
            <manifest:manifest
                xmlns:manifest="urn:oasis:names:tc:opendocument:xmlns:manifest:1.0"
                manifest:version="1.3">
              <manifest:file-entry manifest:full-path="/" manifest:version="1.3"
                                   manifest:media-type="application/vnd.oasis.opendocument.text"/>
              <manifest:file-entry manifest:full-path="content.xml" manifest:media-type="text/xml">
                <manifest:encryption-data manifest:checksum-type="SHA1/1K"
                                          manifest:checksum="ZmFrZQ==">
                  <manifest:algorithm manifest:algorithm-name="AES256-CBC"
                                      manifest:initialisation-vector="ZmFrZQ=="/>
                </manifest:encryption-data>
              </manifest:file-entry>
            </manifest:manifest>
            """;
        using MemoryStream package = BuildPackage(new() { ["META-INF/manifest.xml"] = manifest });

        PasswordRequiredException thrown =
            Should.Throw<PasswordRequiredException>(() => OdfFile.Open(package));
        thrown.PasswordWasSupplied.ShouldBeFalse();
    }

    [Fact]
    public void ADocumentWithNoBodyIsReadableForItsMetadataAndStyles()
    {
        string content = $"""
            <office:document-content xmlns:office="{OdfNamespaces.Office}" office:version="1.3"/>
            """;
        using MemoryStream package = BuildPackage(new() { ["content.xml"] = content });
        using OdfFile file = OdfFile.Open(package);

        file.Body.ShouldBeNull();
        // The styles and metadata are still there, and a caller asking only for those should
        // get them rather than an exception.
        file.Styles.NamedStyles.ShouldNotBeEmpty();
        file.Version.ShouldBe("1.3");
    }

    [Fact]
    public void SomethingThatIsNeitherAZipNorXmlIsRejected()
    {
        using MemoryStream notADocument = new(Encoding.UTF8.GetBytes("this is not a document"));
        Should.Throw<MalformedDocumentException>(() => OdfFile.Open(notADocument));
    }

    [Fact]
    public void AnExternalEntityIsNotResolved()
    {
        // Office documents are untrusted input, and an external entity reference would be an
        // XXE and an SSRF. DTD processing is prohibited, so this is a parse failure rather
        // than a fetch.
        string content = """
            <!DOCTYPE content [<!ENTITY xxe SYSTEM "file:///etc/passwd">]>
            <office:document-content
                xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0">&xxe;</office:document-content>
            """;
        using MemoryStream package = BuildPackage(new() { ["content.xml"] = content });

        Should.Throw<MalformedDocumentException>(() => OdfFile.Open(package));
    }

    [Fact]
    public void DeeplyNestedContentIsBoundedRatherThanOverflowingTheStack()
    {
        // A hostile file can nest arbitrarily; the reader recurses, so the depth is bounded
        // and the overflow is reported as a diagnostic rather than as a crash.
        int depth = OdfContentReader.MaxNestingDepth * 4;
        StringBuilder body = new();
        body.Append($"""<office:document-content xmlns:office="{OdfNamespaces.Office}" """)
            .Append($"""xmlns:text="{OdfNamespaces.Text}"><office:body><office:text>""");
        for (int i = 0; i < depth; i++) body.Append("<text:section>");
        body.Append("<text:p>deep</text:p>");
        for (int i = 0; i < depth; i++) body.Append("</text:section>");
        body.Append("</office:text></office:body></office:document-content>");

        using MemoryStream package = BuildPackage(new() { ["content.xml"] = body.ToString() });
        using OdfFile file = OdfFile.Open(package);

        List<Diagnostic> diagnostics = [];
        OdfContentReader reader = new(file, diagnostics);
        Core.Extraction.ContentSection section = new() { Kind = Core.Extraction.SectionKind.Body };
        reader.ReadBlocks(file.Body!, section);

        diagnostics.ShouldContain(d => d.Code == "PL2020");
    }

    [Fact]
    public void ACyclicStyleParentChainDoesNotHang()
    {
        // Illegal, but converters do emit it, and an infinite loop over untrusted input is not
        // an acceptable failure mode.
        string styles = $"""
            <office:document-styles xmlns:office="{OdfNamespaces.Office}"
                                    xmlns:style="{OdfNamespaces.Style}"
                                    xmlns:fo="{OdfNamespaces.FoCompatible}">
              <office:styles>
                <style:style style:name="A" style:family="paragraph" style:parent-style-name="B"/>
                <style:style style:name="B" style:family="paragraph" style:parent-style-name="A"/>
              </office:styles>
            </office:document-styles>
            """;
        using MemoryStream package = BuildPackage(new() { ["styles.xml"] = styles });
        using OdfFile file = OdfFile.Open(package);

        file.Styles.ResolveProperty("A", Styles.OdfStyleFamily.Paragraph,
                                    Styles.OdfPropertyKind.Text,
                                    OdfNamespaces.FoCompatible, "font-size")
            .HasValue.ShouldBeFalse();
    }

    [Fact]
    public void AnAbsurdRepeatCountDoesNotMaterialiseTheWholeSheet()
    {
        // This is exactly how a real spreadsheet describes an empty sheet's extent, so it is
        // not even a hostile case — expanding it naively is the difference between reading a
        // 7 kB file and allocating billions of cells.
        string content = $"""
            <office:document-content xmlns:office="{OdfNamespaces.Office}"
                                     xmlns:table="{OdfNamespaces.Table}"
                                     xmlns:text="{OdfNamespaces.Text}">
              <office:body><office:spreadsheet>
                <table:table table:name="Big">
                  <table:table-row>
                    <table:table-cell office:value-type="string"><text:p>one</text:p></table:table-cell>
                    <table:table-cell table:number-columns-repeated="16000"/>
                  </table:table-row>
                  <table:table-row table:number-rows-repeated="1048570">
                    <table:table-cell table:number-columns-repeated="16000"/>
                  </table:table-row>
                </table:table>
              </office:spreadsheet></office:body>
            </office:document-content>
            """;
        using MemoryStream package = BuildPackage(new() { ["content.xml"] = content });
        using OdfFile file = OdfFile.Open(package);

        List<Diagnostic> diagnostics = [];
        OdfContentReader reader = new(file, diagnostics);
        Core.Extraction.ContentTable table = reader.ReadTableElement(
            file.Body!.Elements().First());

        // One row with one cell: the padding columns and the million padding rows are all
        // trailing, so none of them is inside the used range.
        table.Children.Count.ShouldBe(1);
        table.ColumnCount.ShouldBe(1);
        table.GetText().ShouldBe("one\n");
    }
}
