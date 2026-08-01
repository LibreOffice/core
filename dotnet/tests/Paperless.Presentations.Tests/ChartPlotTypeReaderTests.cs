using System.Xml.Linq;
using Paperless.Core.Charts;
using Paperless.Ooxml.DrawingML;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// What the OOXML drawing reader makes of the four plot-type groups added last — and of the one
/// it deliberately refuses.
/// </summary>
/// <remarks>
/// Read from markup literals rather than from documents, for the reason
/// <c>DrawingChartPlotLabelTests</c> gives: the shape being tested is the markup's. Every literal
/// here is the shape a file in <c>chart2/qa/extras/data/</c> actually has —
/// <c>testStockChart.docx</c> for the stock cases, <c>barOfPieChart2.xlsx</c> for the split,
/// <c>tdf128627.xlsx</c> for the radar and <c>invertIfNeg_bubble.xlsx</c> for the bubble.
/// </remarks>
public class ChartPlotTypeReaderTests
{
    private const string C = "http://schemas.openxmlformats.org/drawingml/2006/chart";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    private static ChartPlot? Read(string inner)
        => DrawingChartPlot.Read(XElement.Parse(
            $"<c:chartSpace xmlns:c=\"{C}\" xmlns:a=\"{A}\"><c:chart>{inner}</c:chart></c:chartSpace>"));

    private static ChartPlot Require(string inner)
        => Read(inner) ?? throw new InvalidOperationException("the reader found nothing to draw");

    /// <summary>A <c>c:val</c> holding the given numbers.</summary>
    private static string Values(params double[] numbers)
    {
        string points = string.Join("", numbers.Select((n, i) =>
            $"<c:pt idx=\"{i}\"><c:v>{n}</c:v></c:pt>"));

        return $"<c:val><c:numRef><c:numCache><c:ptCount val=\"{numbers.Length}\"/>{points}"
               + "</c:numCache></c:numRef></c:val>";
    }

    // ---------------------------------------------------------------- radar

    /// <summary><c>c:radarChart</c> is read, and <c>c:radarStyle</c> says how it is drawn.</summary>
    [Fact]
    public void ARadarChartIsReadAndItsStyleDecidesWhetherItIsFilled()
    {
        ChartPlot filled = Require(
            $"""
             <c:plotArea><c:radarChart><c:radarStyle val="filled"/>
               <c:ser>{Values(1, 2, 3)}</c:ser>
             </c:radarChart></c:plotArea>
             """);

        filled.Kind.ShouldBe(ChartPlotKind.Radar);
        filled.RadarStyle.ShouldBe(ChartRadarStyle.Filled);

        // A radar has two axes and neither is a straight line, so the cartesian machinery is
        // switched off for it exactly as it is for a pie.
        filled.HasAxes.ShouldBeFalse();

        ChartPlot marked = Require(
            $"""
             <c:plotArea><c:radarChart><c:radarStyle val="marker"/>
               <c:ser>{Values(1, 2, 3)}</c:ser>
             </c:radarChart></c:plotArea>
             """);

        marked.RadarStyle.ShouldBe(ChartRadarStyle.Marker);

        // marker is the style that turns markers on where standard leaves them off, the same
        // asymmetry c:scatterStyle has.
        marked.Series[0].Marker.ShouldBe(ChartMarker.Square);
    }

    // --------------------------------------------------------------- bubble

    /// <summary>
    /// A bubble series' three sequences are read, and its X axis is not a secondary axis.
    /// </summary>
    /// <remarks>
    /// The trap a scatter chart set and a bubble chart repeats: two <c>c:valAx</c>, no
    /// <c>c:catAx</c>, and neither of them secondary. The group's <em>first</em> <c>c:axId</c>
    /// names the X axis. Reading the second as a secondary axis gives a chart with two value axes,
    /// no X scale and every bubble in the wrong place.
    /// </remarks>
    [Fact]
    public void ABubbleChartsFirstValueAxisIsItsDomainAndNotASecondaryAxis()
    {
        ChartPlot plot = Require(
            """
            <c:plotArea><c:bubbleChart>
              <c:ser>
                <c:invertIfNegative val="1"/>
                <c:xVal><c:numRef><c:numCache><c:ptCount val="2"/>
                  <c:pt idx="0"><c:v>1</c:v></c:pt><c:pt idx="1"><c:v>4</c:v></c:pt>
                </c:numCache></c:numRef></c:xVal>
                <c:yVal><c:numRef><c:numCache><c:ptCount val="2"/>
                  <c:pt idx="0"><c:v>2</c:v></c:pt><c:pt idx="1"><c:v>5</c:v></c:pt>
                </c:numCache></c:numRef></c:yVal>
                <c:bubbleSize><c:numLit><c:ptCount val="2"/>
                  <c:pt idx="0"><c:v>3</c:v></c:pt><c:pt idx="1"><c:v>-6</c:v></c:pt>
                </c:numLit></c:bubbleSize>
              </c:ser>
              <c:bubbleScale val="150"/>
              <c:sizeRepresents val="w"/>
              <c:axId val="10"/><c:axId val="20"/>
            </c:bubbleChart>
            <c:valAx><c:axId val="10"/><c:scaling><c:min val="0"/></c:scaling></c:valAx>
            <c:valAx><c:axId val="20"/><c:scaling><c:min val="7"/></c:scaling></c:valAx>
            </c:plotArea>
            """);

        plot.Kind.ShouldBe(ChartPlotKind.Bubble);
        plot.Series[0].XValues.ShouldBe([1.0, 4.0]);
        plot.Series[0].Values.ShouldBe([2.0, 5.0]);
        plot.Series[0].SizeValues.ShouldBe([3.0, -6.0]);
        plot.Series[0].InvertIfNegative.ShouldBeTrue();

        // The group's first axId is 10, so that axis is the domain and 20 is the value axis;
        // there is no secondary axis at all.
        plot.DomainScale.Minimum.ShouldBe(0.0);
        plot.ValueScale.Minimum.ShouldBe(7.0);
        plot.SecondaryValueScale.ShouldBeNull();

        plot.BubbleScale.ShouldBe(150.0);
        plot.BubbleSizeRepresents.ShouldBe(ChartBubbleSize.Width);
    }

    // ---------------------------------------------------------------- stock

    /// <summary>
    /// A four-series stock chart is open, <em>high</em>, <em>low</em>, close — in that order.
    /// </summary>
    /// <remarks>
    /// <c>TypeGroupConverter</c> assigns <c>values-first</c>, <c>values-max</c>, <c>values-min</c>
    /// and <c>values-last</c> by position (<c>typegroupconverter.cxx:517-527</c>). <strong>ODF
    /// orders the middle pair the other way round</strong> — open, low, high, close
    /// (<c>SchXMLChartContext.cxx:1051-1085</c>) — which is why the role is resolved in each
    /// reader rather than inferred from position in the layout.
    /// </remarks>
    [Fact]
    public void AFourSeriesStockChartIsOpenHighLowClose()
    {
        ChartPlot plot = Require(
            $"""
             <c:plotArea><c:stockChart>
               <c:ser>{Values(44)}</c:ser>
               <c:ser>{Values(55)}</c:ser>
               <c:ser>{Values(11)}</c:ser>
               <c:ser>{Values(25)}</c:ser>
               <c:hiLowLines/>
               <c:upDownBars><c:gapWidth val="150"/></c:upDownBars>
             </c:stockChart></c:plotArea>
             """);

        plot.Kind.ShouldBe(ChartPlotKind.Stock);
        plot.Series.Select(s => s.StockRole).ShouldBe(
            [
                ChartStockRole.Open, ChartStockRole.High,
                ChartStockRole.Low, ChartStockRole.Close,
            ]);

        plot.HasHighLowLines.ShouldBeTrue();
        plot.HasUpDownBars.ShouldBeTrue();

        // A candlestick has no c:gapWidth of its own; the one that sizes its box lives inside
        // c:upDownBars, and testStockChart.docx states 150 there.
        plot.GapWidth.ShouldBe(150.0);
    }

    /// <summary>Three series and the open is the one that is missing, not the close.</summary>
    /// <remarks><c>int nRoleIdx = (aSeries.size() == 3) ? 1 : 0;</c>.</remarks>
    [Fact]
    public void AThreeSeriesStockChartHasNoOpen()
    {
        ChartPlot plot = Require(
            $"""
             <c:plotArea><c:stockChart>
               <c:ser>{Values(55)}</c:ser>
               <c:ser>{Values(11)}</c:ser>
               <c:ser>{Values(25)}</c:ser>
             </c:stockChart></c:plotArea>
             """);

        plot.Series.Select(s => s.StockRole).ShouldBe(
            [ChartStockRole.High, ChartStockRole.Low, ChartStockRole.Close]);

        // No c:hiLowLines means no whisker at all, and no c:upDownBars means no opening mark.
        plot.HasHighLowLines.ShouldBeFalse();
        plot.HasUpDownBars.ShouldBeFalse();
    }

    /// <summary>
    /// A stock group beside a bar group is still where the whisker settings are read from.
    /// </summary>
    /// <remarks>
    /// <c>testStockChart.docx</c> writes a <c>c:barChart</c> for its volume series <em>before</em>
    /// the <c>c:stockChart</c>, and the bar group is what decides the chart's own kind. So "the
    /// first group" and "the stock group" are two different elements, and reading
    /// <c>c:hiLowLines</c> off the first finds nothing.
    /// </remarks>
    [Fact]
    public void TheStockSettingsComeFromTheStockGroupWhereverItIs()
    {
        ChartPlot plot = Require(
            $"""
             <c:plotArea>
               <c:barChart><c:ser>{Values(70, 120)}</c:ser></c:barChart>
               <c:stockChart>
                 <c:ser>{Values(55, 57)}</c:ser>
                 <c:ser>{Values(11, 12)}</c:ser>
                 <c:ser>{Values(25, 38)}</c:ser>
                 <c:hiLowLines/>
               </c:stockChart>
             </c:plotArea>
             """);

        plot.Kind.ShouldBe(ChartPlotKind.Bar);
        plot.HasHighLowLines.ShouldBeTrue();
        plot.SeriesOf(ChartPlotKind.Stock).Count.ShouldBe(3);
        plot.SeriesOf(ChartPlotKind.Bar).Count.ShouldBe(1);
    }

    // --------------------------------------------------------------- of-pie

    /// <summary><c>c:ofPieType</c> and <c>c:splitPos</c> are read; the default position is two.</summary>
    /// <remarks><c>PieChart</c>'s constructor initialises <c>m_nSplitPos(2)</c>.</remarks>
    [Fact]
    public void AnOfPieChartCarriesItsTypeAndItsSplit()
    {
        ChartPlot bar = Require(
            $"""
             <c:plotArea><c:ofPieChart>
               <c:ofPieType val="bar"/><c:splitType val="pos"/><c:splitPos val="5"/>
               <c:ser>{Values(9, 8, 7, 6, 5, 4, 3, 2, 1)}</c:ser>
             </c:ofPieChart></c:plotArea>
             """);

        bar.Kind.ShouldBe(ChartPlotKind.OfPie);
        bar.OfPieType.ShouldBe(ChartOfPieType.Bar);
        bar.SplitType.ShouldBe(ChartSplitType.Position);
        bar.SplitPosition.ShouldBe(5);
        bar.HasAxes.ShouldBeFalse();

        ChartPlot pie = Require(
            $"""
             <c:plotArea><c:ofPieChart>
               <c:ofPieType val="pie"/>
               <c:ser>{Values(6, 5, 4, 3, 2, 1)}</c:ser>
             </c:ofPieChart></c:plotArea>
             """);

        pie.OfPieType.ShouldBe(ChartOfPieType.Pie);
        pie.SplitPosition.ShouldBe(2);
    }

    // -------------------------------------------------------------- surface

    /// <summary>
    /// A surface chart reads as nothing at all, and its frame stays empty.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Deliberate, and the discipline is the SmartArt evaluator's: a plot type that is not
    /// implemented draws nothing rather than approximately, because an absent picture reads as a
    /// missing feature and a wrong one reads as a layout bug.
    /// </para>
    /// <para>
    /// Three reasons it is not implemented. There is not one surface chart in the whole of
    /// <c>chart2/qa/extras/data/</c> — zero of 351 OOXML plot groups and zero of 219 ODF
    /// <c>chart:class</c> attributes — so there is nothing to measure against. LibreOffice has no
    /// <c>SurfaceChart</c> either and substitutes "a deep 3D bar chart from all surface charts"
    /// (<c>typegroupconverter.cxx:198-199</c>), so even the reference is a substitution. And the
    /// projection is genuinely three-dimensional, where this engine maps two fractions onto a
    /// rectangle.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData("surfaceChart")]
    [InlineData("surface3DChart")]
    public void ASurfaceChartDrawsNothingRatherThanSomethingElse(string element)
    {
        Read($"<c:plotArea><c:{element}><c:ser>{Values(1, 2, 3)}</c:ser></c:{element}></c:plotArea>")
            .ShouldBeNull();
    }
}
