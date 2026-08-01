using System.Xml.Linq;
using Paperless.Core.Charts;
using Paperless.Ooxml.DrawingML;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// What the drawing reader makes of <c>c:dLbls</c>, <c>c:numFmt</c>, <c>c:delete</c> and a
/// chart's <c>c:axId</c> pairing.
/// </summary>
/// <remarks>
/// Each of these is a piece of a chart part whose absence produces a picture that is entirely
/// plausible and wrong — labels on points the file switched off, an axis that should not be
/// there, a second value axis read as the first. Read from markup literals rather than from a
/// document, because the shape being tested is the markup's and every one of these cases came
/// from a file in <c>chart2/qa/extras/data/pptx/</c>.
/// </remarks>
public class DrawingChartPlotLabelTests
{
    private const string C = "http://schemas.openxmlformats.org/drawingml/2006/chart";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    private static ChartPlot Read(string inner)
        => DrawingChartPlot.Read(XElement.Parse(
               $"<c:chartSpace xmlns:c=\"{C}\" xmlns:a=\"{A}\"><c:chart>{inner}</c:chart></c:chartSpace>"))
           ?? throw new InvalidOperationException("the reader found nothing to draw");

    /// <summary>A one-series bar chart with the given extra markup inside the series.</summary>
    private static string Bar(string extra = "", string axes = "") =>
        $"""
         <c:plotArea><c:barChart>
           <c:ser>
             <c:val><c:numRef><c:numCache>
               <c:formatCode>#,##0</c:formatCode>
               <c:ptCount val="2"/>
               <c:pt idx="0"><c:v>20000</c:v></c:pt><c:pt idx="1"><c:v>40000</c:v></c:pt>
             </c:numCache></c:numRef></c:val>
             {extra}
           </c:ser>
           <c:axId val="1"/><c:axId val="2"/>
         </c:barChart>
         {axes}
         </c:plotArea>
         """;

    /// <summary>
    /// The five <c>show</c> flags default to <em>true</em> when a <c>c:dLbls</c> states none.
    /// </summary>
    /// <remarks>
    /// <c>value_or( !bMSO2007Doc )</c> (<c>seriesconverter.cxx:139-144</c>). The "no labels"
    /// form Excel writes is six explicit zeroes rather than silence, so defaulting to false
    /// looks right on every file that has them and loses every label on the files that do not.
    /// </remarks>
    [Fact]
    public void AnUnstatedShowFlagMeansShown()
    {
        ChartPlot plot = Read(Bar("<c:dLbls/>"));

        plot.Series[0].Label.ShouldNotBeNull();
        plot.Series[0].Label!.ShowValue.ShouldBeTrue();
        plot.Series[0].Label!.ShowCategory.ShouldBeTrue();
    }

    /// <summary>A percentage is a pie's business and nobody else's.</summary>
    /// <remarks><c>bShowPercent</c> is ANDed with the pie type category, <c>seriesconverter.cxx:141</c>.</remarks>
    [Fact]
    public void AColumnChartAskedForPercentagesShowsNone()
    {
        ChartPlot plot = Read(Bar("""<c:dLbls><c:showPercent val="1"/></c:dLbls>"""));

        plot.Series[0].Label!.ShowPercent.ShouldBeFalse();
    }

    /// <summary>
    /// The data's own <c>c:formatCode</c> is what a value label falls back to.
    /// </summary>
    /// <remarks>
    /// <c>VSeriesPlotter::getLabelTextForValue</c> asks the series through
    /// <c>detectNumberFormatKey</c> rather than the axis. Measured on <c>tdf105517.pptx</c>: its
    /// one visible label reads <c>220,000</c> in LibreOffice's PDF, the grouping coming from the
    /// <c>c:numCache</c>'s own format and from nowhere else.
    /// </remarks>
    [Fact]
    public void AValueLabelTakesTheCachedFormatCodeWhenItStatesNoneOfItsOwn()
    {
        ChartPlot plot = Read(Bar("""<c:dLbls><c:showVal val="1"/></c:dLbls>"""));

        plot.Series[0].Label!.Compose(null, null, 220000.0, 0.0).ShouldBe("220,000");
    }

    /// <summary>
    /// A deleted point label overrides the series' rather than falling through to it.
    /// </summary>
    /// <remarks>
    /// The trap: a null in the per-point array means "no override", so returning null for a
    /// deleted label restored exactly the labels the file had switched off.
    /// <c>tdf105517.pptx</c> deletes ten of eleven.
    /// </remarks>
    [Fact]
    public void ADeletedPointLabelDrawsNothing()
    {
        ChartPlot plot = Read(Bar(
            """
            <c:dLbls>
              <c:dLbl><c:idx val="0"/><c:delete val="1"/></c:dLbl>
              <c:showVal val="1"/>
            </c:dLbls>
            """));

        plot.Series[0].LabelAt(0)!.Draws.ShouldBeFalse();
        plot.Series[0].LabelAt(1)!.ShowValue.ShouldBeTrue();
    }

    /// <summary>A custom label's fields become parts rather than their own placeholder text.</summary>
    [Fact]
    public void ACustomLabelBecomesATemplate()
    {
        ChartPlot plot = Read(Bar(
            """
            <c:dLbls><c:dLbl><c:idx val="0"/><c:tx><c:rich><a:p>
              <a:r><a:t>90.0 = </a:t></a:r>
              <a:fld id="{X}" type="VALUE"><a:t>[WARTOŚĆ]</a:t></a:fld>
            </a:p></c:rich></c:tx></c:dLbl></c:dLbls>
            """));

        plot.Series[0].LabelAt(0)!.Compose(null, null, 90.0, 0.0).ShouldBe("90.0 = 90");
    }

    /// <summary>An axis' <c>c:numFmt</c> reaches the model; <c>General</c> reads as none.</summary>
    [Fact]
    public void AnAxisFormatIsReadAndGeneralIsNotOne()
    {
        ChartPlot formatted = Read(Bar(axes:
            """<c:valAx><c:axId val="2"/><c:numFmt formatCode="0.0%" sourceLinked="0"/></c:valAx>"""));

        ChartPlot general = Read(Bar(axes:
            """<c:valAx><c:axId val="2"/><c:numFmt formatCode="General" sourceLinked="0"/></c:valAx>"""));

        ChartDataLabel.Write(0.05, formatted.ValueFormat).ShouldBe("5.0%");
        general.ValueFormat.ShouldBeNull();
    }

    /// <summary>A deleted axis is recorded as hidden.</summary>
    [Fact]
    public void ADeletedAxisIsNotVisible()
    {
        ChartPlot plot = Read(Bar(axes:
            """
            <c:catAx><c:axId val="1"/><c:delete val="1"/></c:catAx>
            <c:valAx><c:axId val="2"/><c:delete val="0"/></c:valAx>
            """));

        plot.CategoryAxisVisible.ShouldBeFalse();
        plot.ValueAxisVisible.ShouldBeTrue();
    }

    /// <summary>
    /// A second <c>c:valAx</c> is a secondary axis, and the group whose ids name it is on it.
    /// </summary>
    [Fact]
    public void ASecondValueAxisIsPairedToItsOwnGroupByAxisId()
    {
        ChartPlot plot = DrawingChartPlot.Read(XElement.Parse(
            $"""
             <c:chartSpace xmlns:c="{C}" xmlns:a="{A}"><c:chart><c:plotArea>
               <c:barChart>
                 <c:ser><c:val><c:numRef><c:numCache><c:ptCount val="1"/>
                   <c:pt idx="0"><c:v>100</c:v></c:pt></c:numCache></c:numRef></c:val></c:ser>
                 <c:axId val="1"/><c:axId val="2"/>
               </c:barChart>
               <c:lineChart>
                 <c:ser><c:val><c:numRef><c:numCache><c:ptCount val="1"/>
                   <c:pt idx="0"><c:v>1</c:v></c:pt></c:numCache></c:numRef></c:val></c:ser>
                 <c:axId val="1"/><c:axId val="3"/>
               </c:lineChart>
               <c:catAx><c:axId val="1"/></c:catAx>
               <c:valAx><c:axId val="2"/></c:valAx>
               <c:valAx><c:axId val="3"/><c:scaling><c:max val="2"/></c:scaling></c:valAx>
             </c:plotArea></c:chart></c:chartSpace>
             """))!;

        plot.SecondaryValueScale.ShouldNotBeNull();
        plot.SecondaryValueScale!.Value.Maximum.ShouldBe(2.0);
        plot.Series[0].AxisIndex.ShouldBe(0);
        plot.Series[1].AxisIndex.ShouldBe(1);
        plot.HasSecondaryAxis.ShouldBeTrue();
    }

    /// <summary>
    /// A scatter chart's two <c>c:valAx</c> are an X axis and a value axis, not two value axes.
    /// </summary>
    /// <remarks>
    /// The trap this type costs an hour to: both dimensions are numeric, so the vocabulary spells
    /// the X axis <c>c:valAx</c> as well, and the group's first <c>c:axId</c> is which one it is.
    /// Reading the second as a secondary axis draws a chart with two value axes and no X scale.
    /// </remarks>
    [Fact]
    public void AScatterChartsFirstValueAxisIsItsDomainAndNotASecondaryAxis()
    {
        ChartPlot plot = DrawingChartPlot.Read(XElement.Parse(
            $"""
             <c:chartSpace xmlns:c="{C}" xmlns:a="{A}"><c:chart><c:plotArea>
               <c:scatterChart>
                 <c:scatterStyle val="lineMarker"/>
                 <c:ser>
                   <c:xVal><c:numRef><c:numCache><c:ptCount val="2"/>
                     <c:pt idx="0"><c:v>0</c:v></c:pt>
                     <c:pt idx="1"><c:v>120</c:v></c:pt></c:numCache></c:numRef></c:xVal>
                   <c:yVal><c:numRef><c:numCache><c:ptCount val="2"/>
                     <c:pt idx="0"><c:v>1</c:v></c:pt>
                     <c:pt idx="1"><c:v>2</c:v></c:pt></c:numCache></c:numRef></c:yVal>
                 </c:ser>
                 <c:axId val="10"/><c:axId val="20"/>
               </c:scatterChart>
               <c:valAx><c:axId val="10"/></c:valAx>
               <c:valAx><c:axId val="20"/></c:valAx>
             </c:plotArea></c:chart></c:chartSpace>
             """))!;

        plot.SecondaryValueScale.ShouldBeNull();
        plot.Series[0].XValues.ShouldNotBeNull();
        plot.Series[0].XValues!.Count.ShouldBe(2);
        plot.Series[0].Marker.ShouldNotBe(ChartMarker.None);
        plot.Series[0].HasLine.ShouldBeTrue();
    }

    /// <summary>A scatter chart asking for markers alone draws no line.</summary>
    [Fact]
    public void AMarkerOnlyScatterChartDrawsNoLine()
    {
        ChartPlot plot = DrawingChartPlot.Read(XElement.Parse(
            $"""
             <c:chartSpace xmlns:c="{C}" xmlns:a="{A}"><c:chart><c:plotArea>
               <c:scatterChart>
                 <c:scatterStyle val="marker"/>
                 <c:ser>
                   <c:yVal><c:numRef><c:numCache><c:ptCount val="1"/>
                     <c:pt idx="0"><c:v>1</c:v></c:pt></c:numCache></c:numRef></c:yVal>
                 </c:ser>
               </c:scatterChart>
             </c:plotArea></c:chart></c:chartSpace>
             """))!;

        plot.Series[0].HasLine.ShouldBeFalse();
        plot.Series[0].Marker.ShouldBe(ChartMarker.Square);
    }
}
