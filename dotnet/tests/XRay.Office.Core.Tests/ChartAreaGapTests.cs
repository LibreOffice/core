using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// What an area series does with a category that has no value.
/// </summary>
/// <remarks>
/// <para>
/// <c>AreaChart::createShapes</c> (<c>chart2/source/view/charttypes/AreaChart.cxx:691-706</c>)
/// <c>continue</c>s past a NaN, contributing no vertex, and under the default <c>LEAVE_GAP</c>
/// treatment advances the polygon index first — so the series is one polygon per run of
/// consecutive real points.
/// </para>
/// <para>
/// Plotting the gap as zero instead is not a small error. Measured on
/// <c>Template Pilot Logbook JAR-FCL V3.0.xls</c>, 615 declared points of which 17 carry a value:
/// with the gaps zeroed our fill was <c>(153.00, 155.89)-(603.95, 190.34)</c>, a 451 pt rectangle
/// lying on the axis, against the reference's <c>(599.73, 167.67)-(639.21, 201.12)</c>.
/// </para>
/// </remarks>
public class ChartAreaGapTests
{
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
            => new(size * (0.5 * text.Length), size * 1.15);
    }

    private static readonly DocRect Frame = new(
        Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300));

    private static ChartDrawing Lay(params double?[] values)
        => ChartLayout.Place(
            new ChartPlot
            {
                Kind = ChartPlotKind.Area,
                Categories = [.. values.Select((_, at) => (string?)$"c{at}")],
                Series = [new ChartSeries("s", values, Colour.FromRgb(0xFF0000), null)],
            },
            Frame,
            new Ruler());

    /// <summary>Every point present: one polygon, as before.</summary>
    /// <remarks>
    /// The control. Nearly every area chart is this, and the gap handling must not cost it a
    /// vertex or split it.
    /// </remarks>
    [Fact]
    public void AnUnbrokenSeriesIsOnePolygon()
    {
        IReadOnlyList<ChartShape> filled = Filled(Lay(1.0, 2.0, 3.0, 4.0));

        filled.Count.ShouldBe(1);
        filled[0].Path.Commands.Count.ShouldBe(9);
    }

    /// <summary>A gap in the middle splits the series into two polygons.</summary>
    /// <remarks>
    /// Two, not one with a notch: <c>LEAVE_GAP</c> starts a new polygon at the gap. A reader that
    /// merely skipped the vertex would draw one polygon whose upper edge jumped the gap, which is
    /// the <c>CONTINUE</c> treatment and a different picture.
    /// </remarks>
    [Fact]
    public void AGapSplitsTheSeriesIntoSeparatePolygons()
    {
        Filled(Lay(1.0, 2.0, null, 3.0, 4.0)).Count.ShouldBe(2);
    }

    /// <summary>A missing point is not drawn as a zero.</summary>
    /// <remarks>
    /// The case the corpus document is: one real point in a long run of blanks. Zeroing the blanks
    /// stretches the polygon across the whole plot at the baseline; skipping them leaves a run of
    /// one, which has no area and is not filled at all.
    /// </remarks>
    [Fact]
    public void AMissingPointDoesNotPinThePolygonToTheBaseline()
    {
        Filled(Lay(null, null, null, 5.0, null, null, null)).ShouldBeEmpty();
    }

    /// <summary>
    /// The polygon spans only the categories that have values, not the whole plot.
    /// </summary>
    /// <remarks>
    /// Stated as a width rather than as a vertex count because the width is what the defect was
    /// visible as, and because it fails whether the gap is zeroed or merely bridged.
    /// </remarks>
    [Fact]
    public void ATrailingRunIsDrawnWhereItIsAndNotAcrossThePlot()
    {
        IReadOnlyList<ChartShape> filled =
            Filled(Lay(null, null, null, null, null, null, 3.0, 4.0));

        filled.Count.ShouldBe(1);

        // The close command carries no point, so it would otherwise contribute an x of zero.
        Length left = filled[0].Path.Commands
            .Where(command => command.Verb != PathVerb.Close)
            .Min(command => command.Point.X);

        left.ShouldBeGreaterThan(Frame.Width / 2);
    }

    private static IReadOnlyList<ChartShape> Filled(ChartDrawing laid)
        => [.. laid.Shapes.Where(shape => shape.Fill == Colour.FromRgb(0xFF0000))];
}
