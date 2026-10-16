using System.Xml.Linq;
using XRay.Office.Core.Charts;
using XRay.Office.Ooxml.DrawingML;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// The two statements an axis makes about which end things go at: <c>c:scaling/c:orientation</c>
/// and <c>c:tickLblPos</c>.
/// </summary>
/// <remarks>
/// Both were being dropped on the way in. <c>c:orientation</c> was read off the value axis into
/// <see cref="ChartScaleRequest.IsReversed"/> and nowhere else, so a category axis stating
/// <c>maxMin</c> — every Gantt chart in the wild — drew its categories the wrong way up; and
/// <c>c:tickLblPos</c> was read only for the word <c>none</c>, so <c>high</c> and <c>low</c> were
/// indistinguishable from the default. What each one then does to the drawing is
/// <c>ChartReversedCategoryAxisTests</c>; this is about the reading.
/// </remarks>
public class DrawingChartAxisOrientationTests
{
    private const string C = "http://schemas.openxmlformats.org/drawingml/2006/chart";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    private static ChartPlot Read(string plotArea)
        => DrawingChartPlot.Read(
               XElement.Parse(
                   $"<c:chartSpace xmlns:c=\"{C}\" xmlns:a=\"{A}\"><c:chart>{plotArea}</c:chart></c:chartSpace>"),
               DrawingTheme.Read(null),
               office2007: false,
               null)
           ?? throw new InvalidOperationException("the reader found nothing to draw");

    /// <summary>One bar chart, with whatever the two axes are given to say.</summary>
    private static string Plot(string categoryAxis, string valueAxis) => $"""
        <c:plotArea><c:barChart><c:barDir val="bar"/><c:ser><c:val><c:numRef><c:numCache>
          <c:ptCount val="2"/><c:pt idx="0"><c:v>1</c:v></c:pt><c:pt idx="1"><c:v>2</c:v></c:pt>
        </c:numCache></c:numRef></c:val></c:ser></c:barChart>
        <c:catAx><c:axId val="2"/><c:crossAx val="1"/>{categoryAxis}</c:catAx>
        <c:valAx><c:axId val="1"/><c:crossAx val="2"/>{valueAxis}</c:valAx>
        </c:plotArea>
        """;

    private const string Reversed = "<c:scaling><c:orientation val=\"maxMin\"/></c:scaling>";
    private const string Forwards = "<c:scaling><c:orientation val=\"minMax\"/></c:scaling>";

    /// <summary>A category axis' own orientation is read, and it is not the value axis'.</summary>
    /// <remarks>
    /// The two are separate statements about separate axes and a chart may make either. The
    /// second arm is the one that was silently wrong: a Gantt states <c>maxMin</c> on
    /// <c>c:catAx</c> and <c>minMax</c> on <c>c:valAx</c>, so a reader looking only at the value
    /// axis sees an entirely ordinary chart.
    /// </remarks>
    [Theory]
    [InlineData("", "", false, false)]
    [InlineData("", "<c:scaling><c:orientation val=\"maxMin\"/></c:scaling>", false, true)]
    [InlineData("<c:scaling><c:orientation val=\"maxMin\"/></c:scaling>", "", true, false)]
    [InlineData(
        "<c:scaling><c:orientation val=\"maxMin\"/></c:scaling>",
        "<c:scaling><c:orientation val=\"maxMin\"/></c:scaling>", true, true)]
    public void EachAxisOrientationIsReadFromItsOwnAxis(
        string categoryAxis, string valueAxis, bool categoriesReversed, bool valuesReversed)
    {
        ChartPlot plot = Read(Plot(categoryAxis, valueAxis));

        plot.CategoriesReversed.ShouldBe(categoriesReversed);
        plot.ValueScale.IsReversed.ShouldBe(valuesReversed);
    }

    /// <summary>An explicit <c>minMax</c> is not a reversal.</summary>
    [Fact]
    public void TheDefaultOrientationIsNotReversed()
        => Read(Plot(Forwards, Forwards)).CategoriesReversed.ShouldBeFalse();

    /// <summary>
    /// <c>c:tickLblPos</c> is read as a position, and <c>none</c> stays a visibility.
    /// </summary>
    /// <remarks>
    /// <c>lclGetLabelPosition</c> (<c>oox/source/drawingml/chart/axisconverter.cxx</c>:92-101)
    /// maps only <c>high</c>, <c>low</c> and <c>nextTo</c>; <c>none</c> falls through to the
    /// default there and is handled separately as <c>DisplayLabels</c>, which is what
    /// <see cref="ChartPlot.ValueLabelsVisible"/> carries here.
    /// </remarks>
    [Theory]
    [InlineData("", ChartValueLabelPosition.NextTo, true)]
    [InlineData("<c:tickLblPos val=\"nextTo\"/>", ChartValueLabelPosition.NextTo, true)]
    [InlineData("<c:tickLblPos val=\"low\"/>", ChartValueLabelPosition.Low, true)]
    [InlineData("<c:tickLblPos val=\"high\"/>", ChartValueLabelPosition.High, true)]
    [InlineData("<c:tickLblPos val=\"none\"/>", ChartValueLabelPosition.NextTo, false)]
    public void TheTickLabelPositionIsReadAndNoneStaysAVisibility(
        string stated, ChartValueLabelPosition position, bool visible)
    {
        ChartPlot plot = Read(Plot("", stated));

        plot.ValueLabelPosition.ShouldBe(position);
        plot.ValueLabelsVisible.ShouldBe(visible);
    }

    /// <summary>
    /// <c>c:crosses</c> is read as an end of the crossing axis.
    /// </summary>
    /// <remarks>
    /// <c>c:crossesAt</c> is deliberately not read: it names a value rather than an end, and on a
    /// category crossing axis that value is a category index. No corpus chart states one on a
    /// value axis.
    /// </remarks>
    [Theory]
    [InlineData("", ChartAxisCrossing.Automatic)]
    [InlineData("<c:crosses val=\"autoZero\"/>", ChartAxisCrossing.Automatic)]
    [InlineData("<c:crosses val=\"min\"/>", ChartAxisCrossing.Minimum)]
    [InlineData("<c:crosses val=\"max\"/>", ChartAxisCrossing.Maximum)]
    [InlineData("<c:crossesAt val=\"3\"/>", ChartAxisCrossing.Automatic)]
    public void TheCrossingPositionIsRead(string stated, ChartAxisCrossing crossing)
        => Read(Plot("", stated)).ValueAxisCrossing.ShouldBe(crossing);

    /// <summary>
    /// The shape <c>N2_E_Maestroni_Swarm_COP.pptx</c> actually states, read in one go.
    /// </summary>
    /// <remarks>
    /// A reversed category axis and a value axis that is <em>not</em> reversed but puts its labels
    /// at the far end of the one that is. Reading either half alone draws a plausible Gantt with
    /// its dates along the wrong edge.
    /// </remarks>
    [Fact]
    public void AGanttStatesAReversedCategoryAxisAndAHighValueLabelPosition()
    {
        ChartPlot plot = Read(Plot(Reversed, $"{Forwards}<c:tickLblPos val=\"high\"/>"));

        plot.CategoriesReversed.ShouldBeTrue();
        plot.ValueScale.IsReversed.ShouldBeFalse();
        plot.ValueLabelPosition.ShouldBe(ChartValueLabelPosition.High);
    }
}
