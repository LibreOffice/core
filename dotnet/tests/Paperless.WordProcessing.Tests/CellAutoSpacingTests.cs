using System.IO.Compression;
using System.Text;
using Paperless.Core.Documents;
using Paperless.Core.Units;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// What <c>w:beforeAutospacing</c> and <c>w:afterAutospacing</c> mean at the edges of a table cell.
/// </summary>
/// <remarks>
/// <para>
/// The flag stands for the fourteen-point margin a browser puts between paragraphs, and Word suppresses
/// it where a cell wall already provides the separation: on the first paragraph of a cell and on the
/// last. LibreOffice does the same in two places —
/// <c>DomainMapper_Impl::finishParagraph</c> forces the top margin to zero when
/// <c>bFirstParagraphInCell</c> holds at matching table depth
/// (<c>sw/source/writerfilter/dmapper/DomainMapper_Impl.cxx:2458-2470</c>), and
/// <c>ClearPreviousParagraph</c>, called from <c>TableManager::closeCell</c>, zeroes the bottom margin of
/// the cell's last paragraph unless it is numbered (<c>:5457-5468</c>).
/// </para>
/// <para>
/// Left unimplemented it costs fourteen points on every row of every table whose style carries the flag,
/// which is what a Word form is made of: the corpus's <c>1528039320.docx</c> laid its thirty single-line
/// rows out at 24.85 pt each against LibreOffice's 17.00, and spilled onto a second page the reference
/// does not have.
/// </para>
/// <para>
/// The middle paragraph is asserted too, and a body paragraph outside any table: the rule is about a
/// cell's <em>edges</em>, and suppressing it everywhere would be as wrong as suppressing it nowhere.
/// </para>
/// </remarks>
public sealed class CellAutoSpacingTests
{
    /// <summary>Fourteen points: the margin the flag stands for, and what a non-edge paragraph keeps.</summary>
    private static readonly Length Auto = Length.FromTwips(280);

    [Fact]
    public void ACellsFirstParagraphLosesItsAutoSpaceBefore()
    {
        List<PageParagraph> cell = CellParagraphs();

        cell[0].Format.SpaceBefore.ShouldBe(Length.Zero);
        cell[0].Format.SpaceAfter.ShouldBe(Auto, "only the top edge is an edge for this paragraph");
    }

    [Fact]
    public void ACellsLastParagraphLosesItsAutoSpaceAfter()
    {
        List<PageParagraph> cell = CellParagraphs();

        cell[^1].Format.SpaceAfter.ShouldBe(Length.Zero);
        cell[^1].Format.SpaceBefore.ShouldBe(Auto);
    }

    [Fact]
    public void AParagraphInTheMiddleOfACellKeepsBoth()
    {
        List<PageParagraph> cell = CellParagraphs();

        cell.Count.ShouldBe(3);
        cell[1].Format.SpaceBefore.ShouldBe(Auto);
        cell[1].Format.SpaceAfter.ShouldBe(Auto);
    }

    [Fact]
    public void AParagraphOutsideATableKeepsBoth()
    {
        using IDocument document = Open();
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        PageParagraph body = pages.Blocks.OfType<PageParagraph>()
            .First(paragraph => paragraph.Text == "body");

        body.Format.SpaceBefore.ShouldBe(Auto);
        body.Format.SpaceAfter.ShouldBe(Auto);
    }

    /// <summary>
    /// A stated <c>w:before</c> survives the same position, because the suppression is of the auto rule.
    /// </summary>
    /// <remarks>
    /// The distinction is the whole reason the reader asks how the margin was arrived at rather than
    /// comparing it against fourteen points: a form that spaces its cells deliberately must keep it.
    /// </remarks>
    [Fact]
    public void AStatedMarginAtTheSameEdgeSurvives()
    {
        using IDocument document = Open();
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        PageTable table = pages.Blocks.OfType<PageTable>().Single();
        PageParagraph stated = table.Rows[1].Cells[0].Blocks.OfType<PageParagraph>().Single();

        stated.Text.ShouldBe("stated");
        stated.Format.SpaceBefore.ShouldBe(Length.FromTwips(200));
        stated.Format.SpaceAfter.ShouldBe(Length.FromTwips(200));
    }

    private static List<PageParagraph> CellParagraphs()
    {
        using IDocument document = Open();
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        PageTable table = pages.Blocks.OfType<PageTable>().Single();
        return [.. table.Rows[0].Cells[0].Blocks.OfType<PageParagraph>()];
    }

    private static IDocument Open()
    {
        MemoryStream package = BuildPackage();
        using DocumentSource source = DocumentSource.FromStream(package, "spacing.docx");
        return new WordProcessingReader().Read(source);
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
            </Types>
            """;

        const string RootRelationships = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
              <Relationship Id="rId1" Target="word/document.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"/>
            </Relationships>
            """;

        // Every paragraph asks for auto spacing on both sides, so what differs between them is only
        // where they sit — which is exactly what the rule is about.
        const string Auto = """
            <w:pPr><w:spacing w:before="100" w:beforeAutospacing="1"
                              w:after="100" w:afterAutospacing="1"/></w:pPr>
            """;

        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:body>
                <w:tbl>
                  <w:tblGrid><w:gridCol w:w="4000"/></w:tblGrid>
                  <w:tr>
                    <w:tc>
                      <w:p>{Auto}<w:r><w:t>first</w:t></w:r></w:p>
                      <w:p>{Auto}<w:r><w:t>middle</w:t></w:r></w:p>
                      <w:p>{Auto}<w:r><w:t>last</w:t></w:r></w:p>
                    </w:tc>
                  </w:tr>
                  <w:tr>
                    <w:tc>
                      <w:p>
                        <w:pPr><w:spacing w:before="200" w:after="200"/></w:pPr>
                        <w:r><w:t>stated</w:t></w:r>
                      </w:p>
                    </w:tc>
                  </w:tr>
                </w:tbl>
                <w:p>{Auto}<w:r><w:t>body</w:t></w:r></w:p>
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
