using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// A bar is clipped to the value axis' own range, and one entirely outside it is not drawn.
/// </summary>
/// <remarks>
/// <para>
/// <c>PlottingPositionHelper::clipYRange</c>
/// (<c>chart2/source/view/inc/PlottingPositionHelper.hxx</c>:401-415) clamps a bar's two values
/// to the axis' minimum and maximum and answers false when nothing of it remains;
/// <c>BarChart::createShapes</c> (<c>chart2/source/view/charttypes/BarChart.cxx</c>:789) calls it
/// before it computes any geometry and <c>continue</c>s on false, which is why a rejected point
/// gets no data label either.
/// </para>
/// <para>
/// <strong>A stack with a stated minimum is what makes it visible.</strong> A Gantt chart is
/// written as an invisible "start" series with a visible "duration" one on top of it and an
/// explicit <c>c:min</c> at the first date, so the start segment runs from serial zero — 41 600
/// days and 192 386 points left of the plot on <c>N2_E_Maestroni_Swarm_COP.pptx</c> page 7.
/// Unclipped, every bar of that chart is drawn from there, and the chart's drawn extent — which
/// is what <see cref="ChartLayout"/> fits onto the frame — is that wide too.
/// </para>
/// </remarks>
public sealed class ChartBarClipTests
{
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
            => new(size * (0.5 * text.Length), size * 1.15);
    }

    private static readonly DocRect Frame =
        new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300));

    /// <summary>A stack of an off-scale base and a visible segment, as a Gantt is written.</summary>
    private static ChartPlot Gantt(bool stated) => new()
    {
        Categories = ["A", "B"],
        IsStacked = true,
        Overlap = 100,
        ValueScale = stated
            ? new ChartScaleRequest(Minimum: 1000.0, Maximum: 1100.0)
            : default,
        Series =
        [
            new ChartSeries("Start", [1010.0, 1040.0], Colour.FromRgb(0xDDDDDD)),
            new ChartSeries("Duration", [20.0, 30.0], Colour.FromRgb(0x0070C0)),
        ],
    };

    private static DocRect Bounds(ChartShape shape)
    {
        Length left = Length.FromEmu(long.MaxValue);
        Length top = Length.FromEmu(long.MaxValue);
        Length right = Length.FromEmu(long.MinValue);
        Length bottom = Length.FromEmu(long.MinValue);

        foreach (PathCommand command in shape.Path.Commands)
        {
            if (command.Verb == PathVerb.Close) continue;
            left = Length.Min(left, command.Point.X);
            top = Length.Min(top, command.Point.Y);
            right = Length.Max(right, command.Point.X);
            bottom = Length.Max(bottom, command.Point.Y);
        }

        return new DocRect(left, top, right - left, bottom - top);
    }

    /// <summary>The base segment is clipped to the plot's edge rather than run off it.</summary>
    [Fact]
    public void ABarStartingBelowTheAxisMinimumIsClippedToIt()
    {
        ChartDrawing drawing = ChartLayout.Place(Gantt(stated: true), Frame, new Ruler());

        drawing.Shapes.ShouldNotBeEmpty();
        foreach (ChartShape shape in drawing.Shapes)
            Bounds(shape).Left.ShouldBeGreaterThanOrEqualTo(drawing.PlotArea.Left);
    }

    /// <summary>Nothing the chart draws reaches outside the plot it is drawn in.</summary>
    [Fact]
    public void NoBarReachesOutsideThePlotArea()
    {
        ChartDrawing drawing = ChartLayout.Place(Gantt(stated: true), Frame, new Ruler());

        foreach (ChartShape shape in drawing.Shapes)
        {
            DocRect bounds = Bounds(shape);
            bounds.Left.ShouldBeGreaterThanOrEqualTo(drawing.PlotArea.Left);
            bounds.Right.ShouldBeLessThanOrEqualTo(drawing.PlotArea.Right);
        }
    }

    /// <summary>
    /// A stacked segment lying entirely beyond the maximum draws nothing at all.
    /// </summary>
    /// <remarks>
    /// The half of <c>clipYRange</c> that answers false rather than clamping. Clamping alone
    /// would leave a zero-height bar sitting on the wall, which is a rule the reference does not
    /// draw. It takes a stack to reach: an unstacked bar always has one end at the baseline,
    /// which <c>getBaseValueY</c> has already put inside the scale, so one end is always in
    /// range and the point is only ever clamped.
    /// </remarks>
    [Fact]
    public void AStackedSegmentEntirelyAboveTheMaximumDrawsNothing()
    {
        ChartPlot plot = new()
        {
            Categories = ["A"],
            IsStacked = true,
            Overlap = 100,
            ValueScale = new ChartScaleRequest(Minimum: 0.0, Maximum: 10.0),
            Series =
            [
                new ChartSeries("Under", [20.0], Colour.FromRgb(0xDDDDDD)),
                new ChartSeries("Over", [5.0], Colour.FromRgb(0x0070C0)),
            ],
        };

        ChartLayout.Place(plot, Frame, new Ruler()).Shapes.Count.ShouldBe(1);
    }

    /// <summary>A chart whose values all fit is drawn exactly as it was before.</summary>
    /// <remarks>
    /// The control. Clipping may only ever remove geometry that was outside the axis, so a
    /// chart with an automatic scale — where the range is derived from the data and nothing can
    /// be outside it — must be untouched.
    /// </remarks>
    [Fact]
    public void AChartWhoseValuesAllFitIsUnchanged()
    {
        ChartPlot plot = new()
        {
            Categories = ["A", "B", "C"],
            Series = [new ChartSeries("In", [4.0, 9.0, 2.0], Colour.FromRgb(0x0070C0))],
        };

        ChartDrawing drawing = ChartLayout.Place(plot, Frame, new Ruler());

        drawing.Shapes.Count.ShouldBe(3);
        foreach (ChartShape shape in drawing.Shapes)
            Bounds(shape).Height.ShouldBeGreaterThan(Length.Zero);
    }
}
