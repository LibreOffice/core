using System.Xml.Linq;
using XRay.Office.Core.Charts;
using XRay.Office.Core.Graphics;
using XRay.Office.Ooxml.DrawingML;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// What the reader makes of an extended ("chartex") chart part, and why it is so much less than
/// the part states.
/// </summary>
/// <remarks>
/// <para>
/// The literals here are the shape both corpus witnesses have —
/// <c>054_Problem_analysis_with_Pareto_chart_11058329.xlsx</c> and
/// <c>051_Manufacturer_defect_analysis_53db27ea.xlsx</c>, which are one template twice: two
/// <c>cx:data</c> blocks, four series alternating <c>clusteredColumn</c> and <c>paretoLine</c>,
/// and three axes.
/// </para>
/// <para>
/// Every assertion below is a measurement of 26.2.4.2's own rendering of one of those two, taken
/// out of the reference bank's content stream — see <c>probes/chart-rest-r104</c> §2.
/// </para>
/// </remarks>
public class DrawingChartexTests
{
    private const string CX = "http://schemas.microsoft.com/office/drawing/2014/chartex";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    /// <summary>The two witnesses' shape, with the series list supplied.</summary>
    private static string Part(string series, string data = "")
        => $"""
            <cx:chartSpace xmlns:cx="{CX}" xmlns:a="{A}">
              <cx:chartData>
                <cx:data id="0">
                  <cx:strDim type="cat"><cx:f>Cats</cx:f></cx:strDim>
                  <cx:numDim type="val"><cx:f>Vals</cx:f></cx:numDim>
                </cx:data>
                <cx:data id="1">
                  <cx:strDim type="cat"><cx:f>WideCats</cx:f></cx:strDim>
                  <cx:numDim type="val"><cx:f>Cumulative</cx:f></cx:numDim>
                </cx:data>
                {data}
              </cx:chartData>
              <cx:chart><cx:plotArea><cx:plotAreaRegion>
                <cx:plotSurface><cx:spPr><a:ln><a:noFill/></a:ln></cx:spPr></cx:plotSurface>
                {series}
              </cx:plotAreaRegion></cx:plotArea></cx:chart>
              <cx:spPr><a:noFill/><a:ln><a:noFill/></a:ln></cx:spPr>
            </cx:chartSpace>
            """;

    /// <summary>The workbook the two witnesses reference through hidden defined names.</summary>
    private static ChartRangeValues? Resolve(string formula) => formula switch
    {
        "Cats" => new ChartRangeValues(["a", "b", "c"], [null, null, null]),

        // The second block's category range is one cell longer than the first's — the header row
        // — which is what both witnesses state and what makes the "longest wins" reading wrong.
        "WideCats" => new ChartRangeValues(["hdr", "a", "b", "c"], [null, null, null, null]),
        "Vals" => new ChartRangeValues(["35", "25", "21"], [35, 25, 21]),
        "Cumulative" => new ChartRangeValues(["0.5", "0.8", "1"], [0.5, 0.8, 1.0]),
        _ => null,
    };

    private static ChartPlot Require(string markup)
        => DrawingChartex.Read(XElement.Parse(markup), ranges: Resolve)
           ?? throw new InvalidOperationException("the reader found nothing to draw");

    private const string TwoColumnsTwoParetos =
        """
        <cx:series layoutId="clusteredColumn" formatIdx="0">
          <cx:tx><cx:txData><cx:f>Head</cx:f><cx:v>OCCURRENCES</cx:v></cx:txData></cx:tx>
          <cx:spPr><a:solidFill><a:srgbClr val="4D62EF"/></a:solidFill></cx:spPr>
          <cx:dataId val="0"/><cx:axisId val="1"/>
        </cx:series>
        <cx:series layoutId="paretoLine" ownerIdx="0" formatIdx="1">
          <cx:spPr><a:ln><a:solidFill><a:srgbClr val="FD4C00"/></a:solidFill></a:ln></cx:spPr>
          <cx:axisId val="2"/>
        </cx:series>
        <cx:series layoutId="clusteredColumn" hidden="1" formatIdx="2">
          <cx:tx><cx:txData><cx:f>Head2</cx:f><cx:v>CUMULATIVE PERCENT</cx:v></cx:txData></cx:tx>
          <cx:dataId val="1"/><cx:axisId val="1"/>
        </cx:series>
        <cx:series layoutId="paretoLine" ownerIdx="2" formatIdx="3">
          <cx:axisId val="2"/>
        </cx:series>
        """;

    /// <summary>
    /// The two data-bearing series become lines and the two <c>paretoLine</c> series draw nothing.
    /// </summary>
    /// <remarks>
    /// 26.2.4.2 draws exactly two eleven-point polylines on each witness' page and no third mark:
    /// on <c>054</c>'s they are <c>(63.649, 141.149)-(556.833, 369.706)</c> and
    /// <c>(63.649, 376.640)-(556.833, 381.966)</c>, ten segments each. The two
    /// <c>cx:paretoLine</c> series carry no <c>cx:dataId</c> at all — they derive from their
    /// <c>@ownerIdx</c> — and the reference's own <c>--convert-to fods</c> gives both of them an
    /// empty <c>chart:values-cell-range-address</c>.
    /// </remarks>
    [Fact]
    public void OnlyTheSeriesThatNameDataAreDrawnAndTheyAreDrawnAsLines()
    {
        ChartPlot plot = Require(Part(TwoColumnsTwoParetos));

        plot.Kind.ShouldBe(ChartPlotKind.Line);
        plot.Series.Count.ShouldBe(2);
        plot.Series[0].Name.ShouldBe("OCCURRENCES");
        plot.Series[0].Values.ShouldBe([35, 25, 21]);
        plot.Series[1].Name.ShouldBe("CUMULATIVE PERCENT");
        plot.Series[1].Values.ShouldBe([0.5, 0.8, 1.0]);
    }

    /// <summary>The categories are the first data block's, not the longest.</summary>
    /// <remarks>
    /// <c>rTypeGroups.front()->createCategorySequence()</c>
    /// (<c>oox/source/drawingml/chart/axisconverter.cxx</c>:290, this tree). Both witnesses'
    /// second block names a range one cell longer than the first, so taking the longest spreads
    /// eleven points over twelve slots: measured on <c>054</c>, the polyline then ends at
    /// x 512.02 where 26.2.4.2's ends at 556.83, and with the first block's it ends at 556.86.
    /// </remarks>
    [Fact]
    public void TheCategoriesComeFromTheFirstDataBearingSeries()
        => Require(Part(TwoColumnsTwoParetos)).Categories.ShouldBe(["a", "b", "c"]);

    /// <summary>
    /// A chartex chart draws no axis, no tick, no label and no legend, because the reference
    /// draws none.
    /// </summary>
    /// <remarks>
    /// Counted out of 26.2.4.2's own page for both witnesses: a white plot rectangle, two
    /// polylines, and nothing else — no axis line, no tick mark, no tick label, no category
    /// label, no legend key and no title. Every one of these properties defaults to *on*, so the
    /// assertion is that the reader turned each of them off rather than that it left them alone.
    /// </remarks>
    [Fact]
    public void NoAxisFurnitureIsDrawn()
    {
        ChartPlot plot = Require(Part(TwoColumnsTwoParetos));

        plot.ValueAxisVisible.ShouldBeFalse();
        plot.CategoryAxisVisible.ShouldBeFalse();
        plot.SecondaryAxisVisible.ShouldBeFalse();
        plot.ValueLabelsVisible.ShouldBeFalse();
        plot.CategoryLabelsVisible.ShouldBeFalse();
        plot.ValueTicks.ShouldBe(ChartTickMark.None);
        plot.CategoryTicks.ShouldBe(ChartTickMark.None);
        plot.Legend.ShouldBe(ChartLegendPosition.None);
    }

    /// <summary>
    /// The plot rectangle is painted white and the chart area is left alone.
    /// </summary>
    /// <remarks>
    /// Both witnesses' <c>cx:spPr</c> is <c>a:noFill</c>, and both pages carry one white filled
    /// rectangle over the plot: on <c>051</c> it is
    /// <c>(122.969, 64.102)-(431.092, 236.696)</c>, and it is what hides the sheet's cream
    /// background — at the round's base, with no chart drawn, that rectangle held 148 032 ink
    /// pixels at 120 dpi against the reference's 2 029.
    /// </remarks>
    [Fact]
    public void ThePlotRectangleIsWhiteAndTheChartAreaIsNot()
    {
        ChartPlot plot = Require(Part(TwoColumnsTwoParetos));

        plot.PlotBackground.ShouldBe(Colour.White);
        plot.Background.ShouldBeNull();
    }

    /// <summary>A series' own <c>cx:spPr</c> decides its line colour.</summary>
    /// <remarks>
    /// <c>054</c>'s first series states <c>a:solidFill/a:schemeClr val="tx2"</c>, whose theme
    /// entry is <c>4D62EF</c>, and 26.2.4.2 strokes that series' polyline in
    /// <c>(0.302, 0.3843, 0.9373)</c> — 77, 98, 239. A <c>cx:series</c> states its fill where a
    /// <c>c:ser</c> drawn as a line would state a line, so the fill is read when there is no
    /// line.
    /// </remarks>
    [Fact]
    public void ASeriesFillBecomesItsLineColour()
        => Require(Part(TwoColumnsTwoParetos)).Series[0].Line
            .ShouldBe(new Colour(0x4D, 0x62, 0xEF));

    /// <summary>A cached <c>cx:lvl</c> is used where the part states one.</summary>
    /// <remarks>
    /// Neither corpus witness states a cache — both reference the workbook through hidden
    /// <c>_xlchart.v1.n</c> defined names — but the element exists, and the order here is the one
    /// <c>ExcelChartConverter::createDataSequence</c> uses for a <c>c:</c> chart.
    /// </remarks>
    [Fact]
    public void ACachedSequenceIsReadWithoutTheWorkbook()
    {
        ChartPlot plot = DrawingChartex.Read(XElement.Parse(
            $"""
             <cx:chartSpace xmlns:cx="{CX}" xmlns:a="{A}">
               <cx:chartData><cx:data id="0">
                 <cx:strDim type="cat"><cx:lvl ptCount="2">
                   <cx:pt idx="0">one</cx:pt><cx:pt idx="1">two</cx:pt></cx:lvl></cx:strDim>
                 <cx:numDim type="val"><cx:lvl ptCount="2">
                   <cx:pt idx="0">4</cx:pt><cx:pt idx="1">9</cx:pt></cx:lvl></cx:numDim>
               </cx:data></cx:chartData>
               <cx:chart><cx:plotArea><cx:plotAreaRegion>
                 <cx:series layoutId="clusteredColumn"><cx:dataId val="0"/></cx:series>
               </cx:plotAreaRegion></cx:plotArea></cx:chart>
             </cx:chartSpace>
             """)) ?? throw new InvalidOperationException("nothing drawable");

        plot.Categories.ShouldBe(["one", "two"]);
        plot.Series.Single().Values.ShouldBe([4, 9]);
    }

    /// <summary>A part whose every series derives from another draws nothing.</summary>
    /// <remarks>
    /// The reader answers null rather than an empty plot, which is what keeps a frame with no
    /// resolvable data from painting a white rectangle over whatever is under it.
    /// </remarks>
    [Fact]
    public void APartWhoseSeriesNameNoDataIsNotDrawn()
        => DrawingChartex.Read(XElement.Parse(Part(
            """
            <cx:series layoutId="paretoLine" ownerIdx="0"><cx:axisId val="2"/></cx:series>
            """)), ranges: Resolve).ShouldBeNull();

    /// <summary>An ordinary <c>c:chartSpace</c> is not a chartex part.</summary>
    [Fact]
    public void APlainChartPartIsRefused()
        => DrawingChartex.Read(XElement.Parse(
            "<c:chartSpace xmlns:c=\"http://schemas.openxmlformats.org/drawingml/2006/chart\"/>"))
            .ShouldBeNull();
}
