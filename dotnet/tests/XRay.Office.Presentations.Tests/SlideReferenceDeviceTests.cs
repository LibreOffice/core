using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Presentations.Layout;
using XRay.Office.Text.Layout;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// That Impress's 600 dpi reference device reaches a laid-out slide, and where it does not.
/// </summary>
/// <remarks>
/// <para>
/// The arithmetic itself is tested in <c>XRay.Office.Text.Tests.ApplicationGridTests</c> against 507
/// (face, size) pairs read out of LibreOffice's own PDFs. What is tested here is the wiring — that
/// a body laid out through <c>SlideTextLayout</c> actually gets the device — and it is asserted on
/// baselines rather than on a resolved <c>LineMetrics</c>, because a constructed metric would agree
/// with whatever it was constructed with.
/// </para>
/// <para>
/// <b><see cref="APptxBodyNeverReachesTheDeviceAtAll"/> is the more surprising half and is here to
/// stop the first being over-generalised.</b> A PPTX or PPT body sets
/// <c>FontIndependentLineSpacing</c>, which sends it to
/// <c>ImplCalculateFontIndependentLineSpacing</c> — twelve tenths of the em, no metric read at all —
/// so the device changes nothing for its paragraphs. Of the 163 presentations in the sample corpus
/// <b>none</b> is an ODP: 112 are pptx and 51 are ppt.
/// </para>
/// <para>
/// <b>Which is not the same as the device being worth nothing to a deck, and the difference was
/// measured rather than assumed.</b> Rendering the whole slides track with each of the two call
/// sites reverted on its own: the paragraph path (<c>FaceHeight</c>, chart labels here) reaches
/// <b>14 of 163</b> documents and the symbol bullet's own baseline (<c>EmitMarker</c>) reaches
/// <b>150</b>. A character bullet is drawn by EditEngine whichever line-spacing rule the paragraph
/// beside it uses, so it goes through the device on every deck that has one.
/// </para>
/// </remarks>
public class SlideReferenceDeviceTests
{
    private static DocRect Area =>
        new(Length.Zero, Length.Zero, Length.FromPoints(500), Length.FromPoints(400));

    private static SlideTextBody Body(bool fontIndependent, double points)
    {
        const string line = "Hxy";

        return new SlideTextBody
        {
            FontIndependentLineSpacing = fontIndependent,
            Insets = new Margins(Length.Zero, Length.Zero, Length.Zero, Length.Zero),
            Paragraphs =
            [
                .. Enumerable.Range(0, 4).Select(_ => new SlideParagraph(
                    line,
                    [
                        new SlideTextRun(
                            0, line.Length, "Liberation Sans", Length.FromPoints(points), 400,
                            false, Colour.Black),
                    ],
                    LineSpacing: LineSpacingRule.SingleSpaced)),
            ],
        };
    }

    private static List<long> Baselines(SlideTextBody body)
        => [.. SlideTextLayout.Place(body, Area, new SlideFonts())
            .Select(placed => placed.Run.Origin.Y.Mm100)
            .Distinct()
            .Order()];

    [Theory]
    // LibreOffice's own PDF, `probes/refdev-01/probe-impress.py`: a six-line ODF text box at these
    // sizes puts consecutive baselines exactly this far apart, in whole 1/100 mm.
    [InlineData(10.0, 394)]
    [InlineData(13.5, 530)]
    [InlineData(18.0, 711)]
    [InlineData(20.5, 809)]
    [InlineData(24.0, 944)]
    public void AnOdfBodysBaselinesLandWhereLibreOfficePutsThem(double points, long pitch)
    {
        List<long> baselines = Baselines(Body(fontIndependent: false, points));

        baselines.Count.ShouldBe(4);
        for (int i = 1; i < baselines.Count; i++)
        {
            (baselines[i] - baselines[i - 1]).ShouldBe(pitch);
        }
    }

    [Fact]
    public void ExactScalingIsNotWhatLibreOfficeDraws()
    {
        // 10 pt Liberation Sans is 1854 + 434 units over a 2048-unit em; scaled exactly on a
        // 353-unit em that is 320 + 75 = 395, and LibreOffice draws 394. The device sets the font at
        // 83 whole pixels and rounds the ascent and descent through it before converting either
        // back, and the order is what the unit is worth.
        //
        // 10 pt rather than 18, deliberately: at 18 pt exact scaling gives 710 and the whole-twip
        // round trip `LineSpacingRule.Apply` used to do turns that into 711, which is the right
        // answer for the wrong reason. A case a broken tree passes by accident is not a test.
        List<long> baselines = Baselines(Body(fontIndependent: false, 10.0));
        (baselines[1] - baselines[0]).ShouldBe(394);

        (Math.Round(1854 * 353.0 / 2048) + Math.Round(434 * 353.0 / 2048)).ShouldBe(395);
    }

    [Fact]
    public void APptxBodyNeverReachesTheDeviceAtAll()
    {
        // `FontIndependentLineSpacing` is what every PPTX and PPT body carries, and it takes the
        // branch that never reads a face metric — twelve tenths of the em, which at 18 pt is
        // 635 × 1.2 = 762 units. So the reference device is worth nothing at all to a deck in
        // either OOXML or binary PowerPoint, and this is the control that says so: if this test
        // ever starts agreeing with the ODF one, the branch has been wired wrongly.
        List<long> independent = Baselines(Body(fontIndependent: true, 18.0));
        List<long> dependent = Baselines(Body(fontIndependent: false, 18.0));

        (independent[1] - independent[0]).ShouldBe(762);
        (dependent[1] - dependent[0]).ShouldBe(711);
    }
}
