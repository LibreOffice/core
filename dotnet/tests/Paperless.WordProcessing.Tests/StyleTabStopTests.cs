using System.IO.Compression;
using System.Text;
using Paperless.Core.Documents;
using Paperless.Core.Units;
using Paperless.Text.Layout;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// A paragraph's <c>w:tabs</c> merges with its style's rather than replacing it.
/// </summary>
/// <remarks>
/// <para>
/// <c>DomainMapper</c> seeds the set from the paragraph style before it reads a single <c>w:tab</c>
/// (<c>sw/source/writerfilter/dmapper/DomainMapper.cxx</c>:2604, <c>InitTabStopFromStyle</c>) and folds
/// each one in with <c>IncorporateTabStop</c> (<c>DomainMapper_Impl.cxx</c>:1485): replace at the same
/// position, delete on <c>w:val="clear"</c>, append otherwise.
/// </para>
/// <para>
/// <c>w:val="clear"</c> is the proof that merging is the rule — a set that replaced its style's outright
/// would leave a clear entry nothing to cancel. The corpus case is
/// <c>johnson_hall_service_log.pdf.docx</c>, whose footer clears the <c>Footer</c> style's right stop and
/// adds its own further out; with only the paragraph's stops its third tab had none left to reach, the
/// trailing text started past the margin, and the footer wrapped to two lines.
/// </para>
/// </remarks>
public sealed class StyleTabStopTests
{
    [Fact]
    public void AParagraphKeepsTheStopsItsStyleSetAndDidNotRestate()
    {
        IReadOnlyList<TabStop> stops = StopsOf("merged");

        stops.Select(stop => stop.Position.Twips).ShouldBe([2000L, 4000L, 6000L]);
    }

    [Fact]
    public void AStopAtTheSamePositionIsReplacedRatherThanDoubled()
    {
        IReadOnlyList<TabStop> stops = StopsOf("merged");

        // The style puts a left stop at 4000 and the paragraph a right one there.
        stops.Single(stop => stop.Position.Twips == 4000).Alignment.ShouldBe(TabAlignment.Right);
        stops.Single(stop => stop.Position.Twips == 2000).Alignment.ShouldBe(TabAlignment.Left);
    }

    [Fact]
    public void AClearRemovesTheStopStandingAtItsPosition()
    {
        IReadOnlyList<TabStop> stops = StopsOf("cleared");

        stops.Select(stop => stop.Position.Twips).ShouldBe([2000L]);
    }

    private static IReadOnlyList<TabStop> StopsOf(string text)
    {
        using IDocument document = Open();
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        return pages.Blocks.OfType<PageParagraph>()
            .Single(paragraph => paragraph.Text == text)
            .Format.TabStops;
    }

    private static IDocument Open()
    {
        MemoryStream package = BuildPackage();
        using DocumentSource source = DocumentSource.FromStream(package, "tabs.docx");
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
              <Override PartName="/word/styles.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
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
              <Relationship Id="rId1" Target="styles.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles"/>
            </Relationships>
            """;

        const string Styles = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:style w:type="paragraph" w:default="1" w:styleId="Normal">
                <w:name w:val="Normal"/>
              </w:style>
              <w:style w:type="paragraph" w:styleId="Ruled">
                <w:name w:val="Ruled"/>
                <w:basedOn w:val="Normal"/>
                <w:pPr>
                  <w:tabs>
                    <w:tab w:val="left" w:pos="2000"/>
                    <w:tab w:val="left" w:pos="4000"/>
                  </w:tabs>
                </w:pPr>
              </w:style>
            </w:styles>
            """;

        const string Document = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:body>
                <w:p>
                  <w:pPr>
                    <w:pStyle w:val="Ruled"/>
                    <w:tabs>
                      <w:tab w:val="right" w:pos="4000"/>
                      <w:tab w:val="right" w:pos="6000"/>
                    </w:tabs>
                  </w:pPr>
                  <w:r><w:t>merged</w:t></w:r>
                </w:p>
                <w:p>
                  <w:pPr>
                    <w:pStyle w:val="Ruled"/>
                    <w:tabs><w:tab w:val="clear" w:pos="4000"/></w:tabs>
                  </w:pPr>
                  <w:r><w:t>cleared</w:t></w:r>
                </w:p>
              </w:body>
            </w:document>
            """;

        MemoryStream result = new();
        using (ZipArchive archive = new(result, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml", ContentTypes);
            Write(archive, "_rels/.rels", RootRelationships);
            Write(archive, "word/_rels/document.xml.rels", DocumentRelationships);
            Write(archive, "word/styles.xml", Styles);
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
