using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// What a DOCX falls back to when its style chain states no size or face, which is not one answer.
/// </summary>
/// <remarks>
/// <para>
/// A DOCX import starts at <b>Calibri 11 pt</b> — <c>DomainMapper::DomainMapper</c>
/// (<c>sw/source/writerfilter/dmapper/DomainMapper.cxx</c>:182-193, tdf#108350), <em>"In Word since
/// version 2007, the default document font is Calibri 11 pt. If a DOCX document doesn't contain font
/// information, we should assume the intended font to provide best layout match."</em> The moment a
/// <c>w:docDefaults/w:rPrDefault</c> is seen at all, <c>StyleSheetTable::applyDefaults(false)</c>
/// resets the document defaults to <b>Times New Roman 10 pt</b> and lays the file's own values over
/// them (<c>StyleSheetTable.cxx</c>:2161-2180, and :341-350 for the 10 pt).
/// </para>
/// <para>
/// <b>Presence decides, not content</b>, which is the half a reader gets wrong: an empty
/// <c>&lt;w:rPrDefault&gt;&lt;w:rPr/&gt;&lt;/w:rPrDefault&gt;</c> gives 10 pt and a missing one gives
/// 11 pt. Measured in <c>dotnet/probes/words-empty-paragraph-height/</c> against both installed
/// references, which agree on every row — an empty paragraph between two 12 pt Liberation Serif
/// lines, the gap between those lines in PDF points:
/// </para>
/// <list type="table">
///   <item><term>no styles part at all</term><description><b>27.25</b></description></item>
///   <item><term>an empty <c>w:rPrDefault</c></term><description>25.35</description></item>
///   <item><term>one naming Carlito</term><description>26.00</description></item>
///   <item><term>one stating <c>w:sz w:val="28"</c></term><description>29.90</description></item>
///   <item><term>both</term><description>30.90</description></item>
/// </list>
/// <para>
/// Only the first row moved: 13.45 pt of empty paragraph against our 11.55, which is Calibri 11 pt
/// against Times New Roman 10. <b>No document in the words corpus reaches it</b> — all 272 of its
/// DOCX-family files declare a <c>w:rPrDefault</c> — so its witnesses are hand-built packages, which
/// is what this project's probe fixtures and many of its test fixtures are.
/// </para>
/// </remarks>
public sealed class DocDefaultsFallbackTests
{
    /// <summary>A package with no <c>w:rPrDefault</c> keeps the importer's Calibri 11 pt.</summary>
    /// <remarks>
    /// Both arms of the absence: a package with no styles part at all, and one whose
    /// <c>w:docDefaults</c> declares only a <c>w:pPrDefault</c>. The second is the one that says the
    /// rule keys on the run element rather than on <c>w:docDefaults</c>.
    /// </remarks>
    [Theory]
    [InlineData(null)]
    [InlineData("<w:docDefaults><w:pPrDefault/></w:docDefaults>")]
    public void WithNoRunDefaultAParagraphIsCalibriElevenPoint(string? defaults)
    {
        PageParagraph paragraph = Empty(defaults);

        paragraph.EmSize.ShouldBe(Length.FromPoints(11));
    }

    /// <summary>A <c>w:rPrDefault</c> resets it to 10 pt, however empty it is.</summary>
    [Theory]
    [InlineData("<w:docDefaults><w:rPrDefault/></w:docDefaults>")]
    [InlineData("<w:docDefaults><w:rPrDefault><w:rPr/></w:rPrDefault></w:docDefaults>")]
    [InlineData("<w:docDefaults><w:rPrDefault><w:rPr>"
                + "<w:rFonts w:ascii=\"Carlito\" w:hAnsi=\"Carlito\"/></w:rPr></w:rPrDefault></w:docDefaults>")]
    public void AnyRunDefaultResetsItToTenPoint(string defaults)
    {
        PageParagraph paragraph = Empty(defaults);

        paragraph.EmSize.ShouldBe(Length.FromPoints(10));
    }

    /// <summary>And the size it states wins over both.</summary>
    [Fact]
    public void AStatedDefaultSizeWins()
    {
        PageParagraph paragraph = Empty(
            "<w:docDefaults><w:rPrDefault><w:rPr><w:sz w:val=\"28\"/></w:rPr>"
            + "</w:rPrDefault></w:docDefaults>");

        paragraph.EmSize.ShouldBe(Length.FromPoints(14));
    }

    /// <summary>The empty paragraph of a package whose <c>w:docDefaults</c> are as given.</summary>
    /// <remarks>
    /// An empty paragraph because it is the case with nothing else to be sized by: its height is its
    /// mark's, and its mark here states nothing at all. The run carries a face and a size that the
    /// reference ignores outright — a mark with no <c>w:rPr</c> does not inherit its paragraph's run
    /// formatting — which is why the probe's four size and two face arms all read the same figure.
    /// </remarks>
    private static PageParagraph Empty(string? defaults)
    {
        string body =
            "<w:p><w:r><w:rPr><w:rFonts w:ascii=\"Liberation Serif\" w:hAnsi=\"Liberation Serif\"/>"
            + "<w:sz w:val=\"48\"/></w:rPr><w:t></w:t></w:r></w:p>";

        using MemoryStream package = BuildPackage(body, defaults);
        using DocumentSource source = DocumentSource.FromStream(package, "doc-defaults.docx");
        using IDocument document = new WordProcessingReader().Read(source);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        return pages.Blocks.OfType<PageParagraph>().Single();
    }

    private static MemoryStream BuildPackage(string body, string? defaults)
    {
        string contentTypes = ContentTypes
            + (defaults is null ? string.Empty : StylesOverride)
            + "</Types>";

        string documentRelationships =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            + "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
            + (defaults is null
                ? string.Empty
                : "<Relationship Id=\"rIdS\" Target=\"styles.xml\" "
                  + "Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles\"/>")
            + "</Relationships>";

        string document =
            $"<?xml version=\"1.0\" encoding=\"UTF-8\"?><w:document xmlns:w=\"{W}\"><w:body>{body}"
            + "<w:sectPr><w:pgSz w:w=\"11906\" w:h=\"16838\"/>"
            + "<w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\"/>"
            + "</w:sectPr></w:body></w:document>";

        MemoryStream result = new();
        using (ZipArchive archive = new(result, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml", contentTypes);
            Write(archive, "_rels/.rels", RootRelationships);
            Write(archive, "word/_rels/document.xml.rels", documentRelationships);
            Write(archive, "word/document.xml", document);
            if (defaults is not null)
            {
                Write(archive, "word/styles.xml",
                    $"<?xml version=\"1.0\" encoding=\"UTF-8\"?><w:styles xmlns:w=\"{W}\">{defaults}</w:styles>");
            }
        }

        result.Position = 0;
        return result;

        static void Write(ZipArchive archive, string name, string content)
        {
            using Stream entry = archive.CreateEntry(name).Open();
            entry.Write(Encoding.UTF8.GetBytes(content));
        }
    }

    private const string ContentTypes =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        + "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
        + "<Default Extension=\"rels\" "
        + "ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
        + "<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
        + "<Override PartName=\"/word/document.xml\" ContentType=\""
        + "application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml\"/>";

    private const string StylesOverride =
        "<Override PartName=\"/word/styles.xml\" ContentType=\""
        + "application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml\"/>";

    private const string RootRelationships =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        + "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
        + "<Relationship Id=\"rId1\" Target=\"word/document.xml\" Type=\""
        + "http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\"/>"
        + "</Relationships>";

    private const string W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
}
