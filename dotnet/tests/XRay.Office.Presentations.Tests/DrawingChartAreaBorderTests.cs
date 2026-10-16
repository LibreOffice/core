using System.Xml.Linq;
using XRay.Office.Core.Charts;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Ooxml.DrawingML;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// The grey line a chart space that states none is drawn with — and the one host that does not
/// get it.
/// </summary>
/// <remarks>
/// <c>LineFormatter</c>'s constructor gives every <c>OBJECTTYPE_CHARTSPACE</c> a solid
/// <c>D9D9D9</c> line 9525 EMU wide, and skips it when the filter name starts with <c>Impress</c>
/// — <c>oox/source/drawingml/chart/objectformatter.cxx:838-848</c>, tdf#81437, tdf#82217 and
/// tdf#150176. Reading that exception as the rule is what left this unimplemented while four
/// blind readers across two rounds reported the missing border on three unrelated spreadsheets
/// and <c>pdf-ops.py</c> agreed every time (reference 12 strokes to our 0 on
/// <c>microsoft_learn_multi_chart_examples</c>, 2 to 0 on <c>023_Waterfall</c>).
/// </remarks>
public class DrawingChartAreaBorderTests
{
    private const string C = "http://schemas.openxmlformats.org/drawingml/2006/chart";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    private static ChartPlot Plot(string spacePr, bool host)
    {
        string xml = $"""
            <c:chartSpace xmlns:c="{C}" xmlns:a="{A}">
            <c:chart><c:plotArea><c:barChart><c:ser><c:val><c:numRef><c:numCache>
              <c:ptCount val="2"/><c:pt idx="0"><c:v>1</c:v></c:pt><c:pt idx="1"><c:v>2</c:v></c:pt>
            </c:numCache></c:numRef></c:val></c:ser></c:barChart>
            <c:catAx><c:axId val="2"/><c:crossAx val="1"/></c:catAx>
            <c:valAx><c:axId val="1"/><c:crossAx val="2"/></c:valAx>
            </c:plotArea></c:chart>{spacePr}</c:chartSpace>
            """;

        return DrawingChartPlot.Read(
            XElement.Parse(xml), theme: null, office2007: false, styles: null, ranges: null,
            automaticChartAreaLine: host)!;
    }

    /// <summary>A chart space with no line of its own gets the grey 0.75 pt default.</summary>
    /// <remarks>
    /// <c>getDefaultChartAreaLineWidth()</c> returns 9525 EMU — "what MSO 2016 writes fixing
    /// incomplete MSO 2010 documents (0.75 pt in emu)".
    /// </remarks>
    [Fact]
    public void AChartSpaceStatingNoLineGetsTheGreyDefaultOutsideImpress()
    {
        ChartPlot plot = Plot("", host: true);

        // `convertEmuToHmm` rounds to a whole hundredth of a millimetre, so the reference never
        // holds the EMU the file states. See `DrawingChartAutoFormat.LineWidth` for the two legs
        // this rests on and `probes/stroke-resid-r117/results.md` §3.
        plot.Border.ShouldBe(Colour.FromRgb(0xD9D9D9));
        plot.BorderWidth.ShouldBe(Length.FromMm100(26));
    }

    /// <summary>And a slide's chart does not, which is the whole of tdf#150176.</summary>
    /// <remarks>
    /// The two cases are the same markup and differ only in the host, so this cannot pass by the
    /// reader failing to find something: the arm above proves it is found.
    /// </remarks>
    [Fact]
    public void TheSameChartSpaceGetsNoBorderUnderImpress()
        => Plot("", host: false).Border.ShouldBeNull();

    /// <summary>A stated line wins over the automatic one, colour and width together.</summary>
    /// <remarks>
    /// <c>convertFormatting</c> assigns the automatic line first and the shape's own over it, each
    /// property separately.
    /// </remarks>
    [Fact]
    public void AStatedLineWinsOverTheAutomaticOne()
    {
        ChartPlot plot = Plot(
            """<c:spPr><a:ln w="19050"><a:solidFill><a:srgbClr val="FF0000"/></a:solidFill></a:ln></c:spPr>""",
            host: true);

        // `convertEmuToHmm` rounds to a whole hundredth of a millimetre, so the reference never
        // holds the EMU the file states. See `DrawingChartAutoFormat.LineWidth` for the two legs
        // this rests on and `probes/stroke-resid-r117/results.md` §3.
        plot.Border.ShouldBe(Colour.FromRgb(0xFF0000));
        plot.BorderWidth.ShouldBe(Length.FromMm100(53));
    }

    /// <summary>An <c>a:noFill</c> is a line the file turns off, not a line it fails to state.</summary>
    /// <remarks>
    /// The distinction <see cref="DrawingChartPlot"/>'s <c>SuppressesLine</c> exists for. Reading
    /// it as "states nothing" would draw the grey border on exactly the charts that ask for none.
    /// </remarks>
    [Fact]
    public void AnExplicitNoFillLeavesTheChartAreaWithNoBorder()
        => Plot("""<c:spPr><a:ln><a:noFill/></a:ln></c:spPr>""", host: true).Border.ShouldBeNull();
}
