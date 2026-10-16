using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// An unstacked area chart paints its <em>first</em> series last, over the others.
/// </summary>
/// <remarks>
/// <para>
/// <c>AreaChart::createShapes</c> reverses its own slot list before it draws anything —
/// <c>lcl_reorderSeries(m_aZSlots)</c> under
/// <c>m_nDimension == 2 &amp;&amp; (m_bArea || !m_bCategoryXAxis)</c>
/// (<c>chart2/source/view/charttypes/AreaChart.cxx:565-568</c>), which is tdf#127813's switch —
/// so series 1 ends up on top of the pile rather than under it.
/// </para>
/// <para>
/// <strong>Found by a page reading and then measured.</strong> A reviewer given only the composed
/// halves of <c>006_advanced_powerpoint_area.pptx</c> page 1 ranked "the dominant colour of the
/// area chart flips from blue to red" as the loudest difference on the page, and said the two
/// silhouettes and the crossing point are identical — which is what says it is a paint order and
/// not a value. Reversing the emission takes that page from
/// <c>diff% 18.67, |ink|% 1.54, MAJOR</c> to <c>diff% 0.84, |ink|% 0.03, ok</c>.
/// </para>
/// <para>
/// <strong>A stacked area is exempt and that is a measurement, not the source.</strong>
/// <c>m_bArea</c> does not exempt it, but <c>stacked_area_chart.pptx</c> is
/// <c>diff% 1.82, |ink|% 0.16</c> in file order and <c>1.87 / 0.22</c> reversed: its bands abut
/// rather than nest, so the shared edge belongs to whichever polygon is drawn last, and the
/// reference draws them in file order.
/// </para>
/// </remarks>
public class ChartAreaPaintOrderTests
{
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
            => new(size * (0.5 * text.Length) * (bold ? 1.1 : 1.0), size * 1.15);
    }

    private static readonly DocRect Frame =
        new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300));

    private static readonly Colour First = Colour.FromRgb(0xC0504D);
    private static readonly Colour Second = Colour.FromRgb(0x4F81BD);

    private static ChartPlot Areas(bool stacked) => new()
    {
        Kind = ChartPlotKind.Area,
        IsStacked = stacked,
        Categories = ["M1", "M2", "M3", "M4"],
        Series =
        [
            new ChartSeries("Actual", [10.0, 20.0, 30.0, 40.0], First)
            {
                Kind = ChartPlotKind.Area,
            },
            new ChartSeries("Plan", [40.0, 30.0, 20.0, 10.0], Second)
            {
                Kind = ChartPlotKind.Area,
            },
        ],
    };

    private static List<Colour> FillOrder(ChartPlot plot)
    {
        ChartDrawing drawing = ChartLayout.Place(plot, Frame, new Ruler());
        List<Colour> order = [];

        foreach (ChartShape shape in drawing.Shapes)
        {
            if (shape.Fill is not { } fill) continue;
            if (fill != First && fill != Second) continue;
            if (order.Count > 0 && order[^1] == fill) continue;

            order.Add(fill);
        }

        return order;
    }

    /// <summary>The first series is emitted last, so it is the one on top.</summary>
    [Fact]
    public void AnUnstackedAreaPaintsTheFirstSeriesOverTheSecond()
    {
        List<Colour> order = FillOrder(Areas(stacked: false));

        order.Count.ShouldBe(2);
        order[0].ShouldBe(Second);
        order[^1].ShouldBe(First);
    }

    /// <summary>The control: a stacked area keeps file order.</summary>
    [Fact]
    public void AStackedAreaKeepsFileOrder()
    {
        List<Colour> order = FillOrder(Areas(stacked: true));

        order.Count.ShouldBe(2);
        order[0].ShouldBe(First);
        order[^1].ShouldBe(Second);
    }

    /// <summary>
    /// The second control, and the one that says the reversal is a paint order and not a swap of
    /// the data: the polygon drawn <em>last</em> still traces the <em>first</em> series' numbers.
    /// </summary>
    /// <remarks>
    /// Series 1 rises 10 → 40 across the four categories and series 2 falls 40 → 10, so the two
    /// are told apart by the sign of their own slope and not by their colour. A reversal that
    /// swapped the series rather than the emission would show the last-drawn polygon falling.
    /// </remarks>
    [Fact]
    public void TheReversalMovesThePaintOrderAndNotTheData()
    {
        ChartDrawing drawing = ChartLayout.Place(Areas(stacked: false), Frame, new Ruler());

        static (Length Left, Length Right) Ends(ChartDrawing drawing, Colour fill)
        {
            Length left = Length.Zero;
            Length right = Length.Zero;
            Length x0 = Length.FromPoints(1e6);
            Length x1 = Length.FromPoints(-1e6);

            foreach (ChartShape shape in drawing.Shapes)
            {
                if (shape.Fill != fill) continue;

                foreach (PathCommand command in shape.Path.Commands)
                {
                    // A Close carries a default point, which is the origin and not on the path.
                    if (command.Verb == PathVerb.Close) continue;

                    if (command.Point.X < x0) x0 = command.Point.X;
                    if (command.Point.X > x1) x1 = command.Point.X;
                }
            }

            // The top of the band at each extreme, which is the smallest y there: an area
            // polygon carries the baseline as well as the plotted vertex at every x.
            left = Length.FromPoints(1e6);
            right = Length.FromPoints(1e6);

            foreach (ChartShape shape in drawing.Shapes)
            {
                if (shape.Fill != fill) continue;

                foreach (PathCommand command in shape.Path.Commands)
                {
                    if (command.Verb == PathVerb.Close) continue;

                    if (Math.Abs((command.Point.X - x0).Points) < 0.01
                        && command.Point.Y < left)
                    {
                        left = command.Point.Y;
                    }

                    if (Math.Abs((command.Point.X - x1).Points) < 0.01
                        && command.Point.Y < right)
                    {
                        right = command.Point.Y;
                    }
                }
            }

            return (left, right);
        }

        // A rising series ends higher on the page, which is a smaller y.
        (Length firstLeft, Length firstRight) = Ends(drawing, First);
        (Length secondLeft, Length secondRight) = Ends(drawing, Second);

        firstRight.Points.ShouldBeLessThan(firstLeft.Points);
        secondRight.Points.ShouldBeGreaterThan(secondLeft.Points);
    }
}
