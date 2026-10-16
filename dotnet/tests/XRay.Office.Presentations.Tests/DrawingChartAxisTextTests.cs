using System.Xml.Linq;
using XRay.Office.Core.Charts;
using XRay.Office.Ooxml.DrawingML;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// Which axes the OOXML importer states <c>TextOverlap</c>, <c>TextBreak</c> and
/// <c>ArrangeOrder</c> on, and which keep chart2's own model defaults.
/// </summary>
/// <remarks>
/// <para>
/// The three lines are inside
/// <c>switch (aScaleData.AxisType) case CATEGORY: case SERIES: case DATE:</c> of
/// <c>AxisConverter::convertFromModel</c> (<c>oox/source/drawingml/chart/axisconverter.cxx</c>:
/// 324-326) and inside the <c>else</c> of that case's <c>mnTypeId == C_TOKEN(dateAx)</c> test
/// (<c>:349-368</c>). So a <c>c:catAx</c> and a <c>c:serAx</c> reach them and a <c>c:dateAx</c>
/// does not — and neither does a <c>c:valAx</c>, which is set to <c>AxisType::REALNUMBER</c> or
/// <c>PERCENT</c> at <c>:306</c> and <c>:311</c> and never enters that case at all. What the ones
/// that miss out keep is <c>Axis.cxx</c>:239-242: <c>TEXT_BREAK</c> false, <c>TEXT_OVERLAP</c>
/// false, <c>ARRANGE_ORDER</c> <c>AUTO</c>.
/// </para>
/// <para>
/// <strong>It is not a detail of bookkeeping: wrapping off is the only route to a 45° axis.</strong>
/// <c>canAutoAdjustLabelPlacement</c> refuses both auto-rotation and auto-staggering while
/// <c>m_bLineBreakAllowed</c> is true (<c>chart2/source/view/axes/VCartesianAxis.cxx</c>:539-556),
/// so an axis carrying it can only raise its rhythm when its labels collide. Measured against
/// 26.2.4.2 on <c>027_Simple_personal_cash_flow_statement.xlsx</c> page 6, whose savings chart runs
/// its money axis along the bottom: the reference draws that axis' eight labels turned 45°.
/// </para>
/// <para>
/// Read from markup literals rather than from documents, as <c>DrawingChartDateAxisTests</c> does
/// and for the same reason: the shape being tested is the markup's.
/// </para>
/// </remarks>
public class DrawingChartAxisTextTests
{
    private const string C = "http://schemas.openxmlformats.org/drawingml/2006/chart";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    private static ChartPlot Require(string axes)
        => DrawingChartPlot.Read(XElement.Parse(
               $"""
                <c:chartSpace xmlns:c="{C}" xmlns:a="{A}"><c:chart><c:plotArea>
                  <c:barChart><c:ser>
                    <c:val><c:numRef><c:numCache><c:ptCount val="2"/>
                      <c:pt idx="0"><c:v>1</c:v></c:pt><c:pt idx="1"><c:v>2</c:v></c:pt>
                    </c:numCache></c:numRef></c:val>
                  </c:ser></c:barChart>{axes}
                </c:plotArea></c:chart></c:chartSpace>
                """))
           ?? throw new InvalidOperationException("the reader found nothing to draw");

    /// <summary>An axis element of the named kind, stating a rotation of exactly zero.</summary>
    private static string Axis(string kind, int id, int crosses, string rot = "0")
        => $"""
            <c:{kind}><c:axId val="{id}"/><c:scaling><c:orientation val="minMax"/></c:scaling>
              <c:txPr><a:bodyPr rot="{rot}"/><a:lstStyle/><a:p><a:pPr/><a:endParaRPr/></a:p></c:txPr>
              <c:crossAx val="{crosses}"/>
            </c:{kind}>
            """;

    /// <summary>
    /// A <c>c:catAx</c> is the one the importer states all three on.
    /// </summary>
    [Fact]
    public void ACategoryAxisTakesTheImportersOwnOverlapWrapAndArrangement()
    {
        ChartAxisText text = Require(Axis("catAx", 1, 2) + Axis("valAx", 2, 1)).CategoryAxisText;

        text.OverlapAllowed.ShouldBeTrue();
        text.LineBreakAllowed.ShouldBeTrue();
        text.Stagger.ShouldBe(ChartLabelStagger.SideBySide);
    }

    /// <summary>
    /// A <c>c:valAx</c> keeps chart2's model defaults, and wrapping off is what lets its labels
    /// turn 45° rather than thin.
    /// </summary>
    /// <remarks>
    /// The importer's <c>rot="0"</c> reading would otherwise allow overlap and wrapping here, and
    /// wrapping is the one that closes <c>canAutoAdjustLabelPlacement</c>.
    /// </remarks>
    [Fact]
    public void AValueAxisKeepsChart2sDefaultsAndSoMayStillTurn()
    {
        ChartAxisText text = Require(Axis("catAx", 1, 2) + Axis("valAx", 2, 1)).ValueAxisText;

        text.OverlapAllowed.ShouldBeFalse();
        text.LineBreakAllowed.ShouldBeFalse();
        text.Stagger.ShouldBe(ChartLabelStagger.Auto);
    }

    /// <summary>
    /// A scatter chart's domain axis is a <c>c:valAx</c> too, and it is the one that reaches
    /// <c>ChartAxisLabels.Resolve</c>, so it has to carry the same defaults.
    /// </summary>
    [Fact]
    public void AScatterDomainAxisIsAValueAxisAndKeepsThemToo()
    {
        ChartPlot plot = DrawingChartPlot.Read(XElement.Parse(
            $"""
             <c:chartSpace xmlns:c="{C}" xmlns:a="{A}"><c:chart><c:plotArea>
               <c:scatterChart><c:ser>
                 <c:xVal><c:numRef><c:numCache><c:ptCount val="2"/>
                   <c:pt idx="0"><c:v>1</c:v></c:pt><c:pt idx="1"><c:v>2</c:v></c:pt>
                 </c:numCache></c:numRef></c:xVal>
                 <c:yVal><c:numRef><c:numCache><c:ptCount val="2"/>
                   <c:pt idx="0"><c:v>3</c:v></c:pt><c:pt idx="1"><c:v>4</c:v></c:pt>
                 </c:numCache></c:numRef></c:yVal>
               </c:ser><c:axId val="1"/><c:axId val="2"/></c:scatterChart>
               {Axis("valAx", 1, 2)}{Axis("valAx", 2, 1)}
             </c:plotArea></c:chart></c:chartSpace>
             """))
            ?? throw new InvalidOperationException("the reader found nothing to draw");

        plot.CategoryAxisText.LineBreakAllowed.ShouldBeFalse();
        plot.CategoryAxisText.OverlapAllowed.ShouldBeFalse();
        plot.CategoryAxisText.Stagger.ShouldBe(ChartLabelStagger.Auto);
    }

    /// <summary>
    /// A <c>c:serAx</c> is <c>AxisType::SERIES</c>, which <em>is</em> inside the case, so it is
    /// stated exactly like a category axis. It is the arm that a "not a category axis" rule would
    /// get wrong.
    /// </summary>
    [Fact]
    public void ASeriesAxisIsStatedLikeACategoryAxis()
    {
        ChartAxisText text = Require(Axis("serAx", 1, 2) + Axis("valAx", 2, 1)).CategoryAxisText;

        text.OverlapAllowed.ShouldBeTrue();
        text.LineBreakAllowed.ShouldBeTrue();
        text.Stagger.ShouldBe(ChartLabelStagger.SideBySide);
    }

    /// <summary>
    /// A <c>c:dateAx</c> is the case that was already right, kept as the control beside the two
    /// that were not.
    /// </summary>
    [Fact]
    public void ADateAxisKeepsChart2sDefaults()
    {
        ChartAxisText text = Require(Axis("dateAx", 1, 2) + Axis("valAx", 2, 1)).CategoryAxisText;

        text.OverlapAllowed.ShouldBeFalse();
        text.LineBreakAllowed.ShouldBeFalse();
        text.Stagger.ShouldBe(ChartLabelStagger.Auto);
    }

    /// <summary>
    /// A chart with no such element at all gets chart2's defaults as well: there is no importer
    /// statement to take, and the axis chart2 makes for it carries the model's own.
    /// </summary>
    [Fact]
    public void AnAbsentAxisKeepsChart2sDefaults()
    {
        ChartAxisText text = Require(Axis("valAx", 2, 1)).CategoryAxisText;

        text.OverlapAllowed.ShouldBeFalse();
        text.LineBreakAllowed.ShouldBeFalse();
        text.Stagger.ShouldBe(ChartLabelStagger.Auto);
    }
}
