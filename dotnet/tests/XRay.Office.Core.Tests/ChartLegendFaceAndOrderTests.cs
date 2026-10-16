using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// The legend's own face, and the order it lists its entries in.
/// </summary>
/// <remarks>
/// <para>
/// <strong>The face.</strong> A chart is not set in one face and the legend is where that shows.
/// Measured on <c>001_advanced_powerpoint_bar.pptx</c> page 1 against 26.2.4.2: its axes state
/// <c>Arial</c>, its legend states nothing, and the reference draws the page's seventeen
/// ten-point axis runs in LiberationSans and its two ten-point legend runs in Carlito — the
/// theme's Calibri. The two assertions here are separate because the code that carries them is
/// separate, exactly as <see cref="ChartTextFamilyTests"/> says: the reservation is measured
/// through <c>ChartText.For</c> and the drawing is stamped onto each label.
/// </para>
/// <para>
/// <strong>The order.</strong> <c>VSeriesPlotter::createLegendEntries</c>
/// (<c>chart2/source/view/charttypes/VSeriesPlotter.cxx</c>:2432-2447) inserts a series' entries
/// at the front rather than the back under two conditions, and 26.2.4.2 was asked about four of
/// them:
/// </para>
/// <code>
///   horizontal bar, clustered, legend right   REVERSED   001_advanced_powerpoint_bar.pptx
///   column,         clustered, legend right   in order   002_advanced_powerpoint_column.pptx
///   area,           clustered, legend right   in order   006_advanced_powerpoint_area.pptx
///   column,         stacked,   legend right   REVERSED   stacked_bar_chart.pptx
///   area,           stacked,   legend right   REVERSED   stacked_area_chart.pptx
/// </code>
/// <para>
/// Two of those five are controls that must not move, and they are the ones that make the other
/// three worth having: the same deck family, the same two series and the same legend position
/// separate the rule from "we list them backwards".
/// </para>
/// </remarks>
public class ChartLegendFaceAndOrderTests
{
    /// <summary>A measurer that records the face it was asked for and answers by character.</summary>
    private sealed class Ruler : IChartTextMeasurer
    {
        public List<(string Text, string? Family)> Asked { get; } = [];

        public DocSize Measure(string text, Length size, string? family, bool bold)
        {
            Asked.Add((text, family));

            // A face-dependent width, so that a caller which measures the legend in the chart's
            // face rather than the legend's is visible in the geometry and not only in the log.
            double per = family == "Narrow" ? 0.35 : 0.5;
            return new DocSize(size * (per * text.Length), size * 1.15);
        }
    }

    private static readonly DocRect Frame =
        new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300));

    private static ChartPlot Two(ChartPlotKind kind = ChartPlotKind.Bar) => new()
    {
        Kind = kind,
        Categories = ["Q1", "Q2", "Q3", "Q4"],
        Series =
        [
            new ChartSeries("Actual", [120.0, 95.0, 143.0, 168.0], Colour.FromRgb(0xC0504D)),
            new ChartSeries("Plan", [100.0, 100.0, 150.0, 150.0], Colour.FromRgb(0x4F81BD)),
        ],
        Legend = ChartLegendPosition.Right,
    };

    private static List<string> LegendNames(ChartDrawing drawing)
    {
        // The legend's labels are the ones carrying a series name; the axis and category labels
        // carry numbers and the category strings.
        List<string> names = [];
        foreach (ChartLabel label in drawing.Labels)
            if (label.Text is "Actual" or "Plan")
                names.Add(label.Text);

        return names;
    }

    // ------------------------------------------------------------------ the face

    [Fact]
    public void TheLegendIsMeasuredInItsOwnFaceAndTheRestOfTheChartInTheChartsFace()
    {
        Ruler ruler = new();
        ChartLayout.Place(
            Two() with { TextFamily = "Wide", LegendFamily = "Narrow" }, Frame, ruler);

        ruler.Asked.ShouldContain(a => a.Text == "Actual" && a.Family == "Narrow");
        ruler.Asked.ShouldContain(a => a.Text == "Q1" && a.Family == "Wide");
    }

    [Fact]
    public void TheLegendsLabelsCarryTheLegendsFaceAndTheOthersTheChartsFace()
    {
        ChartDrawing drawing = ChartLayout.Place(
            Two() with { TextFamily = "Wide", LegendFamily = "Narrow" }, Frame, new Ruler());

        foreach (ChartLabel label in drawing.Labels)
        {
            string? expected = label.Text is "Actual" or "Plan" ? "Narrow" : "Wide";
            label.Family.ShouldBe(expected, $"'{label.Text}'");
        }
    }

    /// <summary>
    /// A narrower legend face leaves the plot rectangle wider, which is the whole point of
    /// measuring it separately.
    /// </summary>
    /// <remarks>
    /// The plot's right edge is <c>frame.Right − margin − legend.Width − LegendMarginX</c> and the
    /// legend's width carries its widest entry, so a face that measures the entry narrower gives
    /// the plot the difference. On the corpus that difference is 2.70 pt on seventeen chart pages.
    /// </remarks>
    [Fact]
    public void ANarrowerLegendFaceWidensThePlotRectangle()
    {
        ChartPlot wide = Two() with { TextFamily = "Wide" };
        ChartPlot narrow = wide with { LegendFamily = "Narrow" };

        ChartLayout.Place(narrow, Frame, new Ruler()).PlotArea.Right
            .ShouldBeGreaterThan(ChartLayout.Place(wide, Frame, new Ruler()).PlotArea.Right);
    }

    /// <summary>A null legend face is the chart's face, which is what every ODF chart states.</summary>
    [Fact]
    public void AnUnstatedLegendFaceIsTheChartsOwn()
    {
        ChartDrawing drawing =
            ChartLayout.Place(Two() with { TextFamily = "Wide" }, Frame, new Ruler());

        drawing.Labels.ShouldAllBe(label => label.Family == "Wide");
    }

    // ------------------------------------------------------------------ the order

    [Theory]
    // A horizontal bar chart reverses unless it stacks in Y.
    [InlineData(ChartPlotKind.Bar, ChartBarDirection.Bar, false, ChartLegendPosition.Right, true)]
    [InlineData(ChartPlotKind.Bar, ChartBarDirection.Bar, true, ChartLegendPosition.Right, false)]
    // …and it reverses beside a top or bottom legend too, because the swap decides it.
    [InlineData(ChartPlotKind.Bar, ChartBarDirection.Bar, false, ChartLegendPosition.Bottom, true)]
    // An unswapped chart reverses only when it stacks in Y and the legend is at a side.
    [InlineData(ChartPlotKind.Bar, ChartBarDirection.Column, true, ChartLegendPosition.Right, true)]
    [InlineData(ChartPlotKind.Bar, ChartBarDirection.Column, false, ChartLegendPosition.Right, false)]
    [InlineData(ChartPlotKind.Area, ChartBarDirection.Column, true, ChartLegendPosition.Left, true)]
    [InlineData(ChartPlotKind.Area, ChartBarDirection.Column, false, ChartLegendPosition.Right, false)]
    // A top or bottom legend on an unswapped chart never reverses, stacked or not.
    [InlineData(ChartPlotKind.Bar, ChartBarDirection.Column, true, ChartLegendPosition.Top, false)]
    [InlineData(ChartPlotKind.Area, ChartBarDirection.Column, true, ChartLegendPosition.Bottom, false)]
    public void TheReversalRule(
        ChartPlotKind kind,
        ChartBarDirection direction,
        bool stacked,
        ChartLegendPosition legend,
        bool reversed)
    {
        ChartPlot plot = Two(kind) with
        {
            Direction = direction,
            IsStacked = stacked,
            Legend = legend,
        };

        plot.LegendReversed.ShouldBe(reversed);
        LegendNames(ChartLayout.Place(plot, Frame, new Ruler()))
            .ShouldBe(reversed ? ["Plan", "Actual"] : ["Actual", "Plan"]);
    }

    /// <summary>
    /// A percent-stacked chart stacks in Y as surely as a stacked one does.
    /// </summary>
    [Fact]
    public void PercentStackingCountsAsStacking()
        => (Two() with { Direction = ChartBarDirection.Column, IsPercentStacked = true })
           .LegendReversed.ShouldBeTrue();

    /// <summary>
    /// <c>Direction</c> defaults to <c>Column</c>, so a chart that is not a bar chart at all
    /// cannot be swapped — the property exists for the bar reader and nothing else sets it.
    /// </summary>
    [Fact]
    public void OnlyABarChartCanBeSwapped()
        => (Two(ChartPlotKind.Line) with { Direction = ChartBarDirection.Bar })
           .LegendReversed.ShouldBeFalse();

    /// <summary>
    /// A combination chart whose bar group runs horizontally is swapped, because the coordinate
    /// system is one for the whole diagram.
    /// </summary>
    [Fact]
    public void ACombinationWithAHorizontalBarGroupIsSwapped()
    {
        ChartPlot plot = new()
        {
            Kind = ChartPlotKind.Line,
            Direction = ChartBarDirection.Bar,
            Categories = ["Q1", "Q2"],
            Series =
            [
                new ChartSeries("Actual", [1.0, 2.0], Colour.FromRgb(0xC0504D)),
                new ChartSeries("Plan", [2.0, 1.0], Colour.FromRgb(0x4F81BD))
                    { Kind = ChartPlotKind.Bar },
            ],
            Legend = ChartLegendPosition.Right,
        };

        plot.LegendReversed.ShouldBeTrue();
    }

    /// <summary>
    /// A pie's legend names its categories and never reverses: the rule reads a stacking
    /// direction and a swapped coordinate system and a pie has neither.
    /// </summary>
    [Fact]
    public void APiesLegendKeepsItsCategoryOrder()
    {
        ChartPlot pie = new()
        {
            Kind = ChartPlotKind.Pie,
            Categories = ["M1", "M2", "M3"],
            Series = [new ChartSeries("Sales", [1.0, 2.0, 3.0], Colour.FromRgb(0xC0504D))],
            Legend = ChartLegendPosition.Right,
        };

        pie.LegendReversed.ShouldBeFalse();

        List<string> names = [];
        foreach (ChartLabel label in ChartLayout.Place(pie, Frame, new Ruler()).Labels)
            if (label.Text is "M1" or "M2" or "M3" && !names.Contains(label.Text))
                names.Add(label.Text);

        names.ShouldBe(["M1", "M2", "M3"]);
    }
}
