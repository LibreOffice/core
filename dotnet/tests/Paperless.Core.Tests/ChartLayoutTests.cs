using Paperless.Core.Charts;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Shouldly;

namespace Paperless.Core.Tests;

/// <summary>
/// What each plot type composes, and the two decisions that are invisible in any one mark.
/// </summary>
/// <remarks>
/// <para>
/// The engine's output is a list of rectangles, lines, paths and labels, so the useful assertions
/// are counts and extents rather than pixels: how many wedges a pie has, whether a line chart
/// reaches both edges of its plot area, whether a chart with no axes drew any tick labels. Those
/// are exactly the properties a whole-page comparison reports as "a bit different" and cannot
/// name.
/// </para>
/// <para>
/// Text is measured by a stand-in rather than by a real face, because none of what is asserted
/// here depends on a particular font and a test that loaded one would fail on a machine without
/// it. The stand-in's line height is 1.15 em, which is Liberation Sans' to three decimal places.
/// </para>
/// </remarks>
public class ChartLayoutTests
{
    /// <summary>A measurer with no fonts: half an em per character, 1.15 em a line.</summary>
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size)
            => new(size * (0.5 * text.Length), size * 1.15);
    }

    private static readonly DocRect Frame =
        new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300));

    private static ChartPlot Bars() => new()
    {
        Categories = ["Q1", "Q2", "Q3", "Q4"],
        Series = [new ChartSeries("North", [120.0, 95.0, 143.0, 168.0], Colour.FromRgb(0x99CCFF))],
    };

    private static ChartDrawing Place(ChartPlot plot) => ChartLayout.Place(plot, Frame, new Ruler());

    [Fact]
    public void AGridlineIsDrawnAcrossThePlotAreaAtEveryMajorTick()
    {
        ChartDrawing without = Place(Bars());
        ChartDrawing with = Place(Bars() with { ValueGrid = Colour.FromRgb(0xB3B3B3) });

        // Ten ticks on the corpus scale, so ten more lines and no other change.
        with.Lines.Count.ShouldBe(without.Lines.Count + 10);

        List<ChartLine> grid =
            [.. with.Lines.Where(line => line.Colour == Colour.FromRgb(0xB3B3B3))];

        grid.Count.ShouldBe(10);

        // Each spans the plot area's full width, which is what distinguishes a gridline from the
        // tick mark at the same height — the tick runs 4.25 pt *outside* the axis.
        foreach (ChartLine line in grid)
        {
            line.From.X.ShouldBe(with.PlotArea.Left);
            line.To.X.ShouldBe(with.PlotArea.Right);
            line.From.Y.ShouldBe(line.To.Y);
        }
    }

    [Fact]
    public void APieDrawsAWedgePerPointAndNoAxisAtAll()
    {
        ChartPlot pie = Bars() with
        {
            Kind = ChartPlotKind.Pie,
            Legend = ChartLegendPosition.Right,
        };

        ChartDrawing drawing = Place(pie);

        drawing.Shapes.Count.ShouldBe(4);

        // No axis line, no tick, no gridline: a pie has neither axis, and the first version of the
        // reader drew both — 82 words of invented labels on a chart the reference gives one.
        drawing.Lines.ShouldBeEmpty();

        // And the labels it does draw are the legend's, which for a pie names the categories
        // rather than the single series.
        List<string> text = [.. drawing.Labels.Select(label => label.Text)];
        text.ShouldBe(["Q1", "Q2", "Q3", "Q4"], ignoreOrder: true);
    }

    [Fact]
    public void APiesWedgesStartAtTwelveOClockAndRunClockwise()
    {
        ChartPlot pie = new()
        {
            Kind = ChartPlotKind.Pie,
            Categories = ["A", "B", "C", "D"],
            Series = [new ChartSeries("s", [1.0, 1.0, 1.0, 1.0], Colour.Black)],
        };

        ChartDrawing drawing = Place(pie);
        drawing.Shapes.Count.ShouldBe(4);

        DocPoint centre = new(
            drawing.PlotArea.X + drawing.PlotArea.Width / 2,
            drawing.PlotArea.Y + drawing.PlotArea.Height / 2);

        // Every wedge starts at the centre and its first straight segment is the radius it opens
        // on. Four equal quarters open at 12, 3, 6 and 9 o'clock in that order.
        List<DocPoint> opens =
            [.. drawing.Shapes.Select(shape => shape.Path.Commands[1].Point)];

        Near(opens[0].X, centre.X);
        opens[0].Y.ShouldBeLessThan(centre.Y);

        opens[1].X.ShouldBeGreaterThan(centre.X);
        Near(opens[1].Y, centre.Y);

        Near(opens[2].X, centre.X);
        opens[2].Y.ShouldBeGreaterThan(centre.Y);

        static void Near(Length actual, Length expected)
            => Math.Abs(actual.Emu - expected.Emu).ShouldBeLessThan(Length.FromPoints(0.01).Emu);
    }

    [Fact]
    public void ALineChartTouchesBothEdgesWhereABarChartNeverDoes()
    {
        ChartDrawing line = Place(Bars() with { Kind = ChartPlotKind.Line });

        line.Shapes.Count.ShouldBe(1);

        List<DocPoint> points = [.. line.Shapes[0].Path.Commands.Select(command => command.Point)];
        points.Count.ShouldBe(4);

        // ShiftedCategoryPosition is false for a line chart, so the first point sits on the plot
        // area's left edge and the last on its right. A bar chart's leftmost bar starts a fraction
        // of a slot in, which is the whole difference between the two axes.
        points[0].X.ShouldBe(line.PlotArea.Left);
        points[^1].X.ShouldBe(line.PlotArea.Right);

        ChartDrawing bars = Place(Bars());
        bars.Boxes.Count.ShouldBe(4);
        bars.Boxes[0].Bounds.Left.ShouldBeGreaterThan(bars.PlotArea.Left);
    }

    [Fact]
    public void ALineIsBrokenAtAGapRatherThanBridgedAcrossIt()
    {
        ChartPlot gapped = Bars() with
        {
            Kind = ChartPlotKind.Line,
            Series = [new ChartSeries("North", [120.0, null, 143.0, 168.0], Colour.Black)],
        };

        GraphicsPath path = Place(gapped).Shapes[0].Path;

        // Two subpaths, so two MoveTo: bridging the hole would give one MoveTo and three LineTo,
        // and would draw a straight segment no reader could tell from a real value.
        path.Commands.Count(command => command.Verb == PathVerb.MoveTo).ShouldBe(2);
        path.Commands.Count(command => command.Verb == PathVerb.LineTo).ShouldBe(1);
    }

    [Fact]
    public void AnAreaIsAClosedRegionBetweenItsPointsAndTheBaseline()
    {
        ChartDrawing drawing = Place(Bars() with { Kind = ChartPlotKind.Area });

        drawing.Shapes.Count.ShouldBe(1);

        GraphicsPath path = drawing.Shapes[0].Path;
        path.Commands[^1].Verb.ShouldBe(PathVerb.Close);
        drawing.Shapes[0].Fill.ShouldNotBeNull();

        // Four points along the top and four back along the baseline.
        path.Commands.Count(command => command.Verb is PathVerb.MoveTo or PathVerb.LineTo)
            .ShouldBe(8);
    }

    [Fact]
    public void ASmallChartGetsFewerTicksThanALargeOneWithTheSameNumbers()
    {
        // The second pass. Both charts hold the same 88..168; the large one has room for ten
        // intervals and lands on 0..180 in steps of 20, and the small one has room for four and
        // is forced up the 1-2-5 ladder to steps of 50.
        ChartPlot plot = Bars();

        int large = ChartLayout
            .Place(plot, Frame, new Ruler())
            .Labels.Count(label => label.Text is "20");

        ChartDrawing small = ChartLayout.Place(
            plot,
            new DocRect(Length.Zero, Length.Zero, Length.FromPoints(220), Length.FromPoints(120)),
            new Ruler());

        large.ShouldBe(1);
        small.Labels.Select(label => label.Text).ShouldNotContain("20");
        small.Labels.Select(label => label.Text).ShouldContain("50");
    }

    [Fact]
    public void AChartWithASpaceOfItsOwnIsStretchedRatherThanRecomposed()
    {
        // An OLE chart is rendered at its own stated size and scaled into the frame that shows it,
        // which is what makes chart-bar-sheet.ods draw the same ten ticks its .odp twin does even
        // though its frame is two thirds the size.
        ChartPlot plot = Bars() with
        {
            Space = new DocSize(Length.FromPoints(400), Length.FromPoints(300)),
            PlotArea = new DocRect(
                Length.FromPoints(50), Length.FromPoints(20),
                Length.FromPoints(300), Length.FromPoints(200)),
        };

        ChartDrawing half = ChartLayout.Place(
            plot,
            new DocRect(Length.Zero, Length.Zero, Length.FromPoints(200), Length.FromPoints(150)),
            new Ruler());

        half.PlotArea.X.ShouldBe(Length.FromPoints(25));
        half.PlotArea.Y.ShouldBe(Length.FromPoints(10));
        half.PlotArea.Width.ShouldBe(Length.FromPoints(150));
        half.PlotArea.Height.ShouldBe(Length.FromPoints(100));

        // The type is stretched with everything else, so a 10 pt label is drawn at 5 pt.
        half.Labels.ShouldAllBe(label => label.Size <= Length.FromPoints(7));
    }
}
