using System.IO.Compression;
using System.Text;
using Paperless.Core.Documents;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// That the readers record where a <c>PAGE</c> field's cached result sits, so that layout can replace it.
/// </summary>
/// <remarks>
/// <para>
/// <see cref="PageNumberFieldTests"/> proves the substitution; this proves the readers hand it anything
/// to substitute. The two halves fail independently and the corpus effect needs both — a reader that
/// records no span leaves the cached number on the page with every layout test still green.
/// </para>
/// <para>
/// Synthetic rather than corpus documents, because what is under test is a *shape* — a field bracketed by
/// <c>w:fldChar</c> markers, and ODF's typed element — rather than any one file. The DOCX carries a
/// <c>word/settings.xml</c> for the reason <see cref="ContinuousSectionGeometryTests"/> gives: without
/// one a hand-built package misses LibreOffice's OOXML compatibility defaults.
/// </para>
/// </remarks>
public sealed class PageNumberReaderTests
{
    /// <summary>
    /// A DOCX footer's <c>PAGE</c> field prints each page's own number rather than the cached one.
    /// </summary>
    /// <remarks>
    /// The instruction is split across two <c>w:instrText</c> runs on purpose. Word does that freely —
    /// <c>PAGE  \* MERGE</c> and <c>FORMAT</c> is ordinary — and a reader that looks at one element at a
    /// time reads a name that is not the field's.
    /// </remarks>
    [Fact]
    public void ADocxFooterFieldIsRecordedAndResolved()
    {
        IReadOnlyList<LaidOutPage> pages = Paginate(BuildPackage(), "pagefield.docx");

        pages.Count.ShouldBeGreaterThan(2);
        FooterText(pages[0]).ShouldBe("Page 1.");
        FooterText(pages[1]).ShouldBe("Page 2.");
        FooterText(pages[2]).ShouldBe("Page 3.");
    }

    /// <summary>An ODF footer's <c>text:page-number</c> likewise.</summary>
    /// <remarks>
    /// Checked against LibreOffice on the same document before this was written: its pages one to three
    /// print "Sheet 1 follows", "Sheet 2 follows", "Sheet 3 follows", and ours printed "Sheet 7 follows"
    /// on all of them.
    /// </remarks>
    [Fact]
    public void AnOdfFooterFieldIsRecordedAndResolved()
    {
        IReadOnlyList<LaidOutPage> pages = Paginate(
            new MemoryStream(Encoding.UTF8.GetBytes(FlatOdf)), "pagefield.fodt");

        pages.Count.ShouldBeGreaterThan(2);
        FooterText(pages[0]).ShouldBe("Sheet 1 follows");
        FooterText(pages[2]).ShouldBe("Sheet 3 follows");
    }

    private static string FooterText(LaidOutPage page)
    {
        page.Footer.ShouldNotBeNull();
        return string.Concat(page.Footer!.Blocks.OfType<PageParagraph>().Select(p => p.Text));
    }

    private static IReadOnlyList<LaidOutPage> Paginate(MemoryStream bytes, string name)
    {
        using DocumentSource source = DocumentSource.FromStream(bytes, name);
        using IDocument document = new WordProcessingReader().Read(source);

        return ((WordProcessingPages)((IPaginatedDocument)document).Layout()).Pages;
    }

    private const string FlatOdf = """
        <?xml version="1.0" encoding="UTF-8"?>
        <office:document
            xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
            xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
            xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
            xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
            office:version="1.3"
            office:mimetype="application/vnd.oasis.opendocument.text">
         <office:automatic-styles>
          <style:page-layout style:name="pm1">
           <style:page-layout-properties fo:page-width="8.5in" fo:page-height="4in"
                fo:margin-top="0.5in" fo:margin-bottom="0.5in"
                fo:margin-left="1in" fo:margin-right="1in" style:num-format="1"/>
           <style:footer-style>
            <style:header-footer-properties fo:min-height="0.2in" fo:margin-top="0.1in"/>
           </style:footer-style>
          </style:page-layout>
         </office:automatic-styles>
         <office:master-styles>
          <style:master-page style:name="Standard" style:page-layout-name="pm1">
           <style:footer>
            <text:p>Sheet <text:page-number text:select-page="current">7</text:page-number> follows</text:p>
           </style:footer>
          </style:master-page>
         </office:master-styles>
         <office:body><office:text>
          <text:p>One</text:p><text:p>Two</text:p><text:p>Three</text:p><text:p>Four</text:p>
          <text:p>Five</text:p><text:p>Six</text:p><text:p>Seven</text:p><text:p>Eight</text:p>
          <text:p>Nine</text:p><text:p>Ten</text:p><text:p>Eleven</text:p><text:p>Twelve</text:p>
          <text:p>Thirteen</text:p><text:p>Fourteen</text:p><text:p>Fifteen</text:p>
          <text:p>Sixteen</text:p><text:p>Seventeen</text:p><text:p>Eighteen</text:p>
          <text:p>Nineteen</text:p><text:p>Twenty</text:p><text:p>Twenty-one</text:p>
          <text:p>Twenty-two</text:p><text:p>Twenty-three</text:p><text:p>Twenty-four</text:p>
          <text:p>Twenty-five</text:p><text:p>Twenty-six</text:p><text:p>Twenty-seven</text:p>
          <text:p>Twenty-eight</text:p><text:p>Twenty-nine</text:p><text:p>Thirty</text:p>
         </office:text></office:body>
        </office:document>
        """;

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
              <Override PartName="/word/footer1.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.footer+xml"/>
              <Override PartName="/word/settings.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
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
              <Relationship Id="rId1" Target="settings.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings"/>
              <Relationship Id="rId2" Target="footer1.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/footer"/>
            </Relationships>
            """;

        const string Settings = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"/>
            """;

        // The cached result is "9", which is what every page printed before this worked. The instruction
        // is split across two runs, as Word writes it.
        const string Footer = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:ftr xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:p>
                <w:r><w:t xml:space="preserve">Page </w:t></w:r>
                <w:r><w:fldChar w:fldCharType="begin"/></w:r>
                <w:r><w:instrText xml:space="preserve"> PA</w:instrText></w:r>
                <w:r><w:instrText xml:space="preserve">GE  \* MERGEFORMAT </w:instrText></w:r>
                <w:r><w:fldChar w:fldCharType="separate"/></w:r>
                <w:r><w:t>9</w:t></w:r>
                <w:r><w:fldChar w:fldCharType="end"/></w:r>
                <w:r><w:t>.</w:t></w:r>
              </w:p>
            </w:ftr>
            """;

        StringBuilder body = new();
        for (int i = 0; i < 40; i++)
        {
            body.Append("<w:p><w:r><w:t>Body paragraph ").Append(i).Append("</w:t></w:r></w:p>");
        }

        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
                        xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
              <w:body>
                {body}
                <w:sectPr>
                  <w:footerReference w:type="default" r:id="rId2"/>
                  <w:pgSz w:w="12240" w:h="4000"/>
                  <w:pgMar w:top="720" w:right="1440" w:bottom="720" w:left="1440"
                           w:header="360" w:footer="360"/>
                </w:sectPr>
              </w:body>
            </w:document>
            """;

        MemoryStream stream = new();
        using (ZipArchive archive = new(stream, ZipArchiveMode.Create, leaveOpen: true))
        {
            Add(archive, "[Content_Types].xml", ContentTypes);
            Add(archive, "_rels/.rels", RootRelationships);
            Add(archive, "word/_rels/document.xml.rels", DocumentRelationships);
            Add(archive, "word/settings.xml", Settings);
            Add(archive, "word/footer1.xml", Footer);
            Add(archive, "word/document.xml", document);
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
