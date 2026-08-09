using System.IO.Compression;
using System.Text;
using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// <c>w:pPr/w:rPr</c> formats the paragraph mark, never the paragraph's runs.
/// </summary>
/// <remarks>
/// <para>
/// ECMA-376 names the element "Run Properties for the Paragraph Mark" and Word applies it to the
/// pilcrow. LibreOffice agrees, visibly: its flat-ODF export of a DOCX puts the element's properties on
/// <c>loext:marker-style-name</c> and an empty <c>text:span</c>, and leaves the paragraph's text in the
/// paragraph style. This reader used to resolve it as the paragraph's character formatting, so every run
/// with no <c>w:rPr</c> of its own — which is most runs — inherited it.
/// </para>
/// <para>
/// Measured against LibreOffice 24.2.7.2 with the probe in <c>dotnet/probes/words-r36</c>. A bold
/// paragraph style whose mark says <c>&lt;w:b w:val="0"/&gt;</c> still draws its text in Liberation Sans
/// Bold; an unstyled paragraph whose mark says <c>&lt;w:b/&gt;&lt;w:sz w:val="48"/&gt;</c> still draws
/// ten-point upright. The corpus case that found it is
/// <c>review-welsh-government-communications-mister-peter-mandelson.docx</c>, whose title is a bold
/// <c>Heading1</c> with exactly that mark, drawn by the reference in DejaVuSans-Bold and by us in
/// DejaVuSans.
/// </para>
/// <para>
/// What the mark <em>does</em> decide is an empty paragraph, which has nothing else in it — hence the
/// last test here. Dropping the mark's properties outright rather than moving them is the obvious fix
/// and the wrong one: it takes the height off every spacer paragraph in the corpus.
/// </para>
/// </remarks>
public sealed class ParagraphMarkPropertiesTests
{
    /// <summary>A run with no properties of its own keeps the style's bold when the mark clears it.</summary>
    [Fact]
    public void TheMarkDoesNotUnboldTheText()
    {
        PageParagraph paragraph = Read(
            markProperties: """<w:rPr><w:b w:val="0"/></w:rPr>""",
            style: "BoldHead");

        paragraph.Face.Weight.ShouldBeGreaterThanOrEqualTo(700);
    }

    /// <summary>And does not lend the text a size the style never gave it.</summary>
    [Fact]
    public void TheMarkDoesNotResizeTheText()
    {
        PageParagraph paragraph = Read(
            markProperties: """<w:rPr><w:sz w:val="48"/></w:rPr>""",
            style: null);

        paragraph.EmSize.ShouldBe(Length.FromPoints(10));
    }

    /// <summary>Nor bold it.</summary>
    [Fact]
    public void TheMarkDoesNotBoldTheText()
    {
        PageParagraph paragraph = Read(
            markProperties: """<w:rPr><w:b/></w:rPr>""",
            style: null);

        paragraph.Face.Weight.ShouldBeLessThan(700);
    }

    /// <summary>A run that states the property still wins, which is the ordinary cascade.</summary>
    [Fact]
    public void ARunsOwnPropertiesStillApply()
    {
        PageParagraph paragraph = Read(
            markProperties: """<w:rPr><w:b w:val="0"/></w:rPr>""",
            style: null,
            runProperties: """<w:rPr><w:b/></w:rPr>""");

        PageRun run = paragraph.Runs.ShouldHaveSingleItem();
        run.Face.Weight.ShouldBeGreaterThanOrEqualTo(700);
    }

    /// <summary>
    /// An empty paragraph is its mark, so the mark still sizes it.
    /// </summary>
    /// <remarks>
    /// The reference's flat-ODF export writes <c>&lt;text:p loext:marker-style-name="T4"/&gt;</c> with
    /// the size on <c>T4</c> and nowhere else, and the paragraph after it sits 36 pt lower on the page.
    /// </remarks>
    [Fact]
    public void AnEmptyParagraphIsStillSizedByItsMark()
    {
        PageParagraph paragraph = Read(
            markProperties: """<w:rPr><w:sz w:val="48"/></w:rPr>""",
            style: null,
            text: "");

        paragraph.EmSize.ShouldBe(Length.FromPoints(24));
    }

    private static PageParagraph Read(
        string markProperties,
        string? style,
        string runProperties = "",
        string text = "Body")
    {
        using IDocument document = Open(markProperties, style, runProperties, text);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        return pages.Blocks.OfType<PageParagraph>().First();
    }

    private static IDocument Open(string mark, string? style, string runProperties, string text)
    {
        MemoryStream package = BuildPackage(mark, style, runProperties, text);
        using DocumentSource source = DocumentSource.FromStream(package, "paragraph-mark.docx");
        return new WordProcessingReader().Read(source);
    }

    private static MemoryStream BuildPackage(
        string mark, string? style, string runProperties, string text)
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

        // Every face and size stated outright. A probe with no styles part lays out in whatever the
        // reader's fallback happens to be, and then a size assertion is an assertion about the
        // fallback rather than about the rule under test.
        const string Styles = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:docDefaults>
                <w:rPrDefault>
                  <w:rPr>
                    <w:rFonts w:ascii="Liberation Serif" w:hAnsi="Liberation Serif"/>
                    <w:sz w:val="20"/>
                  </w:rPr>
                </w:rPrDefault>
              </w:docDefaults>
              <w:style w:type="paragraph" w:default="1" w:styleId="Normal">
                <w:name w:val="Normal"/>
              </w:style>
              <w:style w:type="paragraph" w:styleId="BoldHead">
                <w:name w:val="BoldHead"/>
                <w:basedOn w:val="Normal"/>
                <w:rPr>
                  <w:rFonts w:ascii="Liberation Sans" w:hAnsi="Liberation Sans"/>
                  <w:b/><w:sz w:val="36"/>
                </w:rPr>
              </w:style>
            </w:styles>
            """;

        string named = style is null ? "" : $"""<w:pStyle w:val="{style}"/>""";
        string run = text.Length == 0
            ? ""
            : $"""<w:r>{runProperties}<w:t xml:space="preserve">{text}</w:t></w:r>""";

        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:body>
                <w:p><w:pPr>{named}{mark}</w:pPr>{run}</w:p>
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
            Write(archive, "word/document.xml", document);
        }

        result.Position = 0;
        return result;

        static void Write(ZipArchive archive, string name, string content)
        {
            using Stream entry = archive.CreateEntry(name).Open();
            entry.Write(Encoding.UTF8.GetBytes(content));
        }
    }
}
