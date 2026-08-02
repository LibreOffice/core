using Paperless.Core;
using Paperless.Core.Documents;
using Paperless.Presentations.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// Checks what a binary PowerPoint slide inherits from its master, and what it must not.
/// </summary>
/// <remarks>
/// <para>
/// <c>ppt-master-shapes.ppt</c> is <c>ppt-master-shapes.fodp</c> converted by LibreOffice. Its
/// master carries a title prompt, a body prompt and the date, footer and slide-number
/// placeholders; its two slides carry one text box each and nothing else. LibreOffice's own PDF
/// of it reads "First slide of its own / 14 March 2011 / Confidential draft / 1", then the same
/// for the second page — so all three running placeholders come from the master, and neither
/// prompt appears anywhere.
/// </para>
/// <para>
/// That makes the deck the smallest thing that separates the three rules at once: draw the
/// master's shapes, hold back its prompts, and resolve the field markers against the page.
/// The corpus deck used before this — <c>ppt-features.ppt</c> — has a master too, but every
/// shape on it is a prompt, so it can only prove the negative.
/// </para>
/// </remarks>
public class PptMasterShapeTests
{
    private static SlidePages Layout(string name)
    {
        using IDocument document =
            new PresentationReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

        return (SlidePages)((IPaginatedDocument)document).Layout();
    }

    private static IEnumerable<string> TextOf(LaidOutSlide slide)
        => slide.Shapes
            .Where(shape => shape.Text is not null)
            .SelectMany(shape => shape.Text!.Runs)
            .Select(run => run.Run.Text);

    [Fact]
    public void AMastersRunningPlaceholdersReachEverySlideUnderIt()
    {
        SlidePages pages = Layout("ppt-master-shapes.ppt");
        pages.Slides.Count.ShouldBe(2);

        foreach (LaidOutSlide slide in pages.Slides)
        {
            // The slide's drawing holds one text box. The date, the footer and the slide number
            // are three more shapes, and all three are on the master.
            slide.Shapes.Count(shape => shape.Text is not null).ShouldBe(4);

            List<string> text = [.. TextOf(slide)];
            text.ShouldContain("14 March 2011");
            text.ShouldContain("Confidential draft");
        }
    }

    [Fact]
    public void TheSlideNumberFieldBecomesThePagesOwnNumber()
    {
        SlidePages pages = Layout("ppt-master-shapes.ppt");

        // One shape on the master, one asterisk in its text, a different number on each page.
        TextOf(pages.Slides[0]).ShouldContain("1");
        TextOf(pages.Slides[1]).ShouldContain("2");
        TextOf(pages.Slides[0]).ShouldNotContain("*");
        TextOf(pages.Slides[1]).ShouldNotContain("*");
    }

    [Fact]
    public void ASlideStillDrawsItsOwnShapesOverTheMastersOwn()
    {
        SlidePages pages = Layout("ppt-master-shapes.ppt");

        TextOf(pages.Slides[0]).ShouldContain("First slide of its own");
        TextOf(pages.Slides[1]).ShouldContain("Second slide of its own");
    }

    [Theory]
    [InlineData("ppt-master-shapes.ppt")]
    [InlineData("ppt-features.ppt")]
    public void AMastersOwnPromptsAreNeverDrawnOnASlide(string name)
    {
        foreach (LaidOutSlide slide in Layout(name).Slides)
        {
            foreach (string text in TextOf(slide))
            {
                text.ShouldNotContain("Click to edit", Case.Insensitive);
            }
        }
    }
}
