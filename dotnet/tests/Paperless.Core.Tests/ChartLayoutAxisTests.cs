using Paperless.Core.Charts;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Numbers;
using Paperless.Core.Units;
using Shouldly;

namespace Paperless.Core.Tests;

/// <summary>
/// The three things that change what a chart's axes are: a deleted one, a second one, and a
/// scatter chart's numeric X.
/// </summary>
/// <remarks>
/// Each is a case where the picture stays entirely plausible when it is wrong — a deleted axis
/// drawn anyway is a normal-looking axis, a secondary series drawn against the primary scale is a
/// bar of the wrong height, and a scatter chart spaced evenly is a line chart. So the assertions
/// are about which marks exist and where they fall rather than about the whole drawing.
/// </remarks>
public class ChartLayoutAxisTests
{
    /// <summary>Half an em per character, 1.15 em a line — Liberation Sans to three places.</summary>
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family)
            => new(size * (0.5 * text.Length), size * 1.15);
    }

    private static readonly DocRect Frame =
        new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300));

    private static ChartDrawing Place(ChartPlot plot) => ChartLayout.Place(plot, Frame, new Ruler());

    private static ChartPlot Bars() => new()
    {
        Categories = ["Q1", "Q2", "Q3", "Q4"],
        Series = [new ChartSeries("North", [120.0, 95.0, 143.0, 168.0], Colour.FromRgb(0x99CCFF))],
    };

    /// <summary>
    /// A deleted category axis draws no labels and gives the room back to the plot area.
    /// </summary>
    /// <remarks>
    /// Both halves matter and only the first shows in a word count. <c>tdf105517.pptx</c> and
    /// <c>tdf106217.pptx</c> between them drew twelve category names the reference does not; the
    /// second half is why the bars are also the wrong height without it, the plot area having
    /// reserved a line of labels that are never drawn.
    /// </remarks>
    [Fact]
    public void ADeletedCategoryAxisDrawsNoLabelsAndReservesNoRoom()
    {
        ChartDrawing shown = Place(Bars());
        ChartDrawing hidden = Place(Bars() with { CategoryAxisVisible = false });

        shown.Labels.Count(label => label.Text.StartsWith('Q')).ShouldBe(4);
        hidden.Labels.ShouldNotContain(label => label.Text.StartsWith('Q'));

        hidden.PlotArea.Bottom.ShouldBeGreaterThan(shown.PlotArea.Bottom);
    }

    /// <summary>A deleted value axis keeps its gridlines.</summary>
    /// <remarks>
    /// <c>c:majorGridlines</c> hangs off the axis model and not off its view, so hiding the axis
    /// leaves the grid — which is exactly the chart that looks broken if the two are conflated.
    /// </remarks>
    [Fact]
    public void ADeletedValueAxisKeepsItsGridlines()
    {
        Colour grey = Colour.FromRgb(0xB3B3B3);

        ChartDrawing hidden = Place(
            Bars() with { ValueAxisVisible = false, ValueGrid = grey });

        hidden.Lines.Count(line => line.Colour == grey).ShouldBe(10);
        hidden.Labels.ShouldNotContain(label => label.Text == "180");
    }

    /// <summary>
    /// A series on the secondary axis is measured against the secondary scale.
    /// </summary>
    /// <remarks>
    /// The two series here differ by a factor of a hundred, which is the whole reason a chart has
    /// two value axes: drawn against one scale the small one is a flat line at the bottom, and
    /// drawn against its own it reaches the same height as the large one. The assertion is that
    /// the two topmost bars agree to within a point.
    /// </remarks>
    [Fact]
    public void ASecondarySeriesIsScaledAgainstItsOwnAxis()
    {
        ChartPlot plot = new()
        {
            Categories = ["Q1", "Q2"],
            Series =
            [
                new ChartSeries("Revenue", [100.0, 80.0], Colour.FromRgb(0x99CCFF)),
                new ChartSeries("Margin", [1.0, 0.8], Colour.FromRgb(0xFF9999)) { AxisIndex = 1 },
            ],
            SecondaryValueScale = new ChartScaleRequest(),
        };

        ChartDrawing drawing = Place(plot);
        drawing.Boxes.Count.ShouldBe(4);

        // The tallest bar of each series, by its top edge.
        Length primary = drawing.Boxes.Take(2).Min(box => box.Bounds.Top);
        Length secondary = drawing.Boxes.Skip(2).Min(box => box.Bounds.Top);

        Math.Abs((primary - secondary).Points).ShouldBeLessThan(1.0);
    }

    /// <summary>A secondary axis draws its ticks on the far side of the plot area.</summary>
    [Fact]
    public void ASecondaryAxisIsLabelledOnTheRight()
    {
        ChartPlot plot = new()
        {
            Categories = ["Q1"],
            Series =
            [
                new ChartSeries("Revenue", [100.0]),
                new ChartSeries("Margin", [1.0]) { AxisIndex = 1 },
            ],
            SecondaryValueScale = new ChartScaleRequest(),
        };

        ChartDrawing drawing = Place(plot);

        drawing.Labels.ShouldContain(label =>
            label.Anchor == ChartLabelAnchor.LeftMiddle
            && label.At.X >= drawing.PlotArea.Right);
    }

    /// <summary>
    /// A scatter chart's points are placed by their X values, not by their index.
    /// </summary>
    /// <remarks>
    /// The X here runs 0, 10, 100 over three points. Spaced evenly the middle one sits halfway
    /// across the plot area; spaced by value it sits a tenth of the way across, and the difference
    /// is the whole of what makes a scatter chart a scatter chart.
    /// </remarks>
    [Fact]
    public void AScatterChartSpacesItsPointsByTheirXValues()
    {
        ChartPlot plot = new()
        {
            Kind = ChartPlotKind.Scatter,
            Series =
            [
                new ChartSeries("S", [1.0, 2.0, 3.0], Colour.Black)
                {
                    XValues = [0.0, 10.0, 100.0],
                    Marker = ChartMarker.Square,
                },
            ],
        };

        ChartDrawing drawing = Place(plot);

        // One polyline plus one marker per point.
        drawing.Shapes.Count.ShouldBe(4);

        List<DocPoint> vertices = [.. drawing.Shapes[0].Path.Commands.Select(command => command.Point)];
        vertices.Count.ShouldBe(3);

        double middle =
            (vertices[1].X - drawing.PlotArea.Left).Emu / (double)drawing.PlotArea.Width.Emu;

        middle.ShouldBeLessThan(0.2);
    }

    /// <summary>A scatter chart's horizontal axis is labelled with numbers, not categories.</summary>
    [Fact]
    public void AScatterChartLabelsItsHorizontalAxisWithItsOwnScale()
    {
        ChartPlot plot = new()
        {
            Kind = ChartPlotKind.Scatter,
            Categories = ["one", "two", "three"],
            Series =
            [
                new ChartSeries("S", [1.0, 2.0, 3.0]) { XValues = [0.0, 60.0, 120.0] },
            ],
        };

        ChartDrawing drawing = Place(plot);

        drawing.Labels.ShouldNotContain(label => label.Text == "one");
        drawing.Labels.ShouldContain(label =>
            label.Text == "120" && label.Anchor == ChartLabelAnchor.CentreTop);
    }

    /// <summary>
    /// A value-axis tick goes through the axis' number format.
    /// </summary>
    /// <remarks>
    /// The end-to-end half of the layering move: the format lives in <c>Paperless.Core.Numbers</c>
    /// and the tick is written in <c>Paperless.Core.Charts</c>, so a chart composed in Core can
    /// reach it. Before the move a percentage axis drew <c>0 0.005 0.01 …</c>.
    /// </remarks>
    [Fact]
    public void AFormattedValueAxisWritesItsTicksThroughTheFormat()
    {
        ChartPlot plot = Bars() with
        {
            Series = [new ChartSeries("Share", [0.01, 0.02, 0.03, 0.05])],
            ValueFormat = NumberFormatCode.Parse("0.0%"),
        };

        ChartDrawing drawing = Place(plot);

        drawing.Labels.ShouldContain(label => label.Text == "5.0%");
        drawing.Labels.ShouldNotContain(label => label.Text == "0.05");
    }

    /// <summary>
    /// A bar's data label sits beyond the bar's growing end.
    /// </summary>
    /// <remarks>
    /// <c>OUTSIDE</c> is a bar chart's default placement
    /// (<c>oox/source/drawingml/chart/typegroupconverter.cxx:95</c>), which for a positive column
    /// means above the top of the bar.
    /// </remarks>
    [Fact]
    public void ABarsDataLabelSitsAboveIt()
    {
        // Values chosen off the tick ladder — the scale here is 0, 20 … 180 — so that a data
        // label and a tick label cannot be confused for one another by their text.
        ChartPlot plot = Bars() with
        {
            Series =
            [
                new ChartSeries("North", [123.0, 97.0, 145.0, 168.0])
                {
                    Label = new ChartDataLabel { ShowValue = true },
                },
            ],
        };

        ChartDrawing drawing = Place(plot);

        List<ChartLabel> values =
            [.. drawing.Labels.Where(label => label.Text is "123" or "97" or "145" or "168")];

        values.Count.ShouldBe(4);
        values.ShouldAllBe(label => label.Anchor == ChartLabelAnchor.CentreBottom);

        // Each above its own bar's top edge.
        foreach (ChartLabel label in values)
        {
            ChartBox bar = drawing.Boxes
                .Where(box => box.Bounds.Left <= label.At.X && box.Bounds.Right >= label.At.X)
                .OrderBy(box => box.Bounds.Top.Emu)
                .First();

            label.At.Y.ShouldBeLessThanOrEqualTo(bar.Bounds.Top);
        }
    }

    /// <summary>
    /// A point's own label wins over the series', including when it shows nothing.
    /// </summary>
    /// <remarks>
    /// The deleted case is the one that costs a measurement: <c>tdf105517.pptx</c> switches off
    /// ten of a series' eleven labels and the eleventh is the only one the reference draws.
    /// </remarks>
    [Fact]
    public void ADeletedPointLabelOverridesTheSeriesRatherThanFallingBackToIt()
    {
        ChartPlot plot = Bars() with
        {
            Series =
            [
                new ChartSeries("North", [123.0, 97.0, 145.0, 168.0])
                {
                    Label = new ChartDataLabel { ShowValue = true },
                    PointLabels = [new ChartDataLabel(), new ChartDataLabel(), null, null],
                },
            ],
        };

        ChartDrawing drawing = Place(plot);

        drawing.Labels.ShouldNotContain(label => label.Text == "123");
        drawing.Labels.ShouldNotContain(label => label.Text == "97");
        drawing.Labels.ShouldContain(label => label.Text == "145");
        drawing.Labels.ShouldContain(label => label.Text == "168");
    }

    /// <summary>
    /// A stretched chart carries its residual horizontal scale onto every label.
    /// </summary>
    /// <remarks>
    /// A glyph run has one em, and an embedded chart is scaled by two factors. The label's
    /// <see cref="ChartLabel.Size"/> takes the vertical one and
    /// <see cref="ChartLabel.Stretch"/> the ratio, so a chart squeezed to 0.5 across and 1.0 down
    /// asks its consumer for text at full height and half width — which on
    /// <c>chart-bar-sheet.ods</c> is the difference between type 12% too wide and type that fits.
    /// </remarks>
    [Fact]
    public void AStretchedChartCarriesTheResidualHorizontalScaleOnItsLabels()
    {
        ChartPlot plot = Bars() with
        {
            Title = "Regional revenue",
            Space = new DocSize(Length.FromPoints(400), Length.FromPoints(300)),
        };

        ChartDrawing drawing = ChartLayout.Place(
            plot,
            new DocRect(Length.Zero, Length.Zero, Length.FromPoints(200), Length.FromPoints(300)),
            new Ruler());

        drawing.Labels.ShouldAllBe(label => Math.Abs(label.Stretch - 0.5) < 1e-9);
    }

    /// <summary>An unstretched chart leaves the factor at exactly one.</summary>
    [Fact]
    public void AnUnstretchedChartLeavesEveryLabelAtItsNaturalWidth()
        => Place(Bars() with { Title = "Regional revenue" })
            .Labels.ShouldAllBe(label => label.Stretch == 1.0);
}
