using System.IO.Compression;
using System.Text;
using Paperless.Core.Documents;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// The blank page an odd- or even-page section break leaves behind: it takes a page number and it is
/// never drawn.
/// </summary>
/// <remarks>
/// <para>
/// Writer states the break as a page style whose <c>UseOn</c> names one side only —
/// <c>SectionPropertyMap::CreateEvenOddPageStyleCopy</c> sets <c>PageStyleLayout_RIGHT</c>
/// (<c>sw/source/writerfilter/dmapper/PropertyMap.cxx</c>:1568), and the DOC reader does the same for
/// <c>sprmSBkc</c> 3 and 4 (<c>sw/source/filter/ww8/ww8par.cxx</c>:4470-4479). <c>SwFrame::InsertPage</c>
/// then inserts a page of <c>rDoc.GetEmptyPageFormat()</c> whenever the side the style wants and the side
/// the alternation would give disagree (<c>sw/source/core/layout/pagechg.cxx</c>:1613-1616).
/// </para>
/// <para>
/// PDF export drops that page. <c>SwPrintUIOptions::IsPrintEmptyPages</c> reads <c>IsSkipEmptyPages</c>
/// for an export, whose default is true (<c>sw/source/core/view/printdata.cxx</c>:391-399), so an
/// automatically inserted blank is laid out and not rendered. Measured against LibreOffice 24.2 on the
/// two documents these tests reproduce: both come back as **two**-page PDFs, and the second page's own
/// <c>PAGE</c> field reads 3 and 4 respectively — the blank is there, in the numbering, and not in the
/// file.
/// </para>
/// <para>
/// The side is decided physically rather than from the printed number. The style states one of
/// <c>GetRightFormat</c>/<c>GetLeftFormat</c> and leaves the other null, so <c>InsertPage</c>'s flip
/// overrides the <c>SetNumOffset</c> reading above it, and <c>OnRightPage()</c> is
/// <c>GetPhyPageNum() % 2</c> (<c>sw/source/core/inc/frame.hxx</c>:757). That is what
/// <see cref="TheSideIsThePhysicalPagesNotThePrintedNumbers"/> pins: its first page is *numbered* two, so
/// the next number is odd and the next physical page is even, and LibreOffice still leaves a blank.
/// </para>
/// </remarks>
public sealed class OddPageBreakFillerTests
{
    [Fact]
    public void TheBlankAnOddPageBreakLeavesIsNotDrawn()
    {
        IReadOnlyList<LaidOutPage> pages = Paginate(firstPageNumber: null);

        pages.Count.ShouldBe(2, "LibreOffice's PDF of this document has two pages");
        pages[0].Number.ShouldBe(1);
    }

    [Fact]
    public void TheBlankStillTakesAPageNumber()
    {
        IReadOnlyList<LaidOutPage> pages = Paginate(firstPageNumber: null);

        pages[1].Number.ShouldBe(3, "the undrawn blank is page two");
    }

    [Fact]
    public void TheSideIsThePhysicalPagesNotThePrintedNumbers()
    {
        // Page one is numbered two, so the *next number* is three and already odd while the next
        // *physical* page is the second and even. LibreOffice inserts the blank and the section lands on
        // page four; reading the printed number instead would leave it on three.
        IReadOnlyList<LaidOutPage> pages = Paginate(firstPageNumber: 2);

        pages.Count.ShouldBe(2);
        pages[0].Number.ShouldBe(2);
        pages[1].Number.ShouldBe(4);
    }

    /// <summary>
    /// A restart of the page numbering picks a side of the sheet too, and leaves the same undrawn blank.
    /// </summary>
    /// <remarks>
    /// Three sections, the last broken to an odd page, each printing its own number. Measured against
    /// LibreOffice 24.2: with no restart the three read 1, 2, 3; restarting the middle section at 19 gives
    /// 1, 19, <em>21</em>; restarting it at 20 gives 1, 20, 21. All three export three pages. The 21 in
    /// the middle case is two skipped blanks — one putting an odd restart on an odd sheet, one for the
    /// odd-page break that then lands on an even one.
    /// </remarks>
    [Theory]
    [InlineData(null, 2, 3)]
    [InlineData(19, 19, 21)]
    [InlineData(20, 20, 21)]
    public void ARestartOfTheNumberingPicksASideToo(int? restartAt, int second, int third)
    {
        IReadOnlyList<LaidOutPage> pages = PaginateThree(restartAt);

        pages.Count.ShouldBe(3);
        pages[0].Number.ShouldBe(1);
        pages[1].Number.ShouldBe(second);
        pages[2].Number.ShouldBe(third);
    }

    private static IReadOnlyList<LaidOutPage> PaginateThree(int? restartAt)
    {
        MemoryStream package = BuildThreeSectionPackage(restartAt);
        using DocumentSource source = DocumentSource.FromStream(package, "restart.docx");
        using IDocument document = new WordProcessingReader().Read(source);

        return ((WordProcessingPages)((IPaginatedDocument)document).Layout()).Pages;
    }

    private static IReadOnlyList<LaidOutPage> Paginate(int? firstPageNumber)
    {
        MemoryStream package = BuildPackage(firstPageNumber);
        using DocumentSource source = DocumentSource.FromStream(package, "oddpage.docx");
        using IDocument document = new WordProcessingReader().Read(source);

        return ((WordProcessingPages)((IPaginatedDocument)document).Layout()).Pages;
    }

    /// <summary>
    /// Two one-line sections, the second broken to the next odd page. Deliberately short: the point is
    /// the parity of the page the second section lands on, and a document whose first section already
    /// runs to two pages would need no blank and prove nothing.
    /// </summary>
    private static MemoryStream BuildPackage(int? firstPageNumber)
    {
        string startAt = firstPageNumber is { } n ? $"""<w:pgNumType w:start="{n}"/>""" : string.Empty;

        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:body>
                <w:p>
                  <w:pPr><w:sectPr>{Sheet}{startAt}</w:sectPr></w:pPr>
                  <w:r><w:t>Section one</w:t></w:r>
                </w:p>
                <w:p><w:r><w:t>Section two</w:t></w:r></w:p>
                <w:sectPr><w:type w:val="oddPage"/>{Sheet}</w:sectPr>
              </w:body>
            </w:document>
            """;

        return Package(document);
    }

    /// <summary>Wraps one <c>word/document.xml</c> body in the smallest package the reader accepts.</summary>
    private static MemoryStream Package(string document)
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
            </Relationships>
            """;

        // A hand-built DOCX carrying no settings part misses LibreOffice's OOXML compatibility defaults,
        // which has given clean and wrong answers to several synthetics before.
        const string Settings = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"/>
            """;

        MemoryStream result = new();
        using (ZipArchive archive = new(result, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml", ContentTypes);
            Write(archive, "_rels/.rels", RootRelationships);
            Write(archive, "word/_rels/document.xml.rels", DocumentRelationships);
            Write(archive, "word/settings.xml", Settings);
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

    /// <summary>
    /// Three one-line sections: a plain first, a second broken to the next page and optionally restarting
    /// its numbering, and a third broken to the next odd page.
    /// </summary>
    private static MemoryStream BuildThreeSectionPackage(int? restartAt)
    {
        string restart = restartAt is { } n ? $"""<w:pgNumType w:start="{n}"/>""" : string.Empty;

        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:body>
                <w:p>
                  <w:pPr><w:sectPr>{Sheet}</w:sectPr></w:pPr>
                  <w:r><w:t>Section one</w:t></w:r>
                </w:p>
                <w:p>
                  <w:pPr><w:sectPr>{Sheet}{restart}</w:sectPr></w:pPr>
                  <w:r><w:t>Section two</w:t></w:r>
                </w:p>
                <w:p><w:r><w:t>Section three</w:t></w:r></w:p>
                <w:sectPr><w:type w:val="oddPage"/>{Sheet}</w:sectPr>
              </w:body>
            </w:document>
            """;

        return Package(document);
    }

    /// <summary>A4 with even margins and no header or footer band, shared by every section here.</summary>
    private const string Sheet = """
        <w:pgSz w:w="11906" w:h="16838"/>
        <w:pgMar w:top="1134" w:right="1134" w:bottom="1134" w:left="1134"
                 w:header="0" w:footer="0" w:gutter="0"/>
        """;
}
