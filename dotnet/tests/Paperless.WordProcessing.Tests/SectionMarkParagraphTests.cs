using System.IO.Compression;
using System.Text;
using Paperless.Core.Documents;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Whether the paragraph a <c>w:sectPr</c> hangs off is laid out as a paragraph.
/// </summary>
/// <remarks>
/// <para>
/// Word stores a section break as a paragraph mark carrying the section's properties. That mark is not a
/// paragraph — it takes no line and no spacing — and Writer's DOCX importer says so in as many words:
/// "if the paragraph contains only the section properties and it has no runs, we should not create a
/// paragraph for it in Writer, unless that would remove the whole section"
/// (<c>sw/source/writerfilter/dmapper/DomainMapper.cxx</c>:4840). The exception is the
/// <c>!bSingleParagraphAfterRedline</c> term of the same expression: a mark that is both the first and
/// the last paragraph of its section stays, because dropping it leaves the section with nothing to hang
/// a page on.
/// </para>
/// <para>
/// Measured against LibreOffice 24.2.7.2 on three hand-built probes of exactly these shapes, and on
/// <c>easa-form-1.docx</c> in the corpus, whose first section ends with an ordinary empty paragraph and
/// then a mark: laying the mark out overflowed the page and produced a sixth page carrying nothing but
/// that section's footer, against the reference's five.
/// </para>
/// <para>
/// The packages carry a <c>word/settings.xml</c> deliberately — without one a hand-built DOCX does not
/// get LibreOffice's OOXML compatibility defaults.
/// </para>
/// </remarks>
public sealed class SectionMarkParagraphTests
{
    [Fact]
    public void AnEmptySectionMarkIsNotLaidOutAsAParagraph()
    {
        // Section one is "Alpha" then a mark; section two is "Omega". The mark is not the whole of its
        // section, so it goes — leaving two paragraphs where the file has three.
        WordProcessingPages pages = Paginate(
            Paragraph("Alpha")
            + Paragraph(null, Section("continuous"))
            + Paragraph("Omega"));

        // Across the document rather than on one page: the body's own sectPr states no type, so it is a
        // nextPage break and "Omega" starts a second page — which is what LibreOffice renders too.
        Texts(pages).ShouldBe(["Alpha", "Omega"]);
        Texts(pages, 0).ShouldBe(["Alpha"]);
    }

    [Fact]
    public void ASectionMarkCarryingTextIsLaidOutLikeAnyParagraph()
    {
        // The rule is about a mark with *no runs*. One carrying text is an ordinary paragraph that
        // happens to end a section, and removing it would lose the text.
        WordProcessingPages pages = Paginate(
            Paragraph("Alpha")
            + Paragraph("Middle", Section("continuous"))
            + Paragraph("Omega"));

        Texts(pages).ShouldBe(["Alpha", "Middle", "Omega"]);
        Texts(pages, 0).ShouldBe(["Alpha", "Middle"]);
    }

    [Fact]
    public void ASectionWhoseOnlyParagraphIsItsMarkKeepsIt()
    {
        // Two marks in a row: the first ends a section that also holds "Alpha" and is dropped; the second
        // is the whole of its own section and stays, which is what puts a blank page between the two.
        // LibreOffice renders this shape as three pages, the middle one empty.
        WordProcessingPages pages = Paginate(
            Paragraph("Alpha")
            + Paragraph(null, Section("nextPage"))
            + Paragraph(null, Section("nextPage"))
            + Paragraph("Omega"));

        pages.Count.ShouldBe(3);
        Texts(pages, 0).ShouldBe(["Alpha"]);
        Texts(pages, 1).ShouldBe([""]);
        Texts(pages, 2).ShouldBe(["Omega"]);
    }

    /// <summary>The text of every paragraph the document laid out, in order and once each.</summary>
    private static List<string> Texts(WordProcessingPages pages)
        => [.. Enumerable.Range(0, pages.Count).SelectMany(page => Texts(pages, page))];

    /// <summary>The text of each paragraph a page laid out, in order and once each.</summary>
    private static List<string> Texts(WordProcessingPages pages, int page)
    {
        List<string> texts = [];
        int last = -1;

        foreach (PlacedLine line in pages.Pages[page].Lines)
        {
            if (line.ParagraphIndex == last) continue;
            if (line.ParagraphIndex < 0 || line.ParagraphIndex >= pages.Blocks.Count) continue;
            if (pages.Blocks[line.ParagraphIndex] is not PageParagraph paragraph) continue;

            texts.Add(paragraph.Text);
            last = line.ParagraphIndex;
        }

        return texts;
    }

    private static string Paragraph(string? text, string? section = null)
    {
        string properties = section is null ? "" : $"<w:pPr>{section}</w:pPr>";
        string run = text is null ? "" : $"<w:r><w:t xml:space=\"preserve\">{text}</w:t></w:r>";
        return $"<w:p>{properties}{run}</w:p>";
    }

    private static string Section(string? type)
        => $"<w:sectPr>{(type is null ? "" : $"<w:type w:val=\"{type}\"/>")}{Geometry}</w:sectPr>";

    private const string Geometry =
        """
        <w:pgSz w:w="11906" w:h="16838"/>
        <w:pgMar w:top="1134" w:right="1134" w:bottom="1134" w:left="1134"
                 w:header="709" w:footer="709" w:gutter="0"/>
        """;

    private static WordProcessingPages Paginate(string body)
    {
        MemoryStream package = BuildPackage(body);
        using DocumentSource source = DocumentSource.FromStream(package, "sections.docx");
        using IDocument document = new WordProcessingReader().Read(source);

        return (WordProcessingPages)((IPaginatedDocument)document).Layout();
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

        const string Settings = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:compat>
                <w:compatSetting w:name="compatibilityMode"
                                 w:uri="http://schemas.microsoft.com/office/word" w:val="15"/>
              </w:compat>
            </w:settings>
            """;

        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:body>
                {body}
                <w:sectPr>{Geometry}</w:sectPr>
              </w:body>
            </w:document>
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
}
