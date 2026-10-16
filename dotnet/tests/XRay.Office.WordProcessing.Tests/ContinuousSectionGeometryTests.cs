using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// Which sheet a continuous section's paper size and margins take effect on — and, for most such
/// sections, that the answer is <em>none</em>.
/// </summary>
/// <remarks>
/// <para>
/// Page geometry lives on a page style, and <c>SectionPropertyMap::CloseSectionGroup</c> gives a
/// continuous section no page style of its own: <c>InheritOrFinalizePageStyles</c> hands it the
/// previous section's outright (<c>sw/source/writerfilter/dmapper/PropertyMap.cxx</c>:1309-1323,
/// 1722). A style is created for it only where a header or footer of its own had to be imported into
/// one, and it then reaches a page only by being hung on a hard page break inside the section
/// (<c>:1746-1801</c>). So the margins and the running head stand or fall together — see
/// <see cref="XRay.Office.WordProcessing.Layout.ContinuousPageDescriptors"/>.
/// </para>
/// <para>
/// <strong>This class asserted the opposite until 2026-09-07</strong>, in the form <em>"the new
/// geometry applies from the next page"</em>, and its own synthetic refutes it: rendered through
/// 26.2.4.2, a continuous section naming no furniture leaves page two on the <em>first</em> section's
/// half-inch margins, at 36.1 pt, where the test demanded 72. The corpus measurement it was written
/// from — <c>b050-19.docx</c>, page one's text 36 pt to 574 pt and pages two and three 72 pt to
/// 539 pt — is real and is the <em>other</em> case: that document's continuous section names a header
/// of its own <em>and</em> contains a hard page break, so its descriptor does land, and it lands at
/// the break rather than at the next page. The synthetic never reproduced that shape, so it read the
/// right corpus behaviour off the wrong mechanism.
/// </para>
/// <para>
/// All three variants below are rendered through 26.2.4.2 in
/// <c>dotnet/probes/words-continuous-r72/synthetic.py</c>, and ours agrees with it on every page of
/// every one to 0.1 pt.
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

    /// <summary>The page the break lands on keeps the geometry it started with.</summary>
    [Fact]
    public void APageSharedWithAContinuousSectionKeepsTheGeometryItStartedWith()
    {
        IReadOnlyList<LaidOutPage> pages = Paginate();

        pages.Count.ShouldBeGreaterThan(1, "the filler is sized to need a second page");
        pages[0].BodyArea.Width.ShouldBe(PageWidth - Narrow - Narrow);
    }

    /// <summary>
    /// And so does every page after it, while the section names no furniture of its own.
    /// </summary>
    /// <remarks>
    /// The descriptor is never built, so there is nothing for a later sheet to pick up. 26.2.4.2 puts
    /// page two's text at 36.1 pt on this exact package.
    /// </remarks>
    [Fact]
    public void SoDoesEveryPageAfterIt()
    {
        IReadOnlyList<LaidOutPage> pages = Paginate();

        pages[1].BodyArea.Width.ShouldBe(PageWidth - Narrow - Narrow);
        pages[1].BodyArea.Left.ShouldBe(Narrow);
    }

    /// <summary>
    /// Naming a header of its own is not enough either, while the section holds no hard page break.
    /// </summary>
    /// <remarks>
    /// The style is built and hung on nothing. Measured: 26.2.4.2 draws this header on no page and
    /// leaves both pages at 36.1 pt.
    /// </remarks>
    [Fact]
    public void NamingAHeaderIsNotEnoughWithoutAHardBreak()
    {
        IReadOnlyList<LaidOutPage> pages = Paginate(header: true);

        pages[1].BodyArea.Left.ShouldBe(Narrow);
        pages[1].Header.ShouldBeNull();
    }

    /// <summary>
    /// With both, the margins and the header arrive together, on the page the break starts.
    /// </summary>
    /// <remarks>
    /// This is <c>b050-19.docx</c>'s shape and the case the old assertion was really describing.
    /// Measured: 26.2.4.2 gives three pages, page one at 36.1 pt with no head, pages two and three at
    /// 72.1 pt carrying it.
    /// </remarks>
    [Fact]
    public void AHardBreakBringsTheMarginsAndTheHeaderTogether()
    {
        IReadOnlyList<LaidOutPage> pages = Paginate(header: true, hardBreak: true);

        pages[0].BodyArea.Left.ShouldBe(Narrow);
        pages[0].Header.ShouldBeNull();

        pages[1].BodyArea.Left.ShouldBe(Wide);
        pages[1].BodyArea.Width.ShouldBe(PageWidth - Wide - Wide);
        pages[1].Header.ShouldNotBeNull();
    }

    private static IReadOnlyList<LaidOutPage> Paginate(bool header = false, bool hardBreak = false)
    {
        MemoryStream package = BuildPackage(header, hardBreak);
        using DocumentSource source = DocumentSource.FromStream(package, "continuous.docx");
        using IDocument document = new WordProcessingReader().Read(source);

        return ((WordProcessingPages)((IPaginatedDocument)document).Layout()).Pages;
    }

    private static MemoryStream BuildPackage(bool header, bool hardBreak)
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
              <Override PartName="/word/header1.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml"/>
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
              <Relationship Id="rId2" Target="header1.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/header"/>
            </Relationships>
            """;

        const string Settings = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"/>
            """;

        const string Header = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:hdr xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:p><w:r><w:t>SECOND SECTION RUNNING HEAD</w:t></w:r></w:p>
            </w:hdr>
            """;

        // Enough paragraphs to run past the first page, so the second one can be asked what geometry it
        // took: the rule is about *when* the change lands, and a one-page document cannot show that.
        // The hard break, when the variant wants one, is put well inside the section rather than at its
        // start: the descriptor lands on the page that break begins, and a break at the section's first
        // paragraph would make "the page the break starts" and "the page after the section break" the
        // same sheet, which is exactly the confusion the old assertion here rested on.
        string filler = string.Concat(Enumerable.Range(0, 90).Select(
            i => $"<w:p>{(hardBreak && i == 40 ? "<w:pPr><w:pageBreakBefore/></w:pPr>" : string.Empty)}"
                 + $"<w:r><w:t>Line {i} of the continuous section's body text.</w:t></w:r></w:p>"));

        string headerReference = header
            ? """<w:headerReference w:type="default" r:id="rId2"/>"""
            : string.Empty;

        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
                        xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
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
                  {headerReference}<w:type w:val="continuous"/>
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
            Write(archive, "word/header1.xml", Header);
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
