using System.Xml.Linq;
using XRay.Office.Core.Charts;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Ooxml.DrawingML;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// The sizes and weights an OOXML chart's text takes when the part states none.
/// </summary>
/// <remarks>
/// <para>
/// <strong>An OOXML chart never reaches <c>chart2</c>'s model defaults.</strong> The import
/// applies <c>oox/source/drawingml/chart/objectformatter.cxx</c>'s auto-text table first
/// (<c>:415-434</c>, applied by <c>TextFormatter::TextFormatter</c>, <c>:906-929</c>): a chart
/// title is <c>1800</c> and bold, an axis title <c>1000</c> and bold, and everything else —
/// axis labels, legend entries, data labels — <c>1000</c> and not bold.
/// </para>
/// <para>
/// The values are checked against LibreOffice's own model and not only against its ink.
/// <c>Demick_JetBlue.pptx</c>, whose five chart parts state no <c>sz</c> and no <c>b</c>
/// anywhere, converts to <c>.odp</c> with <c>fo:font-size="18pt" fo:font-weight="bold"</c> on
/// the chart title, <c>10pt</c>/<c>bold</c> on its two axis titles, and <c>10pt</c> with no
/// weight at all on its axes and its legend.
/// </para>
/// </remarks>
public class DrawingChartAutoTextTests
{
    private const string C = "http://schemas.openxmlformats.org/drawingml/2006/chart";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    /// <summary>Reads a chart space whose <c>c:chart</c> holds <paramref name="inner"/>.</summary>
    /// <param name="inner">The markup inside <c>c:chart</c>.</param>
    /// <param name="space">Markup that follows <c>c:chart</c> inside <c>c:chartSpace</c>.</param>
    private static ChartPlot Read(string inner, string space = "")
        => DrawingChartPlot.Read(XElement.Parse(
               $"<c:chartSpace xmlns:c=\"{C}\" xmlns:a=\"{A}\">"
               + $"<c:chart>{inner}</c:chart>{space}</c:chartSpace>"))
           ?? throw new InvalidOperationException("the reader found nothing to draw");

    /// <summary>A one-series bar chart with the given titles.</summary>
    private static string Bar(string title = "", string axisTitle = "") =>
        $"""
         {title}
         <c:plotArea><c:barChart>
           <c:ser>
             <c:val><c:numRef><c:numCache>
               <c:ptCount val="2"/>
               <c:pt idx="0"><c:v>20000</c:v></c:pt><c:pt idx="1"><c:v>40000</c:v></c:pt>
             </c:numCache></c:numRef></c:val>
           </c:ser>
           <c:axId val="1"/><c:axId val="2"/>
         </c:barChart>
         <c:valAx><c:axId val="2"/>{axisTitle}</c:valAx>
         </c:plotArea>
         """;

    private static string Title(string text, string properties = "") =>
        $"""
         <c:title><c:tx><c:rich><a:p><a:pPr><a:defRPr {properties}/></a:pPr>
           <a:r><a:t>{text}</a:t></a:r></a:p></c:rich></c:tx></c:title>
         """;

    /// <summary>A title stating nothing is 18 pt, not <c>chart2</c>'s 13.</summary>
    [Fact]
    public void AnUnstatedTitleIsEighteenPoints()
        => Read(Bar(Title("Sales"))).TitleSize.ShouldBe(Length.FromPoints(18));

    /// <summary>An axis title stating nothing is 10 pt, not <c>chart2</c>'s 9.</summary>
    [Fact]
    public void AnUnstatedAxisTitleIsTenPoints()
        => Read(Bar(axisTitle: Title("Year"))).AxisTitleSize.ShouldBe(Length.FromPoints(10));

    /// <summary>Both titles are bold when the part states no weight.</summary>
    [Fact]
    public void UnstatedTitlesAreBold()
    {
        ChartPlot plot = Read(Bar(Title("Sales"), Title("Year")));

        plot.IsTitleBold.ShouldBeTrue();
        plot.IsAxisTitleBold.ShouldBeTrue();
    }

    /// <summary>
    /// A stated <c>b="0"</c> is regular, which is why the reader distinguishes "states nothing"
    /// from "states false".
    /// </summary>
    /// <remarks>
    /// Collapsing the two would draw <c>b="0"</c> bold, and five of the slides corpus's 61 chart
    /// parts state exactly that on a title.
    /// </remarks>
    [Fact]
    public void AStatedRegularWeightIsHonoured()
    {
        ChartPlot plot = Read(Bar(Title("Sales", "b=\"0\""), Title("Year", "b=\"0\"")));

        plot.IsTitleBold.ShouldBeFalse();
        plot.IsAxisTitleBold.ShouldBeFalse();
    }

    /// <summary>A stated size still wins over the table.</summary>
    [Fact]
    public void AStatedSizeIsHonoured()
    {
        ChartPlot plot = Read(Bar(Title("Sales", "sz=\"2400\""), Title("Year", "sz=\"1200\"")));

        plot.TitleSize.ShouldBe(Length.FromPoints(24));
        plot.AxisTitleSize.ShouldBe(Length.FromPoints(12));
    }

    /// <summary>
    /// The chart space's own <c>c:txPr</c> size replaces the table's absolute default, scaled by
    /// the entry's <c>mnRelFontSize</c> — 120% for the main title and 100% for everything else.
    /// </summary>
    /// <remarks>
    /// <c>objectformatter.cxx:926-928</c>: the absolute default holds until the global text
    /// properties supply a height, and then that height times the percentage is taken instead.
    /// Six of the slides corpus's 61 chart parts state one.
    /// </remarks>
    [Fact]
    public void AGlobalSizeScalesTheTableRatherThanReplacingIt()
    {
        const string Global =
            "<c:txPr><a:p><a:pPr><a:defRPr sz=\"1400\"/></a:pPr></a:p></c:txPr>";

        ChartPlot plot = Read(Bar(Title("Sales"), Title("Year")), Global);

        plot.TitleSize.ShouldBe(Length.FromPoints(16.8));
        plot.AxisTitleSize.ShouldBe(Length.FromPoints(14));
        plot.LabelSize.ShouldBe(Length.FromPoints(14));
    }

    /// <summary>Axis labels state their weight on the axis' own <c>c:txPr</c>.</summary>
    /// <remarks>
    /// Seen on <c>171128IPAP.pptx</c>'s <c>chart4.xml</c>, which puts
    /// <c>&lt;a:defRPr sz="900" b="1"/&gt;</c> on both axes; the reference draws those labels in
    /// Carlito-Bold. 36 of the slides corpus's 61 chart parts state a weight somewhere.
    /// </remarks>
    [Fact]
    public void AStatedAxisLabelWeightIsRead()
    {
        const string Bold = "<c:txPr><a:p><a:pPr><a:defRPr b=\"1\"/></a:pPr></a:p></c:txPr>";

        ChartPlot plot = Read(
            Bar().Replace("<c:valAx><c:axId val=\"2\"/>", "<c:valAx><c:axId val=\"2\"/>" + Bold,
                          StringComparison.Ordinal));

        plot.IsLabelBold.ShouldBeTrue();
    }

    /// <summary>Unlike the titles, an unstated axis-label weight is regular.</summary>
    /// <remarks>
    /// The auto-text table leaves <c>spOtherTexts</c> clear, so this is the one weight on which
    /// the OOXML default and <c>chart2</c>'s model default agree.
    /// </remarks>
    [Fact]
    public void AnUnstatedAxisLabelWeightIsRegular()
        => Read(Bar(Title("Sales"), Title("Year"))).IsLabelBold.ShouldBeFalse();

    /// <summary>
    /// A weight on an axis <em>title</em> says nothing about that axis' labels.
    /// </summary>
    /// <remarks>
    /// The two live in different elements — <c>c:catAx/c:title/…/a:defRPr</c> against
    /// <c>c:catAx/c:txPr/…/a:defRPr</c> — and reading the axis' descendants rather than its own
    /// <c>c:txPr</c> would make every bold axis title bold every label under it.
    /// </remarks>
    [Fact]
    public void AnAxisTitlesWeightDoesNotReachItsLabels()
        => Read(Bar(axisTitle: Title("Year", "b=\"1\""))).IsLabelBold.ShouldBeFalse();

    /// <summary>The legend states its own weight, and leaves it unset when it does not.</summary>
    /// <remarks>
    /// Null rather than false so an unstated legend follows the axis labels — see
    /// <c>ChartPlot.IsLegendBold</c>, which is the same reason <c>LegendSize</c> is nullable.
    /// </remarks>
    [Fact]
    public void TheLegendStatesItsOwnWeight()
    {
        const string Legend =
            "<c:legend><c:legendPos val=\"r\"/>"
            + "<c:txPr><a:p><a:pPr><a:defRPr b=\"1\"/></a:pPr></a:p></c:txPr></c:legend>";

        Read(Bar() + Legend).IsLegendBold.ShouldBe(true);
        Read(Bar()).IsLegendBold.ShouldBeNull();
    }

    /// <summary>A theme whose <c>dk1</c> is not black and whose <c>lt1</c> is not white.</summary>
    /// <remarks>
    /// Both are off the obvious values so that a fallback to <c>Colour.Black</c> or to
    /// <c>Colour.White</c> cannot pass by coincidence.
    /// </remarks>
    private static DrawingTheme Theme() => DrawingTheme.Read(XElement.Parse(
        $"""
         <a:theme xmlns:a="{A}"><a:themeElements><a:clrScheme name="t">
           <a:dk1><a:srgbClr val="3C3D36"/></a:dk1>
           <a:lt1><a:srgbClr val="FDFCF7"/></a:lt1>
           <a:dk2><a:srgbClr val="323232"/></a:dk2>
           <a:lt2><a:srgbClr val="E3DED1"/></a:lt2>
           <a:accent1><a:srgbClr val="F07F09"/></a:accent1>
           <a:accent2><a:srgbClr val="9F2936"/></a:accent2>
           <a:accent3><a:srgbClr val="1B587C"/></a:accent3>
           <a:accent4><a:srgbClr val="4E8542"/></a:accent4>
           <a:accent5><a:srgbClr val="604878"/></a:accent5>
           <a:accent6><a:srgbClr val="C19859"/></a:accent6>
           <a:hlink><a:srgbClr val="6B9F25"/></a:hlink>
           <a:folHlink><a:srgbClr val="B26B02"/></a:folHlink>
         </a:clrScheme></a:themeElements></a:theme>
         """))!;

    private static ChartPlot ReadThemed(string inner, string space = "")
        => DrawingChartPlot.Read(
               XElement.Parse(
                   $"<c:chartSpace xmlns:c=\"{C}\" xmlns:a=\"{A}\">"
                   + $"<c:chart>{inner}</c:chart>{space}</c:chartSpace>"),
               Theme())
           ?? throw new InvalidOperationException("the reader found nothing to draw");

    /// <summary>
    /// Below style 41 a chart's unstated text is the theme's <c>tx1</c>, not black.
    /// </summary>
    /// <remarks>
    /// The colour half of the same three tables the sizes above come from —
    /// <c>spChartTitleTexts</c>, <c>spAxisTitleTexts</c> and <c>spOtherTexts</c>
    /// (<c>objectformatter.cxx:415-434</c>) all carry <c>tx1</c> for styles 1 to 40. It is black
    /// on all but seven of the corpus' chart-bearing files, which is why a hardcoded black
    /// survived so long.
    /// </remarks>
    [Fact]
    public void UnstatedTextBelowStyleFortyOneIsTheThemesDarkColour()
    {
        ChartPlot plot = ReadThemed(Bar(Title("Sales"), Title("Year")));

        plot.TitleColour.ShouldBe(new Colour(0x3C, 0x3D, 0x36));
        plot.AxisTitleColour.ShouldBe(new Colour(0x3C, 0x3D, 0x36));
        plot.LabelColour.ShouldBe(new Colour(0x3C, 0x3D, 0x36));
        plot.LegendColour.ShouldBe(new Colour(0x3C, 0x3D, 0x36));
        plot.DataLabelColour.ShouldBe(new Colour(0x3C, 0x3D, 0x36));
    }

    /// <summary>
    /// From style 41 up it is <c>lt1</c>, which is what makes a dark chart's text visible.
    /// </summary>
    /// <remarks>
    /// The second row of each table. Style 41 and up also paints the chart space <c>dk1</c>
    /// (<c>DrawingChartAutoFormat.FrameFillOf</c>), so taking the first row here draws every
    /// title, tick label and legend entry in dark text on a dark ground — measured on
    /// <c>DynamicBubbleChart.xlsx</c>, style 42, whose seven pieces of chart text were all in
    /// our PDF's text layer and none of them visible.
    /// </remarks>
    [Fact]
    public void UnstatedTextFromStyleFortyOneUpIsTheThemesLightColour()
    {
        ChartPlot plot = ReadThemed(
            Bar(Title("Sales"), Title("Year")), "<c:style val=\"42\"/>");

        plot.TitleColour.ShouldBe(new Colour(0xFD, 0xFC, 0xF7));
        plot.AxisTitleColour.ShouldBe(new Colour(0xFD, 0xFC, 0xF7));
        plot.LabelColour.ShouldBe(new Colour(0xFD, 0xFC, 0xF7));
        plot.LegendColour.ShouldBe(new Colour(0xFD, 0xFC, 0xF7));
    }

    /// <summary>An element that states its own colour keeps it whatever the style says.</summary>
    /// <remarks>
    /// The automatic colour is the placeholder the element's own <c>a:defRPr</c> overrides
    /// (<c>TextFormatter::TextFormatter</c>, <c>:906-928</c>), not a colour applied over it.
    /// </remarks>
    [Fact]
    public void AStatedColourWinsOverTheStylesOwn()
    {
        const string Red =
            "<c:title><c:tx><c:rich><a:p><a:pPr><a:defRPr>"
            + "<a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill>"
            + "</a:defRPr></a:pPr><a:r><a:t>Sales</a:t></a:r></a:p></c:rich></c:tx></c:title>";

        ChartPlot plot = ReadThemed(Bar(Red), "<c:style val=\"42\"/>");

        plot.TitleColour.ShouldBe(new Colour(0xFF, 0x00, 0x00));
    }
}
