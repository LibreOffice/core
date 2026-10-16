using System.Xml.Linq;
using XRay.Office.Core.Charts;
using XRay.Office.Ooxml.DrawingML;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// What the OOXML drawing reader makes of a <c>c:dateAx</c>, and of the dashed lines beside it.
/// </summary>
/// <remarks>
/// <para>
/// <strong>A date axis is not a category axis with date-shaped labels.</strong> Its points sit
/// where their serial falls between the axis' ends, and — because a polyline joins consecutive
/// points rather than consecutive cells — they have to be put in date order first. The reader
/// had neither, so a file whose cells run newest-first was drawn as a mirror image of the
/// reference: measured on <c>southern-classic-kennesaw-state-university-final.pptx</c>, whose
/// 254 daily closes are stored 12 January 2017 down to 12 January 2016, the reference's axis
/// reads <c>Jan-16 … Jan-17</c> and ours read <c>Jan-17 … Jan-16</c>.
/// </para>
/// <para>
/// Read from markup literals rather than from documents, as <c>ChartPlotTypeReaderTests</c> does
/// and for the same reason: the shape being tested is the markup's.
/// </para>
/// </remarks>
public class DrawingChartDateAxisTests
{
    private const string C = "http://schemas.openxmlformats.org/drawingml/2006/chart";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";

    private static ChartPlot Require(string inner, string before = "")
        => DrawingChartPlot.Read(XElement.Parse(
               $"<c:chartSpace xmlns:c=\"{C}\" xmlns:a=\"{A}\">{before}"
               + $"<c:chart>{inner}</c:chart></c:chartSpace>"))
           ?? throw new InvalidOperationException("the reader found nothing to draw");

    /// <summary>A <c>c:cat</c> of serial numbers under a stated format code.</summary>
    private static string Dates(string formatCode, params double[] serials)
    {
        string points = string.Join("", serials.Select((n, i) =>
            $"<c:pt idx=\"{i}\"><c:v>{n}</c:v></c:pt>"));

        return "<c:cat><c:numRef><c:numCache>"
               + $"<c:formatCode>{formatCode}</c:formatCode>"
               + $"<c:ptCount val=\"{serials.Length}\"/>{points}"
               + "</c:numCache></c:numRef></c:cat>";
    }

    private static string Values(params double[] numbers)
    {
        string points = string.Join("", numbers.Select((n, i) =>
            $"<c:pt idx=\"{i}\"><c:v>{n}</c:v></c:pt>"));

        return $"<c:val><c:numRef><c:numCache><c:ptCount val=\"{numbers.Length}\"/>{points}"
               + "</c:numCache></c:numRef></c:val>";
    }

    /// <summary>A <c>c:dateAx</c> and the <c>c:valAx</c> it crosses.</summary>
    private static string Axes(string auto = "1", string format = "mmm\\-yy", string extra = "")
        => $"""
            <c:dateAx><c:axId val="1"/><c:scaling><c:orientation val="minMax"/></c:scaling>
              <c:numFmt formatCode="{format}" sourceLinked="1"/>
              <c:crossAx val="2"/><c:auto val="{auto}"/>{extra}
            </c:dateAx>
            <c:valAx><c:axId val="2"/><c:crossAx val="1"/></c:valAx>
            """;

    // 42381 is 12 January 2016 and 42747 is 12 January 2017; 42016 is 12 January 2015.
    private const double Jan2015 = 42016;
    private const double Jan2016 = 42381;
    private const double Jan2017 = 42747;

    /// <summary>
    /// Descending serials under a <c>c:dateAx</c> come back as a date scale, in date order, with
    /// every series permuted with them.
    /// </summary>
    [Fact]
    public void ADateAxisPutsNewestFirstCellsIntoDateOrder()
    {
        ChartPlot plot = Require(
            $"""
             <c:plotArea><c:lineChart>
               <c:ser>{Dates("mmm\\-yy", Jan2017, Jan2016, Jan2015)}{Values(30, 20, 10)}</c:ser>
             </c:lineChart>{Axes()}</c:plotArea>
             """);

        ChartDateAxis axis = plot.DateAxis.ShouldNotBeNull();

        // The scale spans the data, oldest at its start.
        axis.Minimum.ShouldBeLessThan(axis.Maximum);
        axis.CategoryValues.Select(v => v ?? 0).ShouldBe([Jan2015, Jan2016, Jan2017]);

        // And the values travelled with them: the 10 that was cell three is now point one.
        plot.Series[0].Values.Select(v => v ?? 0).ShouldBe([10.0, 20.0, 30.0]);

        // Which is the whole point: the points now run left to right along the axis.
        double first = axis.FractionOf(0).ShouldNotBeNull();
        double last = axis.FractionOf(2).ShouldNotBeNull();

        first.ShouldBeLessThan(axis.FractionOf(1).ShouldNotBeNull());
        axis.FractionOf(1).ShouldNotBeNull().ShouldBeLessThan(last);

        // The oldest point is at the start; a year resolution snaps the minimum to 1 January and
        // the cell is the 12th, so it is not exactly zero.
        first.ShouldBeLessThan(0.05);

        // And the newest is two thirds along, not at the end, because the axis carries a fourth
        // year that no point sits in. This part states no `c:crossBetween`, and an absent one is
        // `true` for a line group — `AxisConverter::convertFromModel`'s fallback,
        // `oox/source/drawingml/chart/axisconverter.cxx`:298-301 — which adds one resolution
        // unit to the maximum. Measured rather than inferred: `044_Cash_flow_forecast` rendered
        // by 26.2.4.2 with its `c:crossBetween` deleted is identical to the authored `between`,
        // 13 ticks at a 35.23 pt pitch with the first bar at 15.16, while the same file rewritten
        // to `midCat` gives 12 ticks at 38.46 and no extension. `probes/bardate-r189` §2.
        last.ShouldBe(2.0 / 3.0, 0.02);
    }

    /// <summary>
    /// An automatic date axis over cells that are not dates is a category axis, because chart2
    /// asks <c>ExplicitCategoriesProvider::isDateAxis</c> before it uses the scale.
    /// </summary>
    [Fact]
    public void AnAutomaticDateAxisOverPlainNumbersStaysACategoryAxis()
    {
        ChartPlot plot = Require(
            $"""
             <c:plotArea><c:lineChart>
               <c:ser>{Dates("General", 3, 2, 1)}{Values(30, 20, 10)}</c:ser>
             </c:lineChart>{Axes(format: "General")}</c:plotArea>
             """);

        plot.DateAxis.ShouldBeNull();

        // Nothing was reordered either, which is what makes this safe to leave alone.
        plot.Series[0].Values.Select(v => v ?? 0).ShouldBe([30.0, 20.0, 10.0]);
    }

    /// <summary>
    /// <c>c:auto val="0"</c> is the author saying "this is a date axis", and it outranks the
    /// format check.
    /// </summary>
    [Fact]
    public void AnExplicitDateAxisIsHonouredWhateverTheCategoryFormatSays()
    {
        ChartPlot plot = Require(
            $"""
             <c:plotArea><c:lineChart>
               <c:ser>{Dates("General", Jan2017, Jan2016)}{Values(2, 1)}</c:ser>
             </c:lineChart>{Axes(auto: "0", format: "General")}</c:plotArea>
             """);

        ChartDateAxis axis = plot.DateAxis.ShouldNotBeNull();
        axis.CategoryValues.Select(v => v ?? 0).ShouldBe([Jan2016, Jan2017]);
    }

    /// <summary>
    /// <c>c:date1904</c> sits on the chart space and moves every tick by four years and a day.
    /// </summary>
    [Fact]
    public void TheNineteenOhFourEpochIsReadFromTheChartSpace()
    {
        const string dates = "<c:date1904 val=\"1\"/>";

        ChartPlot plot = Require(
            $"""
             <c:plotArea><c:lineChart>
               <c:ser>{Dates("mmm\\-yy", Jan2016, Jan2015)}{Values(2, 1)}</c:ser>
             </c:lineChart>{Axes()}</c:plotArea>
             """,
            dates);

        ChartDateAxis axis = plot.DateAxis.ShouldNotBeNull();

        // Serial 42016 is 12 January 2015 in the 1900 system and 13 January 2019 in the 1904 one,
        // so the axis' first tick is in a different year under each.
        ChartDateScale.DateOf(axis.Minimum, XRay.Office.Core.Numbers.SpreadsheetDateSystem.Date1904)
            .Year.ShouldBe(2019);
    }

    /// <summary>
    /// A stated <c>c:baseTimeUnit</c> and <c>c:majorUnit</c> reach the scale rather than being
    /// re-derived from the data.
    /// </summary>
    [Fact]
    public void AStatedBaseUnitAndMajorUnitAreHonoured()
    {
        ChartPlot plot = Require(
            $"""
             <c:plotArea><c:lineChart>
               <c:ser>{Dates("mmm\\-yy", Jan2017, Jan2015)}{Values(2, 1)}</c:ser>
             </c:lineChart>
             {Axes(extra: "<c:baseTimeUnit val=\"years\"/>")}
             </c:plotArea>
             """);

        ChartDateAxis axis = plot.DateAxis.ShouldNotBeNull();
        axis.TimeResolution.ShouldBe(ChartTimeUnit.Year);

        // A year resolution snaps both ends to 1 January, so the axis covers two whole years.
        axis.MajorInterval.Unit.ShouldBe(ChartTimeUnit.Year);
    }

    /// <summary>
    /// <c>a:prstDash</c> on a series reaches the model as a dash array scaled by the pen width.
    /// </summary>
    /// <remarks>
    /// <c>sysDot</c> is one dot of 100% of the pen and a gap of 100%, and a round cap takes 99%
    /// off the ink and gives it to the gap — so a 38100 EMU pen comes out as roughly 381 EMU of
    /// ink and 75819 of gap, which is what the reference's threshold lines are drawn with.
    /// </remarks>
    [Fact]
    public void APresetDashOnASeriesReachesTheModel()
    {
        ChartPlot plot = Require(
            $$"""
              <c:plotArea><c:lineChart>
                <c:ser>
                  <c:spPr><a:ln w="38100" cap="rnd"><a:prstDash val="sysDot"/></a:ln></c:spPr>
                  {{Values(1, 2, 3)}}
                </c:ser>
                <c:ser><c:spPr><a:ln w="38100"/></c:spPr>{{Values(3, 2, 1)}}</c:ser>
              </c:lineChart></c:plotArea>
              """);

        IReadOnlyList<XRay.Office.Core.Units.Length> dash =
            plot.Series[0].DashPattern.ShouldNotBeNull();

        dash.Count.ShouldBe(2);
        dash[0].Emu.ShouldBeLessThan(dash[1].Emu);
        // A `prstDash` states its ink and gap as multiples of the line's own width, so the
        // total follows the width through `convertEmuToHmm`: 38100 EMU is 106 hundredths of a
        // millimetre, which is 38160 EMU.
        ((double)(dash[0].Emu + dash[1].Emu)).ShouldBe(38160.0 * 2, 2.0);

        // A line that names no pattern stays solid, which is the case every other chart is.
        plot.Series[1].DashPattern.ShouldBeNull();

        // And the cap goes with it. Without it the 1%-long ink is drawn as a hairline rectangle
        // rather than as a dot, which is what the reference's `1 J` beside every `[0.03 5.97] 0 d`
        // says it is not.
        plot.Series[0].LineCap.ShouldBe(XRay.Office.Core.Graphics.LineCap.Round);
        plot.Series[1].LineCap.ShouldBe(XRay.Office.Core.Graphics.LineCap.Butt);
    }

    /// <summary>
    /// <c>c:grouping val="percentStacked"</c> is a stack <em>and</em> a normalisation, and the
    /// two are separate flags because only the second divides by the category's own total.
    /// </summary>
    [Fact]
    public void APercentStackNormalisesItsCategoriesAndKeepsItsRawValues()
    {
        ChartPlot plot = Require(
            $"""
             <c:plotArea><c:barChart><c:grouping val="percentStacked"/>
               <c:ser>{Values(548, 317)}</c:ser>
               <c:ser>{Values(73, 122)}</c:ser>
             </c:barChart></c:plotArea>
             """);

        plot.IsStacked.ShouldBeTrue();
        plot.IsPercentStacked.ShouldBeTrue();

        // 548 + 73 and 317 + 122 — the divisors the reference's 88% and 72% come from.
        plot.StackTotal(0).ShouldNotBeNull().ShouldBe(621.0);
        plot.StackTotal(1).ShouldNotBeNull().ShouldBe(439.0);

        // The axis is one unit tall however large the raw numbers are. Read as an ordinary stack
        // it was 621, and the 0% format then wrote its ticks as 0% … 70000%.
        (double? minimum, double? maximum) = plot.ValueRange();
        minimum.ShouldNotBeNull().ShouldBe(0.0, 1e-9);
        maximum.ShouldNotBeNull().ShouldBe(1.0, 1e-9);

        // And the stored values are still the file's, so a c:showVal label reads 548.
        plot.Series[0].Values[0].ShouldBe(548.0);
    }

    /// <summary>
    /// <c>a:noFill</c> on a series is a suppression and not an absence, so the chart style's
    /// automatic colour must not be substituted for it.
    /// </summary>
    /// <remarks>
    /// Its opposite is the same test: a series stating no <c>c:spPr</c> at all takes the automatic
    /// colour, which is what keeps every unstated series drawn.
    /// </remarks>
    [Fact]
    public void ASeriesStatingNoFillIsNotGivenTheAutomaticOne()
    {
        // Read against a theme, because the thing being suppressed is the accent cycle and a
        // reader with no theme to ask has no colour to substitute either way.
        ChartPlot plot = DrawingChartPlot.Read(
            XElement.Parse(
                $"<c:chartSpace xmlns:c=\"{C}\" xmlns:a=\"{A}\"><c:chart><c:plotArea>"
                + "<c:barChart><c:grouping val=\"stacked\"/>"
                + $"<c:ser><c:idx val=\"0\"/>{Values(548, 317)}</c:ser>"
                + $"<c:ser><c:idx val=\"1\"/><c:spPr><a:noFill/></c:spPr>{Values(73, 122)}</c:ser>"
                + "</c:barChart></c:plotArea></c:chart></c:chartSpace>"),
            DrawingTheme.Read(XElement.Parse(Theme)))
            ?? throw new InvalidOperationException("the reader found nothing to draw");

        plot.Series[1].Fill.ShouldBeNull();

        // The control: the first series states nothing and still takes accent 1.
        plot.Series[0].Fill.ShouldNotBeNull();
    }

    /// <summary>A theme with the six accents, so the automatic cycle has somewhere to resolve.</summary>
    private const string Theme =
        $"<a:theme xmlns:a=\"{A}\"><a:themeElements><a:clrScheme name=\"t\">"
        + "<a:dk1><a:srgbClr val=\"000000\"/></a:dk1><a:lt1><a:srgbClr val=\"FFFFFF\"/></a:lt1>"
        + "<a:dk2><a:srgbClr val=\"222222\"/></a:dk2><a:lt2><a:srgbClr val=\"EEEEEE\"/></a:lt2>"
        + "<a:accent1><a:srgbClr val=\"4472C4\"/></a:accent1>"
        + "<a:accent2><a:srgbClr val=\"ED7D31\"/></a:accent2>"
        + "<a:accent3><a:srgbClr val=\"A5A5A5\"/></a:accent3>"
        + "<a:accent4><a:srgbClr val=\"FFC000\"/></a:accent4>"
        + "<a:accent5><a:srgbClr val=\"5B9BD5\"/></a:accent5>"
        + "<a:accent6><a:srgbClr val=\"70AD47\"/></a:accent6>"
        + "<a:hlink><a:srgbClr val=\"0563C1\"/></a:hlink>"
        + "<a:folHlink><a:srgbClr val=\"954F72\"/></a:folHlink>"
        + "</a:clrScheme></a:themeElements></a:theme>";

    /// <summary>An ordinary stack is unaffected: no normalisation and no divisor.</summary>
    [Fact]
    public void APlainStackIsNotNormalised()
    {
        ChartPlot plot = Require(
            $"""
             <c:plotArea><c:barChart><c:grouping val="stacked"/>
               <c:ser>{Values(548, 317)}</c:ser>
               <c:ser>{Values(73, 122)}</c:ser>
             </c:barChart></c:plotArea>
             """);

        plot.IsStacked.ShouldBeTrue();
        plot.IsPercentStacked.ShouldBeFalse();
        plot.StackTotal(0).ShouldBeNull();

        (_, double? maximum) = plot.ValueRange();
        maximum.ShouldNotBeNull().ShouldBe(621.0, 1e-9);
    }
}
