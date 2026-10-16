using XRay.Office.Core.Charts;
using Shouldly;
using static XRay.Office.Spreadsheets.Tests.BiffChartFixture;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// Which <c>CHFRAMEPOS</c> is the plot area, and when a BIFF chart states one at all.
/// </summary>
/// <remarks>
/// <para>
/// A BIFF chart has no automatic plot area to speak of: <c>XclImpChChart::Convert</c>
/// (<c>sc/source/filter/excel/xichart.cxx</c>:4030-4048, this tree) hands the primary axes set's
/// <c>CHFRAMEPOS</c> rectangle straight to <c>XDiagramPositioning</c>. Which record that is,
/// though, is decided entirely by where it sits — the corpus' own chart substreams carry
/// <c>CHFRAMEPOS</c> under <c>CHTEXT</c> and under <c>CHLEGEND</c> as well, and a reader that
/// took any of them would place the plot area at a title's offset. That is what most of these
/// cases pin.
/// </para>
/// <para>
/// The rectangle stays in the file's own 1/4000 units here; resolving it needs the frame, and
/// <c>ChartLayoutStatedPlotAreaTests</c> is where the conversion is measured.
/// </para>
/// </remarks>
public sealed class XlsChartPlotAreaTests
{
    /// <summary><c>EXC_CHPROPS_MANSERIES | MANPLOTAREA | USEMANPLOTAREA</c>, as the corpus writes it.</summary>
    private const ushort ManualPlotArea = 0x0019;

    /// <summary>The same without <c>EXC_CHPROPS_USEMANPLOTAREA</c>.</summary>
    private const ushort AutomaticPlotArea = 0x0009;

    [Fact]
    public void ThePrimaryAxesSetsFramePositionIsTheOuterPlotArea()
    {
        ChartPlot plot = Chart(Substream(
            [.. Properties(ManualPlotArea), .. AxesSet(0, FramePos(134, 335, 3326, 3412))]));

        plot.OuterPlotAreaUnits.ShouldBe((134, 335, 3326, 3412));
    }

    /// <summary>
    /// A chart whose <c>CHPROPERTIES</c> does not say the layout is manual states no plot area.
    /// </summary>
    /// <remarks>
    /// <c>XclImpChChart::IsManualPlotArea</c> (<c>xichart.cxx</c>:3973-3977). Its other arm —
    /// BIFF5 and earlier, where "there is no real automatic mode" — is not modelled, and has no
    /// witness: every one of the corpus' sixteen BIFF chart substreams sets the flag.
    /// </remarks>
    [Fact]
    public void AChartThatDoesNotSayTheLayoutIsManualStatesNoPlotArea()
    {
        ChartPlot plot = Chart(Substream(
            [.. Properties(AutomaticPlotArea), .. AxesSet(0, FramePos(134, 335, 3326, 3412))]));

        plot.OuterPlotAreaUnits.ShouldBeNull();
    }

    /// <summary>Both ends have to be placed in the parent's units.</summary>
    /// <remarks>
    /// <c>xichart.cxx</c>:4035 tests <c>mnTLMode</c> and <c>mnBRMode</c> against
    /// <c>EXC_CHFRAMEPOS_PARENT</c> and leaves the diagram alone otherwise. Mode 5 is
    /// <c>EXC_CHFRAMEPOS_CHARTSIZE</c>, which is what the corpus' legends state.
    /// </remarks>
    [Fact]
    public void AFramePositionPlacedInAnythingButTheParentIsIgnored()
    {
        ChartPlot plot = Chart(Substream(
            [.. Properties(ManualPlotArea),
             .. AxesSet(0, FramePos(134, 335, 3326, 3412, topLeftMode: 5))]));

        plot.OuterPlotAreaUnits.ShouldBeNull();
    }

    /// <summary>The secondary axes set's is not the plot area.</summary>
    /// <remarks><c>xichart.cxx</c>:4031 reads <c>mxPrimAxesSet</c> and no other.</remarks>
    [Fact]
    public void TheSecondaryAxesSetsFramePositionIsNotThePlotArea()
    {
        ChartPlot plot = Chart(Substream(
            [.. Properties(ManualPlotArea), .. AxesSet(1, FramePos(134, 335, 3326, 3412))]));

        plot.OuterPlotAreaUnits.ShouldBeNull();
    }

    /// <summary>A title's own frame position is not the plot area either.</summary>
    /// <remarks>
    /// This is the case the corpus would have broken: `Template Pilot Logbook JAR-FCL V3.0.xls`
    /// writes four <c>CHFRAMEPOS</c> records under <c>CHTEXT</c> before the axes set's, three of
    /// them all-zero.
    /// </remarks>
    [Fact]
    public void ATitlesFramePositionIsNotThePlotArea()
    {
        ChartPlot plot = Chart(Substream(
            [.. Properties(ManualPlotArea),
             .. Group(ChText, new byte[32], FramePos(0, 0, 132, 29))]));

        plot.OuterPlotAreaUnits.ShouldBeNull();
    }

    /// <summary>An empty rectangle is no statement, so the computed path still runs.</summary>
    [Fact]
    public void AnEmptyRectangleStatesNoPlotArea()
    {
        ChartPlot plot = Chart(Substream(
            [.. Properties(ManualPlotArea), .. AxesSet(0, FramePos(0, 0, 0, 0))]));

        plot.OuterPlotAreaUnits.ShouldBeNull();
    }

    private static byte[] Properties(ushort flags) =>
        Record(ChProperties, [.. Word(flags), 0, 0]);

    /// <summary>
    /// A <c>CHFRAMEPOS</c>: two modes, then four 16-bit values each padded to 32 bits.
    /// </summary>
    /// <remarks>
    /// The padding is what <c>XclImpChFramePos::ReadChFramePos</c> (<c>xichart.cxx</c>:441-452)
    /// skips — "the upper 16 bits of all members in the rectangle are unused and may contain
    /// garbage" — so it is filled with 0xFF here rather than with zeroes, to make a reader that
    /// took 32-bit values fail.
    /// </remarks>
    private static byte[] FramePos(
        short x, short y, short width, short height,
        ushort topLeftMode = 2, ushort bottomRightMode = 2) => Record(ChFramePos,
    [
        .. Word(topLeftMode), .. Word(bottomRightMode),
        .. Word(unchecked((ushort)x)), 0xFF, 0xFF,
        .. Word(unchecked((ushort)y)), 0xFF, 0xFF,
        .. Word(unchecked((ushort)width)), 0xFF, 0xFF,
        .. Word(unchecked((ushort)height)), 0xFF, 0xFF,
    ]);

    private static byte[] AxesSet(ushort id, params byte[][] children)
    {
        byte[] header = new byte[18];
        header[0] = (byte)(id & 0xFF);
        header[1] = (byte)(id >> 8);
        return Group(ChAxesSet, header, children);
    }
}
