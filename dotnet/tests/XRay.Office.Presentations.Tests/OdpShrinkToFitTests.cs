using XRay.Office.Core;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.Presentations.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// Checks that an ODF shape asking for Impress's autofit gets it.
/// </summary>
/// <remarks>
/// <para>
/// <strong>The defect this pins was a spelling, not a missing feature.</strong>
/// <c>SlideAutofit</c> has been a full port of <c>SdrTextObj::autoFitTextForCompatibility</c>
/// since round 52, and the ODF reader reached none of it — because ODF states one property two
/// ways. <c>drawing::TextFitToSizeType</c> is mapped from <em>both</em> <c>draw:fit-to-size</c>
/// and <c>style:shrink-to-fit</c>, with <c>MID_FLAG_MERGE_PROPERTY</c>
/// (<c>xmloff/source/draw/sdpropls.cxx</c>:143-144), because <c>draw:fit-to-size="true"</c>
/// means <em>stretch</em> to an ODF 1.1 consumer. So LibreOffice writes an autofitted shape as
/// <c>draw:fit-to-size="false" style:shrink-to-fit="true"</c>, and a reader that consults only
/// the first attribute concludes the shape does not autofit.
/// </para>
/// <para>
/// Reach: <c>style:shrink-to-fit="true"</c> appears 3515 times in <strong>all 302</strong> of
/// the converted corpus's <c>.odp</c>, and wiring it moved the <c>.odp</c> gate from 259 of 302
/// to 283.
/// </para>
/// <para>
/// The assertions compare the two slides of <c>odp-shrink-to-fit.fodp</c> against each other
/// rather than pinning a size: which of <c>constScaleLevels</c>' twelve levels the fit lands on
/// is that table's business and <c>SlideAutofit</c>'s own tests cover it. Both slides'
/// text is read out of 26.2.4.2's PDF in the file's header.
/// </para>
/// </remarks>
public class OdpShrinkToFitTests
{
    private static SlidePages Layout()
    {
        using IDocument document = new PresentationReader().Read(
            DocumentSource.FromFile(Corpus.Require("odp-shrink-to-fit.fodp")));

        document.ShouldBeAssignableTo<IPaginatedDocument>();
        return (SlidePages)((IPaginatedDocument)document).Layout();
    }

    private static IReadOnlyList<PlacedGlyphRun> RunsOf(LaidOutSlide slide)
        => [.. slide.Shapes.Where(shape => shape.Text is not null)
                          .SelectMany(shape => shape.Text!.Runs)];

    [Fact]
    public void AShrinkToFitShapeDrawsItsTextSmallerThanTheSizeItStates()
    {
        SlidePages pages = Layout();
        pages.Count.ShouldBe(2);

        Length fitted = RunsOf(pages.Slides[0]).Max(run => run.Run.FontSize);
        Length plain = RunsOf(pages.Slides[1]).Max(run => run.Run.FontSize);

        plain.Points.ShouldBe(28, 0.01);
        fitted.ShouldBeLessThan(plain);
    }

    /// <remarks>
    /// Six paragraphs of a 6 cm box at 28 pt: the reference wraps each onto two lines and lets
    /// them overflow, and fits each onto one line when the shape autofits. Counting baselines is
    /// what separates the two — the characters are the same either way, which is why no glyph
    /// count in the gate could see this defect on a document whose shapes happen not to clip.
    /// </remarks>
    [Fact]
    public void AShrinkToFitShapeFitsWhatOverflowsWithoutIt()
    {
        SlidePages pages = Layout();

        int fitted = RunsOf(pages.Slides[0]).Select(run => run.Run.Origin.Y).Distinct().Count();
        int plain = RunsOf(pages.Slides[1]).Select(run => run.Run.Origin.Y).Distinct().Count();

        fitted.ShouldBe(6);
        plain.ShouldBe(12);
    }

    /// <remarks>
    /// The box is 4 cm — 113.39 pt — from <c>svg:y="2cm"</c>, with no padding. The fitted block
    /// ends inside it; the plain one runs past the bottom, which is what the reference draws.
    /// </remarks>
    [Fact]
    public void TheFittedBlockEndsInsideTheBoxAndThePlainOneDoesNot()
    {
        SlidePages pages = Layout();

        Length bottom = Length.FromMillimetres(60);

        RunsOf(pages.Slides[0]).Max(run => run.Run.Origin.Y).ShouldBeLessThan(bottom);
        RunsOf(pages.Slides[1]).Max(run => run.Run.Origin.Y).ShouldBeGreaterThan(bottom);
    }
}
