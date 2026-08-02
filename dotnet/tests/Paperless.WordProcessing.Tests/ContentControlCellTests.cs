using System.IO.Compression;
using System.Text;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// A table cell wrapped in a content control.
/// </summary>
/// <remarks>
/// <para>
/// A <c>w:sdt</c> can sit at three levels of a table — around the whole table's rows, around one row, and
/// around a single cell — and the third is the one every Word form uses: each fill-in box of a printed
/// form is a cell inside its own content control. Both readers followed the wrapper down to a row and
/// stopped, taking only a row's direct <c>w:tc</c> children, so a wrapped cell and everything in it
/// vanished. The corpus's own lease-proposal form lost thirty-six cells that way — a quarter of its
/// words, with its pagination unchanged, which is why nothing else showed it.
/// </para>
/// <para>
/// Asserted on both paths, because they are two walks: extraction builds a
/// <see cref="ContentTable"/> and layout builds a <see cref="PageTable"/>, and fixing one leaves the
/// other silently short.
/// </para>
/// </remarks>
public sealed class ContentControlCellTests
{
    /// <summary>The extraction walk finds a cell inside a content control.</summary>
    [Fact]
    public void ExtractionKeepsACellInsideAContentControl()
    {
        using IDocument document = Open();

        ContentTable table = document.Content.Children.OfType<ContentSection>()
            .SelectMany(section => section.Children).OfType<ContentTable>().Single();

        table.ColumnCount.ShouldBe(3);
        table.GetText().ShouldBe("plain cell\twrapped cell\tdoubly wrapped cell\n");
    }

    /// <summary>And so does the layout walk.</summary>
    [Fact]
    public void LayoutKeepsACellInsideAContentControl()
    {
        using IDocument document = Open();
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        PageTable table = pages.Blocks.OfType<PageTable>().Single();
        List<string> cells =
        [
            .. table.Rows[0].Cells.SelectMany(
                cell => cell.Blocks.OfType<PageParagraph>().Select(block => block.Text)),
        ];

        cells.ShouldBe(["plain cell", "wrapped cell", "doubly wrapped cell"]);
    }

    private static IDocument Open()
    {
        MemoryStream package = BuildPackage();
        using DocumentSource source = DocumentSource.FromStream(package, "form.docx");
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

        const string Document = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:body>
                <w:tbl>
                  <w:tblGrid><w:gridCol w:w="3000"/><w:gridCol w:w="3000"/><w:gridCol w:w="3000"/></w:tblGrid>
                  <w:tr>
                    <w:tc><w:p><w:r><w:t>plain cell</w:t></w:r></w:p></w:tc>
                    <w:sdt>
                      <w:sdtPr><w:alias w:val="one"/></w:sdtPr>
                      <w:sdtContent>
                        <w:tc><w:p><w:r><w:t>wrapped cell</w:t></w:r></w:p></w:tc>
                      </w:sdtContent>
                    </w:sdt>
                    <w:sdt>
                      <w:sdtContent>
                        <w:sdt>
                          <w:sdtContent>
                            <w:tc><w:p><w:r><w:t>doubly wrapped cell</w:t></w:r></w:p></w:tc>
                          </w:sdtContent>
                        </w:sdt>
                      </w:sdtContent>
                    </w:sdt>
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
            Write(archive, "word/document.xml", Document);
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
