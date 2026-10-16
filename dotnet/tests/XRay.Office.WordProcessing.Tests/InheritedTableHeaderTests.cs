using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A header a following section inherits, when the part it would inherit holds no paragraph of its own.
/// </summary>
/// <remarks>
/// <para>
/// §17.10.1 says a section naming no <c>w:headerReference</c> for a slot keeps the one the section above
/// it had, and Word does exactly that. <strong>LibreOffice does not, when the part it would pass down
/// holds nothing but tables.</strong> It copies the previous section's header <em>content</em> from one
/// page style to the next rather than linking them — <c>copyHeaderFooterTextProperty</c> in
/// <c>sw/source/writerfilter/dmapper/PropertyMap.cxx</c> — and a body of text that begins and ends with a
/// table copies as nothing at all. The inheriting section is then left with no running head, and reserves
/// no band for one either.
/// </para>
/// <para>
/// Measured against the installed 26.2.4.2 on a two-section probe varying only the header part's own
/// children: one table alone and two tables alone are <em>not</em> passed down; the same table with an
/// empty <c>w:p</c> before it, the same table with one after it, and a bare paragraph all are. So the
/// rule turns on whether a paragraph is present at all, and where a paragraph is present the tables
/// beside it travel with it.
/// </para>
/// <para>
/// Found on <c>words/extra-001/docx/UG.CAO.00133 Foreign Part 145 approvals - Language.docx</c>, whose
/// header part is one nested table ending <c>&lt;/w:tbl&gt;&lt;/w:hdr&gt;</c> and whose five sections
/// name a default header in only two of them. The reference draws the running head on those two sections'
/// five pages and on none of the other thirteen; we drew it on all eighteen, at 17 extractable words a
/// page. Its sibling
/// <c>UG.CAO.00006 …User Guide for Applicants &amp; Approval Holders.docx</c> is the same shape at 20
/// words over 29 pages, and drawing a header on all of them cost it a page as well as the words.
/// </para>
/// </remarks>
public sealed class InheritedTableHeaderTests
{
    /// <summary>A header holding only a table is not passed down to a section that names none.</summary>
    [Fact]
    public void ATableOnlyHeaderIsNotInheritedByTheNextSection()
    {
        WordProcessingPages pages = Paginate(TableHeader);

        pages.Count.ShouldBe(2, "the fixture has to reach the second section to be worth asserting");
        HeaderText(pages.Pages[0]).ShouldBe(
            "head", "the section that names the part still draws it");
        pages.Pages[1].Header.ShouldBeNull(
            "and the section below it inherits nothing, because there is no paragraph to copy");
    }

    /// <summary>The same table with an empty paragraph after it is passed down whole.</summary>
    [Fact]
    public void ATableHeaderWithATrailingParagraphIsInherited()
    {
        WordProcessingPages pages = Paginate(TableHeader + "<w:p/>");

        HeaderText(pages.Pages[0]).ShouldBe("head");
        HeaderText(pages.Pages[1]).ShouldBe(
            "head", "one paragraph is enough, and the table travels with it");
    }

    /// <summary>And with the empty paragraph before it, which rules out "the last block decides".</summary>
    [Fact]
    public void ATableHeaderWithALeadingParagraphIsInherited()
    {
        WordProcessingPages pages = Paginate("<w:p/>" + TableHeader);

        HeaderText(pages.Pages[1]).ShouldBe("head");
    }

    /// <summary>The control: an ordinary header of paragraphs is inherited, as it always was.</summary>
    [Fact]
    public void AParagraphHeaderIsInherited()
    {
        WordProcessingPages pages = Paginate("<w:p><w:r><w:t>head</w:t></w:r></w:p>");

        HeaderText(pages.Pages[0]).ShouldBe("head");
        HeaderText(pages.Pages[1]).ShouldBe("head");
    }

    /// <summary>A second table changes nothing: it is the paragraph that decides, not the count.</summary>
    [Fact]
    public void TwoTablesAndNoParagraphAreNotInheritedEither()
    {
        WordProcessingPages pages = Paginate(TableHeader + TableHeader);

        pages.Pages[1].Header.ShouldBeNull();
    }

    private const string TableHeader =
        "<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/></w:tblPr>"
        + "<w:tblGrid><w:gridCol w:w=\"4000\"/></w:tblGrid>"
        + "<w:tr><w:tc><w:tcPr><w:tcW w:w=\"4000\" w:type=\"dxa\"/></w:tcPr>"
        + "<w:p><w:r><w:t>head</w:t></w:r></w:p></w:tc></w:tr></w:tbl>";

    private const string Geometry =
        "<w:pgSz w:w=\"12240\" w:h=\"15840\"/>"
        + "<w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" "
        + "w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/>";

    /// <summary>
    /// Lays out two sections — the first naming <paramref name="header"/> as its default header, the
    /// second naming no header at all, so that what it draws is whatever it inherits.
    /// </summary>
    private static WordProcessingPages Paginate(string header)
    {
        string body =
            "<w:p><w:pPr><w:sectPr><w:headerReference w:type=\"default\" r:id=\"rId2\"/>"
            + Geometry + "</w:sectPr></w:pPr><w:r><w:t>Alpha</w:t></w:r></w:p>"
            + "<w:p><w:r><w:t>Omega</w:t></w:r></w:p>";

        MemoryStream package = BuildPackage(body, header);
        using DocumentSource source = DocumentSource.FromStream(package, "inherited-header.docx");
        using IDocument document = new WordProcessingReader().Read(source);

        return (WordProcessingPages)((IPaginatedDocument)document).Layout();
    }

    private static string HeaderText(LaidOutPage page)
    {
        page.Header.ShouldNotBeNull();
        return string.Concat(DrawnText(page.Header!.Blocks));
    }

    private static IEnumerable<string> DrawnText(IReadOnlyList<PageBlock> blocks)
    {
        foreach (PageBlock block in blocks)
        {
            switch (block)
            {
                case PageParagraph paragraph when paragraph.Text.Length > 0:
                    yield return paragraph.Text;
                    break;
                case PageTable table:
                    foreach (PageTableRow row in table.Rows)
                    {
                        foreach (PageTableCell cell in row.Cells)
                        {
                            foreach (string text in DrawnText(cell.Blocks)) yield return text;
                        }
                    }

                    break;
            }
        }
    }

    private static MemoryStream BuildPackage(string body, string header)
    {
        const string ContentTypes = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
              <Default Extension="rels"
                       ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
              <Default Extension="xml" ContentType="application/xml"/>
              <Override PartName="/word/document.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
              <Override PartName="/word/settings.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
              <Override PartName="/word/header1.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml"/>
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
              <Relationship Id="rId2" Target="header1.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/header"/>
            </Relationships>
            """;

        // Without a settings.xml a hand-built DOCX does not get LibreOffice's OOXML compatibility
        // defaults, and several synthetics built without one have given clean, consistent, wrong answers.
        const string Settings = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:compat>
                <w:compatSetting w:name="compatibilityMode"
                                 w:uri="http://schemas.microsoft.com/office/word" w:val="15"/>
              </w:compat>
            </w:settings>
            """;

        const string Namespaces =
            "xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" "
            + "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"";

        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document {Namespaces}>
              <w:body>
                {body}
                <w:sectPr>{Geometry}</w:sectPr>
              </w:body>
            </w:document>
            """;

        string headerPart = $"<?xml version=\"1.0\" encoding=\"UTF-8\"?><w:hdr {Namespaces}>{header}</w:hdr>";

        MemoryStream result = new();
        using (ZipArchive archive = new(result, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml", ContentTypes);
            Write(archive, "_rels/.rels", RootRelationships);
            Write(archive, "word/_rels/document.xml.rels", DocumentRelationships);
            Write(archive, "word/settings.xml", Settings);
            Write(archive, "word/header1.xml", headerPart);
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
