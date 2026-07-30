using Paperless.Core;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// Tests the ODP reader against corpus decks written by LibreOffice itself.
/// </summary>
public class OdpReaderTests
{
    private static IDocument Open(string name)
        => new PresentationReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

    private static List<ContentSection> Sections(IDocument document, SectionKind kind)
        => [.. document.Content.Children.OfType<ContentSection>().Where(s => s.Kind == kind)];

    [Theory]
    [InlineData("slides-odp.odp", DocumentFormat.Odp)]
    [InlineData("slides-otp.otp", DocumentFormat.Otp)]
    [InlineData("slides-fodp.fodp", DocumentFormat.Fodp)]
    public void EveryOdfPresentationVariantReadsThroughTheSamePath(string name, DocumentFormat expected)
    {
        using IDocument document = Open(name);

        document.Format.ShouldBe(expected);
        document.Family.ShouldBe(DocumentFamily.Presentation);
        document.Content.GetText().ShouldContain("Paperless: slide one");
    }

    [Fact]
    public void EachSlideIsItsOwnSectionNamedAsTheFileNamesIt()
    {
        using IDocument document = Open("slides-features.odp");
        List<ContentSection> slides = Sections(document, SectionKind.Slide);

        slides.Select(s => s.Name).ShouldBe(["Introduction", "Shapes", "Skipped"]);
        slides.Select(s => s.Index).ShouldBe([0, 1, 2]);
    }

    [Fact]
    public void AHiddenSlideIsFlaggedButStillExtracted()
    {
        using IDocument document = Open("slides-features.odp");
        ContentSection skipped = Sections(document, SectionKind.Slide).Single(s => s.Name == "Skipped");

        // The flag lives on the slide's drawing-page style, so this also exercises resolving
        // a presentation property through the style chain rather than off the element.
        skipped.IsHidden.ShouldBeTrue();
        skipped.GetText().ShouldContain("This slide is hidden");

        Sections(document, SectionKind.Slide).Single(s => s.Name == "Shapes").IsHidden.ShouldBeFalse();
    }

    [Fact]
    public void SpeakerNotesAreASeparateSectionNextToTheirSlide()
    {
        using IDocument document = Open("slides-features.odp");
        List<ContentSection> notes = Sections(document, SectionKind.SlideNotes);

        // Only the slide that has notes gets a notes section: LibreOffice writes a notes page
        // for every slide, carrying a thumbnail and an empty placeholder, and reporting those
        // as content would invent text for every slide in every deck.
        notes.Count.ShouldBe(1);
        notes[0].Index.ShouldBe(0);
        notes[0].GetText().Trim().ShouldBe("Speaker notes for the first slide.");
    }

    [Fact]
    public void OutlineTextKeepsItsListStructureAndMarkers()
    {
        using IDocument document = Open("slides-features.odp");
        List<ContentParagraph> items =
            [.. Descendants(Sections(document, SectionKind.Slide)[0])
                .OfType<ContentParagraph>()
                .Where(p => p.ListLevel is not null)];

        items.Count.ShouldBe(3);
        // Impress writes its bullet as a symbol-font code point in a Private Use Area, which
        // means nothing outside that font; LibreOffice's own HTML export substitutes a bullet
        // here too, so agreeing with it is the faithful answer rather than a departure.
        items.ShouldAllBe(p => p.ListMarker == "•");

        // The marker is part of the paragraph's text because it is text a reader sees, so the
        // rendered line carries it; the runs beneath do not.
        items[2].GetText().ShouldBe("• A nested point\n");
        items[2].Children.OfType<ContentRun>().Single().Text.ShouldBe("A nested point");
        items[2].ListLevel.ShouldBe(1);
    }

    [Fact]
    public void ShapeTextIsReadFromShapesTheReferenceFilterDrops()
    {
        using IDocument document = Open("slides-features.odp");
        string slide = Sections(document, SectionKind.Slide).Single(s => s.Name == "Shapes").GetText();

        // Grouped shapes and shapes with text bodies are walked through; LibreOffice's HTML
        // export loses both, so extracting more here is correct rather than a defect.
        slide.ShouldContain("Text in a custom shape");
        slide.ShouldContain("Text in a rectangle");
        slide.ShouldContain("A plain text box with an emphasised word.");
    }

    [Fact]
    public void ShapeStyleFormattingReachesTheTextInsideIt()
    {
        using IDocument document = Open("slides-features.odp");

        // On a slide nearly all character formatting comes from the shape's own style rather
        // than from a span, so the shape's style has to join the cascade or the text comes
        // back unformatted.
        Descendants(Sections(document, SectionKind.Slide).Single(s => s.Name == "Shapes"))
            .OfType<ContentRun>()
            .ShouldContain(r => r.Text.Contains("custom shape", StringComparison.Ordinal)
                                && r.Emphasis.HasFlag(RunEmphasis.Italic));
    }

    [Fact]
    public void SlideContentIsReadInDocumentOrder()
    {
        using IDocument document = Open("slides-features.odp");
        string text = Sections(document, SectionKind.Slide)[0].GetText();

        // Document order is what the file states and what the authoring application used.
        // Inferring a reading order from coordinates would guess, and guess differently for
        // every layout.
        text.IndexOf("Feature deck title", StringComparison.Ordinal)
            .ShouldBeLessThan(text.IndexOf("First outline point", StringComparison.Ordinal));
    }

    [Fact]
    public void MetadataIsReadWithoutInventingStatisticsImpressDoesNotRecord()
    {
        using IDocument document = Open("slides-features.odp");

        document.Metadata.Title.ShouldBe("Paperless feature deck");
        document.Metadata.Author.ShouldBe("Ada Lovelace");

        // Impress recomputes meta:document-statistic on save and writes only an object count,
        // so a deck legitimately records no slide count at all. Counting the slides ourselves
        // and reporting the result as metadata would misrepresent the file: DocumentStatistics
        // is what the authoring application recorded, not what Paperless can derive.
        document.Metadata.Statistics!.SlideCount.ShouldBeNull();
        document.Metadata.Statistics!.PageCount.ShouldBeNull();

        // The slides themselves are of course counted; that is content, not metadata.
        Sections(document, SectionKind.Slide).Count.ShouldBe(3);
    }

    [Fact]
    public void AFormatWithNoReaderYetIsReportedAsUnsupported()
    {
        UnsupportedFormatException unimplemented = Should.Throw<UnsupportedFormatException>(
            () => Open("slides-pptx.pptx"));
        unimplemented.Format.ShouldBe(DocumentFormat.Pptx);
        unimplemented.Message.ShouldContain("not implemented yet");
    }

    private static IEnumerable<ContentNode> Descendants(ContentNode root)
    {
        foreach (ContentNode child in root.Children)
        {
            yield return child;
            foreach (ContentNode descendant in Descendants(child)) yield return descendant;
        }
    }
}
