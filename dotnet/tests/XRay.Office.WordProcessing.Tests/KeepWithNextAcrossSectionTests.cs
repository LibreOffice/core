using XRay.Office.Core.Documents;
using XRay.Office.TestKit;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// Keep-with-next is ignored when the paragraph after it opens a page of its own.
/// </summary>
/// <remarks>
/// <para>
/// Writer's rule, stated twice in <c>SwFlowFrame::IsKeep</c>
/// (<c>sw/source/core/layout/flowfrm.cxx</c>:345): it reads the <em>next</em> content's items and
/// drops the keep for <c>if (pPageDesc->GetPageDesc()) bKeep = false;</c> and again for a
/// <c>PageBefore</c> or <c>PageBoth</c> break. The reasoning is the same in both cases — a successor
/// that cannot share this page is not something the paragraph can be kept with, so honouring the
/// keep only empties the bottom of the page.
/// </para>
/// <para>
/// The fixture is the shape that found it: three paragraphs, the last carrying <c>w:keepNext</c>, a
/// <c>nextPage</c> section break, and then a 72 pt line that fits on no page at all. LibreOffice
/// 26.2.4.2 renders two pages, the first holding all three paragraphs.
/// </para>
/// <para>
/// The corpus case is <c>words/done-014/docx/exhibit-06---technical-architecture-template.docx</c>,
/// whose table of contents ends with an empty <c>w:keepNext</c> paragraph in front of a section
/// break. Bouncing that paragraph gave it a page of its own carrying a running head, a page number
/// and no text, and the document paginated to nine pages against the reference's eight.
/// </para>
/// </remarks>
public sealed class KeepWithNextAcrossSectionTests
{
    /// <summary>
    /// The kept paragraph stays where it is, because its successor was never going to join it.
    /// </summary>
    [Fact]
    public void AParagraphIsNotKeptWithASuccessorThatOpensItsOwnSection()
    {
        using IDocument document = new WordProcessingReader().Read(
            DocumentSource.FromFile(Corpus.Require("section-break-keep-with-next.docx")));

        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        pages.Pages.Count.ShouldBe(2);

        // The three body paragraphs; the section mark between them is discarded and not laid out, so
        // the 72 pt paragraph that opens the second section is block three.
        pages.Pages[0].Lines.Select(line => line.ParagraphIndex).ShouldBe([0, 1, 2]);
        pages.Pages[1].Lines.Select(line => line.ParagraphIndex).ShouldBe([3]);
    }
}
