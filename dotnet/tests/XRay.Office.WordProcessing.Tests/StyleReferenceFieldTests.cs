using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// That a <c>STYLEREF … \s</c> quotes the referenced heading's <em>text</em>, as LibreOffice does.
/// </summary>
/// <remarks>
/// <para>
/// Word's <c>\s</c> asks for the referenced paragraph's complete number, and LibreOffice does not
/// implement it: <c>DomainMapper_Impl.cxx</c>:8600 maps <c>\p</c>, <c>\r</c>, <c>\n</c> and <c>\w</c>
/// onto <c>ReferenceFieldPart</c> and has no branch for <c>\s</c>, so the part stays at its default of
/// <c>TEXT</c> and the field draws the heading's text. Reproducing that is what parity means here —
/// the cached result Word left behind is the *other* answer.
/// </para>
/// <para>
/// Measured on <c>words/pagination-001/docx/report-template.docx</c>, whose seven captions read
/// <c>Table 1.2</c> from the cache and <c>Table Main body (Heading 2).2</c> in the reference. The
/// longer text wraps every caption onto a second line, which is a whole page: 19 against the
/// reference's 20 before this, 20 against 20 after.
/// </para>
/// <para>
/// Synthetic rather than that document, because three separate rules are under test and only one of
/// them is visible in it: the bare digit is Word's undocumented shorthand for a built-in heading level
/// (<c>reffld.cxx</c>:1682), a quoted style name is matched by name, and a part switch LibreOffice
/// <em>does</em> implement must leave the cached result alone.
/// </para>
/// </remarks>
public sealed class StyleReferenceFieldTests
{
    /// <summary>A bare digit names the built-in heading of that level, and its text is quoted.</summary>
    [Fact]
    public void ADigitNamesTheBuiltInHeadingAndTheFieldQuotesItsText()
        => BodyText().ShouldContain("[Table Main body.2]");

    /// <summary>A style named in full is matched by its <c>w:name</c>, not by its id.</summary>
    [Fact]
    public void AStyleNamedInFullIsMatchedByItsName()
        => BodyText().ShouldContain("<Appendix title>");

    /// <summary>
    /// A part switch LibreOffice implements keeps the producer's cached result.
    /// </summary>
    /// <remarks>
    /// The conservative half of the rule: <c>\n</c>, <c>\r</c>, <c>\w</c> and <c>\p</c> all ask for
    /// something other than the paragraph's text, and drawing the text for them would be a wrong
    /// substitution rather than a stale one.
    /// </remarks>
    [Fact]
    public void APartSwitchLibreOfficeImplementsKeepsTheCache()
        => BodyText().ShouldContain("{cached}");

    /// <summary>The instruction reader, on the switches that decide whether to substitute at all.</summary>
    [Fact]
    public void TheInstructionReaderNamesTheStyleOnlyWhenTheTextIsWhatIsQuoted()
    {
        FieldInstructions.StyleReferenceName(" STYLEREF 2 \\s ").ShouldBe("2");
        FieldInstructions.StyleReferenceName(" STYLEREF \"Appendix Title\" \\s ")
            .ShouldBe("Appendix Title");
        FieldInstructions.StyleReferenceName(" STYLEREF 2 \\* MERGEFORMAT ").ShouldBe("2");
        FieldInstructions.StyleReferenceName(" STYLEREF 2 \\n ").ShouldBeNull();
        FieldInstructions.StyleReferenceName(" STYLEREF 2 \\w ").ShouldBeNull();
        FieldInstructions.StyleReferenceName(" STYLEREF ").ShouldBeNull();
        FieldInstructions.StyleReferenceName(" PAGEREF _Toc1 \\h ").ShouldBeNull();
    }

    private static string BodyText()
    {
        MemoryStream bytes = BuildPackage();
        using DocumentSource source = DocumentSource.FromStream(bytes, "report.docx");
        using IDocument document = new WordProcessingReader().Read(source);

        var pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        return string.Concat(pages.Paragraphs.Select(p => p.Text));
    }

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

        // The ids deliberately do not look like the names: a field names a style by *name*, a paragraph
        // by id, and matching the two directly would pass on a document whose ids happen to agree.
        const string Styles = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:style w:type="paragraph" w:styleId="H2"><w:name w:val="heading 2"/></w:style>
              <w:style w:type="paragraph" w:styleId="AT"><w:name w:val="Appendix Title"/></w:style>
            </w:styles>
            """;

        const string Document = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:body>
                <w:p><w:pPr><w:pStyle w:val="H2"/></w:pPr><w:r><w:t>Main body</w:t></w:r></w:p>
                <w:p>
                  <w:r><w:t>[Table </w:t></w:r>
                  <w:fldSimple w:instr=" STYLEREF 2 \s "><w:r><w:t>1</w:t></w:r></w:fldSimple>
                  <w:r><w:t>.2]</w:t></w:r>
                </w:p>
                <w:p>
                  <w:r><w:t>{</w:t></w:r>
                  <w:fldSimple w:instr=" STYLEREF 2 \n "><w:r><w:t>cached</w:t></w:r></w:fldSimple>
                  <w:r><w:t>}</w:t></w:r>
                </w:p>
                <w:p><w:pPr><w:pStyle w:val="AT"/></w:pPr><w:r><w:t>Appendix title</w:t></w:r></w:p>
                <w:p>
                  <w:r><w:t>&lt;</w:t></w:r>
                  <w:r><w:fldChar w:fldCharType="begin"/></w:r>
                  <w:r><w:instrText xml:space="preserve"> STYLEREF "Appendix</w:instrText></w:r>
                  <w:r><w:instrText xml:space="preserve"> Title" \s </w:instrText></w:r>
                  <w:r><w:fldChar w:fldCharType="separate"/></w:r>
                  <w:r><w:t>A</w:t></w:r>
                  <w:r><w:fldChar w:fldCharType="end"/></w:r>
                  <w:r><w:t>&gt;</w:t></w:r>
                </w:p>
              </w:body>
            </w:document>
            """;

        MemoryStream stream = new();
        using (ZipArchive archive = new(stream, ZipArchiveMode.Create, leaveOpen: true))
        {
            Add(archive, "[Content_Types].xml", ContentTypes);
            Add(archive, "_rels/.rels", RootRelationships);
            Add(archive, "word/_rels/document.xml.rels", DocumentRelationships);
            Add(archive, "word/styles.xml", Styles);
            Add(archive, "word/document.xml", Document);
        }

        stream.Position = 0;
        return stream;

        static void Add(ZipArchive archive, string name, string content)
        {
            using StreamWriter writer = new(archive.CreateEntry(name).Open(), Encoding.UTF8);
            writer.Write(content);
        }
    }
}
