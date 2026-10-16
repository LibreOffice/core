using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// How far down the page a fly-held table may reach before it is split, and where its continuation
/// lands when it is.
/// </summary>
/// <remarks>
/// <para>
/// 26.2.4.2 marks every DOCX floating table's frame splittable without exception —
/// <c>DomainMapperTableHandler.cxx</c>:1765, <em>"A text frame created for floating tables is always
/// allowed to split"</em> — so the question is never <em>whether</em> it splits but <em>where the
/// deadline is</em>. <c>GetFlyAnchorBottom</c> (<c>sw/source/core/layout/fly.cxx</c>:114) answers it
/// with two rules and <c>isLegacyBehavior</c> (:104) chooses between them from
/// <b>two</b> conditions that must both hold: the document's <c>TAB_OVER_MARGIN</c> compatibility flag,
/// which is <c>compatibilityMode</c> 14 or less, <b>and</b> a fly positioned against the page frame.
/// </para>
/// <para>
/// Measured on the two corpus documents that disagree, against 26.2.4.2, one variable per rendering.
/// <c>080_Printable_Graph_Paper_Template_Black_Theme</c> is mode 14 with <c>w:vertAnchor="page"</c>, and
/// its 691 pt table sits 17.3 pt below the top of a 697.9 pt body — 10.5 pt past its bottom — and the
/// reference draws it whole on <b>one</b> page. Pushed to <c>w:tblpY="2886"</c> it reaches y = 835 on an
/// 841.9 pt sheet and is <em>still</em> one page. Raise that same file to mode 15 and nothing else:
/// <b>two</b>. Leave it at 14 and move the anchor to <c>text</c> at the same position: <b>two</b>.
/// <c>012_Project_Timeline_Template_Black_and_Brown_Theme</c> is mode 15 with the default <c>text</c>
/// anchor and takes two pages; at mode 14 alone it still takes two; with <em>both</em> mode 14 and
/// <c>vertAnchor="page"</c> it takes one.
/// </para>
/// <para>
/// The tests below are those six renderings, plus the height term — a fly taller than the sheet's print
/// area splits whatever the compatibility mode, which is <c>nFlyHeight &lt;= nPageHeight</c> failing —
/// and the continuation's own position, which is the one thing a page count cannot see.
/// </para>
/// </remarks>
public sealed class FloatedTableDeadlineTests
{
    /// <summary>An A5-height sheet's body, 841.9 pt of paper less two 72 pt margins.</summary>
    private static readonly Length BodyHeight = Length.FromTwips(16838 - 2880);

    /// <summary>The body's top edge on the page, which is the top margin.</summary>
    private static readonly Length BodyTop = Length.FromTwips(1440);

    /// <summary>
    /// A Word 2013 file's fly stops at the body's bottom: the rows past it go to the next page.
    /// </summary>
    /// <remarks>
    /// Six 100 pt rows anchored 100 pt below the body top reach 700 pt against a 697.9 pt body, so five
    /// fit — 100 + 500 = 600 — and the sixth would reach 700. The whole table is 600 pt, which is
    /// shorter than the body, so this is the fly arm and not the flow arm; see
    /// <see cref="ATableTallerThanTheBodyIsFloatedAndSplit"/> for the guard that separates them.
    /// </remarks>
    [Fact]
    public void AWordTwentyThirteenFlyIsSplitAtTheBottomOfTheBody()
    {
        WordProcessingPages pages = Lay(mode: 15, anchor: "page", rows: 6);

        pages.Pages.Count.ShouldBe(2);
        RowsOn(pages, 0).ShouldBe([0, 1, 2, 3, 4]);
        RowsOn(pages, 1).ShouldBe([5]);
    }

    /// <summary>
    /// The continuation starts at the top of the next page's text area, with <c>w:tblpY</c> applied
    /// once and not again.
    /// </summary>
    /// <remarks>
    /// <b>This is the assertion the page count cannot make.</b> A continuation drawn at the wrong offset
    /// — or at <c>w:tblpY</c> a second time — produces exactly the same number of pages, and round 61
    /// recorded what that costs: three passing table-position assertions over a paragraph drawn 7.35 pt
    /// too high, and <c>verify-test.sh</c> reporting NOT DETECTED. The reference's own answer on
    /// <c>012</c> is <c>12.40 489.65 99.95 50.35 re f*</c> on page 2 — a top edge at 72.00 pt from the
    /// sheet's top, which is the top margin exactly.
    /// </remarks>
    [Fact]
    public void TheContinuationStartsAtTheTopOfTheNextPagesTextArea()
    {
        WordProcessingPages pages = Lay(mode: 15, anchor: "page", rows: 6);

        pages.Pages[0].Tables.ShouldHaveSingleItem().Area.Y
            .ShouldBe(BodyTop + Length.FromPoints(100), "the fly's first part keeps w:tblpY");
        pages.Pages[1].Tables.ShouldHaveSingleItem().Area.Y
            .ShouldBe(BodyTop, "and its follow starts at the top of the text area");
    }

    /// <summary>
    /// A Word 2010 file's page-anchored fly hangs into the bottom margin instead of splitting.
    /// </summary>
    /// <remarks>
    /// Six 100 pt rows 100 pt below the body top reach 700 pt, 2.1 pt past the 697.9 pt body — and
    /// under the legacy rule the deadline is the sheet's own bottom edge, so nothing moves.
    /// </remarks>
    [Fact]
    public void AWordTwentyTenPageAnchoredFlyHangsIntoTheBottomMargin()
    {
        WordProcessingPages pages = Lay(mode: 14, anchor: "page", rows: 6);

        pages.Pages.Count.ShouldBe(1);
        RowsOn(pages, 0).ShouldBe([0, 1, 2, 3, 4, 5]);

        // And it really does hang below the body: 100 + 600 against 697.9.
        pages.Pages[0].Tables.ShouldHaveSingleItem().Area.Bottom
            .ShouldBeGreaterThan(BodyTop + BodyHeight);
    }

    /// <summary>
    /// The first control, and the mirror of the assertion above it: at Word 2013 the same page-anchored
    /// fly is not allowed below the body at all.
    /// </summary>
    /// <remarks>
    /// The anchor alone does not grant the overlap. Assert the position rather than the page count,
    /// because a page count is a consequence and a consequence can be right for the wrong reason — the
    /// lesson round 61 paid for with a `NOT DETECTED` from <c>verify-test.sh</c>.
    /// </remarks>
    [Fact]
    public void TheModeAloneDoesNotGrantTheOverlap()
    {
        WordProcessingPages pages = Lay(mode: 15, anchor: "page", rows: 6);

        pages.Pages.Count.ShouldBe(2);
        pages.Pages[0].Tables.ShouldHaveSingleItem().Area.Bottom
            .ShouldBeLessThanOrEqualTo(BodyTop + BodyHeight);
    }

    /// <summary>
    /// The second control: the anchor alone is not enough either. Word 2010 with a <c>text</c> anchor
    /// splits, and this is the arm that separates the rule from "an old file never splits".
    /// </summary>
    [Fact]
    public void TheAnchorAloneDoesNotGrantTheOverlap()
    {
        Lay(mode: 14, anchor: "text", rows: 6).Pages.Count.ShouldBe(2);
    }

    /// <summary>
    /// A table taller than a whole body is floated and split like any other fly, at <c>w:tblpY</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Eight 100 pt rows are 800 pt against a 697.9 pt body. The page count is the same either way and
    /// every row is drawn once either way, so neither of those can tell the two behaviours apart — the
    /// first part's own y is what does: floated it starts 100 pt down the body, where <c>w:tblpY</c>
    /// puts it, and in the flow it starts at the body's top.
    /// </para>
    /// <para>
    /// <b>This test used to assert the opposite, and said so in its own remark: a known divergence
    /// pinned rather than a correct behaviour.</b> <c>PlaceFloatedTable</c> carried a
    /// <c>height &gt; area.Height</c> guard that left such a table in the flow, which dropped
    /// <c>w:tblpY</c> along with the fly treatment. Round 62 kept it deliberately — the two corpus
    /// documents then in its class, <c>ESPN-R - MCF - RA - Ed1.docx</c> and
    /// <c>part-147_approval list_20230119.docx</c>, both passed the gate, so trading it blind would have
    /// risked them for nothing. Both are unchanged by its removal, at 58 and 2 pages.
    /// </para>
    /// <para>
    /// What settled it is <c>Case-Study-Heathrow-Airport.docx</c>, a third document in the class that
    /// does <em>not</em> pass: its whole first page is a three-page fly, and the guard cost it the 33 pt
    /// its <c>w:tblpY="662"</c> states. Measured on ten authored fixtures against both installed
    /// references, which agree to a tenth of a point: a 90-row table at that offset puts its first row at
    /// y = 105.6 and its thirty-third at 72.5 on <em>page two</em>, the body's own top, with the same x.
    /// See <c>probes/words-page-anchored-table/</c>. The corpus half was already recorded here too —
    /// doubling every row of <c>080_Printable_Graph_Paper_Template_Black_Theme</c>, to 1382 pt against a
    /// 697.9 pt print area, brings 26.2.4.2's own split back.
    /// </para>
    /// </remarks>
    [Fact]
    public void ATableTallerThanTheBodyIsFloatedAndSplit()
    {
        WordProcessingPages pages = Lay(mode: 14, anchor: "page", rows: 8);

        pages.Pages[0].Tables.ShouldHaveSingleItem().Area.Y
            .ShouldBe(BodyTop + Length.FromPoints(100), "floated, at w:tblpY down the body");

        List<int> all = [.. RowsOn(pages, 0), .. RowsOn(pages, 1)];
        all.ShouldBe([0, 1, 2, 3, 4, 5, 6, 7], "and every row is still drawn exactly once");
    }

    /// <summary>Which rows of the fly landed on one page, in order.</summary>
    private static List<int> RowsOn(WordProcessingPages pages, int page)
        => [.. pages.Pages[page].Tables
            .SelectMany(part => Enumerable.Range(part.FirstRow, part.RowEnd - part.FirstRow))
            .Order()];

    private static WordProcessingPages Lay(int mode, string anchor, int rows)
    {
        using IDocument document = Open(mode, anchor, rows);
        return (WordProcessingPages)((IPaginatedDocument)document).Layout();
    }

    private static IDocument Open(int mode, string anchor, int rows)
    {
        MemoryStream package = BuildPackage(mode, anchor, rows);
        using DocumentSource source = DocumentSource.FromStream(package, "floated-table-deadline.docx");
        return new WordProcessingReader().Read(source);
    }

    private static MemoryStream BuildPackage(int mode, string anchor, int rows)
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

        // The part this whole fixture turns on, so it is stated rather than defaulted. A package with no
        // settings part reads as `compatibilityMode` 0, which is a third answer and not one of the two
        // being separated here.
        string settings = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:compat>
                <w:compatSetting w:name="compatibilityMode"
                                 w:uri="http://schemas.microsoft.com/office/word" w:val="{mode}"/>
              </w:compat>
            </w:settings>
            """;

        // Each row is 2000 twips — 100 pt — stated exactly, so where the deadline falls is arithmetic
        // and not a font metric.
        string body = string.Concat(Enumerable.Range(0, rows).Select(row => $"""
                  <w:tr>
                    <w:trPr><w:trHeight w:val="2000" w:hRule="exact"/></w:trPr>
                    <w:tc><w:tcPr><w:tcW w:w="5000" w:type="dxa"/></w:tcPr>
                      <w:p><w:r><w:t>Row {row}</w:t></w:r></w:p>
                    </w:tc>
                  </w:tr>
            """));

        // 3440 twips from the page's top is 100 pt below a 72 pt margin; 2000 from the text's is the
        // same place, since the fly is the first thing in the body.
        string position = anchor == "page"
            ? """<w:tblpPr w:vertAnchor="page" w:horzAnchor="margin" w:tblpY="3440"/>"""
            : """<w:tblpPr w:vertAnchor="text" w:horzAnchor="margin" w:tblpY="2000"/>""";

        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:body>
                <w:tbl>
                  <w:tblPr>
                    {position}
                    <w:tblW w:w="5000" w:type="dxa"/>
                  </w:tblPr>
                  <w:tblGrid><w:gridCol w:w="5000"/></w:tblGrid>
            {body}
                </w:tbl>
                <w:p/>
                <w:sectPr>
                  <w:pgSz w:w="11906" w:h="16838"/>
                  <w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"
                           w:header="708" w:footer="708" w:gutter="0"/>
                </w:sectPr>
              </w:body>
            </w:document>
            """;

        MemoryStream result = new();
        using (ZipArchive archive = new(result, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml", ContentTypes);
            Write(archive, "_rels/.rels", RootRelationships);
            Write(archive, "word/_rels/document.xml.rels", DocumentRelationships);
            Write(archive, "word/settings.xml", settings);
            Write(archive, "word/document.xml", document);
        }

        result.Position = 0;
        return result;
    }

    private static void Write(ZipArchive archive, string path, string content)
    {
        using StreamWriter writer = new(archive.CreateEntry(path).Open(), Encoding.UTF8);
        writer.Write(content);
    }
}
