using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// An axis reserves its tick length only when it draws a tick <em>outside</em> the plot area.
/// </summary>
/// <remarks>
/// <para>
/// <c>lclGetTickMark</c> (<c>oox/source/drawingml/chart/axisconverter.cxx:104-115</c>) maps
/// <c>out</c> and <c>cross</c> to a tick style carrying <c>OUTER</c> and <c>in</c> and
/// <c>none</c> to one that does not; only an outward tick extends past the plot area, so only
/// an outward tick is charged to it by <c>VDiagram::adjustInnerSize</c>.
/// </para>
/// <para>
/// <strong>Measured before it was written</strong>, one property and one axis at a time, on a
/// corpus chart already stating <c>c:majorTickMark val="none"</c> on both axes and rendered six
/// ways through 26.2.4.2. The plot edge moves by
/// <c>none 0.00 / in 0.00 / out +4.25 / cross +4.25</c> — <c>AXIS2D_TICKLENGTH</c> exactly — on
/// that axis' own edge and on no other, and the leftmost value label's pen sits at the same
/// <c>x</c> in all four arms.
/// </para>
/// </remarks>
public class ChartLayoutTickMarkTests
{
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
            => new(size * (0.5 * text.Length) * (bold ? 1.1 : 1.0), size * 1.15);
    }

    private static readonly DocRect Frame =
        new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300));

    /// <summary><c>AXIS2D_TICKLENGTH</c>, 150 hundredths of a millimetre.</summary>
    private static readonly Length TickLength = Length.FromMm100(150);

    private static ChartPlot Bars() => new()
    {
        Categories = ["Q1", "Q2", "Q3", "Q4"],
        Series = [new ChartSeries("North", [120.0, 95.0, 143.0, 168.0], Colour.FromRgb(0x99CCFF))],
    };

    private static ChartDrawing Place(ChartPlot plot) => ChartLayout.Place(plot, Frame, new Ruler());

    [Theory]
    [InlineData(ChartTickMark.Outer, true)]
    [InlineData(ChartTickMark.Cross, true)]
    [InlineData(ChartTickMark.Inner, false)]
    [InlineData(ChartTickMark.None, false)]
    public void OnlyAnOutwardTickIsChargedToTheValueAxisEdge(ChartTickMark mark, bool reserves)
    {
        ChartDrawing outward = Place(Bars() with { ValueTicks = ChartTickMark.Outer });
        ChartDrawing drawing = Place(Bars() with { ValueTicks = mark });

        Length expected = reserves ? Length.Zero : TickLength;

        (outward.PlotArea.Left - drawing.PlotArea.Left).Points
            .ShouldBe(expected.Points, 0.001);

        // And on that axis' edge only: the bottom is the category axis' and does not move.
        drawing.PlotArea.Bottom.Points.ShouldBe(outward.PlotArea.Bottom.Points, 0.001);
    }

    [Theory]
    [InlineData(ChartTickMark.Outer, true)]
    [InlineData(ChartTickMark.Cross, true)]
    [InlineData(ChartTickMark.Inner, false)]
    [InlineData(ChartTickMark.None, false)]
    public void OnlyAnOutwardTickIsChargedToTheCategoryAxisEdge(ChartTickMark mark, bool reserves)
    {
        ChartDrawing outward = Place(Bars() with { CategoryTicks = ChartTickMark.Outer });
        ChartDrawing drawing = Place(Bars() with { CategoryTicks = mark });

        Length expected = reserves ? Length.Zero : TickLength;

        (drawing.PlotArea.Bottom - outward.PlotArea.Bottom).Points
            .ShouldBe(expected.Points, 0.001);

        drawing.PlotArea.Left.Points.ShouldBe(outward.PlotArea.Left.Points, 0.001);
    }

    /// <summary>
    /// The label's outer edge does not move with the tick — only the gap between it and the axis.
    /// </summary>
    /// <remarks>
    /// The half a reservation test cannot see, and the half the reference's own arms settle: the
    /// leftmost value label's pen sits at the same <c>x</c> whether the tick is drawn or not, so
    /// a reader that stopped reserving the tick and kept offsetting the label by it would move
    /// every label 4.25 pt outward.
    /// </remarks>
    [Fact]
    public void TheLabelsOuterEdgeIsWhereItWasWithoutTheTick()
    {
        ChartDrawing outward = Place(Bars() with { ValueTicks = ChartTickMark.Outer });
        ChartDrawing none = Place(Bars() with { ValueTicks = ChartTickMark.None });

        Length withTick = Leftmost(outward);
        Length without = Leftmost(none);

        without.Points.ShouldBe(withTick.Points, 0.001);

        static Length Leftmost(ChartDrawing drawing)
        {
            Length least = Length.FromPoints(10_000);
            foreach (ChartLabel label in drawing.Labels)
            {
                if (label.Anchor is ChartLabelAnchor.RightMiddle && label.At.X < least)
                    least = label.At.X;
            }

            return least;
        }
    }

    /// <summary>
    /// <c>none</c> draws no tick at all, and <c>in</c> draws one on the inside.
    /// </summary>
    /// <remarks>
    /// Reservation and drawing are two different questions and this is the second: an inward
    /// tick reserves nothing but is still 4.25 pt of ink, and drawing it outward or not at all
    /// are both wrong.
    /// </remarks>
    [Fact]
    public void TheTickIsDrawnOnTheSideTheAxisStates()
    {
        ChartDrawing none = Place(Bars() with { ValueTicks = ChartTickMark.None });
        ChartDrawing inner = Place(Bars() with { ValueTicks = ChartTickMark.Inner });
        ChartDrawing outer = Place(Bars() with { ValueTicks = ChartTickMark.Outer });

        Ticks(none, outside: true).ShouldBe(0);
        Ticks(none, outside: false).ShouldBe(0);

        Ticks(inner, outside: true).ShouldBe(0);
        Ticks(inner, outside: false).ShouldBeGreaterThan(0);

        Ticks(outer, outside: true).ShouldBeGreaterThan(0);
        Ticks(outer, outside: false).ShouldBe(0);

        int Ticks(ChartDrawing drawing, bool outside)
        {
            int count = 0;
            foreach (ChartLine line in drawing.Lines)
            {
                if (line.From.Y != line.To.Y) continue;

                Length left = Length.Min(line.From.X, line.To.X);
                Length right = Length.Max(line.From.X, line.To.X);
                if ((right - left - TickLength).Points is > 0.01 or < -0.01) continue;

                bool isOutside = right <= drawing.PlotArea.Left + Length.FromPoints(0.01);
                if (isOutside == outside) count++;
            }

            return count;
        }
    }
}
