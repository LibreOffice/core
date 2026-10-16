using System.Xml.Linq;
using XRay.Office.Core.Charts;
using XRay.Office.Core.Graphics;
using XRay.Office.Ooxml.DrawingML;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// A marker states its own paint, and a stated <c>a:noFill</c> on a series line is a suppression
/// rather than a silence. The two are one round because fixing either alone is worse than neither.
/// </summary>
/// <remarks>
/// <para>
/// Both defects meet on <c>slides/batch-016/pptx/FAAAIandtheArtandScienceofV&amp;Vfinal.pptx</c>,
/// whose single scatter series states
/// <c>&lt;c:spPr&gt;&lt;a:ln w="25400" cap="rnd"&gt;&lt;a:noFill/&gt;…</c> and then
/// <c>&lt;c:marker&gt;&lt;c:symbol val="circle"/&gt;&lt;c:size val="5"/&gt;&lt;c:spPr&gt;&lt;a:solidFill&gt;&lt;a:schemeClr
/// val="accent1"/&gt;…</c>. The reference draws its markers in the theme's raw accent 1,
/// <c>850F89</c>, seventeen times on page 7.
/// </para>
/// <para>
/// <strong>Why they are coupled.</strong> <c>ColourOf</c>'s linear-series fill table is empty
/// (<c>DrawingChartAutoFormat.cs:191</c>), so a line series has no automatic fill at all and its
/// marker was painted from <see cref="ChartSeries.Line"/> — which for this deck was the automatic
/// stroke the file had explicitly denied. Honouring the <c>a:noFill</c> on its own leaves the
/// marker with nothing to inherit and draws it black;
/// <see cref="AMarkerKeepsItsOwnColourWhenTheSeriesLineIsSuppressed"/> is that case, and it is the
/// test that fails if either half of this round is reverted.
/// </para>
/// <para>
/// The rule for a marker's colour is <c>TypeGroupConverter::convertMarker</c>
/// (<c>oox/source/drawingml/chart/typegroupconverter.cxx:657-678</c>): the symbol's own fill
/// colour, and when it has none the symbol's line colour — tdf#124817, whose comment says exactly
/// that. Only <c>a:solidFill</c> sets <c>maFillColor</c>, so a gradient marker takes the line
/// colour, which is what <c>8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx</c> states.
/// </para>
/// </remarks>
public class DrawingChartMarkerPaintTests
{
    private const string C = "http://schemas.openxmlformats.org/drawingml/2006/chart";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    /// <summary>The FAAAI deck's accent 1, and the colour its reference PDF fills the markers.</summary>
    private static readonly Colour Accent1 = new(0x85, 0x0F, 0x89);

    private static readonly Colour Accent2 = new(0x2A, 0x6E, 0x3F);

    private static DrawingTheme Theme() => DrawingTheme.Read(XElement.Parse(
        $"""
         <a:theme xmlns:a="{A}"><a:themeElements>
           <a:clrScheme name="FAAAI">
             <a:dk1><a:sysClr val="windowText" lastClr="000000"/></a:dk1>
             <a:lt1><a:sysClr val="window" lastClr="FFFFFF"/></a:lt1>
             <a:dk2><a:srgbClr val="1F1F1F"/></a:dk2>
             <a:lt2><a:srgbClr val="EEEEEE"/></a:lt2>
             <a:accent1><a:srgbClr val="850F89"/></a:accent1>
             <a:accent2><a:srgbClr val="2A6E3F"/></a:accent2>
             <a:accent3><a:srgbClr val="1B587C"/></a:accent3>
             <a:accent4><a:srgbClr val="4E8542"/></a:accent4>
             <a:accent5><a:srgbClr val="604878"/></a:accent5>
             <a:accent6><a:srgbClr val="C19859"/></a:accent6>
             <a:hlink><a:srgbClr val="6B9F25"/></a:hlink>
             <a:folHlink><a:srgbClr val="B26B02"/></a:folHlink>
           </a:clrScheme>
         </a:themeElements></a:theme>
         """))!;

    /// <summary>A matrix whose subtle line style shades its placeholder, as most themes do.</summary>
    private static DrawingStyleMatrix Shading() => DrawingStyleMatrix.Read(XElement.Parse(
        $"""
         <a:theme xmlns:a="{A}"><a:themeElements><a:fmtScheme>
           <a:lnStyleLst>
             <a:ln w="9525"><a:solidFill><a:schemeClr val="phClr">
               <a:shade val="50000"/><a:satMod val="103000"/>
             </a:schemeClr></a:solidFill></a:ln>
             <a:ln w="20320"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln>
           </a:lnStyleLst>
         </a:fmtScheme></a:themeElements></a:theme>
         """))!;

    private static string Series(int index, string body) =>
        $"""
         <c:ser><c:idx val="{index}"/><c:order val="{index}"/>{body}
           <c:val><c:numRef><c:numCache><c:ptCount val="2"/>
             <c:pt idx="0"><c:v>1</c:v></c:pt><c:pt idx="1"><c:v>2</c:v></c:pt>
           </c:numCache></c:numRef></c:val>
         </c:ser>
         """;

    private static ChartPlot Read(string plotArea)
        => DrawingChartPlot.Read(
               XElement.Parse(
                   $"<c:chartSpace xmlns:c=\"{C}\" xmlns:a=\"{A}\">"
                   + $"<c:chart><c:plotArea>{plotArea}</c:plotArea></c:chart></c:chartSpace>"),
               Theme(),
               office2007: false,
               Shading())
           ?? throw new InvalidOperationException("the reader found nothing to draw");

    /// <summary>The FAAAI series, verbatim in structure: a suppressed line and a stated marker.</summary>
    private const string SuppressedLineWithStatedMarker =
        """
        <c:spPr><a:ln w="25400" cap="rnd"><a:noFill/><a:round/></a:ln><a:effectLst/></c:spPr>
        <c:marker><c:symbol val="circle"/><c:size val="5"/><c:spPr>
          <a:solidFill><a:schemeClr val="accent1"/></a:solidFill>
          <a:ln w="9525"><a:solidFill><a:schemeClr val="accent1"/></a:solidFill></a:ln>
        </c:spPr></c:marker>
        """;

    // ------------------------------------------------------------ the marker's own spPr

    [Fact]
    public void AMarkerIsPaintedInTheColourItStates()
    {
        ChartPlot plot = Read(
            $"<c:scatterChart><c:scatterStyle val=\"lineMarker\"/>"
            + $"{Series(0, SuppressedLineWithStatedMarker)}</c:scatterChart>");

        plot.Series[0].MarkerFill.ShouldBe(Accent1);
        plot.Series[0].MarkerLine.ShouldBe(Accent1);
    }

    [Fact]
    public void AMarkerStatingNothingCarriesNoColourOfItsOwn()
    {
        // Null is "the file says nothing", which is what keeps every unstated marker — and every
        // ODF chart, whose reader has no such element — drawn exactly as it was.
        ChartPlot plot = Read(
            $"<c:lineChart>{Series(0, "<c:marker><c:symbol val=\"circle\"/></c:marker>")}</c:lineChart>");

        plot.Series[0].Marker.ShouldBe(ChartMarker.Circle);
        plot.Series[0].MarkerFill.ShouldBeNull();
        plot.Series[0].MarkerLine.ShouldBeNull();
    }

    [Fact]
    public void AMarkerFilledWithAGradientTakesItsLineColourAndNotTheGradient()
    {
        // 8_P-Pavese_AIRBUS's markers, structurally: a three-stop gradient over accent 6 and an
        // a:ln stating the bare accent. convertMarker only ever reads maFillColor, which a
        // gradient does not set, so the reference draws the line colour. FillOf would have
        // returned the middle stop — right for a bar, wrong here.
        ChartPlot plot = Read(
            $"""
             <c:scatterChart><c:scatterStyle val="lineMarker"/>{Series(0,
             """
             <c:marker><c:symbol val="circle"/><c:spPr>
               <a:gradFill><a:gsLst>
                 <a:gs pos="0"><a:schemeClr val="accent3"/></a:gs>
                 <a:gs pos="50000"><a:schemeClr val="accent4"/></a:gs>
                 <a:gs pos="100000"><a:schemeClr val="accent5"/></a:gs>
               </a:gsLst></a:gradFill>
               <a:ln w="9525"><a:solidFill><a:schemeClr val="accent2"/></a:solidFill></a:ln>
             </c:spPr></c:marker>
             """)}</c:scatterChart>
             """);

        plot.Series[0].MarkerFill.ShouldBe(Accent2);
    }

    [Fact]
    public void AMarkerStatingOnlyANoFillHasNoColourRatherThanBlack()
    {
        // Its spPr exists but names no colour at all; the series' own is then the right answer,
        // and that is expressed by leaving the member null rather than by inventing one here.
        ChartPlot plot = Read(
            $"""
             <c:lineChart>{Series(0,
             "<c:marker><c:symbol val=\"circle\"/><c:spPr><a:ln><a:noFill/></a:ln></c:spPr></c:marker>")}
             </c:lineChart>
             """);

        plot.Series[0].MarkerFill.ShouldBeNull();
        plot.Series[0].MarkerLine.ShouldBeNull();
    }

    // ------------------------------------------------------------ a:noFill as a suppression

    [Fact]
    public void AStatedNoFillOnASeriesLineIsNotTheAutomaticColour()
    {
        // The defect, stated as an assertion: the file says this series has no line, and the
        // automatic table must not answer a question the file already answered.
        ChartPlot plot = Read(
            $"<c:lineChart>{Series(0, "<c:spPr><a:ln w=\"25400\"><a:noFill/></a:ln></c:spPr>")}</c:lineChart>");

        plot.Series[0].Line.ShouldBeNull();
        plot.Series[0].HasLine.ShouldBeFalse();
    }

    [Fact]
    public void AnAbsentLineStillTakesTheAutomaticColour()
    {
        // The other half, and the one that makes the first a distinction rather than a deletion.
        // Accent 1 through the shading matrix is 42, 07, 44 — half of 850F89, saturation nudged.
        ChartPlot plot = Read($"<c:lineChart>{Series(0, "")}</c:lineChart>");

        plot.Series[0].Line.ShouldNotBeNull();
        plot.Series[0].HasLine.ShouldBeTrue();
    }

    [Fact]
    public void AMarkerKeepsItsOwnColourWhenTheSeriesLineIsSuppressed()
    {
        // The coupling, in one test. Suppressing the line removes what the marker used to
        // inherit; the marker's own spPr is what replaces it. Fail this and one half of the round
        // has been reverted — whichever half, this is the test that says so.
        ChartPlot plot = Read(
            $"<c:scatterChart><c:scatterStyle val=\"lineMarker\"/>"
            + $"{Series(0, SuppressedLineWithStatedMarker)}</c:scatterChart>");

        plot.Series[0].Line.ShouldBeNull();
        plot.Series[0].MarkerFill.ShouldBe(Accent1);
    }

    [Fact]
    public void AStatedLineColourIsUnaffectedByEitherHalf()
    {
        // 1_Country-Updates_DRC_English.pptx: every one of its three stated markers names the
        // same accent its series' line names. It is in the census and it must not move.
        ChartPlot plot = Read(
            $"""
             <c:lineChart>{Series(0,
             """
             <c:spPr><a:ln w="28575"><a:solidFill><a:schemeClr val="accent1"/></a:solidFill></a:ln></c:spPr>
             <c:marker><c:symbol val="circle"/><c:spPr>
               <a:solidFill><a:schemeClr val="accent1"/></a:solidFill>
             </c:spPr></c:marker>
             """)}</c:lineChart>
             """);

        plot.Series[0].Line.ShouldBe(Accent1);
        plot.Series[0].MarkerFill.ShouldBe(Accent1);
    }
}
