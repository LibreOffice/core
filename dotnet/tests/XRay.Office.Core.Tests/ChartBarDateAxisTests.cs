using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// A bar on a date axis sits at its own date, not at its index.
/// </summary>
/// <remarks>
/// <para>
/// <c>BarChart::createShapes</c> takes the point's own x, rasterises it to the axis' resolution,
/// drops it outright when it falls outside the axis' range, and hands what is left to
/// <c>BarPositionHelper::getScaledSlotPos</c>
/// (<c>chart2/source/view/charttypes/BarChart.cxx</c>:694-705, this tree). A line series has
/// gone through the date scale here since it was written; a bar series went through its index,
/// so on <c>171128IPAP.pptx</c> page 40 this tree drew <strong>67 bars across the left
/// two-thirds of a plot where 26.2.4.2 draws 44 across the whole of it</strong>.
/// </para>
/// <para>
/// Three rules, all measured on 26.2.4.2 rather than derived —
/// <c>probes/bardate-r189</c>:
/// </para>
/// <list type="number">
/// <item><description>
/// <strong>A category is one unit of the axis' time resolution wide</strong>, not one n-th of
/// the points: <c>PlottingPositionHelper::setTimeResolution</c> sets the width to 1, and to 12
/// at year resolution because the scaling counts months.
/// </description></item>
/// <item><description>
/// <strong>The axis is linear in that unit and not in days.</strong>
/// <c>DateScaling::doScaling</c> returns <c>year × 12 + month</c> plus the fraction of the month
/// elapsed. Over 132 months the difference is under a tenth of a percent; over eleven it is 5%,
/// and 26.2.4.2 spaces <c>044_Cash_flow_forecast</c>'s twelve monthly bars at a flat 35.26 pt
/// where days would give 30.2 to 33.4.
/// </description></item>
/// <item><description>
/// <strong>A shifted axis carries one extra interval and its categories lead their own
/// date.</strong> <c>c:crossBetween="between"</c> adds a resolution unit to the maximum
/// (<c>ScaleAutomatism.cxx</c>:565-597) and the category runs from the point rather than
/// straddling it; <c>midCat</c> does neither. Solved from three renderings of one file — see
/// <see cref="ChartDateAxis.SlotStart"/>.
/// </description></item>
/// </list>
/// </remarks>
public sealed class ChartBarDateAxisTests
{
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
            => new(size * (0.5 * text.Length) * (bold ? 1.1 : 1.0), size * 1.15);
    }

    private static readonly DocRect Frame =
        new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300));

    /// <summary>1 January 2009, in the 1900 serial numbering.</summary>
    private const double Jan2009 = 39814.0;

    /// <summary>Quarterly serials from <see cref="Jan2009"/>, <paramref name="count"/> of them.</summary>
    private static double?[] Quarterly(int count)
    {
        double?[] dates = new double?[count];
        DateOnly at = new(2009, 1, 1);

        for (int i = 0; i < count; i++)
        {
            dates[i] = at.DayNumber - new DateOnly(1899, 12, 30).DayNumber;
            at = at.AddMonths(3);
        }

        return dates;
    }

    private static ChartPlot Bars(ChartDateAxis? axis, int points)
    {
        double?[] values = new double?[points];
        for (int i = 0; i < points; i++) values[i] = 1.0 + i;

        return new ChartPlot
        {
            Kind = ChartPlotKind.Bar,
            DateAxis = axis,
            Categories = new string?[points],
            Legend = ChartLegendPosition.None,
            ValueAxisVisible = false,
            CategoryAxisVisible = false,
            Series = [new ChartSeries("S", values, Colour.FromRgb(0x4472C4))],
        };
    }

    /// <summary>Every filled shape's left edge, in order.</summary>
    private static List<double> BarEdges(ChartDrawing drawing)
    {
        List<double> edges = [];
        foreach (ChartShape shape in drawing.Shapes)
        {
            if (shape.Fill is null) continue;
            edges.Add(shape.Bounds().Left.Points);
        }

        edges.Sort();
        return edges;
    }

    /// <summary>
    /// A point beyond the axis' stated maximum is not drawn at all.
    /// </summary>
    /// <remarks>
    /// The three <c>continue</c>s of <c>BarChart::createShapes</c>. Laying the points out by
    /// index instead squeezes every one of them inside, which is what drew 67 bars for a series
    /// the reference draws 44 of.
    /// </remarks>
    [Fact]
    public void APointPastTheAxisMaximumIsNotDrawn()
    {
        double?[] dates = Quarterly(12);

        // Half the points, to the quarter before the eighth.
        ChartDateAxis axis = ChartDateScale.Resolve(
            dates, statedMinimum: Jan2009, statedMaximum: dates[7]!.Value,
            statedResolution: ChartTimeUnit.Month)!;

        BarEdges(ChartLayout.Place(Bars(axis, 12), Frame, new Ruler())).Count.ShouldBe(8);
    }

    /// <summary>
    /// The bars are evenly spaced in months, which is not evenly spaced in days.
    /// </summary>
    /// <remarks>
    /// Quarters are 90, 91 or 92 days long. A day-linear axis spaces them by up to 2% apart and
    /// a month-linear one exactly evenly, which is what 26.2.4.2 draws.
    /// </remarks>
    [Fact]
    public void QuarterlyBarsAreEvenlySpacedAlthoughTheirQuartersAreNot()
    {
        double?[] dates = Quarterly(12);

        ChartDateAxis axis = ChartDateScale.Resolve(
            dates, statedMinimum: Jan2009, statedMaximum: dates[11]!.Value,
            statedResolution: ChartTimeUnit.Month)!;

        List<double> edges = BarEdges(ChartLayout.Place(Bars(axis, 12), Frame, new Ruler()));
        edges.Count.ShouldBe(12);

        double first = edges[1] - edges[0];
        for (int at = 2; at < edges.Count; at++)
            (edges[at] - edges[at - 1]).ShouldBe(first, 0.01);
    }

    /// <summary>
    /// A shifted axis carries one extra interval, so its bars are narrower and start further left.
    /// </summary>
    /// <remarks>
    /// <c>"for explicit scales we need one interval more (maximum excluded)"</c>. With eleven
    /// months of data the unshifted axis divides the plot into eleven and the shifted one into
    /// twelve; and the shifted categories lead their own dates where the unshifted ones straddle
    /// them, so the first bar moves right by half a category rather than left.
    /// </remarks>
    [Fact]
    public void AShiftedAxisAddsAnIntervalAndLeadsItsDates()
    {
        double?[] dates = Quarterly(12);

        ChartDateAxis plain = ChartDateScale.Resolve(
            dates, statedMinimum: Jan2009, statedMaximum: dates[11]!.Value,
            statedResolution: ChartTimeUnit.Month, shifted: false)!;
        ChartDateAxis shifted = ChartDateScale.Resolve(
            dates, statedMinimum: Jan2009, statedMaximum: dates[11]!.Value,
            statedResolution: ChartTimeUnit.Month, shifted: true)!;

        // 33 months of data: unshifted spans 33, shifted 34.
        (shifted.Maximum - plain.Maximum).ShouldBeGreaterThan(27.0);

        List<double> loose = BarEdges(ChartLayout.Place(Bars(plain, 12), Frame, new Ruler()));
        List<double> tight = BarEdges(ChartLayout.Place(Bars(shifted, 12), Frame, new Ruler()));

        loose.Count.ShouldBe(12);
        tight.Count.ShouldBe(12);

        // The shifted axis fits the same twelve into a longer span, so they are closer together.
        (tight[1] - tight[0]).ShouldBeLessThan(loose[1] - loose[0]);

        // And each one leads its date instead of straddling it, which moves the first right.
        tight[0].ShouldBeGreaterThan(loose[0]);
    }

    /// <summary>A chart with no date axis is laid out by index, exactly as before.</summary>
    /// <remarks>
    /// The control. Every non-date bar chart in the corpus goes through this arm and none of
    /// them may move.
    /// </remarks>
    [Fact]
    public void ACategoryAxisIsStillLaidOutByIndex()
    {
        List<double> edges = BarEdges(ChartLayout.Place(Bars(null, 4), Frame, new Ruler()));

        edges.Count.ShouldBe(4);

        double pitch = edges[1] - edges[0];
        (edges[2] - edges[1]).ShouldBe(pitch, 0.01);
        (edges[3] - edges[2]).ShouldBe(pitch, 0.01);
    }
}
