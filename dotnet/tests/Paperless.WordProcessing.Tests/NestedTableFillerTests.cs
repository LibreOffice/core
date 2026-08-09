using System.IO.Compression;
using System.Text;
using Paperless.Core.Documents;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// The empty paragraph OOXML makes mandatory after a nested table, and the two conditions that
/// decide whether LibreOffice lays it out.
/// </summary>
/// <remarks>
/// <para>
/// A <c>w:tc</c> may not end with a <c>w:tbl</c>, so Word writes an empty paragraph after every
/// nested table whether or not the author put one there. LibreOffice does not lay that paragraph
/// out, and the difference is a whole line per nested table: on
/// <c>UG.CAO.00133 … Language.docx</c>'s header the row is 26.35 pt in the reference against our
/// 36.65 before this, drawn as three vertical cell edges the operator diff pairs exactly.
/// </para>
/// <para>
/// <strong>The rule was read off the reference, not off the specification</strong>, by mutating that
/// one real cell seven ways and measuring the drawn cell edges in both PDFs
/// (<c>dotnet/probes/words-r44/header-row-mutations.py</c>). The two cases that pin it are the last
/// two below: a trailing paragraph carrying text is laid out, and *two* trailing empty paragraphs
/// are **both** laid out, because the last of them follows a paragraph rather than a table. A rule
/// stated as "drop a cell's trailing empty paragraph" or as "drop an empty paragraph after a table"
/// fails one of those.
/// </para>
/// </remarks>
public sealed class NestedTableFillerTests
{
    [Fact]
    public void TheEmptyParagraphAfterANestedTableIsNotLaidOut()
    {
        IReadOnlyList<PageBlock> cell = Cell("<w:tbl><w:tblGrid><w:gridCol w:w=\"2000\"/></w:tblGrid>"
            + "<w:tr><w:tc><w:p><w:r><w:t>inner</w:t></w:r></w:p></w:tc></w:tr></w:tbl>"
            + "<w:p/>");

        cell.Count.ShouldBe(1);
        cell[0].ShouldBeOfType<PageTable>();
    }

    [Fact]
    public void ATrailingParagraphWithTextIsLaidOut()
    {
        IReadOnlyList<PageBlock> cell = Cell("<w:tbl><w:tblGrid><w:gridCol w:w=\"2000\"/></w:tblGrid>"
            + "<w:tr><w:tc><w:p><w:r><w:t>inner</w:t></w:r></w:p></w:tc></w:tr></w:tbl>"
            + "<w:p><w:r><w:t>after</w:t></w:r></w:p>");

        cell.Count.ShouldBe(2);
        cell[1].ShouldBeOfType<PageParagraph>().Text.ShouldBe("after");
    }

    /// <summary>
    /// Two trailing empty paragraphs are both kept, because the last follows a paragraph.
    /// </summary>
    /// <remarks>
    /// The measurement: LibreOffice draws that cell 49.55 pt tall against 26.35 for the same table
    /// with one trailing paragraph — 23.2 pt more, which is two of its lines and not one. So the
    /// mandatory filler is identified by *what precedes it*, not merely by being last and empty.
    /// </remarks>
    [Fact]
    public void TwoTrailingEmptyParagraphsAreBothLaidOut()
    {
        IReadOnlyList<PageBlock> cell = Cell("<w:tbl><w:tblGrid><w:gridCol w:w=\"2000\"/></w:tblGrid>"
            + "<w:tr><w:tc><w:p><w:r><w:t>inner</w:t></w:r></w:p></w:tc></w:tr></w:tbl>"
            + "<w:p/><w:p/>");

        cell.Count.ShouldBe(3);
        cell.OfType<PageParagraph>().Count().ShouldBe(2);
    }

    /// <summary>
    /// An empty paragraph before the nested table survives; only the trailing one goes.
    /// </summary>
    [Fact]
    public void AnEmptyParagraphBeforeTheNestedTableSurvives()
    {
        IReadOnlyList<PageBlock> cell = Cell("<w:p/>"
            + "<w:tbl><w:tblGrid><w:gridCol w:w=\"2000\"/></w:tblGrid>"
            + "<w:tr><w:tc><w:p><w:r><w:t>inner</w:t></w:r></w:p></w:tc></w:tr></w:tbl>"
            + "<w:p/>");

        cell.Count.ShouldBe(2);
        cell[0].ShouldBeOfType<PageParagraph>();
        cell[1].ShouldBeOfType<PageTable>();
    }

    /// <summary>
    /// A cell of nothing but one empty paragraph keeps it — there is no table for it to follow.
    /// </summary>
    /// <remarks>
    /// The control the rule needs most: an empty cell is how every form in this corpus leaves room to
    /// write, and dropping its paragraph would collapse the row.
    /// </remarks>
    [Fact]
    public void ACellHoldingOnlyAnEmptyParagraphKeepsIt()
    {
        IReadOnlyList<PageBlock> cell = Cell("<w:p/>");

        cell.Count.ShouldBe(1);
        cell[0].ShouldBeOfType<PageParagraph>();
    }

    private static IReadOnlyList<PageBlock> Cell(string content)
    {
        using IDocument document = Open(content);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        return pages.Blocks.OfType<PageTable>().First().Rows[0].Cells[0].Blocks;
    }

    private static IDocument Open(string content)
    {
        MemoryStream package = BuildPackage(content);
        using DocumentSource source = DocumentSource.FromStream(package, "filler.docx");
        return new WordProcessingReader().Read(source);
    }

    private static MemoryStream BuildPackage(string content)
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
                  <w:tblGrid><w:gridCol w:w="6000"/></w:tblGrid>
                  <w:tr>
                    <w:tc>{content}</w:tc>
                  </w:tr>
                </w:tbl>
                <w:p><w:r><w:t>body</w:t></w:r></w:p>
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
