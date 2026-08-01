using Paperless.Core.Graphics;

namespace Paperless.Core.Charts;

/// <summary>How a radar chart draws each series.</summary>
/// <remarks>
/// <c>c:radarStyle</c>, ODF's <c>chart:class="chart:radar"</c> against
/// <c>"chart:filled-radar"</c>. One <c>NetChart</c> serves all three
/// (<c>chart2/source/view/charttypes/NetChart.cxx</c>); its <c>bNoArea</c> constructor argument
/// is exactly this distinction, and it decides whether the closed polygon is filled or only
/// stroked.
/// </remarks>
public enum ChartRadarStyle
{
    /// <summary>A stroked polygon and no markers — <c>c:radarStyle val="standard"</c>.</summary>
    Standard = 0,

    /// <summary>A stroked polygon with a marker at each vertex — <c>val="marker"</c>.</summary>
    Marker,

    /// <summary>A filled polygon — <c>val="filled"</c>, ODF <c>chart:filled-radar</c>.</summary>
    Filled,
}

/// <summary>What an of-pie chart's tail becomes.</summary>
/// <remarks><c>c:ofPieType</c>; <c>PieChartSubType_PIE</c> and <c>_BAR</c> in chart2.</remarks>
public enum ChartOfPieType
{
    /// <summary>A second, smaller pie to the right — <c>c:ofPieType val="pie"</c>.</summary>
    Pie = 0,

    /// <summary>A single stacked bar to the right — <c>val="bar"</c>.</summary>
    Bar,
}

/// <summary>What decides which points move into an of-pie chart's second plot.</summary>
/// <remarks>
/// <c>c:splitType</c>. Only <c>auto</c> and <c>pos</c> reach chart2 —
/// <c>TypeGroupConverter::convertFromModel</c> sets <c>PROP_SplitPos</c> for those two and for
/// no other (<c>oox/source/drawingml/chart/typegroupconverter.cxx:474-481</c>) — so
/// <c>val</c>, <c>percent</c> and <c>custom</c> fall back to the positional split rather than
/// being honoured, which is what LibreOffice itself does with them.
/// </remarks>
public enum ChartSplitType
{
    /// <summary>The application chooses — <c>val="auto"</c>, and the position is still stated.</summary>
    Auto = 0,

    /// <summary>The last <em>n</em> points, <em>n</em> being <c>c:splitPos</c>.</summary>
    Position,
}

/// <summary>What a bubble's stated size measures.</summary>
/// <remarks>
/// <para>
/// <c>c:sizeRepresents</c>, whose values are <c>area</c> (the default) and <c>w</c>. The
/// difference is a square root: an area-proportional bubble's <em>diameter</em> grows as
/// √size, a width-proportional one's grows as size itself, so a series running 1 … 9 draws its
/// largest bubble three times the smallest under one reading and nine times under the other.
/// </para>
/// <para>
/// <strong>The named trap.</strong> LibreOffice <em>parses</em> both this and
/// <c>c:bubbleScale</c> into <c>TypeGroupModel</c> and then never reads them again — a grep for
/// <c>mnSizeRepresents</c> and <c>mnBubbleScale</c> across <c>oox/</c> finds only the context
/// that writes them and the model that holds them. So LibreOffice's own rendering, which is the
/// oracle every measurement here is against, is always <see cref="Area"/> at 100%. Honouring a
/// stated <c>w</c> is therefore right by the specification and a <em>disagreement</em> with the
/// reference; it is honoured anyway, because a wrong bubble size is exactly the failure this
/// distinction exists to prevent, and because no corpus file states anything but the default.
/// </para>
/// </remarks>
public enum ChartBubbleSize
{
    /// <summary>The value is the bubble's area, so the diameter goes as its square root.</summary>
    Area = 0,

    /// <summary>The value is the bubble's width, so the diameter is proportional to it.</summary>
    Width,
}

/// <summary>Which of a stock chart's four numbers a series carries.</summary>
/// <remarks>
/// <para>
/// A stock chart is one plot made of three or four ordinary series, and which is which comes
/// from their <em>order</em> — chart2 merges them into a single <c>VDataSeries</c> carrying four
/// sequences under the roles <c>values-first</c>, <c>values-max</c>, <c>values-min</c> and
/// <c>values-last</c>.
/// </para>
/// <para>
/// <strong>The named trap, and it cost the most time on this run: the two vocabularies order the
/// middle pair the other way round.</strong> OOXML is open, <em>high</em>, <em>low</em>, close
/// (<c>typegroupconverter.cxx:517-527</c>, which assigns <c>values-max</c> before
/// <c>values-min</c> and starts at index 1 when there are three series); ODF is open,
/// <em>low</em>, <em>high</em>, close (<c>SchXMLChartContext.cxx:1051-1085</c>, whose comment
/// reads "with japanese candlesticks: open, low, high, close; otherwise: low, high, close").
/// Reading either order into the other draws whiskers that are upside down whenever high and low
/// happen to be swapped and identical whenever they are not, which is why the role is resolved in
/// each reader and carried here rather than being inferred from position in the layout.
/// </para>
/// </remarks>
public enum ChartStockRole
{
    /// <summary>Not part of a stock plot — every other chart type's series.</summary>
    None = 0,

    /// <summary>The opening price, drawn as the left tick or the box's near edge.</summary>
    Open,

    /// <summary>The high, the top of the whisker.</summary>
    High,

    /// <summary>The low, the bottom of the whisker.</summary>
    Low,

    /// <summary>The closing price, drawn as the right tick or the box's far edge.</summary>
    Close,
}

public sealed partial record ChartSeries
{
    /// <summary>
    /// A bubble series' third dimension, one size per point, or null for every other type.
    /// </summary>
    /// <remarks>
    /// <c>c:bubbleSize</c>, ODF's <c>chart:values-cell-range-address</c> on a bubble series
    /// (whose X and Y come from its two <c>chart:domain</c> children instead). A point with no
    /// size, a zero size or — unless the series states <c>c:invertIfNegative</c> — a negative one
    /// is not drawn at all, which is <c>BubbleChart::createShapes</c>'s own three <c>continue</c>s
    /// (<c>BubbleChart.cxx:237-246</c>).
    /// </remarks>
    public IReadOnlyList<double?>? SizeValues { get; init; }

    /// <summary>
    /// Whether a negative bubble size is drawn at its absolute size rather than skipped.
    /// </summary>
    /// <remarks><c>c:invertIfNegative</c>; <c>BubbleChart.cxx:220-243</c>.</remarks>
    public bool InvertIfNegative { get; init; }

    /// <summary>Which of a stock plot's numbers this series carries.</summary>
    public ChartStockRole StockRole { get; init; }
}

public sealed partial record ChartPlot
{
    /// <summary>How a radar chart's series are drawn.</summary>
    public ChartRadarStyle RadarStyle { get; init; }

    /// <summary>What an of-pie chart's second plot is.</summary>
    public ChartOfPieType OfPieType { get; init; }

    /// <summary>What decides an of-pie chart's split.</summary>
    public ChartSplitType SplitType { get; init; }

    /// <summary>
    /// How many of the series' last points move into the second plot.
    /// </summary>
    /// <remarks>
    /// <c>c:splitPos</c>. Two is chart2's own default — <c>PieChart</c>'s constructor initialises
    /// <c>m_nSplitPos(2)</c> (<c>PieChart.cxx:199</c>) — and it is what a file stating
    /// <c>c:splitType val="auto"</c> with no position gets.
    /// </remarks>
    public int SplitPosition { get; init; } = 2;

    /// <summary>What a bubble's stated size measures.</summary>
    public ChartBubbleSize BubbleSizeRepresents { get; init; }

    /// <summary>
    /// The percentage every bubble's diameter is scaled by, 100 for unscaled.
    /// </summary>
    /// <remarks>
    /// <c>c:bubbleScale</c>, whose schema range is 0 to 300. See <see cref="ChartBubbleSize"/>
    /// for why honouring it is a deliberate departure from the reference rendering.
    /// </remarks>
    public double BubbleScale { get; init; } = 100.0;

    /// <summary>
    /// Whether a stock chart draws the vertical line from each low to each high.
    /// </summary>
    /// <remarks>
    /// <c>c:hiLowLines</c>, ODF's <c>chart:stock-range-line</c>. Absent means <em>no whisker at
    /// all</em>, not a defaulted one: <c>TypeGroupConverter</c> sets the merged series'
    /// <c>LineStyle</c> to <c>NONE</c> when the element is missing, its own comment recording
    /// that "hi/low-lines cannot be switched off via ShowHighLow property"
    /// (<c>typegroupconverter.cxx:543-546</c>).
    /// </remarks>
    public bool HasHighLowLines { get; init; }

    /// <summary>
    /// Whether a stock chart draws a box between open and close rather than two ticks.
    /// </summary>
    /// <remarks>
    /// <c>c:upDownBars</c>, ODF's <c>chart:japanese-candle-stick</c> on the plot area's style.
    /// It sets both <c>Japanese</c> and <c>ShowFirst</c> on the chart type
    /// (<c>typegroupconverter.cxx:550-552</c>), which is why a file without it draws no opening
    /// mark either.
    /// </remarks>
    public bool HasUpDownBars { get; init; }

    /// <summary>The fill a rising candle's box takes, or null for chart2's white.</summary>
    public Colour? StockGainFill { get; init; }

    /// <summary>The fill a falling candle's box takes, or null for chart2's black.</summary>
    public Colour? StockLossFill { get; init; }
}
