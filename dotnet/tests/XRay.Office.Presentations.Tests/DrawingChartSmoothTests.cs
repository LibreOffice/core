using System.Xml.Linq;
using XRay.Office.Core.Charts;
using XRay.Office.Ooxml.DrawingML;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// Which OOXML plot groups are drawn as flattened cubic splines — <c>c:smooth</c>.
/// </summary>
/// <remarks>
/// <para>
/// Three rules, all read out of the 27.2 tree at <c>/home/user/libreoffice-core</c> and each
/// confirmed a second time against 26.2.4.2's own output (see <c>probes/chart-smooth-r102</c>).
/// </para>
/// <para>
/// <strong>An absent <c>c:smooth</c> means "smooth" unless Office 2007 wrote the file.</strong>
/// <c>SeriesModel</c>'s constructor is <c>mbSmooth( !bMSO2007Doc )</c>
/// (<c>oox/source/drawingml/chart/seriesmodel.cxx</c>:124) and the line and scatter series
/// contexts read the element as <c>getBool( XML_val, !bMSO2007Doc )</c>
/// (<c>seriescontext.cxx</c>:618, :726). The corpus witnesses it both ways: of the three
/// documents holding a line or scatter group that states no <c>c:smooth</c> at all,
/// <c>Demick_JetBlue.pptx</c> and <c>171128IPAP.pptx</c> declare
/// <c>AppVersion 12.0000</c> and 26.2.4.2's ODF export of them carries no
/// <c>chart:interpolation</c>, while <c>microsoft_learn_multi_chart_examples.xlsx</c> declares
/// <c>3.1</c>, is therefore not an Office 2007 file, and its export carries
/// <c>chart:interpolation="cubic-spline"</c> — with 60 and 200 line segments in the reference's
/// PDF to match.
/// </para>
/// <para>
/// <strong>The property is the chart type's, so one smoothed series smooths its group.</strong>
/// <c>OOX_CHART_SMOOTHED_PER_SERIES</c> is <c>0</c>
/// (<c>oox/inc/drawingml/chart/seriesconverter.hxx</c>:37), which leaves
/// <c>typegroupconverter.cxx</c>:585-588 calling <c>convertLineSmooth</c> against the type's
/// property set for each series whose own flag is set. <em>No corpus document distinguishes
/// this from a per-series rule</em>: all fourteen smoothed groups are uniform.
/// </para>
/// <para>
/// <strong>Only a two-dimensional line, stock or scatter group is smoothed.</strong>
/// <c>convertLineSmooth</c> returns early unless <c>!isSeriesFrameFormat()</c> and the category
/// is not radar (<c>:686</c>), and <c>isSeriesFrameFormat()</c> is
/// <c>mb3dChart || mbSeriesIsFrame2d</c> (<c>:261-264</c>) — false for exactly
/// <c>TYPEID_LINE</c>, <c>TYPEID_STOCK</c> and <c>TYPEID_SCATTER</c> in the type table at
/// <c>:95-117</c>.
/// </para>
/// </remarks>
public class DrawingChartSmoothTests
{
    private const string C = "http://schemas.openxmlformats.org/drawingml/2006/chart";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    private static ChartPlot Read(string plotArea, bool office2007)
        => DrawingChartPlot.Read(
               XElement.Parse(
                   $"<c:chartSpace xmlns:c=\"{C}\" xmlns:a=\"{A}\"><c:chart>{plotArea}</c:chart></c:chartSpace>"),
               DrawingTheme.Read(null),
               office2007,
               null)
           ?? throw new InvalidOperationException("the reader found nothing to draw");

    /// <summary>One series, with <c>c:smooth</c> stated or left out.</summary>
    private static string Series(string? smooth)
    {
        string stated = smooth is null ? "" : $"<c:smooth val=\"{smooth}\"/>";

        return $"""
            <c:ser><c:val><c:numRef><c:numCache>
              <c:ptCount val="2"/><c:pt idx="0"><c:v>1</c:v></c:pt><c:pt idx="1"><c:v>2</c:v></c:pt>
            </c:numCache></c:numRef></c:val>{stated}</c:ser>
            """;
    }

    /// <summary>One type group holding the given series, against one pair of axes.</summary>
    private static string Plot(string group, params string?[] smooth)
    {
        string series = string.Concat(smooth.Select(Series));

        return $"""
            <c:plotArea><c:{group}>{series}</c:{group}>
            <c:catAx><c:axId val="2"/><c:crossAx val="1"/></c:catAx>
            <c:valAx><c:axId val="1"/><c:crossAx val="2"/></c:valAx>
            </c:plotArea>
            """;
    }

    [Theory]
    [InlineData("lineChart")]
    [InlineData("scatterChart")]
    [InlineData("stockChart")]
    public void AStatedSmoothIsHonouredOnTheThreeNonFrameGroups(string group)
    {
        Read(Plot(group, "1"), office2007: true).Series[0].Smooth.ShouldBeTrue();
        Read(Plot(group, "0"), office2007: false).Series[0].Smooth.ShouldBeFalse();
    }

    [Fact]
    public void AnAbsentSmoothIsTrueUnlessOfficeTwoThousandSevenWroteTheFile()
    {
        Read(Plot("lineChart", [null]), office2007: false).Series[0].Smooth.ShouldBeTrue();
        Read(Plot("lineChart", [null]), office2007: true).Series[0].Smooth.ShouldBeFalse();
    }

    /// <summary>
    /// The flag belongs to the group, so one smoothed series carries the rest with it.
    /// </summary>
    [Fact]
    public void OneSmoothedSeriesSmoothsEverySeriesInItsGroup()
    {
        ChartPlot plot = Read(Plot("lineChart", "0", "1", "0"), office2007: true);

        plot.Series.Count.ShouldBe(3);
        plot.Series.ShouldAllBe(series => series.Smooth);
    }

    /// <summary>
    /// A frame group's <c>c:smooth</c> is read by the reference and discarded, and so is a
    /// radar group's and a three-dimensional line group's.
    /// </summary>
    [Theory]
    [InlineData("barChart")]
    [InlineData("areaChart")]
    [InlineData("bubbleChart")]
    [InlineData("radarChart")]
    [InlineData("line3DChart")]
    public void AFrameOrRadarOrThreeDimensionalGroupIsNeverSmoothed(string group)
    {
        Read(Plot(group, "1"), office2007: true)
            .Series.ShouldAllBe(series => !series.Smooth);
        Read(Plot(group, [null]), office2007: false)
            .Series.ShouldAllBe(series => !series.Smooth);
    }

    /// <summary>
    /// A combination chart smooths the line half and leaves the bar half alone, because the
    /// property sits on each group's own chart type.
    /// </summary>
    [Fact]
    public void ACombinationChartSmoothsOnlyItsLineGroup()
    {
        string plotArea = $"""
            <c:plotArea>
            <c:barChart>{Series(null)}</c:barChart>
            <c:lineChart>{Series(null)}</c:lineChart>
            <c:catAx><c:axId val="2"/><c:crossAx val="1"/></c:catAx>
            <c:valAx><c:axId val="1"/><c:crossAx val="2"/></c:valAx>
            </c:plotArea>
            """;

        ChartPlot plot = Read(plotArea, office2007: false);

        plot.Series.Count.ShouldBe(2);
        plot.Series.Single(series => series.Kind == ChartPlotKind.Bar).Smooth.ShouldBeFalse();
        plot.Series.Single(series => series.Kind == ChartPlotKind.Line).Smooth.ShouldBeTrue();
    }
}
