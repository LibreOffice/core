using XRay.Office.Core.Numbers;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;

namespace XRay.Office.Core.Charts;

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

    /// <summary>
    /// A closed polygon on a polar category axis — <c>c:radarChart</c>, <c>chart:radar</c>.
    /// </summary>
    Radar,

    /// <summary>
    /// Circles at (x, y) sized by a third number — <c>c:bubbleChart</c>, <c>chart:bubble</c>.
    /// </summary>
    Bubble,

    /// <summary>
    /// High/low whiskers and open/close marks — <c>c:stockChart</c>, <c>chart:stock</c>.
    /// </summary>
    Stock,

    /// <summary>
    /// A pie whose tail becomes a second pie or a bar — <c>c:ofPieChart</c>.
    /// </summary>
    OfPie,
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
public sealed partial record ChartSeries(
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

    /// <summary>
    /// The colour the marker is filled in where it states one of its own, or null to take the
    /// series' colour.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>c:marker/c:spPr</c>. A marker carries its own shape properties and they are not the
    /// series': <c>TypeGroupConverter::convertMarker</c> reads the symbol's fill colour, falling
    /// back to the symbol's line colour when there is no fill
    /// (<c>typegroupconverter.cxx:657-678</c>, tdf#124817), and only then does the series' colour
    /// come into it.
    /// </para>
    /// <para>
    /// Null means "the file states nothing", which keeps every marker whose file says nothing
    /// drawn exactly as before — including every ODF chart, whose reader has no such element to
    /// give.
    /// </para>
    /// </remarks>
    public Colour? MarkerFill { get; init; }

    /// <summary>The marker's own outline colour, or null to take the series'.</summary>
    /// <remarks><c>c:marker/c:spPr/a:ln</c>. See <see cref="MarkerFill"/>.</remarks>
    public Colour? MarkerLine { get; init; }

    /// <summary>
    /// The marker's side, or null when the file states none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>c:marker/c:size</c>, <strong>in whole points</strong>, which
    /// <c>TypeGroupConverter::convertMarker</c> turns into a square symbol of
    /// <c>convertPointToMm100(size)</c> hundredths of a millimetre
    /// (<c>typegroupconverter.cxx:652-654</c>). A part that states no size still gets one:
    /// <c>SeriesModel</c>'s constructor is <c>mnMarkerSize( 5 )</c>
    /// (<c>seriesmodel.cxx:118</c>), so an OOXML marker is 5 pt unless the file says otherwise
    /// and is never the 250 hundredths of a millimetre that <see cref="ChartLayout"/> falls back
    /// to.
    /// </para>
    /// <para>
    /// <strong>Null is not "5 pt", it is "no OOXML marker element reached here".</strong> That
    /// distinction is the whole point of the property: an ODF or binary chart has no
    /// <c>c:marker</c> to read and its symbol keeps chart2's own unset default of 250 × 250
    /// (<c>VDataSeries::getSymbolProperties</c>), which is 7.09 pt and a different number. Giving
    /// this a non-null default would silently move every ODF and <c>.ppt</c> chart in the corpus.
    /// </para>
    /// <para>
    /// Measured on <c>003_advanced_powerpoint_line.pptx</c>, which states
    /// <c>&lt;c:symbol val="circle"/&gt;&lt;c:size val="6"/&gt;</c>: <c>6 × 2540 / 72</c> rounds to
    /// 212 hundredths of a millimetre, which is <b>6.01 pt</b> — exactly the width round 62
    /// measured the reference drawing on that page, and exactly the figure it refuted as a
    /// <em>legend key</em> claim while confirming it as a plot-marker one.
    /// </para>
    /// </remarks>
    public Length? MarkerSize { get; init; }

    /// <summary>
    /// The dash array the series' line is stroked with, or null for a solid line.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Alternating ink and gap lengths, already expanded from whatever the file named — OOXML's
    /// <c>a:prstDash</c> through <see cref="DashPresets"/>, ODF's <c>draw:stroke-dash</c>. It is
    /// stored expanded rather than as a preset name because the lengths are a percentage of the
    /// pen width, so the name alone does not determine the pattern.
    /// </para>
    /// <para>
    /// <strong>A threshold line is the case this exists for.</strong> A chart that draws its
    /// target as a dotted horizontal rule and its data as a solid curve is telling the reader
    /// which is which by the dash and by nothing else; drawn solid, the three lines of
    /// <c>southern-classic-kennesaw-state-university-final.pptx</c>'s price chart read as four
    /// series of equal standing. It also widens the legend key, which
    /// <c>getPreferredLegendKeyAspectRatio</c> doubles from 800 to 1600 for a dashed line.
    /// </para>
    /// </remarks>
    public IReadOnlyList<Length>? DashPattern { get; init; }

    /// <summary>How the ends of the series' line — and of each of its dashes — are drawn.</summary>
    /// <remarks>
    /// <para>
    /// <strong>The cap is half of what a dash pattern looks like, not a refinement of it.</strong>
    /// <c>DashPresets</c> takes 99% off each ink length for a round or square cap because
    /// Microsoft measures the cap inside the dash and ODF adds it on — so a <c>sysDot</c> on a
    /// 3 pt pen becomes 0.03 pt of ink and 5.97 of gap, and the cap is the whole of the dot.
    /// Drawn butt, that same array is a row of hairline rectangles.
    /// </para>
    /// <para>
    /// Measured in the PDF operators: the reference emits <c>1 J</c> beside every
    /// <c>[0.03 5.97] 0 d</c> on
    /// <c>southern-classic-kennesaw-state-university-final.pptx</c> page 2, and we emitted
    /// <c>0 J</c> for all 893 of our strokes. A blind reviewer sent that page called ours
    /// "narrow, tall, rectangular dashes with wide gaps" against the reference's "clearly round
    /// dots", and named the cap as the first candidate — before the operator census was run.
    /// </para>
    /// </remarks>
    public LineCap LineCap { get; init; } = LineCap.Butt;

    /// <summary>Whether the series joins its points with a line.</summary>
    /// <remarks>
    /// False for a scatter chart whose <c>c:scatterStyle</c> is <c>marker</c>, and for a line
    /// series whose <c>a:ln</c> states <c>a:noFill</c>.
    /// </remarks>
    public bool HasLine { get; init; } = true;

    /// <summary>
    /// Whether the line through this series' points is a flattened cubic spline rather than a
    /// polyline.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>CurveStyle_CUBIC_SPLINES</c>. OOXML spells it <c>c:smooth</c> on each
    /// <c>c:ser</c> and ODF <c>chart:interpolation="cubic-spline"</c> on the plot area's style;
    /// <see cref="ChartSpline"/> is what it means for the geometry.
    /// </para>
    /// <para>
    /// <strong>It is a property of the plot group and not of the series, however the file spells
    /// it.</strong> <c>OOX_CHART_SMOOTHED_PER_SERIES</c> is <c>0</c>
    /// (<c>oox/inc/drawingml/chart/seriesconverter.hxx</c>:37), so
    /// <c>TypeGroupConverter::convertLineSmooth</c> is called once per series
    /// <em>against the chart type's</em> property set
    /// (<c>oox/source/drawingml/chart/typegroupconverter.cxx</c>:585-588) — one series stating
    /// <c>c:smooth val="1"</c> smooths every series in its group. The readers therefore resolve
    /// the flag per group and put the same answer on each of that group's series. No corpus
    /// document distinguishes the two rules: all fourteen smoothed plot groups are uniform.
    /// </para>
    /// <para>
    /// <strong>An absent <c>c:smooth</c> does not mean "not smooth".</strong>
    /// <c>SeriesModel</c>'s constructor is <c>mbSmooth( !bMSO2007Doc )</c>
    /// (<c>oox/source/drawingml/chart/seriesmodel.cxx</c>:124) and both the line and the scatter
    /// series contexts read it as <c>getBool( XML_val, !bMSO2007Doc )</c>
    /// (<c>seriescontext.cxx</c>:618, :726), so a file that is not an Office 2007 one smooths by
    /// default. That is not a curiosity: it is the whole of one corpus witness. See
    /// <c>probes/chart-smooth-r102</c>.
    /// </para>
    /// </remarks>
    public bool Smooth { get; init; }

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
public sealed partial record ChartPlot
{
    /// <summary>The chart's title, or null when it states none.</summary>
    public string? Title { get; init; }

    /// <summary>The category axis' title, or null.</summary>
    public string? CategoryAxisTitle { get; init; }

    /// <summary>The value axis' title, or null.</summary>
    public string? ValueAxisTitle { get; init; }

    /// <summary>The category labels, in order. Empty for a chart with no category axis.</summary>
    public IReadOnlyList<string?> Categories { get; init; } = [];

    /// <summary>
    /// The levels of a complex category axis, innermost first, or null for an ordinary one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A complex category axis is drawn as rows, not as one joined string.</strong>
    /// <see cref="Categories"/> holds the join — <c>AM 9/5/2026</c> — because that is what
    /// <c>ExplicitCategoriesProvider::getSimpleCategories</c> hands to a legend entry and to a
    /// data label. The axis is the other consumer and it draws each level on a row of its own,
    /// level zero nearest the axis line, with a long tick at every run boundary
    /// (<c>VCartesianAxis::createAllTickInfosFromComplexCategories</c>,
    /// <c>chart2/source/view/axes/VCartesianAxis.cxx:575-610</c>, and the extra ticks at
    /// <c>:1913-1955</c>).
    /// </para>
    /// <para>
    /// <strong>A run ends at the next value, not at the next <em>different</em> value.</strong>
    /// <c>lcl_DataSequenceToComplexCategoryVector</c>
    /// (<c>chart2/source/tools/ExplicitCategoriesProvider.cxx:275-311</c>) says so in its own
    /// comment: "Empty value is interpreted as a continuation of the previous category. Note that
    /// having the same value as the previous one does not equate to a continuation." So a null or
    /// empty entry here extends the run above it and a repeated string starts a new one — which
    /// is why <c>040_Blood_pressure_tracker</c> draws its date twice, once under <c>AM</c> and
    /// once under <c>PM</c>, rather than once across the pair.
    /// </para>
    /// <para>
    /// Each level holds one entry per category, so every level is as long as
    /// <see cref="Categories"/>. Null when the file states a single level or none, which keeps
    /// every ordinary axis on exactly the path it was on.
    /// </para>
    /// </remarks>
    public IReadOnlyList<IReadOnlyList<string?>>? CategoryLevels { get; init; }

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

    /// <summary>Whether the value axis' tick labels are drawn.</summary>
    /// <remarks>
    /// <para>
    /// <strong>Hiding the labels is not deleting the axis, and the two reserve different
    /// amounts.</strong> <c>c:tickLblPos val="none"</c> maps to chart2's <c>DisplayLabels</c>
    /// (<c>oox/source/drawingml/chart/axisconverter.cxx:221</c>,
    /// <c>aAxisProp.setProperty(PROP_DisplayLabels, mrModel.mnTickLabelPos != XML_none)</c>),
    /// which <c>VAxisProperties</c> reads at <c>:307</c> and which suppresses the labels alone —
    /// the axis line and its tick marks are still drawn. <see cref="ValueAxisVisible"/> is
    /// <c>c:delete</c> and removes all three.
    /// </para>
    /// <para>
    /// Measured on a probe over <c>chart-face-theme-minor.pptx</c> with
    /// <c>tickLblPos="none"</c> on its category axis: the reference stops drawing
    /// <c>Q1 Q2 Q3 Q4</c> and its plot area's bottom edge drops 12.70 pt, keeping the 4.25 pt
    /// tick. Reading only <c>c:delete</c> both drew four labels the reference does not and held
    /// the plot area 12.79 pt short.
    /// </para>
    /// </remarks>
    public bool ValueLabelsVisible { get; init; } = true;

    /// <summary>Whether the category axis' tick labels are drawn.</summary>
    /// <remarks><c>c:catAx/c:tickLblPos</c>; see <see cref="ValueLabelsVisible"/>.</remarks>
    public bool CategoryLabelsVisible { get; init; } = true;

    /// <summary>Whether the secondary value axis' tick labels are drawn.</summary>
    /// <remarks><c>c:valAx/c:tickLblPos</c>; see <see cref="ValueLabelsVisible"/>.</remarks>
    public bool SecondaryLabelsVisible { get; init; } = true;

    /// <summary>How the value axis' own line and tick marks are painted.</summary>
    /// <remarks>
    /// <para>
    /// <strong>An axis line is not black, and this reader drew every one of them black.</strong>
    /// <c>spAxisLines</c> (<c>oox/source/drawingml/chart/objectformatter.cxx:215-220</c>) gives it
    /// the theme's <c>tx1</c> at <c>tint 75000</c> — the same entry as a major gridline — and the
    /// theme's subtle line style supplies its width. On <c>Demick_JetBlue.pptx</c> page 4 the
    /// reference draws its two axis lines <c>#666666</c> at 0.73 pt where we drew <c>#000000</c>
    /// hairlines.
    /// </para>
    /// <para>
    /// The <em>tick labels</em> are a separate statement and do not move with it: both stacks
    /// draw that page's labels <c>#000000</c>, which is <c>c:txPr</c>'s answer and not this one.
    /// </para>
    /// <para>
    /// Not nullable — an axis that is drawn at all has a stroke — and the default is the black
    /// hairline every non-OOXML reader has always produced, so ODF and BIFF are unchanged.
    /// </para>
    /// </remarks>
    public ChartGrid ValueAxisLine { get; init; } = new(Colour.Black);

    /// <summary>How the category axis' own line and tick marks are painted.</summary>
    /// <remarks><c>c:catAx/c:spPr/a:ln</c>; see <see cref="ValueAxisLine"/>.</remarks>
    public ChartGrid CategoryAxisLine { get; init; } = new(Colour.Black);

    /// <summary>How the secondary value axis' own line and tick marks are painted.</summary>
    /// <remarks>See <see cref="ValueAxisLine"/>.</remarks>
    public ChartGrid SecondaryAxisLine { get; init; } = new(Colour.Black);

    /// <summary>Where the value axis puts its major tick marks.</summary>
    /// <remarks>
    /// <para>
    /// <strong>An axis reserves its tick length only when it actually draws a tick outside the
    /// plot area.</strong> <c>c:majorTickMark</c> takes <c>out</c>, <c>cross</c>, <c>in</c> and
    /// <c>none</c>; <c>lclGetTickMark</c>
    /// (<c>oox/source/drawingml/chart/axisconverter.cxx:104-115</c>) maps the first two to a
    /// style carrying <c>OUTER</c> and the last two to one that does not, and only an outward
    /// tick extends past the plot area and is therefore charged to it by
    /// <c>VDiagram::adjustInnerSize</c>.
    /// </para>
    /// <para>
    /// Measured one property and one axis at a time on a corpus chart already stating
    /// <c>none</c> on both axes — six arms, and the plot edge moves by <c>AXIS2D_TICKLENGTH</c>
    /// exactly, on that axis' own edge and on no other:
    /// <c>none 0.00 / in 0.00 / out +4.25 / cross +4.25</c>. The labels do not move with it: the
    /// leftmost value label's pen sits at the same <c>x</c> in all four arms, so what the tick
    /// buys is the gap between the label and the axis and not the label's own place.
    /// </para>
    /// <para>
    /// The default is <see cref="ChartTickMark.Outer"/> because an <em>absent</em>
    /// <c>c:majorTickMark</c> is not <c>none</c>: <c>AxisModel</c>'s constructor defaults it to
    /// <c>out</c> for a 2007 chart and to <c>cross</c> for a later one
    /// (<c>oox/source/drawingml/chart/axismodel.cxx:42-48</c>), and both of those reserve. Only a
    /// stated <c>none</c> or <c>in</c> changes anything, and ODF — which has no census behind it
    /// here — is left on the default.
    /// </para>
    /// </remarks>
    public ChartTickMark ValueTicks { get; init; } = ChartTickMark.Outer;

    /// <summary>Where the category axis puts its major tick marks.</summary>
    /// <remarks><c>c:catAx/c:majorTickMark</c>; see <see cref="ValueTicks"/>.</remarks>
    public ChartTickMark CategoryTicks { get; init; } = ChartTickMark.Outer;

    /// <summary>Where the secondary value axis puts its major tick marks.</summary>
    /// <remarks>
    /// <c>c:majorTickMark</c> on the secondary <c>c:valAx</c>; see <see cref="ValueTicks"/>.
    /// </remarks>
    public ChartTickMark SecondaryTicks { get; init; } = ChartTickMark.Outer;

    /// <summary>Whether the chart has a pair of axes at all.</summary>
    /// <remarks>
    /// A pie has neither, so it gets no axis lines, no ticks, no gridlines and — the part that
    /// matters most for a word count — no tick labels, which is the eighty-one words the first
    /// version of the reader invented for <c>PieChartWithAutomaticLayout_SizeAndPosition.pptx</c>.
    /// </remarks>
    /// <remarks>
    /// An of-pie chart is a pie and has none either. A <em>radar</em> chart has two axes and
    /// neither of them is a pair of straight lines — its category axis is angular and its value
    /// axis is a radius — so it is excluded here and draws its own web and spokes in
    /// <c>ChartLayout</c>'s radar half instead.
    /// </remarks>
    public bool HasAxes
        => Kind is not (ChartPlotKind.Pie or ChartPlotKind.OfPie or ChartPlotKind.Radar);

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
            // A candlestick is measured against a BarPositionHelper — CandleStickChart places
            // each candle with getScaledSlotPos and sizes it with getScaledSlotWidth
            // (CandleStickChart.cxx:160-180) — so a stock chart's categories are slots exactly as
            // a bar chart's are, and its whiskers stand in the middle of each rather than on the
            // plot area's edges.
            //
            // The bar test comes *first* and is not overridable, which is a measurement and not a
            // reading of the source: on 26.2.4.2 a column chart stating c:crossBetween="midCat"
            // renders byte-for-byte as the same chart stating "between". See CategoriesBetween.
            if (Kind is ChartPlotKind.Bar or ChartPlotKind.Stock) return true;

            foreach (ChartSeries series in Series)
                if (series.Kind is ChartPlotKind.Bar or ChartPlotKind.Stock) return true;

            return CategoriesBetween ?? false;
        }
    }

    /// <summary>
    /// Whether the file itself says the categories occupy slots — <c>c:crossBetween</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>The chart type is not the whole rule for an OOXML chart, and for a line or an area
    /// chart it is not the rule at all.</strong> <c>c:crossBetween</c> is stated on the value axis
    /// the category axis crosses, and
    /// <c>oox/source/drawingml/chart/axisconverter.cxx:292-301</c> reads it into
    /// <c>ScaleData::ShiftedCategoryPosition</c> ahead of the type, falling back to the first type
    /// group's own type only when the element is absent.
    /// </para>
    /// <para>
    /// Measured on the running binary rather than read off that source, because the two disagree
    /// about one of the nine cases. Nine arms — three chart types by
    /// <c>between</c> / <c>midCat</c> / element deleted, one property patched per arm in a corpus
    /// deck, each rendered through 26.2.4.2 and read back from the category labels' own pen
    /// positions (<c>probes/slides-r60/make-crossbetween-probe.py</c>, <c>cbread.py</c>):
    /// </para>
    /// <code>
    ///                between    midCat     absent
    ///   areaChart    shifted    unshifted  unshifted
    ///   lineChart    shifted    unshifted  SHIFTED
    ///   barChart     shifted    SHIFTED    shifted
    /// </code>
    /// <para>
    /// So a bar or column chart ignores the element outright — which is why the test for it in
    /// <see cref="ShiftedCategories"/> runs before this one — and a line chart with the element
    /// absent is shifted while an area chart with it absent is not.
    /// </para>
    /// <para>
    /// <see langword="null"/> means the format made no statement, which is the case for every ODF
    /// chart: ODF has no such attribute and <c>ChartTypeTemplate::adaptScales</c> shifts Column,
    /// Bar and Close alone, so leaving it null keeps an ODF plot on exactly the answer the type
    /// test gives.
    /// </para>
    /// </remarks>
    public bool? CategoriesBetween { get; init; }

    /// <summary>
    /// Whether the category axis runs backwards — the last category where the first would be.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>c:catAx/c:scaling/c:orientation val="maxMin"</c>, ODF's
    /// <c>chart:reverse-direction</c>, and <c>ScaleData::Orientation ==
    /// AxisOrientation_REVERSE</c> in chart2's own model. It is what a Gantt chart is made of: a
    /// horizontal bar chart's categories run bottom-to-top, so putting the first task at the top
    /// takes a reversed axis, and every real Gantt in the wild states one.
    /// </para>
    /// <para>
    /// <strong>It moves the value axis as well as the categories, and that is one rule rather than
    /// two.</strong> The value axis is drawn at the <em>start</em> of the axis it crosses —
    /// <c>AxisProperties::initAxisPositioning</c>, <c>chart2/source/view/axes/VAxisProperties.cxx</c>
    /// :232-234, <c>m_eCrossoverType = START</c> and <c>END</c> exactly when
    /// <c>m_bIsMainAxis == m_bCrossingAxisHasReverseDirection</c>, with the flag set from the
    /// crossing scale's orientation at
    /// <c>chart2/source/view/axes/VCartesianCoordinateSystem.cxx:145</c>. Reversing the category
    /// axis therefore moves its start to the other end of the plot and the value axis' line, ticks
    /// and labels go with it.
    /// </para>
    /// <para>
    /// Measured on both directions against 26.2.4.2, by patching one attribute of a corpus chart
    /// and rendering both versions (<c>probes/chart-cat-reverse/</c>): on
    /// <c>N2_E_Maestroni_Swarm_COP.pptx</c>'s Gantt, <c>minMax</c> puts <c>LEOP [0000]</c> at the
    /// bottom with the value axis line at y = 514.97 and <c>maxMin</c> puts it at the top with the
    /// line at y = 108.00; on <c>002_advanced_powerpoint_column.pptx</c>'s column chart,
    /// <c>M1</c> moves from x = 109.87 to x = 521.29 and the value axis' line and labels move from
    /// the left edge (x = 85.58, labels at 62–73) to the right (x = 559.11, labels at 566.19),
    /// while the category labels stay on the bottom in both.
    /// </para>
    /// <para>
    /// The value axis' own <c>c:orientation</c> is <see cref="ChartScaleRequest.IsReversed"/> and
    /// is a different statement about a different axis; a chart may make either, both or neither.
    /// </para>
    /// </remarks>
    public bool CategoriesReversed { get; init; }

    /// <summary>Which end of the category axis the value axis' tick labels sit at.</summary>
    /// <remarks>See <see cref="ChartValueLabelPosition"/>; the axis <em>line</em> does not move
    /// with them.</remarks>
    public ChartValueLabelPosition ValueLabelPosition { get; init; }

    /// <summary>The same statement made by a secondary value axis.</summary>
    public ChartValueLabelPosition SecondaryLabelPosition { get; init; }

    /// <summary>
    /// Where the <em>category</em> axis' own tick labels sit along the value axis.
    /// </summary>
    /// <remarks>
    /// <c>c:catAx/c:tickLblPos</c>, the counterpart of <see cref="ValueLabelPosition"/> one axis
    /// over. <c>nextTo</c> — the default, and what nearly every chart states — puts them beside
    /// the axis <em>line</em>, wherever <see cref="CategoryAxisCrossing"/> has put that;
    /// <c>low</c> and <c>high</c> send them to an end of the value axis whatever the line does.
    /// <c>getLabelLineIntersectionValue</c> (<c>chart2/source/view/axes/VCartesianAxis.cxx</c>:1103-1113)
    /// is the one function that decides it: the two outside positions answer the value axis'
    /// minimum and maximum outright, and <c>NEAR_AXIS</c> falls through to
    /// <c>getAxisIntersectionValue</c>, which is the axis line's own.
    /// </remarks>
    public ChartValueLabelPosition CategoryLabelPosition { get; init; }

    /// <summary>Where the category axis' line stands along the value axis.</summary>
    /// <remarks>
    /// <para>
    /// <c>c:catAx/c:crosses</c>. <strong><see cref="ChartAxisCrossing.Automatic"/> is a real value
    /// here and not a synonym for the minimum</strong>, which is what separates this from
    /// <see cref="ValueAxisCrossing"/>: <c>autoZero</c> asks for <em>value zero on the crossing
    /// axis</em> — <c>m_pfMainLinePositionAtOtherAxis = 0.0</c>,
    /// <c>chart2/source/view/axes/VAxisProperties.cxx</c>:224-225 — and the crossing axis of a
    /// category axis is a value axis, whose scale usually contains zero. A value axis crossing a
    /// <em>category</em> axis clamps to the minimum because a category axis' own scale starts at a
    /// half; this one does not.
    /// </para>
    /// <para>
    /// It is clamped into the value axis' range by <c>VCartesianAxis::get2DAxisMainLine</c>
    /// (<c>:1253-1256</c>), so a chart whose values are all positive draws its category axis along
    /// the bottom exactly as before, and only a chart whose values span zero moves.
    /// </para>
    /// <para>
    /// Measured on <c>Demick_JetBlue.pptx</c> page 5, whose column chart runs
    /// −44 587 … 1 200 000: 26.2.4.2 draws the plot from y = 214.58 to 401.56 and its category
    /// axis line at <strong>y = 374.83</strong>, which is the <code>$-</code> gridline, with the
    /// category labels hanging from that line inside the plot. Drawing them at the plot's bottom
    /// edge instead is legible by accident and is a whole band of the frame out.
    /// </para>
    /// </remarks>
    public ChartAxisCrossing CategoryAxisCrossing { get; init; }

    /// <summary>
    /// The value the category axis' line stands at, when the file states one rather than an end.
    /// </summary>
    /// <remarks>
    /// <c>c:catAx/c:crossesAt</c>, which is <c>ChartAxisPosition_VALUE</c> and outranks
    /// <c>c:crosses</c> — <c>bManualCrossing</c> at
    /// <c>oox/source/drawingml/chart/axisconverter.cxx</c>:443-455. Null when the file states none.
    /// </remarks>
    public double? CategoryCrossesAt { get; init; }

    /// <summary>Which end of the category axis the value axis itself stands at.</summary>
    /// <remarks>
    /// <c>c:valAx/c:crosses</c>; see <see cref="ChartAxisCrossing"/>. A secondary axis takes the
    /// other end from the primary rather than reading its own statement — every secondary axis in
    /// the corpus says <c>max</c> against an <c>autoZero</c> primary, which is that same rule
    /// spelled out, and honouring both statements separately would let a file put two axes on one
    /// edge.
    /// </remarks>
    public ChartAxisCrossing ValueAxisCrossing { get; init; }

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

    /// <summary>
    /// Whether the stack is normalised to fill the plot — <c>c:grouping val="percentStacked"</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A percent stack is not a stack with a percentage format on its axis: every category is
    /// divided by its own total, so the columns are the same height whatever the raw numbers are
    /// and the axis runs 0 to 1 regardless of them. Reading it as an ordinary stack and then
    /// applying the axis' <c>0%</c> format multiplies the raw total by a hundred — measured on
    /// <c>8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx</c>, whose stack of 548 over 73 gave an axis
    /// reading <c>0%, 10000% … 70000%</c> against the reference's <c>0% … 100%</c>.
    /// </para>
    /// <para>
    /// <strong>The raw value survives into the data labels.</strong> Normalising the stored
    /// values instead would be a smaller change and would make every <c>c:showVal</c> label read
    /// <c>0.88</c> where both the file and the reference say <c>548</c>, so the normalisation is
    /// applied where the geometry is computed and nowhere else.
    /// </para>
    /// </remarks>
    public bool IsPercentStacked { get; init; }

    /// <summary>
    /// What one category's values are divided by on a percent stack, or null when they are not.
    /// </summary>
    /// <remarks>
    /// The sum of the <em>absolute</em> values in the category, which is what makes a category
    /// holding +50 and −50 fill the plot in both directions rather than divide by zero. Null for
    /// any chart that is not percent-stacked and for a category that sums to nothing, and a null
    /// means "draw the value as it stands".
    /// </remarks>
    /// <param name="index">The category's index.</param>
    /// <param name="axis">
    /// Which value axis to total, matching <see cref="ValueRange"/>: −1 for every series.
    /// </param>
    public double? StackTotal(int index, int axis = -1)
    {
        if (!IsPercentStacked) return null;

        double total = 0.0;

        foreach (ChartSeries series in Series)
        {
            if (axis >= 0 && series.AxisIndex != axis) continue;
            if (index < 0 || index >= series.Values.Count) continue;
            if (series.Values[index] is not { } value || !double.IsFinite(value)) continue;

            total += Math.Abs(value);
        }

        return total > 0.0 ? total : null;
    }

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
    /// What the file says about how the <em>value</em> axis' labels are set.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>c:valAx/c:txPr/a:bodyPr</c>, read by the same function that reads the category axis' —
    /// and until this existed it was not read at all, so a chart stating its own value-label
    /// rotation drew them upright. Only <see cref="ChartAxisText.Rotation"/> is used: the other
    /// three decide an arrangement, and a value axis has none. <c>isBreakOfLabelsAllowed</c> says
    /// so outright — "no break for value axis", <c>!m_bUseTextLabels</c>
    /// (<c>chart2/source/view/axes/VCartesianAxis.cxx</c>:521-522) — and
    /// <c>canAutoAdjustLabelPlacement</c> never reaches one either, because a value axis' tick
    /// count is reduced until its labels fit rather than its labels being turned.
    /// </para>
    /// <para>
    /// <strong>Reach: one document, and the axis' own title is what the other thirty-six
    /// are.</strong> Over the corpus' 307 chart parts, 37 value axes carry an in-range non-zero
    /// <c>rot</c> somewhere inside <c>c:valAx</c> and <strong>35 of them are on the axis
    /// <em>title</em></strong>, which is a quarter-turn and was already drawn. Read from
    /// <c>c:txPr</c> alone the count is two, and one of those two is not a value axis either:
    /// <c>047_Date_tracker_Gantt_chart</c> is a <c>c:scatterChart</c>, whose <em>domain</em> is
    /// also spelt <c>c:valAx</c>, and its −30° is on the bottom axis that
    /// <see cref="ChartPlot.DomainScale"/> carries and <c>AddDomainAxis</c> draws. What is left is
    /// <c>N2_E_Maestroni_Swarm_COP.pptx</c> at −45°, a bar chart whose value axis runs along the
    /// bottom and whose dates would otherwise overlap. See <c>probes/chart-layout/census.py</c>.
    /// </para>
    /// <para>
    /// <strong>The ODF reader deliberately does not set this yet.</strong> <c>OdfChartPlot</c>
    /// has the same <c>AxisTextOf</c> and could read <c>style:rotation-angle</c> off the value
    /// axis' style in one line, but no corpus ODF chart was measured for it and an unmeasured
    /// rotation would move ODF charts on a rule taken from OOXML's reference. It is a one-line
    /// change for whoever measures it.
    /// </para>
    /// </remarks>
    public ChartAxisText ValueAxisText { get; init; }

    /// <summary>The same, for a secondary value axis.</summary>
    public ChartAxisText SecondaryValueAxisText { get; init; }

    /// <summary>
    /// How the category axis' labels are written, or null to draw the cached text as it stands.
    /// </summary>
    /// <remarks>
    /// Only a date or a numeric category axis has one; a text axis' labels are already strings.
    /// See <see cref="ChartDataLabel.WriteCategory"/> for what it costs to miss.
    /// </remarks>
    public NumberFormatCode? CategoryFormat { get; init; }

    /// <summary>
    /// The category axis resolved as a <em>date</em> axis, or null when it is a run of category
    /// slots like any other.
    /// </summary>
    /// <remarks>
    /// Set only where the file states a date axis and the categories bear it out. Everything that
    /// maps a category across the plot area consults it first — see
    /// <see cref="ChartDateAxis"/> for why the two answers differ by more than a tick label.
    /// </remarks>
    public ChartDateAxis? DateAxis { get; init; }

    /// <summary>Where the legend goes.</summary>
    public ChartLegendPosition Legend { get; init; } = ChartLegendPosition.None;

    /// <summary>The chart area's own fill, or null when it states none.</summary>
    /// <summary>The colour an axis' tick labels are drawn in — the axes' own <c>c:txPr</c>.</summary>
    /// <remarks>
    /// <para>
    /// <strong>A chart's text is not black because LibreOffice draws chart text black.</strong>
    /// It is black because <c>tx1</c> usually resolves to black, and a chart that names anything
    /// else gets it. This reader drew every piece of chart text in one hardcoded
    /// <c>Colour.Black</c> until round 60, which is right for the majority and wrong for every
    /// chart on a dark master: <c>8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx</c> names
    /// <c>a:schemeClr val="bg1"</c> on its title, on both axes and on its data labels, and the
    /// reference draws all fourteen of page 8's text runs white where this reader drew
    /// twenty-two black ones.
    /// </para>
    /// <para>
    /// Five separate properties rather than one, because the five objects state the colour in
    /// five different places and a chart routinely states it in only some of them — a corpus
    /// census over every OOXML chart part counts <c>c:valAx</c>, <c>c:catAx</c>, <c>c:title</c>,
    /// <c>c:dLbls</c> and <c>c:legend</c> separately, and the commonest single statement
    /// (<c>tx1</c>, 385 times) sits on an axis while the second (<c>bg1</c>, 45) is spread over
    /// data labels and titles.
    /// </para>
    /// <para>
    /// The default is black on all five, so a format that states nothing — every ODF chart, every
    /// binary workbook chart — keeps exactly the answer it had.
    /// </para>
    /// </remarks>
    public Colour LabelColour { get; init; } = Colour.Black;

    /// <summary>The colour the chart's main title is drawn in.</summary>
    /// <remarks>See <see cref="LabelColour"/>.</remarks>
    public Colour TitleColour { get; init; } = Colour.Black;

    /// <summary>The colour an axis title is drawn in.</summary>
    /// <remarks>See <see cref="LabelColour"/>.</remarks>
    public Colour AxisTitleColour { get; init; } = Colour.Black;

    /// <summary>The colour a series' data labels are drawn in.</summary>
    /// <remarks>See <see cref="LabelColour"/>.</remarks>
    public Colour DataLabelColour { get; init; } = Colour.Black;

    /// <summary>The colour a legend entry's name is drawn in.</summary>
    /// <remarks>See <see cref="LabelColour"/>.</remarks>
    public Colour LegendColour { get; init; } = Colour.Black;

    public Colour? Background { get; init; }

    /// <summary>
    /// The frame the chart draws around its whole area, or null when it draws none.
    /// </summary>
    /// <remarks>
    /// <c>c:chartSpace/c:spPr/a:ln</c>. Only a stated one: a PPTX chart's <em>automatic</em>
    /// chart-space line is <c>spNoFormats</c> — an invisible entry for every style — and the grey
    /// <c>D9D9D9</c> default <c>LineFormatter</c> otherwise supplies is explicitly skipped for
    /// the Impress filter (<c>oox/source/drawingml/chart/objectformatter.cxx:837-847</c>,
    /// tdf#150176). So a chart with no <c>a:ln</c> has no frame and one with an <c>a:ln</c> has
    /// exactly what it says.
    /// </remarks>
    public Colour? Border { get; init; }

    /// <summary>How wide <see cref="Border"/> is; zero is a hairline.</summary>
    public Length BorderWidth { get; init; }

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

    /// <summary>Whether the main title is drawn in the family's bold face.</summary>
    /// <remarks>
    /// <para>
    /// False here because this is the <c>chart2</c> model's answer, which is what an ODF chart
    /// gets: an ODF chart states its weight in the title's own <c>style:style</c> and states it
    /// on every title LibreOffice itself writes, so a reader that finds nothing has found a
    /// document that means regular.
    /// </para>
    /// <para>
    /// <strong>An OOXML chart never reaches this default</strong> — its import applies
    /// <c>objectformatter.cxx</c>'s auto-text table first, which makes a chart title bold and an
    /// axis title bold before any of the file's own properties are read. That is why
    /// <c>DrawingChartPlot</c> sets both of these true when the part states nothing, and why the
    /// two readers disagree about the default rather than sharing one.
    /// </para>
    /// </remarks>
    public bool IsTitleBold { get; init; }

    /// <summary>Whether the axis titles are drawn in the family's bold face.</summary>
    /// <remarks>See <see cref="IsTitleBold"/>; the same table decides both.</remarks>
    public bool IsAxisTitleBold { get; init; }

    /// <summary>The size the axis labels are set at; the default is 10 pt.</summary>
    public Length LabelSize { get; init; } = Length.FromPoints(10);

    /// <summary>
    /// The zoom already applied to every type size in this plot, or 1 when none was.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A chart is laid out at the size its own file states, and whatever scales the
    /// picture afterwards is not part of that.</strong> A worksheet's print zoom is the case that
    /// exists: <c>ScPrintFunc</c> scales the output device and the chart's primitives ride that
    /// transform, but <c>chart2</c> has already composed the chart on its own draw page at the
    /// stated type size and knows nothing about it. <c>SheetChart</c> takes the zoom through the
    /// type sizes instead of pushing a transform at the sink, so that every glyph run stays in
    /// page coordinates — and this records what it multiplied by, so the one measurement that is
    /// <em>not</em> proportional to the type size can be taken at the size the chart states.
    /// </para>
    /// <para>
    /// <strong>Which measurement, and why it is not proportional.</strong> A chart's text is
    /// measured on a 96 dpi device, so a face is instantiated at a whole number of pixels and its
    /// line height is <c>round(asc) + round(desc)</c> of them — see
    /// <c>MetricGrid.Chart.PixelEmScale</c> and this file's own notes on the vertical. That
    /// rounding makes the line height a step function of the size rather than a fixed fraction of
    /// the em: DejaVu Sans answers 1.2266 em at 11 pt and 1.0670 em at 7.04 pt, which is the same
    /// type at a 64% print zoom. Every length in the composition scales with the zoom and cancels;
    /// <see cref="ChartLayout"/>'s interval cap is a <em>ratio</em> of a length to that height, so
    /// it does not, and a chart on a zoomed sheet drew a denser axis than the reference for that
    /// reason alone.
    /// </para>
    /// <para>
    /// <strong>No consumer sets it any more, and the mechanism it belongs to is why.</strong>
    /// <c>SheetChart</c> multiplied the print zoom into a chart's type sizes and recorded it here;
    /// it now lays the chart out on the chart's own page and scales the finished drawing, so the
    /// zoom never reaches the composition and this is 1 everywhere. Kept because the quantity is
    /// real — a caller that does scale a chart's type has to say so — and because
    /// <see cref="ChartLayout"/>'s interval cap is still the ratio that would notice.
    /// </para>
    /// </remarks>
    public double TypeScale { get; init; } = 1.0;

    /// <summary>Whether the axis labels — and the data labels — are set in the bold face.</summary>
    /// <remarks>
    /// <para>
    /// False by default in both readers, and for once they agree: the OOXML auto-text table
    /// leaves <c>spOtherTexts</c> regular (<c>objectformatter.cxx</c>:415-434) and chart2's own
    /// model default is regular too, so a file that states nothing means regular either way. That
    /// is the difference from <see cref="IsTitleBold"/>, where the two disagree.
    /// </para>
    /// <para>
    /// <strong>It is read because it decides layout, not only appearance.</strong> The widest
    /// value label reserves the plot rectangle's left edge and the widest category label its
    /// bottom band, and a bold face is wider — so measuring a bold axis as regular puts the plot
    /// rectangle in the wrong place and every mark inside it follows. <c>171128IPAP.pptx</c>'s
    /// <c>chart4.xml</c> states <c>&lt;a:defRPr sz="900" b="1"/&gt;</c> on both axes and the
    /// reference draws those labels in Carlito-Bold; 36 of the slides corpus's 61 chart parts
    /// state a weight somewhere.
    /// </para>
    /// </remarks>
    public bool IsLabelBold { get; init; }

    /// <summary>
    /// The size a series' data labels are set at, when the file states one of their own.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A data label is not an axis label and states its size somewhere else — in the series'
    /// <c>c:dLbls/c:txPr</c> rather than on an axis — so collapsing both into
    /// <see cref="LabelSize"/> draws one of them at the other's size whenever a file states two.
    /// </para>
    /// <para>
    /// <strong>Measured, on the page that named the defect.</strong> Page 11 of
    /// <c>southern-classic-kennesaw-state-university-final.pptx</c> carries four charts;
    /// <c>chart15.xml</c> states <c>sz="1600"</c> on each of its four series' <c>c:dLbls</c> and
    /// <c>sz="1400"</c> on its category axis and its legend. LibreOffice's own <c>odp</c> of the
    /// deck writes that chart's styles as four at <c>16pt</c> and two at <c>14pt</c>, so the two
    /// sizes are in the imported model and not an artefact of the rendering; we drew all
    /// seventy-four records on the page at one flat 14 pt.
    /// </para>
    /// <para>
    /// Null rather than a default, exactly as <see cref="LegendSize"/> is: a reader that does not
    /// look for it leaves every data label reading <see cref="LabelSize"/>, which is what all
    /// three readers did before this existed.
    /// </para>
    /// </remarks>
    public Length? DataLabelSize { get; init; }

    /// <summary>The size a data label is drawn at, falling back to the axis labels'.</summary>
    public Length DataLabelFont => DataLabelSize ?? LabelSize;

    /// <summary>
    /// Whether a series' data labels are set bold, when the file states a weight of their own.
    /// </summary>
    /// <remarks>
    /// Null rather than false for the reason <see cref="DataLabelSize"/> is null: an unstated
    /// data-label weight has to keep taking the axis labels', which is what round thirty measured
    /// and what <see cref="ChartLayout"/>'s stamping pass does. This only overrides the ones the
    /// file states directly.
    /// </remarks>
    public bool? IsDataLabelBold { get; init; }

    /// <summary>Whether a data label is drawn bold, falling back to the axis labels'.</summary>
    public bool DataLabelBold => IsDataLabelBold ?? IsLabelBold;

    /// <summary>
    /// The size a legend entry's name is set at, when the legend states one of its own.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>Every length in the legend is a fraction of this and not of
    /// <see cref="LabelSize"/>.</strong> <c>lcl_placeLegendEntries</c> takes its font height from
    /// the legend's own character properties (<c>VLegend.cxx:263-320</c>) and derives the padding
    /// <c>0.33</c>, the column gap <c>0.66</c>, the key gap <c>0.22</c>, the row gap <c>0.20</c>
    /// and the key's <c>0.6</c> square from it — so a legend set larger than the axis labels is
    /// wider, taller, and pushes the plot rectangle's right edge in by the difference.
    /// </para>
    /// <para>
    /// Measured on <c>research/probes/slides-r23</c>'s decks, which vary only the legend's own
    /// <c>sz</c>: the reference's key is <strong>4.20, 5.98 and 8.39 pt</strong> and its row pitch
    /// <strong>10.34, 14.09 and 19.33 pt</strong> at a 7, 10 and 14 pt legend font, while ours was
    /// 6.00 and 14.18 at all three — the axis label size, which the probe never changes. 22 of the
    /// 61 chart parts in the slides corpus state a legend font size, at 9, 11, 12, 14, 15, 16, 18
    /// and 22 pt.
    /// </para>
    /// <para>
    /// Null rather than a 10 pt default so that a reader which does not set it keeps whatever it
    /// set <see cref="LabelSize"/> to, exactly as before this existed. Only the readers that were
    /// taught to look for it change behaviour.
    /// </para>
    /// </remarks>
    public Length? LegendSize { get; init; }

    /// <summary>The size a legend entry's name is set at, falling back to the axis labels'.</summary>
    public Length LegendFont => LegendSize ?? LabelSize;

    /// <summary>
    /// Whether a legend entry's name is set bold, when the legend states a weight of its own.
    /// </summary>
    /// <remarks>
    /// Null rather than false for the reason <see cref="LegendSize"/> is: the legend's own
    /// <c>c:txPr</c> is a separate statement from the axes', and a reader that finds none should
    /// leave the legend reading whatever the axis labels read rather than force it regular.
    /// </remarks>
    public bool? IsLegendBold { get; init; }

    /// <summary>Whether a legend entry is drawn bold, falling back to the axis labels'.</summary>
    public bool LegendBold => IsLegendBold ?? IsLabelBold;

    /// <summary>Whether the legend lists the series in reverse.</summary>
    /// <remarks>
    /// <para>
    /// <c>VSeriesPlotter::createLegendEntries</c>
    /// (<c>chart2/source/view/charttypes/VSeriesPlotter.cxx</c>:2432-2447) inserts each series'
    /// entries at the <em>front</em> of the list under two conditions and appends them otherwise:
    /// with the coordinate system's <c>SwapXAndYAxis</c> set — which is a horizontal bar chart and
    /// nothing else — the entries reverse unless the series stack in Y; and with it unset, and
    /// only for a legend at the line start or the line end, they reverse when the series
    /// <em>do</em> stack in Y. A top or bottom legend on an unswapped chart never reverses.
    /// </para>
    /// <para>
    /// Four arms measured on 26.2.4.2 rather than taken from the source, two of them controls:
    /// <c>001_advanced_powerpoint_bar.pptx</c> (horizontal bar, clustered, legend right) lists
    /// <em>Plan</em> above <em>Actual</em> where <c>002_advanced_powerpoint_column.pptx</c> and
    /// <c>006_advanced_powerpoint_area.pptx</c> — same two series, same legend position — list
    /// <em>Actual</em> above <em>Plan</em>; and <c>stacked_bar_chart.pptx</c> and
    /// <c>stacked_area_chart.pptx</c> (stacked, legend right) list <em>In-Store Sales</em> above
    /// <em>Online Sales</em> where we listed them the other way round.
    /// </para>
    /// <para>
    /// It changes the order and not the box: each column of a legend takes its own widest entry,
    /// so a one-column legend reserves the same width either way. 17 corpus documents are
    /// affected — 8 slides, 9 sheets, no words.
    /// </para>
    /// </remarks>
    public bool LegendReversed
    {
        get
        {
            bool stacked = IsStacked || IsPercentStacked;

            // SwapXAndYAxis is set by the bar template for a horizontal bar chart, so a plot that
            // is not a bar chart at all cannot be swapped whatever its Direction defaulted to.
            bool swapped = Direction == ChartBarDirection.Bar && IsBarLike;

            if (swapped) return !stacked;

            return Legend is ChartLegendPosition.Left or ChartLegendPosition.Right && stacked;
        }
    }

    /// <summary>Whether any of the chart's type groups draws bars.</summary>
    private bool IsBarLike
    {
        get
        {
            if (Kind == ChartPlotKind.Bar) return true;

            foreach (ChartSeries series in Series)
                if (series.Kind == ChartPlotKind.Bar) return true;

            return false;
        }
    }

    /// <summary>
    /// The face a legend entry's name is set in, or null to take <see cref="TextFamily"/>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A chart is not set in one face, and the legend is where that shows.</strong>
    /// <see cref="TextFamily"/> is one answer for the whole chart, and the OOXML reader arrives
    /// at it by taking the first literal <c>a:latin</c> anywhere in the part when the chart space
    /// states none of its own. That is an approximation, and on a deck whose <em>axes</em> state
    /// a face and whose legend states nothing it hands the axes' face to the legend.
    /// </para>
    /// <para>
    /// Measured on <c>001_advanced_powerpoint_bar.pptx</c> page 1, whose <c>c:catAx/c:txPr</c> and
    /// <c>c:valAx/c:txPr</c> both state <c>Arial</c>, whose <c>c:legend</c> states nothing and
    /// whose chart space states nothing: 26.2.4.2 draws the page's seventeen ten-point axis and
    /// category runs in <strong>LiberationSans</strong> — Arial's metric substitute — and its two
    /// ten-point legend runs in <strong>Carlito</strong>, the theme's Calibri. We drew all
    /// nineteen in LiberationSans. Carlito is the narrower face, so the legend's widest entry
    /// measured 27.81 pt against the reference's 25.12, the legend box came out 2.69 pt too wide,
    /// and the plot rectangle's right edge — which is
    /// <c>frame.Right − margin − legend.Width − LegendMarginX</c> — gave up exactly that. It is
    /// the residue seventeen of the corpus' fifty-seven chart pages were sitting at.
    /// </para>
    /// <para>
    /// The reference resolves each chart object separately: <c>ObjectFormatter</c>'s automatic
    /// text table names <c>XML_minor</c> for every entry it has
    /// (<c>oox/source/drawingml/chart/objectformatter.cxx</c>:415-434) and an object's own
    /// <c>c:txPr</c> overrides it for that object alone. So the precedence here is the legend's
    /// own <c>c:txPr</c>, then the chart space's, then the theme's minor face — and pointedly
    /// <em>not</em> "some other element's".
    /// </para>
    /// <para>
    /// Null keeps <see cref="TextFamily"/>, which is what the ODF and BIFF readers set and what a
    /// consumer with no theme falls back to, so nothing outside the OOXML reader moves. The other
    /// four roles — axis labels, axis titles, data labels and the main title — are <strong>not</strong>
    /// resolved this way yet, deliberately: the same census says an axis label's face differs from
    /// the one-face answer on 45 corpus documents, but every slides one among them is a pie or
    /// doughnut deck that draws no axis label at all, so there is nothing on this track to measure
    /// such a change against. See <see cref="TitleFamily"/>, which is the one other role that has
    /// its own face for its own measured reason.
    /// </para>
    /// </remarks>
    public string? LegendFamily { get; init; }

    /// <summary>
    /// The family the chart's own text is set in, or null when the file states none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A chart does not inherit the face of the shape or cell it sits in, and it is not
    /// one fixed face either.</strong> Every automatic text entry in LibreOffice's OOXML chart
    /// import names the theme's <em>minor</em> font — chart titles, axis titles and everything
    /// else alike (<c>oox/source/drawingml/chart/objectformatter.cxx</c>:415-434) — and a
    /// <c>c:txPr</c> stating <c>&lt;a:latin typeface="+mn-lt"/&gt;</c>, which is what most decks
    /// write, resolves to exactly the same face. So the chart's face is the theme's body face,
    /// with a stated <c>a:latin</c> overriding it.
    /// </para>
    /// <para>
    /// <strong>It is read because it decides layout, not appearance.</strong> The widest axis
    /// label reserves the plot area's left edge and the legend's widest entry reserves its
    /// right, so a chart measured in the wrong face has its plot rectangle in the wrong place
    /// and every mark inside it follows. Measured on the corpus: the two decks that pinned this
    /// have theme minor faces of Constantia and Calibri, whose reference renderings draw chart
    /// text in DejaVu Serif and Carlito respectively — neither of them the Liberation Sans a
    /// hardcoded default assumed, and the first of them a serif.
    /// </para>
    /// <para>
    /// Null rather than a default here on purpose: which face "no family stated" resolves to is
    /// the consumer's question, and a slide, a sheet and a text frame answer it through
    /// different caches. See <see cref="IChartTextMeasurer.Measure"/>.
    /// </para>
    /// </remarks>
    public string? TextFamily { get; init; }

    /// <summary>
    /// The face the chart's <em>main title</em> is set in, when it names one of its own, or null
    /// to take <see cref="TextFamily"/> like everything else.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>The one element of a chart that routinely disagrees with its chart space.</strong>
    /// <see cref="TextFamily"/> is the chart space's statement and reaches every axis label,
    /// legend entry and data label, which is right: they are all "the chart's text". A title is
    /// authored separately and OOXML lets it say so — <c>171128IPAP.pptx</c>'s <c>chart7.xml</c>
    /// states <c>Arial</c> on <c>c:title/c:txPr</c> and <c>Calibri</c> on
    /// <c>c:chartSpace/c:txPr</c>, and LibreOffice draws its title in Arial and everything else
    /// in Carlito.
    /// </para>
    /// <para>
    /// Nullable, and null for the ODF and BIFF readers, so neither the sheets nor the words track
    /// can move on it: those two formats state a chart's faces in one place and there is nothing
    /// yet measured to say they do otherwise. The same reason
    /// <see cref="DataLabelSize"/> is nullable.
    /// </para>
    /// </remarks>
    public string? TitleFamily { get; init; }

    /// <summary>The plot area's fill — DrawingML's wall — or null.</summary>
    public Colour? PlotBackground { get; init; }

    /// <summary>
    /// How the value axis' major gridlines are painted, or null when it has none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Present rather than absent is the question the file answers — <c>c:majorGridlines</c>,
    /// <c>chart:grid class="major"</c> — and both usually leave the paint to a default.
    /// <c>0xB3B3B3</c> is chart2's own, set on <c>GridProperties</c> as
    /// <c>LINE_COLOR = 0xb3b3b3 // gray30</c>
    /// (<c>chart2/source/model/main/GridProperties.cxx:64-66</c>), and it is what the ODF and
    /// BIFF readers supply.
    /// </para>
    /// <para>
    /// <strong>It is not what an OOXML chart draws.</strong> <c>ObjectFormatter</c>'s automatic
    /// table gives a major gridline the theme's <c>tx1</c> at <c>tint 75000</c> put through the
    /// theme's subtle line style, which on a black <c>tx1</c> is <c>0x666666</c> and not
    /// <c>0xB3B3B3</c> — and the same line style states the width, which is 0.75 pt on every
    /// theme Office ships and not a hairline. See
    /// <c>DrawingChartAutoFormat.LineColourOf</c>.
    /// </para>
    /// <para>
    /// A <see cref="ChartGrid"/> rather than a bare colour for the same reason the minor grid is
    /// one: the width is as visible as the colour and there is no reason for the two grids to
    /// carry different amounts of information about themselves.
    /// </para>
    /// </remarks>
    public ChartGrid? ValueGrid { get; init; }

    /// <summary>How the category axis' major gridlines are painted, or null.</summary>
    /// <remarks>Far rarer than <see cref="ValueGrid"/>; see it for the defaults.</remarks>
    public ChartGrid? CategoryGrid { get; init; }

    /// <summary>
    /// The colour the value axis' <em>minor</em> gridlines are drawn in, or null when it has none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>c:minorGridlines</c>, <c>chart:grid class="minor"</c>. A separate element from
    /// <see cref="ValueGrid"/> and a separate answer: an axis may state either, both or neither.
    /// </para>
    /// <para>
    /// <strong>What this is worth is a whole page, and no gate column can see it.</strong> On
    /// <c>Demick_JetBlue.pptx</c> page 4 the reference draws 28 horizontal and 21 vertical minor
    /// lines over the plot area and we drew none, which is the <c>pdf-image-diff</c> report's
    /// <em>"a solid area drawn differently, 31.18% of page"</em> — 31% of a page of pure ink
    /// difference, on a document whose only gate failure is a word count that a gridline cannot
    /// move. The brief that sent this round looking at that page had it diagnosed as a rotated
    /// category axis; the axis is drawn identically on both sides.
    /// </para>
    /// </remarks>
    public ChartGrid? ValueMinorGrid { get; init; }

    /// <summary>The colour the category axis' minor gridlines are drawn in, or null.</summary>
    /// <inheritdoc cref="ValueMinorGrid"/>
    public ChartGrid? CategoryMinorGrid { get; init; }

    /// <summary>
    /// How many sub-intervals a minor gridline divides one major interval into.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>chart2</c> models the minor grid as a <em>count of sub-intervals</em> rather than as a
    /// step, so <c>n</c> here draws <c>n − 1</c> lines strictly between each pair of major ticks
    /// and none outside them. Its own default is 2 (<c>ScaleAutomatism.cxx:75</c>) and that is
    /// what a category axis gets, because <c>AxisConverter</c>'s <c>CATEGORY</c> branch never sets
    /// a sub-increment.
    /// </para>
    /// <para>
    /// <strong>A value axis is 5, not 2, and the reason is in the reference's source.</strong>
    /// <c>AxisConverter::convertFromModel</c> (<c>oox/source/drawingml/chart/axisconverter.cxx:405-409</c>)
    /// ends its <c>REALNUMBER</c> branch with <c>// tdf#114168 If minor unit is not set then set
    /// interval to 5, as MS Excel do</c>. With both units stated it is
    /// <c>round(majorUnit / minorUnit)</c>, and a logarithmic axis stating a minor unit is 9.
    /// Measured on 26.2.4.2: <c>Demick_JetBlue.pptx</c> page 4's left axis has 8 major ticks
    /// 25.97 pt apart and 28 minor lines 5.19 pt apart — 25.97 / 5.19 = 5.00 exactly — while its
    /// category axis draws one minor line per interval, at the midpoint.
    /// </para>
    /// </remarks>
    public int ValueMinorIntervals { get; init; } = 5;

    /// <summary>How many sub-intervals the category axis' minor grid divides an interval into.</summary>
    /// <inheritdoc cref="ValueMinorIntervals"/>
    public int CategoryMinorIntervals { get; init; } = 2;

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
    /// The <em>outer</em> plot rectangle a BIFF chart states, in 1/4000 of the chart frame.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>CHFRAMEPOS</c> (0x104F) inside the primary <c>CHAXESSET</c> group. A BIFF chart has no
    /// real automatic plot area: <c>XclImpChChart::Convert</c>
    /// (<c>sc/source/filter/excel/xichart.cxx</c>:4030-4048, this tree) hands the record's
    /// rectangle straight to <c>XDiagramPositioning</c> whenever
    /// <c>IsManualPlotArea()</c> holds and both position modes are
    /// <c>EXC_CHFRAMEPOS_PARENT</c>, so no layout heuristic runs at all.
    /// </para>
    /// <para>
    /// It is the rectangle <em>including</em> the axes' labels and excluding their titles —
    /// <c>setDiagramPositionIncludingAxes</c> — which is why it stands beside
    /// <see cref="PlotArea"/> (ODF's inner <c>chart:coordinate-region</c>) rather than replacing
    /// it: what it supplies is the rectangle the labels are then taken out of.
    /// </para>
    /// <para>
    /// Kept in chart units rather than resolved by the reader because the conversion needs the
    /// frame: <c>XclImpChRoot::CalcHmmFromChartX</c> (<c>:310-318</c>) is
    /// <c>unit * n + gap</c> with <c>unit = (frame - 2 * gap) / 4000</c> and
    /// <c>gap = GetHmmFromPixelX(5.0)</c> (<c>xlchart.cxx</c>:1245-1251). The gap is
    /// <strong>250 in hundredths of a millimetre</strong> for the headless reference, because
    /// <c>XclRootData</c>'s screen-pixel width keeps its 50.0 default when there is no active
    /// frame to ask (<c>xlroot.cxx</c>:105 against :150-163) — and headless conversion has none.
    /// Measured against 26.2.4.2's own resolved model in
    /// <c>probes/chart-resid-r99/results.md</c> §1.
    /// </para>
    /// </remarks>
    public (int X, int Y, int Width, int Height)? OuterPlotAreaUnits { get; init; }

    /// <summary>
    /// Whether a pie is drawn as concentric rings — a doughnut — rather than as one filled disc.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>c:doughnutChart</c>, ODF's <c>chart:class="chart:ring"</c>. A flag rather than a
    /// <see cref="Kind"/> of its own because everything else about a doughnut is a pie: the
    /// wedges, the colours per point, the legend of categories, the label placement. What changes
    /// is the radius each series occupies and, through
    /// <c>getAvailablePosAndSizeForDiagram</c>'s test on the chart type, the page margin — a pie
    /// gets a flat 350 and a doughnut the ordinary two per cent.
    /// </para>
    /// <para>
    /// <strong>The hole is always half the radius, whatever the file says.</strong>
    /// <c>PieChart</c>'s constructor sets <c>m_fRadiusOffset = 1.0</c> for a ring chart and
    /// nothing else (<c>PieChart.cxx:212-216</c>), which puts ring <em>k</em> of <em>n</em>
    /// between <c>k/(n+1)</c> and <c>(k+1)/(n+1)</c> of the outer radius — so a single-ring
    /// doughnut has a hole at exactly half. OOXML's <c>c:holeSize</c> is parsed into
    /// <c>mnHoleSize</c> and then read by nothing at all, exactly as <c>c:bubbleScale</c> is, so
    /// honouring it would be a disagreement with the reference rather than a refinement of it.
    /// </para>
    /// </remarks>
    public bool Rings { get; init; }

    /// <summary>
    /// The elevation a three-dimensional pie is seen from, in degrees, or null for a flat one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>c:view3D/c:rotX</c> on a <c>c:pie3DChart</c>. <c>View3DConverter::convertFromModel</c>
    /// clamps it to <c>[0, 90]</c>, defaults it to <strong>15</strong> and subtracts 90 to get
    /// chart2's <c>RotationHorizontal</c>
    /// (<c>oox/source/drawingml/chart/plotareaconverter.cxx</c>:273-278, this tree); the value
    /// kept here is the OOXML one, so 90 is straight down and 0 is edge on.
    /// </para>
    /// <para>
    /// <strong>Only a pie has one.</strong> The corpus holds no <c>bar3DChart</c>,
    /// <c>line3DChart</c>, <c>area3DChart</c> or <c>surface3DChart</c> at all — three documents
    /// carry a <c>pie3DChart</c> and nothing else is three-dimensional — so this models the one
    /// 3-D chart there is anything to measure against. See <c>probes/pie3d-r187</c>.
    /// </para>
    /// <para>
    /// <strong>26.2.4.2 rasterises a 3-D chart</strong>, which is why its geometry had to be read
    /// out of a bitmap rather than out of the PDF's operators: the whole solid arrives as one
    /// <c>Do</c> of an RGB image with a soft mask, and only the labels and the chart's frame are
    /// drawn as operators. The consequence for a comparison is that our vector rendering can
    /// never be byte-equal to it, and the consequence for this property is that the measurement
    /// behind it is a silhouette fit.
    /// </para>
    /// </remarks>
    public double? Elevation { get; init; }

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

                // A percent stack's category is its own unit, so each half is a fraction of the
                // total rather than a count — which is what puts the axis at 0 to 1.
                if (StackTotal(at, axis) is { } total)
                {
                    positive /= total;
                    negative /= total;
                }

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

/// <summary>Where an axis draws its major tick marks.</summary>
/// <remarks>
/// <c>chart2::TickmarkStyle</c>, which is a flag pair — <c>INNER = 1</c>, <c>OUTER = 2</c> — and
/// <c>c:majorTickMark</c>'s four tokens map onto it exactly
/// (<c>oox/source/drawingml/chart/axisconverter.cxx:104-115</c>). Kept as four named states
/// rather than as two booleans because the reservation asks only about <c>OUTER</c> and the
/// drawing asks about both, and a pair of booleans lets the two questions drift apart.
/// </remarks>
public enum ChartTickMark
{
    /// <summary><c>none</c> — no tick drawn, and nothing reserved.</summary>
    None,

    /// <summary><c>in</c> — a tick inside the plot area. Reserves nothing.</summary>
    Inner,

    /// <summary><c>out</c> — a tick outside the plot area. Reserves its length.</summary>
    Outer,

    /// <summary><c>cross</c> — a tick on both sides. Reserves its outward half's length.</summary>
    Cross,
}
