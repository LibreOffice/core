using XRay.Office.Core;
using XRay.Office.Core.Documents;
using XRay.Office.Presentations.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// Checks which of a master page's running objects — header, footer, date-time and slide
/// number — reach a slide, and what each of them says there.
/// </summary>
/// <remarks>
/// <para>
/// <see cref="OdpMasterShapeTests"/>'s rule is that a <c>presentation:class</c> frame on a master
/// is never drawn on a slide. These four are the exception, and reading the rule without the
/// exception left 23 of the converted corpus's 302 <c>.odp</c> short: a footer, a date and a
/// slide number reached only the slides that happened to carry a copy of their own.
/// <see cref="OpenDocument.OdpRunningObjects"/> carries the mechanism and the citations.
/// </para>
/// <para>
/// Every claim below is what LibreOffice 26.2.4.2 does with
/// <c>odp-master-running.fodp</c>, read out of its PDF; the file's own header records the five
/// pages line for line.
/// </para>
/// </remarks>
public class OdpMasterRunningObjectTests
{
    private static SlidePages Layout()
    {
        using IDocument document = new PresentationReader().Read(
            DocumentSource.FromFile(Corpus.Require("odp-master-running.fodp")));

        document.ShouldBeAssignableTo<IPaginatedDocument>();
        return (SlidePages)((IPaginatedDocument)document).Layout();
    }

    private static IEnumerable<string> TextOf(LaidOutSlide slide)
        => slide.Shapes
            .Where(shape => shape.Text is not null)
            .SelectMany(shape => shape.Text!.Runs)
            .Select(run => run.Run.Text);

    private static string LineOf(LaidOutSlide slide) => string.Join(" | ", TextOf(slide));

    /// <remarks>
    /// The switch is the <em>slide's</em>, not the master's: <c>SdPage::checkVisibility</c>
    /// answers a footer, header, date-time or slide-number object from the visualised page's
    /// <c>HeaderFooterSettings</c> (<c>sd/source/core/sdpage.cxx</c>:2957-2984), which ODF states
    /// as <c>presentation:display-*</c> on the page's drawing-page style.
    /// </remarks>
    [Fact]
    public void AMastersRunningObjectsReachEverySlideThatSwitchesThemOn()
    {
        SlidePages pages = Layout();
        pages.Count.ShouldBe(5);

        LineOf(pages.Slides[0]).ShouldContain("Head Office");
        LineOf(pages.Slides[0]).ShouldContain("Spring 2026");
        LineOf(pages.Slides[0]).ShouldContain("1");
    }

    [Fact]
    public void ASlideThatSwitchesThemOffInheritsNoneOfThem()
    {
        LaidOutSlide slide = Layout().Slides[1];

        LineOf(slide).ShouldNotContain("Head Office");
        LineOf(slide).ShouldNotContain("Spring 2026");

        // The strapline is a background object and is unaffected; the body is the slide's own.
        LineOf(slide).ShouldContain("Global Widget Corporation");
        LineOf(slide).ShouldContain("Second slide of its own");
    }

    /// <remarks>
    /// The three text-bearing kinds are fields: the master frame holds
    /// <c>presentation:footer</c> and the text comes from the declaration the slide names
    /// through <c>presentation:use-footer-name</c>
    /// (<c>xmloff/source/draw/ximppage.cxx</c>:301-360). One master frame therefore draws
    /// different words on different slides, which is why the declaration cannot be resolved once
    /// per master.
    /// </remarks>
    [Fact]
    public void AFieldTakesItsTextFromTheDeclarationTheSlideNames()
    {
        SlidePages pages = Layout();

        LineOf(pages.Slides[0]).ShouldContain("Head Office");
        LineOf(pages.Slides[2]).ShouldContain("Branch Office");
        LineOf(pages.Slides[2]).ShouldNotContain("Head Office");
    }

    /// <remarks>
    /// A slide naming no declaration leaves the field with nothing to say, and the other two
    /// running objects on the same master are unaffected — so an absent declaration is not an
    /// absent frame.
    /// </remarks>
    [Fact]
    public void AFieldWhoseSlideNamesNoDeclarationDrawsNothing()
    {
        LaidOutSlide slide = Layout().Slides[4];

        LineOf(slide).ShouldNotContain("Office");
        LineOf(slide).ShouldContain("Spring 2026");
        LineOf(slide).ShouldContain("5");
    }

    /// <remarks>
    /// The date-time frame here holds <em>characters</em> rather than a field, which is the other
    /// shape LibreOffice's own export writes — 18 of the converted corpus's master date-time
    /// frames against 992 carrying a field, and 11 footers against 983. Characters are drawn as
    /// they stand: measured on <c>ws_prod-…-M.017-(French)-France.odp</c>, whose footer
    /// declaration says <c>DGINT/2</c> while its master's footer frame holds the characters
    /// <c>WG M.017: …</c>, and 26.2.4.2 draws the characters.
    /// </remarks>
    [Fact]
    public void ARunningObjectHoldingCharactersDrawsThemRatherThanADeclaration()
    {
        LineOf(Layout().Slides[0]).ShouldContain("Spring 2026");
    }

    /// <remarks>
    /// <c>HeaderFooterSettings</c>'s constructor (<c>sd/source/core/sdpage.cxx</c>:3222-3230)
    /// makes the header, footer and date-time visible and the <strong>slide number not</strong>.
    /// Every one of the converted corpus's 8810 page-to-running-object pairings states its
    /// attribute explicitly, so no corpus row can witness this — the fourth page of the probe
    /// states none of the four and is the only place the defaults are visible.
    /// </remarks>
    [Fact]
    public void APageStatingNoneOfTheFourTakesImpressOwnDefaults()
    {
        LaidOutSlide slide = Layout().Slides[3];

        LineOf(slide).ShouldContain("Spring 2026");
        TextOf(slide).ShouldNotContain("4");
    }

    /// <remarks>
    /// <c>text:page-number</c> carries the placeholder LibreOffice stores against the field — the
    /// literal string <c>&lt;number&gt;</c> — and drawing that content is drawing the
    /// placeholder. The field draws the slide's own one-based number.
    /// </remarks>
    [Fact]
    public void ASlideNumberFieldDrawsTheNumberAndNotTheStoredPlaceholder()
    {
        SlidePages pages = Layout();

        foreach (LaidOutSlide slide in pages.Slides)
        {
            LineOf(slide).ShouldNotContain("number", Case.Insensitive);
        }

        TextOf(pages.Slides[0]).ShouldContain("1");
        TextOf(pages.Slides[4]).ShouldContain("5");
    }

    /// <remarks>
    /// The exception is only the four. A master's title placeholder is
    /// <c>SetNotVisibleAsMaster(true)</c> (<c>sd/source/core/sdpage.cxx</c>:305-310) and
    /// <c>ViewObjectContactOfSdrObj::isPrimitiveVisible</c>
    /// (<c>svx/source/sdr/contact/viewobjectcontactofsdrobj.cxx</c>:81-85) drops it while the
    /// master is drawn under a slide.
    /// </remarks>
    [Fact]
    public void AMastersTitleIsStillNeverDrawnOnASlide()
    {
        foreach (LaidOutSlide slide in Layout().Slides)
        {
            LineOf(slide).ShouldNotContain("Click to edit", Case.Insensitive);
        }
    }
}
