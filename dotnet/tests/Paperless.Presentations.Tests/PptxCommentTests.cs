using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// Tests review comments on a slide, in both of the vocabularies that carry them.
/// </summary>
/// <remarks>
/// <c>comment-deck.pptx</c> and <c>annotated-slides.odp</c> are the same hand-written deck
/// converted twice by LibreOffice 24.2.7.2: two slides, the first carrying two comments by two
/// different authors, the first of which has two paragraphs. No other corpus deck has a comment
/// at all, which is why this file exists — a reader written without one is a guess, and the
/// guess this deck refutes is that a comment's author is on the comment. It is not: the comment
/// names an id, and the names are in a part of their own.
/// </remarks>
public class PptxCommentTests
{
    private static IDocument Open(string name)
        => new PresentationReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

    private static List<ContentSection> Sections(IDocument document, SectionKind kind)
        => [.. document.Content.Children.OfType<ContentSection>().Where(s => s.Kind == kind)];

    [Fact]
    public void ACommentIsItsOwnSectionCarryingTheAuthorFromTheDeckWideList()
    {
        using IDocument document = Open("comment-deck.pptx");
        List<ContentSection> comments = Sections(document, SectionKind.Comment);

        // p:cm states authorId="0"; the name lives in ppt/commentAuthors.xml, reached by a
        // relationship from the presentation part rather than by its conventional name.
        comments.Count.ShouldBe(2);
        comments[0].Name.ShouldBe("Ada Lovelace");
        comments[1].Name.ShouldBe("Grace Hopper");
        comments.Select(c => c.Index).ShouldBe([0, 1]);
    }

    [Fact]
    public void AMultiParagraphCommentKeepsItsParagraphs()
    {
        using IDocument document = Open("comment-deck.pptx");
        ContentSection comment = Sections(document, SectionKind.Comment)[0];

        // A comment has no runs and no formatting: PresentationML gives it one p:text element,
        // and a two-paragraph comment arrives as one string with a newline in it. Reading the
        // element's value and stopping there would report the two paragraphs as one.
        comment.Children.OfType<ContentParagraph>().Count().ShouldBe(2);
        comment.Children.OfType<ContentParagraph>().First().GetText().Trim()
            .ShouldBe("Tighten this heading before the review.");
        comment.Children.OfType<ContentParagraph>().Last().GetText().Trim()
            .ShouldBe("Second paragraph of the same comment.");
    }

    [Fact]
    public void ACommentFollowsTheSlideItIsOnRatherThanCollectingAtTheEnd()
    {
        using IDocument document = Open("comment-deck.pptx");
        List<ContentSection> sections = [.. document.Content.Children.OfType<ContentSection>()];

        sections.Select(s => s.Kind).ShouldBe(
            [SectionKind.Slide, SectionKind.Comment, SectionKind.Comment, SectionKind.Slide]);

        // Neither comment's text leaks into the slide's own, which is the failure mode that
        // matters to a caller indexing the deck: a reviewer's note read as slide content.
        sections[0].GetText().Trim().ShouldBe("A slide that has been commented on");
        sections[3].GetText().Trim().ShouldBe("A slide with no comment at all");
    }

    [Fact]
    public void ASlideCommentInOdfIsAlsoASectionRatherThanSlideText()
    {
        using IDocument document = Open("annotated-slides.odp");
        List<ContentSection> comments = Sections(document, SectionKind.Comment);

        // ODF's own office:annotation belongs inside a paragraph or a cell, so Impress writes a
        // page-level comment as officeooo:annotation in the OpenOffice.org extension namespace
        // (xmloff/source/draw/sdxmlexp.cxx:2647). A reader that knows only the ODF-namespaced
        // element does not lose the text — it puts it in the slide, where nothing tells it
        // apart from what the slide says.
        comments.Count.ShouldBe(2);
        comments[0].Name.ShouldBe("Ada Lovelace");
        Sections(document, SectionKind.Slide)[0].GetText().Trim()
            .ShouldBe("A slide that has been commented on");
    }

    [Fact]
    public void TheSameCommentedDeckExtractsIdenticallyThroughTheOoxmlAndOdfPaths()
    {
        using IDocument ooxml = Open("comment-deck.pptx");
        using IDocument odf = Open("annotated-slides.odp");

        // A comment's author is compared; a slide's name is not. That asymmetry is the file's
        // rather than the reader's: LibreOffice's PPTX exporter writes no p:cSld/@name, so the
        // OOXML deck genuinely does not say what its slides are called, while both formats do
        // say who wrote each comment.
        static string Normalise(IDocument document)
            => string.Join(
                "\n",
                document.Content.Children.OfType<ContentSection>()
                    .Select(s => s.Kind == SectionKind.Slide
                        ? $"{s.Kind}:{s.Index}:{s.GetText().Trim()}"
                        : $"{s.Kind}:{s.Index}:{s.Name}:{s.GetText().Trim()}"));

        Normalise(ooxml).ShouldBe(Normalise(odf));
    }

    [Fact]
    public void ADeckWithNoCommentsPartReportsNoComments()
    {
        using IDocument document = Open("deck-features.pptx");
        Sections(document, SectionKind.Comment).ShouldBeEmpty();
    }
}
