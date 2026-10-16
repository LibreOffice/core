using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// The pie's second pass runs for every pie and doughnut, and for nothing that states its own
/// inner rectangle.
/// </summary>
/// <remarks>
/// <para>
/// <c>impl_createDiagramAndContent</c> draws a pie once, takes the bounding box of everything the
/// diagram group produced and recreates the whole chart at
/// <c>VDiagram::adjustInnerSize(consumedOuterRect)</c>. The gate is
/// <c>if( bIsPieOrDonut )</c> (<c>chart2/source/view/main/ChartView.cxx</c>:682) and
/// <c>lcl_IsPieOrDonut</c> is <c>xDiagram-&gt;isPieOrDonutChart()</c> (<c>:339-346</c>) — the
/// chart type and nothing else — with each call inside guarded by
/// <c>if (!rParam.mbUseFixedInnerSize)</c>.
/// </para>
/// <para>
/// <strong>Both halves of that are measured against 26.2.4.2's own resolved view rather than read
/// out of a tree that is not its source.</strong> <c>--convert-to fodt/fods/fodp</c> writes
/// <c>&lt;chart:plot-area&gt;</c> — the rectangle the diagram was given — and
/// <c>&lt;chart:coordinate-region&gt;</c>, the one <c>adjustInnerSize</c> settled on. Over the
/// corpus's 37 pie, of-pie and doughnut documents:
/// </para>
/// <list type="bullet">
/// <item><description>
/// <c>027_Unit_Circle_Chart_Graphical_Chart</c>, whose labels are all <c>outEnd</c> and none
/// best-fit, has a coordinate region 0.876 of its plot area's inscribed square — so the pass runs
/// with no best-fit label anywhere.
/// </description></item>
/// <item><description>
/// all fifteen doughnuts have one of 0.9999 — so the pass runs and consumes nothing, because
/// <c>AVOID_OVERLAP</c> is turned into <c>CENTER</c> for a ring chart and its labels never leave
/// the wall.
/// </description></item>
/// <item><description>
/// <c>003_Contextures_chart_sample</c>'s pie states <c>c:layoutTarget val="inner"</c> and its
/// coordinate region is its stated rectangle to a hundredth of a point — 179.86 × 169.68 against
/// this tree's 179.82 × 169.69 — where running the pass on it gave 142.69 square.
/// </description></item>
/// </list>
/// </remarks>
public sealed class ChartPieSecondPassTests
{
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
            => new(size * (0.5 * text.Length) * (bold ? 1.1 : 1.0), size * 1.15);
    }

    private static readonly DocRect Frame =
        new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300));

    private static ChartPlot Pie(
        bool rings, ChartLabelPlacement? placement, ChartPlotKind kind = ChartPlotKind.Pie)
        => new()
        {
            Kind = kind,
            Rings = rings,
            Legend = ChartLegendPosition.None,
            Categories =
            [
                "A category name of some length",
                "Another category name of some length",
                "A third category name of some length",
                "A fourth category name of some length",
            ],
            Series =
            [
                new ChartSeries("S", [1.0, 2.0, 3.0, 4.0], Colour.FromRgb(0x99CCFF))
                {
                    Label = placement is null
                        ? null
                        : new ChartDataLabel { ShowCategory = true, Placement = placement },
                },
            ],
        };

    /// <summary>
    /// How much of the diagram rectangle the plot rectangle ended up with, vertically.
    /// </summary>
    /// <remarks>
    /// <strong>A ratio and not a length, because <c>ChartLayout.Place</c> squeezes the whole
    /// drawing to its own drawn extent afterwards</strong> — <c>ChartDrawnExtentFitTests</c>, and
    /// 26.2.4.2 does the same in <c>ViewContactOfSdrOle2Obj</c> — by two factors, one per axis. A
    /// pie whose labels overflow is therefore smaller in the finished drawing whatever the second
    /// pass did, and comparing lengths measures the fit rather than the pass. The fit scales the
    /// plot rectangle and the diagram rectangle by the same per-axis factor, so their ratio is
    /// invariant under it.
    /// </remarks>
    private static double Shrink(ChartDrawing drawing)
        => drawing.PlotArea.Height.Points / drawing.DiagramArea.Height.Points;

    /// <summary>
    /// A pie whose labels are <c>outEnd</c> — none of them best-fit — is shrunk by them.
    /// </summary>
    /// <remarks>
    /// This is the case the gate used to miss: it stood on "there is a best-fit label somewhere"
    /// for eleven rounds, and the reference's own <c>fodt</c> for
    /// <c>027_Unit_Circle_Chart_Graphical_Chart</c> refutes it — as does its own rendering, whose
    /// drawn pie is 350.40 × 352.10 pt where this tree drew 406.28 × 390.45.
    /// </remarks>
    [Fact]
    public void APieWithOutsideLabelsAndNoBestFitLabelIsStillShrunkByThem()
    {
        double shrunk = Shrink(
            ChartLayout.Place(Pie(false, ChartLabelPlacement.Outside), Frame, new Ruler()));

        shrunk.ShouldBeLessThan(0.9);
    }

    /// <summary>
    /// A pie whose labels are drawn inside its own wedges consumes nothing outside the wall, so
    /// the pass runs and leaves the diagram at the whole available square.
    /// </summary>
    [Fact]
    public void APieWithCentredLabelsKeepsTheWholeAvailableSquare()
    {
        Shrink(ChartLayout.Place(Pie(false, ChartLabelPlacement.Centre), Frame, new Ruler()))
            .ShouldBe(1.0, 0.001);
    }

    /// <summary>
    /// A doughnut is not shrunk by its labels, whatever they say, because a ring chart's labels
    /// are centred and cannot move.
    /// </summary>
    /// <remarks>
    /// Running this file's single-pie label placer over a ring would put labels outside it and
    /// shrink every one of the corpus's fifteen doughnuts, whose reference coordinate regions are
    /// 0.9999 of their available square.
    /// </remarks>
    [Theory]
    [InlineData(ChartLabelPlacement.Outside)]
    [InlineData(ChartLabelPlacement.BestFit)]
    [InlineData(ChartLabelPlacement.Centre)]
    public void ADoughnutIsNotShrunkByItsLabels(ChartLabelPlacement placement)
        => Shrink(ChartLayout.Place(Pie(true, placement), Frame, new Ruler()))
            .ShouldBe(1.0, 0.001);

    /// <summary>
    /// A pie that states its own <em>inner</em> rectangle keeps it: that is
    /// <c>mbUseFixedInnerSize</c>, and every <c>adjustInnerSize</c> call is guarded by it.
    /// </summary>
    /// <remarks>
    /// The labels are centred so that nothing overflows and <see cref="Shrink"/>'s extent fit
    /// stays out of the measurement. It still separates the two states: the pass ignores the
    /// stated rectangle entirely and recomposes from the diagram rectangle, so running it here
    /// gives the 288 pt square inscribed in that and skipping it gives the stated
    /// 200 × 180 at 40, 30.
    /// </remarks>
    [Fact]
    public void AStatedInnerRectangleIsNotAdjusted()
    {
        ChartPlot plot = Pie(false, ChartLabelPlacement.Centre) with
        {
            PlotAreaFraction = (0.1, 0.1, 0.5, 0.6),
        };

        ChartDrawing drawing = ChartLayout.Place(plot, Frame, new Ruler());

        drawing.PlotArea.X.Points.ShouldBe(40.0, 0.05);
        drawing.PlotArea.Y.Points.ShouldBe(30.0, 0.05);
        drawing.PlotArea.Width.Points.ShouldBe(200.0, 0.05);
        drawing.PlotArea.Height.Points.ShouldBe(180.0, 0.05);
    }

    /// <summary>
    /// An of-pie's own composition is part of what the second pass measures, and it overflows the
    /// diagram square on both sides by design.
    /// </summary>
    /// <remarks>
    /// <c>m_fLeftShift − m_fLeftScale</c> = −1.4167 unit radii to <c>m_fBarRight</c> = 1.25, so
    /// 2.667 radii against the square's 2 (<c>PieChart.hxx:258-269</c>). Leaving it out makes the
    /// consumed rectangle the wall's and the diagram grows back to the whole available rectangle
    /// centred, which is a different <em>position</em> even where the size is unchanged.
    /// </remarks>
    [Fact]
    public void AnOfPiesOwnCompositionIsInWhatTheSecondPassMeasures()
    {
        ChartPlot plot = Pie(false, ChartLabelPlacement.Centre, ChartPlotKind.OfPie) with
        {
            OfPieType = ChartOfPieType.Bar,
            SplitPosition = 2,
        };

        ChartDrawing drawing = ChartLayout.Place(plot, Frame, new Ruler());

        // The composition is 2.667 radii wide against the square's 2 and hangs 0.4167 radii off
        // the left, so the square is pushed right of centre by exactly that overhang's share of
        // the room the pass hands back.
        DocRect area = drawing.PlotArea;
        DocRect outer = drawing.DiagramArea;

        Length centred = outer.X + ((outer.Width - area.Width) / 2);
        area.X.ShouldBeGreaterThan(centred);
    }
}
