using Paperless.Core.Charts;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Shouldly;

namespace Paperless.Core.Tests;

/// <summary>
/// A stated weight on the axis labels or the legend reaches every label of the chart, and the
/// measurement that places them.
/// </summary>
/// <remarks>
/// <para>
/// The counterpart of <see cref="ChartTitleWeightTests"/>, which covers the weights the OOXML
/// auto-text table sets by itself. This covers the one a file states: <c>b="1"</c> on
/// <c>c:valAx/c:txPr</c> is a statement about the axis' <em>labels</em>, and 36 of the slides
/// corpus's 61 chart parts state a weight somewhere.
/// </para>
/// <para>
/// <strong>The geometric assertions are the load-bearing ones.</strong> The flag on a label only
/// changes which face a consumer draws it in; the weight reaching
/// the plot rectangle's own measurements is what moves the plot rectangle, and a
/// stamping pass that filled the labels while leaving the reservations regular would satisfy
/// every flag assertion here and still put every mark in the wrong place.
/// </para>
/// </remarks>
public class ChartLabelWeightTests
{
    /// <summary>Half an em per character, a tenth wider bold — so a dropped weight is geometry.</summary>
    private sealed class Ruler : IChartTextMeasurer
    {
        public List<(string Text, bool Bold)> Weights { get; } = [];

        public DocSize Measure(string text, Length size, string? family, bool bold)
        {
            Weights.Add((text, bold));
            return new DocSize(size * (0.5 * text.Length) * (bold ? 1.1 : 1.0), size * 1.15);
        }
    }

    private static readonly DocRect Frame =
        new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300));

    private static ChartPlot Bars() => new()
    {
        Categories = ["Q1", "Q2"],
        Series = [new ChartSeries("Northern region", [120.0, 95.0], Colour.FromRgb(0x99CCFF))],
        Title = "Sales",
        Legend = ChartLegendPosition.Right,
    };

    /// <summary>An unstated weight leaves the axis labels regular in the drawing.</summary>
    [Fact]
    public void RegularAxisLabelsStayRegular()
    {
        ChartDrawing drawing = ChartLayout.Place(Bars(), Frame, new Ruler());

        drawing.Labels.First(label => label.Text == "Q1").IsBold.ShouldBe(false);
    }

    /// <summary>A stated weight reaches the category labels, which no site passes it to.</summary>
    /// <remarks>
    /// The whole point of the stamping pass: <c>AddCategoryAxis</c> constructs its labels without
    /// mentioning a weight at all.
    /// </remarks>
    [Fact]
    public void AStatedWeightReachesTheCategoryLabels()
    {
        ChartDrawing drawing =
            ChartLayout.Place(Bars() with { IsLabelBold = true }, Frame, new Ruler());

        drawing.Labels.First(label => label.Text == "Q1").IsBold.ShouldBe(true);
    }

    /// <summary>And the value labels, which are constructed at a different site.</summary>
    [Fact]
    public void AStatedWeightReachesTheValueLabels()
    {
        ChartDrawing drawing =
            ChartLayout.Place(Bars() with { IsLabelBold = true }, Frame, new Ruler());

        drawing.Labels.Where(label => label.Text is not ("Sales" or "Q1" or "Q2"))
            .ShouldAllBe(label => label.IsBold == true);
    }

    /// <summary>The main title keeps its own weight rather than taking the labels'.</summary>
    [Fact]
    public void TheTitleKeepsItsOwnWeight()
    {
        ChartDrawing drawing = ChartLayout.Place(
            Bars() with { IsLabelBold = true, IsTitleBold = false }, Frame, new Ruler());

        drawing.Labels.First(label => label.Text == "Sales").IsBold.ShouldBe(false);
    }

    /// <summary>A legend with no weight of its own reads as the axis labels do.</summary>
    [Fact]
    public void TheLegendFollowsTheLabelsWhenItStatesNothing()
    {
        ChartDrawing drawing =
            ChartLayout.Place(Bars() with { IsLabelBold = true }, Frame, new Ruler());

        drawing.Labels.First(label => label.Text == "Northern region").IsBold.ShouldBe(true);
    }

    /// <summary>A legend that states its own weight keeps it against bold axis labels.</summary>
    [Fact]
    public void TheLegendKeepsItsOwnWeight()
    {
        ChartDrawing drawing = ChartLayout.Place(
            Bars() with { IsLabelBold = true, IsLegendBold = false }, Frame, new Ruler());

        drawing.Labels.First(label => label.Text == "Northern region").IsBold.ShouldBe(false);
    }

    /// <summary>
    /// The weight reaches the value labels' <em>measurement</em>, so the plot rectangle's left
    /// edge moves — which is the half of this that a flag assertion cannot see.
    /// </summary>
    [Fact]
    public void ABoldValueLabelWidensTheLeftReservation()
    {
        ChartDrawing regular = ChartLayout.Place(Bars(), Frame, new Ruler());
        ChartDrawing bold =
            ChartLayout.Place(Bars() with { IsLabelBold = true }, Frame, new Ruler());

        bold.PlotArea.Left.ShouldBeGreaterThan(regular.PlotArea.Left);
    }

    /// <summary>
    /// And the legend's, so its column is wider and the plot rectangle's right edge comes in.
    /// </summary>
    [Fact]
    public void ABoldLegendNarrowsThePlotRectangle()
    {
        ChartDrawing regular = ChartLayout.Place(Bars(), Frame, new Ruler());
        ChartDrawing bold =
            ChartLayout.Place(Bars() with { IsLegendBold = true }, Frame, new Ruler());

        bold.PlotArea.Right.ShouldBeLessThan(regular.PlotArea.Right);
    }

    /// <summary>The measurer is asked for the labels in the face they are drawn in.</summary>
    /// <remarks>
    /// A consumer places a label from the width it measures, so a weight that reaches the model
    /// and not the measurer draws bold text at regular spacing.
    /// </remarks>
    [Fact]
    public void TheMeasurerIsToldTheLabelWeight()
    {
        Ruler ruler = new();
        ChartLayout.Place(Bars() with { IsLabelBold = true }, Frame, ruler);

        ruler.Weights.ShouldContain(("Q1", true));
        ruler.Weights.ShouldNotContain(("Q1", false));
    }
}
