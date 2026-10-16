using System.Xml.Linq;
using XRay.Office.Core.Charts;
using XRay.Office.Ooxml.DrawingML;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// Which axis element supplies the number format for the ticks along a scatter group's X axis.
/// </summary>
/// <remarks>
/// <para>
/// A scatter chart states two <c>c:valAx</c> and <c>ChartAxes</c> picks the one the first plot
/// group names first as the domain. A <em>combination</em> chart that pairs a scatter group with
/// an area or a line group has only one <c>c:valAx</c>, because the horizontal axis is the
/// chart's <c>c:catAx</c> or <c>c:dateAx</c> — so there is no domain element and the ticks were
/// written through <c>General</c>. On <c>065_Weight_loss_tracker_ff1c89af.xlsx</c> that printed
/// a date column's serial numbers, <c>44790</c> to <c>44880</c>, along the bottom of a chart
/// whose axis states <c>&lt;c:numFmt formatCode="m/d"/&gt;</c> and whose reference draws dates.
/// </para>
/// <para>
/// The same <c>?? axes.Category</c> fallback is already what the axis' text properties, its
/// visibility and its labels take. The <em>scale</em> deliberately does not take it: it decides
/// where the points sit and not only how a tick reads, and the reference's tick positions on that
/// chart come from a date scale this does not reach. Reach of the format alone, censused over
/// every chart part in the corpus: 32 documents hold a scatter or bubble group, 4 of them state a
/// <c>c:catAx</c>/<c>c:dateAx</c> instead of a second <c>c:valAx</c>, and exactly <strong>one</strong>
/// gives that axis a format other than <c>General</c>.
/// </para>
/// </remarks>
public class DrawingChartDomainFormatTests
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

    /// <summary>A scatter group whose X axis is the chart's own date axis.</summary>
    private const string ScatterOverDateAxis = """
        <c:plotArea><c:scatterChart><c:ser>
        <c:xVal><c:numRef><c:numCache><c:formatCode>m/d</c:formatCode>
          <c:ptCount val="2"/><c:pt idx="0"><c:v>44794</c:v></c:pt>
          <c:pt idx="1"><c:v>44871</c:v></c:pt></c:numCache></c:numRef></c:xVal>
        <c:yVal><c:numRef><c:numCache><c:ptCount val="2"/>
          <c:pt idx="0"><c:v>176</c:v></c:pt><c:pt idx="1"><c:v>172</c:v></c:pt>
        </c:numCache></c:numRef></c:yVal>
        </c:ser><c:axId val="2"/><c:axId val="1"/></c:scatterChart>
        <c:dateAx><c:axId val="2"/><c:crossAx val="1"/><c:auto val="1"/>
          <c:numFmt formatCode="m/d" sourceLinked="1"/><c:baseTimeUnit val="days"/></c:dateAx>
        <c:valAx><c:axId val="1"/><c:crossAx val="2"/>
          <c:numFmt formatCode="General" sourceLinked="1"/></c:valAx>
        </c:plotArea>
        """;

    /// <summary>A proper scatter chart: two value axes, so the domain element exists.</summary>
    private const string ScatterOverTwoValueAxes = """
        <c:plotArea><c:scatterChart><c:ser>
        <c:xVal><c:numRef><c:numCache><c:ptCount val="2"/>
          <c:pt idx="0"><c:v>1</c:v></c:pt><c:pt idx="1"><c:v>2</c:v></c:pt>
        </c:numCache></c:numRef></c:xVal>
        <c:yVal><c:numRef><c:numCache><c:ptCount val="2"/>
          <c:pt idx="0"><c:v>3</c:v></c:pt><c:pt idx="1"><c:v>4</c:v></c:pt>
        </c:numCache></c:numRef></c:yVal>
        </c:ser><c:axId val="2"/><c:axId val="1"/></c:scatterChart>
        <c:valAx><c:axId val="2"/><c:crossAx val="1"/>
          <c:numFmt formatCode="0.0%" sourceLinked="0"/></c:valAx>
        <c:valAx><c:axId val="1"/><c:crossAx val="2"/>
          <c:numFmt formatCode="#,##0" sourceLinked="0"/></c:valAx>
        </c:plotArea>
        """;

    [Fact]
    public void ADateAxisStandingInForTheDomainSuppliesItsFormat()
    {
        ChartPlot plot = Read(ScatterOverDateAxis);

        NumberFormatCodeShouldBe(plot.DomainFormat, "m/d");

        // A tick along that axis is a serial, and the format is what turns it into a date.
        // 44794 is 21 August 2022.
        ChartDataLabel.Write(44794.0, plot.DomainFormat).ShouldBe("8/21");
    }

    [Fact]
    public void ASecondValueAxisStillWinsOverTheCategoryAxis()
    {
        // The ordinary case must not change: where the file states a real domain axis, its
        // format is the one used and the fallback never fires.
        ChartPlot plot = Read(ScatterOverTwoValueAxes);
        NumberFormatCodeShouldBe(plot.DomainFormat, "0.0%");
        NumberFormatCodeShouldBe(plot.ValueFormat, "#,##0");
    }

    private static void NumberFormatCodeShouldBe(
        Core.Numbers.NumberFormatCode? actual, string expected)
        => actual.ShouldNotBeNull("the axis states a format, so one must have been read")
                 .Code.ShouldBe(expected);
}
