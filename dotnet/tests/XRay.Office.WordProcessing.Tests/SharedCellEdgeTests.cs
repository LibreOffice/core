using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A horizontal edge two cells share is one border, and both of them get it.
/// </summary>
/// <remarks>
/// <para>
/// A cell stating <c>&lt;w:top w:val="nil"/&gt;</c> under a cell stating a bottom border is still
/// separated from it by that border. On a page where both rows are drawn the point is invisible —
/// the row above's bottom already covers the line — so the only place it shows is the top of a
/// table's <em>continuation</em> page, where the row above is on the previous sheet and the line
/// comes out drawn only for the cells that state a top of their own. That is O82, and on
/// <c>review-welsh-government-communications-mister-peter-mandelson.docx</c> it was 1 511 pt of
/// hole over eight pages.
/// </para>
/// <para>
/// Measured at 26.2.4.2 rather than read off ECMA-376, on
/// <c>dotnet/probes/wordstable-r131/borderprobe.docx</c> — eight two-row arms rendered three times
/// into separate user profiles, byte-identical geometry all three times. <c>bottom=1pt / top=nil</c>
/// and <c>bottom=nil / top=1pt</c> both draw the line; <c>nil/nil</c> draws none; a 3 pt facing a
/// 0.5 pt draws 3 pt whichever side states which; and a row where only one cell of two nils the edge
/// is drawn for the other cell alone. It is §17.4.62's conflict resolution as far as width goes; the
/// style half of that precedence is not modelled, and a tie keeps the cell's own.
/// </para>
/// <para>
/// The same probe says the row <em>height</em> should follow the resolved border too — its
/// <c>nil/nil</c> arm is a whole point shorter than the other three — and that half is deliberately
/// not implemented. Applied to the corpus it over-charges: on this document's first table it takes
/// twenty rows from 1.45 pt short of 26.2.4.2 to 6.05 pt long, because a <c>w:trHeight</c> row's
/// border charge is not the same sum. <c>probes/wordstable-r131/results.md</c> §4 and seat O84.
/// </para>
/// </remarks>
public sealed class SharedCellEdgeTests
{
    /// <summary>The wider of the two facing edges wins, and both cells end up with it.</summary>
    /// <remarks>
    /// Four arms of the probe in one theory: the control where both sides agree, the two asymmetric
    /// cases in both directions, and the pair where the two widths differ. The expectation is in
    /// eighths of a point, as the file states it.
    /// </remarks>
    [Theory]
    [InlineData(8, 8, 8, 8)]        // BB — both state a point, nothing to resolve
    [InlineData(8, 0, 8, 8)]        // BN — the lower row nils it and takes the upper row's
    [InlineData(0, 8, 8, 8)]        // NT — and the other way round
    [InlineData(0, 0, 0, 0)]        // NN — neither states it and neither gets one
    [InlineData(24, 4, 24, 24)]     // WB — 3 pt above a half point: the wider wins
    [InlineData(4, 24, 24, 24)]     // WT — and it wins from below too
    public void TheWiderOfTwoFacingEdgesIsWhatBothCellsGet(
        int upperBottom, int lowerTop, int expectedUpperBottom, int expectedLowerTop)
    {
        PageTable table = TwoRows(upperBottom, lowerTop);
        IReadOnlyList<PageTableRow> shared = table.RowsWithSharedEdges;

        shared[0].Cells[0].Borders.Bottom.Width
            .ShouldBe(Length.FromTwips(expectedUpperBottom * 20 / 8));
        shared[1].Cells[0].Borders.Top.Width
            .ShouldBe(Length.FromTwips(expectedLowerTop * 20 / 8));
    }

    /// <summary>
    /// What the file stated is still there: the resolution is a second view of the rows, not a
    /// rewrite of them.
    /// </summary>
    /// <remarks>
    /// The row heights are computed from <see cref="PageTable.Rows"/> and only the drawing reads the
    /// resolved view, so a test that could not tell the two apart would pass with the layout on the
    /// wrong one.
    /// </remarks>
    [Fact]
    public void TheStatedRowsAreLeftAsTheFileWroteThem()
    {
        PageTable table = TwoRows(upperBottom: 8, lowerTop: 0);

        table.Rows[1].Cells[0].Borders.Top.IsNone.ShouldBeTrue();
        table.RowsWithSharedEdges[1].Cells[0].Borders.Top.IsNone.ShouldBeFalse();
    }

    /// <summary>
    /// A cell whose facing neighbour nils the edge as well keeps nothing, even where the cell beside
    /// it in the same row has a border.
    /// </summary>
    /// <remarks>
    /// The <c>MIXN</c> arm, and the one that says the resolution is per column rather than per row:
    /// 26.2.4.2 draws that grid line for the right-hand cell alone, over half the table's width. A
    /// rule that took the widest border anywhere on the line would draw it whole and be wrong.
    /// </remarks>
    [Fact]
    public void ACellIsResolvedAgainstTheCellAboveItAndNotAgainstItsRow()
    {
        PageTable table = TwoColumns();
        IReadOnlyList<PageTableRow> shared = table.RowsWithSharedEdges;

        shared[1].Cells[0].Borders.Top.IsNone.ShouldBeTrue();
        shared[1].Cells[1].Borders.Top.Width.ShouldBe(Length.FromTwips(20));
    }

    private static PageTable TwoRows(int upperBottom, int lowerTop)
    {
        string rows = $"""
              <w:tr>
                <w:tc>
                  <w:tcPr><w:tcW w:w="4800" w:type="dxa"/><w:tcBorders>
                    <w:top w:val="single" w:sz="8" w:space="0" w:color="000000"/>
                    {Edge("bottom", upperBottom)}
                  </w:tcBorders></w:tcPr>
                  <w:p><w:r><w:t>upper</w:t></w:r></w:p>
                </w:tc>
              </w:tr>
              <w:tr>
                <w:tc>
                  <w:tcPr><w:tcW w:w="4800" w:type="dxa"/><w:tcBorders>
                    {Edge("top", lowerTop)}
                    <w:bottom w:val="single" w:sz="8" w:space="0" w:color="000000"/>
                  </w:tcBorders></w:tcPr>
                  <w:p><w:r><w:t>lower</w:t></w:r></w:p>
                </w:tc>
              </w:tr>
            """;

        return Layout(rows, "<w:gridCol w:w=\"4800\"/>");
    }

    private static PageTable TwoColumns()
    {
        const string Rows = """
              <w:tr>
                <w:tc>
                  <w:tcPr><w:tcW w:w="2400" w:type="dxa"/><w:tcBorders>
                    <w:bottom w:val="nil"/>
                  </w:tcBorders></w:tcPr>
                  <w:p><w:r><w:t>a</w:t></w:r></w:p>
                </w:tc>
                <w:tc>
                  <w:tcPr><w:tcW w:w="2400" w:type="dxa"/><w:tcBorders>
                    <w:bottom w:val="single" w:sz="8" w:space="0" w:color="000000"/>
                  </w:tcBorders></w:tcPr>
                  <w:p><w:r><w:t>b</w:t></w:r></w:p>
                </w:tc>
              </w:tr>
              <w:tr>
                <w:tc>
                  <w:tcPr><w:tcW w:w="2400" w:type="dxa"/><w:tcBorders>
                    <w:top w:val="nil"/>
                  </w:tcBorders></w:tcPr>
                  <w:p><w:r><w:t>c</w:t></w:r></w:p>
                </w:tc>
                <w:tc>
                  <w:tcPr><w:tcW w:w="2400" w:type="dxa"/><w:tcBorders>
                    <w:top w:val="nil"/>
                  </w:tcBorders></w:tcPr>
                  <w:p><w:r><w:t>d</w:t></w:r></w:p>
                </w:tc>
              </w:tr>
            """;

        return Layout(Rows, "<w:gridCol w:w=\"2400\"/><w:gridCol w:w=\"2400\"/>");
    }

    private static string Edge(string side, int eighths)
        => eighths == 0
            ? $"<w:{side} w:val=\"nil\"/>"
            : $"<w:{side} w:val=\"single\" w:sz=\"{eighths}\" w:space=\"0\" w:color=\"000000\"/>";

    private static PageTable Layout(string rows, string grid)
    {
        using IDocument document = Open(rows, grid);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        return pages.Blocks.OfType<PageTable>().Single();
    }

    private static IDocument Open(string rows, string grid)
    {
        MemoryStream package = BuildPackage(rows, grid);
        using DocumentSource source = DocumentSource.FromStream(package, "shared-cell-edge.docx");
        return new WordProcessingReader().Read(source);
    }

    private static MemoryStream BuildPackage(string rows, string grid)
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

        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:body>
                <w:tbl>
                  <w:tblPr>
                    <w:tblW w:w="4800" w:type="dxa"/>
                    <w:tblLayout w:type="fixed"/>
                  </w:tblPr>
                  <w:tblGrid>{grid}</w:tblGrid>
            {rows}
                </w:tbl>
                <w:p><w:r><w:t>after</w:t></w:r></w:p>
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

        static void Write(ZipArchive archive, string name, string content)
        {
            using Stream entry = archive.CreateEntry(name).Open();
            entry.Write(Encoding.UTF8.GetBytes(content));
        }
    }
}
