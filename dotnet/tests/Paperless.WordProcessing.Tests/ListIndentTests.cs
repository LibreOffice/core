using System.IO.Compression;
using System.Text;
using Paperless.Core.Documents;
using Paperless.Core.Units;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Which of a list level's two indents a DOCX paragraph actually takes.
/// </summary>
/// <remarks>
/// <para>
/// Writer's <c>SwTextNode::AreListLevelIndentsApplicableImpl</c>
/// (<c>sw/source/core/txtnode/ndtxt.cxx:4851</c>) has three arms and the middle one is easy to lose: an
/// indent set hard <em>on the paragraph</em> beats the list, but a numbering rule applied directly to
/// the paragraph beats the indents its <em>style chain</em> sets. Only when the numbering arrives
/// through a style does the chain get a say.
/// </para>
/// <para>
/// The middle arm is not a corner case: Word's own <c>ListParagraph</c> style states
/// <c>w:ind w:left</c> and no hanging indent, and every bulleted or numbered paragraph Word writes uses
/// it. Treating that style's indent as beating the level left those paragraphs with no hanging indent
/// at all, so the label was drawn exactly where the item's first word starts — visible in the corpus as
/// <c>1.All</c> and <c>-Stills</c> where LibreOffice extracts two words.
/// </para>
/// <para>
/// Asserted per item, because OOXML writes both margins into one <c>w:ind</c>: a paragraph that hard-sets
/// only <c>w:left</c> must still take the level's <c>w:hanging</c>.
/// </para>
/// </remarks>
public sealed class ListIndentTests
{
    /// <summary>
    /// A direct <c>w:numPr</c> beats the style's indent, which is the case Word's own output writes.
    /// </summary>
    [Fact]
    public void DirectNumberingBeatsTheStylesIndent()
    {
        PageParagraph paragraph = Paragraph("direct numbering, styled indent");

        paragraph.DeclaredFormat.StartIndent.ShouldBe(Length.FromTwips(720));
        paragraph.DeclaredFormat.FirstLineIndent.ShouldBe(Length.FromTwips(-360));
    }

    /// <summary>
    /// An indent hard-set on the paragraph beats the level — but only the item it actually states.
    /// </summary>
    [Fact]
    public void AHardSetIndentBeatsTheLevelOneItemAtATime()
    {
        PageParagraph paragraph = Paragraph("direct numbering, hard left indent");

        paragraph.DeclaredFormat.StartIndent.ShouldBe(Length.FromTwips(1440));
        paragraph.DeclaredFormat.FirstLineIndent.ShouldBe(Length.FromTwips(-360));
    }

    /// <summary>
    /// Numbering reached through a style loses to an indent the same style states — item by item.
    /// </summary>
    /// <remarks>
    /// The style states a left indent and no hanging one, so it takes the left margin from the style and
    /// the first line from the level. Answering the two together would lose whichever the style did not
    /// mention, which for Word's list styles is always the hanging indent.
    /// </remarks>
    [Fact]
    public void NumberingViaAStyleLosesOnlyTheItemThatStyleStates()
    {
        PageParagraph paragraph = Paragraph("numbering via a style that indents");

        paragraph.DeclaredFormat.StartIndent.ShouldBe(Length.FromTwips(1080));
        paragraph.DeclaredFormat.FirstLineIndent.ShouldBe(Length.FromTwips(-360));
    }

    /// <summary>
    /// Numbering reached through a style still wins when the indent is further up the chain.
    /// </summary>
    /// <remarks>
    /// The race Writer's walk runs: it stops at whichever it meets first, so an indent on a base style
    /// <em>below</em> the style carrying the numbering never gets to speak.
    /// </remarks>
    [Fact]
    public void NumberingViaAStyleBeatsAnIndentFurtherUpTheChain()
    {
        PageParagraph paragraph = Paragraph("numbering via a style whose base indents");

        paragraph.DeclaredFormat.StartIndent.ShouldBe(Length.FromTwips(720));
        paragraph.DeclaredFormat.FirstLineIndent.ShouldBe(Length.FromTwips(-360));
    }

    /// <summary>
    /// The label ends up clear of the text it labels, which is the defect this was found through.
    /// </summary>
    /// <remarks>
    /// The label hangs at the declared first line's start and the text begins a whole hanging indent
    /// later, so there is real space between them rather than the nought that glued <c>1.</c> to
    /// <c>All</c>.
    /// </remarks>
    [Fact]
    public void TheLabelIsDrawnClearOfTheItemsFirstWord()
    {
        PageParagraph paragraph = Paragraph("direct numbering, styled indent");

        paragraph.Label.ShouldNotBeNull();
        paragraph.DeclaredFormat.LineStart(isFirstLine: true).ShouldBe(Length.FromTwips(360));
        paragraph.Format.LineStart(isFirstLine: true).ShouldBe(Length.FromTwips(720));

        // The gap the defect closed: the label is 360 twips of room wide and does not fill it.
        paragraph.Label!.Width.ShouldBeLessThan(Length.FromTwips(360));
    }

    private static PageParagraph Paragraph(string text)
    {
        using MemoryStream package = BuildPackage();
        using DocumentSource source = DocumentSource.FromStream(package, "lists.docx");
        using IDocument document = new WordProcessingReader().Read(source);

        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        return pages.Paragraphs.Single(paragraph => paragraph.Text == text);
    }

    /// <summary>
    /// A DOCX with one numbering level and four ways of reaching it.
    /// </summary>
    /// <remarks>
    /// Built here rather than committed because the subject is the precedence rule, and a package small
    /// enough to read in one screen makes the rule the only thing under test.
    /// </remarks>
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
              <Override PartName="/word/numbering.xml"
                        ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml"/>
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
              <Relationship Id="rId2" Target="numbering.xml"
                            Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/numbering"/>
            </Relationships>
            """;

        const string Numbering = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:numbering xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:abstractNum w:abstractNumId="0">
                <w:lvl w:ilvl="0">
                  <w:start w:val="1"/>
                  <w:numFmt w:val="decimal"/>
                  <w:lvlText w:val="%1."/>
                  <w:pPr><w:ind w:left="720" w:hanging="360"/></w:pPr>
                </w:lvl>
              </w:abstractNum>
              <w:num w:numId="1"><w:abstractNumId w:val="0"/></w:num>
            </w:numbering>
            """;

        const string Styles = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:docDefaults>
                <w:rPrDefault><w:rPr><w:sz w:val="24"/></w:rPr></w:rPrDefault>
              </w:docDefaults>
              <w:style w:type="paragraph" w:default="1" w:styleId="Normal">
                <w:name w:val="Normal"/>
              </w:style>
              <w:style w:type="paragraph" w:styleId="ListParagraph">
                <w:name w:val="List Paragraph"/>
                <w:basedOn w:val="Normal"/>
                <w:pPr><w:ind w:left="720"/></w:pPr>
              </w:style>
              <w:style w:type="paragraph" w:styleId="IndentedNumbered">
                <w:name w:val="Indented Numbered"/>
                <w:basedOn w:val="Normal"/>
                <w:pPr>
                  <w:ind w:left="1080"/>
                  <w:numPr><w:ilvl w:val="0"/><w:numId w:val="1"/></w:numPr>
                </w:pPr>
              </w:style>
              <w:style w:type="paragraph" w:styleId="IndentedBase">
                <w:name w:val="Indented Base"/>
                <w:basedOn w:val="Normal"/>
                <w:pPr><w:ind w:left="1080"/></w:pPr>
              </w:style>
              <w:style w:type="paragraph" w:styleId="NumberedOnly">
                <w:name w:val="Numbered Only"/>
                <w:basedOn w:val="IndentedBase"/>
                <w:pPr><w:numPr><w:ilvl w:val="0"/><w:numId w:val="1"/></w:numPr></w:pPr>
              </w:style>
            </w:styles>
            """;

        const string Document = """
            <?xml version="1.0" encoding="UTF-8"?>
            <w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
              <w:body>
                <w:p>
                  <w:pPr>
                    <w:pStyle w:val="ListParagraph"/>
                    <w:numPr><w:ilvl w:val="0"/><w:numId w:val="1"/></w:numPr>
                  </w:pPr>
                  <w:r><w:t>direct numbering, styled indent</w:t></w:r>
                </w:p>
                <w:p>
                  <w:pPr>
                    <w:pStyle w:val="ListParagraph"/>
                    <w:numPr><w:ilvl w:val="0"/><w:numId w:val="1"/></w:numPr>
                    <w:ind w:left="1440"/>
                  </w:pPr>
                  <w:r><w:t>direct numbering, hard left indent</w:t></w:r>
                </w:p>
                <w:p>
                  <w:pPr><w:pStyle w:val="IndentedNumbered"/></w:pPr>
                  <w:r><w:t>numbering via a style that indents</w:t></w:r>
                </w:p>
                <w:p>
                  <w:pPr><w:pStyle w:val="NumberedOnly"/></w:pPr>
                  <w:r><w:t>numbering via a style whose base indents</w:t></w:r>
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
            Write(archive, "word/numbering.xml", Numbering);
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
