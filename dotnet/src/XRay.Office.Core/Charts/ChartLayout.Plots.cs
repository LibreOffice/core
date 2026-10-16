using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;

namespace XRay.Office.Core.Charts;

/// <summary>
/// The four plot types that are not a rectangle, a polyline or a wedge: radar, bubble, stock and
/// of-pie.
/// </summary>
/// <remarks>
/// <para>
/// A separate half of <see cref="ChartLayout"/> rather than more of the same file, because these
/// four share nothing with the cartesian geometry but the plot rectangle and the resolved scale:
/// two of them are polar, one has a third dimension and one draws four numbers per category
/// instead of one.
/// </para>
/// <para>
/// <strong>There is no fifth type here, and the paragraphs that used to say otherwise are
/// withdrawn.</strong> This file argued at length that <em>"a surface chart's frame stays
/// empty"</em> and that drawing one as a bar chart was <em>"the tempting shortcut"</em>. Whatever
/// the merits of that when it was written, it has not described the code for several rounds:
/// <c>XRay.Office.Ooxml.DrawingML.DrawingChartPlot.KindOf</c> maps <c>c:surfaceChart</c> and
/// <c>c:surface3DChart</c> onto <see cref="ChartPlotKind.Bar"/>, and
/// <c>XRay.Office.OpenDocument.OdfChartPlot.KindOf</c> does the same for
/// <c>chart:class="chart:surface"</c>. Nothing here draws a surface chart's frame empty. The
/// measurement that overturned the older view is recorded on the OOXML reader's own
/// <c>KindOf</c> and is not repeated.
/// </para>
/// <para>
/// <strong>What the withdrawn argument was reaching for is true and is a different claim:
/// <c>Surface</c> has never been a chart type in LibreOffice, in either version.</strong>
/// <c>template.Surface</c> is commented out of the registry
/// (<c>chart2/source/model/template/ChartTypeManager.cxx</c>:223, this tree) and there is no
/// <c>SurfaceChartType</c> among the nineteen services in
/// <c>chart2/source/inc/servicenames_charttypes.hxx</c>:23-59; grepping 26.2.4.2's own
/// <c>libmergedlo.so</c> and <c>services.rdb</c> finds neither name there either. All three of
/// its importers substitute a column chart with the same <c>// Todo</c> — OOXML at
/// <c>oox/source/drawingml/chart/typegroupconverter.cxx</c>:79 with <em>"create a deep 3D bar
/// chart from surface charts"</em> at <c>:217-219</c>, BIFF at
/// <c>sc/source/filter/excel/xlchart.cxx</c>:469, and ODF at
/// <c>xmloff/source/chart/SchXMLTools.cxx</c>:148 with <em>"@todo change this if a surface chart
/// is available"</em>. So reading one as a bar chart is not a shortcut past the reference; it is
/// the reference's own answer.
/// </para>
/// <para>
/// <strong>Its reach is nil in all four streams, twice over</strong>, which is why none of this
/// costs anything either way: <c>0 of 766</c> OOXML documents state <c>c:surfaceChart</c> or
/// <c>c:surface3DChart</c>, <c>0 of 947</c> of 26.2.4.2's own ODF exports state
/// <c>chart:class="chart:surface"</c>, and no <c>CHSURFACE</c> (0x103F) record appears in any of
/// the 64 <c>.xls</c> or the 51 <c>.ppt</c>. <c>probes/chart-types-r101</c> holds both censuses.
/// </para>
/// </remarks>
public static partial class ChartLayout
{
    /// <summary>
    /// How many major intervals a radar chart's radius axis may have.
    /// </summary>
    /// <remarks>
    /// <c>VPolarRadiusAxis::estimateMaximumAutoMainIncrementCount</c> returns a flat <c>2</c>
    /// (<c>chart2/source/view/axes/VPolarRadiusAxis.cxx:87-90</c>) where
    /// <c>VCartesianAxis</c>'s derives a count from the axis' own length. So a radar chart's web
    /// has three rings — the centre, the middle and the rim — however large it is drawn.
    /// </remarks>
    private const int RadarIntervalCount = 2;

    /// <summary>
    /// A radar chart's series: one closed polygon per series around a polar category axis.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>NetChart</c>, which is the line and area plotter over a
    /// <c>PolarPlottingPositionHelper</c> rather than a cartesian one
    /// (<c>chart2/source/view/charttypes/NetChart.cxx</c>). Two things it does that a line chart
    /// does not: it <em>closes</em> the polygon, joining the last point back to the first
    /// (<c>impl_createLine</c>'s "connect last point in last polygon with first point in first
    /// polygon"), and it places each point by an angle rather than by a distance along an edge.
    /// </para>
    /// <para>
    /// <strong>The first category is at twelve o'clock and they run clockwise</strong>, the same
    /// convention a pie uses and for the same reason — both go through
    /// <c>transformToAngleDegree</c> with the pie's 90° offset. Measured on
    /// <c>chart2/qa/extras/data/docx/radar-chart-labels.docx</c>, whose five categories put the
    /// web's vertices at 90°, 18°, −54°, −126° and 162° from a centre at (261.9, 582.6) in
    /// LibreOffice's own PDF — 72° apart, clockwise, starting straight up.
    /// </para>
    /// </remarks>
    private static void AddRadar(
        ChartPlot plot,
        DocRect area,
        ChartScaleResult scale,
        int categories,
        List<ChartShape> shapes,
        List<ChartLabel> labels)
    {
        if (categories <= 0 || plot.Series.Count == 0) return;
        if (RadiusOf(area) is not { } web) return;

        foreach (ChartSeries series in plot.Series)
        {
            List<(DocPoint At, int Index, double Value)> points = [];

            for (int at = 0; at < categories && at < series.Values.Count; at++)
            {
                if (series.Values[at] is not { } value || !double.IsFinite(value)) continue;
                points.Add((RadarPoint(web, at, categories, scale.Fraction(value)), at, value));
            }

            if (points.Count == 0) continue;

            // A filled radar is one closed region; a standard or marker one is a closed stroke.
            // NetChart is constructed with bNoArea from the chart type and draws one or the other,
            // never both (NetChart.cxx:49-56).
            bool filled = plot.RadarStyle is ChartRadarStyle.Filled;

            if (points.Count > 1)
            {
                GraphicsPath path = new();
                path.MoveTo(points[0].At);
                for (int at = 1; at < points.Count; at++) path.LineTo(points[at].At);
                path.Close();

                shapes.Add(filled
                    ? new ChartShape(path, series.Fill, series.Line, series.LineWidth)
                    : new ChartShape(path, null, series.Line ?? series.Fill, series.LineWidth));
            }

            if (series.Marker is not ChartMarker.None)
            {
                Length size = series.MarkerSize ?? plot.LabelSize * MarkerSize;
                Colour fill = series.MarkerFill ?? series.Fill ?? series.Line ?? AxisColour;
                Colour stroke = series.MarkerLine ?? series.Line ?? fill;

                foreach ((DocPoint at, _, _) in points)
                    shapes.Add(Marker(series.Marker, at, size, fill, stroke));
            }

            AddPointLabels(plot, series, points, ChartLabelPlacement.Top, area, labels);
        }
    }

    /// <summary>
    /// A radar chart's two axes: the web at each major tick and a spoke per category.
    /// </summary>
    /// <remarks>
    /// <c>VPolarGrid</c> and <c>VPolarAngleAxis</c>. The web is the value axis' major gridlines
    /// bent into closed polygons — one per tick, the outermost of which is also the plot area's
    /// boundary — and the spokes are the category axis, one line from the centre to each vertex.
    /// The value axis itself is the spoke of the <em>first</em> category, drawn straight up, and
    /// its tick labels sit along it.
    /// </remarks>
    private static void AddRadarAxis(
        ChartPlot plot,
        DocRect area,
        ChartScaleResult scale,
        int categories,
        List<ChartLine> lines,
        List<ChartLabel> labels)
    {
        if (categories <= 0) return;
        if (RadiusOf(area) is not { } web) return;

        Colour grid = plot.ValueGrid?.Colour ?? plot.CategoryGrid?.Colour ?? AxisColour;

        foreach (double tick in scale.MajorTicks())
        {
            double along = scale.Fraction(tick);
            if (along <= 0.0) continue;

            for (int at = 0; at < categories; at++)
            {
                lines.Add(new ChartLine(
                    RadarPoint(web, at, categories, along),
                    RadarPoint(web, at + 1, categories, along),
                    grid));
            }
        }

        if (!plot.CategoryAxisVisible && !plot.ValueAxisVisible) return;

        for (int at = 0; at < categories; at++)
        {
            DocPoint rim = RadarPoint(web, at, categories, 1.0);

            if (plot.CategoryAxisVisible || at == 0)
                lines.Add(new ChartLine(web.Centre, rim, AxisColour));

            if (!plot.CategoryAxisVisible) continue;
            if (at >= plot.Categories.Count) continue;
            if (ChartDataLabel.WriteCategory(plot.Categories[at], plot.CategoryFormat)
                is not { Length: > 0 } text)
            {
                continue;
            }

            // The label sits just beyond its own vertex, on the same ray. Anchoring every one of
            // them centred is what LibreOffice does for a polar category axis, whose labels are
            // placed by the ray and not by a side of the plot area.
            DocPoint beyond = RadarPoint(web, at, categories, 1.0 + RadarLabelReach);
            labels.Add(new ChartLabel(
                text, beyond, ChartLabelAnchor.Centre, plot.LabelSize, plot.LabelColour));
        }

        if (!plot.ValueAxisVisible) return;

        foreach (double tick in scale.MajorTicks())
        {
            DocPoint at = RadarPoint(web, 0, categories, scale.Fraction(tick));
            labels.Add(new ChartLabel(
                ChartDataLabel.Write(tick, plot.ValueFormat),
                new DocPoint(at.X - LabelSpacing, at.Y),
                ChartLabelAnchor.RightMiddle,
                plot.LabelSize,
                plot.LabelColour));
        }
    }

    /// <summary>How far beyond the rim a radar chart's category label sits, as a fraction.</summary>
    private const double RadarLabelReach = 0.12;

    /// <summary>The circle a polar chart is drawn in: the plot area's centre and half its lesser side.</summary>
    private static (DocPoint Centre, Length Radius)? RadiusOf(DocRect area)
    {
        Length radius = area.Width < area.Height ? area.Width / 2 : area.Height / 2;
        return radius <= Length.Zero
            ? null
            : (new DocPoint(area.X + area.Width / 2, area.Y + area.Height / 2), radius);
    }

    /// <summary>Where one category's point sits at a given fraction of the radius.</summary>
    private static DocPoint RadarPoint(
        (DocPoint Centre, Length Radius) web, int index, int categories, double along)
    {
        double angle = Math.PI / 2 - (2 * Math.PI * index / categories);
        Length reach = web.Radius * along;

        return new DocPoint(
            web.Centre.X + reach * Math.Cos(angle),
            web.Centre.Y - reach * Math.Sin(angle));
    }

    /// <summary>
    /// The largest bubble's diameter, as a fraction of the plot area's lesser side.
    /// </summary>
    /// <remarks>
    /// <c>BubbleChart::calculateBubbleSizeScalingFactor</c> sets
    /// <c>m_fBubbleSizeFactorToScreen = nMinExtend * 0.25</c> and comments it "max bubble size is
    /// 25 percent of diagram size" (<c>BubbleChart.cxx:80-95</c>). That factor is the
    /// <em>diameter</em> and not the radius: it becomes the <c>Direction3D</c> handed to
    /// <c>ShapeFactory::createCircle2D</c>, which uses it as the shape's size and offsets the
    /// centre by half of it (<c>ShapeFactory.cxx:1729-1734</c>). Reading it as a radius draws
    /// every bubble twice as wide as the reference, which looks like a plausible chart with an
    /// over-enthusiastic scale.
    /// </remarks>
    private const double BubbleMaximumExtent = 0.25;

    /// <summary>
    /// A bubble chart: a circle at each (x, y), sized by a third number.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>The size is an area, so the diameter goes as its square root.</strong>
    /// <c>transformToScreenBubbleSize</c> computes <c>sqrt(size / π) / sqrt(max / π)</c> and
    /// multiplies the screen factor by it (<c>BubbleChart.cxx:97-113</c>) — the two π cancel, so
    /// what survives is <c>√(size / max)</c>. A series running 1 … 9 therefore draws its largest
    /// bubble three times the width of its smallest and nine times its area. Taking the ratio
    /// directly instead draws nine times the width and eighty-one times the area, which is a
    /// picture that reads perfectly well and is wrong by the square of the data.
    /// </para>
    /// <para>
    /// See <see cref="ChartBubbleSize"/> for <c>c:sizeRepresents</c> and <c>c:bubbleScale</c>,
    /// which LibreOffice parses and never uses, and for why they are honoured here anyway.
    /// </para>
    /// </remarks>
    private static void AddBubbles(
        ChartPlot plot,
        DocRect area,
        ChartScaleResult scale,
        ChartScaleResult? domain,
        List<ChartShape> shapes,
        List<ChartLabel> labels)
    {
        if (plot.Series.Count == 0) return;

        // One maximum over every series of the plot, not one per series — the loop in
        // calculateMaximumLogicBubbleSize walks every z slot, x slot and series before any shape
        // is made (BubbleChart.cxx:56-79), so two series share a scale and the larger one's
        // bubbles are larger.
        double largest = 0.0;
        foreach (ChartSeries series in plot.Series)
        {
            if (series.SizeValues is not { } sizes) continue;
            foreach (double? size in sizes)
            {
                if (size is not { } stated || !double.IsFinite(stated)) continue;
                if (stated > largest) largest = stated;
            }
        }

        if (!(largest > 0.0)) return;

        Length extent = area.Width < area.Height ? area.Width : area.Height;
        Length widest = extent * (BubbleMaximumExtent * Math.Clamp(plot.BubbleScale / 100.0, 0.0, 3.0));
        if (widest <= Length.Zero) return;

        foreach (ChartSeries series in plot.Series)
        {
            if (series.SizeValues is not { } sizes) continue;

            List<(DocPoint At, int Index, double Value)> points = [];

            for (int at = 0; at < series.Values.Count; at++)
            {
                if (series.Values[at] is not { } y || !double.IsFinite(y)) continue;
                if (at >= sizes.Count) continue;
                if (sizes[at] is not { } size || !double.IsFinite(size)) continue;

                // A negative size is skipped unless the series states c:invertIfNegative, and a
                // zero one always is — BubbleChart.cxx:237-246, three consecutive continues.
                if (size < 0.0)
                {
                    if (!series.InvertIfNegative) continue;
                    size = -size;
                }

                if (size == 0.0) continue;

                double across = domain is { } span && series.XValues is { } xs
                                && at < xs.Count && xs[at] is { } x && double.IsFinite(x)
                    ? span.Fraction(x)
                    : (series.Values.Count == 1 ? 0.5 : (double)at / (series.Values.Count - 1));

                DocPoint centre = Point(area, across, scale.Fraction(y), true);

                double relative = plot.BubbleSizeRepresents is ChartBubbleSize.Width
                    ? size / largest
                    : Math.Sqrt(size / largest);

                shapes.Add(new ChartShape(
                    Circle(centre, widest * (relative / 2)),
                    series.FillAt(at),
                    series.Line,
                    series.LineWidth));

                points.Add((centre, at, y));
            }

            AddPointLabels(plot, series, points, ChartLabelPlacement.Top, area, labels);
        }
    }

    /// <summary>One circle, as four cubics — the standard 0.5523 approximation.</summary>
    private static GraphicsPath Circle(DocPoint at, Length radius)
    {
        Length k = radius * 0.5522847498307936;
        GraphicsPath path = new();

        path.MoveTo(new DocPoint(at.X + radius, at.Y));
        path.CubicTo(
            new DocPoint(at.X + radius, at.Y + k), new DocPoint(at.X + k, at.Y + radius),
            new DocPoint(at.X, at.Y + radius));
        path.CubicTo(
            new DocPoint(at.X - k, at.Y + radius), new DocPoint(at.X - radius, at.Y + k),
            new DocPoint(at.X - radius, at.Y));
        path.CubicTo(
            new DocPoint(at.X - radius, at.Y - k), new DocPoint(at.X - k, at.Y - radius),
            new DocPoint(at.X, at.Y - radius));
        path.CubicTo(
            new DocPoint(at.X + k, at.Y - radius), new DocPoint(at.X + radius, at.Y - k),
            new DocPoint(at.X + radius, at.Y));
        path.Close();

        return path;
    }

    /// <summary>chart2's fill for a candle that closed above its open.</summary>
    /// <remarks>
    /// The "white day" box. <c>CandleStickChartTypeTemplate</c> creates <c>WhiteDay</c> with a
    /// white fill and a black outline and <c>BlackDay</c> with a black fill, which is what makes
    /// a falling candle solid and a rising one hollow.
    /// </remarks>
    private static readonly Colour StockGain = Colour.White;

    /// <summary>chart2's fill for a candle that closed at or below its open.</summary>
    private static readonly Colour StockLoss = Colour.Black;

    /// <summary>
    /// A stock chart: one whisker per category, with either a box or two ticks across it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>CandleStickChart::createShapes</c> (<c>chart2/source/view/charttypes/CandleStickChart.cxx</c>).
    /// A stock chart is <em>not</em> a shape per point of a series: three or four ordinary series
    /// are merged into one, and what is drawn per category is a line from that category's low to
    /// its high plus a mark for its open and its close. Drawing the four series as four lines
    /// instead — which is what a reader that does not know the type does — puts four plausible
    /// polylines on the page and no candles at all.
    /// </para>
    /// <para>
    /// <strong>Which mark depends on one element being present.</strong> With
    /// <c>c:upDownBars</c> the open and close become a filled box spanning the category slot,
    /// white when the close is above the open and black when it is not, and the swap is
    /// <c>if(fUnscaledY_Last&lt;=fUnscaledY_First)</c> — equality counts as a fall
    /// (<c>CandleStickChart.cxx:170-175</c>). Without it they become a tick to the left of the
    /// whisker at the open and one to the right at the close, and the open tick is drawn only
    /// when <c>ShowFirst</c> is set, which the same element sets. Without <c>c:hiLowLines</c>
    /// there is no whisker at all — the importer sets the merged series' line style to
    /// <c>NONE</c> rather than defaulting it (<c>typegroupconverter.cxx:543-546</c>).
    /// </para>
    /// </remarks>
    private static void AddCandles(
        ChartPlot plot,
        DocRect area,
        ChartScaleResult scale,
        int categories,
        List<ChartShape> shapes,
        List<ChartLine> lines,
        List<ChartLabel> labels)
    {
        if (categories <= 0 || plot.Series.Count == 0) return;

        ChartSeries? open = RoleOf(plot, ChartStockRole.Open);
        ChartSeries? high = RoleOf(plot, ChartStockRole.High);
        ChartSeries? low = RoleOf(plot, ChartStockRole.Low);
        ChartSeries? close = RoleOf(plot, ChartStockRole.Close);

        // A slot holds one candle plus the gap, which is CategoryPositionHelper's arithmetic with
        // a single series: slot / (1 + gapWidth/100). The default gap is the 100 both formats
        // default to, and c:upDownBars states its own — 150 in the corpus file, which makes the
        // box two fifths of the slot.
        Length slot = area.Width / categories;
        double outer = Math.Clamp(plot.GapWidth / 100.0, 0.0, 6.0);
        Length candle = slot / (1.0 + outer);

        Colour stroke = high?.Line ?? low?.Line ?? AxisColour;
        Length width = high?.LineWidth ?? Length.Zero;

        for (int at = 0; at < categories; at++)
        {
            double? highest = At(high, at);
            double? lowest = At(low, at);
            double? first = At(open, at);
            double? last = At(close, at);

            Length middle = area.Left + slot * (at + 0.5);

            if (plot.HasHighLowLines && highest is { } top && lowest is { } bottom)
            {
                if (top < bottom) (top, bottom) = (bottom, top);

                lines.Add(new ChartLine(
                    new DocPoint(middle, area.Bottom - area.Height * scale.Fraction(bottom)),
                    new DocPoint(middle, area.Bottom - area.Height * scale.Fraction(top)),
                    stroke,
                    width));
            }

            if (plot.HasUpDownBars && first is { } opened && last is { } closed)
            {
                bool fell = closed <= opened;
                if (fell) (opened, closed) = (closed, opened);

                Length upper = area.Bottom - area.Height * scale.Fraction(closed);
                Length lower = area.Bottom - area.Height * scale.Fraction(opened);

                // A path rather than a ChartBox, for the reason ChartDrawing.Shapes gives: a
                // candle is a series mark and paints over the grid, not under it.
                shapes.Add(new ChartShape(
                    GraphicsPath.Rectangle(
                        new DocRect(middle - candle / 2, upper, candle, lower - upper)),
                    fell
                        ? plot.StockLossFill ?? StockLoss
                        : plot.StockGainFill ?? StockGain,
                    stroke,
                    width));
            }
            else
            {
                // The non-Japanese form: a tick left of the whisker at the open and one right of
                // it at the close. ShowFirst rides on the same element as the boxes, so a file
                // with neither draws the close alone.
                if (plot.HasUpDownBars && first is { } shownOpen)
                {
                    Length y = area.Bottom - area.Height * scale.Fraction(shownOpen);
                    lines.Add(new ChartLine(
                        new DocPoint(middle - candle / 2, y), new DocPoint(middle, y),
                        stroke, width));
                }

                if (last is { } shownClose)
                {
                    Length y = area.Bottom - area.Height * scale.Fraction(shownClose);
                    lines.Add(new ChartLine(
                        new DocPoint(middle, y), new DocPoint(middle + candle / 2, y),
                        stroke, width));
                }
            }

            // A stock chart's labels are the four numbers themselves, each against its own mark
            // (CandleStickChart.cxx:265-278). Only the close carries one here, because that is
            // the sequence a c:dLbls on the merged series belongs to.
            if (close is { } series && last is { } value)
            {
                Length y = area.Bottom - area.Height * scale.Fraction(value);
                AddPointLabels(
                    plot,
                    series,
                    [(new DocPoint(middle, y), at, value)],
                    ChartLabelPlacement.Top,
                    area,
                    labels);
            }
        }
    }

    /// <summary>The series carrying one of a stock plot's four numbers, or null when it has none.</summary>
    private static ChartSeries? RoleOf(ChartPlot plot, ChartStockRole role)
    {
        foreach (ChartSeries series in plot.Series)
            if (series.StockRole == role) return series;

        return null;
    }

    /// <summary>One point of a series, or null when it has none there.</summary>
    private static double? At(ChartSeries? series, int index)
    {
        if (series is null || index < 0 || index >= series.Values.Count) return null;
        return series.Values[index] is { } value && double.IsFinite(value) ? value : null;
    }

    /// <summary>How far the main pie's centre is shifted, as a fraction of the unit radius.</summary>
    /// <remarks><c>PieChart::m_fLeftShift = -0.75</c> (<c>PieChart.hxx:262</c>).</remarks>
    private const double OfPieLeftShift = -0.75;

    /// <summary>How far the second pie's centre is shifted.</summary>
    /// <remarks><c>m_fRightShift = 0.75</c>.</remarks>
    private const double OfPieRightShift = 0.75;

    /// <summary>The main pie's radius, as a fraction of the unit radius.</summary>
    /// <remarks><c>m_fLeftScale = 2.0/3</c>.</remarks>
    private const double OfPieLeftScale = 2.0 / 3.0;

    /// <summary>The second pie's radius.</summary>
    /// <remarks><c>m_fRightScale = 1.0/3</c>.</remarks>
    private const double OfPieRightScale = 1.0 / 3.0;

    /// <summary>The bar's left and right edges, and its height, in unit-radius units.</summary>
    /// <remarks><c>m_fBarLeft = 0.75</c>, <c>m_fBarRight = 1.25</c>, <c>m_fFullBarHeight = 1.0</c>.</remarks>
    private const double OfPieBarLeft = 0.75;

    private const double OfPieBarRight = 1.25;

    private const double OfPieBarHeight = 1.0;

    /// <summary>Where an of-pie's main ring opens, counter-clockwise from three o'clock.</summary>
    /// <remarks>
    /// <c>createOneRing</c>'s <c>sAngle</c> lambda (<c>PieChart.cxx</c>:1229-1244):
    /// <c>degAng = compositeVal * 360 / (ringSum * 2)</c>, and the offset is <c>360 − degAng</c>
    /// because a pie's wedges are clockwise. So the first kept wedge opens at minus half the
    /// composite sweep and the composite — drawn last — closes back onto it across three
    /// o'clock, which is what the two connecting lines meet.
    /// </remarks>
    /// <param name="composite">The composite wedge's value.</param>
    /// <param name="total">The ring's total.</param>
    private static double OfPieStartDegrees(double composite, double total)
        => total > 0.0 ? -composite / (total * 2.0) * 360.0 : 0.0;

    /// <summary>An of-pie main ring's label schedule: where it opens and how many points it
    /// labels.</summary>
    /// <param name="plot">The chart, for its split position.</param>
    /// <param name="series">The plotted series.</param>
    private static (double StartDegrees, int Kept) OfPieRingLabels(
        ChartPlot plot, ChartSeries series)
    {
        int points = series.Values.Count;
        int split = Math.Clamp(plot.SplitPosition, 1, Math.Max(1, points - 1));
        int kept = points - split;

        double composite = 0.0;
        for (int at = kept; at < points; at++)
            if (at >= 0 && series.Values[at] is { } value && double.IsFinite(value))
                composite += Math.Abs(value);

        return (OfPieStartDegrees(composite, series.Total()), kept);
    }

    /// <summary>LibreOffice's own twelve default series colours, cycled.</summary>
    /// <remarks>
    /// <c>Office.Chart/DefaultColor/Series</c>
    /// (<c>officecfg/registry/schema/org/openoffice/Office/Chart.xcs</c>:35-36), which
    /// <c>ConfigColorScheme::getColorByIndex</c> indexes modulo its own length
    /// (<c>chart2/source/tools/ConfigColorScheme.cxx</c>:126-133).
    /// </remarks>
    private static readonly Colour[] DefaultSeriesColours =
    [
        new Colour(0x00, 0x45, 0x86), new Colour(0xFF, 0x42, 0x0E),
        new Colour(0xFF, 0xD3, 0x20), new Colour(0x57, 0x9D, 0x1C),
        new Colour(0x7E, 0x00, 0x21), new Colour(0x83, 0xCA, 0xFF),
        new Colour(0x31, 0x40, 0x04), new Colour(0xAE, 0xCF, 0x00),
        new Colour(0x4B, 0x1F, 0x6F), new Colour(0xFF, 0x95, 0x0E),
        new Colour(0xC5, 0x00, 0x0B), new Colour(0x00, 0x84, 0xD1),
    ];

    /// <summary>The colour an of-pie's composite wedge is drawn in.</summary>
    /// <remarks>
    /// <para>
    /// <strong>Not the colour the file states for it, because the reference never sees that
    /// colour.</strong> The composite wedge's property index is <c>propIndex</c>'s
    /// <c>pSeries-&gt;getTotalPointCount()</c> (<c>PieChart.cxx</c>:1185-1191) — one past the
    /// last value, always — and <c>DataSeries::getDataPointByIndex</c> answers an empty
    /// reference outside <c>0 &lt;= nIndex &lt; getData().getLength()</c>
    /// (<c>chart2/source/model/main/DataSeries.cxx</c>:312-337). So the import drops the extra
    /// <c>c:dPt</c> a writer emits for that wedge, <c>hasPointOwnColor</c> answers false, and
    /// <c>createOneRing</c> falls through to
    /// <c>m_xColorScheme-&gt;getColorByIndex(nPropIdx)</c> (<c>:1325-1331</c>) — the twelve
    /// configured defaults above, cycled.
    /// </para>
    /// <para>
    /// <strong>It is a divergence from the writer's intent and is reproduced anyway.</strong>
    /// <c>028_Unit_Circle_Chart_Optimized_Graph</c> states a seventeenth
    /// <c>c:dPt idx="16"</c> of <c>accent4 lumMod 50%</c>, which is <c>#806000</c>, for a series
    /// of sixteen values; 26.2.4.2 draws that wedge <c>#7E0021</c>, which is
    /// <c>DefaultSeriesColours[16 % 12]</c> and has nothing to do with the document. Reading the
    /// stated fill instead would disagree with the reference on every of-pie chart that states
    /// one, and this tree is calibrated to the reference.
    /// </para>
    /// </remarks>
    /// <param name="points">The series' point count, which is the wedge's property index.</param>
    private static Colour CompositeWedgeColour(int points)
        => DefaultSeriesColours[
            ((points % DefaultSeriesColours.Length) + DefaultSeriesColours.Length)
            % DefaultSeriesColours.Length];

    /// <summary>The fewest points an of-pie chart is worth splitting.</summary>
    /// <remarks><c>OfPieDataSrc::minPoints = 4</c> (<c>PieChart.hxx:108</c>): a series with fewer
    /// falls back to an ordinary pie, which is <c>createShapes</c>' own first decision
    /// (<c>PieChart.cxx:1052-1056</c>).</remarks>
    private const int OfPieMinimumPoints = 4;

    /// <summary>
    /// An of-pie chart: a main pie whose last wedge is expanded into a second pie or a bar.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>PieChart::createShapes</c>' <c>PieChartSubType_PIE</c> and <c>_BAR</c> branches with
    /// <c>OfPieDataSrc</c> (<c>chart2/source/view/charttypes/PieChart.cxx:1050-1170, 2307-2360</c>).
    /// The split is positional: the <em>last</em> <c>c:splitPos</c> points leave the main pie and
    /// become the second plot, and the main pie gains one <em>composite</em> wedge equal to their
    /// sum. So a six-point series split at 2 draws five wedges on the left — four of its own and
    /// one worth the last two — and two on the right.
    /// </para>
    /// <para>
    /// <strong>The composite wedge faces the second plot, and that is what the start angle is
    /// for.</strong> The main pie starts at half the composite wedge's own width so that the
    /// wedge straddles three o'clock and the two connecting lines can meet it
    /// (<c>createOneRing</c>'s <c>sAngle</c> lambda, <c>PieChart.cxx:1228-1244</c>). Starting at
    /// twelve o'clock like an ordinary pie draws the connectors across the face of the pie.
    /// </para>
    /// <para>
    /// <strong>The oracle is 26.2.4.2 and it does draw one, so the split arithmetic is measured
    /// now and the radius is measured to be wrong.</strong> The paragraph that stood here said the
    /// geometry below could not be checked against any rendering, because the binary on the path
    /// was 24.2, which predates of-pie support. It is not: over 28 one-attribute variants of
    /// <c>029_Unit_Circle_Chart_Pie_Theme</c> put through 26.2.4.2
    /// (<c>probes/chart-rest-r104/o38-ofpie.tsv</c>), the reference's own wedge count is
    /// <em>n + 1</em> for the pie form at every split position and <em>n − split + 1</em> for the
    /// bar form — which is exactly <c>OfPieDataSrc</c>'s two arms — and the fallback below
    /// fires at exactly the same place it does: three points draw one plain pie and no connector,
    /// four draw two connectors and a second plot, in <em>both</em> forms.
    /// </para>
    /// <para>
    /// <strong>What is measured to be wrong is the unit radius, and it is left.</strong> On the
    /// corpus's one drawn of-pie, <c>028_Unit_Circle_Chart_Optimized_Graph</c>, 26.2.4.2's main
    /// ring is 181.0 pt across and centred at (202.8, 425.4) where this tree's is 247.5 × 256.4
    /// centred at (158.5, 372.5) — a unit radius 1.37 times the reference's — while on the eight-
    /// point variant of <c>029</c> the same ratio is 1.065 and on a plain pie it is 1.017. So the
    /// reference fits something wider than a unit circle into the plot rectangle and the factor is
    /// not a constant; <c>probes/chart-rest-r104</c> §4 states the hypothesis and says why it was
    /// not implemented from it.
    /// </para>
    /// </remarks>
    private static void AddOfPie(
        ChartPlot plot,
        DocRect area,
        DocRect available,
        ChartText measurer,
        List<ChartShape> shapes,
        List<ChartLine> lines,
        List<ChartLabel> labels)
    {
        if (plot.Series.Count == 0) return;
        if (RadiusOf(area) is not { } unit) return;

        ChartSeries series = plot.Series[0];
        int points = series.Values.Count;

        // Too few points to be worth splitting, and chart2 says so before anything else: the
        // whole plot falls back to an ordinary pie rather than to a pie of one wedge.
        int split = Math.Clamp(plot.SplitPosition, 1, Math.Max(1, points - 1));
        if (points < OfPieMinimumPoints)
        {
            AddWedges(plot, area, available, measurer, shapes, labels);
            return;
        }

        int kept = points - split;

        double Value(int at)
            => at >= 0 && at < points && series.Values[at] is { } stated && double.IsFinite(stated)
                ? Math.Abs(stated)
                : 0.0;

        double composite = 0.0;
        for (int at = kept; at < points; at++) composite += Value(at);

        double mainTotal = composite;
        for (int at = 0; at < kept; at++) mainTotal += Value(at);
        if (!(mainTotal > 0.0)) return;

        Length mainRadius = unit.Radius * OfPieLeftScale;
        DocPoint mainCentre = new(unit.Centre.X + unit.Radius * OfPieLeftShift, unit.Centre.Y);

        // The composite wedge straddles three o'clock, so the ring starts half of it below the
        // axis and runs clockwise. In a y-down space "clockwise" is a negative sweep.
        //
        // Below the axis, and the sign was the other way round until round 190. createOneRing's
        // sAngle lambda is `m_aPosHelper.clockwiseWedges() ? 360 - degAng : degAng`
        // (PieChart.cxx:1229-1244) with `degAng = compositeVal * 360 / (ringSum * 2)`, and a
        // pie's wedges are clockwise, so the first kept wedge opens at MINUS half the composite
        // sweep and the composite -- drawn last, after the other 360 - its own degrees -- closes
        // back onto it across three o'clock. Opening at plus half puts the whole composite above
        // the axis instead, which is where this tree drew 028's and where its two connecting
        // lines then met nothing: measured against 26.2.4.2, the reference's composite spans
        // 416.54 +/- 21.4 pt about a centre at y 416.54 and this tree's ran from the centre
        // upwards only.
        double start = Radians(OfPieStartDegrees(composite, mainTotal));

        for (int at = 0; at < kept; at++)
        {
            double sweep = Value(at) / mainTotal * (2 * Math.PI);
            if (sweep <= 0.0) continue;

            shapes.Add(new ChartShape(
                Wedge(mainCentre, mainRadius, start, -sweep),
                series.FillAt(at),
                series.Line,
                series.LineWidth));

            start -= sweep;
        }

        // The main ring's labels go through the pie's own placer -- the wrapping, the inner
        // best-fit and the outside fallback that createTextLabelShape gives every pie, of-pie
        // included (PieChart.cxx:1346-1350 skips only the composite wedge). AddWedgeLabel, which
        // stood here, put each label flat on its bisector at half the radius with no wrapping and
        // no fit, so an of-pie's labels neither wrapped nor moved out of their slices -- and,
        // because PieConsumedRect measures the same placement, the diagram's second pass had
        // nothing to shrink against.
        AddPieLabels(
            plot, series, mainCentre, mainRadius, mainRadius, available, measurer, shapes, labels,
            OfPieStartDegrees(composite, mainTotal), kept);

        // The composite wedge itself, which carries no label of its own — createOneRing skips
        // createTextLabelShape for exactly this point (PieChart.cxx:1341-1345).
        double compositeSweep = composite / mainTotal * (2 * Math.PI);
        if (compositeSweep > 0.0)
        {
            shapes.Add(new ChartShape(
                Wedge(mainCentre, mainRadius, start, -compositeSweep),
                CompositeWedgeColour(points),
                series.Line,
                series.LineWidth));
        }

        // Where the two connecting lines leave the main pie: the composite wedge's own corners.
        Length reach = mainRadius * Math.Cos(compositeSweep / 2.0);
        Length rise = mainRadius * Math.Sin(compositeSweep / 2.0);
        DocPoint from = new(mainCentre.X + reach, mainCentre.Y - rise);
        DocPoint under = new(mainCentre.X + reach, mainCentre.Y + rise);

        if (plot.OfPieType is ChartOfPieType.Bar) AddOfPieBar();
        else AddOfPieSecondPie();

        void AddOfPieSecondPie()
        {
            Length radius = unit.Radius * OfPieRightScale;
            DocPoint centre = new(unit.Centre.X + unit.Radius * OfPieRightShift, unit.Centre.Y);

            double total = 0.0;
            for (int at = kept; at < points; at++) total += Value(at);
            if (!(total > 0.0)) return;

            double angle = Math.PI / 2;
            for (int at = kept; at < points; at++)
            {
                double sweep = Value(at) / total * (2 * Math.PI);
                if (sweep <= 0.0) continue;

                shapes.Add(new ChartShape(
                    Wedge(centre, radius, angle, -sweep),
                    series.FillAt(at),
                    series.Line,
                    series.LineWidth));

                AddWedgeLabel(plot, series, at, total, centre, radius, angle, sweep, labels);
                angle -= sweep;
            }

            // The connectors run to the tangent points of the second circle, which is the pair of
            // equations PieChart.cxx:1131-1141 solves. Reduced to this space: the tangent from an
            // external point at distance d is at an angle acos(r/d) from the line of centres.
            double dx = (double)(centre.X - from.X).Emu;
            double dy = (double)(centre.Y - from.Y).Emu;
            double distance = Math.Sqrt(dx * dx + dy * dy);
            double r = (double)radius.Emu;
            if (distance <= r) return;

            double axis = Math.Atan2(dy, dx);
            double spread = Math.Acos(r / distance);

            DocPoint Tangent(double turn) => new(
                centre.X - radius * Math.Cos(axis + turn),
                centre.Y - radius * Math.Sin(axis + turn));

            lines.Add(new ChartLine(from, Tangent(-spread), AxisColour));
            lines.Add(new ChartLine(under, Tangent(spread), AxisColour));
        }

        void AddOfPieBar()
        {
            Length left = unit.Centre.X + unit.Radius * OfPieBarLeft;
            Length right = unit.Centre.X + unit.Radius * OfPieBarRight;
            Length half = unit.Radius * (OfPieBarHeight / 2.0);

            double total = 0.0;
            for (int at = kept; at < points; at++) total += Value(at);
            if (!(total > 0.0)) return;

            // The first split point is the BOTTOM segment. createOneBar opens at
            // `fBarTop = -0.5` and adds each share upwards -- "make the bar go from -0.5 to 0.5"
            // (PieChart.cxx:1416-1430) -- and the value axis points up, so data order runs from
            // the foot of the bar to its head. Stacking downwards from the top inverts every
            // bar of more than one segment: on 028 this tree drew Leaf 15 over Leaf 16 where
            // 26.2.4.2 draws Leaf 16 over Leaf 15, at the same two heights.
            Length top = unit.Centre.Y + half;

            for (int at = kept; at < points; at++)
            {
                Length height = half * 2 * (Value(at) / total);
                if (height <= Length.Zero) continue;

                top -= height;

                DocRect segment = new(left, top, right - left, height);
                shapes.Add(new ChartShape(
                    Rectangle(segment), series.FillAt(at), series.Line, series.LineWidth));

                if (series.LabelAt(at) is { Draws: true } label
                    && label.Compose(
                        CategoryTextAt(plot, at), series.Name, Value(at), total)
                        is { Length: > 0 } text)
                {
                    labels.Add(new ChartLabel(
                        text,
                        new DocPoint(left + (right - left) / 2, top + height / 2),
                        ChartLabelAnchor.Centre,
                        plot.DataLabelFont,
                        plot.DataLabelColour,
                        IsBold: plot.IsDataLabelBold));
                }
            }

            lines.Add(new ChartLine(from, new DocPoint(left, unit.Centre.Y - half), AxisColour));
            lines.Add(new ChartLine(under, new DocPoint(left, unit.Centre.Y + half), AxisColour));
        }
    }

    /// <summary>A rectangle as a closed path, for the of-pie bar's segments.</summary>
    private static GraphicsPath Rectangle(DocRect bounds)
    {
        GraphicsPath path = new();
        path.MoveTo(new DocPoint(bounds.Left, bounds.Top));
        path.LineTo(new DocPoint(bounds.Right, bounds.Top));
        path.LineTo(new DocPoint(bounds.Right, bounds.Bottom));
        path.LineTo(new DocPoint(bounds.Left, bounds.Bottom));
        path.Close();
        return path;
    }

    /// <summary>One wedge's label, on its own bisector — the same placement a pie uses.</summary>
    private static void AddWedgeLabel(
        ChartPlot plot,
        ChartSeries series,
        int index,
        double total,
        DocPoint centre,
        Length radius,
        double start,
        double sweep,
        List<ChartLabel> labels)
    {
        if (series.LabelAt(index) is not { Draws: true } label) return;

        if (series.Values[index] is not { } value || !double.IsFinite(value)) return;

        string? text = label.Compose(
            CategoryTextAt(plot, index), series.Name, value, total);

        if (text is not { Length: > 0 }) return;

        double middle = start - sweep / 2;
        double reach = label.Placement is ChartLabelPlacement.Outside ? 1.1 : 0.5;

        labels.Add(new ChartLabel(
            text,
            new DocPoint(
                centre.X + radius * (reach * Math.Cos(middle)),
                centre.Y - radius * (reach * Math.Sin(middle))),
            ChartLabelAnchor.Centre,
            plot.DataLabelFont,
            plot.DataLabelColour,
            IsBold: plot.IsDataLabelBold));
    }
}
