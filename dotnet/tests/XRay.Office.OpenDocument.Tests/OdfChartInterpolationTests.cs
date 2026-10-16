using System.Xml.Linq;
using XRay.Office.Core.Charts;
using Shouldly;

namespace XRay.Office.OpenDocument.Tests;

/// <summary>
/// ODF's spelling of a smoothed series — <c>chart:interpolation</c> on the plot area's style.
/// </summary>
/// <remarks>
/// <para>
/// <c>PropertyMaps.cxx</c>:101 maps the attribute onto the chart type's <c>SplineType</c>, which
/// is what chart2 resolves to a <c>CurveStyle</c>; it is a property of the plot and not of a
/// series, and that is where LibreOffice writes it. Measured over
/// <c>/home/user/corpus-odf</c>, which is 26.2.4.2's own export of the whole corpus:
/// <strong>12 of 947 renderings state it, all twelve on a <c>chart:plot-area</c>'s own style,
/// all twelve <c>cubic-spline</c>, and not one states <c>chart:spline-resolution</c></strong> —
/// so the granularity is the default 20 everywhere the corpus can see.
/// </para>
/// <para>
/// The other values are deliberately not curves here. <c>b-spline</c> is a different flattening
/// (<c>SplineCalculator::CalculateBSplines</c>, of order <c>chart:spline-order</c>) and has no
/// corpus witness; ODF 1.3's four step shapes are not curves at all; <c>none</c> is the
/// polyline.
/// </para>
/// </remarks>
public class OdfChartInterpolationTests
{
    private const string Chart = "urn:oasis:names:tc:opendocument:xmlns:chart:1.0";
    private const string Svg = "urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0";
    private const string Style = "urn:oasis:names:tc:opendocument:xmlns:style:1.0";
    private const string Table = "urn:oasis:names:tc:opendocument:xmlns:table:1.0";
    private const string Text = "urn:oasis:names:tc:opendocument:xmlns:text:1.0";
    private const string Office = "urn:oasis:names:tc:opendocument:xmlns:office:1.0";
    private const string LoExt =
        "urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0";

    /// <summary>A two-series chart of the given class whose plot area names style <c>p1</c>.</summary>
    private static string Document(string chartClass, string? interpolation, string extra = "")
    {
        string stated = interpolation is null
            ? ""
            : $"chart:interpolation=\"{interpolation}\"";

        return $"""
            <office:document xmlns:office="{Office}" xmlns:chart="{Chart}" xmlns:svg="{Svg}"
                             xmlns:style="{Style}" xmlns:table="{Table}" xmlns:text="{Text}"
                             xmlns:loext="{LoExt}">
              <office:automatic-styles>
                <style:style style:name="p1" style:family="chart">
                  <style:chart-properties {stated}/>
                </style:style>
              </office:automatic-styles>
              <office:body><office:chart>
                <chart:chart chart:class="{chartClass}" svg:width="12cm" svg:height="7cm" {extra}>
                  <chart:plot-area chart:style-name="p1">
                    <chart:axis chart:dimension="x" chart:name="primary-x"/>
                    <chart:axis chart:dimension="y" chart:name="primary-y"/>
                    <chart:series chart:values-cell-range-address="local-table.$B$2:.$B$3"/>
                    <chart:series chart:values-cell-range-address="local-table.$C$2:.$C$3"
                                  chart:class="chart:bar"/>
                  </chart:plot-area>
                  <table:table table:name="local-table">
                    <table:table-row>
                      <table:table-cell/>
                      <table:table-cell><text:p>One</text:p></table:table-cell>
                      <table:table-cell><text:p>Two</text:p></table:table-cell>
                    </table:table-row>
                    <table:table-row>
                      <table:table-cell><text:p>Mon</text:p></table:table-cell>
                      <table:table-cell office:value="15"><text:p>15</text:p></table:table-cell>
                      <table:table-cell office:value="25"><text:p>25</text:p></table:table-cell>
                    </table:table-row>
                    <table:table-row>
                      <table:table-cell><text:p>Tue</text:p></table:table-cell>
                      <table:table-cell office:value="22"><text:p>22</text:p></table:table-cell>
                      <table:table-cell office:value="37"><text:p>37</text:p></table:table-cell>
                    </table:table-row>
                  </table:table>
                </chart:chart>
              </office:chart></office:body>
            </office:document>
            """;
    }

    private static ChartPlot Read(string chartClass, string? interpolation, string extra = "")
    {
        XElement document = XElement.Parse(Document(chartClass, interpolation, extra));
        XElement chart = document.Descendants(XName.Get("chart", Chart)).Single();

        return OdfChartPlot.Read(chart, new OdfChartStyles(document))
               ?? throw new InvalidOperationException("the reader found nothing to draw");
    }

    [Fact]
    public void CubicSplineOnThePlotAreaSmoothsTheLineSeries()
        => Read("chart:line", "cubic-spline").Series[0].Smooth.ShouldBeTrue();

    [Theory]
    [InlineData(null)]
    [InlineData("none")]
    [InlineData("b-spline")]
    [InlineData("step-start")]
    public void EveryOtherValueLeavesThePolylineAlone(string? interpolation)
        => Read("chart:line", interpolation).Series[0].Smooth.ShouldBeFalse();

    /// <summary>
    /// The curve style belongs to the chart type, so a combination chart's bar series is not
    /// smoothed by the plot area's own attribute.
    /// </summary>
    [Fact]
    public void ABarSeriesInTheSamePlotAreaIsNotSmoothed()
    {
        ChartPlot plot = Read("chart:line", "cubic-spline");

        plot.Series.Count.ShouldBe(2);
        plot.Series[0].Smooth.ShouldBeTrue();
        plot.Series[1].Smooth.ShouldBeFalse();
    }

    /// <summary>A scatter chart takes it too; a pie and a radar cannot.</summary>
    [Theory]
    [InlineData("chart:scatter", true)]
    [InlineData("chart:circle", false)]
    [InlineData("chart:radar", false)]
    [InlineData("chart:bar", false)]
    public void OnlyTheNonFrameClassesTakeIt(string chartClass, bool expected)
        => Read(chartClass, "cubic-spline").Series[0].Smooth.ShouldBe(expected);

    // ---- and the other thing the of-pie has no ODF class for -------------------------------

    /// <summary>
    /// <c>loext:sub-bar</c> and <c>loext:sub-pie</c> on <c>chart:chart</c> are ODF's of-pie.
    /// </summary>
    /// <remarks>
    /// <para>
    /// ODF's <c>aXMLChartClassMap</c> has thirteen classes and no of-pie
    /// (<c>xmloff/source/chart/SchXMLTools.cxx</c>:136-151), so LibreOffice writes the sub-type
    /// as two extension attributes beside <c>chart:class="chart:circle"</c> —
    /// <c>SchXMLExport.cxx</c>:1334-1337 and :1370, read back at
    /// <c>SchXMLChartContext.cxx</c>:446-458. <strong>Only the <c>loext:</c> spelling exists</strong>,
    /// which is the namespace trap this family's notes already record five times over.
    /// </para>
    /// <para>
    /// Reach is 1 of the 947 renderings in <c>/home/user/corpus-odf</c> —
    /// <c>028_Unit_Circle_Chart_Optimized_Graph_83d9c756.odt</c>, whose
    /// <c>chart:chart</c> reads <c>chart:class="chart:circle" loext:sub-bar="true"
    /// loext:split-position="2"</c> — and reading it takes that rendering from
    /// <c>|ink|%</c> 3.54 to 1.41 against 26.2.4.2.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData("loext:sub-bar=\"true\"", ChartOfPieType.Bar)]
    [InlineData("loext:sub-pie=\"true\"", ChartOfPieType.Pie)]
    public void AnExtensionAttributeMakesTheCircleAnOfPie(string extra, ChartOfPieType expected)
    {
        ChartPlot plot = Read("chart:circle", null, extra);

        plot.Kind.ShouldBe(ChartPlotKind.OfPie);
        plot.OfPieType.ShouldBe(expected);
        plot.Series[0].Kind.ShouldBe(ChartPlotKind.OfPie);

        // chart2's own m_nSplitPos(2) when the file states none.
        plot.SplitPosition.ShouldBe(2);
    }

    [Fact]
    public void TheStatedSplitPositionIsRead()
        => Read("chart:circle", null, "loext:sub-bar=\"true\" loext:split-position=\"5\"")
            .SplitPosition.ShouldBe(5);

    /// <summary>A circle stating neither attribute is still a plain pie.</summary>
    [Fact]
    public void ACircleWithNoExtensionAttributeStaysAPie()
        => Read("chart:circle", null).Kind.ShouldBe(ChartPlotKind.Pie);
}
