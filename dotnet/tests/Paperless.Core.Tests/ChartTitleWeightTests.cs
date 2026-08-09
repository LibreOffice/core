using Paperless.Core.Charts;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Shouldly;

namespace Paperless.Core.Tests;

/// <summary>
/// A chart's titles are the only text it draws bold, and the weight reaches the measurement as
/// well as the label.
/// </summary>
/// <remarks>
/// <para>
/// The composition has to carry a weight at all because an OOXML chart's automatic text makes
/// the main title and the axis titles bold and leaves everything else regular
/// (<c>oox/source/drawingml/chart/objectformatter.cxx</c>:415-434).
/// </para>
/// <para>
/// <strong>Both the flag on the label and the flag the measurer was asked with are asserted</strong>,
/// because a consumer centres a title on the width it measures: <c>SlideChart</c> measures the
/// label to place its corner, and a bold title measured in the regular face lands half the
/// difference to the right. The layout's own reservations use the line height rather than the
/// width, so a measurer that is not told the weight goes wrong at the consumer and not here —
/// which is exactly the kind of defect that survives a layout test.
/// </para>
/// <para>
/// Asserted on the model rather than on any one format: the two readers disagree about the
/// <em>default</em> weight and agree about everything that follows from it.
/// </para>
/// </remarks>
public class ChartTitleWeightTests
{
    /// <summary>
    /// Half an em per character and 1.15 em a line, a bold line a tenth wider — which is what
    /// makes a dropped weight visible in the geometry rather than only in <see cref="Weights"/>.
    /// </summary>
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
        Series = [new ChartSeries("North", [120.0, 95.0], Colour.FromRgb(0x99CCFF))],
        Title = "Sales",
        CategoryAxisTitle = "Quarter",
        ValueAxisTitle = "Gallons",
        Legend = ChartLegendPosition.None,
    };

    /// <summary>Nothing is bold when the model says nothing is.</summary>
    [Fact]
    public void ARegularChartDrawsNoBoldLabel()
    {
        ChartDrawing drawing = ChartLayout.Place(Bars(), Frame, new Ruler());

        drawing.Labels.ShouldAllBe(label => label.IsBold != true);
    }

    /// <summary>The titles carry the weight and the categories do not.</summary>
    [Fact]
    public void OnlyTheTitlesAreBold()
    {
        ChartDrawing drawing = ChartLayout.Place(
            Bars() with { IsTitleBold = true, IsAxisTitleBold = true }, Frame, new Ruler());

        drawing.Labels.First(label => label.Text == "Sales").IsBold.ShouldBe(true);
        drawing.Labels.First(label => label.Text == "Quarter").IsBold.ShouldBe(true);
        drawing.Labels.First(label => label.Text == "Gallons").IsBold.ShouldBe(true);
        drawing.Labels.First(label => label.Text == "Q1").IsBold.ShouldBe(false);
    }

    /// <summary>The two weights are independent, because a file may state one and not the other.</summary>
    [Fact]
    public void ABoldMainTitleDoesNotMakeTheAxisTitlesBold()
    {
        ChartDrawing drawing = ChartLayout.Place(
            Bars() with { IsTitleBold = true }, Frame, new Ruler());

        drawing.Labels.First(label => label.Text == "Sales").IsBold.ShouldBe(true);
        drawing.Labels.First(label => label.Text == "Quarter").IsBold.ShouldBe(false);
    }

    /// <summary>
    /// The measurer is asked for the title in the weight the title is drawn in.
    /// </summary>
    /// <remarks>
    /// This is the half that a flag on the label alone would not give: the title's own width is
    /// what centres it, and the axis title's line height is what reserves room for it, so a
    /// measurement taken in the regular face misplaces text that is drawn correctly.
    /// </remarks>
    [Fact]
    public void TheMeasurerIsAskedInTheWeightTheTextIsDrawnIn()
    {
        Ruler ruler = new();
        ChartLayout.Place(
            Bars() with { IsTitleBold = true, IsAxisTitleBold = true }, Frame, ruler);

        // Asserted as a presence and an absence rather than with ShouldAllBe, which passes
        // vacuously on an empty sequence — a title that was never measured at all would satisfy
        // every "all of them are bold" predicate there is.
        ruler.Weights.ShouldContain(w => w.Text == "Sales" && w.Bold);
        ruler.Weights.ShouldContain(w => w.Text == "Quarter" && w.Bold);
        ruler.Weights.ShouldContain(w => w.Text == "Gallons" && w.Bold);
        ruler.Weights.ShouldNotContain(w => w.Text == "Sales" && !w.Bold);
        ruler.Weights.ShouldNotContain(w => w.Text == "Quarter" && !w.Bold);
        ruler.Weights.ShouldNotContain(w => w.Text == "Gallons" && !w.Bold);
        ruler.Weights.ShouldContain(w => w.Text == "Q1" && !w.Bold);
    }
}
