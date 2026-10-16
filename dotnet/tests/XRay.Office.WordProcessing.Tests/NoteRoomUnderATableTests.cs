using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// The room left for a page's notes is measured from the body's top, not from the page's.
/// </summary>
/// <remarks>
/// <para>
/// A <see cref="PlacedLine"/>'s <c>Top</c> is relative to its column, because the column rectangle is
/// applied when it is drawn; a <see cref="PlacedTable"/> is offset into page coordinates as it is placed.
/// Taking the maximum of the two raw and subtracting it from the body's <em>height</em> compares a depth
/// with a position, and understates the room by the top margin — enough to push a note off any page whose
/// table reaches far enough down that its page-coordinate bottom exceeds the body's height.
/// </para>
/// <para>
/// <strong>Measured on <c>TE.CAO.00125 Foreign Part 145 approvals - OJT Logbook.docx</c>, 16 pages
/// against 15 and now 15.</strong> Its body starts 129.0 pt down a landscape page; page 3 ends its tables
/// at 492.4 with 61.9 pt free and cites a footnote needing 52.17, and the subtraction returned
/// <em>−67.05</em>. The note spilled to page 4, found the same arithmetic there, and landed on page 5 —
/// where LibreOffice draws it at the foot of page 3. The note's own measurement was never wrong: it draws
/// in five lines both ways.
/// </para>
/// </remarks>
public sealed class NoteRoomUnderATableTests
{
    /// <summary>
    /// A note cited from a table cell stays on the page the table is drawn on, when there is room.
    /// </summary>
    /// <remarks>
    /// The rows are chosen so the table's page-coordinate bottom exceeds the body's height while its
    /// column-relative bottom still leaves room for the note — which is the only regime where the two
    /// coordinate systems can be told apart, and the regime the corpus document sits in.
    /// </remarks>
    [Theory]
    [InlineData(37)]
    [InlineData(38)]
    [InlineData(39)]
    public void ANoteCitedFromATableCellStaysOnTheTablesPage(int rows)
    {
        WordProcessingPages pages = Paginate(Document(rows));

        int tablePage = PageOf(pages, page => page.Tables.Count > 0);
        int notePage = PageOf(pages, page => page.Notes is { Lines.Count: > 0 });

        tablePage.ShouldBeGreaterThanOrEqualTo(0);
        notePage.ShouldBe(tablePage, "the note belongs at the foot of the page that cites it");
    }

    /// <summary>
    /// The regime check: the table really does reach past the body's height in page coordinates.
    /// </summary>
    /// <remarks>
    /// Without this the theory above would pass on a document where the two coordinate systems never
    /// diverge far enough to matter, and would go on passing if the subtraction were wrong again.
    /// </remarks>
    [Fact]
    public void TheTableReachesPastTheBodyHeightInPageCoordinates()
    {
        LaidOutPage page = Paginate(Document(38)).Pages[0];

        page.Tables.Count.ShouldBeGreaterThan(0);
        page.Tables[^1].Area.Bottom.ShouldBeGreaterThan(
            page.BodyArea.Height,
            "otherwise the page-coordinate bottom and the column-relative one cannot be told apart");
    }

    private static int PageOf(WordProcessingPages pages, Func<LaidOutPage, bool> test)
    {
        for (int page = 0; page < pages.Pages.Count; page++)
        {
            if (test(pages.Pages[page])) return page;
        }

        return -1;
    }

    private static WordProcessingPages Paginate(string body)
    {
        MemoryStream package = BuildPackage(body);
        using DocumentSource source = DocumentSource.FromStream(package, "note-room.docx");
        using IDocument document = new WordProcessingReader().Read(source);
        return (WordProcessingPages)((IPaginatedDocument)document).Layout();
    }

    /// <summary>A table filling most of the page, its first cell citing the one footnote.</summary>
    private static string Document(int rows)
    {
        string cells = string.Concat(Enumerable.Range(0, rows).Select(i => $"""
              <w:tr><w:tc><w:tcPr><w:tcW w:w="5000" w:type="dxa"/></w:tcPr>
                <w:p><w:r><w:rPr><w:sz w:val="24"/></w:rPr><w:t>row {i}</w:t></w:r>
                  {(i == 0 ? "<w:r><w:rPr><w:rStyle w:val=\"FootnoteReference\"/></w:rPr>"
                             + "<w:footnoteReference w:id=\"2\"/></w:r>" : string.Empty)}
                </w:p>
              </w:tc></w:tr>
            """));

        return $"""
              <w:tbl>
                <w:tblPr><w:tblW w:w="5000" w:type="dxa"/><w:tblLayout w:type="fixed"/></w:tblPr>
                <w:tblGrid><w:gridCol w:w="5000"/></w:tblGrid>
                {cells}
              </w:tbl>
            """;
    }

    private static MemoryStream BuildPackage(string body)
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
              <Override PartName="/word/footnotes.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.footnotes+xml"/>
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
              <Relationship Id="rId2" Target="footnotes.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/footnotes"/>
            </Relationships>
            """;

        const string Settings = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:compat>
                <w:compatSetting w:name="compatibilityMode"
                                 w:uri="http://schemas.microsoft.com/office/word" w:val="15"/>
              </w:compat>
            </w:settings>
            """;

        const string Footnotes = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:footnotes xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:footnote w:type="separator" w:id="0"><w:p><w:r><w:separator/></w:r></w:p></w:footnote>
              <w:footnote w:type="continuationSeparator" w:id="1">
                <w:p><w:r><w:continuationSeparator/></w:r></w:p>
              </w:footnote>
              <w:footnote w:id="2">
                <w:p><w:r><w:rPr><w:sz w:val="18"/></w:rPr>
                  <w:t>The note cited from the first cell of the table above.</w:t></w:r></w:p>
              </w:footnote>
            </w:footnotes>
            """;

        // A generous top margin is what makes the two coordinate systems differ by a visible amount.
        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:body>
                {body}
                <w:sectPr>
                  <w:pgSz w:w="12240" w:h="15840"/>
                  <w:pgMar w:top="2160" w:right="1440" w:bottom="1440" w:left="1440"/>
                </w:sectPr>
              </w:body>
            </w:document>
            """;

        MemoryStream package = new();

        using (ZipArchive archive = new(package, ZipArchiveMode.Create, leaveOpen: true))
        {
            Add(archive, "[Content_Types].xml", ContentTypes);
            Add(archive, "_rels/.rels", RootRelationships);
            Add(archive, "word/_rels/document.xml.rels", DocumentRelationships);
            Add(archive, "word/settings.xml", Settings);
            Add(archive, "word/footnotes.xml", Footnotes);
            Add(archive, "word/document.xml", document);
        }

        package.Position = 0;
        return package;
    }

    private static void Add(ZipArchive archive, string name, string content)
    {
        using Stream entry = archive.CreateEntry(name).Open();
        byte[] bytes = Encoding.UTF8.GetBytes(content);
        entry.Write(bytes, 0, bytes.Length);
    }
}
