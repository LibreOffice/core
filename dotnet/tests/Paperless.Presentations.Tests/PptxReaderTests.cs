using Paperless.Core;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;
using Paperless.Presentations.Ooxml;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// Tests the PPTX reader against corpus decks written by LibreOffice itself.
/// </summary>
/// <remarks>
/// <c>deck-features.pptx</c> is <c>slides-features.odp</c> plus a table slide, converted by
/// LibreOffice 24.2.7.2 — so the committed file is one LibreOffice itself wrote, and the two
/// decks can be asserted to extract to the same shape through two entirely different
/// vocabularies.
/// </remarks>
public class PptxReaderTests
{
    private static IDocument Open(string name)
        => new PresentationReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

    private static List<ContentSection> Sections(IDocument document, SectionKind kind)
        => [.. document.Content.Children.OfType<ContentSection>().Where(s => s.Kind == kind)];

    private static IEnumerable<ContentNode> Descendants(ContentNode root)
    {
        foreach (ContentNode child in root.Children)
        {
            yield return child;
            foreach (ContentNode descendant in Descendants(child)) yield return descendant;
        }
    }

    [Theory]
    [InlineData("slides-pptx.pptx", DocumentFormat.Pptx)]
    [InlineData("deck-features.pptx", DocumentFormat.Pptx)]
    public void APptxReadsAsAPresentation(string name, DocumentFormat expected)
    {
        using IDocument document = Open(name);

        document.Format.ShouldBe(expected);
        document.Family.ShouldBe(DocumentFamily.Presentation);
        document.Diagnostics.ShouldBeEmpty();
    }

    [Fact]
    public void SlidesComeOutInPresentationOrderRatherThanPartOrder()
    {
        using IDocument document = Open("deck-features.pptx");
        List<ContentSection> slides = Sections(document, SectionKind.Slide);

        // The order is p:sldIdLst's order of r:ids. It coincides with slide1..slide4 here, so
        // what this really pins down is that all four are found and none is duplicated by the
        // notes walk.
        slides.Select(s => s.Index).ShouldBe([0, 1, 2, 3]);
        slides.Select(s => s.GetText().Trim().Split('\n')[0]).ShouldBe(
            ["Feature deck title", "Shapes and groups", "A table on a slide", "This slide is hidden"]);
    }

    [Fact]
    public void ASlideHasNoNameUnlessTheDeckRecordsOne()
    {
        using IDocument document = Open("deck-features.pptx");

        // p:cSld/@name is optional and LibreOffice's PPTX export writes none, so a deck
        // round-tripped through Impress genuinely has unnamed slides. Manufacturing "Slide 1"
        // would claim the file says something it does not — the same distinction
        // DocumentMetadata draws between "absent" and "empty".
        Sections(document, SectionKind.Slide).ShouldAllBe(s => s.Name == null);
    }

    [Fact]
    public void AHiddenSlideIsFlaggedButStillExtracted()
    {
        using IDocument document = Open("deck-features.pptx");
        List<ContentSection> slides = Sections(document, SectionKind.Slide);

        // p:sld/@show="0". Absent means shown, which is why the flag cannot be read as a
        // plain presence test — that would hide every slide in every deck.
        slides[3].IsHidden.ShouldBeTrue();
        slides[3].GetText().ShouldContain("This slide is hidden");
        slides.Take(3).ShouldAllBe(s => !s.IsHidden);
    }

    [Fact]
    public void SpeakerNotesAreASeparateSectionNextToTheirSlide()
    {
        using IDocument document = Open("deck-features.pptx");
        List<ContentSection> notes = Sections(document, SectionKind.SlideNotes);

        // Only the slide that has notes gets a notes section. PowerPoint and Impress both write
        // a notes part for slides without notes, carrying a slide-image placeholder and an empty
        // body, and reporting those would attach an empty section to every slide in every deck.
        notes.Count.ShouldBe(1);
        notes[0].Index.ShouldBe(0);
        notes[0].GetText().Trim().ShouldBe("Speaker notes for the first slide.");

        // Adjacency: the notes follow their own slide rather than collecting at the end.
        List<ContentNode> children = [.. document.Content.Children];
        children.IndexOf(notes[0]).ShouldBe(children.IndexOf(Sections(document, SectionKind.Slide)[0]) + 1);
    }

    [Fact]
    public void ANotesPageDoesNotReportItsSlideThumbnailAsAnImage()
    {
        using IDocument document = Open("deck-features.pptx");

        // <p:ph type="sldImg"/> is a live preview of the slide, not an embedded picture, and the
        // slide it previews has already been extracted next to it.
        Sections(document, SectionKind.SlideNotes)
            .SelectMany(Descendants).OfType<ContentImage>().ShouldBeEmpty();
    }

    [Fact]
    public void GroupedShapesAndShapeTextBodiesAreReadInDocumentOrder()
    {
        using IDocument document = Open("deck-features.pptx");
        string slide = Sections(document, SectionKind.Slide)[1].GetText();

        // LibreOffice's HTML export loses the group's two shapes and the plain text box, so
        // finding them is a deliberate improvement over the reference rather than a difference
        // to reconcile — measured on this file: the reference emits only "Shapes and groups".
        slide.ShouldContain("Text in a custom shape");
        slide.ShouldContain("Text in a rectangle");
        slide.ShouldContain("A plain text box with an emphasised word.");

        // Document order, not position order: the group's first child precedes its second.
        slide.IndexOf("Text in a custom shape", StringComparison.Ordinal)
            .ShouldBeLessThan(slide.IndexOf("Text in a rectangle", StringComparison.Ordinal));
    }

    [Fact]
    public void ATableInsideAGraphicFrameBecomesATableWithTabSeparatedRows()
    {
        using IDocument document = Open("deck-features.pptx");
        ContentTable table = Descendants(Sections(document, SectionKind.Slide)[2])
            .OfType<ContentTable>().Single();

        table.ColumnCount.ShouldBe(2);
        table.Children.Count.ShouldBe(3);
        table.GetText().ShouldBe("Format\tExtracted\nPPTX\tYes\nODP\tYes\n");
    }

    [Fact]
    public void OutlineTextKeepsItsLevelAndItsBulletMarker()
    {
        using IDocument document = Open("deck-features.pptx");
        List<ContentParagraph> items =
            [.. Descendants(Sections(document, SectionKind.Slide)[0])
                .OfType<ContentParagraph>()
                .Where(p => p.ListLevel is not null)];

        items.Count.ShouldBe(3);

        // LibreOffice's exporter writes the bullet as a Symbol-font code point in a Private Use
        // Area (U+F0B7), which means nothing outside that font — its own HTML export passes it
        // straight through, which is why the reference shows "First outline point" and
        // this shows a bullet. The ODP path makes the same substitution.
        items.ShouldAllBe(p => p.ListMarker == "•");
        items.Select(p => p.ListLevel).ShouldBe([0, 0, 1]);
        items[2].GetText().ShouldBe("• A nested point\n");
    }

    [Fact]
    public void TheTitleAndTheOutlineAreNotConfusedWithEachOther()
    {
        using IDocument document = Open("deck-features.pptx");
        List<ContentParagraph> paragraphs =
            [.. Descendants(Sections(document, SectionKind.Slide)[0]).OfType<ContentParagraph>()];

        // The title placeholder's a:buNone must win over the body style that bullets, or every
        // title in the deck comes out bulleted. It is the first paragraph and the only unmarked
        // one on this slide.
        paragraphs[0].ListMarker.ShouldBeNull();
        paragraphs[0].GetText().ShouldBe("Feature deck title\n");
    }

    [Fact]
    public void TheMastersPromptTextNeverReachesASlide()
    {
        using IDocument document = Open("deck-features.pptx");
        string text = document.Content.GetText();

        // The master's placeholders carry "Click to edit the title text format" and seven
        // outline levels of sample text. A reader that walked the master — or that filled an
        // empty slide placeholder from its layout — would put that on every slide of every deck.
        // LibreOffice never copies it either: Shape::applyShapeReference resets the text body,
        // and pptgraphicshapecontext.cxx passes bUseText=false for every placeholder type.
        text.ShouldNotContain("Click to edit");
        text.ShouldNotContain("Outline Level");
    }

    [Fact]
    public void RunFormattingSurvivesIntoTheExtractedRuns()
    {
        using IDocument document = Open("deck-features.pptx");

        // On a slide almost all character formatting is stated on the run itself, so a:rPr is
        // where it has to be read from. strike="noStrike" and u="none" are written on every run
        // LibreOffice exports, so testing for the attribute's presence rather than its value
        // would mark the whole deck struck through and underlined.
        List<ContentRun> runs = [.. Descendants(document.Content).OfType<ContentRun>()];
        runs.ShouldContain(r => r.Text.Contains("custom shape", StringComparison.Ordinal)
                                && r.Emphasis.HasFlag(RunEmphasis.Italic));
        runs.ShouldAllBe(r => !r.Emphasis.HasFlag(RunEmphasis.Strikethrough));
        runs.ShouldAllBe(r => !r.Emphasis.HasFlag(RunEmphasis.Underline));
        runs.ShouldContain(r => r.Language == "en-GB");
    }

    [Fact]
    public void TheSlideSizeIsReadFromTheDeckRatherThanAssumed()
    {
        using PptxDocument document = (PptxDocument)Open("deck-features.pptx");

        // p:sldSz is in EMUs already — the one measurement in an office format that needs no
        // conversion. 10080625 x 5670550 is Impress's 28cm x 15.75cm default.
        document.SlideSize.Width.Emu.ShouldBe(10080625);
        document.SlideSize.Height.Emu.ShouldBe(5670550);
    }

    [Fact]
    public void MetadataIsReadWithoutInventingStatisticsTheDeckDoesNotRecord()
    {
        using IDocument document = Open("deck-features.pptx");

        document.Metadata.Title.ShouldBe("Paperless feature deck");
        document.Metadata.Author.ShouldBe("Ada Lovelace");
        document.Metadata.Language.ShouldBe("en-US");

        // A deck records its slide count in app.xml as Slides, and LibreOffice's PPTX export
        // writes no such element at all — the same gap the ODP path found in meta.xml. Counting
        // the slides ourselves and reporting the result as metadata would misrepresent the file:
        // DocumentStatistics is what the authoring application recorded, not what Paperless can
        // derive.
        document.Metadata.Statistics!.SlideCount.ShouldBeNull();
        document.Metadata.Statistics!.PageCount.ShouldBeNull();

        // The slides themselves are of course counted; that is content, not metadata.
        Sections(document, SectionKind.Slide).Count.ShouldBe(4);
    }

    [Fact]
    public void ThePptxAndOdpPathsAgreeOnEverythingBothFilesContain()
    {
        using IDocument pptx = Open("deck-features.pptx");
        using IDocument odp = Open("slides-features.odp");

        // The PPTX is the ODP converted, plus one slide. Two importers, two vocabularies, one
        // content tree: what the ODP says the PPTX must say too, in the same order.
        string[] odpLines = [.. odp.Content.GetText()
            .Split('\n', StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries)];
        string[] pptxLines = [.. pptx.Content.GetText()
            .Split('\n', StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries)];

        int at = 0;
        foreach (string line in odpLines)
        {
            int found = Array.IndexOf(pptxLines, line, at);
            found.ShouldBeGreaterThanOrEqualTo(0, $"'{line}' is in the ODP but not, in order, in the PPTX");
            at = found + 1;
        }
    }
}
