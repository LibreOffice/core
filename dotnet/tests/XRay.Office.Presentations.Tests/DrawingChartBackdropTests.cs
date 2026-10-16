using System.Xml.Linq;
using XRay.Office.Core.Charts;
using XRay.Office.Core.Graphics;
using XRay.Office.Ooxml.DrawingML;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// What a chart paints behind itself, and what colour it draws its text in, when the file says
/// nothing about either.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Both were found by instrumenting a page reading rather than by a metric.</strong> A
/// reviewer given only the composed image of <c>8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx</c>
/// page 8 reported that the reference draws a black chart background, a grey plot wall and a
/// white title where we drew none of the three. A fill census of both PDFs then measured it:
/// the reference paints <c>#000000</c> over 720 x 391 pt and <c>#454545</c> over 640 x 292 pt
/// and this reader painted neither, and a text-colour census counted fourteen white runs against
/// our twenty-two black ones. That page was 43.67 of the track's unsigned ink on its own.
/// </para>
/// <para>
/// The numbers below are <c>ObjectFormatter</c>'s two fill tables
/// (<c>objectformatter.cxx:174-197</c>) and the <c>#454545</c> is the one the reference actually
/// drew — <c>dk1</c> under <c>tint 95000</c>, to the byte.
/// </para>
/// </remarks>
public class DrawingChartBackdropTests
{
    private const string C = "http://schemas.openxmlformats.org/drawingml/2006/chart";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    private static string ThemeXml()
        => $"""
            <a:theme xmlns:a="{A}" name="t"><a:themeElements>
              <a:clrScheme name="c">
                <a:dk1><a:srgbClr val="000000"/></a:dk1><a:lt1><a:srgbClr val="FFFFFF"/></a:lt1>
                <a:dk2><a:srgbClr val="44546A"/></a:dk2><a:lt2><a:srgbClr val="E7E6E6"/></a:lt2>
                <a:accent1><a:srgbClr val="4472C4"/></a:accent1>
                <a:accent2><a:srgbClr val="ED7D31"/></a:accent2>
                <a:accent3><a:srgbClr val="A5A5A5"/></a:accent3>
                <a:accent4><a:srgbClr val="FFC000"/></a:accent4>
                <a:accent5><a:srgbClr val="5B9BD5"/></a:accent5>
                <a:accent6><a:srgbClr val="70AD47"/></a:accent6>
                <a:hlink><a:srgbClr val="0563C1"/></a:hlink>
                <a:folHlink><a:srgbClr val="954F72"/></a:folHlink>
              </a:clrScheme>
              <a:fontScheme name="f">
                <a:majorFont><a:latin typeface="Calibri"/><a:ea typeface=""/><a:cs typeface=""/></a:majorFont>
                <a:minorFont><a:latin typeface="Calibri"/><a:ea typeface=""/><a:cs typeface=""/></a:minorFont>
              </a:fontScheme>
              <a:fmtScheme name="s">
                <a:fillStyleLst><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:fillStyleLst>
                <a:lnStyleLst>
                  <a:ln w="9525"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln>
                  <a:ln w="25400"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln>
                  <a:ln w="38100"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln>
                </a:lnStyleLst>
                <a:effectStyleLst><a:effectStyle><a:effectLst/></a:effectStyle></a:effectStyleLst>
                <a:bgFillStyleLst><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:bgFillStyleLst>
              </a:fmtScheme>
            </a:themeElements></a:theme>
            """;

    /// <param name="style">The chart's <c>c:style/@val</c>, or null to state none.</param>
    /// <param name="spacePr">A <c>c:spPr</c> for the chart space, or "".</param>
    /// <param name="plotPr">A <c>c:spPr</c> for the plot area, or "".</param>
    /// <param name="axisText">A <c>c:txPr</c> for both axes, or "".</param>
    /// <param name="titleText">The chart title's own markup, or "".</param>
    /// <param name="labelText">A plot-area <c>c:dLbls</c>, or "".</param>
    /// <param name="legendText">A <c>c:legend</c>, or "".</param>
    private static ChartPlot Read(
        int? style = null,
        string spacePr = "",
        string plotPr = "",
        string axisText = "",
        string titleText = "",
        string labelText = "",
        string legendText = "")
    {
        XElement theme = XElement.Parse(ThemeXml());
        string stated = style is null ? "" : $"<c:style val=\"{style}\"/>";

        string xml = $"""
            <c:chartSpace xmlns:c="{C}" xmlns:a="{A}">
            {stated}
            <c:chart>{titleText}
            <c:plotArea>{labelText}<c:barChart><c:ser><c:val><c:numRef><c:numCache>
              <c:ptCount val="2"/><c:pt idx="0"><c:v>1</c:v></c:pt><c:pt idx="1"><c:v>2</c:v></c:pt>
            </c:numCache></c:numRef></c:val></c:ser></c:barChart>
            <c:catAx><c:axId val="2"/><c:crossAx val="1"/>{axisText}</c:catAx>
            <c:valAx><c:axId val="1"/><c:crossAx val="2"/>{axisText}</c:valAx>
            {plotPr}</c:plotArea>{legendText}</c:chart>
            {spacePr}</c:chartSpace>
            """;

        return DrawingChartPlot.Read(
                   XElement.Parse(xml), DrawingTheme.Read(theme), office2007: false,
                   DrawingStyleMatrix.Read(theme))
               ?? throw new InvalidOperationException("the reader found nothing to draw");
    }

    private static string Text(string element, string colour)
        => $"""
            <c:{element}><c:txPr><a:bodyPr/><a:lstStyle/><a:p><a:pPr><a:defRPr>
              <a:solidFill><a:srgbClr val="{colour}"/></a:solidFill>
            </a:defRPr></a:pPr></a:p></c:txPr></c:{element}>
            """;

    // ------------------------------------------------------------------ the backdrops

    /// <summary>
    /// A pptx chart below style 33 paints neither background, and that is a quirk rather than a
    /// table row.
    /// </summary>
    /// <remarks>
    /// <c>ObjectTypeFormatter</c>'s constructor overrides the fill style for exactly these two
    /// object types when the style is 32 or less, and <c>PptGraphicHelper</c> answers
    /// <c>XML_noFill</c> (<c>objectformatter.cxx:956-959</c>,
    /// <c>oox/source/ppt/pptimport.cxx:309-312</c>). 160 of the corpus' 163 slides chart parts
    /// are style 2, so this case is almost the whole corpus and it must stay empty.
    /// </remarks>
    [Theory]
    [InlineData(null)]
    [InlineData(1)]
    [InlineData(2)]
    [InlineData(32)]
    public void BelowStyleThirtyThreeAChartPaintsNoBackdrop(int? style)
    {
        ChartPlot plot = Read(style);

        plot.Background.ShouldBeNull();
        plot.PlotBackground.ShouldBeNull();
    }

    /// <summary>Style 42's two backdrops, which are the two the reference drew on Pavese p8.</summary>
    [Fact]
    public void StyleFortyTwoPaintsTheDarkBackdropAndTheTintedWall()
    {
        ChartPlot plot = Read(42);

        plot.Background.ShouldBe(Colour.FromRgb(0x000000));
        plot.PlotBackground.ShouldBe(Colour.FromRgb(0x454545));
    }

    /// <summary>
    /// The middle band of both tables, which no corpus document reaches and which is therefore a
    /// drift guard rather than a measurement.
    /// </summary>
    [Fact]
    public void StyleThirtyThreePaintsTheLightBackdropAndADarkerWall()
    {
        ChartPlot plot = Read(33);

        plot.Background.ShouldBe(Colour.FromRgb(0xFFFFFF));
        plot.PlotBackground.ShouldNotBeNull().ShouldNotBe(Colour.FromRgb(0xFFFFFF));
    }

    /// <summary>A stated <c>c:spPr</c> beats the automatic table, on both.</summary>
    [Fact]
    public void AStatedFillWins()
    {
        ChartPlot plot = Read(
            42,
            spacePr: "<c:spPr><a:solidFill><a:srgbClr val=\"112233\"/></a:solidFill></c:spPr>",
            plotPr: "<c:spPr><a:solidFill><a:srgbClr val=\"445566\"/></a:solidFill></c:spPr>");

        plot.Background.ShouldBe(Colour.FromRgb(0x112233));
        plot.PlotBackground.ShouldBe(Colour.FromRgb(0x445566));
    }

    // ------------------------------------------------------------------ the five text colours

    /// <summary>
    /// The control, and it is the one that keeps every other format where it was: a chart that
    /// states no colour anywhere draws all five in black, which is what
    /// <c>ChartLayout.AxisColour</c> alone used to give.
    /// </summary>
    [Fact]
    public void AChartThatStatesNoColourDrawsEverythingBlack()
    {
        ChartPlot plot = Read(2);

        plot.LabelColour.ShouldBe(Colour.Black);
        plot.TitleColour.ShouldBe(Colour.Black);
        plot.AxisTitleColour.ShouldBe(Colour.Black);
        plot.DataLabelColour.ShouldBe(Colour.Black);
        plot.LegendColour.ShouldBe(Colour.Black);
    }

    /// <summary>The axes' own <c>c:txPr</c> reaches the tick labels.</summary>
    [Fact]
    public void TheAxesTextPropertiesGiveTheLabelColour()
        => Read(2, axisText: "<c:txPr><a:bodyPr/><a:lstStyle/><a:p><a:pPr><a:defRPr>"
                             + "<a:solidFill><a:srgbClr val=\"FF0000\"/></a:solidFill>"
                             + "</a:defRPr></a:pPr></a:p></c:txPr>")
            .LabelColour.ShouldBe(Colour.FromRgb(0xFF0000));

    /// <summary>
    /// A scheme name resolves through the theme, which is how a chart on a dark master gets its
    /// white text: Pavese states <c>a:schemeClr val="bg1"</c> and nothing else.
    /// </summary>
    [Fact]
    public void ASchemeNameResolvesThroughTheTheme()
        => Read(42, axisText: "<c:txPr><a:bodyPr/><a:lstStyle/><a:p><a:pPr><a:defRPr>"
                              + "<a:solidFill><a:schemeClr val=\"bg1\"/></a:solidFill>"
                              + "</a:defRPr></a:pPr></a:p></c:txPr>")
            .LabelColour.ShouldBe(Colour.FromRgb(0xFFFFFF));

    /// <summary>The title's own runs, which are an <c>a:rPr</c> and not a <c>c:txPr</c>.</summary>
    [Fact]
    public void TheTitlesOwnRunsGiveTheTitleColour()
    {
        ChartPlot plot = Read(2, titleText: """
            <c:title><c:tx><c:rich><a:bodyPr/><a:lstStyle/><a:p><a:r>
              <a:rPr lang="en-GB"><a:solidFill><a:srgbClr val="00FF00"/></a:solidFill></a:rPr>
              <a:t>Taux</a:t>
            </a:r></a:p></c:rich></c:tx></c:title>
            """);

        plot.TitleColour.ShouldBe(Colour.FromRgb(0x00FF00));
    }

    /// <summary>The plot area's own <c>c:dLbls</c>, and the legend's <c>c:txPr</c>.</summary>
    [Fact]
    public void TheDataLabelsAndTheLegendStateTheirOwn()
    {
        ChartPlot plot = Read(
            2, labelText: Text("dLbls", "0000FF"), legendText: Text("legend", "FF00FF"));

        plot.DataLabelColour.ShouldBe(Colour.FromRgb(0x0000FF));
        plot.LegendColour.ShouldBe(Colour.FromRgb(0xFF00FF));
    }

    /// <summary>
    /// The fall-backs: data labels and the legend take the axes' colour when they state none, and
    /// an axis title takes the chart title's.
    /// </summary>
    /// <remarks>
    /// Which is the whole of what makes Pavese right — its <c>c:dLbls</c> states no colour and
    /// its labels are white because its axes are.
    /// </remarks>
    [Fact]
    public void TheUnstatedObjectsFallBackRatherThanToBlack()
    {
        ChartPlot plot = Read(
            42,
            axisText: "<c:txPr><a:bodyPr/><a:lstStyle/><a:p><a:pPr><a:defRPr>"
                      + "<a:solidFill><a:schemeClr val=\"bg1\"/></a:solidFill>"
                      + "</a:defRPr></a:pPr></a:p></c:txPr>");

        plot.LabelColour.ShouldBe(Colour.FromRgb(0xFFFFFF));
        plot.DataLabelColour.ShouldBe(Colour.FromRgb(0xFFFFFF));
        plot.LegendColour.ShouldBe(Colour.FromRgb(0xFFFFFF));
    }
}
