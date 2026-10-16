using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// That a <c>FILENAME</c> and a <c>TITLE</c> field are evaluated rather than drawn from their cache.
/// </summary>
/// <remarks>
/// <para>
/// The cached result of these two is a statement about a file that no longer exists, and LibreOffice
/// re-evaluates both on load. Measured on <c>CRIF - Sp…cification technique - Socle applicatif.docx</c>,
/// whose header caches <c>ENT</c> for its title and whose footer caches
/// <c>SPECTECH-socle-applicatif.doc</c> for two <c>FILENAME</c> fields: the reference drew 13 words a
/// page that we did not, on all 27 pages, which was 351 of that document's 363-word gap.
/// </para>
/// <para>
/// Synthetic rather than a corpus document, because what is under test is a shape rather than a file:
/// the three spellings a producer uses — <c>w:fldSimple</c>, the <c>w:fldChar</c> form with a separator
/// and a cached result, and the <c>w:fldChar</c> form with <em>no</em> separator and so no result at
/// all. The third is the one a reader is most likely to pass over in silence, and the CRIF footer has
/// one.
/// </para>
/// </remarks>
public sealed class ConstantFieldTests
{
    /// <summary>Each of the three spellings draws the value rather than the cache.</summary>
    [Fact]
    public void AFileNameFieldIsEvaluatedInEveryFormItIsWrittenIn()
    {
        string footer = FooterText(Paginate("report-2026.docx")[0]);

        footer.ShouldBe("[report-2026.docx|report-2026.docx|report-2026.docx]");
    }

    /// <summary>A <c>TITLE</c> field takes the package's <c>dc:title</c>, not its cached result.</summary>
    [Fact]
    public void ATitleFieldTakesThePackageTitle()
    {
        HeaderText(Paginate("report-2026.docx")[0]).ShouldBe("<A Longer Title Than The Cache>");
    }

    /// <summary>
    /// A document read from a nameless stream keeps its cached file name.
    /// </summary>
    /// <remarks>
    /// The lenient half of the rule, and the reason <c>ConstantFields.FileName</c> is nullable rather
    /// than defaulted to something: X-Ray.Office reads streams as readily as files, and a stale name is
    /// closer to what a reader saw than an empty footer is.
    /// </remarks>
    [Fact]
    public void ANamelessStreamKeepsTheCachedFileName()
    {
        FooterText(Paginate(null)[0]).ShouldBe("[cached.doc|cached.doc|]");
    }

    /// <summary>
    /// A <c>FILENAME \p</c> keeps its cache, because the path it asks for is not knowable here.
    /// </summary>
    /// <remarks>
    /// <c>DomainMapper_Impl.cxx</c>:8296 switches on <c>\p</c> to
    /// <c>FilenameDisplayFormat::FULL</c>. Substituting the leaf name there would draw a shorter string
    /// than the reference rather than a different one, which moves line breaks for no gain.
    /// </remarks>
    [Fact]
    public void APathSwitchedFileNameFieldKeepsItsCache()
    {
        FieldInstructions.ConstantFieldOf(" FILENAME \\p ").ShouldBeNull();
        FieldInstructions.ConstantFieldOf(" FILENAME  \\* MERGEFORMAT ")
            .ShouldBe(ConstantField.FileName);
        FieldInstructions.ConstantFieldOf(" TITLE   \\* MERGEFORMAT ").ShouldBe(ConstantField.Title);
        FieldInstructions.ConstantFieldOf(" DOCPROPERTY Title ").ShouldBeNull();
    }

    private static string HeaderText(LaidOutPage page)
    {
        page.Header.ShouldNotBeNull();
        return string.Concat(page.Header!.Blocks.OfType<PageParagraph>().Select(p => p.Text));
    }

    private static string FooterText(LaidOutPage page)
    {
        page.Footer.ShouldNotBeNull();
        return string.Concat(page.Footer!.Blocks.OfType<PageParagraph>().Select(p => p.Text));
    }

    private static IReadOnlyList<LaidOutPage> Paginate(string? name)
    {
        MemoryStream bytes = BuildPackage();
        using DocumentSource source = name is null
            ? DocumentSource.FromStream(bytes)
            : DocumentSource.FromStream(bytes, name);

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
              <Override PartName="/word/header1.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml"/>
              <Override PartName="/word/footer1.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.footer+xml"/>
              <Override PartName="/word/settings.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
              <Override PartName="/docProps/core.xml"
                        ContentType="application/vnd.openxmlformats-package.core-properties+xml"/>
            </Types>
            """;

        const string RootRelationships = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
              <Relationship Id="rId1" Target="word/document.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"/>
              <Relationship Id="rId9" Target="docProps/core.xml"
                            Type="http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties"/>
            </Relationships>
            """;

        const string DocumentRelationships = """
            <?xml version="1.0" encoding="UTF-8"?>
            <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
              <Relationship Id="rId1" Target="settings.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings"/>
              <Relationship Id="rId2" Target="footer1.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/footer"/>
              <Relationship Id="rId3" Target="header1.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/header"/>
            </Relationships>
            """;

        const string Settings = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"/>
            """;

        const string Core = """
            <?xml version="1.0" encoding="UTF-8"?>
            <cp:coreProperties
                xmlns:cp="http://schemas.openxmlformats.org/package/2006/metadata/core-properties"
                xmlns:dc="http://purl.org/dc/elements/1.1/">
              <dc:title>A Longer Title Than The Cache</dc:title>
            </cp:coreProperties>
            """;

        // Three spellings of the same field. The last has no `separate` and so no cached result at all,
        // which is how the CRIF footer's second FILENAME is written and what LibreOffice still draws.
        const string Footer = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:ftr xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:p>
                <w:r><w:t>[</w:t></w:r>
                <w:fldSimple w:instr=" FILENAME   \* MERGEFORMAT ">
                  <w:r><w:t>cached</w:t></w:r><w:r><w:t>.doc</w:t></w:r>
                </w:fldSimple>
                <w:r><w:t>|</w:t></w:r>
                <w:r><w:fldChar w:fldCharType="begin"/></w:r>
                <w:r><w:instrText xml:space="preserve"> FILE</w:instrText></w:r>
                <w:r><w:instrText xml:space="preserve">NAME  \* MERGEFORMAT </w:instrText></w:r>
                <w:r><w:fldChar w:fldCharType="separate"/></w:r>
                <w:r><w:t>cached.doc</w:t></w:r>
                <w:r><w:fldChar w:fldCharType="end"/></w:r>
                <w:r><w:t>|</w:t></w:r>
                <w:r><w:fldChar w:fldCharType="begin"/></w:r>
                <w:r><w:instrText xml:space="preserve"> FILENAME </w:instrText></w:r>
                <w:r><w:fldChar w:fldCharType="end"/></w:r>
                <w:r><w:t>]</w:t></w:r>
              </w:p>
            </w:ftr>
            """;

        const string Header = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:hdr xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:p>
                <w:r><w:t>&lt;</w:t></w:r>
                <w:fldSimple w:instr=" TITLE   \* MERGEFORMAT ">
                  <w:r><w:t>ENT</w:t></w:r>
                </w:fldSimple>
                <w:r><w:t>&gt;</w:t></w:r>
              </w:p>
            </w:hdr>
            """;

        const string Document = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
                        xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
              <w:body>
                <w:p><w:r><w:t>Body.</w:t></w:r></w:p>
                <w:sectPr>
                  <w:headerReference w:type="default" r:id="rId3"/>
                  <w:footerReference w:type="default" r:id="rId2"/>
                  <w:pgSz w:w="12240" w:h="4000"/>
                  <w:pgMar w:top="720" w:right="1440" w:bottom="720" w:left="1440"
                           w:header="360" w:footer="360"/>
                </w:sectPr>
              </w:body>
            </w:document>
            """;

        MemoryStream stream = new();
        using (ZipArchive archive = new(stream, ZipArchiveMode.Create, leaveOpen: true))
        {
            Add(archive, "[Content_Types].xml", ContentTypes);
            Add(archive, "_rels/.rels", RootRelationships);
            Add(archive, "docProps/core.xml", Core);
            Add(archive, "word/_rels/document.xml.rels", DocumentRelationships);
            Add(archive, "word/settings.xml", Settings);
            Add(archive, "word/header1.xml", Header);
            Add(archive, "word/footer1.xml", Footer);
            Add(archive, "word/document.xml", Document);
        }

        stream.Position = 0;
        return stream;

        static void Add(ZipArchive archive, string name, string content)
        {
            using StreamWriter writer = new(archive.CreateEntry(name).Open(), Encoding.UTF8);
            writer.Write(content);
        }
    }
}
