using XRay.Office.Core.Charts;
using XRay.Office.Core.Units;
using Shouldly;
using static XRay.Office.Spreadsheets.Tests.BiffChartFixture;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// How wide a BIFF chart's own rules are, which <c>XlsChartReader</c> used to skip over.
/// </summary>
/// <remarks>
/// <para>
/// A <c>CHLINEFORMAT</c>'s third field is a weight and this reader stepped past it —
/// <c>stream.Skip(2)</c>, commented <em>"the weight, in Excel's four steps"</em> — so every rule a
/// <c>.xls</c> chart drew was a hairline. <c>XclChPropSetHelper::WriteLineProperties</c>
/// (<c>sc/source/filter/excel/xlchart.cxx:906-925</c>) turns the four steps into an API width in
/// hundredths of a millimetre: it opens at <c>nApiWidth = 0</c>, <em>"0 is the width of a hair
/// line"</em>, and switches <c>SINGLE</c> to 35, <c>DOUBLE</c> to 70 and <c>TRIPLE</c> to 105.
/// </para>
/// <para>
/// <strong>Confirmed against 26.2.4.2's own resolved view as well as in source.</strong>
/// <c>--convert-to ods</c> of <c>014_Contextures_chart_sample_991ecfc5.xls</c> writes its chart's
/// graphic styles with <c>svg:stroke-width="0.035cm"</c> and <c>"0.07cm"</c> — 35 and 70
/// hundredths of a millimetre exactly, and nothing else — and its PDF strokes them at 0.9884 and
/// 1.9768 pt, which are those two times 0.0283465 less the 0.38 % the chart's own anisotropic fit
/// scale takes off both.
/// </para>
/// <para>
/// <strong>The weights are signed and <c>SINGLE</c> is zero</strong>
/// (<c>sc/source/filter/inc/xlchart.hxx:260-263</c>), so a reader that took the field as unsigned
/// would map a hairline's −1 to 65535 and fall through to the hairline anyway — right by accident
/// — while a reader that treated 0 as "unstated" would make every single-weight line a hairline,
/// which is exactly the state this replaces.
/// </para>
/// </remarks>
public sealed class XlsChartLineWidthTests
{
    private const ushort Grey = 23;
    private const ushort ValueDimension = 1;

    /// <summary><c>EXC_CHAXISLINE_AXISLINE</c> and <c>EXC_CHAXISLINE_MAJORGRID</c>.</summary>
    private const ushort AxisLineItself = 0;
    private const ushort MajorGrid = 1;

    /// <summary>Each of the four weights maps to the width the reference's own table gives it.</summary>
    /// <remarks>
    /// −1 is <c>HAIR</c> and 3 is not a weight at all; both take the switch's default of zero,
    /// which is a hairline and is what the reference draws for an unrecognised value too.
    /// </remarks>
    [Theory]
    [InlineData(-1, 0)]
    [InlineData(0, 35)]
    [InlineData(1, 70)]
    [InlineData(2, 105)]
    [InlineData(3, 0)]
    public void ASeriesTakesTheWidthItsWeightNames(short weight, int mm100)
    {
        ChartPlot plot = Chart(
            Substream([.. Series(AreaFormat(2), LineFormat(Grey, weight: weight))]),
            withData: true);

        plot.Series.Count.ShouldBe(1);
        plot.Series[0].LineWidth.ShouldBe(Length.FromMm100(mm100));
    }

    /// <summary>An axis line and its major grid each take their own weight.</summary>
    /// <remarks>
    /// The pairing with <c>CHAXISLINE</c> is positional, so the two must not be able to swap: this
    /// gives the axis line a triple and the grid a single, which no single reading satisfies.
    /// </remarks>
    [Fact]
    public void AnAxisLineAndItsGridTakeTheirOwnWidths()
    {
        ChartPlot plot = Chart(Substream(
        [
            .. AxesSet(Axis(
                ValueDimension,
                AxisLine(AxisLineItself), LineFormat(Grey, weight: 2),
                AxisLine(MajorGrid), LineFormat(Grey, weight: 0))),
        ]));

        plot.ValueAxisLine.Width.ShouldBe(Length.FromMm100(105));
        plot.ValueGrid.ShouldNotBeNull().Width.ShouldBe(Length.FromMm100(35));
    }

    /// <summary>An automatic series outline is single-weight, not a hairline.</summary>
    /// <remarks>
    /// <c>spFmtInfos</c> (<c>sc/source/filter/excel/xlchart.cxx:420-440</c>) gives twelve of its
    /// sixteen object types an automatic weight of <c>HAIR</c> and gives
    /// <c>LINEARSERIES</c>, <c>FILLEDSERIES</c> and <c>ERRORBAR</c> <c>SINGLE</c>. So a series is
    /// the one place an automatic line is not a hairline, and its colour still comes from
    /// elsewhere — <see cref="XlsChartAxisLineTests"/> pins that an automatic format names none.
    /// </remarks>
    [Fact]
    public void AnAutomaticSeriesOutlineIsSingleWeightAndNamesNoColour()
    {
        ChartPlot plot = Chart(
            Substream([.. Series(AreaFormat(2), LineFormat(Grey, automatic: true, weight: -1))]),
            withData: true);

        plot.Series.Count.ShouldBe(1);
        plot.Series[0].LineWidth.ShouldBe(Length.FromMm100(35));
        plot.Series[0].Line.ShouldBeNull();
    }

    /// <summary>An automatic line on an axis stays a hairline, which is its own row of that table.</summary>
    [Fact]
    public void AnAutomaticAxisLineStaysAHairline()
    {
        ChartPlot plot = Chart(Substream(
        [
            .. AxesSet(Axis(
                ValueDimension,
                AxisLine(MajorGrid), LineFormat(Grey, automatic: true, weight: 2))),
        ]));

        plot.ValueGrid.ShouldNotBeNull().Width.ShouldBe(Length.Zero);
    }

    /// <summary>A line whose pattern is <c>NONE</c> takes no width at all, whatever it weighs.</summary>
    /// <remarks>
    /// A stated absence, the same distinction <see cref="XlsChartFormatTests"/> draws for a
    /// pattern of <c>EXC_PATT_NONE</c> on a fill. Recording the width anyway would leave a series
    /// that draws nothing carrying a triple weight, which the next reader of the model would have
    /// no way to tell from one that draws.
    /// </remarks>
    [Fact]
    public void ALineStatingNoPatternTakesNoWidth()
    {
        ChartPlot plot = Chart(
            Substream([.. Series(AreaFormat(2), LineFormat(Grey, pattern: 5, weight: 2))]),
            withData: true);

        plot.Series.Count.ShouldBe(1);
        plot.Series[0].LineWidth.ShouldBe(Length.Zero);
    }

    /// <summary>One series with a value link that resolves, and the formats given.</summary>
    private static byte[] Series(params byte[][] formats)
        => Group(ChSeries, new byte[8], SeriesLink(),
            Group(ChDataFormat, [.. Word(0xFFFF), .. Word(0), .. Word(0), .. Word(0)], formats));

    /// <summary>A <c>CHAXESSET</c> holding the axes given, as the primary set.</summary>
    private static byte[] AxesSet(params byte[][] axes)
        => Group(ChAxesSet, [.. Word(0), .. new byte[16]], axes);

    /// <summary>A <c>CHAXIS</c> of one dimension holding the records given.</summary>
    private static byte[] Axis(ushort dimension, params byte[][] children)
        => Group(ChAxis, [.. Word(dimension), .. new byte[16]], children);

    /// <summary>A <c>CHAXISLINE</c> naming which of an axis' lines follows.</summary>
    private static byte[] AxisLine(ushort id) => Record(ChAxisLine, [.. Word(id)]);
}
