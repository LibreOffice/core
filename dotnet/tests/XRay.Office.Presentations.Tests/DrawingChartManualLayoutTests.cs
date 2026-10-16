using System.Xml.Linq;
using XRay.Office.Core.Charts;
using XRay.Office.Ooxml.DrawingML;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// What happens to a <c>c:plotArea/c:layout/c:manualLayout</c> whose rectangle does not fit the
/// chart it is stated in.
/// </summary>
/// <remarks>
/// <para>
/// <strong>The position moves; the size does not shrink.</strong> The importer resolves the four
/// fractions against the chart's page and hands the result to <c>XDiagramPositioning</c>
/// (<c>PlotAreaConverter::convertPositionFromModel</c>,
/// <c>oox/source/drawingml/chart/plotareaconverter.cxx:510-538</c>), which for
/// <c>layoutTarget="inner"</c> is <c>setDiagramPositionExcludingAxes</c> and therefore
/// <c>DiagramHelper::setDiagramPositioning</c>
/// (<c>chart2/source/tools/DiagramHelper.cxx:434-476</c>). That clamps each of the four to
/// <c>[0, 1]</c> and then, if position and size still overrun,
/// <c>aNewPos.Primary = 1.0 - aNewSize.Primary</c>.
/// </para>
/// <para>
/// <strong>And nothing else touches it.</strong> <c>layoutTarget="inner"</c> sets
/// <c>PosSizeExcludeAxes</c>, which becomes <c>CreateShapeParam2D::mbUseFixedInnerSize</c>
/// (<c>chart2/source/view/main/ChartView.cxx:946-980</c>), and every call to
/// <c>VDiagram::adjustInnerSize</c> in <c>impl_createDiagramAndContent</c> is guarded by
/// <c>!rParam.mbUseFixedInnerSize</c> (<c>:559</c>, <c>:594</c>, <c>:619</c>, <c>:690</c>). So a
/// stated inner rectangle is <em>not</em> refitted around the labels that overflow it — the seat
/// an earlier round named for this. The one correction it gets is the clamp above.
/// </para>
/// <para>
/// <strong>Measured on <c>N2_E_Maestroni_Swarm_COP.pptx</c> page 7</strong>, whose Gantt states
/// <c>x=0.20148</c> and <c>w=0.82271</c> — 1.0242 between them. Taking the pair verbatim runs the
/// plot 17.4 pt off the right edge of a 720 pt frame. 26.2.4.2 draws that chart's plot rectangle
/// from <strong>225.581 to 719.660</strong> on the page, and fits the chart's own primitives into
/// the frame from 119.083 to 719.660, so the reference's left edge in the chart's coordinates is
/// <c>(225.581 - 119.083) / (719.660 - 119.083) × 720 = 127.65</c> pt, which is
/// <c>(1 - 0.82271) × 720 = 127.65</c> — the same number, with no free parameter. Shrinking the
/// width instead would put it at 145.06.
/// </para>
/// <para>
/// <strong>And a second witness, built rather than found, separates the two readings by 89 pt.</strong>
/// <c>probes/chart-layout/mkprobe3.py</c>'s <c>OVER.pptx</c> is a bar chart in a 590.4 pt frame
/// stating <c>x = 0.30</c>, <c>w = 0.85</c> — an overrun of 0.15, sixty times Maestroni's.
/// 26.2.4.2 draws its first value label <c>0</c> at 135.5…141.0, centred on the plot's left edge
/// at 138.25. Moving the position puts that edge at <c>50.4 + 0.15 × 590.4 = 138.9</c>; shrinking
/// the width puts it at 227.5.
/// </para>
/// </remarks>
public class DrawingChartManualLayoutTests
{
    private const string C = "http://schemas.openxmlformats.org/drawingml/2006/chart";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    private static ChartPlot Read(string layout)
        => DrawingChartPlot.Read(
               XElement.Parse(
                   $"<c:chartSpace xmlns:c=\"{C}\" xmlns:a=\"{A}\"><c:chart><c:plotArea>{layout}"
                   + "<c:barChart><c:barDir val=\"bar\"/><c:ser><c:val><c:numRef><c:numCache>"
                   + "<c:ptCount val=\"2\"/><c:pt idx=\"0\"><c:v>1</c:v></c:pt>"
                   + "<c:pt idx=\"1\"><c:v>2</c:v></c:pt></c:numCache></c:numRef></c:val>"
                   + "</c:ser></c:barChart></c:plotArea></c:chart></c:chartSpace>"),
               DrawingTheme.Read(null),
               office2007: false,
               null)
           ?? throw new InvalidOperationException("the reader found nothing to draw");

    private static string Manual(string x, string y, string w, string h) => $"""
        <c:layout><c:manualLayout><c:layoutTarget val="inner"/>
        <c:xMode val="edge"/><c:yMode val="edge"/>
        <c:x val="{x}"/><c:y val="{y}"/><c:w val="{w}"/><c:h val="{h}"/>
        </c:manualLayout></c:layout>
        """;

    /// <summary>A rectangle that fits is taken exactly as it is written.</summary>
    [Fact]
    public void ARectangleThatFitsIsTakenAsWritten()
    {
        ChartPlot plot = Read(Manual("0.1", "0.2", "0.6", "0.5"));

        plot.PlotAreaFraction.ShouldNotBeNull();
        (double x, double y, double w, double h) = plot.PlotAreaFraction!.Value;

        x.ShouldBe(0.1, 1e-12);
        y.ShouldBe(0.2, 1e-12);
        w.ShouldBe(0.6, 1e-12);
        h.ShouldBe(0.5, 1e-12);
    }

    /// <summary>
    /// A rectangle that overruns keeps its size and gives up its position, on both axes
    /// independently.
    /// </summary>
    /// <remarks>
    /// The Maestroni numbers are the first row. The rival reading — shrink the size to
    /// <c>1 - x</c>, which is what <c>lclCalcRelSize</c> does for a <em>title's</em> layout
    /// (<c>oox/source/drawingml/chart/converterbase.cxx:322-338</c>) and what this took for the
    /// plot area — leaves the position at 0.20148 and the width at 0.79852, which is 17.4 pt to
    /// the right of where 26.2.4.2 draws it on a 720 pt frame.
    /// </remarks>
    [Theory]
    [InlineData("0.20147577075253653", "0.82271157597837585", 0.17728842402162415)]
    [InlineData("0.5", "0.75", 0.25)]
    [InlineData("0.9", "0.2", 0.8)]
    public void AnOverrunningRectangleMovesItsPositionAndKeepsItsSize(
        string stated, string size, double moved)
    {
        ChartPlot horizontal = Read(Manual(stated, "0.05", size, "0.5"));

        horizontal.PlotAreaFraction.ShouldNotBeNull();
        horizontal.PlotAreaFraction!.Value.X.ShouldBe(moved, 1e-12);
        horizontal.PlotAreaFraction!.Value.Width.ShouldBe(double.Parse(size, System.Globalization.CultureInfo.InvariantCulture), 1e-12);

        ChartPlot vertical = Read(Manual("0.05", stated, "0.5", size));

        vertical.PlotAreaFraction.ShouldNotBeNull();
        vertical.PlotAreaFraction!.Value.Y.ShouldBe(moved, 1e-12);
        vertical.PlotAreaFraction!.Value.Height.ShouldBe(double.Parse(size, System.Globalization.CultureInfo.InvariantCulture), 1e-12);
    }

    /// <summary>
    /// A size larger than the chart takes the whole of it and the position goes to zero.
    /// </summary>
    /// <remarks>
    /// <c>lcl_ensureRange0to1</c> runs on each of the four before the overrun test, so the size is
    /// already 1 by the time <c>pos = 1 - size</c> is reached — which is why the order in
    /// <c>setDiagramPositioning</c> matters and a negative position is not reachable.
    /// </remarks>
    [Fact]
    public void ASizeLargerThanTheChartTakesAllOfItFromTheOrigin()
    {
        ChartPlot plot = Read(Manual("0.3", "0.3", "1.4", "1.2"));

        plot.PlotAreaFraction.ShouldNotBeNull();
        (double x, double y, double w, double h) = plot.PlotAreaFraction!.Value;

        x.ShouldBe(0.0, 1e-12);
        y.ShouldBe(0.0, 1e-12);
        w.ShouldBe(1.0, 1e-12);
        h.ShouldBe(1.0, 1e-12);
    }

    /// <summary>An <c>outer</c> layout is still not honoured at all.</summary>
    /// <remarks>
    /// Unchanged, and stated here so that the clamp above is not read as making the outer target
    /// reachable: an outer rectangle includes the axis labels and needs their sizes subtracted
    /// from it, which is the computation the manual path exists to avoid.
    /// </remarks>
    [Fact]
    public void AnOuterLayoutIsNotRead()
    {
        ChartPlot plot = Read(
            """
            <c:layout><c:manualLayout><c:layoutTarget val="outer"/>
            <c:xMode val="edge"/><c:yMode val="edge"/>
            <c:x val="0.5"/><c:y val="0.05"/><c:w val="0.75"/><c:h val="0.5"/>
            </c:manualLayout></c:layout>
            """);

        plot.PlotAreaFraction.ShouldBeNull();
    }
}
