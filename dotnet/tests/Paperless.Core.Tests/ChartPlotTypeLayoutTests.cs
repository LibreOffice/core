using Paperless.Core.Charts;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Shouldly;

namespace Paperless.Core.Tests;

/// <summary>
/// What the four plot types added last compose — radar, bubble, stock and of-pie — and the one
/// that composes nothing on purpose.
/// </summary>
/// <remarks>
/// <para>
/// Every assertion here is a number taken out of <c>chart2/source/view/charttypes/</c> or out of
/// LibreOffice's own PDF for a file in <c>chart2/qa/extras/data/</c>, because all four types have
/// the same failure mode: a picture that is entirely plausible and wrong by a constant. A bubble
/// scaled by the value instead of its square root, a radar turned a quarter turn, a candle whose
/// high and low are swapped and an of-pie split at the wrong end all read as data, not as
/// geometry.
/// </para>
/// <para>
/// Composed from hand-built <see cref="ChartPlot"/> values rather than from documents, because
/// what is being tested is the geometry and not either reader; the readers are covered in
/// <c>Paperless.Presentations.Tests</c> beside the rest of the chart-part reading.
/// </para>
/// </remarks>
public class ChartPlotTypeLayoutTests
{
    /// <summary>A measurer with no fonts: half an em per character, 1.15 em a line.</summary>
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family)
            => new(size * (0.5 * text.Length), size * 1.15);
    }

    private static readonly DocRect Frame =
        new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300));

    private static ChartDrawing Place(ChartPlot plot) => ChartLayout.Place(plot, Frame, new Ruler());

    private static double Points(Length length) => length.Emu / 914400.0 * 72.0;

    // ---------------------------------------------------------------- radar

    private static ChartPlot Radar() => new()
    {
        Kind = ChartPlotKind.Radar,
        Categories = ["A", "B", "C", "D", "E"],
        Series = [new ChartSeries("S", [40.0, 40.0, 40.0, 40.0, 40.0], Colour.FromRgb(0x004586))],

        // Stated rather than computed, so that the ring count and the radii are the test's and
        // not the scale algorithm's — which has a case of its own in ChartScaleTests.
        ValueScale = new ChartScaleRequest(0.0, 40.0, 20.0),
    };

    /// <summary>
    /// The first category is straight up and they run clockwise, 360/n apart.
    /// </summary>
    /// <remarks>
    /// Measured in LibreOffice's own PDF for
    /// <c>chart2/qa/extras/data/docx/radar-chart-labels.docx</c>, whose five categories put the
    /// outermost web's vertices at 90°, 18°, −54°, −126° and 162° from a centre at
    /// (261.9, 582.6). Starting at three o'clock — which is what a naive polar conversion gives —
    /// turns the whole picture a quarter turn and looks like the categories are in the wrong
    /// order.
    /// </remarks>
    [Fact]
    public void ARadarChartStartsAtTwelveOClockAndRunsClockwise()
    {
        ChartDrawing drawing = Place(Radar());

        // One closed polygon for the series, drawn as a stroke because the style is not filled.
        ChartShape polygon = drawing.Shapes.Single();
        List<DocPoint> vertices =
            [.. polygon.Path.Commands.Where(c => c.Verb != PathVerb.Close).Select(c => c.Point)];

        vertices.Count.ShouldBe(5);

        DocRect area = drawing.PlotArea;
        DocPoint centre = new(area.X + area.Width / 2, area.Y + area.Height / 2);

        // Every point of the series is at the scale's maximum, so all five sit on the rim.
        double radius = Points(area.Width < area.Height ? area.Width / 2 : area.Height / 2);

        for (int at = 0; at < 5; at++)
        {
            double expected = (Math.PI / 2) - (2 * Math.PI * at / 5);
            double x = Points(vertices[at].X - centre.X);
            double y = Points(centre.Y - vertices[at].Y);

            Math.Sqrt((x * x) + (y * y)).ShouldBe(radius, 0.01);
            Math.Atan2(y, x).ShouldBe(Math.Atan2(Math.Sin(expected), Math.Cos(expected)), 0.001);
        }
    }

    /// <summary>
    /// A radar's radius axis has three rings whatever size it is drawn at.
    /// </summary>
    /// <remarks>
    /// <c>VPolarRadiusAxis::estimateMaximumAutoMainIncrementCount</c> returns a flat 2
    /// (<c>chart2/source/view/axes/VPolarRadiusAxis.cxx:87-90</c>), where the cartesian axis
    /// derives its count from the axis' own length and lands on ten. Measured: LibreOffice draws
    /// <c>radar-chart-labels.docx</c>, which peaks at 40, with rings at 0, 20 and 40 — the middle
    /// one at exactly half the outer radius, 52.4 pt against 104.8.
    /// </remarks>
    [Fact]
    public void ARadarsWebHasThreeRingsAndNotTen()
    {
        ChartDrawing drawing = Place(Radar() with { ValueGrid = Colour.FromRgb(0xB3B3B3) });

        // One segment per category per ring, and the ring at zero is a point rather than a ring.
        List<ChartLine> web =
            [.. drawing.Lines.Where(line => line.Colour == Colour.FromRgb(0xB3B3B3))];

        web.Count.ShouldBe(5 * 2);
    }

    /// <summary>A radar chart draws no cartesian axis lines, ticks or tick labels along an edge.</summary>
    [Fact]
    public void ARadarChartHasNoCartesianAxes()
    {
        ChartPlot plot = Radar();
        plot.HasAxes.ShouldBeFalse();

        ChartDrawing drawing = Place(plot);
        DocRect area = drawing.PlotArea;

        // Every line it draws is a spoke from the centre; none of them runs along an edge of the
        // plot rectangle the way a value or category axis does.
        foreach (ChartLine line in drawing.Lines)
        {
            bool alongLeft = line.From.X == area.Left && line.To.X == area.Left;
            bool alongBottom = line.From.Y == area.Bottom && line.To.Y == area.Bottom;
            (alongLeft || alongBottom).ShouldBeFalse();
        }
    }

    // --------------------------------------------------------------- bubble

    private static ChartPlot Bubbles(IReadOnlyList<double?> sizes) => new()
    {
        Kind = ChartPlotKind.Bubble,
        Series =
        [
            new ChartSeries("S", [1.0, 1.0], Colour.FromRgb(0x004586))
            {
                XValues = [1.0, 2.0],
                SizeValues = sizes,
            },
        ],
    };

    private static double Diameter(ChartShape shape)
    {
        List<DocPoint> points =
            [.. shape.Path.Commands.Where(c => c.Verb != PathVerb.Close).Select(c => c.Point)];

        return Points(points.Max(p => p.X) - points.Min(p => p.X));
    }

    /// <summary>
    /// A bubble's diameter goes as the square root of its stated size, not as the size.
    /// </summary>
    /// <remarks>
    /// <c>BubbleChart::transformToScreenBubbleSize</c> is
    /// <c>sqrt(size/π) / sqrt(max/π)</c> — the two π cancel — times the screen factor
    /// (<c>BubbleChart.cxx:97-113</c>). So a 1 beside a 9 is a third of its width and a ninth of
    /// its area. Taking the ratio directly gives a ninth of the width and an eighty-first of the
    /// area, which is the "plausible picture, every bubble the wrong size" failure.
    /// </remarks>
    [Fact]
    public void ABubblesDiameterGoesAsTheSquareRootOfItsSize()
    {
        ChartDrawing drawing = Place(Bubbles([1.0, 9.0]));

        List<ChartShape> bubbles = [.. drawing.Shapes];
        bubbles.Count.ShouldBe(2);

        (Diameter(bubbles[1]) / Diameter(bubbles[0])).ShouldBe(3.0, 0.001);
    }

    /// <summary>
    /// The largest bubble is a quarter of the plot area's lesser side across.
    /// </summary>
    /// <remarks>
    /// <c>calculateBubbleSizeScalingFactor</c>: <c>m_fBubbleSizeFactorToScreen = nMinExtend *
    /// 0.25</c>, commented "max bubble size is 25 percent of diagram size"
    /// (<c>BubbleChart.cxx:80-95</c>). It is the <em>diameter</em>: it becomes the
    /// <c>Direction3D</c> that <c>ShapeFactory::createCircle2D</c> uses as the shape's size,
    /// offsetting the centre by half of it. Reading it as a radius draws every bubble twice as
    /// wide as the reference.
    /// </remarks>
    [Fact]
    public void TheLargestBubbleIsAQuarterOfThePlotAreasLesserSide()
    {
        ChartDrawing drawing = Place(Bubbles([1.0, 9.0]));
        DocRect area = drawing.PlotArea;

        double lesser = Points(area.Width < area.Height ? area.Width : area.Height);
        Diameter(drawing.Shapes[1]).ShouldBe(lesser * 0.25, 0.01);
    }

    /// <summary>
    /// A negative size is skipped, and drawn at its absolute size when the series inverts.
    /// </summary>
    /// <remarks><c>BubbleChart.cxx:237-246</c>, three consecutive <c>continue</c>s.</remarks>
    [Fact]
    public void ANegativeBubbleIsSkippedUnlessTheSeriesInvertsIt()
    {
        ChartPlot plain = Bubbles([9.0, -1.0]);
        Place(plain).Shapes.Count.ShouldBe(1);

        ChartPlot inverted = plain with
        {
            Series = [plain.Series[0] with { InvertIfNegative = true }],
        };

        Place(inverted).Shapes.Count.ShouldBe(2);
    }

    /// <summary>
    /// <c>c:sizeRepresents val="w"</c> makes the diameter proportional to the size itself.
    /// </summary>
    /// <remarks>
    /// The named trap: LibreOffice parses <c>c:sizeRepresents</c> and <c>c:bubbleScale</c> into
    /// <c>TypeGroupModel</c> and never reads them again, so its own rendering is always
    /// area-at-100% and honouring a stated <c>w</c> is a deliberate <em>disagreement</em> with the
    /// oracle. No file in <c>chart2/qa/extras/data/</c> states anything but the default, so the
    /// disagreement is never exercised there — which is exactly why it needs a test.
    /// </remarks>
    [Fact]
    public void SizeRepresentsWidthDropsTheSquareRoot()
    {
        ChartDrawing drawing = Place(
            Bubbles([1.0, 9.0]) with { BubbleSizeRepresents = ChartBubbleSize.Width });

        (Diameter(drawing.Shapes[1]) / Diameter(drawing.Shapes[0])).ShouldBe(9.0, 0.001);
    }

    // ---------------------------------------------------------------- stock

    private static ChartPlot Stock(bool candles) => new()
    {
        Kind = ChartPlotKind.Stock,
        Categories = ["Mon", "Tue"],
        HasHighLowLines = true,
        HasUpDownBars = candles,
        Series =
        [
            new ChartSeries("Open", [10.0, 40.0]) { StockRole = ChartStockRole.Open },
            new ChartSeries("High", [50.0, 60.0]) { StockRole = ChartStockRole.High },
            new ChartSeries("Low", [5.0, 20.0]) { StockRole = ChartStockRole.Low },
            new ChartSeries("Close", [30.0, 25.0]) { StockRole = ChartStockRole.Close },
        ],
        ValueScale = new ChartScaleRequest(0.0, 60.0, 20.0),
    };

    /// <summary>
    /// A stock chart draws one whisker per category, from that category's low to its high.
    /// </summary>
    /// <remarks>
    /// Not four polylines. Three or four series are merged into one <c>VDataSeries</c> carrying
    /// four sequences and <c>CandleStickChart::createShapes</c> walks the <em>categories</em>,
    /// drawing a min-max line at each. Drawing the series instead puts four plausible lines on the
    /// page and no candles at all.
    /// </remarks>
    [Fact]
    public void AStockChartDrawsOneWhiskerPerCategoryAndNotAPolylinePerSeries()
    {
        ChartDrawing drawing = Place(Stock(candles: false));
        DocRect area = drawing.PlotArea;

        // Inside the plot rectangle on both axes, which is what tells a whisker from the
        // category axis' own tick at the boundary between two slots — that one is vertical too,
        // and it hangs *below* the plot area.
        List<ChartLine> vertical = [.. Whiskers(drawing)];

        vertical.Count.ShouldBe(2);

        // The first category's whisker spans 5 to 50 on a 0..60 axis.
        double top = Points(area.Bottom - vertical[0].To.Y) / Points(area.Height);
        double bottom = Points(area.Bottom - vertical[0].From.Y) / Points(area.Height);

        top.ShouldBe(50.0 / 60.0, 0.001);
        bottom.ShouldBe(5.0 / 60.0, 0.001);
    }

    /// <summary>
    /// <c>c:upDownBars</c> turns the open and close into a box, and a fall makes it dark.
    /// </summary>
    /// <remarks>
    /// <c>if(fUnscaledY_Last&lt;=fUnscaledY_First)</c> — a close equal to the open counts as a
    /// fall (<c>CandleStickChart.cxx:170-175</c>). Without the element there is no box and no
    /// opening mark at all, because the same element sets both <c>Japanese</c> and
    /// <c>ShowFirst</c> (<c>typegroupconverter.cxx:550-552</c>).
    /// </remarks>
    [Fact]
    public void UpDownBarsMakeABoxWhoseColourSaysWhichWayThePriceWent()
    {
        Place(Stock(candles: false)).Boxes.Count(box => box.Fill is not null).ShouldBe(0);

        ChartDrawing drawing = Place(Stock(candles: true));
        List<ChartBox> candles = [.. drawing.Boxes.Where(box => box.Fill is not null)];

        candles.Count.ShouldBe(2);

        // Monday opened at 10 and closed at 30 — a rise, so the hollow box; Tuesday opened at 40
        // and closed at 25 — a fall, so the solid one.
        candles[0].Fill.ShouldBe(Colour.White);
        candles[1].Fill.ShouldBe(Colour.Black);
    }

    /// <summary>
    /// Without <c>c:hiLowLines</c> there is no whisker, which is not the same as a defaulted one.
    /// </summary>
    /// <remarks>
    /// The importer sets the merged series' <c>LineStyle</c> to <c>NONE</c> when the element is
    /// absent, its own comment recording that "hi/low-lines cannot be switched off via ShowHighLow
    /// property" (<c>typegroupconverter.cxx:543-546</c>).
    /// </remarks>
    [Fact]
    public void WithoutHighLowLinesThereIsNoWhisker()
    {
        ChartDrawing drawing = Place(Stock(candles: true) with { HasHighLowLines = false });

        Whiskers(drawing).Count.ShouldBe(0);
    }

    /// <summary>The vertical lines strictly inside the plot rectangle: the candles' whiskers.</summary>
    private static List<ChartLine> Whiskers(ChartDrawing drawing)
    {
        DocRect area = drawing.PlotArea;

        return
        [
            .. drawing.Lines.Where(line =>
                line.From.X == line.To.X
                && line.From.X > area.Left && line.From.X < area.Right
                && line.From.Y >= area.Top && line.From.Y <= area.Bottom
                && line.To.Y >= area.Top && line.To.Y <= area.Bottom),
        ];
    }

    // --------------------------------------------------------------- of-pie

    private static ChartPlot OfPie(ChartOfPieType type, int split) => new()
    {
        Kind = ChartPlotKind.OfPie,
        OfPieType = type,
        SplitType = ChartSplitType.Position,
        SplitPosition = split,
        Series =
        [
            new ChartSeries("S", [9.0, 8.0, 7.0, 6.0, 5.0, 4.0], Colour.FromRgb(0x004586)),
        ],
    };

    /// <summary>
    /// The split takes the series' <em>last</em> points, and the main pie gains a composite wedge.
    /// </summary>
    /// <remarks>
    /// <c>OfPieDataSrc::getNPoints</c> is <c>total − splitPos + 1</c> for the main pie and
    /// <c>splitPos</c> for the second, and <c>getData</c>'s last main point is the sum of the
    /// ones that left (<c>PieChart.cxx:2307-2339</c>). So a six-point series split at two draws
    /// five wedges on the left and two on the right — seven paths for six numbers.
    /// </remarks>
    [Fact]
    public void AnOfPieSplitsTheLastPointsIntoASecondPie()
    {
        ChartDrawing drawing = Place(OfPie(ChartOfPieType.Pie, 2));

        // Five wedges left, two right; the two connecting lines are lines, not paths.
        drawing.Shapes.Count.ShouldBe(7);
        drawing.Lines.Count.ShouldBe(2);
    }

    /// <summary>
    /// A bar-of-pie's tail is one stacked bar, and its segments together are the full bar height.
    /// </summary>
    /// <remarks>
    /// <c>createOneBar</c> runs the bar from −0.5 to +0.5 of the unit radius and divides it by
    /// each point's share of the tail's sum (<c>PieChart.cxx:1400-1432</c>), between
    /// <c>m_fBarLeft = 0.75</c> and <c>m_fBarRight = 1.25</c> (<c>PieChart.hxx:267-269</c>).
    /// </remarks>
    [Fact]
    public void ABarOfPiesTailIsOneStackedBar()
    {
        ChartDrawing drawing = Place(OfPie(ChartOfPieType.Bar, 3));

        // Four wedges — three kept plus the composite — and three bar segments.
        drawing.Shapes.Count.ShouldBe(7);

        List<ChartShape> segments = [.. drawing.Shapes.Skip(4)];
        DocRect area = drawing.PlotArea;
        Length unit = area.Width < area.Height ? area.Width / 2 : area.Height / 2;

        double total = 0.0;
        foreach (ChartShape segment in segments)
        {
            List<DocPoint> points =
                [.. segment.Path.Commands.Where(c => c.Verb != PathVerb.Close).Select(c => c.Point)];

            total += Points(points.Max(p => p.Y) - points.Min(p => p.Y));

            // Every segment spans the same half-unit-radius width.
            Points(points.Max(p => p.X) - points.Min(p => p.X)).ShouldBe(Points(unit * 0.5), 0.01);
        }

        total.ShouldBe(Points(unit), 0.01);
    }

    /// <summary>
    /// Too few points and an of-pie falls back to an ordinary pie rather than splitting.
    /// </summary>
    /// <remarks>
    /// <c>OfPieDataSrc::minPoints = 4</c> (<c>PieChart.hxx:108</c>), tested before the sub-type is
    /// chosen at all (<c>PieChart.cxx:1052-1056</c>).
    /// </remarks>
    [Fact]
    public void AnOfPieWithTooFewPointsIsAnOrdinaryPie()
    {
        ChartPlot plot = OfPie(ChartOfPieType.Pie, 2) with
        {
            Series = [new ChartSeries("S", [3.0, 2.0, 1.0], Colour.FromRgb(0x004586))],
        };

        ChartDrawing drawing = Place(plot);

        drawing.Shapes.Count.ShouldBe(3);
        drawing.Lines.Count.ShouldBe(0);
    }

    /// <summary>
    /// A pie whose chart part states no categories numbers its legend entries from one.
    /// </summary>
    /// <remarks>
    /// <c>barOfPieChart.xlsx</c> and <c>pieOfPieChart.xlsx</c> state a <c>c:val</c> and no
    /// <c>c:cat</c>, and LibreOffice's PDF draws a legend reading <c>1 2 … 9</c> against a sheet
    /// whose own cells read <c>9 8 … 1</c> — the generated categories of
    /// <c>ExplicitCategoriesProvider</c>. Skipping the unnamed ones instead draws no legend at
    /// all.
    /// </remarks>
    [Fact]
    public void APieWithNoStatedCategoriesNumbersItsLegend()
    {
        ChartDrawing drawing = Place(
            OfPie(ChartOfPieType.Bar, 2) with { Legend = ChartLegendPosition.Right });

        List<string> entries = [.. drawing.Labels.Select(label => label.Text)];
        entries.ShouldBe(["1", "2", "3", "4", "5", "6"]);
    }
}
