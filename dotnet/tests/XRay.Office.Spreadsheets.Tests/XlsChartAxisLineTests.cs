using XRay.Office.Core.Charts;
using XRay.Office.Core.Graphics;
using Shouldly;
using static XRay.Office.Spreadsheets.Tests.BiffChartFixture;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// What a BIFF axis states about its own tick marks and about the colour of its lines.
/// </summary>
/// <remarks>
/// <para>
/// Neither was read before this. Every BIFF axis took <see cref="ChartPlot"/>'s OOXML default of
/// an <em>outward</em> tick and drew its axis line and its gridlines black, so a chart whose
/// <c>CHTICK</c> says no tick at all was given a 4.25 pt tick at every gridline <em>and</em> a
/// band of that width off the plot rectangle for it.
/// </para>
/// <para>
/// Both are confirmed against 26.2.4.2 itself rather than only against a source reading.
/// <c>--convert-to fods</c> on <c>EHEST-Pre-departure-checklist-Rev.-1-06-12-2016.xls</c> resolves
/// all eighteen of that workbook's chart axes to
/// <c>chart:tick-marks-major-inner="false" chart:tick-marks-major-outer="false"</c> and its axis
/// and gridline styles to <c>svg:stroke-color="#808080"</c>; the reference's own rendering of
/// page 15 draws no tick mark anywhere and strokes all eight gauge gridlines <c>#808080</c>.
/// </para>
/// </remarks>
public sealed class XlsChartAxisLineTests
{
    /// <summary>Grey 50 % — palette index 23, and what <c>EHEST</c>'s gridlines state.</summary>
    private const ushort Grey = 23;

    private const ushort ValueDimension = 1;
    private const ushort CategoryDimension = 0;

    /// <summary><c>EXC_CHAXISLINE_AXISLINE</c>.</summary>
    private const ushort AxisLineItself = 0;

    /// <summary><c>EXC_CHAXISLINE_MAJORGRID</c>.</summary>
    private const ushort MajorGrid = 1;

    [Fact]
    public void AnAxisWhoseTickRecordStatesNoTickMarkReservesAndDrawsNone()
    {
        ChartPlot plot = Chart(Substream([.. AxesSet(Axis(ValueDimension, Tick(0)))]));

        plot.ValueTicks.ShouldBe(ChartTickMark.None);
    }

    /// <summary>The two flags are read separately, so all four states are reachable.</summary>
    /// <remarks>
    /// <c>lclGetApiTickmarks</c> (<c>sc/source/filter/excel/xichart.cxx</c>:3166-3172, this tree)
    /// sets <c>INNER</c> from <c>EXC_CHTICK_INSIDE</c> 0x01 and <c>OUTER</c> from
    /// <c>EXC_CHTICK_OUTSIDE</c> 0x02 — so 3 is a crossed tick and not a third kind.
    /// </remarks>
    [Theory]
    [InlineData(0, ChartTickMark.None)]
    [InlineData(1, ChartTickMark.Inner)]
    [InlineData(2, ChartTickMark.Outer)]
    [InlineData(3, ChartTickMark.Cross)]
    public void EachTickTypeIsReadAsItsOwnPairOfFlags(byte major, ChartTickMark expected)
    {
        ChartPlot plot = Chart(Substream([.. AxesSet(Axis(ValueDimension, Tick(major)))]));

        plot.ValueTicks.ShouldBe(expected);
    }

    /// <summary>The value axis' record must not answer for the category axis.</summary>
    [Fact]
    public void EachAxisTakesItsOwnTickRecord()
    {
        ChartPlot plot = Chart(Substream(
        [
            .. AxesSet(
                Axis(CategoryDimension, Tick(2)),
                Axis(ValueDimension, Tick(0))),
        ]));

        plot.CategoryTicks.ShouldBe(ChartTickMark.Outer);
        plot.ValueTicks.ShouldBe(ChartTickMark.None);
    }

    /// <summary>An axis stating no <c>CHTICK</c> is crossed, which is BIFF's own default.</summary>
    /// <remarks>
    /// <c>XclChTick</c>'s constructor takes <c>mnMajor(EXC_CHTICK_INSIDE | EXC_CHTICK_OUTSIDE)</c>
    /// (<c>sc/source/filter/excel/xlchart.cxx</c>:304-313) and <c>XclImpChAxis::Finalize</c> makes
    /// one for an axis that read no record (<c>xichart.cxx</c>:3303-3304). The old behaviour —
    /// <see cref="ChartTickMark.Outer"/>, which is OOXML's default — differed from it in what is
    /// <em>drawn</em> and not in what is reserved, because only the outward half takes a band.
    /// </remarks>
    [Fact]
    public void AnAxisStatingNoTickRecordIsCrossed()
    {
        ChartPlot plot = Chart(Substream([.. AxesSet(Axis(ValueDimension))]));

        plot.ValueTicks.ShouldBe(ChartTickMark.Cross);
        plot.CategoryTicks.ShouldBe(ChartTickMark.Cross);
    }

    /// <summary>
    /// The <c>CHLINEFORMAT</c> after a <c>CHAXISLINE</c> formats the line that record named.
    /// </summary>
    [Fact]
    public void TheFormatAfterAnAxisLineRecordColoursThatLine()
    {
        ChartPlot plot = Chart(Substream(
        [
            .. AxesSet(Axis(
                ValueDimension,
                AxisLine(AxisLineItself), LineFormat(Grey),
                AxisLine(MajorGrid), LineFormat(Grey))),
        ]));

        plot.ValueAxisLine.Colour.ShouldBe(Colour.FromRgb(0x808080));
        plot.ValueGrid.ShouldNotBeNull().Colour.ShouldBe(Colour.FromRgb(0x808080));
    }

    /// <summary>
    /// An automatic format names no colour, and black is what the reference falls back to.
    /// </summary>
    /// <remarks>
    /// <c>XclImpChLineFormat::Convert</c> takes
    /// <c>GetPalette().GetColor(rFmtInfo.mnAutoLineColorIdx)</c> for an automatic line, and for a
    /// gridline that index is <c>EXC_COLOR_CHWINDOWTEXT</c>, which
    /// <c>sc/source/filter/excel/xlstyle.cxx</c>:150 answers <c>COL_BLACK</c> for.
    /// </remarks>
    [Fact]
    public void AnAutomaticAxisLineFormatLeavesTheLineBlack()
    {
        ChartPlot plot = Chart(Substream(
        [
            .. AxesSet(Axis(
                ValueDimension,
                AxisLine(MajorGrid), LineFormat(Grey, automatic: true))),
        ]));

        plot.ValueGrid.ShouldNotBeNull().Colour.ShouldBe(Colour.Black);
    }

    /// <summary>
    /// A series' own <c>CHLINEFORMAT</c> is not swallowed by an axis line that took no format.
    /// </summary>
    /// <remarks>
    /// The pairing is positional, so a <c>CHAXISLINE</c> with nothing after it inside its own
    /// group must not claim the next line format the substream states anywhere else. That is what
    /// closing the pairing at the axis' own header record buys.
    /// </remarks>
    [Fact]
    public void AnAxisLineWithNoFormatDoesNotClaimTheNextSeriesLine()
    {
        ChartPlot plot = Chart(
            Substream(
            [
                .. AxesSet(Axis(ValueDimension, AxisLine(MajorGrid))),
                .. Series(AreaFormat(2), LineFormat(3)),
            ]),
            withData: true);

        plot.ValueGrid.ShouldNotBeNull().Colour.ShouldBe(Colour.Black);
        plot.Series.Count.ShouldBe(1);
        plot.Series[0].Line.ShouldBe(Colour.FromRgb(0x00FF00));
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

    /// <summary>
    /// A BIFF8 <c>CHTICK</c>: the major and minor tick types, the label position, the background
    /// mode, sixteen ignored bytes, an <c>RGB</c>, the flags, a palette index and a rotation.
    /// </summary>
    private static byte[] Tick(byte major) => Record(ChTick,
    [
        major, 0, 3, 1,
        .. new byte[16],
        .. Dword(0),
        .. Word(0x0023),
        .. Word(0x004D),
        .. Word(0),
    ]);
}
