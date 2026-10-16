using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A positioned table that does not fit the room left on its page is placed <em>once</em>: only the
/// table's first part may be floated, and its continuation is the flow's.
/// </summary>
/// <remarks>
/// <para>
/// <see cref="PositionedBodyTableTests"/> covers the placement itself. This is the interaction between
/// it and a page break, and it is the defect round 51 found underneath what round 50 had filed as a
/// missing text wrap.
/// </para>
/// <para>
/// <c>Paginator.Fill</c>'s table arm is re-entered for every page a table touches, because a split table
/// carries on with <c>paragraphIndex</c> still on it and <c>lineIndex</c> at the row that did not fit.
/// <c>PlaceFloatedTable</c> reads neither, so on the continuation page it floated the table again —
/// <b>from row 0, entire</b> — on top of the part already drawn.
/// </para>
/// <para>
/// Measured on <c>AFS-050-004-F2_0i.docx</c> (<c>words/done-014</c>), whose first positioned table has
/// 37 rows. Traced: block 23 placed in the flow on page 2 as <c>from=0 to=36 placed=True</c>, then
/// floated whole on page 3. Its pages 2 and 3 differ by <b>five tokens</b> — a heading and the two page
/// numbers. A multiset diff against 26.2.4.2's own PDF gave <b>318 tokens only in ours, 0 only in the
/// reference, and not one of the 318 a string the reference never draws</b>: every one was a repeat.
/// Deleting the document's four <c>w:tblpPr</c> elements and changing nothing else rendered the
/// reference's raw total exactly, 2384 to 2384. With this guard the unmodified document renders 8 pages
/// and 2384 words against the reference's 8 and 2384, with an empty token diff in both directions.
/// </para>
/// </remarks>
public sealed class PositionedTableSplitTests
{
    /// <summary>
    /// Every row of a split positioned table is drawn exactly once, across all the pages it touches.
    /// </summary>
    /// <remarks>
    /// The geometry is chosen so the table must split <em>and</em> so the continuation page would float
    /// it if asked. A5-height page with 72 pt margins gives 697.9 pt of body; a 150 pt paragraph is
    /// followed by six 100 pt rows, so five of them fit and the sixth does not. The fly is anchored
    /// 100 pt below the body top, which is <em>above</em> where the flow has reached on page one — so
    /// <c>RunsIntoTheFly</c> keeps it in the flow there — and above where the flow starts on page two,
    /// where the only block after it is one short line that ends clear of the fly.
    /// </remarks>
    [Fact]
    public void ASplitPositionedTableDrawsEachRowOnce()
    {
        WordProcessingPages pages = Lay();

        List<PlacedTable> parts = [.. pages.Pages.SelectMany(page => page.Tables)];

        parts.Count.ShouldBeGreaterThan(1, "the table has to split for this to be testing anything");

        List<int> rows = [.. parts.SelectMany(part => Enumerable.Range(part.FirstRow, part.RowEnd - part.FirstRow))];

        rows.Count.ShouldBe(6, "six rows, drawn once each");
        rows.Order().ShouldBe([0, 1, 2, 3, 4, 5]);
    }

    /// <summary>
    /// The control: the table's <em>first</em> part is still floated when nothing stops it, so the guard
    /// has not simply switched positioned tables off.
    /// </summary>
    [Fact]
    public void AnUnsplitPositionedTableIsStillFloated()
    {
        WordProcessingPages pages = Lay(leading: 0, rows: 3);

        // Floated: the paragraph after it stays at the top of the page rather than below the table.
        pages.Pages[0].Tables.ShouldHaveSingleItem().Area.Y.ShouldBe(Length.FromPoints(72 + 100));
        pages.Pages[0].Lines.First(line => line.StartsParagraph).Top.ShouldBe(Length.Zero);
    }

    private static WordProcessingPages Lay(int leading = 3000, int rows = 6)
    {
        using IDocument document = Open(leading, rows);
        return (WordProcessingPages)((IPaginatedDocument)document).Layout();
    }

    private static IDocument Open(int leading, int rows)
    {
        MemoryStream package = BuildPackage(leading, rows);
        using DocumentSource source = DocumentSource.FromStream(package, "split-positioned-table.docx");
        return new WordProcessingReader().Read(source);
    }

    private static MemoryStream BuildPackage(int leading, int rows)
    {
        const string ContentTypes = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
              <Default Extension="rels"
                       ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
              <Default Extension="xml" ContentType="application/xml"/>
              <Override PartName="/word/document.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
            </Types>
            """;

        const string RootRelationships = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
              <Relationship Id="rId1" Target="word/document.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"/>
            </Relationships>
            """;

        // Each row is 2000 twips — 100 pt — stated exactly, so the split point is arithmetic rather than
        // a font metric.
        string body = string.Concat(Enumerable.Range(0, rows).Select(row => $"""
                  <w:tr>
                    <w:trPr><w:trHeight w:val="2000" w:hRule="exact"/></w:trPr>
                    <w:tc><w:tcPr><w:tcW w:w="5000" w:type="dxa"/></w:tcPr>
                      <w:p><w:r><w:t>Row {row}</w:t></w:r></w:p>
                    </w:tc>
                  </w:tr>
            """));

        // 150 pt of leading paragraph when asked for, so the table cannot fit whole beneath it.
        string above = leading > 0
            ? $"""<w:p><w:pPr><w:spacing w:line="{leading}" w:lineRule="exact"/></w:pPr><w:r><w:t>Above</w:t></w:r></w:p>"""
            : "";

        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:body>
                {above}
                <w:tbl>
                  <w:tblPr>
                    <w:tblpPr w:vertAnchor="page" w:horzAnchor="margin" w:tblpY="3440"/>
                    <w:tblW w:w="5000" w:type="dxa"/>
                  </w:tblPr>
                  <w:tblGrid><w:gridCol w:w="5000"/></w:tblGrid>
            {body}
                </w:tbl>
                <w:p><w:r><w:t>After</w:t></w:r></w:p>
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
