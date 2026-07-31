using Paperless.Core.Documents;
using Paperless.TestKit;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Checks that a note's <em>position</em> is read apart from its class.
/// </summary>
/// <remarks>
/// An endnote is not always at the end of the document, and the exception is not exotic: LibreOffice's own
/// WW8 export writes the DOP's <c>epc</c> as zero, "collect at the end of the section". So a document
/// round-tripped through DOC has its endnotes in the page-bottom note area instead of on pages of their own —
/// which means a reader that placed notes by class rather than by position gets the same document laid out two
/// different ways depending on which format it was saved in.
/// </remarks>
public sealed class NotePlacementTests
{
    [Theory]
    [InlineData("endnotes.fodt", NotePlacement.DocumentEnd)]
    [InlineData("endnotes.odt", NotePlacement.DocumentEnd)]
    [InlineData("endnotes.docx", NotePlacement.DocumentEnd)]
    [InlineData("endnotes.rtf", NotePlacement.DocumentEnd)]
    // The one that differs, and it is LibreOffice's own export that makes it differ.
    [InlineData("endnotes.doc", NotePlacement.SectionEnd)]
    public void AnEndnoteCollectsWhereItsDocumentSays(string fileName, NotePlacement expected)
    {
        List<PageParagraph> citing = [.. Paragraphs(fileName).Where(p => p.Notes.Count > 0)];

        citing.Count.ShouldBe(2, $"{fileName}: expected two paragraphs to cite a note");

        foreach (PageParagraph paragraph in citing)
        {
            PageNote note = paragraph.Notes.ShouldHaveSingleItem();

            note.IsEndnote.ShouldBeTrue($"{fileName}: the note is an endnote whatever its position");
            note.Placement.ShouldBe(expected, $"{fileName}: the endnote's position");
        }
    }

    [Theory]
    [InlineData("footnotes.fodt")]
    [InlineData("footnotes.odt")]
    [InlineData("footnotes.docx")]
    [InlineData("footnotes.doc")]
    [InlineData("footnotes.rtf")]
    public void AFootnoteAlwaysCollectsAtTheFootOfItsPage(string fileName)
    {
        foreach (PageParagraph paragraph in Paragraphs(fileName).Where(p => p.Notes.Count > 0))
        {
            paragraph.Notes.ShouldHaveSingleItem().Placement.ShouldBe(
                NotePlacement.PageBottom, $"{fileName}: a footnote's position");
        }
    }

    // ------------------------------------------------------------------------- the machinery

    private static List<PageParagraph> Paragraphs(string fileName)
    {
        using FileStream stream = File.OpenRead(Corpus.Require(fileName));
        using DocumentSource source = DocumentSource.FromStream(stream, Path.GetFileName(fileName));
        using IDocument document = new WordProcessingReader().Read(source);

        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();
        return [.. pages.Blocks.OfType<PageParagraph>()];
    }
}
