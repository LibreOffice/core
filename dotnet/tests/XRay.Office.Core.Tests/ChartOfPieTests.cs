using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// The three things an of-pie chart's composition gets from <c>PieChart</c> rather than from the
/// file: where the main ring opens, which end of the bar the first split point takes, and what
/// colour the composite wedge is.
/// </summary>
/// <remarks>
/// <para>
/// All three were measured against 26.2.4.2 on
/// <c>028_Unit_Circle_Chart_Optimized_Graph_83d9c756.docx</c> — the corpus's one drawn bar-of-pie
/// — by reading the page's own fill operators, which give each wedge's colour and bounding box
/// exactly. <c>probes/ofpie-r190</c> has the numbers.
/// </para>
/// <para>
/// <strong>The composite wedge's colour is the sharpest of the three, because the file states a
/// different one and the reference never reads it.</strong> That document's series carries
/// sixteen values and <em>seventeen</em> <c>c:dPt</c>, the last of them
/// <c>accent4 lumMod 50%</c> = <c>#806000</c> for the composite wedge; 26.2.4.2 draws
/// <c>#7E0021</c>, which is entry 4 of its own twelve configured defaults and has nothing to do
/// with the document.
/// </para>
/// </remarks>
public sealed class ChartOfPieTests
{
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
            => new(size * (0.5 * text.Length) * (bold ? 1.1 : 1.0), size * 1.15);
    }

    private static readonly DocRect Frame =
        new(Length.Zero, Length.Zero, Length.FromPoints(600), Length.FromPoints(400));

    /// <summary>A bar-of-pie of six points split at two, with no labels and no legend.</summary>
    private static ChartPlot OfPie(
        ChartOfPieType form = ChartOfPieType.Bar, int split = 2, params double[] values)
        => new()
        {
            Kind = ChartPlotKind.OfPie,
            OfPieType = form,
            SplitPosition = split,
            Legend = ChartLegendPosition.None,
            Series =
            [
                new ChartSeries(
                    "S",
                    [.. Array.ConvertAll(
                        values.Length > 0 ? values : [40.0, 30.0, 20.0, 6.0, 30.0, 10.0],
                        v => (double?)v)],
                    Colour.FromRgb(0x4472C4)),
            ],
        };

    /// <summary>Every filled shape, in paint order.</summary>
    private static List<ChartShape> Wedges(ChartPlot plot)
        => [.. ChartLayout.Place(plot, Frame, new Ruler()).Shapes];

    /// <summary>
    /// The composite wedge straddles three o'clock, so the two connecting lines meet it.
    /// </summary>
    /// <remarks>
    /// <c>createOneRing</c>'s <c>sAngle</c> lambda (<c>PieChart.cxx</c>:1229-1244) opens the ring
    /// at <c>360 − compositeVal·360/(2·ringSum)</c> for clockwise wedges — minus half the
    /// composite sweep — and the composite is drawn last, so it closes back across the axis.
    /// Measured: 26.2.4.2 draws <c>028</c>'s composite from y 395.14 to 437.94 about a centre at
    /// 416.54, which is ±21.4 pt for a half-sweep of 13.73°; opening at <em>plus</em> half, as
    /// this tree did, put the whole wedge above the centre and left the connectors meeting
    /// nothing.
    /// </remarks>
    [Fact]
    public void TheCompositeWedgeStraddlesThreeOClock()
    {
        List<ChartShape> shapes = Wedges(OfPie());

        // Four kept wedges, then the composite, then the bar's two segments.
        shapes.Count.ShouldBe(7);

        DocRect composite = shapes[4].Bounds();

        // The ring's own centre. Its five wedges together are the whole circle, so the union of
        // their boxes is the circle's box and its middle is the centre.
        DocRect ring = shapes[0].Bounds();
        for (int at = 1; at <= 4; at++)
        {
            DocRect one = shapes[at].Bounds();
            ring = new DocRect(
                Length.Min(ring.Left, one.Left),
                Length.Min(ring.Top, one.Top),
                Length.Max(ring.Right, one.Right) - Length.Min(ring.Left, one.Left),
                Length.Max(ring.Bottom, one.Bottom) - Length.Min(ring.Top, one.Top));
        }

        Length centreY = ring.Top + (ring.Height / 2);

        composite.Top.Points.ShouldBeLessThan(centreY.Points);
        composite.Bottom.Points.ShouldBeGreaterThan(centreY.Points);

        // Symmetric about it, because the composite is bisected by the axis.
        (centreY - composite.Top).Points
            .ShouldBe((composite.Bottom - centreY).Points, 0.5);
    }

    /// <summary>The first split point is the bar's bottom segment, not its top.</summary>
    /// <remarks>
    /// <c>createOneBar</c> opens at <c>fBarTop = -0.5</c> and adds each share upwards — the
    /// comment is <c>// make the bar go from -0.5 to 0.5</c>
    /// (<c>PieChart.cxx</c>:1416-1430) — and the value axis points up. On <c>028</c> 26.2.4.2
    /// draws Leaf 16 (value 21) above Leaf 15 (value 23), 64.60 pt over 70.80; this tree drew
    /// them the other way round at the same two heights.
    /// </remarks>
    [Fact]
    public void TheBarStacksTheFirstSplitPointAtTheFoot()
    {
        // The split points are 30 and 10, so the two segments are plainly different sizes.
        List<ChartShape> shapes = Wedges(OfPie());

        DocRect first = shapes[5].Bounds();
        DocRect second = shapes[6].Bounds();

        // Drawn in data order, and the first is the lower of the two.
        first.Top.Points.ShouldBeGreaterThan(second.Top.Points);
        first.Bottom.Points.ShouldBeGreaterThan(second.Bottom.Points);

        // They meet, and the taller share belongs to the larger value.
        first.Top.Points.ShouldBe(second.Bottom.Points, 0.01);
        first.Height.Points.ShouldBe(second.Height.Points * 3.0, 0.05);
    }

    /// <summary>
    /// The composite wedge takes LibreOffice's own default series colour, not the series'.
    /// </summary>
    /// <remarks>
    /// Its property index is <c>getTotalPointCount()</c> (<c>propIndex</c>,
    /// <c>PieChart.cxx</c>:1185-1191), which is always one past the last value, and
    /// <c>DataSeries::getDataPointByIndex</c> answers an empty reference outside the data's own
    /// length (<c>DataSeries.cxx</c>:312-337). So <c>hasPointOwnColor</c> is false whatever the
    /// file states and <c>createOneRing</c> falls through to
    /// <c>m_xColorScheme->getColorByIndex</c> (<c>:1325-1331</c>), which is
    /// <c>Office.Chart/DefaultColor/Series</c> cycled at twelve.
    /// </remarks>
    [Theory]
    [InlineData(4, 0x7E0021u)]   // palette entry 4
    [InlineData(5, 0x83CAFFu)]
    [InlineData(12, 0x004586u)]  // wraps at twelve
    [InlineData(16, 0x7E0021u)]  // 028's own sixteen points
    public void TheCompositeWedgeTakesTheDefaultPaletteColour(int points, uint expected)
    {
        double[] values = new double[points];
        for (int at = 0; at < points; at++) values[at] = at + 1;

        List<ChartShape> shapes = Wedges(OfPie(ChartOfPieType.Bar, 2, values));

        // Kept wedges, then the composite, then the bar's two segments.
        ChartShape composite = shapes[points - 2];

        composite.Fill.ShouldBe(Colour.FromRgb(expected));

        // And every other wedge keeps the series' own colour, so this is the composite alone.
        shapes[0].Fill.ShouldBe(Colour.FromRgb(0x4472C4));
    }

    /// <summary>
    /// A series of fewer than four points is drawn as a plain pie, with no bar and no composite.
    /// </summary>
    /// <remarks>
    /// <c>OfPieDataSrc::minPoints = 4</c> (<c>PieChart.hxx</c>:108), tested by
    /// <c>createShapes</c> before anything else (<c>PieChart.cxx</c>:1052-1056). It is the
    /// control for the theory above: the palette colour must not reach a chart that never splits.
    /// </remarks>
    [Fact]
    public void TooFewPointsDrawAPlainPie()
    {
        List<ChartShape> shapes = Wedges(OfPie(ChartOfPieType.Bar, 2, 40.0, 30.0, 20.0));

        shapes.Count.ShouldBe(3);
        foreach (ChartShape shape in shapes)
            shape.Fill.ShouldBe(Colour.FromRgb(0x4472C4));
    }
}
