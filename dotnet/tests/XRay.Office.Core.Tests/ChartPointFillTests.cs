using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// A bar's colour is the point's where the file states one, and the series' where it does not.
/// </summary>
/// <remarks>
/// <para>
/// <strong>A pie's per-point fills were honoured from the start and a bar's were not.</strong>
/// <see cref="ChartSeries.PointFills"/> has always carried what <c>c:dPt/c:spPr</c> states, and
/// <c>AddWedges</c>, <c>AddAreas</c>, <c>AddBubbles</c> and the label builders all asked
/// <see cref="ChartSeries.FillAt"/> for it — <c>AddBars</c> alone asked
/// <see cref="ChartSeries.Fill"/>, so every column and every horizontal bar in the corpus was
/// drawn in one flat colour whatever the file said.
/// </para>
/// <para>
/// <strong>Reach: 33 non-pie plot groups in 13 corpus documents state a per-point fill</strong>,
/// thirty of them bar groups, and <em>no</em> non-pie group states <c>c:varyColors val="1"</c> —
/// so this can only put a stated colour where a stated colour belongs. Measured on
/// <c>029_Annual_budget</c>, whose two columns state <c>tx2</c> at <c>lumMod</c> 60/40 and 20/80
/// and were drawn in the theme's <c>accent1</c>: the reference's <c>#5C64B7</c> and
/// <c>#C9CBE7</c> against our <c>#4A66AC</c> twice. See <c>probes/chart-secaxis/dpt-census.py</c>.
/// </para>
/// </remarks>
public sealed class ChartPointFillTests
{
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
            => new(size * (0.5 * text.Length), size * 1.15);
    }

    private static readonly DocRect Frame =
        new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300));

    private static readonly Colour Series = Colour.FromRgb(0x4A66AC);
    private static readonly Colour First = Colour.FromRgb(0x5C64B7);
    private static readonly Colour Second = Colour.FromRgb(0xC9CBE7);

    private static ChartPlot Columns(IReadOnlyList<Colour?>? points) => new()
    {
        Categories = ["Income", "Expenses"],
        Series =
        [
            new ChartSeries("ChartData", [4000.0, 2476.0], Series, PointFills: points),
        ],
    };

    /// <summary>Each column takes the colour its own point states.</summary>
    [Fact]
    public void EachBarTakesItsOwnPointsFill()
    {
        ChartDrawing drawing = ChartLayout.Place(
            Columns([First, Second]), Frame, new Ruler());

        List<Colour?> fills = [.. drawing.Shapes.Select(shape => shape.Fill)];

        fills.ShouldContain(First);
        fills.ShouldContain(Second);
        fills.ShouldNotContain(Series);
    }

    /// <summary>A point the file says nothing about keeps the series' own colour.</summary>
    /// <remarks>
    /// The control that keeps the change from being a rewrite: <c>FillAt</c> falls back to
    /// <see cref="ChartSeries.Fill"/> for a null entry, so a chart with one <c>c:dPt</c> among
    /// twenty points draws nineteen exactly as before.
    /// </remarks>
    [Fact]
    public void APointStatingNothingKeepsTheSeriesFill()
    {
        ChartDrawing drawing = ChartLayout.Place(
            Columns([First, null]), Frame, new Ruler());

        List<Colour?> fills = [.. drawing.Shapes.Select(shape => shape.Fill)];

        fills.ShouldContain(First);
        fills.ShouldContain(Series);
        fills.ShouldNotContain(Second);
    }

    /// <summary>A series with no per-point fills at all is drawn exactly as before.</summary>
    [Fact]
    public void ASeriesWithNoPointFillsIsAllOneColour()
    {
        ChartDrawing drawing = ChartLayout.Place(Columns(null), Frame, new Ruler());

        drawing.Shapes
            .Where(shape => shape.Fill is not null)
            .ShouldAllBe(shape => shape.Fill == Series);
    }
}
