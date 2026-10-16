using XRay.Office.Core;
using XRay.Office.Core.Documents;
using XRay.Office.Presentations.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// Checks what an ODF slide inherits from its master page, and what it must not.
/// </summary>
/// <remarks>
/// <para>
/// The ODF path drew <em>nothing</em> from a master page until this was written, and no test
/// could see it: the sample corpus holds no ODF presentation at all, and the only master-page
/// coverage in the suite went through the binary PowerPoint reader
/// (<see cref="PptMasterShapeTests"/>). Converting the whole corpus through 26.2.4.2 made it
/// visible at once — it is the largest single cause in the <c>.odp</c> column of the first ODF
/// gate.
/// </para>
/// <para>
/// <c>odp-master-background.fodp</c> is hand-authored and its every claim below is what
/// LibreOffice 26.2.4.2 does with it, read out of its PDF: the strapline on both slides that
/// name the master, nothing from the master on the third, the parked shape nowhere, and the
/// title prompt nowhere.
/// </para>
/// </remarks>
public class OdpMasterShapeTests
{
    private static SlidePages Layout()
    {
        using IDocument document = new PresentationReader().Read(
            DocumentSource.FromFile(Corpus.Require("odp-master-background.fodp")));

        document.ShouldBeAssignableTo<IPaginatedDocument>();
        return (SlidePages)((IPaginatedDocument)document).Layout();
    }

    private static IEnumerable<string> TextOf(LaidOutSlide slide)
        => slide.Shapes
            .Where(shape => shape.Text is not null)
            .SelectMany(shape => shape.Text!.Runs)
            .Select(run => run.Run.Text);

    [Fact]
    public void AMastersBackgroundObjectIsDrawnUnderEverySlideThatNamesIt()
    {
        SlidePages pages = Layout();
        pages.Count.ShouldBe(5);

        TextOf(pages.Slides[0]).ShouldContain("Global Widget Corporation");
        TextOf(pages.Slides[1]).ShouldContain("Global Widget Corporation");
    }

    [Fact]
    public void ASlideStillDrawsItsOwnShapesOverTheMastersOwn()
    {
        SlidePages pages = Layout();

        TextOf(pages.Slides[0]).ShouldContain("First slide of its own");
        TextOf(pages.Slides[1]).ShouldContain("Second slide of its own");
        TextOf(pages.Slides[2]).ShouldContain("Third slide of its own");
    }

    /// <remarks>
    /// <c>draw:display</c> sets two flags — <c>Visible = always | screen</c> and
    /// <c>Printable = always | printer</c> (<c>xmloff/source/draw/ximpshap.cxx</c>:840-844) — and
    /// rendering to PDF is printing, so <c>none</c> reaches no page. It is not an exotic
    /// attribute: LibreOffice's own ODP export writes a PowerPoint master's Slide Number, Footer
    /// and Date placeholders as ordinary custom shapes carrying their prompt text and
    /// <c>drawooo:display="none"</c>, so a reader that draws master shapes without reading it puts
    /// <c>&lt;#&gt; Footer Date</c> under every slide. 887 occurrences in 49 of the converted
    /// corpus's 302 <c>.odp</c>, and every one of them in the extension namespace.
    /// </remarks>
    [Fact]
    public void AShapeMarkedNotForDisplayIsNotDrawnAtAll()
    {
        foreach (LaidOutSlide slide in Layout().Slides)
        {
            slide.Shapes.ShouldNotContain(shape => shape.Name == "not printed");

            foreach (string text in TextOf(slide))
            {
                text.ShouldNotContain("drawooo", Case.Insensitive);
            }
        }
    }

    /// <remarks>
    /// <strong>A control on a rule that was implemented and withdrawn.</strong> The fourth slide's
    /// drawing-page style states <c>presentation:display-footer="false"</c> and the fifth states
    /// <c>"true"</c>; 26.2.4.2 draws both footers. The settings decide the <em>master's</em>
    /// running objects while they are drawn as a slide's background — <c>bSubContentProcessing</c>
    /// in <c>SdPage::checkVisibility</c> — not the slide's own copy of the frame. Every one of the
    /// 1630 such frames in the converted corpus states <c>false</c>, so a reader that honours the
    /// attribute here looks right on paper and loses text on 42 documents.
    /// </remarks>
    [Fact]
    public void ASlidesOwnFooterIsDrawnWhateverThePagesDisplayFooterSays()
    {
        SlidePages pages = Layout();

        TextOf(pages.Slides[3]).ShouldContain("Footer the fourth page turns off");
        TextOf(pages.Slides[4]).ShouldContain("Footer the fifth page turns on");
    }

    /// <remarks>
    /// Order is the whole of z-order here: a master page is drawn under the slide, so its shapes
    /// have to be placed before the slide's own rather than appended after them. Getting this
    /// backwards paints a master's full-bleed background picture over every slide, and no glyph
    /// or page count can see it.
    /// </remarks>
    [Fact]
    public void AMastersShapesArePlacedBeneathTheSlidesOwn()
    {
        LaidOutSlide slide = Layout().Slides[0];

        int strapline = slide.Shapes.ToList().FindIndex(shape => shape.Name == "strapline");
        int body = slide.Shapes.ToList().FindIndex(shape => shape.Name == "body one");

        strapline.ShouldBeGreaterThanOrEqualTo(0);
        body.ShouldBeGreaterThan(strapline);
    }

    /// <remarks>
    /// <c>sd/source/core/sdpage.cxx</c>:2986-2991 — <em>"presentation objects on master slide are
    /// always invisible if slide is shown"</em>. In ODF the mark is <c>presentation:class</c>, and
    /// the master's title prompt is the case that would otherwise put one sentence on every slide
    /// of every deck.
    /// </remarks>
    [Fact]
    public void AMastersOwnPresentationObjectIsNeverDrawnOnASlide()
    {
        foreach (LaidOutSlide slide in Layout().Slides)
        {
            foreach (string text in TextOf(slide))
            {
                text.ShouldNotContain("Click to edit", Case.Insensitive);
            }
        }
    }

    /// <remarks>
    /// <c>presentation:background-objects-visible</c> is a property of the <em>slide's</em>
    /// drawing-page style rather than an attribute on the page, and has to be resolved through the
    /// style chain — the same shape as <c>presentation:visibility</c>. LibreOffice keeps it as the
    /// <c>backgroundobjects</c> bit of the slide's master-page visible-layer set
    /// (<c>sd/source/ui/unoidl/unopage.cxx</c>:794-809).
    /// </remarks>
    [Fact]
    public void ASlideThatTurnsBackgroundObjectsOffInheritsNoneOfThem()
    {
        LaidOutSlide slide = Layout().Slides[2];

        TextOf(slide).ShouldNotContain("Global Widget Corporation");
        slide.Shapes.Count(shape => shape.Text is not null).ShouldBe(1);
    }

    /// <remarks>
    /// A master shape parked off the sheet is laid out where its attributes say and is culled by
    /// the media box, exactly as on the OOXML side — the reference's PDF of this deck carries no
    /// text past the page and neither does ours. Suppressing it here instead would be a rule
    /// LibreOffice does not have, and would go wrong the moment a shape merely overhangs.
    /// </remarks>
    [Fact]
    public void AMasterShapeParkedOffTheSheetIsPlacedWhereItsAttributesSay()
    {
        LaidOutSlide slide = Layout().Slides[0];

        PlacedShape parked = slide.Shapes.First(shape => shape.Name == "parked");

        // svg:y="20cm" on a 19.05 cm page: 566.93 pt down a 540 pt sheet.
        parked.Bounds.Y.Points.ShouldBe(566.93, 0.05);
        parked.Bounds.Y.ShouldBeGreaterThan(slide.Size.Height);
    }
}
