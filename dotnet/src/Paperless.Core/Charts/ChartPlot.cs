using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Core.Charts;

/// <summary>Which way a bar chart's bars run.</summary>
public enum ChartBarDirection
{
    /// <summary>Vertical bars growing up from a horizontal category axis — <c>c:barDir val="col"</c>.</summary>
    Column = 0,

    /// <summary>Horizontal bars growing right from a vertical category axis — <c>val="bar"</c>.</summary>
    Bar,
}

/// <summary>What geometry a chart's series are drawn as.</summary>
/// <remarks>
/// Matched by element name in both vocabularies rather than taken from the first plot group,
/// because a chart part does not otherwise say what it is in any way a suffix match can see —
/// and drawing a pie with the bar engine produced eighty-one words of category and value-axis
/// labels for a chart that has no axes at all.
/// </remarks>
public enum ChartPlotKind
{
    /// <summary>Rectangles against a category axis — <c>c:barChart</c>, <c>chart:bar</c>.</summary>
    Bar = 0,

    /// <summary>A polyline through the points — <c>c:lineChart</c>, <c>chart:line</c>.</summary>
    Line,

    /// <summary>Wedges of a circle — <c>c:pieChart</c>, <c>chart:circle</c>.</summary>
    Pie,

    /// <summary>A filled region under the line — <c>c:areaChart</c>, <c>chart:area</c>.</summary>
    Area,

    /// <summary>Markers at (x, y) — <c>c:scatterChart</c>, <c>chart:scatter</c>.</summary>
    Scatter,
}

/// <summary>Where the legend sits relative to the plot area.</summary>
public enum ChartLegendPosition
{
    /// <summary>No legend at all.</summary>
    None = 0,

    /// <summary>To the right — OOXML <c>r</c>, ODF <c>end</c>. The default both write.</summary>
    Right,

    /// <summary>To the left — <c>l</c>, <c>start</c>.</summary>
    Left,

    /// <summary>Above — <c>t</c>, <c>top</c>.</summary>
    Top,

    /// <summary>Below — <c>b</c>, <c>bottom</c>.</summary>
    Bottom,
}

/// <summary>
/// One series of a chart, ready to draw: its name, its colours and its numbers.
/// </summary>
/// <param name="Name">The series' label, as the legend and any data label show it.</param>
/// <param name="Values">
/// The cached values, one per category and indexed by category. Null is a genuine gap — a
/// category the series has no value for — and is skipped rather than drawn as zero, which is
/// what <c>c:dispBlanksAs val="gap"</c> asks for and what LibreOffice does by default.
/// </param>
/// <param name="Fill">The bar or marker fill, or null when the file states none.</param>
/// <param name="Line">The outline colour, or null for none.</param>
/// <param name="LineWidth">The outline width; zero means a hairline, which is what OOXML's
/// <c>a:ln w="0"</c> means and what LibreOffice draws as the thinnest line the device has.</param>
/// <param name="PointFills">
/// A fill per point where the file states one, or null where the series' own fill serves.
/// </param>
/// <param name="Kind">
/// What this series is drawn as when it differs from the chart's own kind, or null when it does
/// not. Set only by a chart part holding several plot groups — a column chart with a line over it
/// writes a <c>c:barChart</c> and a <c>c:lineChart</c> sharing one pair of axes — and it is what
/// lets all of them be drawn instead of only the first.
/// </param>
public sealed record ChartSeries(
    string? Name,
    IReadOnlyList<double?> Values,
    Colour? Fill = null,
    Colour? Line = null,
    Length LineWidth = default,
    IReadOnlyList<Colour?>? PointFills = null,
    ChartPlotKind? Kind = null)
{
    /// <summary>
    /// The fill one point is drawn in: its own where the file states one, the series' otherwise.
    /// </summary>
    /// <remarks>
    /// <strong>A pie's colours belong to its points and every other chart's belong to its
    /// series.</strong> A pie plots one series and needs a colour per wedge, which both formats
    /// state as a per-point override — <c>c:dPt/c:spPr</c>, ODF's <c>chart:data-point</c> with a
    /// style of its own. Falling back to the series' single fill draws a pie in one colour, which
    /// reads as a rendering failure rather than as a missing feature.
    /// </remarks>
    /// <param name="index">The point's index.</param>
    public Colour? FillAt(int index)
        => PointFills is { } fills && index >= 0 && index < fills.Count && fills[index] is { } own
            ? own
            : Fill;
}

/// <summary>
/// A chart reduced to what drawing it needs, in neither vocabulary.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Why a second model beside <c>DrawingChart</c>.</strong> That one answers "what
/// words does this chart contain", which is a content-tree question and needs no geometry at
/// all. This one answers "where does every mark go", which needs the axis scaling, the plot
/// rectangle, the fills and the bar arithmetic and needs none of the content tree. Merging them
/// would make every extraction pay for geometry it never looks at, which the project's
/// extraction/rendering split exists to prevent.
/// </para>
/// <para>
/// <strong>Both vocabularies reach this shape, and one of them brings the answer with it.</strong>
/// ODF writes <c>chart:plot-area/chart:coordinate-region</c> — the exact inner plot rectangle
/// LibreOffice last computed — into the file. OOXML writes nothing of the kind unless the author
/// dragged the plot area, in which case it is <c>c:plotArea/c:layout/c:manualLayout</c>. So
/// <see cref="PlotArea"/> is set from the file when the file says, and computed when it does
/// not; see <see cref="ChartLayout"/> for what the computation costs in accuracy.
/// </para>
/// </remarks>
public sealed record ChartPlot
{
    /// <summary>The chart's title, or null when it states none.</summary>
    public string? Title { get; init; }

    /// <summary>The category axis' title, or null.</summary>
    public string? CategoryAxisTitle { get; init; }

    /// <summary>The value axis' title, or null.</summary>
    public string? ValueAxisTitle { get; init; }

    /// <summary>The category labels, in order. Empty for a chart with no category axis.</summary>
    public IReadOnlyList<string?> Categories { get; init; } = [];

    /// <summary>The series, in the order the file states them, which is drawing order.</summary>
    public IReadOnlyList<ChartSeries> Series { get; init; } = [];

    /// <summary>What the series are drawn as.</summary>
    public ChartPlotKind Kind { get; init; }

    /// <summary>Which way the bars run.</summary>
    public ChartBarDirection Direction { get; init; }

    /// <summary>Whether the chart has a pair of axes at all.</summary>
    /// <remarks>
    /// A pie has neither, so it gets no axis lines, no ticks, no gridlines and — the part that
    /// matters most for a word count — no tick labels, which is the eighty-one words the first
    /// version of the reader invented for <c>PieChartWithAutomaticLayout_SizeAndPosition.pptx</c>.
    /// </remarks>
    public bool HasAxes => Kind is not ChartPlotKind.Pie;

    /// <summary>
    /// Whether the category axis is divided into slots rather than marked at points.
    /// </summary>
    /// <remarks>
    /// <c>ScaleData::ShiftedCategoryPosition</c>, which
    /// <c>ChartTypeTemplate::adaptScales</c> sets for a column or bar chart and for nothing else
    /// (<c>chart2/source/model/template/ChartTypeTemplate.cxx:580-589</c>). It is the difference
    /// between four categories drawn as four slots with a bar in the middle of each and four
    /// categories drawn as four points, the first on the plot area's left edge and the last on its
    /// right — which is what a line or an area chart does, and it is why a line chart's polyline
    /// touches both edges where a bar chart's bars never do.
    /// </remarks>
    public bool ShiftedCategories
    {
        get
        {
            if (Kind is ChartPlotKind.Bar) return true;

            foreach (ChartSeries series in Series)
                if (series.Kind is ChartPlotKind.Bar) return true;

            return false;
        }
    }

    /// <summary>The series drawn as one kind, in file order.</summary>
    /// <param name="kind">The geometry.</param>
    public List<ChartSeries> SeriesOf(ChartPlotKind kind)
    {
        List<ChartSeries> matched = [];

        foreach (ChartSeries series in Series)
            if ((series.Kind ?? Kind) == kind) matched.Add(series);

        return matched;
    }

    /// <summary>
    /// The gap between category slots, as a percentage of one bar's width.
    /// </summary>
    /// <remarks>
    /// <c>c:gapWidth</c>, ODF <c>chart:gap-width</c>. 100 is the value both formats default to
    /// and the one the corpus chart states: a category slot then holds <em>n</em> bars plus one
    /// bar's width of gap, so a two-series clustered chart divides its slot into three.
    /// </remarks>
    public double GapWidth { get; init; } = 100.0;

    /// <summary>
    /// How much adjacent bars in a category overlap, as a percentage of a bar's width.
    /// </summary>
    /// <remarks>
    /// <c>c:overlap</c>. Zero for a clustered chart, 100 for a stacked one — a stacked chart
    /// draws its series in the same slot, which is exactly a full overlap.
    /// </remarks>
    public double Overlap { get; init; }

    /// <summary>Whether the series are stacked on top of one another.</summary>
    public bool IsStacked { get; init; }

    /// <summary>What the value axis states, before the automatic parts are resolved.</summary>
    public ChartScaleRequest ValueScale { get; init; }

    /// <summary>Where the legend goes.</summary>
    public ChartLegendPosition Legend { get; init; } = ChartLegendPosition.None;

    /// <summary>The chart area's own fill, or null when it states none.</summary>
    public Colour? Background { get; init; }

    /// <summary>
    /// The size the main title is set at.
    /// </summary>
    /// <remarks>
    /// Read from the file rather than assumed, because it decides how much room the title is
    /// given at the top of the chart and therefore where the plot area starts. Measured on
    /// <c>chart-bar-deck.pptx</c>, whose <c>c:title</c> states <c>sz="1300"</c>: assuming a
    /// 10 pt title instead reserves 3.5 pt too little and moves every bar's base down by that
    /// much. Thirteen points is LibreOffice's own default for a chart title
    /// (<c>chart2/source/model/main/Title.cxx</c>), which is why it is the fallback.
    /// </remarks>
    public Length TitleSize { get; init; } = Length.FromPoints(13);

    /// <summary>The size the axis titles are set at; LibreOffice's default is 9 pt.</summary>
    public Length AxisTitleSize { get; init; } = Length.FromPoints(9);

    /// <summary>The size the axis labels and legend entries are set at; the default is 10 pt.</summary>
    public Length LabelSize { get; init; } = Length.FromPoints(10);

    /// <summary>The plot area's fill — DrawingML's wall — or null.</summary>
    public Colour? PlotBackground { get; init; }

    /// <summary>
    /// The colour the value axis' major gridlines are drawn in, or null when it has none.
    /// </summary>
    /// <remarks>
    /// Present rather than absent is the question the file answers — <c>c:majorGridlines</c>,
    /// <c>chart:grid class="major"</c> — and both default the colour rather than stating it.
    /// <c>0xB3B3B3</c> is chart2's own default, set on <c>GridProperties</c> as
    /// <c>LINE_COLOR = 0xb3b3b3 // gray30</c>
    /// (<c>chart2/source/model/main/GridProperties.cxx:64-66</c>), and it is what a reader must
    /// supply for a gridline that states nothing about itself.
    /// </remarks>
    public Colour? ValueGrid { get; init; }

    /// <summary>The colour the category axis' major gridlines are drawn in, or null.</summary>
    /// <remarks>Far rarer than <see cref="ValueGrid"/>; see it for the default.</remarks>
    public Colour? CategoryGrid { get; init; }

    /// <summary>
    /// The inner plot rectangle the file states, relative to the chart frame, or null.
    /// </summary>
    /// <remarks>
    /// ODF's <c>chart:coordinate-region</c>, which is the *inner* rectangle — the axes' extent,
    /// excluding their labels — and is therefore directly the rectangle a bar is measured
    /// against. Measured on <c>chart-bar-deck.odp</c>: the file states
    /// <c>svg:x="2.258cm" svg:y="1.594cm" svg:width="17.674cm" svg:height="8.538cm"</c> and
    /// LibreOffice's own PDF draws the plot area at 2258, 1594, 17672, 8537 in hundredths of a
    /// millimetre — the same rectangle to within the rounding of a centimetre-formatted
    /// attribute. So when this is set, no layout heuristic is involved at all.
    /// </remarks>
    public DocRect? PlotArea { get; init; }

    /// <summary>
    /// The inner plot rectangle as fractions of the frame, or null.
    /// </summary>
    /// <remarks>
    /// OOXML's <c>c:plotArea/c:layout/c:manualLayout</c>, which is written only when the author
    /// dragged the plot area and states its four numbers as fractions of the chart frame. Separate
    /// from <see cref="PlotArea"/> rather than converted into it because the two are stated in
    /// different spaces and only one of them can be resolved without knowing the frame — and
    /// because folding them together made an OOXML chart look as if it had a coordinate space of
    /// its own, which decides whether the whole picture is stretched.
    /// </remarks>
    public (double X, double Y, double Width, double Height)? PlotAreaFraction { get; init; }

    /// <summary>
    /// The chart's own coordinate space, when the file states one.
    /// </summary>
    /// <remarks>
    /// ODF's <c>chart:chart/@svg:width</c> and <c>@svg:height</c> — 22 cm by 12 cm for the corpus
    /// deck — which is the space <see cref="PlotArea"/>, and every other stated position, is
    /// expressed in. It is not usually the size of the frame the chart is drawn into, so the two
    /// are related by a scale rather than by equality. Null for OOXML, which states no such
    /// thing: its <c>c:manualLayout</c> is in fractions of the frame instead.
    /// </remarks>
    public DocSize? Space { get; init; }

    /// <summary>True when there is nothing at all to draw.</summary>
    public bool IsEmpty => Series.Count == 0 && Title is null;

    /// <summary>
    /// The smallest and largest value any series contributes to the value axis.
    /// </summary>
    /// <remarks>
    /// <para>
    /// For an unstacked chart this is the plain minimum and maximum over every point, which is
    /// what <c>VDataSeriesGroup::calculateYMinAndMaxForCategory</c> reduces to when each series
    /// sits in its own slot (<c>VSeriesPlotter.cxx</c>). For a stacked one the maximum is the
    /// per-category <em>sum</em>, because that is how tall the column is — taking the largest
    /// single value instead would put an axis at 168 under a stack that reaches 289.
    /// </para>
    /// <para>
    /// Positives and negatives are summed separately, as LibreOffice does
    /// (<c>isSeparateStackingForDifferentSigns</c>, true for every Y axis), so a category
    /// holding +50 and −30 contributes 50 and −30 rather than 20 and 20.
    /// </para>
    /// </remarks>
    public (double? Minimum, double? Maximum) ValueRange()
    {
        double minimum = double.PositiveInfinity;
        double maximum = double.NegativeInfinity;

        if (IsStacked)
        {
            int categories = 0;
            foreach (ChartSeries series in Series) categories = Math.Max(categories, series.Values.Count);

            for (int at = 0; at < categories; at++)
            {
                double positive = 0.0;
                double negative = 0.0;
                bool any = false;

                foreach (ChartSeries series in Series)
                {
                    if (at >= series.Values.Count) continue;
                    if (series.Values[at] is not { } value || !double.IsFinite(value)) continue;

                    any = true;
                    if (value >= 0.0) positive += value; else negative += value;
                }

                if (!any) continue;
                minimum = Math.Min(minimum, negative);
                maximum = Math.Max(maximum, positive);
            }
        }
        else
        {
            foreach (ChartSeries series in Series)
            {
                foreach (double? point in series.Values)
                {
                    if (point is not { } value || !double.IsFinite(value)) continue;
                    minimum = Math.Min(minimum, value);
                    maximum = Math.Max(maximum, value);
                }
            }
        }

        return double.IsInfinity(minimum)
            ? (null, null)
            : (minimum, maximum);
    }

    /// <summary>How many categories the chart plots.</summary>
    /// <remarks>
    /// The larger of the stated labels and the longest series, because a chart may plot four
    /// numbers under three labels — the fourth category then has no label but still has a bar,
    /// which is what the reference draws.
    /// </remarks>
    public int CategoryCount()
    {
        int count = Categories.Count;
        foreach (ChartSeries series in Series) count = Math.Max(count, series.Values.Count);
        return count;
    }
}
