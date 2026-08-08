using Paperless.Core.Charts;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Shouldly;

namespace Paperless.Core.Tests;

/// <summary>
/// Turning an axis' tick labels off, which is not the same as deleting the axis, and what the
/// bottom edge of the plot area gives up once they are gone.
/// </summary>
/// <remarks>
/// <c>c:tickLblPos val="none"</c> — ODF's <c>chart:display-label="false"</c> — maps to chart2's
/// <c>DisplayLabels</c> and suppresses the labels alone; the axis line and its tick marks are
/// still drawn (<c>oox/source/drawingml/chart/axisconverter.cxx:221</c>,
/// <c>chart2/source/view/axes/VAxisProperties.cxx:307</c>). <c>c:delete</c> removes all three.
/// Conflating them costs a tick's length in one direction and a whole label line in the other.
/// </remarks>
public class ChartAxisLabelVisibilityTests
{
    /// <summary>Half an em per character, 1.15 em a line — Liberation Sans to three places.</summary>
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
            => new(size * (0.5 * text.Length) * (bold ? 1.1 : 1.0), size * 1.15);
    }

    private static readonly DocRect Frame =
        new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300));

    private static ChartDrawing Place(ChartPlot plot) => ChartLayout.Place(plot, Frame, new Ruler());

    /// <summary>
    /// How deep the band under the plot area is, in points: what the bottom edge gave up out of
    /// the frame once the two per cent page margin is taken off.
    /// </summary>
    private static double Band(ChartDrawing drawing)
        => (Frame.Bottom - Frame.Height * 0.02 - drawing.PlotArea.Bottom).Points;

    private static ChartPlot Bars() => new()
    {
        Categories = ["Q1", "Q2", "Q3", "Q4"],
        Series = [new ChartSeries("North", [120.0, 95.0, 143.0, 168.0], Colour.FromRgb(0x99CCFF))],
    };

    /// <summary>Turning the category labels off draws none of them.</summary>
    [Fact]
    public void HiddenCategoryLabelsAreNotDrawn()
    {
        Place(Bars()).Labels.Count(label => label.Text.StartsWith('Q')).ShouldBe(4);

        Place(Bars() with { CategoryLabelsVisible = false })
            .Labels.ShouldNotContain(label => label.Text.StartsWith('Q'));
    }

    /// <summary>Turning the value labels off draws none of them.</summary>
    [Fact]
    public void HiddenValueLabelsAreNotDrawn()
    {
        Place(Bars()).Labels.ShouldContain(label => label.Text == "180");

        Place(Bars() with { ValueLabelsVisible = false })
            .Labels.ShouldNotContain(label => label.Text == "180");
    }

    /// <summary>
    /// Hidden labels give their room back — all but the tick, which is still drawn.
    /// </summary>
    /// <remarks>
    /// The measurement this reproduces: on a probe over <c>chart-face-theme-minor.pptx</c> whose
    /// category axis states <c>tickLblPos="none"</c>, LibreOffice's plot area drops 12.70 pt and
    /// its axis keeps five 4.25 pt ticks below the new edge. So the recovered room is the label's
    /// own height plus the tick-to-label spacing, and never the tick.
    /// </remarks>
    [Fact]
    public void HiddenCategoryLabelsGiveBackEverythingButTheTick()
    {
        // Half a value label is deeper than a bare tick and would mask it, so the tick is read
        // off a chart whose value labels are gone too.
        ChartPlot plain = Bars() with { ValueLabelsVisible = false };

        double shown = Band(Place(plain));
        double hidden = Band(Place(plain with { CategoryLabelsVisible = false }));
        double deleted = Band(Place(plain with { CategoryAxisVisible = false }));

        // A 10 pt label on a 1.15 em ruler, the 100 tick-label spacing, and the 150 tick.
        shown.ShouldBe(11.5 + Length.FromMm100(100).Points + Length.FromMm100(150).Points, 0.001);
        hidden.ShouldBe(Length.FromMm100(150).Points, 0.001);
        deleted.ShouldBe(0.0, 0.001);
    }

    /// <summary>An axis with no labels still draws its line and its ticks.</summary>
    [Fact]
    public void AnUnlabelledAxisStillDrawsItsLineAndTicks()
    {
        ChartDrawing hidden = Place(Bars() with { CategoryLabelsVisible = false });
        ChartDrawing deleted = Place(Bars() with { CategoryAxisVisible = false });

        Length edge = hidden.PlotArea.Bottom;

        hidden.Lines.ShouldContain(
            line => line.From.Y == edge && line.To.Y == edge && line.From.X != line.To.X);
        hidden.Lines.Count(line => line.From.X == line.To.X && line.From.Y == edge).ShouldBe(5);

        Length gone = deleted.PlotArea.Bottom;
        deleted.Lines.ShouldNotContain(
            line => line.From.X == line.To.X && line.From.Y == gone);
    }

    /// <summary>
    /// The bottom edge still gives up half a value label once the category labels are gone.
    /// </summary>
    /// <remarks>
    /// The bottommost value label is centred on the plot area's bottom-left corner exactly as the
    /// topmost is centred on its top-left, so half of it hangs below. It shares the strip with the
    /// category band rather than stacking under it — LibreOffice reserves the bounding box of what
    /// its axes drew (<c>VDiagram::adjustInnerSize</c>) — so what the edge gives up is the deeper
    /// of the two and not their sum. Measured with the category labels off: the reference's bottom
    /// edge sits 5.65 pt below the plot against half a label's 5.67.
    /// </remarks>
    [Fact]
    public void TheBottomEdgeKeepsHalfAValueLabelWhenTheCategoryLabelsAreGone()
    {
        ChartPlot plot = Bars() with { CategoryLabelsVisible = false };

        // 10 pt at 1.15 em a line is an 11.5 pt label, so half of it is 5.75 — deeper than the
        // 4.25 pt tick that is all the unlabelled axis claims on its own.
        Band(Place(plot)).ShouldBe(11.5 / 2.0, 0.001);
        Band(Place(plot with { ValueLabelsVisible = false }))
            .ShouldBe(Length.FromMm100(150).Points, 0.001);
    }

    /// <summary>Half a value label never widens the band a category label already claims.</summary>
    /// <remarks>
    /// The two occupy one strip, so an ordinary chart — where a whole label line is deeper than
    /// half of one — must reserve exactly what it did before this rule existed.
    /// </remarks>
    [Fact]
    public void HalfAValueLabelDoesNotAddToADeeperCategoryBand()
    {
        ChartDrawing both = Place(Bars());
        ChartDrawing noValueLabels = Place(Bars() with { ValueLabelsVisible = false });

        both.PlotArea.Bottom.Points.ShouldBe(noValueLabels.PlotArea.Bottom.Points, 0.001);
    }
}
