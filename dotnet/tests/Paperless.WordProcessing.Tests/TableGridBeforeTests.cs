using System.IO.Compression;
using System.Text;
using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.TestKit;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// A row that starts part-way along the grid — <c>w:gridBefore</c>.
/// </summary>
/// <remarks>
/// <para>
/// DOCX writes no placeholder cell for the columns a row skips, exactly as it writes none for the columns
/// a <c>w:gridSpan</c> swallows. A reader that starts every row at column zero therefore gives the row's
/// first cell the first column's width, and every cell after it the wrong one too — the whole row is
/// shifted left and squeezed.
/// </para>
/// <para>
/// LibreOffice reaches the right layout by materialising the skipped columns as borderless empty cells
/// (<c>TableManager::endRow</c>, <c>sw/source/writerfilter/dmapper/TableManager.cxx</c>:667–702); an
/// absent cell draws the same nothing, so shifting the column index is the same answer.
/// </para>
/// <para>
/// Found on <c>words/batch-011/docx/FG-AI4NDM-TR-skeleton_template.docx</c>, whose title block is a table
/// whose title rows skip a narrow first column. Measured against the first column instead of its own, the
/// 36 pt title wrapped to one word a line and the block grew onto a page of its own: 6 pages against the
/// reference's 5.
/// </para>
/// </remarks>
public sealed class TableGridBeforeTests
{
    /// <summary>The row's first cell sits in the column <c>w:gridBefore</c> names, not in the first.</summary>
    [Fact]
    public void ARowSkipsTheColumnsGridBeforeNames()
    {
        PageTable table = Table();

        // Row one fills the grid; row two skips the first two columns.
        table.Rows[0].Cells.Select(cell => cell.Column).ShouldBe([0, 1, 2]);
        table.Rows[1].Cells.Select(cell => cell.Column).ShouldBe([2]);
    }

    /// <summary>And so it is measured against that column's width rather than the first's.</summary>
    /// <remarks>
    /// The assertion that would have caught the defect: the column indices could be right and the width
    /// still taken from the wrong column, which is what actually reflows the text.
    /// </remarks>
    [Fact]
    public void TheSkippingRowsCellTakesItsOwnColumnsWidth()
    {
        PageTable table = Table();

        PageTableCell first = table.Rows[0].Cells[2];
        PageTableCell skipping = table.Rows[1].Cells[0];

        first.Column.ShouldBe(skipping.Column, "both cells sit in the third column");
        table.ColumnWidths[skipping.Column].ShouldBe(Length.FromTwips(6000));
    }

    /// <summary>A <c>w:gridBefore</c> the grid cannot hold does not push the row off the table.</summary>
    /// <remarks>
    /// Real files state nonsense here, and a count past the last column would otherwise index a cell into
    /// a column that does not exist. Clamped rather than dropped: the text is still the document's.
    /// </remarks>
    [Fact]
    public void AnAbsurdGridBeforeIsClamped()
    {
        PageTable table = Table(gridBefore: 99);

        table.Rows[1].Cells[0].Column.ShouldBeLessThanOrEqualTo(PageTable.MaxColumns);
    }

    /// <summary>
    /// The corpus fixture places and breaks its skipping row where LibreOffice does.
    /// </summary>
    /// <remarks>
    /// <c>table-grid-before.docx</c> was rendered by LibreOffice 24.2.7.2, which starts the skipping row's
    /// cell at the third column's left edge and wraps its sentence onto exactly two lines. Measured against
    /// the first column instead — 1500 twips rather than 6000 — the same sentence takes eight, which is the
    /// difference the corpus document exists to pin: a wrong column index is a wrong *width*, and that is
    /// what reflows a page.
    /// </remarks>
    [Fact]
    public void TheCorpusFixtureIsPlacedAndBrokenAsLibreOfficeDoesIt()
    {
        using IDocument document =
            new WordProcessingReader().Read(DocumentSource.FromFile(Corpus.Require("table-grid-before.docx")));
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        PageTable table = pages.Blocks.OfType<PageTable>().Single();
        (List<PlacedTableCell> cells, _) =
            TableLayouter.LayOut(table, new DocPoint(Length.Zero, Length.Zero));

        PlacedTableCell third = cells.Single(cell => cell.Row == 0 && cell.Cell.Column == 2);
        PlacedTableCell skipping = cells.Single(cell => cell.Row == 1);

        skipping.Area.X.ShouldBe(third.Area.X, "the skipping row starts at the third column");
        skipping.Area.Width.ShouldBe(third.Area.Width);
        skipping.Content!.Lines.Count.ShouldBe(2, "LibreOffice breaks the same sentence onto two lines");
    }

    private static PageTable Table(int gridBefore = 2)
    {
        using IDocument document = Open(gridBefore);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        return pages.Blocks.OfType<PageTable>().Single();
    }

    private static IDocument Open(int gridBefore)
    {
        MemoryStream package = BuildPackage(gridBefore);
        using DocumentSource source = DocumentSource.FromStream(package, "grid-before.docx");
        return new WordProcessingReader().Read(source);
    }

    private static MemoryStream BuildPackage(int gridBefore)
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
                  <w:tblPr><w:tblW w:w="9000" w:type="dxa"/><w:tblLayout w:type="fixed"/></w:tblPr>
                  <w:tblGrid>
                    <w:gridCol w:w="1500"/><w:gridCol w:w="1500"/><w:gridCol w:w="6000"/>
                  </w:tblGrid>
                  <w:tr>
                    <w:tc><w:tcPr><w:tcW w:w="1500" w:type="dxa"/></w:tcPr>
                      <w:p><w:r><w:t>a</w:t></w:r></w:p></w:tc>
                    <w:tc><w:tcPr><w:tcW w:w="1500" w:type="dxa"/></w:tcPr>
                      <w:p><w:r><w:t>b</w:t></w:r></w:p></w:tc>
                    <w:tc><w:tcPr><w:tcW w:w="6000" w:type="dxa"/></w:tcPr>
                      <w:p><w:r><w:t>c</w:t></w:r></w:p></w:tc>
                  </w:tr>
                  <w:tr>
                    <w:trPr>
                      <w:gridBefore w:val="{gridBefore}"/>
                      <w:wBefore w:w="3000" w:type="dxa"/>
                    </w:trPr>
                    <w:tc><w:tcPr><w:tcW w:w="6000" w:type="dxa"/></w:tcPr>
                      <w:p><w:r><w:t>wide</w:t></w:r></w:p></w:tc>
                  </w:tr>
                </w:tbl>
                <w:p/>
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
