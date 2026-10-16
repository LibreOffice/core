using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A sheet's print zoom scales the finished chart; it does not make a smaller chart.
/// </summary>
/// <remarks>
/// <para>
/// A BIFF or ODF chart is an OLE object with a page of its own. Calc lays the chart out on that
/// page — <c>&lt;chart:chart svg:width&gt;</c> in the reference's own resolved view — and the
/// print zoom scales the whole object into the anchor the sheet gives it. Everything in the
/// composition that is <em>proportional</em> to the zoom cancels either way; three families are
/// not, and each of them was wrong while the chart was laid out inside the already-zoomed box:
/// </para>
/// <list type="bullet">
/// <item><description>
/// the BIFF unit grid's border gap, a flat 250 hundredths of a millimetre off each edge of the
/// <em>chart's</em> page before the rest is divided into 4000 units
/// (<c>XclChRootData::InitConversion</c>, <c>sc/source/filter/excel/xlchart.cxx</c>:1245-1251);
/// </description></item>
/// <item><description>
/// every absolute constant in <c>ChartLayout</c> — <c>AXIS2D_TICKLABELSPACING</c>'s 100,
/// <c>AXIS2D_TICKLENGTH</c>'s 150, a pie's flat 350 page margin;
/// </description></item>
/// <item><description>
/// the 96 dpi whole-pixel em a chart's text is measured on, which makes a line height a step
/// function of the size rather than a fixed fraction of the em.
/// </description></item>
/// </list>
/// <para>
/// <strong>Measured.</strong> <c>EHEST-Pre-departure-checklist-Rev.-1-06-12-2016.xls</c> is
/// fit-to-width and printed at 65%. 26.2.4.2's own <c>--convert-to fods</c> resolves the page-15
/// gauge to <c>&lt;chart:plot-area&gt;</c> 654.32 × 266.00 pt of a 744.89 pt chart page and a
/// <c>&lt;chart:coordinate-region&gt;</c> that lands on the sheet at
/// <c>65.19, 417.00 – 478.94, 577.04</c>. This tree drew <c>69.06, 419.42 – 478.06, 574.44</c>
/// and now draws <c>65.18, 417.50 – 477.73, 576.88</c>: the left edge from 3.87 pt out to 0.01,
/// the width from 4.75 pt out to 1.20, and the worst of the eight value-axis gridlines from
/// 2.95 pt to 0.50.
/// </para>
/// <para>
/// The invariant asserted here is the one that says the chart was laid out on its own page: the
/// drawing at a zoom of <em>s</em> in a box of <em>s</em>×<em>w</em> is the drawing at 100% in a
/// box of <em>w</em>, scaled by <em>s</em> about the box's corner. Nothing about the reference is
/// needed to check it, and it fails on the old model for the three reasons above.
/// </para>
/// </remarks>
public sealed class SheetChartPrintZoomTests
{
    private static ChartPlot Gauge() => new()
    {
        Background = Colour.FromRgb(0xFFFFFF),
        Legend = ChartLegendPosition.None,
        Categories = ["One", "Two", "Three", "Four"],
        Series = [new ChartSeries("Values", [10.0, 40.0, 30.0, 70.0], Colour.FromRgb(0x6699FF))],
    };

    private static DocRect PlotOf(ChartPlot plot, DocRect box, double scale)
    {
        RecordingDrawingSink sink = new();
        sink.BeginPage(new DocSize(Length.FromPoints(800), Length.FromPoints(600)));
        SheetChart.Draw(sink, plot, box, scale);
        sink.EndPage();

        // The plot area's wall is the second fill: the chart's own background is the first.
        DrawnFill wall = sink.Pages[0].FilledPaths[1];
        return wall.Bounds;
    }

    /// <summary>
    /// The same chart at 65% is the 100% chart scaled by 0.65, to a hundredth of a point.
    /// </summary>
    [Theory]
    [InlineData(0.65)]
    [InlineData(0.5)]
    [InlineData(0.4)]
    public void AZoomedChartIsTheUnzoomedChartScaled(double scale)
    {
        DocRect page = new(
            Length.FromPoints(20), Length.FromPoints(30),
            Length.FromPoints(480), Length.FromPoints(200));

        DocRect box = new(
            page.X, page.Y, page.Width * scale, page.Height * scale);

        DocRect full = PlotOf(Gauge(), page, 1.0);
        DocRect zoomed = PlotOf(Gauge(), box, scale);

        zoomed.X.Points.ShouldBe(
            box.X.Points + ((full.X - page.X).Points * scale), 0.02);
        zoomed.Y.Points.ShouldBe(
            box.Y.Points + ((full.Y - page.Y).Points * scale), 0.02);
        zoomed.Width.Points.ShouldBe(full.Width.Points * scale, 0.02);
        zoomed.Height.Points.ShouldBe(full.Height.Points * scale, 0.02);
    }

    /// <summary>
    /// A chart at 100% is unchanged: the zoomed path allocates nothing and the box is used as it
    /// stands.
    /// </summary>
    [Fact]
    public void AnUnzoomedChartTakesTheBoxItIsGiven()
    {
        DocRect page = new(
            Length.FromPoints(20), Length.FromPoints(30),
            Length.FromPoints(480), Length.FromPoints(200));

        DocRect plot = PlotOf(Gauge(), page, 1.0);

        plot.X.ShouldBeGreaterThan(page.X);
        plot.Right.ShouldBeLessThanOrEqualTo(page.Right);
        plot.Bottom.ShouldBeLessThanOrEqualTo(page.Bottom);
    }
}
