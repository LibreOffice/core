using System.IO.Compression;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// What a DOCX or an RTF that states no page size and no margins is: US Letter, one inch all round.
/// </summary>
/// <remarks>
/// <para>
/// Both formats reach the page through <c>SectionPropertyMap</c>, whose constructor
/// (<c>sw/source/writerfilter/dmapper/PropertyMap.cxx</c>:459-467) inserts
/// <c>PaperInfo aLetter( PAPER_LETTER )</c>'s width and height with no condition on it at all, and
/// sets all four margins to one inch (<c>:429-434</c>). The locale's own paper — which is what
/// <c>SvxPaperInfo::GetDefaultPaperSize()</c> answers, and what a blank Writer document gets through
/// <c>lcl_DefaultPageFormat</c> (<c>sw/source/core/doc/docdesc.cxx</c>:80) — never enters.
/// </para>
/// <para>
/// The distinction is what the numbers here rest on, because a machine whose locale <em>is</em>
/// American cannot tell the two rules apart. Measured on 24.2.7.2 in a container whose Writer
/// default is A4: a <c>.txt</c> converted through Writer comes out 595 × 842 pt, while an RTF with
/// no <c>\paperw</c> and a DOCX with no <c>w:sectPr</c> both come out 612 × 792 pt with their first
/// glyph at x = 72.1 pt. So Letter here is the filter's, not the machine's.
/// </para>
/// <para>
/// Reaching for A4 instead is not a small difference: every line in the document breaks somewhere
/// else, which put 13 RTFs of a 436-document benchmark corpus wholly out of step with the reference
/// and made them its worst word-processing cases.
/// </para>
/// </remarks>
public sealed class UnstatedPageGeometryTests
{
    private static readonly Length LetterWidth = Length.FromTwips(12240);

    private static readonly Length LetterHeight = Length.FromTwips(15840);

    private static readonly Length OneInch = Length.FromTwips(1440);

    /// <summary>A DOCX with no <c>w:sectPr</c> at all — the shape a generator emits.</summary>
    [Fact]
    public void ADocxWithNoSectionPropertiesIsLetterWithOneInchMargins()
    {
        LaidOutPage page = FirstPage(Docx(sectionProperties: null));

        page.Size.Width.ShouldBe(LetterWidth);
        page.Size.Height.ShouldBe(LetterHeight);
        page.BodyArea.X.ShouldBe(OneInch);
        page.BodyArea.Y.ShouldBe(OneInch);
        page.BodyArea.Width.ShouldBe(LetterWidth - (OneInch * 2));
    }

    /// <summary>
    /// A <c>w:sectPr</c> that states neither <c>w:pgSz</c> nor <c>w:pgMar</c> reaches the same
    /// defaults — the section exists, it just says nothing about the page.
    /// </summary>
    [Fact]
    public void ADocxSectionStatingNeitherSizeNorMarginsIsLetterWithOneInchMargins()
    {
        LaidOutPage page = FirstPage(Docx("<w:sectPr/>"));

        page.Size.Width.ShouldBe(LetterWidth);
        page.Size.Height.ShouldBe(LetterHeight);
        page.BodyArea.X.ShouldBe(OneInch);
    }

    /// <summary>
    /// The half of the rule that matters most in practice: only the missing half falls back.
    /// </summary>
    /// <remarks>
    /// A section stating A4 keeps A4, and one stating its own margins keeps them. Without this the
    /// change would be a magnet rather than a default, and would move every document in the corpus
    /// instead of the ones that state nothing.
    /// </remarks>
    [Fact]
    public void AStatedPageSizeAndStatedMarginsAreLeftAlone()
    {
        LaidOutPage page = FirstPage(Docx(
            """
            <w:sectPr>
              <w:pgSz w:w="11906" w:h="16838"/>
              <w:pgMar w:top="1134" w:right="1134" w:bottom="1134" w:left="1134"
                       w:header="0" w:footer="0" w:gutter="0"/>
            </w:sectPr>
            """));

        page.Size.Width.ShouldBe(Length.FromTwips(11906));
        page.Size.Height.ShouldBe(Length.FromTwips(16838));
        page.BodyArea.X.ShouldBe(Length.FromTwips(1134));
    }

    /// <summary>
    /// A stated size with unstated margins takes one inch, not the 2 cm a blank Writer page has.
    /// </summary>
    [Fact]
    public void UnstatedMarginsAreOneInchEvenWhenTheSizeIsStated()
    {
        LaidOutPage page = FirstPage(Docx("""<w:sectPr><w:pgSz w:w="11906" w:h="16838"/></w:sectPr>"""));

        page.Size.Width.ShouldBe(Length.FromTwips(11906));
        page.BodyArea.X.ShouldBe(OneInch);
        page.BodyArea.Y.ShouldBe(OneInch);
    }

    /// <summary>
    /// RTF reaches the same defaults, and this is the arm the corpus actually hit: pandoc and other
    /// generators write an RTF with a font table, a colour table and no page geometry whatsoever.
    /// </summary>
    [Fact]
    public void AnRtfWithNoPaperOrMarginTokensIsLetterWithOneInchMargins()
    {
        LaidOutPage page = FirstPage(Rtf(
            @"{\rtf1\ansi\deff0{\fonttbl{\f0 \fswiss Helvetica;}}"
            + @"{\pard \f0 One paragraph on one sheet.\par}}"));

        page.Size.Width.ShouldBe(LetterWidth);
        page.Size.Height.ShouldBe(LetterHeight);
        page.BodyArea.X.ShouldBe(OneInch);
        page.BodyArea.Y.ShouldBe(OneInch);
    }

    /// <summary>An RTF that states its paper and margins keeps them.</summary>
    [Fact]
    public void AnRtfStatingPaperAndMarginsIsLeftAlone()
    {
        LaidOutPage page = FirstPage(Rtf(
            @"{\rtf1\ansi\paperw11906\paperh16838\margl720\margr720\margt720\margb720"
            + @"\pard One paragraph on one sheet.\par}"));

        page.Size.Width.ShouldBe(Length.FromTwips(11906));
        page.BodyArea.X.ShouldBe(Length.FromTwips(720));
    }

    private static LaidOutPage FirstPage(DocumentSource source)
    {
        using (source)
        {
            using IDocument document = new WordProcessingReader().Read(source);
            return ((WordProcessingPages)((IPaginatedDocument)document).Layout()).Pages[0];
        }
    }

    private static DocumentSource Rtf(string text)
        => DocumentSource.FromStream(new MemoryStream(Encoding.ASCII.GetBytes(text)), "page.rtf");

    private static DocumentSource Docx(string? sectionProperties)
        => DocumentSource.FromStream(BuildPackage(sectionProperties), "page.docx");

    private static MemoryStream BuildPackage(string? sectionProperties)
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

        // As in the neighbouring geometry tests: a hand-built DOCX with no settings part misses
        // LibreOffice's OOXML compatibility defaults and can give a clean, consistent, wrong answer.
        const string Settings = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"/>
            """;

        string document = $"""
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:body>
                <w:p><w:r><w:t>One paragraph on one sheet.</w:t></w:r></w:p>
                {sectionProperties}
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

        static void Write(ZipArchive archive, string path, string content)
        {
            using StreamWriter writer = new(archive.CreateEntry(path).Open(), Encoding.UTF8);
            writer.Write(content);
        }
    }
}
