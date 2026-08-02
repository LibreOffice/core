using System.IO.Compression;
using System.Text;
using Paperless.Core.Documents;
using Paperless.Core.Units;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Which sheet a continuous section's paper size and margins take effect on.
/// </summary>
/// <remarks>
/// <para>
/// A sheet has one paper size and one set of margins, so a section break that does not start a page
/// cannot re-cut the page it lands on: the new geometry applies from the next one. Writer's model forces
/// it — page geometry lives on a page style, <c>SectionPropertyMap::CloseSectionGroup</c> gives a
/// continuous section no page style of its own, and <c>InheritOrFinalizePageStyles</c> then hands it the
/// previous section's (<c>sw/source/writerfilter/dmapper/PropertyMap.cxx</c>:1309-1323, 1722). It is the
/// same rule the page's running head already follows.
/// </para>
/// <para>
/// The corpus measurement this came from is <c>b050-19.docx</c>, whose first section is one empty
/// half-inch-margined paragraph and whose continuous second section is inch-margined: LibreOffice sets
/// page one's text from 36 pt to 574 pt and pages two and three from 72 pt to 539 pt. Switching at the
/// break narrowed page one by a tenth of its measure, which rewrapped it and spilled the document onto a
/// fourth page the reference does not have.
/// </para>
/// <para>
/// The package carries a <c>word/settings.xml</c> deliberately. Without one a hand-built DOCX does not
/// get LibreOffice's OOXML compatibility defaults, and several synthetics built without it have given
/// clean, consistent, wrong answers.
/// </para>
/// </remarks>
public sealed class ContinuousSectionGeometryTests
{
    /// <summary>The first section's margin, half an inch.</summary>
    private static readonly Length Narrow = Length.FromTwips(720);

    /// <summary>The second section's, a whole inch.</summary>
    private static readonly Length Wide = Length.FromTwips(1440);

    /// <summary>Letter, which is what both sections declare.</summary>
    private static readonly Length PageWidth = Length.FromTwips(12240);

    [Fact]
    public void APageSharedWithAContinuousSectionKeepsTheGeometryItStartedWith()
    {
        IReadOnlyList<LaidOutPage> pages = Paginate();

        pages.Count.ShouldBeGreaterThan(1, "the filler is sized to need a second page");
        pages[0].BodyArea.Width.ShouldBe(PageWidth - Narrow - Narrow);
    }

    [Fact]
    public void TheNextPageTakesTheContinuousSectionsGeometry()
    {
        IReadOnlyList<LaidOutPage> pages = Paginate();

        pages[1].BodyArea.Width.ShouldBe(PageWidth - Wide - Wide);
        pages[1].BodyArea.Left.ShouldBe(Wide);
    }

    private static IReadOnlyList<LaidOutPage> Paginate()
    {
        MemoryStream package = BuildPackage();
        using DocumentSource source = DocumentSource.FromStream(package, "continuous.docx");
        using IDocument document = new WordProcessingReader().Read(source);

        return ((WordProcessingPages)((IPaginatedDocument)document).Layout()).Pages;
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
            <w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"/>
            """;

        // Enough paragraphs to run past the first page, so the second one can be asked what geometry it
        // took: the rule is about *when* the change lands, and a one-page document cannot show that.
        string filler = string.Concat(Enumerable.Range(0, 90).Select(
            i => $"<w:p><w:r><w:t>Line {i} of the continuous section's body text.</w:t></w:r></w:p>"));

        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:body>
                <w:p>
                  <w:pPr>
                    <w:sectPr>
                      <w:pgSz w:w="12240" w:h="15840"/>
                      <w:pgMar w:top="720" w:right="720" w:bottom="720" w:left="720"
                               w:header="720" w:footer="720" w:gutter="0"/>
                    </w:sectPr>
                  </w:pPr>
                  <w:r><w:t>Title section</w:t></w:r>
                </w:p>
                {filler}
                <w:sectPr>
                  <w:type w:val="continuous"/>
                  <w:pgSz w:w="12240" w:h="15840"/>
                  <w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"
                           w:header="1440" w:footer="720" w:gutter="0"/>
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
