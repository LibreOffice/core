using System.Xml.Linq;
using XRay.Office.Core.Charts;
using XRay.Office.Core.Graphics;
using XRay.Office.Ooxml.DrawingML;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// An automatic series stroke is the accent <em>put through</em> the theme's subtle line style,
/// not the accent itself.
/// </summary>
/// <remarks>
/// <para>
/// Every value here is read out of the reference rendering of
/// <c>slides/batch-017/pptx/Demick_JetBlue.pptx</c> at LibreOffice 26.2.4.2, page 4: the three
/// automatic line series are stroked <c>B45D03</c>, <c>761D26</c> and <c>12415C</c> — 46 records
/// each — where the deck's Aspect theme's accents 1 to 3 are <c>F07F09</c>, <c>9F2936</c> and
/// <c>1B587C</c>. The difference is the theme's own first <c>a:lnStyleLst</c> entry, whose
/// <c>phClr</c> carries <c>&lt;a:shade val="50000"/&gt;&lt;a:satMod val="103000"/&gt;</c>.
/// </para>
/// <para>
/// <c>LineFormatter::convertFormatting</c> is where that happens in LibreOffice: the themed
/// <c>LineProperties</c> are copied whole and then resolved with <c>getPhColor(nSeriesIdx)</c> as
/// the placeholder — <c>oox/source/drawingml/chart/objectformatter.cxx:857-864</c>. The accent is
/// the <em>input</em> to the theme entry, and reading it as the output draws every automatic chart
/// line too bright on any theme that states a transform there.
/// </para>
/// <para>
/// <strong>The control that makes this a fix rather than a difference</strong> is
/// <see cref="AThemeStatingNoTransformLeavesTheAccentAlone"/>. The corpus holds exactly two decks
/// with automatic-stroke series;
/// <c>Sector_Skills_Insights_Advanced_Manufacturing_summary_slide_pack.pptx</c> is the other, and
/// its subtle line style is a bare <c>phClr</c>. It must not move, and measured over a full
/// re-render of the 163-deck track it did not.
/// </para>
/// </remarks>
public class DrawingChartThemedLineTests
{
    private const string C = "http://schemas.openxmlformats.org/drawingml/2006/chart";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    private static readonly Colour Accent1 = new(0xF0, 0x7F, 0x09);
    private static readonly Colour Accent2 = new(0x9F, 0x29, 0x36);
    private static readonly Colour Accent3 = new(0x1B, 0x58, 0x7C);

    /// <summary>The three the reference actually strokes on <c>Demick_JetBlue</c> page 4.</summary>
    private static readonly Colour Themed1 = new(0xB4, 0x5D, 0x03);
    private static readonly Colour Themed2 = new(0x76, 0x1D, 0x26);
    private static readonly Colour Themed3 = new(0x12, 0x41, 0x5C);

    private static DrawingTheme Aspect() => DrawingTheme.Read(XElement.Parse(
        $"""
         <a:theme xmlns:a="{A}"><a:themeElements>
           <a:clrScheme name="Aspect">
             <a:dk1><a:sysClr val="windowText" lastClr="000000"/></a:dk1>
             <a:lt1><a:sysClr val="window" lastClr="FFFFFF"/></a:lt1>
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
           </a:clrScheme>
         </a:themeElements></a:theme>
         """))!;

    /// <summary>A format matrix whose first line style is the given inner markup.</summary>
    private static DrawingStyleMatrix Matrix(string first, string second = "") =>
        DrawingStyleMatrix.Read(XElement.Parse(
            $"""
             <a:theme xmlns:a="{A}"><a:themeElements><a:fmtScheme>
               <a:lnStyleLst>
                 <a:ln w="9525" cap="flat" cmpd="sng" algn="ctr">{first}<a:prstDash val="solid"/></a:ln>
                 <a:ln w="20320">{(second.Length == 0 ? first : second)}</a:ln>
               </a:lnStyleLst>
             </a:fmtScheme></a:themeElements></a:theme>
             """))!;

    /// <summary><c>Demick_JetBlue</c>'s own subtle line style, verbatim.</summary>
    private static DrawingStyleMatrix Demick() => Matrix(
        """
        <a:solidFill><a:schemeClr val="phClr">
          <a:shade val="50000"/><a:satMod val="103000"/>
        </a:schemeClr></a:solidFill>
        """);

    /// <summary>A bare placeholder, which is what most themes state.</summary>
    private static DrawingStyleMatrix Plain() =>
        Matrix("<a:solidFill><a:schemeClr val=\"phClr\"/></a:solidFill>");

    private static string Series(int index, string body = "") =>
        $"""
         <c:ser><c:idx val="{index}"/><c:order val="{index}"/>{body}
           <c:val><c:numRef><c:numCache><c:ptCount val="2"/>
             <c:pt idx="0"><c:v>1</c:v></c:pt><c:pt idx="1"><c:v>2</c:v></c:pt>
           </c:numCache></c:numRef></c:val>
         </c:ser>
         """;

    private static ChartPlot Read(string plotArea, DrawingStyleMatrix? styles)
        => DrawingChartPlot.Read(
               XElement.Parse(
                   $"<c:chartSpace xmlns:c=\"{C}\" xmlns:a=\"{A}\">"
                   + $"<c:chart><c:plotArea>{plotArea}</c:plotArea></c:chart></c:chartSpace>"),
               Aspect(),
               office2007: false,
               styles)
           ?? throw new InvalidOperationException("the reader found nothing to draw");

    [Fact]
    public void AnAutomaticStrokeIsTheAccentPutThroughTheThemesSubtleLineStyle()
    {
        // The whole round in one assertion: these are the reference's own three values.
        ChartPlot plot = Read(
            $"<c:lineChart>{Series(0)}{Series(1)}{Series(2)}</c:lineChart>", Demick());

        plot.Series[0].Line.ShouldBe(Themed1);
        plot.Series[1].Line.ShouldBe(Themed2);
        plot.Series[2].Line.ShouldBe(Themed3);
    }

    [Fact]
    public void TheSameThreeComeOutOfColourOfDirectly()
    {
        DrawingChartAutoFormat.ColourOf(
                DrawingChartAutoFormat.DefaultStyle, ChartAutoObject.LinearSeries,
                stroke: true, index: 0, maximum: 2, Aspect(), Demick())
            .ShouldBe(Themed1);
    }

    [Fact]
    public void AThemeStatingNoTransformLeavesTheAccentAlone()
    {
        // The corpus control. Sector_Skills's theme is exactly this, and it must not move.
        ChartPlot plot = Read(
            $"<c:lineChart>{Series(0)}{Series(1)}{Series(2)}</c:lineChart>", Plain());

        plot.Series[0].Line.ShouldBe(Accent1);
        plot.Series[1].Line.ShouldBe(Accent2);
        plot.Series[2].Line.ShouldBe(Accent3);
    }

    [Fact]
    public void WithNoFormatMatrixTheAccentIsRaw()
    {
        // This is why the words and sheets tracks are untouched by construction rather than by
        // luck: DocxPictures passes no matrix and XlsxDrawings passes `styles: null`, and with a
        // null matrix there is nothing to put the accent through.
        ChartPlot plot = Read($"<c:lineChart>{Series(0)}</c:lineChart>", styles: null);

        plot.Series[0].Line.ShouldBe(Accent1);
    }

    [Fact]
    public void AFillIsNotPutThroughTheLineStyle()
    {
        // spFilledSeries2dFills reaches Theme::getFillStyle, not getLineStyle. A bar drawn in the
        // line style's shade would be half the colour the chart asked for.
        ChartPlot plot = Read(
            $"<c:barChart><c:varyColors val=\"0\"/>{Series(0)}</c:barChart>", Demick());

        plot.Series[0].Fill.ShouldBe(Accent1);
    }

    [Fact]
    public void ItIsTheFirstLineStyleAndNotTheSecond()
    {
        // THEMED_STYLE_SUBTLE is 1. Every automatic series entry in objectformatter.cxx names it;
        // taking the second would darken by whatever the theme's intense entry states.
        DrawingStyleMatrix matrix = Matrix(
            "<a:solidFill><a:schemeClr val=\"phClr\"/></a:solidFill>",
            "<a:solidFill><a:schemeClr val=\"phClr\"><a:shade val=\"50000\"/></a:schemeClr></a:solidFill>");

        Read($"<c:lineChart>{Series(0)}</c:lineChart>", matrix).Series[0].Line.ShouldBe(Accent1);
    }

    [Fact]
    public void AThemeLineStyleWithNoSolidFillLeavesTheAccentAlone()
    {
        // A width-only entry. LibreOffice would leave the line's fill type unset and draw nothing;
        // keeping the accent is the cheaper way to be wrong, and no corpus theme states it.
        Read($"<c:lineChart>{Series(0)}</c:lineChart>", Matrix("")).Series[0].Line.ShouldBe(Accent1);
    }

    [Fact]
    public void AThemeLineStyleStatingALiteralColourOverridesTheAccent()
    {
        // Faithful rather than convenient: assignUsed copies the themed colour verbatim and
        // pushToPropMap only substitutes where the colour *is* phClr, so a theme naming a literal
        // wins over the accent cycle. Unexercised by the corpus; pinned so a future simplification
        // that "obviously" ought to prefer the accent has to argue with the C++.
        DrawingStyleMatrix matrix =
            Matrix("<a:solidFill><a:srgbClr val=\"00FF00\"/></a:solidFill>");

        Read($"<c:lineChart>{Series(0)}</c:lineChart>", matrix)
            .Series[0].Line.ShouldBe(new Colour(0x00, 0xFF, 0x00));
    }

    [Fact]
    public void TheThemeActsOnTheCycleShadedAccentAndNotOnTheBareOne()
    {
        // Order matters and the two are not commutative. getPhColor applies the cycle shade first
        // and hands the *result* to pushToPropMap as the placeholder, so with seven series — one
        // past the six-accent cycle — series 0 is a shaded accent 1 shaded again by the theme.
        string series = string.Concat(Enumerable.Range(0, 7).Select(i => Series(i)));
        Colour drawn = Read($"<c:lineChart>{series}</c:lineChart>", Demick()).Series[0].Line!.Value;

        Colour cycled = DrawingChartAutoFormat.ColourOf(
            DrawingChartAutoFormat.DefaultStyle, ChartAutoObject.LinearSeries,
            stroke: true, index: 0, maximum: 6, Aspect(), styles: null)!.Value;

        drawn.ShouldNotBe(cycled);
        drawn.ShouldBe(
            DrawingChartAutoFormat.ThroughSubtleLineStyle(cycled, Demick(), Aspect()));
        drawn.ShouldNotBe(DrawingChartAutoFormat.ChartTint(Themed1, -0.35));
    }

    [Fact]
    public void AStatedColourStillWinsOverTheThemedAccent()
    {
        // Drift guard for the merge order: the theme is the base and the file's own statement is
        // the override, so threading the matrix must not start overriding the file.
        ChartPlot plot = Read(
            "<c:lineChart>"
            + Series(0, "<c:spPr><a:ln><a:solidFill><a:srgbClr val=\"00FF00\"/></a:solidFill></a:ln></c:spPr>")
            + "</c:lineChart>",
            Demick());

        plot.Series[0].Line.ShouldBe(new Colour(0x00, 0xFF, 0x00));
    }
}
