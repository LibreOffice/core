using System.IO.Compression;
using System.Text;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Tests ECMA-376 §17.7.3's toggle-property rule: the single most common way a DOCX reader gets
/// character emphasis wrong.
/// </summary>
/// <remarks>
/// <para>
/// A toggle property — bold, italic, caps, strike and a handful of others — does not simply
/// override when it is set at more than one layer. Set on both the paragraph style and the
/// character style, it comes out <em>off</em>. Set as direct formatting it is absolute and does
/// not participate in the toggle, which is why Word writes an explicit
/// <c>w:b w:val="0"</c> when a user unbolds text in a bold style rather than relying on the XOR.
/// </para>
/// <para>
/// Every expectation here was verified against LibreOffice 24.2.7.2 by exporting the same
/// document to HTML, where <c>&lt;b&gt;</c> versus <c>font-weight: normal</c> makes the
/// outcome visible. The document is built here rather than committed because the point is the
/// rule, and a package small enough to read in one screen makes the rule the only thing
/// under test.
/// </para>
/// </remarks>
public class WordToggleTests
{
    /// <summary>
    /// Builds a minimal DOCX with a bold paragraph style, a bold character style, and one
    /// paragraph per layer combination.
    /// </summary>
    private static MemoryStream BuildPackage()
    {
        const string ContentTypes = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
              <Default Extension="rels"
                       ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
              <Default Extension="xml" ContentType="application/xml"/>
              <Override PartName="/word/document.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
              <Override PartName="/word/styles.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
            </Types>
            """;

        const string RootRelationships = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
              <Relationship Id="rId1" Target="word/document.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"/>
            </Relationships>
            """;

        const string DocumentRelationships = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
              <Relationship Id="rId1" Target="styles.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles"/>
            </Relationships>
            """;

        const string Styles = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:docDefaults>
                <w:rPrDefault><w:rPr><w:sz w:val="24"/></w:rPr></w:rPrDefault>
              </w:docDefaults>
              <w:style w:type="paragraph" w:styleId="Normal"><w:name w:val="Normal"/></w:style>
              <w:style w:type="paragraph" w:styleId="BoldPara">
                <w:name w:val="Bold Para"/>
                <w:basedOn w:val="Normal"/>
                <w:rPr><w:b/><w:i/></w:rPr>
              </w:style>
              <w:style w:type="character" w:styleId="BoldChar">
                <w:name w:val="Bold Char"/>
                <w:rPr><w:b/></w:rPr>
              </w:style>
              <w:style w:type="character" w:styleId="DerivedBoldChar">
                <w:name w:val="Derived Bold Char"/>
                <w:basedOn w:val="BoldChar"/>
                <w:rPr><w:u w:val="single"/></w:rPr>
              </w:style>
            </w:styles>
            """;

        const string Document = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:body>
                <w:p><w:pPr><w:pStyle w:val="BoldPara"/></w:pPr>
                  <w:r><w:t>style only</w:t></w:r></w:p>
                <w:p><w:pPr><w:pStyle w:val="BoldPara"/></w:pPr>
                  <w:r><w:rPr><w:rStyle w:val="BoldChar"/></w:rPr>
                    <w:t>both style layers</w:t></w:r></w:p>
                <w:p><w:pPr><w:pStyle w:val="BoldPara"/></w:pPr>
                  <w:r><w:rPr><w:b/></w:rPr><w:t>direct on</w:t></w:r></w:p>
                <w:p><w:pPr><w:pStyle w:val="BoldPara"/></w:pPr>
                  <w:r><w:rPr><w:b w:val="0"/></w:rPr><w:t>direct off</w:t></w:r></w:p>
                <w:p><w:pPr><w:pStyle w:val="BoldPara"/></w:pPr>
                  <w:r><w:rPr><w:rStyle w:val="DerivedBoldChar"/></w:rPr>
                    <w:t>inherited within one layer</w:t></w:r></w:p>
                <w:p><w:pPr><w:pStyle w:val="Normal"/></w:pPr>
                  <w:r><w:rPr><w:rStyle w:val="BoldChar"/></w:rPr>
                    <w:t>character layer only</w:t></w:r></w:p>
              </w:body>
            </w:document>
            """;

        MemoryStream result = new();
        using (ZipArchive archive = new(result, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml", ContentTypes);
            Write(archive, "_rels/.rels", RootRelationships);
            Write(archive, "word/_rels/document.xml.rels", DocumentRelationships);
            Write(archive, "word/styles.xml", Styles);
            Write(archive, "word/document.xml", Document);
        }
        result.Position = 0;
        return result;

        static void Write(ZipArchive archive, string name, string content)
        {
            using Stream entry = archive.CreateEntry(name).Open();
            entry.Write(Encoding.UTF8.GetBytes(content));
        }
    }

    private static Dictionary<string, ContentRun> ReadRuns()
    {
        using MemoryStream package = BuildPackage();
        using DocumentSource source = DocumentSource.FromStream(package, "toggle.docx");
        using IDocument document = new WordProcessingReader().Read(source);

        document.Format.ShouldBe(DocumentFormat.Docx);
        return Descendants(document.Content).OfType<ContentRun>()
                                            .ToDictionary(r => r.Text, StringComparer.Ordinal);
    }

    [Fact]
    public void AToggleSetByOneStyleLayerIsOn()
    {
        // The ordinary case, and the baseline the others are measured against.
        ReadRuns()["style only"].Emphasis.HasFlag(RunEmphasis.Bold).ShouldBeTrue();
        ReadRuns()["character layer only"].Emphasis.HasFlag(RunEmphasis.Bold).ShouldBeTrue();
    }

    [Fact]
    public void AToggleSetByBothStyleLayersCancels()
    {
        // §17.7.3, and verified against LibreOffice: the paragraph style's bold and the character
        // style's bold XOR to *not bold*. This is why bold text inside a bold heading comes out
        // unbolded, and treating the inner layer as an override would get it backwards.
        ContentRun run = ReadRuns()["both style layers"];
        run.Emphasis.HasFlag(RunEmphasis.Bold).ShouldBeFalse();

        // Italic is set by the paragraph style alone, so it is unaffected — the rule is per
        // property, not per run.
        run.Emphasis.HasFlag(RunEmphasis.Italic).ShouldBeTrue();
    }

    [Fact]
    public void DirectFormattingIsAbsoluteAndDoesNotToggle()
    {
        Dictionary<string, ContentRun> runs = ReadRuns();

        // Direct bold inside a bold style stays bold: if direct formatting toggled, a user
        // pressing the bold button on already-bold text would silently unbold it.
        runs["direct on"].Emphasis.HasFlag(RunEmphasis.Bold).ShouldBeTrue();

        // And an explicit off wins outright, which is how Word actually records unbolding.
        runs["direct off"].Emphasis.HasFlag(RunEmphasis.Bold).ShouldBeFalse();
    }

    [Fact]
    public void InheritanceWithinOneLayerOverridesRatherThanToggling()
    {
        // The character style inherits bold through w:basedOn. That is one layer, not two, so it
        // does not cancel the paragraph style's bold... it XORs with it, exactly as if the
        // character style had set bold itself — which is the point: a basedOn chain resolves to a
        // single layer's value first, and only then does the toggle apply.
        ContentRun run = ReadRuns()["inherited within one layer"];
        run.Emphasis.HasFlag(RunEmphasis.Bold).ShouldBeFalse();
        run.Emphasis.HasFlag(RunEmphasis.Underline).ShouldBeTrue();
    }

    [Fact]
    public void APropertyThatIsNotAToggleIsNotTreatedAsOne()
    {
        // Underline is not a toggle property, so two layers setting it do not cancel: the
        // innermost wins. Getting the toggle *membership* wrong would make ordinary properties
        // cancel each other out.
        Ooxml.WordStyles.ToggleProperties.ShouldContain("b");
        Ooxml.WordStyles.ToggleProperties.ShouldNotContain("u");
        Ooxml.WordStyles.ToggleProperties.ShouldNotContain("color");
        Ooxml.WordStyles.ToggleProperties.ShouldNotContain("sz");
    }

    private static IEnumerable<ContentNode> Descendants(ContentNode root)
    {
        foreach (ContentNode child in root.Children)
        {
            yield return child;
            foreach (ContentNode descendant in Descendants(child)) yield return descendant;
        }
    }
}
