using System.Xml.Linq;
using XRay.Office.Core.Charts;
using XRay.Office.Core.Graphics;
using XRay.Office.Ooxml.DrawingML;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// The colours, markers and widths a chart gives a series that states none.
/// </summary>
/// <remarks>
/// <para>
/// The reference for every number here is <c>Demick_JetBlue.pptx</c> rendered by
/// LibreOffice 24.2.7.2, whose five chart parts state no <c>c:style</c>, no <c>c:spPr</c> on any
/// series and no <c>c:marker</c> on any series. Its theme is Aspect, whose first three accents
/// are <c>F07F09</c>, <c>9F2936</c> and <c>1B587C</c> — and those are the three colours the
/// reference draws its three line series in, with no shade or tint on any of them.
/// </para>
/// <para>
/// The refutations pinned here, each of which was in the tree before this round:
/// a missing <c>c:spPr</c> meaning "no colour"; a missing <c>c:marker</c> meaning "no marker" on
/// a line chart; and the chart-style default being anything but 2.
/// </para>
/// </remarks>
public class DrawingChartAutoFormatTests
{
    private const string C = "http://schemas.openxmlformats.org/drawingml/2006/chart";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    /// <summary>Aspect's accents 1 to 3, which are what every assertion here names.</summary>
    private static readonly Colour Accent1 = new(0xF0, 0x7F, 0x09);
    private static readonly Colour Accent2 = new(0x9F, 0x29, 0x36);
    private static readonly Colour Accent3 = new(0x1B, 0x58, 0x7C);

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
           <a:fmtScheme name="Aspect">
             <a:fillStyleLst><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:fillStyleLst>
             <a:lnStyleLst>
               <a:ln w="9525"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln>
               <a:ln w="20320"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln>
             </a:lnStyleLst>
           </a:fmtScheme>
         </a:themeElements></a:theme>
         """))!;

    private static DrawingStyleMatrix Styles() => DrawingStyleMatrix.Read(XElement.Parse(
        $"""
         <a:theme xmlns:a="{A}"><a:themeElements><a:fmtScheme>
           <a:lnStyleLst>
             <a:ln w="9525"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln>
             <a:ln w="20320"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln>
           </a:lnStyleLst>
         </a:fmtScheme></a:themeElements></a:theme>
         """))!;

    /// <summary>One series of a group, with the given index and body.</summary>
    private static string Series(int index, string body = "") =>
        $"""
         <c:ser><c:idx val="{index}"/><c:order val="{index}"/>{body}
           <c:cat><c:strRef><c:strCache><c:ptCount val="2"/>
             <c:pt idx="0"><c:v>a</c:v></c:pt><c:pt idx="1"><c:v>b</c:v></c:pt>
           </c:strCache></c:strRef></c:cat>
           <c:val><c:numRef><c:numCache><c:ptCount val="2"/>
             <c:pt idx="0"><c:v>1</c:v></c:pt><c:pt idx="1"><c:v>2</c:v></c:pt>
           </c:numCache></c:numRef></c:val>
         </c:ser>
         """;

    private static ChartPlot Read(string plotArea, string space = "", bool styles = true)
        => DrawingChartPlot.Read(
               XElement.Parse(
                   $"<c:chartSpace xmlns:c=\"{C}\" xmlns:a=\"{A}\">"
                   + $"<c:chart><c:plotArea>{plotArea}</c:plotArea></c:chart>{space}</c:chartSpace>"),
               Aspect(),
               office2007: false,
               styles ? Styles() : null)
           ?? throw new InvalidOperationException("the reader found nothing to draw");

    [Fact]
    public void ALineSeriesStatingNoPropertiesTakesTheThemesAccentCycle()
    {
        ChartPlot plot = Read(
            $"<c:lineChart>{Series(0)}{Series(1)}{Series(2)}</c:lineChart>");

        plot.Series.Count.ShouldBe(3);
        plot.Series[0].Line.ShouldBe(Accent1);
        plot.Series[1].Line.ShouldBe(Accent2);
        plot.Series[2].Line.ShouldBe(Accent3);
    }

    [Fact]
    public void TheCycleIsNumberedByTheSeriesIndexAndNotByItsPositionInTheFile()
    {
        // A combination chart's second group carries c:idx 2 and holds one series. It takes
        // accent 3, because the cycle is numbered over the whole plot area.
        ChartPlot plot = Read(
            $"<c:lineChart>{Series(0)}{Series(1)}</c:lineChart>"
            + $"<c:lineChart>{Series(2)}</c:lineChart>");

        plot.Series.Count.ShouldBe(3);
        plot.Series[2].Line.ShouldBe(Accent3);
    }

    [Fact]
    public void ASeriesThatStatesItsOwnColourKeepsIt()
    {
        ChartPlot plot = Read(
            "<c:lineChart>"
            + Series(0, "<c:spPr><a:ln><a:solidFill><a:srgbClr val=\"00FF00\"/></a:solidFill></a:ln></c:spPr>")
            + Series(1)
            + "</c:lineChart>");

        plot.Series[0].Line.ShouldBe(new Colour(0x00, 0xFF, 0x00));
        plot.Series[1].Line.ShouldBe(Accent2);
    }

    [Fact]
    public void AnUnstatedChartStyleIsTwoAndNotOne()
    {
        // Style 1 is the greyscale pattern and style 2 the accents, so reading the default as 1
        // draws every automatic chart in tints of black. ChartSpaceModel's mnStyle( 2 ).
        DrawingChartAutoFormat.StyleOf(null).ShouldBe(2);

        ChartPlot plot = Read($"<c:lineChart>{Series(0)}</c:lineChart>");
        plot.Series[0].Line.ShouldBe(Accent1);
    }

    [Fact]
    public void AStatedChartStyleIsRead()
    {
        // Style 1 is spAutoFormatPattern1 — six tints of dk1 — so the first series is not an
        // accent at all. Its first entry is dk1 under tint 88500.
        ChartPlot plot = Read(
            $"<c:lineChart>{Series(0)}</c:lineChart>",
            space: "<c:style val=\"1\"/>");

        plot.Series[0].Line.ShouldNotBe(Accent1);
        plot.Series[0].Line.ShouldNotBeNull();
    }

    [Fact]
    public void ASingleColourStyleShadesAndTintsAcrossTheSeries()
    {
        // Styles 3 to 8 are AUTOFORMAT_FADEDACCENTS: one accent, a cycle of one, so every series
        // lands in its own cycle and is separated by getPhColor's shade/tint step. Three series
        // step -35%, 0%, +35% of accent 1, so the middle one is the accent exactly and the
        // outer two are not.
        ChartPlot plot = Read(
            $"<c:lineChart>{Series(0)}{Series(1)}{Series(2)}</c:lineChart>",
            space: "<c:style val=\"3\"/>");

        plot.Series[1].Line.ShouldBe(Accent1);
        plot.Series[0].Line.ShouldNotBe(Accent1);
        plot.Series[2].Line.ShouldNotBe(Accent1);

        // Leading series are darkened and trailing ones lightened, in that order.
        Colour dark = plot.Series[0].Line!.Value;
        Colour light = plot.Series[2].Line!.Value;
        (dark.R + dark.G + dark.B).ShouldBeLessThan(Accent1.R + Accent1.G + Accent1.B);
        (light.R + light.G + light.B).ShouldBeGreaterThan(Accent1.R + Accent1.G + Accent1.B);
    }

    [Fact]
    public void ABarSeriesTakesTheCycleAsItsFillAndDrawsNoOutline()
    {
        // spFilledSeriesLines is AUTOFORMAT_INVISIBLE for styles 1 to 8, which is why an
        // ordinary bar chart has no bar outline.
        ChartPlot plot = Read(
            $"<c:barChart><c:varyColors val=\"0\"/>{Series(0)}{Series(1)}</c:barChart>");

        plot.Series[0].Fill.ShouldBe(Accent1);
        plot.Series[1].Fill.ShouldBe(Accent2);
        plot.Series[0].Line.ShouldBeNull();
    }

    [Fact]
    public void ALineSeriesStatingNoWidthTakesTheThemesSubtleLineTrebled()
    {
        // 9525 EMU at 300% is 28575 EMU, which is 2.25 pt. Against the hairline this otherwise
        // gets, that is the difference between a chart and a wireframe.
        ChartPlot plot = Read($"<c:lineChart>{Series(0)}</c:lineChart>");
        // `convertEmuToHmm` rounds a DrawingML width to a whole hundredth of a millimetre, so the
        // reference never holds the EMU the file states. `DrawingChartAutoFormat.LineWidth` has
        // the two legs; `probes/stroke-resid-r117/results.md` §3 has the measurements.
        // 28575 EMU is 79.375 of them and 26.2.4.2 draws 79 — measured on
        // `words-chart-auto-line.docx`, whose line series states exactly this width.
        plot.Series[0].LineWidth.ShouldBe(Core.Units.Length.FromMm100(79));
    }

    [Fact]
    public void AutomaticWidthNeedsTheFormatMatrixAndInventsNothingWithoutIt()
    {
        // The route this round added. Without the matrix the width stays at the reader's
        // hairline rather than at a guessed constant — and the colour still resolves, because
        // that comes from the colour scheme.
        ChartPlot plot = Read($"<c:lineChart>{Series(0)}</c:lineChart>", styles: false);

        plot.Series[0].LineWidth.Emu.ShouldBe(0);
        plot.Series[0].Line.ShouldBe(Accent1);
    }

    [Fact]
    public void ALineSeriesStatingNoMarkerDrawsAnAutomaticOneThatCyclesWithTheIndex()
    {
        ChartPlot plot = Read(
            $"<c:lineChart>{Series(0)}{Series(1)}{Series(2)}</c:lineChart>");

        plot.Series[0].Marker.ShouldBe(ChartMarker.Square);
        plot.Series[1].Marker.ShouldBe(ChartMarker.Diamond);
        plot.Series[2].Marker.ShouldBe(ChartMarker.Triangle);
    }

    [Fact]
    public void ASymbolOfNoneTurnsTheMarkerOffAndTheGroupsShowFlagDoesNot()
    {
        // c:ser/c:marker/c:symbol is what suppresses a marker — seriescontext.cxx reads it as
        // getToken( XML_val, XML_none ). The group's own <c:marker val="0"/> is
        // TypeGroupModel::mbShowMarker, which is parsed by oox and read by nothing in oox or
        // chart2, so honouring it would draw fewer markers than the reference.
        ChartPlot off = Read(
            "<c:lineChart>"
            + Series(0, "<c:marker><c:symbol val=\"none\"/></c:marker>")
            + "</c:lineChart>");
        off.Series[0].Marker.ShouldBe(ChartMarker.None);

        ChartPlot group = Read(
            $"<c:lineChart>{Series(0)}<c:marker val=\"0\"/></c:lineChart>");
        group.Series[0].Marker.ShouldBe(ChartMarker.Square);
    }

    [Fact]
    public void ABarSeriesDrawsNoMarker()
    {
        ChartPlot plot = Read($"<c:barChart>{Series(0)}</c:barChart>");
        plot.Series[0].Marker.ShouldBe(ChartMarker.None);
    }

    [Fact]
    public void APieColoursEveryWedgeFromTheCycleWithThePointCountSizingIt()
    {
        ChartPlot plot = Read($"<c:pieChart>{Series(0)}</c:pieChart>");

        // Two points, so the shade/tint step is 1.4/3 rather than nought and neither wedge is a
        // bare accent — but they are different from each other, which is the whole point.
        plot.Series[0].FillAt(0).ShouldNotBeNull();
        plot.Series[0].FillAt(1).ShouldNotBeNull();
        plot.Series[0].FillAt(0).ShouldNotBe(plot.Series[0].FillAt(1));
    }

    [Fact]
    public void AChartSpacesStatedOutlineIsRead()
    {
        ChartPlot plot = Read(
            $"<c:lineChart>{Series(0)}</c:lineChart>",
            space: "<c:spPr><a:ln w=\"25400\"><a:solidFill><a:srgbClr val=\"000000\"/></a:solidFill></a:ln></c:spPr>");

        plot.Border.ShouldBe(new Colour(0, 0, 0));
        // `convertEmuToHmm` rounds a DrawingML width to a whole hundredth of a millimetre, so the
        // reference never holds the EMU the file states. `DrawingChartAutoFormat.LineWidth` has
        // the two legs; `probes/stroke-resid-r117/results.md` §3 has the measurements.
        plot.BorderWidth.ShouldBe(Core.Units.Length.FromMm100(71));
    }

    [Fact]
    public void AChartSpaceStatingNoOutlineHasNone()
    {
        // A PPTX chart's automatic chart-space line is spNoFormats — invisible for every style —
        // and the grey D9D9D9 default is skipped for the Impress filter (tdf#150176).
        Read($"<c:lineChart>{Series(0)}</c:lineChart>").Border.ShouldBeNull();
    }

    [Fact]
    public void AGradientFilledSeriesIsDrawnInItsMiddleStopRatherThanNotAtAll()
    {
        ChartPlot plot = Read(
            "<c:barChart><c:varyColors val=\"0\"/>"
            + Series(0, """
                <c:spPr><a:gradFill><a:gsLst>
                  <a:gs pos="0"><a:srgbClr val="000000"/></a:gs>
                  <a:gs pos="50000"><a:srgbClr val="336699"/></a:gs>
                  <a:gs pos="100000"><a:srgbClr val="FFFFFF"/></a:gs>
                </a:gsLst></a:gradFill></c:spPr>
                """)
            + "</c:barChart>");

        plot.Series[0].Fill.ShouldBe(new Colour(0x33, 0x66, 0x99));
    }
}
