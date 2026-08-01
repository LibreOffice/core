using Paperless.Core.Charts;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Shouldly;

namespace Paperless.Core.Tests;

/// <summary>
/// A doughnut is a pie of concentric rings, and the hole's size does not come from the file.
/// </summary>
/// <remarks>
/// <c>PieChart</c>'s constructor sets <c>m_fRadiusOffset = 1.0</c> for a ring chart and nothing
/// else (<c>chart2/source/view/charttypes/PieChart.cxx:212-216</c>), which puts ring <em>k</em>
/// of <em>n</em> between <c>k/(n+1)</c> and <c>(k+1)/(n+1)</c> of the outer radius — so a
/// single-ring doughnut has a hole at exactly half. OOXML's <c>c:holeSize</c> is parsed into
/// <c>mnHoleSize</c> and then read by nothing at all, exactly as <c>c:bubbleScale</c> is, so
/// honouring it would be a disagreement with the reference rather than a refinement of it.
/// </remarks>
public class ChartDoughnutLayoutTests
{
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size)
            => new(size * (0.5 * text.Length), size * 1.15);
    }

    private static readonly DocRect Frame =
        new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300));

    private static ChartPlot Pie(bool rings, int series) => new()
    {
        Kind = ChartPlotKind.Pie,
        Rings = rings,
        Categories = ["A", "B", "C", "D"],
        Series = [.. Enumerable.Range(0, series).Select(at =>
            new ChartSeries($"S{at}", [1.0, 2.0, 3.0, 4.0], Colour.FromRgb(0x99CCFF)))],
    };

    /// <summary>
    /// A pie draws one wedge per point and a doughnut draws a ring per <em>series</em>, so a
    /// two-series doughnut has twice a pie's paths.
    /// </summary>
    /// <remarks>
    /// Before <see cref="ChartPlot.Rings"/> existed, a doughnut was drawn as a pie: the first
    /// series became a filled disc and every later one was not drawn at all. The picture was a
    /// plausible pie of the right colours, which is why it survived three chart merges.
    /// </remarks>
    [Fact]
    public void ADoughnutDrawsARingPerSeriesWhereAPieDrawsTheFirstSeriesAlone()
    {
        ChartLayout.Place(Pie(false, 2), Frame, new Ruler()).Shapes.Count.ShouldBe(4);
        ChartLayout.Place(Pie(true, 2), Frame, new Ruler()).Shapes.Count.ShouldBe(8);
    }

    /// <summary>
    /// A single-ring doughnut's hole is exactly half the outer radius, and no point of any wedge
    /// falls inside it.
    /// </summary>
    [Fact]
    public void ASingleRingDoughnutHasAHoleAtHalfTheRadius()
    {
        ChartDrawing drawing = ChartLayout.Place(Pie(true, 1), Frame, new Ruler());

        DocRect area = drawing.PlotArea;
        DocPoint centre = new(area.X + (area.Width / 2), area.Y + (area.Height / 2));
        double outer = (area.Width < area.Height ? area.Width : area.Height).Points / 2;

        double nearest = double.MaxValue;

        foreach (ChartShape shape in drawing.Shapes)
        {
            foreach (PathCommand command in shape.Path.Commands)
            {
                if (command.Verb == PathVerb.Close) continue;

                double dx = command.Point.X.Points - centre.X.Points;
                double dy = command.Point.Y.Points - centre.Y.Points;
                nearest = Math.Min(nearest, Math.Sqrt((dx * dx) + (dy * dy)));
            }
        }

        // The hole is half the radius; nothing is drawn nearer the centre than that, and a pie
        // would have put every wedge's apex at zero.
        nearest.ShouldBeInRange((outer / 2) - 0.5, (outer / 2) + 0.5);
    }
}
