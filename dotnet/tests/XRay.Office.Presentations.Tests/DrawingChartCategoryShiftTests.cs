using System.Xml.Linq;
using XRay.Office.Core.Charts;
using XRay.Office.Ooxml.DrawingML;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// Whether a chart's categories occupy slots or sit on points — <c>c:crossBetween</c>.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Measured on 26.2.4.2 before it was written, one property per arm.</strong> Nine arms:
/// three chart types by <c>between</c> / <c>midCat</c> / the element deleted, patched into three
/// corpus decks that differ in nothing else, each rendered through the reference binary and read
/// back from the category labels' own pen positions — the ratio of the label span to the plot
/// width is <c>(n−1)/n</c> when the categories are slots and 1 when they are points, and the
/// label width cancels because every label in those decks is the same width.
/// </para>
/// <code>
///                between    midCat     absent
///   areaChart    shifted    unshifted  unshifted
///   lineChart    shifted    unshifted  SHIFTED
///   barChart     shifted    SHIFTED    shifted
/// </code>
/// <para>
/// Two of the nine are the ones worth having. <em>lineChart absent</em> is shifted where an area
/// chart is not, which is <c>axisconverter.cxx:300-301</c>'s fall-back naming <c>TYPEID_LINE</c>
/// alongside the bar category. And <em>barChart midCat</em> is shifted even though the file says
/// otherwise: the column arm's rendering is byte-identical to its own <c>between</c> arm, so a
/// bar or column chart ignores the element outright. That last one is where the running binary
/// and the 27.2 source tree disagree — the tree reads the element ahead of the type for
/// everything but a 3-D bar — and the binary is the ground truth.
/// </para>
/// </remarks>
public class DrawingChartCategoryShiftTests
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

    /// <summary>One type group against one value and one category axis.</summary>
    /// <param name="group">The chart-type element name, without its namespace prefix.</param>
    /// <param name="crossBetween">The element's own value, or null to leave it out.</param>
    private static string Plot(string group, string? crossBetween)
    {
        string stated = crossBetween is null
            ? ""
            : $"<c:crossBetween val=\"{crossBetween}\"/>";

        return $"""
            <c:plotArea><c:{group}><c:ser><c:val><c:numRef><c:numCache>
              <c:ptCount val="2"/><c:pt idx="0"><c:v>1</c:v></c:pt><c:pt idx="1"><c:v>2</c:v></c:pt>
            </c:numCache></c:numRef></c:val></c:ser></c:{group}>
            <c:catAx><c:axId val="2"/><c:crossAx val="1"/></c:catAx>
            <c:valAx><c:axId val="1"/><c:crossAx val="2"/>{stated}</c:valAx>
            </c:plotArea>
            """;
    }

    // ------------------------------------------------------------------ the nine arms

    [Theory]
    [InlineData("areaChart", "between", true)]
    [InlineData("areaChart", "midCat", false)]
    [InlineData("areaChart", null, false)]
    [InlineData("lineChart", "between", true)]
    [InlineData("lineChart", "midCat", false)]
    [InlineData("lineChart", null, true)]
    [InlineData("barChart", "between", true)]
    [InlineData("barChart", "midCat", true)]
    [InlineData("barChart", null, true)]
    public void TheNineArmsOfTheProbe(string group, string? crossBetween, bool shifted)
        => Read(Plot(group, crossBetween)).ShiftedCategories.ShouldBe(shifted);

    // ------------------------------------------------------------------ controls

    /// <summary>
    /// The element is read from the axis the category axis <em>crosses</em>, not from the first
    /// <c>c:valAx</c> in the part.
    /// </summary>
    /// <remarks>
    /// <c>plotareaconverter.cxx:229-231</c> hands the category axis' converter the Y axis of its
    /// own axes set as <c>pCrossingAxis</c>. Here the first <c>c:valAx</c> in document order is
    /// the secondary one and says <c>midCat</c>; the crossing one says <c>between</c>.
    /// </remarks>
    [Fact]
    public void TheCrossingAxisDecidesAndNotTheFirstValueAxis()
    {
        ChartPlot plot = Read($"""
            <c:plotArea><c:lineChart><c:ser><c:val><c:numRef><c:numCache>
              <c:ptCount val="2"/><c:pt idx="0"><c:v>1</c:v></c:pt><c:pt idx="1"><c:v>2</c:v></c:pt>
            </c:numCache></c:numRef></c:val></c:ser></c:lineChart>
            <c:valAx><c:axId val="9"/><c:crossAx val="2"/><c:crossBetween val="midCat"/></c:valAx>
            <c:catAx><c:axId val="2"/><c:crossAx val="1"/></c:catAx>
            <c:valAx><c:axId val="1"/><c:crossAx val="2"/><c:crossBetween val="between"/></c:valAx>
            </c:plotArea>
            """);

        plot.ShiftedCategories.ShouldBeTrue();
    }

    /// <summary>
    /// A radar chart is unshifted whatever the element says — and three corpus decks say
    /// <c>between</c>, which is exactly the case that would go wrong.
    /// </summary>
    /// <remarks><c>axisconverter.cxx:295-296</c>, ahead of reading <c>c:crossBetween</c>.</remarks>
    [Fact]
    public void ARadarChartIgnoresTheElement()
        => Read(Plot("radarChart", "between")).ShiftedCategories.ShouldBeFalse();

    /// <summary>A chart with no category axis has nothing to shift.</summary>
    [Fact]
    public void AScatterChartHasNoCategoryAxisToShift()
    {
        ChartPlot plot = Read($"""
            <c:plotArea><c:scatterChart><c:ser>
              <c:xVal><c:numRef><c:numCache><c:ptCount val="2"/>
                <c:pt idx="0"><c:v>1</c:v></c:pt><c:pt idx="1"><c:v>2</c:v></c:pt>
              </c:numCache></c:numRef></c:xVal>
              <c:yVal><c:numRef><c:numCache><c:ptCount val="2"/>
                <c:pt idx="0"><c:v>3</c:v></c:pt><c:pt idx="1"><c:v>4</c:v></c:pt>
              </c:numCache></c:numRef></c:yVal>
            </c:ser></c:scatterChart>
            <c:valAx><c:axId val="1"/><c:crossAx val="2"/><c:crossBetween val="midCat"/></c:valAx>
            <c:valAx><c:axId val="2"/><c:crossAx val="1"/><c:crossBetween val="midCat"/></c:valAx>
            </c:plotArea>
            """);

        plot.ShiftedCategories.ShouldBeFalse();
    }

    /// <summary>
    /// A bar series anywhere in a combination chart shifts it, whatever the element says.
    /// </summary>
    /// <remarks>
    /// The bar test runs ahead of the stated value in <see cref="ChartPlot.ShiftedCategories"/>,
    /// which is what the column arm of the probe measured. The corpus holds one such deck in
    /// slides (<c>combo_bar_line_chart.pptx</c>) and it did not move when this was implemented.
    /// </remarks>
    [Fact]
    public void ABarInACombinationChartShiftsItWhateverTheElementSays()
    {
        ChartPlot plot = Read($"""
            <c:plotArea>
            <c:lineChart><c:ser><c:val><c:numRef><c:numCache><c:ptCount val="2"/>
              <c:pt idx="0"><c:v>1</c:v></c:pt><c:pt idx="1"><c:v>2</c:v></c:pt>
            </c:numCache></c:numRef></c:val></c:ser></c:lineChart>
            <c:barChart><c:ser><c:val><c:numRef><c:numCache><c:ptCount val="2"/>
              <c:pt idx="0"><c:v>3</c:v></c:pt><c:pt idx="1"><c:v>4</c:v></c:pt>
            </c:numCache></c:numRef></c:val></c:ser></c:barChart>
            <c:catAx><c:axId val="2"/><c:crossAx val="1"/></c:catAx>
            <c:valAx><c:axId val="1"/><c:crossAx val="2"/><c:crossBetween val="midCat"/></c:valAx>
            </c:plotArea>
            """);

        plot.ShiftedCategories.ShouldBeTrue();
    }
}
