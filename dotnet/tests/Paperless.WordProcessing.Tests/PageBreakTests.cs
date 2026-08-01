using Paperless.Core.Documents;
using Paperless.TestKit;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Where a document says "start a new page here", and the two shapes that sentence takes.
/// </summary>
/// <remarks>
/// <para>
/// DOC and RTF state a page break as a property of the paragraph that <em>follows</em> it —
/// <c>sprmPFPageBreakBefore</c> and <c>\pagebb</c> — and DOCX states it as a <c>w:br w:type="page"</c> in
/// the run where the page ends. The two mean the same thing and only one of them fits the layout model
/// directly, so the OOXML form has to be deferred onto the next paragraph. LibreOffice does exactly that:
/// it turns the <c>w:br</c> back into the DOC's own U+000C and applies the deferred break as
/// <c>BreakType_PAGE_BEFORE</c> (<c>writerfilter/ooxml/Handler.cxx:246</c> and
/// <c>dmapper/DomainMapper.cxx:4379</c>).
/// </para>
/// <para>
/// Asserted across all four formats of one document, because that is what caught it: the same file as
/// <c>.doc</c> and <c>.rtf</c> paginated correctly and only the <c>.docx</c> came out one page short,
/// which rules out the shared layout engine and points at the reader.
/// </para>
/// </remarks>
public sealed class PageBreakTests
{
    /// <summary>
    /// One document, four formats, two pages.
    /// </summary>
    /// <remarks>
    /// <c>word-features</c>'s break sits at the end of the paragraph holding its text box, so the paragraph
    /// that starts "This paragraph starts a new page" is the first thing on page two. LibreOffice's own PDF
    /// of every one of these is two pages, and the sentence saying so is the first line of the second.
    /// </remarks>
    [Theory]
    [InlineData("word-features.docx")]
    [InlineData("word-features.dotx")]
    [InlineData("word-features.doc")]
    [InlineData("word-features.rtf")]
    public void AStatedPageBreakStartsANewPage(string name)
    {
        using IDocument document =
            new WordProcessingReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        pages.Pages.Count.ShouldBe(2, $"{name} states a page break");

        // The break is where the document put it rather than wherever the page happened to fill, which is
        // the difference a page count alone would not catch: the sentence after the break is the first
        // line of the second page, and it says so in its own words.
        pages.TextOf(pages.Pages[1].Lines[0])
            .ShouldStartWith("This paragraph starts a new page", Case.Sensitive);
    }
}
