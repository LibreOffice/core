using Paperless.Core.Charts;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Shouldly;

namespace Paperless.Core.Tests;

/// <summary>
/// The table of numbers under the plot, and the two things it changes about everything else.
/// </summary>
/// <remarks>
/// A data table is not an ornament: it takes its room out of the plot rectangle, and it takes the
/// category axis' labels away entirely. Both are easy to leave out and neither shows as an error —
/// the first draws a table over the category labels, the second draws every category name twice.
/// </remarks>
public class ChartDataTableTests
{
    /// <summary>Half an em per character, 1.15 em a line.</summary>
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size)
            => new(size * (0.5 * text.Length), size * 1.15);
    }

    private static readonly DocRect Frame =
        new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300));

    private static ChartPlot Plot() => new()
    {
        Categories = ["Q1", "Q2", "Q3"],
        Series =
        [
            new ChartSeries("Banana", [120.0, 95.0, 143.0], Colour.FromRgb(0x99CCFF)),
            new ChartSeries("Pineapple", [80.0, 130.0, 60.0], Colour.FromRgb(0xFFCC99)),
        ],
    };

    private static readonly ChartDataTable Table =
        new(HorizontalBorders: true, VerticalBorders: true, Outline: true, Keys: true,
            Colour.FromRgb(0xB3B3B3));

    /// <summary>
    /// The table draws a cell per value, a row header per series and the category names once.
    /// </summary>
    /// <remarks>
    /// Measured on <c>chart2/qa/extras/data/pptx/tdf137691_dataTable.pptx</c>, whose reference
    /// draws two series names, five category names and ten values that our render had none of:
    /// its total word count went from 68 against 80 to 80 against 80.
    /// </remarks>
    [Fact]
    public void ADataTableDrawsEveryValueAndEverySeriesName()
    {
        ChartDrawing drawing = ChartLayout.Place(
            Plot() with { DataTable = Table }, Frame, new Ruler());

        drawing.Labels.ShouldContain(label => label.Text == "Banana");
        drawing.Labels.ShouldContain(label => label.Text == "Pineapple");

        foreach (string value in new[] { "120", "95", "143", "80", "130", "60" })
            drawing.Labels.ShouldContain(label => label.Text == value);

        foreach (string category in new[] { "Q1", "Q2", "Q3" })
            drawing.Labels.Count(label => label.Text == category).ShouldBe(1);
    }

    /// <summary>
    /// The category axis stops labelling itself, because the table's header row is its labels.
    /// </summary>
    /// <remarks>
    /// <c>VAxisProperties.cxx:336-343</c> sets <c>m_bDisplayLabels = false</c> whenever a data
    /// table is present on dimension 0. Without it the names appear twice — once under the axis
    /// and once in the header — which reads as a duplicated series rather than as a layout rule.
    /// </remarks>
    [Fact]
    public void TheCategoryAxisDrawsNoLabelsOfItsOwn()
    {
        ChartDrawing without = ChartLayout.Place(Plot(), Frame, new Ruler());
        ChartDrawing with = ChartLayout.Place(
            Plot() with { DataTable = Table }, Frame, new Ruler());

        // Above the plot's bottom edge is the axis' own row; below it is the table.
        without.Labels.ShouldContain(
            label => label.Text == "Q1" && label.At.Y > without.PlotArea.Bottom);

        with.Labels.Count(label => label.Text == "Q1").ShouldBe(1);
    }

    /// <summary>The plot rectangle gives up the table's height.</summary>
    /// <remarks>
    /// One row per series plus a header row, each a text shape tall. On the corpus deck that is
    /// three rows against one line of category labels, which is what moves the value axis' tick
    /// count as well as the bars.
    /// </remarks>
    [Fact]
    public void ThePlotAreaShrinksByTheTableSHeight()
    {
        ChartDrawing without = ChartLayout.Place(Plot(), Frame, new Ruler());
        ChartDrawing with = ChartLayout.Place(
            Plot() with { DataTable = Table }, Frame, new Ruler());

        with.PlotArea.Bottom.ShouldBeLessThan(without.PlotArea.Bottom);

        Length row = Length.FromPoints(10) * 1.15 + Length.FromPoints(10) * 0.6;
        (without.PlotArea.Bottom - with.PlotArea.Bottom).Emu
            .ShouldBeGreaterThan((row * 2).Emu);
    }

    /// <summary>
    /// A table that states no borders draws none.
    /// </summary>
    /// <remarks>
    /// All four <c>c:dTable</c> flags default to false, unlike the <c>c:show*</c> family beside
    /// them — <c>DataTableModel</c> initialises each to <c>false</c> outright rather than to
    /// <c>!bMSO2007Doc</c>. Carrying the data-label rule across draws a grid on every table that
    /// asks for none.
    /// </remarks>
    [Fact]
    public void ATableWithNoBordersDrawsNoLines()
    {
        ChartPlot plot = Plot() with
        {
            DataTable = new ChartDataTable(false, false, false, false, Colour.FromRgb(0xB3B3B3)),
        };

        ChartDrawing bare = ChartLayout.Place(plot, Frame, new Ruler());
        ChartDrawing ruled = ChartLayout.Place(
            Plot() with { DataTable = Table }, Frame, new Ruler());

        bare.Lines.Count.ShouldBeLessThan(ruled.Lines.Count);
        bare.Labels.ShouldContain(label => label.Text == "Banana");
    }
}
