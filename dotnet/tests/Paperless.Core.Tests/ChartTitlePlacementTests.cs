using Paperless.Core.Charts;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Shouldly;

namespace Paperless.Core.Tests;

/// <summary>
/// Where a chart's three titles go: the main one, and the two the axes carry.
/// </summary>
/// <remarks>
/// <para>
/// Both rules here are about a title being in the band that was already reserved for it, which is
/// the failure mode that looks least like a bug: the room is taken off the plot area either way,
/// so the picture stays the right size and the words sit in the wrong place.
/// </para>
/// <para>
/// Found on <c>2012-GA-Survey-Chapter-5-Tables-16Dec2013-V2.xls</c>, a horizontal bar chart with a
/// two-line title. Both defects are reachable from every family — a DrawingML title states a second
/// <c>a:p</c>, and a bar chart is a bar chart wherever it was read from — so they are asserted on
/// the model rather than on any one format.
/// </para>
/// </remarks>
public class ChartTitlePlacementTests
{
    /// <summary>Half an em per character, 1.15 em a line — Liberation Sans to three places.</summary>
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size)
            => new(size * (0.5 * text.Length), size * 1.15);
    }

    private static readonly DocRect Frame =
        new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300));

    private static ChartDrawing Place(ChartPlot plot) => ChartLayout.Place(plot, Frame, new Ruler());

    private static ChartPlot Bars() => new()
    {
        Categories = ["Q1", "Q2", "Q3", "Q4"],
        Series = [new ChartSeries("North", [120.0, 95.0, 143.0, 168.0], Colour.FromRgb(0x99CCFF))],
        CategoryAxisTitle = "Aircraft Type",
        ValueAxisTitle = "Gallons Per Hour",
    };

    /// <summary>
    /// A title stating its own line break is drawn as two lines and reserves room for both.
    /// </summary>
    /// <remarks>
    /// The break is part of the title rather than wrapping the engine chose — Excel's BIFF
    /// <c>CHSTRING</c> writes it as a newline inside one string — so a measurer handed the whole
    /// thing reports one line, the plot area starts a line too high, and the second line is drawn
    /// over the first.
    /// </remarks>
    [Fact]
    public void ATitleWithALineBreakIsDrawnAsTwoLines()
    {
        ChartDrawing one = Place(Bars() with { Title = "Chart 8" });
        ChartDrawing two = Place(Bars() with { Title = "Chart 8\n2012 Average Fuel Consumption" });

        two.Labels.ShouldContain(label => label.Text == "Chart 8");
        two.Labels.ShouldContain(label => label.Text == "2012 Average Fuel Consumption");
        two.Labels.ShouldNotContain(label => label.Text.Contains('\n', StringComparison.Ordinal));

        ChartLabel first = two.Labels.First(label => label.Text == "Chart 8");
        ChartLabel second = two.Labels.First(label => label.Text.StartsWith("2012", StringComparison.Ordinal));
        second.At.Y.ShouldBeGreaterThan(first.At.Y, "the second line is under the first");

        two.PlotArea.Y.ShouldBeGreaterThan(one.PlotArea.Y, "and the extra line is reserved for");
    }

    /// <summary>
    /// A bar chart's titles swap places, because the axes do.
    /// </summary>
    /// <remarks>
    /// Which title goes where follows the direction rather than the role:
    /// <c>TITLE_AT_STANDARD_X_AXIS_POSITION</c> is always <c>ALIGN_BOTTOM</c>, so whatever runs
    /// horizontally is titled underneath. The room was already reserved that way — the same
    /// <c>columns</c> test picks <c>beside</c> and <c>below</c> when the plot area is composed —
    /// so drawing them the other way round put each title in the other's band.
    /// </remarks>
    [Fact]
    public void ABarChartTitlesTheValueAxisUnderneath()
    {
        ChartDrawing columns = Place(Bars() with { Direction = ChartBarDirection.Column });
        ChartDrawing bars = Place(Bars() with { Direction = ChartBarDirection.Bar });

        ChartLabel Label(ChartDrawing drawing, string text)
            => drawing.Labels.First(label => label.Text == text);

        // On a column chart the categories run along the bottom and the values up the side.
        Label(columns, "Aircraft Type").Rotation.ShouldBe(0.0);
        Label(columns, "Gallons Per Hour").Rotation.ShouldNotBe(0.0);

        // Turned on its side, the two exchange both their positions and their rotations.
        Label(bars, "Aircraft Type").Rotation.ShouldNotBe(0.0);
        Label(bars, "Gallons Per Hour").Rotation.ShouldBe(0.0);

        Label(bars, "Gallons Per Hour").At.Y
            .ShouldBe(Label(columns, "Aircraft Type").At.Y, "the band under the plot area");
        Label(bars, "Aircraft Type").At.X
            .ShouldBe(Label(columns, "Gallons Per Hour").At.X, "the band beside it");
    }
}
