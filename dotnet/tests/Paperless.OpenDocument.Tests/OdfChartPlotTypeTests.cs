using System.Xml.Linq;
using Paperless.Core.Charts;
using Shouldly;

namespace Paperless.OpenDocument.Tests;

/// <summary>
/// What the ODF drawing reader makes of the plot classes it used to refuse — and of the one it
/// still refuses.
/// </summary>
/// <remarks>
/// The one that matters is <c>chart:stock</c>, because ODF and OOXML order a stock chart's series
/// differently and nothing in either file says so. The rest are a class name each.
/// </remarks>
public class OdfChartPlotTypeTests
{
    private const string Chart = "urn:oasis:names:tc:opendocument:xmlns:chart:1.0";
    private const string ChartOoo = "http://openoffice.org/2010/chart";
    private const string Svg = "urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0";
    private const string Style = "urn:oasis:names:tc:opendocument:xmlns:style:1.0";
    private const string Table = "urn:oasis:names:tc:opendocument:xmlns:table:1.0";
    private const string Text = "urn:oasis:names:tc:opendocument:xmlns:text:1.0";
    private const string Office = "urn:oasis:names:tc:opendocument:xmlns:office:1.0";
    private const string Fo = "urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0";
    private const string Draw = "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0";
    private const string Number = "urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0";

    /// <summary>A chart of the given class, with the given plot-area content and styles.</summary>
    private static string Document(string chartClass, string plotExtra, string plotStyle, string styles) =>
        $"""
         <office:document xmlns:office="{Office}" xmlns:chart="{Chart}" xmlns:chartooo="{ChartOoo}"
                          xmlns:svg="{Svg}" xmlns:style="{Style}" xmlns:table="{Table}"
                          xmlns:text="{Text}" xmlns:fo="{Fo}" xmlns:draw="{Draw}"
                          xmlns:number="{Number}">
           <office:automatic-styles>{styles}</office:automatic-styles>
           <office:body><office:chart>
             <chart:chart chart:class="{chartClass}" svg:width="12cm" svg:height="7cm">
               <chart:plot-area {plotStyle}>
                 {plotExtra}
                 <chart:axis chart:dimension="x" chart:name="primary-x"/>
                 <chart:axis chart:dimension="y" chart:name="primary-y"/>
                 <chart:series chart:values-cell-range-address="local-table.$B$2:.$B$3"/>
                 <chart:series chart:values-cell-range-address="local-table.$C$2:.$C$3"/>
                 <chart:series chart:values-cell-range-address="local-table.$D$2:.$D$3"/>
               </chart:plot-area>
               <table:table table:name="local-table">
                 <table:table-row>
                   <table:table-cell/><table:table-cell><text:p>Low</text:p></table:table-cell>
                   <table:table-cell><text:p>High</text:p></table:table-cell>
                   <table:table-cell><text:p>Close</text:p></table:table-cell>
                 </table:table-row>
                 <table:table-row>
                   <table:table-cell><text:p>Mon</text:p></table:table-cell>
                   <table:table-cell office:value="15"><text:p>15</text:p></table:table-cell>
                   <table:table-cell office:value="25"><text:p>25</text:p></table:table-cell>
                   <table:table-cell office:value="20"><text:p>20</text:p></table:table-cell>
                 </table:table-row>
                 <table:table-row>
                   <table:table-cell><text:p>Tue</text:p></table:table-cell>
                   <table:table-cell office:value="22"><text:p>22</text:p></table:table-cell>
                   <table:table-cell office:value="37"><text:p>37</text:p></table:table-cell>
                   <table:table-cell office:value="30"><text:p>30</text:p></table:table-cell>
                 </table:table-row>
               </table:table>
             </chart:chart>
           </office:chart></office:body>
         </office:document>
         """;

    private static ChartPlot? Read(
        string chartClass, string plotExtra = "", string plotStyle = "", string styles = "")
    {
        XElement document = XElement.Parse(Document(chartClass, plotExtra, plotStyle, styles));
        XElement chart = document.Descendants(XName.Get("chart", Chart)).Single();

        return OdfChartPlot.Read(chart, new OdfChartStyles(document));
    }

    private static ChartPlot Require(
        string chartClass, string plotExtra = "", string plotStyle = "", string styles = "")
        => Read(chartClass, plotExtra, plotStyle, styles)
           ?? throw new InvalidOperationException("the reader found nothing to draw");

    /// <summary>The four classes that used to read as null now reach the right geometry.</summary>
    [Theory]
    [InlineData("chart:radar", ChartPlotKind.Radar)]
    [InlineData("chart:filled-radar", ChartPlotKind.Radar)]
    [InlineData("chart:bubble", ChartPlotKind.Bubble)]
    [InlineData("chart:stock", ChartPlotKind.Stock)]
    public void TheseClassesAreDrawnNow(string chartClass, ChartPlotKind expected)
        => Require(chartClass).Kind.ShouldBe(expected);

    /// <summary>Only <c>chart:filled-radar</c> fills; <c>chart:radar</c> strokes.</summary>
    /// <remarks>
    /// ODF's two classes cover OOXML's three <c>c:radarStyle</c> values because ODF puts the
    /// marker on the series' own <c>chart:symbol-type</c> rather than on the plot group.
    /// </remarks>
    [Fact]
    public void OnlyTheFilledRadarClassFills()
    {
        Require("chart:filled-radar").RadarStyle.ShouldBe(ChartRadarStyle.Filled);
        Require("chart:radar").RadarStyle.ShouldBe(ChartRadarStyle.Standard);
    }

    /// <summary>
    /// ODF orders a stock chart's series low, high, close — the reverse of OOXML's middle pair.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>The named trap, and the one that cost the most time on this run.</strong>
    /// <c>SchXMLChartContext.cxx:1051-1085</c> carries the comment "with japanese candlesticks:
    /// open, low, high, close; otherwise: low, high, close", where
    /// <c>typegroupconverter.cxx:517-527</c> assigns <c>values-max</c> before <c>values-min</c>.
    /// Nothing in either file says which convention it is using — the series are three anonymous
    /// sequences in both — so reading one order into the other draws every whisker upside down on
    /// files where the high and low happen to be swapped and looks entirely correct on the rest.
    /// </para>
    /// <para>
    /// <c>chart2/qa/extras/data/ods/tdf166428_Low_High_StockChart_LO248.ods</c> is the file that
    /// makes it visible: its two named columns read "Low (bottom price)" and "High (top price)",
    /// in that order, and its third series is the empty close.
    /// </para>
    /// </remarks>
    [Fact]
    public void AnOdfStockChartIsLowHighClose()
    {
        ChartPlot plot = Require("chart:stock");

        plot.Series.Select(series => series.StockRole).ShouldBe(
            [ChartStockRole.Low, ChartStockRole.High, ChartStockRole.Close]);

        // The low series carries the smaller numbers, which is the whole point of the ordering.
        plot.Series[0].Values.ShouldBe([15.0, 22.0]);
        plot.Series[1].Values.ShouldBe([25.0, 37.0]);
    }

    /// <summary>
    /// A Japanese candlestick gains an opening series in front, still with low before high.
    /// </summary>
    /// <remarks><c>chart:japanese-candle-stick</c> on the plot area's style.</remarks>
    [Fact]
    public void AJapaneseCandlestickPutsTheOpenFirst()
    {
        ChartPlot plot = Require(
            "chart:stock",
            plotStyle: """chart:style-name="ch1" """,
            styles:
            """
            <style:style style:name="ch1" style:family="chart">
              <style:chart-properties chart:japanese-candle-stick="true"/>
            </style:style>
            """);

        plot.HasUpDownBars.ShouldBeTrue();
        plot.Series.Select(series => series.StockRole).ShouldBe(
            [ChartStockRole.Open, ChartStockRole.Low, ChartStockRole.High]);
    }

    /// <summary>
    /// <c>chart:stock-range-line</c> is ODF's <c>c:hiLowLines</c>, and absent means no whisker.
    /// </summary>
    [Fact]
    public void TheRangeLineElementIsWhatTurnsTheWhiskerOn()
    {
        Require("chart:stock").HasHighLowLines.ShouldBeFalse();
        Require("chart:stock", "<chart:stock-range-line/>").HasHighLowLines.ShouldBeTrue();
    }

    /// <summary>
    /// <c>chart:surface</c> reads as a bar chart, which is the substitution the reference makes.
    /// </summary>
    /// <remarks>
    /// There is no <c>chart:class="chart:surface"</c> anywhere in
    /// <c>chart2/qa/extras/data/</c>'s 219 ODF chart classes, so nothing here is measured; what
    /// is measured is the OOXML side, where a synthesised <c>c:surfaceChart</c> converts through
    /// <c>soffice</c> to a bar chart of 25 words. Both vocabularies reach the same
    /// <c>ColumnChartType</c> in chart2, so both read the same way — see
    /// <c>Paperless.Ooxml.DrawingML.DrawingChartPlot.KindOf</c> for the measurement.
    /// </remarks>
    [Fact]
    public void ASurfaceChartReadsAsTheBarChartTheReferenceSubstitutes()
        => Read("chart:surface").ShouldNotBeNull().Kind.ShouldBe(ChartPlotKind.Bar);
}
