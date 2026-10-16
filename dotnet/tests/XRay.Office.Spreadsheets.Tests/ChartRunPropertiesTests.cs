using System.Xml.Linq;
using XRay.Office.Core.Charts;
using XRay.Office.Ooxml.DrawingML;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A chart title takes its run's properties, not the paragraph default the run overrides.
/// </summary>
/// <remarks>
/// <para>
/// A <c>c:rich</c> writes <c>a:pPr/a:defRPr</c> before <c>a:r/a:rPr</c>, so a reader that takes
/// the first of either in document order reads exactly the value the run is there to override.
/// The witness is <c>003_advanced_excel_pie.xlsx</c>, whose title paragraph defaults to
/// <c>sz="1300" b="0"</c> in Arial and whose one run states <c>sz="1800" b="1"</c> in Calibri.
/// LibreOffice 26.2.4.2 draws **18.01 pt Carlito Bold** on it, twice — measured from its own PDF
/// with <c>pdf-ops.py</c> — where this reader used to produce 13.00 pt Liberation Sans.
/// </para>
/// <para>
/// The values here are the witness's, transcribed, and the fallback case beside them is what
/// keeps every element that has no runs at all reading its <c>a:defRPr</c> as before: an axis'
/// <c>c:txPr</c> and a <c>c:dLbls</c> are paragraphs with no <c>a:r</c> in them.
/// </para>
/// <para>
/// Censused over all 946 corpus documents: 169 hold a chart part and 39 hold a run that states
/// something different from its paragraph's default — 37 sheets, one deck, one document. This is
/// <c>XRay.Office.Ooxml</c> and reaches all three tracks.
/// </para>
/// </remarks>
public sealed class ChartRunPropertiesTests
{
    private const string C = "http://schemas.openxmlformats.org/drawingml/2006/chart";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    /// <summary>The witness's own title, verbatim but for the text.</summary>
    private const string WitnessTitle =
        "<c:title><c:tx><c:rich><a:bodyPr rot=\"0\"/><a:lstStyle/><a:p>"
        + "<a:pPr><a:defRPr sz=\"1300\" b=\"0\"><a:latin typeface=\"Arial\"/></a:defRPr></a:pPr>"
        + "<a:r><a:rPr sz=\"1800\" b=\"1\"><a:latin typeface=\"Calibri\"/></a:rPr>"
        + "<a:t>Rolling 12-month trend</a:t></a:r>"
        + "</a:p></c:rich></c:tx></c:title>";

    /// <summary>The same title with no run at all, which is every axis label and data label.</summary>
    private const string DefaultOnlyTitle =
        "<c:title><c:tx><c:rich><a:bodyPr rot=\"0\"/><a:lstStyle/><a:p>"
        + "<a:pPr><a:defRPr sz=\"1300\" b=\"0\"><a:latin typeface=\"Arial\"/></a:defRPr></a:pPr>"
        + "</a:p></c:rich></c:tx></c:title>";

    [Fact]
    public void ATitlesRunBeatsTheParagraphDefaultItOverrides()
    {
        ChartPlot plot = Read(WitnessTitle).ShouldNotBeNull();

        plot.TitleSize.Points.ShouldBe(18, 0.001);
        plot.IsTitleBold.ShouldBeTrue();
        plot.TitleFamily.ShouldBe("Calibri");
    }

    [Fact]
    public void ATitleWithNoRunStillReadsItsParagraphDefault()
    {
        // The control, and it is what says the change is a preference and not a replacement:
        // strip the run and the same three answers come back from the a:defRPr.
        ChartPlot plot = Read(DefaultOnlyTitle).ShouldNotBeNull();

        plot.TitleSize.Points.ShouldBe(13, 0.001);
        plot.IsTitleBold.ShouldBeFalse();
        plot.TitleFamily.ShouldBe("Arial");
    }

    private static ChartPlot? Read(string title)
        => DrawingChartPlot.Read(XElement.Parse(
            $"<c:chartSpace xmlns:c=\"{C}\" xmlns:a=\"{A}\"><c:chart>{title}<c:plotArea>"
            + "<c:barChart><c:barDir val=\"col\"/><c:ser><c:idx val=\"0\"/><c:order val=\"0\"/>"
            + "<c:val><c:numRef><c:numCache><c:ptCount val=\"2\"/>"
            + "<c:pt idx=\"0\"><c:v>1</c:v></c:pt><c:pt idx=\"1\"><c:v>2</c:v></c:pt>"
            + "</c:numCache></c:numRef></c:val></c:ser></c:barChart>"
            + "<c:catAx><c:axId val=\"1\"/></c:catAx><c:valAx><c:axId val=\"2\"/></c:valAx>"
            + "</c:plotArea></c:chart></c:chartSpace>"));
}
