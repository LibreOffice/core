using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// When a <c>continuous</c> section break is not honoured, because the sheet would have to turn.
/// </summary>
/// <remarks>
/// <para>
/// A sheet has one orientation, so a section asking for the other one cannot share a page with the
/// section above it. LibreOffice promotes the break rather than the page:
/// <c>SectionPropertyMap::CloseSectionGroup</c> — "if page orientation differs from previous section, it
/// can't be treated as continuous" — rewrites the type to <c>nextPage</c>
/// (<c>sw/source/writerfilter/dmapper/PropertyMap.cxx</c>:1661-1678).
/// </para>
/// <para>
/// <strong>What it compares is the <c>w:orient</c> flag and nothing else</strong>, which is the half worth
/// pinning: the property is <c>PROP_IS_LANDSCAPE</c>, written once from <c>w:orient</c> alone after being
/// reset to false (<c>DomainMapper.cxx</c>:2859 and :830), so the stated width and height never enter it.
/// Measured across eight authored variants on the installed 26.2.4.2, each a first section holding a page
/// and a line so that a promoted break shows as an extra page: a continuous section 720 twips wider, 720
/// twips taller, one twip wider, or an inch deeper in top margin all stay on the page, and only the one
/// stating <c>w:orient="landscape"</c> takes a new one.
/// </para>
/// <para>
/// No document in the words corpus reaches this — a census of all 134 DOCX finds 19 with a continuous
/// section and <strong>none</strong> across an orientation change — so these tests are the rule's only
/// evidence, and they are written to fail in both directions rather than one.
/// </para>
/// </remarks>
public sealed class ContinuousOrientationBreakTests
{
    [Fact]
    public void AContinuousSectionThatTurnsTheSheetStartsAPage()
    {
        WordProcessingPages pages = Paginate(
            "Alpha", Portrait, $"<w:type w:val=\"continuous\"/>{Landscape}");

        pages.Count.ShouldBe(2, "the sheet has to turn, so the break cannot be continuous");
        pages.Pages[1].Size.Width.ShouldBeGreaterThan(pages.Pages[1].Size.Height);
    }

    [Fact]
    public void AContinuousSectionThatKeepsTheOrientationDoesNotStartAPage()
    {
        // The control that catches promoting every continuous break: same flag on both sides, so the
        // break stays continuous and "Omega" shares Alpha's page.
        WordProcessingPages pages = Paginate(
            "Alpha", Portrait, $"<w:type w:val=\"continuous\"/>{Portrait}");

        pages.Count.ShouldBe(1);
    }

    [Fact]
    public void AContinuousSectionOfADifferentSizeButTheSameOrientationDoesNotStartAPage()
    {
        // The control that catches comparing the *sheet* instead of the flag. Half an inch wider and
        // an inch taller, with neither side stating w:orient — 26.2.4.2 keeps this on one page, and it
        // keeps it there even one twip apart.
        WordProcessingPages pages = Paginate(
            "Alpha", Portrait,
            "<w:type w:val=\"continuous\"/><w:pgSz w:w=\"12960\" w:h=\"16560\"/>" + Margins);

        pages.Count.ShouldBe(1);
    }

    [Fact]
    public void ALandscapeShapedSheetWithoutTheFlagDoesNotCountAsLandscape()
    {
        // The corner that decides between "the flag" and "width against height". Section one states
        // 15840 x 12240 — a physically landscape sheet — with no w:orient; section two is continuous and
        // portrait-shaped. Neither carries the flag, so 26.2.4.2 does not break, and the whole document
        // stays on section one's landscape-shaped sheet.
        WordProcessingPages pages = Paginate(
            "Alpha", $"<w:pgSz w:w=\"15840\" w:h=\"12240\"/>{Margins}",
            $"<w:type w:val=\"continuous\"/>{Portrait}");

        pages.Count.ShouldBe(1);
        pages.Pages[0].Size.Width.ShouldBeGreaterThan(pages.Pages[0].Size.Height);
    }

    private const string Margins =
        "<w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" "
        + "w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/>";

    private const string Portrait = "<w:pgSz w:w=\"12240\" w:h=\"15840\"/>" + Margins;

    private const string Landscape =
        "<w:pgSz w:w=\"15840\" w:h=\"12240\" w:orient=\"landscape\"/>" + Margins;

    /// <summary>
    /// Lays out a two-section document: one paragraph ending the first section, one in the second.
    /// </summary>
    /// <param name="text">The first section's only paragraph.</param>
    /// <param name="first">The first section's <c>w:sectPr</c> content — its geometry and margins.</param>
    /// <param name="second">The document's own <c>w:sectPr</c> content, which closes the second section.</param>
    private static WordProcessingPages Paginate(string text, string first, string second)
    {
        // A DOCX states a section's properties at its end, inside the last paragraph's w:pPr — so the
        // first section's geometry travels with the paragraph that finishes it, not with the one after.
        string body = $"<w:p><w:pPr><w:sectPr>{first}</w:sectPr></w:pPr>"
                      + $"<w:r><w:t xml:space=\"preserve\">{text}</w:t></w:r></w:p>"
                      + "<w:p><w:r><w:t>Omega</w:t></w:r></w:p>";

        MemoryStream package = BuildPackage(body, second);
        using DocumentSource source = DocumentSource.FromStream(package, "orientation.docx");
        using IDocument document = new WordProcessingReader().Read(source);

        return (WordProcessingPages)((IPaginatedDocument)document).Layout();
    }

    private static MemoryStream BuildPackage(string body, string documentSection)
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

        // Without a settings.xml a hand-built DOCX does not get LibreOffice's OOXML compatibility
        // defaults, and several synthetics built without one have given clean, consistent, wrong answers.
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
                <w:sectPr>{documentSection}</w:sectPr>
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
