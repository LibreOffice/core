using System.Xml.Linq;
using XRay.Office.Core.Charts;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Ooxml.DrawingML;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// A chart's furniture: where an axis puts its tick marks, and what its lines are painted in
/// when the file states nothing.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Both laws were measured on 26.2.4.2 before they were written, one property at a time
/// in a real corpus deck.</strong> The tick probe patches <c>c:majorTickMark</c> in a chart that
/// already states <c>none</c> on both axes and reads the plot rectangle off the gridlines:
/// <c>none</c> and <c>in</c> move it by 0.00, <c>out</c> and <c>cross</c> by 4.25 pt —
/// <c>AXIS2D_TICKLENGTH</c> exactly — and on that axis' own edge only.
/// </para>
/// <para>
/// The colour probe patches the theme instead. With <c>tx1</c> black the reference draws a major
/// gridline <c>#666666</c> and a minor one <c>#8B8B8B</c>; with <c>tx1</c> <c>#FFFFFF</c> it
/// draws <c>#BCBCBC</c> for <em>both</em>, which is what says the tint is not the whole story —
/// a tint of white is white, and only the theme's own <c>shade 50000</c> on top of the
/// substituted <c>phClr</c> can collapse two tints onto one value.
/// </para>
/// </remarks>
public class DrawingChartFurnitureTests
{
    private const string C = "http://schemas.openxmlformats.org/drawingml/2006/chart";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    /// <summary>A theme whose <c>tx1</c> is a stated colour and whose subtle line is 9525 EMU.</summary>
    /// <remarks>
    /// The subtle line carries <c>shade 50000</c> and <c>satMod 103000</c> around its
    /// <c>phClr</c>, which is what every theme Office ships states there and what the measured
    /// numbers depend on.
    /// </remarks>
    private static string ThemeXml(string tx1, string width = "9525")
        => $"""
            <a:theme xmlns:a="{A}" name="t"><a:themeElements>
              <a:clrScheme name="c">
                <a:dk1><a:srgbClr val="{tx1}"/></a:dk1><a:lt1><a:srgbClr val="FFFFFF"/></a:lt1>
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
                  <a:ln w="{width}" cap="flat" cmpd="sng" algn="ctr">
                    <a:solidFill><a:schemeClr val="phClr"><a:shade val="50000"/><a:satMod val="103000"/></a:schemeClr></a:solidFill>
                    <a:prstDash val="solid"/>
                  </a:ln>
                  <a:ln w="25400"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln>
                  <a:ln w="38100"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln>
                </a:lnStyleLst>
                <a:effectStyleLst><a:effectStyle><a:effectLst/></a:effectStyle></a:effectStyleLst>
                <a:bgFillStyleLst><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:bgFillStyleLst>
              </a:fmtScheme>
            </a:themeElements></a:theme>
            """;

    private static ChartPlot Read(string plotArea, string? theme = null)
    {
        XElement? themed = theme is null ? null : XElement.Parse(theme);

        return DrawingChartPlot.Read(
                   XElement.Parse(
                       $"<c:chartSpace xmlns:c=\"{C}\" xmlns:a=\"{A}\"><c:chart>{plotArea}</c:chart></c:chartSpace>"),
                   DrawingTheme.Read(themed),
                   office2007: false,
                   DrawingStyleMatrix.Read(themed))
               ?? throw new InvalidOperationException("the reader found nothing to draw");
    }

    private static string Bars(string valueAxis, string categoryAxis = "")
        => $"""
            <c:plotArea><c:barChart><c:ser><c:val><c:numRef><c:numCache>
              <c:ptCount val="2"/><c:pt idx="0"><c:v>1</c:v></c:pt><c:pt idx="1"><c:v>2</c:v></c:pt>
            </c:numCache></c:numRef></c:val></c:ser></c:barChart>
            <c:valAx><c:axId val="1"/>{valueAxis}<c:crossAx val="2"/></c:valAx>
            <c:catAx><c:axId val="2"/>{categoryAxis}<c:crossAx val="1"/></c:catAx>
            </c:plotArea>
            """;

    // ------------------------------------------------------------------ tick marks

    [Theory]
    [InlineData("out", ChartTickMark.Outer)]
    [InlineData("cross", ChartTickMark.Cross)]
    [InlineData("in", ChartTickMark.Inner)]
    [InlineData("none", ChartTickMark.None)]
    public void AStatedTickMarkIsRead(string stated, ChartTickMark expected)
    {
        ChartPlot plot = Read(Bars(
            $"<c:majorTickMark val=\"{stated}\"/>",
            $"<c:majorTickMark val=\"{stated}\"/>"));

        plot.ValueTicks.ShouldBe(expected);
        plot.CategoryTicks.ShouldBe(expected);
    }

    /// <summary>
    /// The control, and it is the half a census cannot see.
    /// </summary>
    /// <remarks>
    /// <c>AxisModel</c>'s constructor defaults <c>c:majorTickMark</c> to <c>out</c> for a 2007
    /// chart part and to <c>cross</c> for a later one
    /// (<c>oox/source/drawingml/chart/axismodel.cxx:42-48</c>) — both of which reserve the tick
    /// length. Reading an absent element as <c>none</c> would take 4.25 pt off the plot area of
    /// every chart that states nothing, which is 13 of the corpus' 494 axes.
    /// </remarks>
    [Fact]
    public void AnAbsentTickMarkIsOuterAndNotNone()
    {
        ChartPlot plot = Read(Bars(""));

        plot.ValueTicks.ShouldBe(ChartTickMark.Outer);
        plot.CategoryTicks.ShouldBe(ChartTickMark.Outer);
        plot.SecondaryTicks.ShouldBe(ChartTickMark.Outer);
    }

    // ------------------------------------------------------------------ automatic lines

    /// <summary>
    /// The measured colours, on a black <c>tx1</c>: the numbers 26.2.4.2 draws.
    /// </summary>
    [Fact]
    public void AnUnstatedGridAndAxisLineTakeTheThemesSubtleLineStyle()
    {
        ChartPlot plot = Read(
            Bars("<c:majorGridlines/><c:minorGridlines/>"),
            ThemeXml("000000"));

        plot.ValueGrid.ShouldNotBeNull().Colour.ShouldBe(Colour.FromRgb(0x666666));
        plot.ValueMinorGrid.ShouldNotBeNull().Colour.ShouldBe(Colour.FromRgb(0x8B8B8B));
        plot.ValueAxisLine.Colour.ShouldBe(Colour.FromRgb(0x666666));
        plot.CategoryAxisLine.Colour.ShouldBe(Colour.FromRgb(0x666666));
    }

    /// <summary>
    /// The discriminator: a tint of white is white, so two tints can only give one colour if the
    /// theme's own <c>shade</c> is applied after them.
    /// </summary>
    [Fact]
    public void AWhiteTextColourCollapsesBothTintsOntoOneShade()
    {
        ChartPlot plot = Read(
            Bars("<c:majorGridlines/><c:minorGridlines/>"),
            ThemeXml("FFFFFF"));

        plot.ValueGrid.ShouldNotBeNull().Colour.ShouldBe(Colour.FromRgb(0xBCBCBC));
        plot.ValueMinorGrid.ShouldNotBeNull().Colour.ShouldBe(Colour.FromRgb(0xBCBCBC));
    }

    /// <summary>The width is the theme's, not a constant.</summary>
    [Fact]
    public void AnUnstatedGridTakesTheThemesSubtleLineWidth()
    {
        ChartPlot narrow = Read(Bars("<c:majorGridlines/>"), ThemeXml("000000"));
        ChartPlot wide = Read(Bars("<c:majorGridlines/>"), ThemeXml("000000", width: "38100"));

        // `convertEmuToHmm` rounds a DrawingML width to a whole hundredth of a millimetre, so the
        // reference never holds the EMU the file states. `DrawingChartAutoFormat.LineWidth` has
        // the two legs; `probes/stroke-resid-r117/results.md` §3 has the measurements.
        // 9525 EMU is 26 hundredths of a millimetre and 38100 is 106. 26.2.4.2 draws the first
        // at 0.7357 pt on `sheet-chart-auto-line.xlsx`, whose theme states 9525.
        narrow.ValueGrid.ShouldNotBeNull().Width.ShouldBe(Length.FromMm100(26));
        wide.ValueGrid.ShouldNotBeNull().Width.ShouldBe(Length.FromMm100(106));
        wide.ValueAxisLine.Width.ShouldBe(Length.FromMm100(106));
    }

    /// <summary>
    /// A stated colour wins over the automatic entry, and a stated <em>width alone</em> does not
    /// take the colour with it.
    /// </summary>
    /// <remarks>
    /// <c>LineFormatter::convertFormatting</c> is <c>assignUsed</c> twice, so each property the
    /// shape states wins separately. <c>Demick_JetBlue.pptx</c>'s value axis states
    /// <c>&lt;a:ln w="9525"/&gt;</c> and nothing else, and the reference draws it in the
    /// automatic <c>#666666</c> — reading "states an <c>a:ln</c>" as "states everything" is what
    /// drew it black.
    /// </remarks>
    [Fact]
    public void AStatedColourWinsAndAStatedWidthAloneDoesNot()
    {
        ChartPlot plot = Read(
            Bars("""
                 <c:majorGridlines><c:spPr><a:ln><a:solidFill><a:srgbClr val="FF0000"/></a:solidFill></a:ln></c:spPr></c:majorGridlines>
                 <c:spPr><a:ln w="19050"/></c:spPr>
                 """),
            ThemeXml("000000"));

        plot.ValueGrid.ShouldNotBeNull().Colour.ShouldBe(Colour.FromRgb(0xFF0000));
        plot.ValueAxisLine.Colour.ShouldBe(Colour.FromRgb(0x666666));
        // `convertEmuToHmm` rounds a DrawingML width to a whole hundredth of a millimetre, so the
        // reference never holds the EMU the file states. `DrawingChartAutoFormat.LineWidth` has
        // the two legs; `probes/stroke-resid-r117/results.md` §3 has the measurements.
        plot.ValueAxisLine.Width.ShouldBe(Length.FromMm100(53));
    }

    /// <summary>
    /// With no theme to ask, the reader falls back rather than inventing a colour.
    /// </summary>
    [Fact]
    public void WithNoThemeTheGridKeepsChart2sOwnGrey()
    {
        ChartPlot plot = Read(Bars("<c:majorGridlines/>"));

        plot.ValueGrid.ShouldNotBeNull().Colour.ShouldBe(Colour.FromRgb(0xB3B3B3));
    }
}
