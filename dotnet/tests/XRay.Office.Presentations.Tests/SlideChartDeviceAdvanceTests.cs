using XRay.Office.Core.Charts;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Graphics;
using XRay.Office.Presentations.Layout;
using XRay.Office.TestKit;
using XRay.Office.Text.Fonts;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// A chart's text is measured on <c>chart2</c>'s own 96 dpi device, so its advances are the
/// device's and not the face's design metric.
/// </summary>
/// <remarks>
/// <para>
/// A chart's labels are not laid out by Impress. <c>chart2</c>'s view builds them as plain text
/// shapes on the <c>VirtualDevice</c> that <c>DrawModelWrapper</c> creates from
/// <c>Application::GetDefaultDevice()</c> with <c>MapUnit::Map100thMM</c>
/// (<c>chart2/source/view/main/DrawModelWrapper.cxx</c>:88-99), and that device is
/// <strong>96 dpi</strong> (<c>SvpSalGraphics::GetResolution</c>,
/// <c>vcl/headless/svpgdi.cxx</c>:44). An <c>OutputDevice</c> instantiates a font at a whole
/// number of device pixels, so a 10 pt label is laid out at <strong>13</strong> pixels rather
/// than 13.333 and every advance in it comes back 2.5% narrow.
/// </para>
/// <para>
/// <strong>The deck is what makes this a test of the rule rather than of a number.</strong>
/// <c>chart-face-theme-minor.pptx</c> sets its theme's minor Latin face to Liberation Mono — so
/// every glyph has one design advance, 1229/2048 em, and no kerning pair can be confused with the
/// effect — and it states <strong>three</strong> sizes: 10 pt for the axis labels, where the
/// device rounds 13.333 down to 13; 13 pt for the title, where it rounds 17.333 down to 17; and
/// 9 pt for the axis titles, where 12.0 is already whole and the rule must apply
/// <em>nothing at all</em>. A constant fudge passes the first two and fails the third.
/// </para>
/// <para>
/// Measured against both reference binaries in
/// <c>probes/chart-text-metafile/</c>: over twelve sizes the drawn advance follows
/// <c>round(px96)/px96</c> in 24.2.7.2 and 26.2.4.2 alike, residual at most 0.003, while the same
/// string in an ordinary slide text box on the same slide of the same deck stays within 0.7% of
/// the design metric at every one of them. It is the chart path and not the deck.
/// </para>
/// </remarks>
public class SlideChartDeviceAdvanceTests
{
    /// <summary>Liberation Mono's advance for every glyph, from its <c>hmtx</c>.</summary>
    private const double MonoAdvanceEm = 1229.0 / 2048.0;

    /// <summary>
    /// A thousandth of an em, which is finer than anything this discriminates.
    /// </summary>
    /// <remarks>
    /// The quantities compared are our own layout's, so there is no instrument between the rule
    /// and the assertion; the tolerance is here only so that an exact double comparison does not
    /// fail on the last bit of an EMU division. The term under test is 2.5%.
    /// </remarks>
    private const double Tolerance = 0.001;

    private static LaidOutSlide Slide(string name)
    {
        using IDocument document =
            new PresentationReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

        return ((SlidePages)((IPaginatedDocument)document).Layout()).Slides[0];
    }

    /// <summary>Every glyph run the chart draws, whatever shape carries it.</summary>
    private static List<GlyphRun> Runs(LaidOutSlide slide)
        => [.. slide.Shapes
            .Where(shape => shape.Text is not null)
            .SelectMany(shape => shape.Text!.Runs)
            .Select(placed => placed.Run)];

    /// <summary>The mean advance of one run's glyphs, in ems of the size it states.</summary>
    private static double AdvanceEm(GlyphRun run)
        => run.Glyphs.Sum(glyph => glyph.Advance.Emu) / (double)run.Glyphs.Count / run.FontSize.Emu;

    /// <summary>The run's em in 96 dpi pixels, before the device rounds it to a whole one.</summary>
    /// <remarks>
    /// Taken from the size the run actually carries rather than from the size the file states,
    /// because the two differ by design: a slide's text sizes go through hundredths of a
    /// millimetre, so a stated 9 pt is carried as 318 of them and reads back as 9.0142 pt. The
    /// reference does the same — its own PDF sets that run in <c>9.013</c> pt — and the whole
    /// pixel count, which is what this rule turns on, is unchanged by the difference.
    /// </remarks>
    private static double PixelEm(GlyphRun run) => run.FontSize.Points * 96.0 / 72.0;

    private static GlyphRun Find(LaidOutSlide slide, string text)
    {
        List<GlyphRun> found = [.. Runs(slide).Where(run => run.Text == text)];
        found.Count.ShouldBe(1, $"one run reading {text}");
        found[0].Glyphs.Count.ShouldBeGreaterThan(1, $"more than one glyph in {text}");
        return found[0];
    }

    /// <summary>
    /// The value axis' labels take the 96 dpi device's advance, which is 2.5% short of the face's.
    /// </summary>
    [Fact]
    public void ATenPointChartLabelIsAdvancedAtThirteenPixelsRatherThanThirteenAndAThird()
    {
        LaidOutSlide slide = Slide("chart-face-theme-minor.pptx");
        GlyphRun label = Find(slide, "180");

        label.FontSize.Points.ShouldBe(10.0, 0.02, "the deck states ten point axis labels");
        Math.Round(PixelEm(label)).ShouldBe(13.0, "13 whole pixels for 13.34");

        double scale = MetricGrid.Chart.PixelEmScale(label.FontSize);
        scale.ShouldBe(Math.Round(PixelEm(label)) / PixelEm(label), Tolerance);

        AdvanceEm(label).ShouldBe(MonoAdvanceEm * scale, Tolerance);

        // And it is genuinely not the design metric: the whole point is that the two differ.
        Math.Abs(AdvanceEm(label) - MonoAdvanceEm).ShouldBeGreaterThan(
            0.01, "the device's advance and the face's own are 2.5% apart at ten point");
    }

    /// <summary>The title is 13 pt, where the device rounds 17.333 down to 17.</summary>
    /// <remarks>
    /// A second size at which the rule bites, and it bites by a different amount — 1.9% against
    /// the labels' 2.5%. A rule that scaled every chart label by one constant would pass the test
    /// above and fail this one.
    /// </remarks>
    [Fact]
    public void AThirteenPointChartTitleTakesItsOwnPixelEm()
    {
        LaidOutSlide slide = Slide("chart-face-theme-minor.pptx");
        GlyphRun title = Find(slide, "Regional revenue");

        title.FontSize.Points.ShouldBe(13.0, 0.02, "the deck states a thirteen point title");
        Math.Round(PixelEm(title)).ShouldBe(17.0, "17 whole pixels for 17.35");

        double scale = MetricGrid.Chart.PixelEmScale(title.FontSize);
        scale.ShouldBe(Math.Round(PixelEm(title)) / PixelEm(title), Tolerance);

        AdvanceEm(title).ShouldBe(MonoAdvanceEm * scale, Tolerance);

        // And it is a different scale from the labels' -- 1.9% against 2.5% -- which is what a
        // constant correction could not produce.
        Math.Abs(scale - MetricGrid.Chart.PixelEmScale(Find(slide, "180").FontSize))
            .ShouldBeGreaterThan(0.005);
    }

    /// <summary>
    /// The axis title is 9 pt, which is exactly 12 pixels — so nothing is applied.
    /// </summary>
    /// <remarks>
    /// The control, and the assertion that separates this rule from any constant correction: at a
    /// size whose pixel em is already whole the chart's advances must be the face's own, to the
    /// last EMU. The reference agrees — at 9, 12 and 18 pt its drawn advance sits on the design
    /// metric in both binaries.
    /// </remarks>
    [Fact]
    public void ANinePointAxisTitleIsAWholePixelEmAndIsLeftAlone()
    {
        LaidOutSlide slide = Slide("chart-face-theme-minor.pptx");
        GlyphRun axisTitle = Find(slide, "Quarter");

        axisTitle.FontSize.Points.ShouldBe(9.0, 0.02, "the deck states nine point axis titles");
        Math.Round(PixelEm(axisTitle)).ShouldBe(12.0, "12 whole pixels, and 12.02 before rounding");

        // Within a fifth of a percent of the face's own advance, against the 2.5% the ten-point
        // labels move by: at a size whose pixel em is already whole there is nothing to apply.
        // The residue is the hundredth-of-a-millimetre hop in the size itself and is below what
        // either reference binary can be measured to.
        AdvanceEm(axisTitle).ShouldBe(MonoAdvanceEm, MonoAdvanceEm * 0.002);
    }

    /// <summary>
    /// The width the layout reserved for a label is the width it is drawn at.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The failure this guards against is the one 24.2.7.2 actually ships: it right-aligns the
    /// value axis' labels on their <em>design</em> widths and then draws them from the device's
    /// narrower array, so <c>100</c> is reserved 18.012 pt and drawn 17.249 — measured in
    /// <c>probes/chart-text-metafile/facts.py</c>. 26.2.4.2 uses one width for both.
    /// </para>
    /// <para>
    /// Here that is checked as a geometric consequence rather than by reaching into the
    /// measurer: the axis' labels are right-aligned on one edge, so the pen of the two-glyph
    /// label must sit exactly one drawn advance to the right of the three-glyph one. If the
    /// reservation and the drawing disagreed, that gap would be the design advance while the
    /// glyphs inside each label were the device's.
    /// </para>
    /// </remarks>
    [Fact]
    public void ALabelIsAlignedOnTheWidthItIsDrawnAt()
    {
        LaidOutSlide slide = Slide("chart-face-theme-minor.pptx");
        GlyphRun three = Find(slide, "180");
        GlyphRun two = Find(slide, "80");

        double gap =
            (two.Origin.X - three.Origin.X).Emu / (double)three.FontSize.Emu;

        gap.ShouldBe(MonoAdvanceEm * MetricGrid.Chart.PixelEmScale(three.FontSize), 0.005);
    }
}
