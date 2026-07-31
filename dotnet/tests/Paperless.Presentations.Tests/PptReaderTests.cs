using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// Tests the legacy binary PowerPoint reader against corpus decks written by LibreOffice.
/// </summary>
/// <remarks>
/// The assertions deliberately mirror <c>OdpReaderTests</c>, because
/// <c>ppt-features.ppt</c> is <c>slides-features.odp</c> converted: the same deck through a
/// different vocabulary should come out as the same content tree, and a difference between the
/// two files' extractions is a defect in whichever reader disagrees.
/// </remarks>
public class PptReaderTests
{
    private static IDocument Open(string name)
        => new PresentationReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

    private static List<ContentSection> Sections(IDocument document, SectionKind kind)
        => [.. document.Content.Children.OfType<ContentSection>().Where(s => s.Kind == kind)];

    [Fact]
    public void ABinaryPresentationReadsThroughTheSameFacadeAsTheOdfOne()
    {
        using IDocument document = Open("slides-ppt.ppt");

        document.Format.ShouldBe(DocumentFormat.Ppt);
        document.Family.ShouldBe(DocumentFamily.Presentation);
        document.Content.GetText().ShouldContain("Paperless: slide one");
        document.Content.GetText().ShouldContain("Paperless: slide two");
    }

    [Fact]
    public void EachSlideIsItsOwnSectionInTheOrderTheSlideListStates()
    {
        using IDocument document = Open("ppt-features.ppt");
        List<ContentSection> slides = Sections(document, SectionKind.Slide);

        slides.Select(s => s.Index).ShouldBe([0, 1, 2]);
        slides[0].GetText().ShouldContain("Feature deck title");
        slides[1].GetText().ShouldContain("Shapes and groups");
        slides[2].GetText().ShouldContain("This slide is hidden");
    }

    [Fact]
    public void AHiddenSlideIsFlaggedButStillExtracted()
    {
        using IDocument document = Open("ppt-features.ppt");
        List<ContentSection> slides = Sections(document, SectionKind.Slide);

        // The flag is bit 2 of a word buried ten bytes into the slide-show information atom,
        // behind an advance time, a sound reference and two single-byte transition fields.
        slides[2].IsHidden.ShouldBeTrue();
        slides[0].IsHidden.ShouldBeFalse();
        slides[1].IsHidden.ShouldBeFalse();
    }

    [Fact]
    public void SpeakerNotesAreASeparateSectionNextToTheirSlide()
    {
        using IDocument document = Open("ppt-features.ppt");
        List<ContentSection> notes = Sections(document, SectionKind.SlideNotes);

        // PowerPoint writes a notes page for every slide whether or not it has notes, so only
        // the one carrying text becomes a section — the rule the ODF path also applies.
        notes.Count.ShouldBe(1);
        notes[0].Index.ShouldBe(0);
        notes[0].GetText().Trim().ShouldBe("Speaker notes for the first slide.");

        // The notes section follows its own slide rather than being collected at the end.
        List<ContentNode> children = [.. document.Content.Children];
        children.IndexOf(notes[0]).ShouldBe(1);
    }

    [Fact]
    public void GroupedShapesAreWalkedThroughInDocumentOrder()
    {
        using IDocument document = Open("ppt-features.ppt");
        string slide = Sections(document, SectionKind.Slide)[1].GetText();

        // The group's own shape record carries no text; its two children do. A reader that
        // treated the first shape container of a group as content would emit a phantom empty
        // shape, and one that did not descend would lose both of these.
        slide.ShouldContain("Text in a custom shape");
        slide.ShouldContain("Text in a rectangle");
        slide.ShouldContain("A plain text box with an emphasised word.");

        slide.IndexOf("Shapes and groups", StringComparison.Ordinal)
            .ShouldBeLessThan(slide.IndexOf("Text in a custom shape", StringComparison.Ordinal));
        slide.IndexOf("Text in a rectangle", StringComparison.Ordinal)
            .ShouldBeLessThan(slide.IndexOf("A plain text box", StringComparison.Ordinal));
    }

    [Fact]
    public void OutlineTextKeepsItsLevelsAndBulletMarkers()
    {
        using IDocument document = Open("ppt-features.ppt");
        List<ContentParagraph> items =
            [.. Descendants(Sections(document, SectionKind.Slide)[0])
                .OfType<ContentParagraph>()
                .Where(p => p.ListLevel is not null)];

        items.Count.ShouldBe(3);
        items.Select(p => p.ListLevel).ShouldBe([0, 0, 1]);

        // The stored bullet is a Wingdings code point in a Private Use Area, which means
        // nothing outside that font; the ODF path substitutes the same bullet here.
        items.ShouldAllBe(p => p.ListMarker == "•");
        items[2].GetText().ShouldBe("• A nested point\n");
    }

    [Fact]
    public void CharacterEmphasisSurvivesTheStyleTextPropRunStructure()
    {
        using IDocument document = Open("ppt-features.ppt");
        List<ContentRun> runs =
            [.. Descendants(Sections(document, SectionKind.Slide)[1]).OfType<ContentRun>()];

        // Emphasis is stated by a flags word that is only present when the run's mask asks for
        // it, so a reader that always reads it is four bytes out for every later property.
        runs.ShouldContain(r => r.Text.Contains("custom shape", StringComparison.Ordinal)
                                && r.Emphasis.HasFlag(RunEmphasis.Italic));
        runs.ShouldContain(r => r.Text.Contains("Shapes and groups", StringComparison.Ordinal)
                                && r.Emphasis.HasFlag(RunEmphasis.Bold));
        runs.ShouldContain(r => r.Text.Contains("rectangle", StringComparison.Ordinal)
                                && r.Emphasis == RunEmphasis.None);
    }

    [Fact]
    public void MetadataComesFromTheOlePropertySetsRatherThanTheRecordTree()
    {
        using IDocument document = Open("ppt-features.ppt");

        document.Metadata.Title.ShouldBe("Paperless feature deck");
        document.Metadata.Author.ShouldBe("Ada Lovelace");
    }

    [Fact]
    public void ADeckIsReadWithoutReportingAnythingWrongWithIt()
    {
        using IDocument document = Open("ppt-features.ppt");
        document.Diagnostics.ShouldBeEmpty();
    }

    [Fact]
    public void TheSameDeckExtractsIdenticallyThroughTheBinaryAndOdfPaths()
    {
        using IDocument binary = Open("ppt-features.ppt");
        using IDocument odf = Open("slides-features.odp");

        static string Normalise(IDocument document)
            => string.Join(
                "\n",
                document.Content.Children.OfType<ContentSection>()
                    .Select(s => $"{s.Kind}:{s.Index}:{s.IsHidden}:{s.GetText().Trim()}"));

        // The binary file is the ODF one converted by LibreOffice, so any difference is one of
        // the two readers disagreeing about the same deck rather than a difference of content.
        Normalise(binary).ShouldBe(Normalise(odf));
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
