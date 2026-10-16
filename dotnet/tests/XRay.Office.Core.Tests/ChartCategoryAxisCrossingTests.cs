using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// The category axis stands where it crosses the value axis, and its labels hang from that line.
/// </summary>
/// <remarks>
/// <para>
/// <c>c:catAx/c:crosses val="autoZero"</c> — the default — asks for value zero on the axis it
/// crosses, which <c>AxisProperties::initAxisPositioning</c> turns into
/// <c>m_pfMainLinePositionAtOtherAxis = 0.0</c>
/// (<c>chart2/source/view/axes/VAxisProperties.cxx</c>:224-225);
/// <c>VCartesianAxis::getAxisIntersectionValue</c> (<c>VCartesianAxis.cxx</c>:1092-1101) reads it
/// back and <c>get2DAxisMainLine</c> clamps it into the value axis' range (<c>:1253-1256</c>).
/// Under the default <c>c:tickLblPos val="nextTo"</c> the labels take the same line, because
/// <c>getLabelLineIntersectionValue</c> (<c>:1103-1113</c>) falls through to it.
/// </para>
/// <para>
/// <strong>Measured on <c>Demick_JetBlue.pptx</c> page 5</strong>, a column chart running
/// −44 587 … 1 200 000. 26.2.4.2 draws the plot from y = 214.58 to 401.56 with the category axis
/// line at <strong>374.83</strong> — the <c>$-</c> gridline — and its labels hanging from there,
/// inside the plot. This tree drew the line and the labels at the plot's bottom edge, y = 376.18
/// and 383.08, which is legible by accident.
/// </para>
/// <para>
/// <strong>Reach: 4 documents, 5 chart parts</strong> state a category axis crossing at zero with
/// <c>nextTo</c> labels over values that span zero (<c>probes/chart-crossing/census.py</c>). The
/// clamp is what keeps it to those: a chart whose values are all of one sign puts the line back on
/// the edge it has always been drawn at.
/// </para>
/// </remarks>
public sealed class ChartCategoryAxisCrossingTests
{
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
            => new(size * (0.5 * text.Length), size * 1.15);
    }

    private static readonly DocRect Frame =
        new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300));

    private static ChartPlot Columns(IReadOnlyList<double?> values) => new()
    {
        Categories = ["A", "B", "C"],
        CategoryAxisLine = new ChartGrid(Colour.FromRgb(0x000000)),
        Series = [new ChartSeries("S", values, Colour.FromRgb(0x0070C0))],
    };

    /// <summary>The horizontal line the category axis draws for itself.</summary>
    private static ChartLine AxisLine(ChartDrawing drawing)
        => drawing.Lines.First(
            line => line.From.Y == line.To.Y
                    && line.From.X == drawing.PlotArea.Left
                    && line.To.X == drawing.PlotArea.Right);

    /// <summary>A chart whose values span zero puts its category axis on the zero line.</summary>
    [Fact]
    public void AnAxisCrossingAtZeroIsDrawnInsideThePlot()
    {
        ChartDrawing drawing = ChartLayout.Place(Columns([-40.0, 100.0, 60.0]), Frame, new Ruler());

        DocRect plot = drawing.PlotArea;
        ChartLine axis = AxisLine(drawing);

        axis.From.Y.ShouldBeGreaterThan(plot.Top);
        axis.From.Y.ShouldBeLessThan(plot.Bottom);
    }

    /// <summary>And it is at the value zero, not merely somewhere inside.</summary>
    /// <remarks>
    /// The discriminator against "put it in the middle": the axis is one number, and that number
    /// is where the scale puts zero. Ours resolves this chart's scale to −50 … 100, so zero is a
    /// third of the way up.
    /// </remarks>
    [Fact]
    public void TheAxisSitsWhereTheScalePutsZero()
    {
        ChartPlot plot = Columns([-40.0, 100.0, 60.0]) with
        {
            ValueScale = new ChartScaleRequest(Minimum: -50.0, Maximum: 100.0),
        };

        ChartDrawing drawing = ChartLayout.Place(plot, Frame, new Ruler());
        DocRect area = drawing.PlotArea;
        ChartLine axis = AxisLine(drawing);

        double along = (double)(area.Bottom - axis.From.Y).Emu / area.Height.Emu;
        along.ShouldBe(50.0 / 150.0, 0.002);
    }

    /// <summary>The labels hang from the axis' own line rather than from the plot's edge.</summary>
    [Fact]
    public void TheLabelsHangFromTheCrossingLine()
    {
        ChartDrawing drawing = ChartLayout.Place(Columns([-40.0, 100.0, 60.0]), Frame, new Ruler());

        ChartLine axis = AxisLine(drawing);
        List<ChartLabel> categories =
            [.. drawing.Labels.Where(label => label.Anchor == ChartLabelAnchor.CentreTop)];

        categories.Count.ShouldBe(3);
        foreach (ChartLabel label in categories)
        {
            label.At.Y.ShouldBeGreaterThanOrEqualTo(axis.From.Y);
            label.At.Y.ShouldBeLessThan(axis.From.Y + Length.FromPoints(12));
        }
    }

    /// <summary>
    /// <c>c:tickLblPos val="low"</c> sends the labels to the axis' minimum, line or no line.
    /// </summary>
    /// <remarks>
    /// The half of <c>getLabelLineIntersectionValue</c> that does <em>not</em> fall through: the
    /// two outside positions answer the crossing axis' own minimum and maximum, so a file can put
    /// the line at zero and the labels under the plot.
    /// </remarks>
    [Fact]
    public void LowLabelsStayAtThePlotsBottomWhileTheLineCrossesAtZero()
    {
        ChartPlot plot = Columns([-40.0, 100.0, 60.0]) with
        {
            CategoryLabelPosition = ChartValueLabelPosition.Low,
        };

        ChartDrawing drawing = ChartLayout.Place(plot, Frame, new Ruler());
        ChartLine axis = AxisLine(drawing);

        axis.From.Y.ShouldBeLessThan(drawing.PlotArea.Bottom);
        foreach (ChartLabel label in drawing.Labels.Where(
                     one => one.Anchor == ChartLabelAnchor.CentreTop))
        {
            label.At.Y.ShouldBeGreaterThanOrEqualTo(drawing.PlotArea.Bottom);
        }
    }

    /// <summary>A chart whose values are all positive is drawn exactly as it was.</summary>
    /// <remarks>
    /// The control, and the reason this is safe corpus-wide: <c>get2DAxisMainLine</c> clamps the
    /// crossing into the value range, and an all-positive scale starts at zero, so the line lands
    /// back on the plot's own bottom edge and the labels take their band under it as before.
    /// </remarks>
    [Fact]
    public void AnAllPositiveChartKeepsItsAxisOnThePlotsEdge()
    {
        ChartDrawing drawing = ChartLayout.Place(Columns([40.0, 100.0, 60.0]), Frame, new Ruler());

        AxisLine(drawing).From.Y.ShouldBe(drawing.PlotArea.Bottom);

        foreach (ChartLabel label in drawing.Labels.Where(
                     one => one.Anchor == ChartLabelAnchor.CentreTop))
        {
            label.At.Y.ShouldBeGreaterThanOrEqualTo(drawing.PlotArea.Bottom);
        }
    }

    /// <summary>
    /// Labels drawn inside the plot take no band off it, so the plot is taller.
    /// </summary>
    /// <remarks>
    /// <c>VDiagram::adjustInnerSize</c> shrinks the inner rectangle by how far the drawn labels
    /// overflow the available one (<c>chart2/source/view/diagram/VDiagram.cxx</c>:661-669), and
    /// labels drawn inside overflow nothing. On the witness this is the difference between a plot
    /// ending at y = 376.18 and one ending at 405.62 against the reference's 401.56.
    /// </remarks>
    [Fact]
    public void AChartWithItsLabelsInsideGivesUpNoBandForThem()
    {
        DocRect inside = ChartLayout.Place(
            Columns([-40.0, 100.0, 60.0]), Frame, new Ruler()).PlotArea;
        DocRect below = ChartLayout.Place(
            Columns([40.0, 100.0, 60.0]), Frame, new Ruler()).PlotArea;

        inside.Bottom.ShouldBeGreaterThan(below.Bottom);
    }
}
