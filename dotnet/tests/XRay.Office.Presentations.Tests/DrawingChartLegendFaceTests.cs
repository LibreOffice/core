using System.Xml.Linq;
using XRay.Office.Core.Charts;
using XRay.Office.Ooxml.DrawingML;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// Which face a chart's legend is set in — <c>c:legend/c:txPr</c>, then the chart space's, then
/// the theme's minor Latin face.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Never some other element's, and that is the whole of this.</strong>
/// <c>DrawingChartPlot.FamilyOf</c> answers one face for the whole chart, and when the chart
/// space states no <c>c:txPr</c> it takes the first literal <c>a:latin</c> anywhere in the part.
/// On a deck whose axes state a face and whose legend states none, that hands the axes' face to
/// the legend.
/// </para>
/// <para>
/// Measured on 26.2.4.2 before this was written. <c>001_advanced_powerpoint_bar.pptx</c> states
/// <c>&lt;a:latin typeface="Arial"/&gt;</c> on both <c>c:catAx/c:txPr</c> and
/// <c>c:valAx/c:txPr</c>, nothing on <c>c:legend</c> and nothing on <c>c:chartSpace</c>. Its
/// page 1 comes out of the reference with <strong>seventeen LiberationSans runs at 10.005 pt —
/// its axis and category labels, Arial's metric substitute — and two Carlito runs at the same
/// size</strong>, which are the legend's two entries and which are the theme's Calibri. We drew
/// all nineteen in LiberationSans, so the legend's widest entry measured 27.81 pt against the
/// reference's 25.12, the legend box came out 2.69 pt too wide, and the plot rectangle's right
/// edge gave up exactly that on seventeen of the corpus' fifty-seven chart pages.
/// </para>
/// <para>
/// The mechanism is <c>ObjectFormatter</c>'s automatic text table, which names <c>XML_minor</c>
/// for every automatic entry it has (<c>oox/source/drawingml/chart/objectformatter.cxx</c>
/// :415-434) and lets an object's own <c>c:txPr</c> override it for that object alone.
/// </para>
/// <para>
/// <strong>The stated-legend arm is a unit test and nothing more</strong>, and it is worth saying
/// so: no corpus chart part states a literal <c>a:latin</c> under <c>c:legend/c:txPr</c>, so that
/// row of the precedence has no rendering behind it — only the reference's documented rule.
/// </para>
/// </remarks>
public class DrawingChartLegendFaceTests
{
    private const string C = "http://schemas.openxmlformats.org/drawingml/2006/chart";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    private static string TxPr(string face)
        => $"<c:txPr><a:bodyPr/><a:lstStyle/><a:p><a:pPr><a:defRPr>"
           + $"<a:latin typeface=\"{face}\"/></a:defRPr></a:pPr></a:p></c:txPr>";

    /// <param name="spaceFace">The chart space's own <c>c:txPr</c> face, or null for none.</param>
    /// <param name="axisFace">The face both axes state, or null for none.</param>
    /// <param name="legendFace">The legend's own <c>c:txPr</c> face, or null for none.</param>
    /// <param name="withTheme">Whether a theme is supplied at all.</param>
    private static ChartPlot Read(
        string? spaceFace, string? axisFace, string? legendFace, bool withTheme = true)
    {
        string axis = axisFace is null ? "" : TxPr(axisFace);
        string legend = $"<c:legend><c:legendPos val=\"r\"/>"
                        + (legendFace is null ? "" : TxPr(legendFace)) + "</c:legend>";
        string space = spaceFace is null ? "" : TxPr(spaceFace);

        return DrawingChartPlot.Read(
                   XElement.Parse(
                       $"""
                        <c:chartSpace xmlns:c="{C}" xmlns:a="{A}"><c:chart>
                          <c:plotArea><c:barChart><c:ser><c:val><c:numRef><c:numCache>
                            <c:ptCount val="2"/><c:pt idx="0"><c:v>1</c:v></c:pt>
                            <c:pt idx="1"><c:v>2</c:v></c:pt>
                          </c:numCache></c:numRef></c:val></c:ser></c:barChart>
                          <c:catAx><c:axId val="2"/><c:crossAx val="1"/>{axis}</c:catAx>
                          <c:valAx><c:axId val="1"/><c:crossAx val="2"/>{axis}</c:valAx>
                          </c:plotArea>
                          {legend}
                        </c:chart>{space}</c:chartSpace>
                        """),
                   DrawingTheme.Read(withTheme
                       ? XElement.Parse(
                           $"""
                            <a:theme xmlns:a="{A}"><a:themeElements><a:fontScheme name="Office">
                              <a:minorFont><a:latin typeface="Calibri"/></a:minorFont>
                            </a:fontScheme></a:themeElements></a:theme>
                            """)
                       : null),
                   office2007: false,
                   null)
               ?? throw new InvalidOperationException("the reader found nothing to draw");
    }

    /// <summary>The corpus case: the axes state a face, the legend does not, and it must not take it.</summary>
    [Fact]
    public void AnAxisFaceDoesNotReachALegendThatStatesNone()
    {
        ChartPlot plot = Read(spaceFace: null, axisFace: "Arial", legendFace: null);

        plot.TextFamily.ShouldBe("Arial", "the axis labels still take the part's stated face");
        plot.LegendFamily.ShouldBe("Calibri", "and the legend takes the theme's minor face");
    }

    /// <summary>The chart space's own statement is what a legend that states nothing inherits.</summary>
    [Fact]
    public void TheChartSpacesOwnFaceReachesTheLegend()
        => Read(spaceFace: "Verdana", axisFace: "Arial", legendFace: null)
           .LegendFamily.ShouldBe("Verdana");

    /// <summary>And the legend's own statement beats both.</summary>
    [Fact]
    public void TheLegendsOwnFaceWins()
        => Read(spaceFace: "Verdana", axisFace: "Arial", legendFace: "Courier New")
           .LegendFamily.ShouldBe("Courier New");

    /// <summary>
    /// With nothing stated anywhere and no theme, the legend is left null and
    /// <see cref="ChartPlot.TextFamily"/> decides — which is exactly the behaviour that stood
    /// before this existed.
    /// </summary>
    [Fact]
    public void NothingStatedAndNoThemeLeavesTheLegendTakingTheChartsFace()
    {
        ChartPlot plot = Read(null, null, null, withTheme: false);

        plot.LegendFamily.ShouldBeNull();
    }

    /// <summary>
    /// A chart whose objects all agree gives the legend the same face the chart has, so the
    /// change cannot move a deck that states one face throughout.
    /// </summary>
    [Fact]
    public void OneFaceThroughoutIsUnchanged()
    {
        ChartPlot plot = Read(spaceFace: "Arial", axisFace: "Arial", legendFace: null);

        plot.TextFamily.ShouldBe("Arial");
        plot.LegendFamily.ShouldBe("Arial");
    }

    /// <summary>
    /// A theme reference — <c>+mn-lt</c> — is not a face name, so a legend stating one falls
    /// through to the same answer as a legend stating nothing. That is what most decks write.
    /// </summary>
    [Fact]
    public void AThemeReferenceIsNotAFaceName()
        => Read(spaceFace: null, axisFace: "Arial", legendFace: "+mn-lt")
           .LegendFamily.ShouldBe("Calibri");
}
