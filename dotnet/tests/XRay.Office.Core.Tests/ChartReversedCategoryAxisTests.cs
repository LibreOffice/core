using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// What <c>c:orientation val="maxMin"</c> on a category axis does, and what it does not do.
/// </summary>
/// <remarks>
/// <para>
/// Every assertion here is the mechanism rather than a picture: which end a category is at, which
/// edge the value axis' line stands on, which end its labels sit at, and the order two series take
/// inside one category. All four were read off 26.2.4.2 by patching one attribute of a corpus
/// chart and rendering both versions — <c>probes/chart-cat-reverse/results.md</c> — because the
/// two renderings differ in nothing else.
/// </para>
/// <para>
/// <strong>A reversed category axis is one statement with two consequences and they must not be
/// separated.</strong> The categories turn round, and the value axis goes with them because it
/// stands at the <em>start</em> of the axis it crosses
/// (<c>AxisProperties::initAxisPositioning</c>,
/// <c>chart2/source/view/axes/VAxisProperties.cxx</c>:232-234). Reversing the order alone leaves a
/// Gantt with its tasks the right way up and its dates along the wrong edge.
/// </para>
/// </remarks>
public class ChartReversedCategoryAxisTests
{
    /// <summary>Half an em per character, 1.15 em a line — Liberation Sans to three places.</summary>
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
            => new(size * (0.5 * text.Length) * (bold ? 1.1 : 1.0), size * 1.15);
    }

    private static readonly DocRect Frame =
        new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300));

    private static ChartDrawing Place(ChartPlot plot) => ChartLayout.Place(plot, Frame, new Ruler());

    private static ChartPlot Columns() => new()
    {
        Categories = ["Q1", "Q2", "Q3", "Q4"],
        Series =
        [
            new ChartSeries("North", [120.0, 95.0, 143.0, 168.0], Colour.FromRgb(0x99CCFF)),
            new ChartSeries("South", [80.0, 110.0, 90.0, 130.0], Colour.FromRgb(0xCC6666)),
        ],
    };

    private static ChartPlot Bars() => Columns() with { Direction = ChartBarDirection.Bar };

    private static readonly string[] Names = ["Q1", "Q2", "Q3", "Q4"];

    /// <summary>A shape's extent, taken off the path rather than from a rectangle it is not.</summary>
    private static DocRect Bounds(ChartShape shape)
    {
        List<DocPoint> points =
            [.. shape.Path.Commands.Where(command => command.Verb != PathVerb.Close)
                        .Select(command => command.Point)];

        Length left = points.Min(point => point.X);
        Length top = points.Min(point => point.Y);

        return new DocRect(
            left, top, points.Max(point => point.X) - left, points.Max(point => point.Y) - top);
    }

    private static Length LabelY(ChartDrawing drawing, string text)
        => drawing.Labels.Single(label => label.Text == text).At.Y;

    private static Length LabelX(ChartDrawing drawing, string text)
        => drawing.Labels.Single(label => label.Text == text).At.X;

    /// <summary>
    /// A horizontal bar chart draws its first category at the bottom, and a reversed axis draws
    /// it at the top.
    /// </summary>
    /// <remarks>
    /// This is what a Gantt chart is: the tasks are meant to read downwards, a bar chart's
    /// categories run upwards, and every real Gantt states <c>maxMin</c> to turn them round.
    /// Measured on <c>N2_E_Maestroni_Swarm_COP.pptx</c> page 7 against 26.2.4.2, one attribute
    /// apart: <c>LEOP [0000]</c> is drawn at y = 507.14 with <c>minMax</c> and at y = 106.97 with
    /// the <c>maxMin</c> the file actually states.
    /// </remarks>
    [Fact]
    public void AReversedCategoryAxisPutsTheFirstCategoryAtTheTopOfABarChart()
    {
        ChartDrawing plain = Place(Bars());
        ChartDrawing turned = Place(Bars() with { CategoriesReversed = true });

        LabelY(plain, "Q1").ShouldBeGreaterThan(LabelY(plain, "Q4"));
        LabelY(turned, "Q1").ShouldBeLessThan(LabelY(turned, "Q4"));

        // And it is a mirror rather than a re-sort: each row sits as far from the plot's top as
        // it sat from its bottom. The plot rectangle itself moves — the value labels go to the
        // other edge with the axis — so the comparison has to be made inside it.
        static double[] Down(ChartDrawing drawing) =>
            [.. Names.Select(
                name => (double)(LabelY(drawing, name) - drawing.PlotArea.Top).Emu
                        / drawing.PlotArea.Height.Emu)];

        double[] before = Down(plain);
        double[] after = Down(turned);

        for (int at = 0; at < 4; at++) after[at].ShouldBe(1.0 - before[at], 0.001);
    }

    /// <summary>The same statement on a column chart moves the first category to the right.</summary>
    /// <remarks>
    /// Measured on <c>002_advanced_powerpoint_column.pptx</c> against 26.2.4.2: <c>M1</c> is drawn
    /// at x = 109.87 as authored and at x = 521.29 with the category axis reversed, while the
    /// category labels stay along the bottom in both.
    /// </remarks>
    [Fact]
    public void AReversedCategoryAxisPutsTheFirstCategoryAtTheRightOfAColumnChart()
    {
        ChartDrawing plain = Place(Columns());
        ChartDrawing turned = Place(Columns() with { CategoriesReversed = true });

        LabelX(plain, "Q1").ShouldBeLessThan(LabelX(plain, "Q4"));
        LabelX(turned, "Q1").ShouldBeGreaterThan(LabelX(turned, "Q4"));
    }

    /// <summary>A category's label moves with its own bars and not merely to the other end.</summary>
    /// <remarks>
    /// The trap this guards is a chart that turns its bars round and leaves its labels where they
    /// were, which reads as a perfectly ordinary chart of entirely wrong data. So the assertion is
    /// that the label and the bar it names still coincide, measured rather than assumed.
    /// </remarks>
    [Fact]
    public void ACategoryLabelMovesWithItsOwnBars()
    {
        ChartDrawing turned = Place(Bars() with { CategoriesReversed = true });

        foreach (string name in Names)
        {
            int index = Array.IndexOf(Names, name);
            Length y = LabelY(turned, name);

            // The bars of one category are the shapes whose own vertical extent contains the
            // label's baseline: a slot holds this category's bars and nothing else.
            List<DocRect> slot =
                [.. turned.Shapes.Select(Bounds)
                          .Where(bounds => bounds.Top <= y && y <= bounds.Bottom)];

            slot.Count.ShouldBe(2, $"{name} names one slot and its two series");

            // And that slot is the index-th from the top, the axis having been turned round.
            List<DocRect> ordered = [.. turned.Shapes.Select(Bounds)
                                              .OrderBy(bounds => bounds.Top)];
            ordered.IndexOf(slot[0]).ShouldBeInRange(index * 2, (index * 2) + 1);
        }
    }

    /// <summary>
    /// The series inside one category turn round with the categories.
    /// </summary>
    /// <remarks>
    /// They are laid along the same axis, so mirroring the axis mirrors them: the bar's whole
    /// extent is reflected and not merely its slot. Measured on
    /// <c>002_advanced_powerpoint_column.pptx</c> against 26.2.4.2, whose two clustered series are
    /// separately coloured — as authored the red series is drawn at x 98.87-116.62 and the blue at
    /// 116.62-134.36 in the first pair, and with the axis reversed the blue is at 75.49-93.23 and
    /// the red at 93.23-110.98.
    /// </remarks>
    [Fact]
    public void ReversingTheAxisSwapsTheSeriesWithinACategory()
    {
        static (Colour First, Colour Second) LeftmostPair(ChartDrawing drawing)
        {
            List<ChartShape> ordered =
                [.. drawing.Shapes.OrderBy(shape => Bounds(shape).Left)];

            return (ordered[0].Fill!.Value, ordered[1].Fill!.Value);
        }

        (Colour first, Colour second) = LeftmostPair(Place(Columns()));
        (Colour turnedFirst, Colour turnedSecond) =
            LeftmostPair(Place(Columns() with { CategoriesReversed = true }));

        first.ShouldBe(Colour.FromRgb(0x99CCFF));
        second.ShouldBe(Colour.FromRgb(0xCC6666));
        turnedFirst.ShouldBe(second);
        turnedSecond.ShouldBe(first);
    }

    /// <summary>
    /// The value axis' line stands at the start of the axis it crosses, so reversing that axis
    /// moves the line to the other edge.
    /// </summary>
    /// <remarks>
    /// Measured on both directions against 26.2.4.2. On the Gantt the horizontal value axis line
    /// is drawn at y = 514.97 with <c>minMax</c> and at y = 108.00 with <c>maxMin</c>; on the
    /// column chart the vertical one moves from x = 85.58 to x = 559.11 and its labels go with it,
    /// from x = 62-73 to x = 566.19. The rule is
    /// <c>AxisProperties::initAxisPositioning</c>:232-234.
    /// </remarks>
    [Fact]
    public void TheValueAxisStandsAtTheOtherEndOfAReversedCategoryAxis()
    {
        ChartDrawing plain = Place(Columns());
        ChartDrawing turned = Place(Columns() with { CategoriesReversed = true });

        // The value axis' labels are the numeric ones; on this chart every category label starts
        // with a Q and nothing else is drawn.
        static Length ValueLabelX(ChartDrawing drawing)
            => drawing.Labels.First(label => !label.Text.StartsWith('Q')).At.X;

        ValueLabelX(plain).ShouldBeLessThan(plain.PlotArea.Left);
        ValueLabelX(turned).ShouldBeGreaterThan(turned.PlotArea.Right);

        // A vertical axis line spanning the plot's whole height, on the side the labels are on.
        static Length AxisLineX(ChartDrawing drawing)
            => drawing.Lines
                      .Where(line => line.From.X == line.To.X
                                     && line.From.Y == drawing.PlotArea.Top
                                     && line.To.Y == drawing.PlotArea.Bottom)
                      .Select(line => line.From.X)
                      .Single();

        AxisLineX(plain).ShouldBe(plain.PlotArea.Left);
        AxisLineX(turned).ShouldBe(turned.PlotArea.Right);
    }

    /// <summary>
    /// <c>c:tickLblPos</c> names an end of the crossing axis, so it can send the value labels to
    /// the opposite edge from the axis line — and a reversed axis swaps which edge that is.
    /// </summary>
    /// <remarks>
    /// <c>VCartesianAxis::getLabelLineIntersectionValue</c>
    /// (<c>chart2/source/view/axes/VCartesianAxis.cxx</c>:1103-1113) gives the labels a line of
    /// their own at the crossing axis' logical minimum for <c>low</c> and its maximum for
    /// <c>high</c>. On <c>N2_E_Maestroni_Swarm_COP.pptx</c> that is the whole reason 26.2.4.2
    /// draws the value axis along the top of the Gantt and its date labels along the bottom.
    /// </remarks>
    [Theory]
    [InlineData(ChartValueLabelPosition.NextTo, false, true)]
    [InlineData(ChartValueLabelPosition.Low, false, true)]
    [InlineData(ChartValueLabelPosition.High, false, false)]
    [InlineData(ChartValueLabelPosition.NextTo, true, false)]
    [InlineData(ChartValueLabelPosition.Low, true, false)]
    [InlineData(ChartValueLabelPosition.High, true, true)]
    public void TickLabelPositionNamesAnEndOfTheCategoryAxisRatherThanASideOfThePage(
        ChartValueLabelPosition stated, bool reversed, bool labelsOnTheLeft)
    {
        ChartDrawing drawing = Place(Columns() with
        {
            CategoriesReversed = reversed,
            ValueLabelPosition = stated,
        });

        Length x = drawing.Labels.First(label => !label.Text.StartsWith('Q')).At.X;

        if (labelsOnTheLeft) x.ShouldBeLessThan(drawing.PlotArea.Left);
        else x.ShouldBeGreaterThan(drawing.PlotArea.Right);
    }

    /// <summary>
    /// <c>c:crosses</c> names the end the axis itself stands at, and the reversal mirrors that too.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Without it every reversed bar chart puts its value axis at the far edge, which is right for
    /// one of the corpus's five and wrong for the other four. Measured against 26.2.4.2 on all
    /// five: <c>045_Check_register_with_chart</c> says <c>autoZero</c> and the reference draws its
    /// value axis along the top of a reversed chart, while
    /// <c>003_Contextures_chart_sample</c>, <c>008_Contextures_chart_sample</c>,
    /// <c>010_Contextures_chart_sample</c> and <c>023_Waterfall_Chart_Template</c> all say
    /// <c>max</c> and the reference draws theirs along the bottom — which is what each file's own
    /// <c>c:axPos</c> says as well.
    /// </para>
    /// <para>
    /// Of 281 value axes over the corpus's chart parts, no primary one crossing a forward category
    /// axis says anything but <c>autoZero</c>, so the rule costs nothing anywhere else.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData(ChartAxisCrossing.Automatic, false, false)]
    [InlineData(ChartAxisCrossing.Minimum, false, false)]
    [InlineData(ChartAxisCrossing.Maximum, false, true)]
    [InlineData(ChartAxisCrossing.Automatic, true, true)]
    [InlineData(ChartAxisCrossing.Minimum, true, true)]
    [InlineData(ChartAxisCrossing.Maximum, true, false)]
    public void TheCrossingPositionNamesTheEndAndTheReversalMirrorsIt(
        ChartAxisCrossing crossing, bool reversed, bool axisOnTheRight)
    {
        ChartDrawing drawing = Place(Columns() with
        {
            CategoriesReversed = reversed,
            ValueAxisCrossing = crossing,
        });

        Length x = drawing.Labels.First(label => !label.Text.StartsWith('Q')).At.X;

        if (axisOnTheRight) x.ShouldBeGreaterThan(drawing.PlotArea.Right);
        else x.ShouldBeLessThan(drawing.PlotArea.Left);
    }

    /// <summary>
    /// The room the labels take comes off the side they are drawn on.
    /// </summary>
    /// <remarks>
    /// Reserving it on the axis line's side instead is invisible on every chart that says nothing
    /// — the two coincide — and draws the labels off the edge of the frame on one that says
    /// <c>high</c>.
    /// </remarks>
    [Fact]
    public void TheValueLabelsReserveTheirRoomOnTheSideTheyAreDrawnOn()
    {
        ChartDrawing plain = Place(Columns());
        ChartDrawing high = Place(Columns() with
        {
            ValueLabelPosition = ChartValueLabelPosition.High,
        });

        // Mirror images: the strip the plain chart takes off the left is the strip the `high`
        // chart takes off the right, and each keeps the other's spare margin.
        (Frame.Right - high.PlotArea.Right).ShouldBe(plain.PlotArea.Left - Frame.Left);
        (high.PlotArea.Left - Frame.Left).ShouldBe(Frame.Right - plain.PlotArea.Right);
    }
}
