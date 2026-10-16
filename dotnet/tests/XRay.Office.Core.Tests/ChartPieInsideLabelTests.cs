using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// A pie label placed <c>inEnd</c> sits at the rim, not in the middle of the ring.
/// </summary>
/// <remarks>
/// <para>
/// <c>PolarLabelPositionHelper::getLabelScreenPositionAndAlignmentForUnitCircleValues</c> takes
/// the ring's <em>outer</em> radius for <c>INSIDE</c> and <c>OUTSIDE</c> alike and the middle of
/// the ring for everything else — <c>bCenter</c> is exactly
/// <c>nLabelPlacement != OUTSIDE &amp;&amp; != INSIDE</c>
/// (<c>PolarLabelPositionHelper.cxx</c>:68-76) — and <c>PieChart::createTextLabelShape</c> then
/// pulls an <c>INSIDE</c> anchor <strong>back</strong> along the radius by the same flat 150
/// hundredths of a millimetre it pushes an <c>OUTSIDE</c> one out by (<c>PieChart.cxx</c>:439-452).
/// The eight-row alignment table is written as <c>bOutside ? A : B</c> with B the opposite of A
/// on every row, so the block hangs back over the slice instead of away from it.
/// </para>
/// <para>
/// <strong>Measured on <c>029_Unit_Circle_Chart_Pie_Theme_8a922142.docx</c></strong>, whose four
/// labels 26.2.4.2 places <c>inEnd</c>. Each label's distance from the pie's centre as a fraction
/// of its radius, reference against this tree before and after:
/// </para>
/// <code>
/// reference  0.804  0.839  0.917  0.953  0.890  0.923  0.841
/// before     0.414  0.451  0.563  0.588  0.527  0.549  0.467
/// after      0.806  0.841  0.918  0.953  0.891  0.924  0.842
/// </code>
/// <para>
/// Seven of seven within <strong>0.002</strong>, against a rule with no free parameter in it —
/// which is what says the block's own alignment is right as well as its anchor, since the
/// distance is measured to the block and not to the anchor.
/// </para>
/// </remarks>
public sealed class ChartPieInsideLabelTests
{
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
            => new(size * (0.5 * text.Length) * (bold ? 1.1 : 1.0), size * 1.15);
    }

    private static readonly DocRect Frame =
        new(Length.Zero, Length.Zero, Length.FromPoints(500), Length.FromPoints(400));

    private static ChartPlot Pie(ChartLabelPlacement placement)
        => new()
        {
            Kind = ChartPlotKind.Pie,
            Legend = ChartLegendPosition.None,
            Categories = ["North", "South", "East", "West"],
            Series =
            [
                new ChartSeries("S", [25.0, 25.0, 25.0, 25.0], Colour.FromRgb(0x70AD47))
                {
                    Label = new ChartDataLabel
                    {
                        ShowCategory = true,
                        Placement = placement,
                    },
                },
            ],
        };

    /// <summary>Every label's distance from the pie's centre, as a fraction of its radius.</summary>
    private static List<double> Reach(ChartLabelPlacement placement)
    {
        ChartDrawing drawing = ChartLayout.Place(Pie(placement), Frame, new Ruler());

        // The wedges together are the whole circle, so their union is its box.
        DocRect ring = drawing.Shapes[0].Bounds();
        foreach (ChartShape shape in drawing.Shapes)
        {
            DocRect one = shape.Bounds();
            ring = new DocRect(
                Length.Min(ring.Left, one.Left),
                Length.Min(ring.Top, one.Top),
                Length.Max(ring.Right, one.Right) - Length.Min(ring.Left, one.Left),
                Length.Max(ring.Bottom, one.Bottom) - Length.Min(ring.Top, one.Top));
        }

        double cx = ring.Left.Points + (ring.Width.Points / 2);
        double cy = ring.Top.Points + (ring.Height.Points / 2);
        double radius = ring.Width.Points / 2;

        List<double> reach = [];
        foreach (ChartLabel label in drawing.Labels)
        {
            double dx = label.At.X.Points - cx;
            double dy = label.At.Y.Points - cy;
            reach.Add(Math.Sqrt((dx * dx) + (dy * dy)) / radius);
        }

        return reach;
    }

    /// <summary>
    /// A centred label is at half the radius and an <c>inEnd</c> one is out at the rim.
    /// </summary>
    /// <remarks>
    /// The discriminator that needs no measured constant: the same pie, the same four labels,
    /// one attribute apart. Before this existed both drew them at half the radius.
    /// </remarks>
    [Fact]
    public void AnInsideLabelIsOutAtTheRimAndACentredOneIsHalfWay()
    {
        List<double> centred = Reach(ChartLabelPlacement.Centre);
        List<double> inside = Reach(ChartLabelPlacement.Inside);

        centred.Count.ShouldBe(4);
        inside.Count.ShouldBe(4);

        foreach (double reach in centred) reach.ShouldBeLessThan(0.6);
        foreach (double reach in inside) reach.ShouldBeGreaterThan(0.6);
    }

    /// <summary>An <c>inEnd</c> label stays inside the rim and an <c>outEnd</c> one leaves it.</summary>
    /// <remarks>
    /// The two share one anchor and one alignment table and differ only in the sign of the 150
    /// and in which half of each `bOutside ? A : B` row applies, so this is the assertion that
    /// says the sign was not applied twice or not at all.
    /// </remarks>
    [Fact]
    public void AnInsideLabelStaysInsideTheRimAndAnOutsideOneLeavesIt()
    {
        List<double> inside = Reach(ChartLabelPlacement.Inside);
        List<double> outside = Reach(ChartLabelPlacement.Outside);

        for (int at = 0; at < inside.Count; at++)
        {
            inside[at].ShouldBeLessThan(1.0);
            outside[at].ShouldBeGreaterThan(1.0);
        }
    }
}
