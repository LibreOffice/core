using System.Xml.Linq;
using Paperless.Core.Charts;
using Paperless.Core.Units;
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

    /// <summary>
    /// <c>c:tickLblPos val="none"</c> hides the labels and leaves the axis.
    /// </summary>
    /// <remarks>
    /// One line in <c>AxisConverter::convertFromModel</c> —
    /// <c>setProperty(PROP_DisplayLabels, mnTickLabelPos != XML_none)</c>
    /// (<c>axisconverter.cxx:221</c>) — and a different property from <c>c:delete</c>, which is
    /// what makes the two separable at all. Measured on a probe: LibreOffice stops drawing the
    /// four category names and goes on drawing the axis line and its five ticks.
    /// </remarks>
    [Fact]
    public void TickLabelPositionNoneHidesTheLabelsAndKeepsTheAxis()
    {
        ChartPlot plot = Read(Bar(axes:
            """
            <c:catAx><c:axId val="1"/><c:tickLblPos val="none"/></c:catAx>
            <c:valAx><c:axId val="2"/><c:tickLblPos val="nextTo"/></c:valAx>
            """));

        plot.CategoryLabelsVisible.ShouldBeFalse();
        plot.CategoryAxisVisible.ShouldBeTrue();
        plot.ValueLabelsVisible.ShouldBeTrue();
    }

    /// <summary>An axis that states no <c>c:tickLblPos</c> draws its labels.</summary>
    /// <remarks>
    /// Only <c>none</c> hides them: <c>high</c> and <c>low</c> move where they sit and an absent
    /// element is <c>nextTo</c>. Reading the element's presence rather than its value would take
    /// the labels off every chart that states nothing, which is most of them.
    /// </remarks>
    [Fact]
    public void AnAxisStatingNoTickLabelPositionKeepsItsLabels()
    {
        ChartPlot silent = Read(Bar(axes:
            """<c:catAx><c:axId val="1"/></c:catAx><c:valAx><c:axId val="2"/></c:valAx>"""));

        ChartPlot high = Read(Bar(axes:
            """
            <c:catAx><c:axId val="1"/><c:tickLblPos val="high"/></c:catAx>
            <c:valAx><c:axId val="2"/></c:valAx>
            """));

        silent.CategoryLabelsVisible.ShouldBeTrue();
        high.CategoryLabelsVisible.ShouldBeTrue();
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

    /// <summary>
    /// A legend's own <c>c:txPr</c> is the size the legend is laid out at, and a
    /// <c>c:legendEntry</c>'s is not.
    /// </summary>
    /// <remarks>
    /// Every length in <c>lcl_placeLegendEntries</c> is a fraction of the legend's character
    /// height, and nothing read one: the layout used the <em>axis</em> label size, so a legend
    /// set at 14 pt was drawn with a 10 pt legend's key, gaps and row pitch. 22 of the 61 chart
    /// parts in the slides corpus state one, at 9, 11, 12, 14, 15, 16, 18 and 22 pt.
    ///
    /// The second half matters because <c>c:legendEntry</c> precedes <c>c:txPr</c> in the
    /// schema's order, so a reader that searches the legend's descendants for the first
    /// <c>a:defRPr</c> finds the entry's override and sizes the whole legend by it.
    /// </remarks>
    [Fact]
    public void ALegendsOwnTextPropertiesGiveItsSize()
    {
        string legend = """
            <c:legend>
              <c:legendPos val="r"/>
              <c:legendEntry><c:idx val="0"/>
                <c:txPr><a:bodyPr/><a:p><a:pPr><a:defRPr sz="2400"/></a:pPr></a:p></c:txPr>
              </c:legendEntry>
              <c:txPr><a:bodyPr/><a:p><a:pPr><a:defRPr sz="1400"/></a:pPr></a:p></c:txPr>
            </c:legend>
            """;

        Read(Bar() + legend).LegendSize.ShouldBe(Length.FromPoints(14));

        // And a legend that states none leaves it unset, so the layout keeps using the axis
        // labels' size exactly as it did before this was read at all.
        Read(Bar() + "<c:legend><c:legendPos val=\"r\"/></c:legend>").LegendSize.ShouldBeNull();
    }

    /// <summary>
    /// A series' own <c>c:dLbls/c:txPr</c> is the size its data labels are drawn at, and a
    /// single point's <c>c:dLbl</c> override is not.
    /// </summary>
    /// <remarks>
    /// The same shape as the legend's, and it was collapsed the same way: <c>LabelSize</c> was
    /// the axes' answer and the data labels took it too, so a chart stating 14 pt on its
    /// category axis and 16 pt on its series drew both at 14. Measured on page 11 of
    /// <c>southern-classic-kennesaw-state-university-final.pptx</c>, whose <c>chart15.xml</c>
    /// states exactly that pair and whose reference draws 12 records at the larger size.
    ///
    /// <c>c:dLbl</c> precedes <c>c:txPr</c> in <c>CT_DLbls</c>' order, so searching the
    /// <c>c:dLbls</c> element's descendants for the first <c>a:defRPr</c> finds one point's
    /// override and sizes every label by it.
    /// </remarks>
    [Fact]
    public void ASeriesOwnTextPropertiesGiveItsDataLabelSize()
    {
        string labels = """
            <c:dLbls>
              <c:dLbl><c:idx val="0"/>
                <c:txPr><a:bodyPr/><a:p><a:pPr><a:defRPr sz="2400" b="0"/></a:pPr></a:p></c:txPr>
              </c:dLbl>
              <c:txPr><a:bodyPr/><a:p><a:pPr><a:defRPr sz="1600" b="1"/></a:pPr></a:p></c:txPr>
            </c:dLbls>
            """;

        ChartPlot stated = Read(Bar(labels));
        stated.DataLabelSize.ShouldBe(Length.FromPoints(16));
        stated.DataLabelFont.ShouldBe(Length.FromPoints(16));
        stated.IsDataLabelBold.ShouldBe(true);

        // A chart that states none leaves both unset, so a data label keeps taking the axis
        // labels' size and weight exactly as it did before either was read.
        ChartPlot silent = Read(Bar("<c:dLbls/>"));
        silent.DataLabelSize.ShouldBeNull();
        silent.IsDataLabelBold.ShouldBeNull();
        silent.DataLabelFont.ShouldBe(silent.LabelSize);
    }

    /// <summary>
    /// The chart space's own <c>c:txPr</c> names the face, not whichever element states one
    /// first.
    /// </summary>
    /// <remarks>
    /// <c>c:chart</c> precedes <c>c:txPr</c> under <c>c:chartSpace</c>, so a part whose title
    /// names Arial and whose chart space names Calibri had every axis label, legend entry and
    /// data label measured and drawn in Arial. <c>171128IPAP.pptx</c>'s <c>chart7.xml</c> is
    /// exactly that file, and on page 38 the reference draws 44 of its 46 records in Carlito.
    /// </remarks>
    [Fact]
    public void TheChartSpacesOwnTextPropertiesNameTheFace()
    {
        string title = """
            <c:title><c:txPr><a:bodyPr/><a:p><a:pPr>
              <a:defRPr><a:latin typeface="Arial"/></a:defRPr>
            </a:pPr></a:p></c:txPr></c:title>
            """;
        string global = """
            <c:txPr><a:bodyPr/><a:p><a:pPr>
              <a:defRPr><a:latin typeface="Calibri"/></a:defRPr>
            </a:pPr></a:p></c:txPr>
            """;

        XElement space = XElement.Parse(
            $"<c:chartSpace xmlns:c=\"{C}\" xmlns:a=\"{A}\">"
            + $"<c:chart>{title}{Bar()}</c:chart>{global}</c:chartSpace>");

        DrawingChartPlot.Read(space)!.TextFamily.ShouldBe("Calibri");

        // With no global statement the first literal face anywhere is still the answer, which
        // is what every part that names one face relies on.
        XElement only = XElement.Parse(
            $"<c:chartSpace xmlns:c=\"{C}\" xmlns:a=\"{A}\">"
            + $"<c:chart>{title}{Bar()}</c:chart></c:chartSpace>");

        DrawingChartPlot.Read(only)!.TextFamily.ShouldBe("Arial");
    }

    /// <summary>
    /// A title naming its own face is carried separately, and only when it names one.
    /// </summary>
    /// <remarks>
    /// The other half of the file above. Once the chart space decides the chart's face the
    /// title's statement has nowhere to go, and on <c>171128IPAP.pptx</c> page 38 that left
    /// exactly two records wrong out of forty-six — the title's, which the reference draws in
    /// Arial while everything around it is Carlito. Census over the corpus's OOXML half: 2 of
    /// 61 chart parts on slides, and none at all on sheets or words, so no other track moves.
    /// </remarks>
    [Fact]
    public void ATitleNamingItsOwnFaceIsCarriedApartFromTheCharts()
    {
        string title = """
            <c:title><c:txPr><a:bodyPr/><a:p><a:pPr>
              <a:defRPr><a:latin typeface="Arial"/></a:defRPr>
            </a:pPr></a:p></c:txPr></c:title>
            """;
        string global = """
            <c:txPr><a:bodyPr/><a:p><a:pPr>
              <a:defRPr><a:latin typeface="Calibri"/></a:defRPr>
            </a:pPr></a:p></c:txPr>
            """;

        ChartPlot both = DrawingChartPlot.Read(XElement.Parse(
            $"<c:chartSpace xmlns:c=\"{C}\" xmlns:a=\"{A}\">"
            + $"<c:chart>{title}{Bar()}</c:chart>{global}</c:chartSpace>"))!;

        both.TextFamily.ShouldBe("Calibri");
        both.TitleFamily.ShouldBe("Arial");

        // A title that says nothing leaves the field null, so the chart's own family reaches it
        // through the stamping pass exactly as before this existed.
        ChartPlot quiet = DrawingChartPlot.Read(XElement.Parse(
            $"<c:chartSpace xmlns:c=\"{C}\" xmlns:a=\"{A}\">"
            + $"<c:chart>{Bar()}</c:chart>{global}</c:chartSpace>"))!;

        quiet.TextFamily.ShouldBe("Calibri");
        quiet.TitleFamily.ShouldBeNull();
    }
}
