using Paperless.Core.Numbers;
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

/// <summary>What shape a series draws at each of its points.</summary>
/// <remarks>
/// <c>c:marker/c:symbol</c>, ODF's <c>chart:symbol-type</c> and <c>chart:symbol-name</c>. The
/// seven shapes here are what <c>DataPointSymbolSupplier</c> draws
/// (<c>chart2/source/view/main/DataPointSymbolSupplier.cxx</c>) reduced to the ones a path can
/// express directly; the rest of OOXML's list — <c>dot</c>, <c>dash</c>, <c>picture</c> — falls
/// back to <see cref="Square"/> rather than to nothing, because a marker in the wrong shape is
/// nearer the reference than a plot area with no marks in it.
/// </remarks>
public enum ChartMarker
{
    /// <summary>No marker — <c>c:symbol val="none"</c>.</summary>
    None = 0,

    /// <summary>A filled square, which is <c>auto</c>'s first shape.</summary>
    Square,

    /// <summary>A filled circle.</summary>
    Circle,

    /// <summary>A filled diamond.</summary>
    Diamond,

    /// <summary>A filled upward triangle.</summary>
    Triangle,

    /// <summary>A stroked cross.</summary>
    Cross,

    /// <summary>A stroked saltire.</summary>
    Star,
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
    /// The X values a scatter series states, one per point, or null for every other chart type.
    /// </summary>
    /// <remarks>
    /// <c>c:xVal</c> against <c>c:yVal</c>, ODF's second <c>chart:domain</c>. These are a numeric
    /// dimension with a scale of their own rather than category indices, which is the whole
    /// difference between a scatter chart and a line chart: spacing the points evenly along the
    /// category axis instead is right only when the X values happen to be evenly spaced. Measured
    /// on <c>chart2/qa/extras/data/pptx/tdf127720.pptx</c>, whose X runs 0 to 120 in irregular
    /// steps and whose reference draws a value axis labelled <c>0 20 40 60 80 100 120</c> where an
    /// evenly-spaced reading draws the four category names it does not have.
    /// </remarks>
    public IReadOnlyList<double?>? XValues { get; init; }

    /// <summary>
    /// Whether the series draws a marker at each point, and which one.
    /// </summary>
    /// <remarks>
    /// <c>c:marker/c:symbol</c>. A scatter chart with <c>c:scatterStyle val="lineMarker"</c> draws
    /// both; one with <c>val="marker"</c> draws markers alone and no line at all, which is what
    /// makes an unread marker the difference between a picture and an empty plot area.
    /// </remarks>
    public ChartMarker Marker { get; init; } = ChartMarker.None;

    /// <summary>Whether the series joins its points with a line.</summary>
    /// <remarks>
    /// False for a scatter chart whose <c>c:scatterStyle</c> is <c>marker</c>, and for a line
    /// series whose <c>a:ln</c> states <c>a:noFill</c>.
    /// </remarks>
    public bool HasLine { get; init; } = true;

    /// <summary>The label every point of this series carries, or null for none.</summary>
    public ChartDataLabel? Label { get; init; }

    /// <summary>
    /// The trendlines fitted to this series, or null when it has none.
    /// </summary>
    /// <remarks>
    /// <c>c:ser/c:trendline</c>, of which a series may carry several — a linear fit and a moving
    /// average over the same points is an ordinary chart. ODF's counterparts are
    /// <c>chart:regression-curve</c> and <c>chart:mean-value</c>, which hang off the series in the
    /// same way.
    /// </remarks>
    public IReadOnlyList<ChartTrendline>? Trendlines { get; init; }

    /// <summary>
    /// A label per point where the file overrides the series', or null where it does not.
    /// </summary>
    /// <remarks><c>c:dLbl</c> inside <c>c:dLbls</c>, addressed by <c>c:idx</c>.</remarks>
    public IReadOnlyList<ChartDataLabel?>? PointLabels { get; init; }

    /// <summary>
    /// Which value axis this series is measured against — 0 for the primary, 1 for the secondary.
    /// </summary>
    /// <remarks>
    /// A chart part states it indirectly: each plot group lists the <c>c:axId</c> of the pair of
    /// axes it uses, and the group whose ids match the second <c>c:valAx</c> is on the secondary
    /// axis. Every series in a group shares the group's axes, so this is a property of the group
    /// carried onto the series — which is how one <c>ChartPlot</c> can hold both.
    /// </remarks>
    public int AxisIndex { get; init; }

    /// <summary>The label for one point: its own where the file states one, the series' otherwise.</summary>
    /// <param name="index">The point's index.</param>
    public ChartDataLabel? LabelAt(int index)
        => PointLabels is { } labels && index >= 0 && index < labels.Count && labels[index] is { } own
            ? own
            : Label;

    /// <summary>The sum of the absolute values, which is what a percentage label divides by.</summary>
    public double Total()
    {
        double total = 0.0;
        foreach (double? point in Values)
            if (point is { } value && double.IsFinite(value)) total += Math.Abs(value);

        return total;
    }

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

    /// <summary>
    /// Whether the value axis is drawn at all.
    /// </summary>
    /// <remarks>
    /// <c>c:valAx/c:delete val="1"</c>, ODF's absence of a <c>chart:axis</c> for the dimension. A
    /// deleted axis draws no line, no ticks and — the part a word count sees — no labels, and it
    /// reserves no room for them either, so the plot area grows to fill what it would have taken.
    /// Its gridlines survive, because <c>c:majorGridlines</c> is a separate property that chart2
    /// keeps on the axis model rather than on its view. Measured on
    /// <c>chart2/qa/extras/data/pptx/tdf116163.pptx</c>, whose value axis is deleted: drawing it
    /// anyway adds the five words <c>20.0 15.0 10.0 5.0 0.0</c> to a reference that draws five
    /// category names and nothing else.
    /// </remarks>
    public bool ValueAxisVisible { get; init; } = true;

    /// <summary>Whether the category axis — or a scatter chart's X axis — is drawn.</summary>
    /// <remarks>
    /// <c>c:catAx/c:delete</c>. See <see cref="ValueAxisVisible"/>; measured on
    /// <c>tdf105517.pptx</c> and <c>tdf106217.pptx</c>, which between them draw twelve category
    /// names the reference does not.
    /// </remarks>
    public bool CategoryAxisVisible { get; init; } = true;

    /// <summary>Whether the secondary value axis is drawn.</summary>
    public bool SecondaryAxisVisible { get; init; } = true;

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

    /// <summary>The series drawn as one kind against one value axis, in file order.</summary>
    /// <param name="kind">The geometry.</param>
    /// <param name="axis">
    /// The value axis: 0 for the primary, 1 for the secondary, or −1 for either.
    /// </param>
    public List<ChartSeries> SeriesOf(ChartPlotKind kind, int axis = -1)
    {
        List<ChartSeries> matched = [];

        foreach (ChartSeries series in Series)
        {
            if ((series.Kind ?? Kind) != kind) continue;
            if (axis >= 0 && series.AxisIndex != axis) continue;
            matched.Add(series);
        }

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

    /// <summary>
    /// What a secondary value axis states, or null when the chart has only one.
    /// </summary>
    /// <remarks>
    /// A chart part with two <c>c:valAx</c> puts some plot groups on each, and the two carry
    /// scales of their own — which is the point of the feature: a revenue series in millions and
    /// a margin series in percent share a category axis and nothing else. The series that belong
    /// to it are those whose <see cref="ChartSeries.AxisIndex"/> is 1.
    /// </remarks>
    public ChartScaleRequest? SecondaryValueScale { get; init; }

    /// <summary>The secondary value axis' title, or null.</summary>
    public string? SecondaryValueAxisTitle { get; init; }

    /// <summary>How the secondary axis' ticks are written, or null for the general format.</summary>
    public NumberFormatCode? SecondaryValueFormat { get; init; }

    /// <summary>Whether any series is measured against a secondary value axis.</summary>
    public bool HasSecondaryAxis
    {
        get
        {
            if (SecondaryValueScale is null) return false;

            foreach (ChartSeries series in Series)
                if (series.AxisIndex == 1) return true;

            return false;
        }
    }

    /// <summary>
    /// How the value axis' ticks are written, or null for the general format.
    /// </summary>
    /// <remarks>
    /// <c>c:valAx/c:numFmt/@formatCode</c>, ODF's <c>style:data-style-name</c> on the axis' style.
    /// A code of <c>General</c> reads as null, because <c>General</c> is not a format code but a
    /// request for the number formatter's standard index
    /// (<c>oox/source/drawingml/chart/objectformatter.cxx:1132</c>).
    /// </remarks>
    public NumberFormatCode? ValueFormat { get; init; }

    /// <summary>
    /// What the file says about how the category axis' labels are set.
    /// </summary>
    /// <remarks>
    /// The rotation, whether they may overlap, whether they may wrap and whether they may be
    /// staggered — four things that between them decide whether a crowded axis draws sixteen
    /// month names turned a quarter turn, eight of sixteen upright, or all sixteen on top of one
    /// another. Resolved by <see cref="ChartAxisLabels"/>, which is where the rules live.
    /// </remarks>
    public ChartAxisText CategoryAxisText { get; init; }

    /// <summary>
    /// How the category axis' labels are written, or null to draw the cached text as it stands.
    /// </summary>
    /// <remarks>
    /// Only a date or a numeric category axis has one; a text axis' labels are already strings.
    /// See <see cref="ChartDataLabel.WriteCategory"/> for what it costs to miss.
    /// </remarks>
    public NumberFormatCode? CategoryFormat { get; init; }

    /// <summary>Where the legend goes.</summary>
    public ChartLegendPosition Legend { get; init; } = ChartLegendPosition.None;

    /// <summary>The chart area's own fill, or null when it states none.</summary>
    public Colour? Background { get; init; }

    /// <summary>
    /// The table of numbers drawn under the plot, or null when the chart has none.
    /// </summary>
    /// <remarks>
    /// <c>c:plotArea/c:dTable</c>. It is not a decoration: the plot rectangle gives up the room it
    /// takes, and the category axis stops drawing its own labels because the table's header row is
    /// them — <c>m_bDisplayLabels = false</c> whenever <c>m_bDisplayDataTable</c> is set on
    /// dimension 0 (<c>chart2/source/view/axes/VAxisProperties.cxx:336-343</c>). Drawing both is
    /// the mistake it invites, and it shows as every category name twice.
    /// </remarks>
    public ChartDataTable? DataTable { get; init; }

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

    /// <summary>
    /// What a scatter chart's X axis states about its scale, before the automatic parts resolve.
    /// </summary>
    /// <remarks>
    /// The first <c>c:valAx</c> of a scatter chart is its <em>X</em> axis, not its value axis —
    /// both dimensions are numeric, so the vocabulary uses the same element twice and tells them
    /// apart by <c>c:crosses</c> and by which axis each <c>c:axId</c> pair names.
    /// </remarks>
    public ChartScaleRequest DomainScale { get; init; }

    /// <summary>How a scatter chart's X ticks are written, or null for the general format.</summary>
    public NumberFormatCode? DomainFormat { get; init; }

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
    /// <param name="axis">
    /// Which value axis to measure: 0 for the primary, 1 for the secondary, −1 for every series
    /// whichever axis it is on. A chart with no secondary axis has every series on axis 0, so the
    /// default is the whole chart either way.
    /// </param>
    public (double? Minimum, double? Maximum) ValueRange(int axis = -1)
    {
        double minimum = double.PositiveInfinity;
        double maximum = double.NegativeInfinity;

        bool Included(ChartSeries series) => axis < 0 || series.AxisIndex == axis;

        if (IsStacked)
        {
            int categories = 0;
            foreach (ChartSeries series in Series)
                if (Included(series)) categories = Math.Max(categories, series.Values.Count);

            for (int at = 0; at < categories; at++)
            {
                double positive = 0.0;
                double negative = 0.0;
                bool any = false;

                foreach (ChartSeries series in Series)
                {
                    if (!Included(series)) continue;
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
                if (!Included(series)) continue;

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
