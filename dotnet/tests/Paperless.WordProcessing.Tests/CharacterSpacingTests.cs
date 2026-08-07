using System.IO.Compression;
using System.Text;
using Paperless.Core.Documents;
using Paperless.Core.Units;
using Paperless.TestKit;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// The <c>w:spacing</c> of a <c>w:rPr</c>, which is tracking and not paragraph spacing.
/// </summary>
/// <remarks>
/// <para>
/// Two unrelated properties share the name. In a <c>w:pPr</c> it is the gap above and below a paragraph;
/// in a <c>w:rPr</c> it is a constant distance put <em>between characters</em> — <c>SvxKerningItem</c>,
/// which <c>DomainMapper</c> reaches by converting the twips straight to <c>CharKerning</c>
/// (<c>sw/source/writerfilter/dmapper/DomainMapper.cxx</c>:2468–2480). It is not the face's own pair
/// kerning either; that is <c>w:kern</c> and a different item.
/// </para>
/// <para>
/// Nothing read the character one. <c>FormattedRun.Tracking</c> had been built for it and only the
/// presentations reader populated it, which is the shape of defect this project keeps finding: a property
/// modelled everywhere and consumed nowhere.
/// </para>
/// <para>
/// Found on <c>words/batch-011/docx/FG-AI4NDM-TR-skeleton_template.docx</c>, whose ITU cover sets
/// "International Telecommunication Union" at <c>w:spacing w:val="100"</c> — 5 pt between every letter.
/// 58 of the words track's 134 DOCX state one, most of them negative, and a run at −16 twips over a
/// fifty-character line is 40 pt of width the reference does not spend.
/// </para>
/// </remarks>
public sealed class CharacterSpacingTests
{
    /// <summary>Tracking widens a paragraph by one unit per character gap.</summary>
    /// <remarks>
    /// Asserted as a difference against the same text untracked rather than against an absolute width, so
    /// the test says what tracking costs without also pinning the face's advances.
    /// </remarks>
    [Theory]
    [InlineData(100)]
    [InlineData(-10)]
    public void TrackingChangesTheTextsWidthByOneUnitPerGap(int twips)
    {
        const string Sentence = "Tracking is a distance between letters.";

        Length bare = Width(Sentence, spacing: null);
        Length tracked = Width(Sentence, spacing: twips);

        // The measurement charges the gap before each character and the first pays nothing, so a
        // paragraph of n characters carries n − 1. See FormattedRun.Tracking.
        (tracked - bare).ShouldBe(Length.FromTwips(twips) * (Sentence.Length - 1));
    }

    /// <summary>A run's own tracking survives the uniform-paragraph shortcut.</summary>
    /// <remarks>
    /// The readers drop a paragraph's run list when every run agrees with the paragraph mark, because a
    /// property that only decides what a mark looks like cannot decide where it lands. Tracking is not
    /// such a property — it is a width — so a tracked run inside an untracked paragraph has to keep its
    /// run or it is measured at the paragraph's answer. This is the case that would silently pass.
    /// </remarks>
    [Fact]
    public void ATrackedRunInsideAnUntrackedParagraphKeepsItsWidth()
    {
        PageParagraph paragraph = Paragraph(MixedDocument);

        paragraph.Runs.Count.ShouldBeGreaterThan(1,
            "the tracked run differs from the paragraph mark, so the shortcut must not fold it away");
        paragraph.Runs.Select(run => run.Tracking)
            .ShouldContain(Length.FromTwips(100));
    }

    /// <summary>The paragraph <c>w:spacing</c> is not mistaken for the character one.</summary>
    /// <remarks>
    /// The two elements have the same name and different parents, and the resolution that finds the
    /// character one only ever looks inside <c>w:rPr</c>. Without this the gap above a paragraph would be
    /// charged as tracking on every one of its letters, which is a defect no width test on a tracked
    /// document would catch.
    /// </remarks>
    [Fact]
    public void AParagraphSpacingIsNotReadAsTracking()
    {
        PageParagraph paragraph = Paragraph(ParagraphSpacingDocument);

        paragraph.Tracking.ShouldBe(Length.Zero);
        paragraph.Runs.Select(run => run.Tracking).ShouldAllBe(tracking => tracking == Length.Zero);
    }

    /// <summary>The corpus fixture is the width LibreOffice makes it.</summary>
    /// <remarks>
    /// <c>character-spacing.docx</c> was rendered by LibreOffice 24.2.7.2, which breaks its tracked
    /// paragraph onto three lines and its untracked twin onto two. Untracked, both come out at two.
    /// </remarks>
    [Fact]
    public void TheCorpusFixtureBreaksWhereLibreOfficeBreaksIt()
    {
        using IDocument document = new WordProcessingReader()
            .Read(DocumentSource.FromFile(Corpus.Require("character-spacing.docx")));
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        List<PageParagraph> paragraphs = [.. pages.Blocks.OfType<PageParagraph>()];
        PageParagraph tracked = paragraphs.First(p => p.Text.StartsWith("Tracked:", StringComparison.Ordinal));
        PageParagraph plain = paragraphs.First(p => p.Text.StartsWith("Plain:", StringComparison.Ordinal));

        Lines(pages, tracked).ShouldBe(3);
        Lines(pages, plain).ShouldBe(2);
    }

    private static int Lines(WordProcessingPages pages, PageParagraph paragraph)
        => pages.Pages.SelectMany(page => page.Lines)
            .Count(line => ReferenceEquals(pages.Blocks[line.ParagraphIndex], paragraph));

    private static Length Width(string sentence, int? spacing)
    {
        string runProperties = spacing is { } twips
            ? $"<w:rPr><w:spacing w:val=\"{twips}\"/></w:rPr>"
            : string.Empty;

        PageParagraph paragraph = Paragraph($"""
            <w:p>
              <w:r>{runProperties}<w:t xml:space="preserve">{sentence}</w:t></w:r>
            </w:p>
            """);

        return paragraph.Measure().WidthBetween(0, paragraph.Text.Length);
    }

    private const string MixedDocument = """
        <w:p>
          <w:r><w:t xml:space="preserve">plain </w:t></w:r>
          <w:r><w:rPr><w:spacing w:val="100"/></w:rPr><w:t>tracked</w:t></w:r>
        </w:p>
        """;

    private const string ParagraphSpacingDocument = """
        <w:p>
          <w:pPr><w:spacing w:before="240" w:after="240"/></w:pPr>
          <w:r><w:t>a paragraph whose spacing is between paragraphs</w:t></w:r>
        </w:p>
        """;

    private static PageParagraph Paragraph(string body)
    {
        using IDocument document = Open(body);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        return pages.Blocks.OfType<PageParagraph>().First(block => block.Text.Length > 0);
    }

    private static IDocument Open(string body)
    {
        MemoryStream package = BuildPackage(body);
        using DocumentSource source = DocumentSource.FromStream(package, "tracking.docx");
        return new WordProcessingReader().Read(source);
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
                {body}
                <w:sectPr><w:pgSz w:w="11906" w:h="16838"/></w:sectPr>
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
