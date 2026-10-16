using XRay.Office.Core.Charts;
using Shouldly;
using static XRay.Office.Spreadsheets.Tests.BiffChartFixture;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// The two BIFF chart types that are a flag inside another type's record rather than a record of
/// their own — a doughnut inside <c>CHPIE</c> and a bubble chart inside <c>CHSCATTER</c>.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Neither type has an id, so a reader that switches on the record alone cannot see
/// either.</strong> <c>XclImpChType::Finalize</c> (<c>sc/source/filter/excel/xichart.cxx</c>
/// :2288-2303, this tree) decides the type <em>after</em> the body is read:
/// <c>(maData.mnPieHole > 0) ? EXC_CHTYPEID_DONUT : EXC_CHTYPEID_PIE</c> and
/// <c>get_flagvalue(maData.mnFlags, EXC_CHSCATTER_BUBBLES, EXC_CHTYPEID_BUBBLES,
/// EXC_CHTYPEID_SCATTER)</c>. This reader had skipped both bodies entirely, so every BIFF
/// doughnut was drawn as a pie and every BIFF bubble chart as a scatter.
/// </para>
/// <para>
/// Confirmed against 26.2.4.2 on the two fixtures that state one each —
/// <c>007_advanced_excel_bubble.xls</c> (<c>CHSCATTER</c> <c>grbit</c> <c>0x0001</c>) and
/// <c>028_advanced_excel_doughnut.xls</c> (<c>CHPIE</c> <c>pcDonut</c> 50), both 26.2.4.2's
/// own <c>--convert-to 'xls:MS Excel 97'</c> of the corpus workbooks, so both renderers read
/// identical bytes. Worst-page unsigned ink falls <b>0.88 → 0.37</b> and <b>0.36 → 0.16</b>;
/// the six controls beside them are unmoved to the hundredth, the two that matter being their
/// own type-mates <c>003_advanced_excel_pie.xls</c> at 0.24 and
/// <c>006_advanced_excel_scatter.xls</c> at 0.23, whose <c>pcBubble</c> and <c>wBubble</c> are
/// identical to the bubble fixture's. <c>probes/chartbiff-r186</c>.
/// </para>
/// <para>
/// <strong>No corpus rendering moves.</strong> Of the corpus's 180 OLE2 files exactly one
/// states either record — a plain scatter, already right — so this is correctness for a
/// construct the corpus holds only in its OOXML spelling, measured on the reference's own
/// conversion of that same content. The census walks every OLE2 file rather than every
/// <c>.xls</c>, because a chart substream can also sit in a <c>.doc</c> or <c>.ppt</c>
/// ObjectPool.
/// </para>
/// <para>
/// Synthetic, because the point of each case is one field of one record: the corpus files state
/// one answer apiece and cannot show that the field is what decides.
/// </para>
/// </remarks>
public sealed class XlsChartBubbleDoughnutTests
{
    /// <summary>A <c>CHPIE</c> stating no hole is a pie.</summary>
    [Fact]
    public void APieWithNoHoleIsAPie()
    {
        ChartPlot plot = Pie(hole: 0);

        plot.Kind.ShouldBe(ChartPlotKind.Pie);
        plot.Rings.ShouldBeFalse();
    }

    /// <summary>A <c>CHPIE</c> whose <c>pcDonut</c> is non-zero is a doughnut.</summary>
    /// <remarks>
    /// The test is <c>&gt; 0</c> and not a threshold, so the smallest hole a file can state is
    /// enough — which is the whole difference between the two types.
    /// </remarks>
    [Theory]
    [InlineData(1)]
    [InlineData(50)]
    [InlineData(90)]
    public void APieStatingAHoleIsADoughnut(int hole)
    {
        ChartPlot plot = Pie(hole);

        plot.Kind.ShouldBe(ChartPlotKind.Pie);
        plot.Rings.ShouldBeTrue();
    }

    /// <summary>A <c>CHSCATTER</c> without the bubble flag is a scatter chart.</summary>
    [Fact]
    public void AScatterWithoutTheBubbleFlagIsAScatter()
        => Scatter(bubbles: false).Kind.ShouldBe(ChartPlotKind.Scatter);

    /// <summary><c>EXC_CHSCATTER_BUBBLES</c> makes the same record a bubble chart.</summary>
    [Fact]
    public void AScatterStatingTheBubbleFlagIsABubbleChart()
        => Scatter(bubbles: true).Kind.ShouldBe(ChartPlotKind.Bubble);

    /// <summary>
    /// A bubble series takes its sizes from the <c>EXC_CHSRCLINK_BUBBLES</c> link and not from
    /// its values.
    /// </summary>
    /// <remarks>
    /// The two links name different cells — 42 and 7 — so a reader that fed the values through
    /// twice, or that ignored the third link, would be visible here. Without the sizes every
    /// bubble is drawn at the same radius, which is what made the type change alone insufficient.
    /// </remarks>
    [Fact]
    public void ABubbleSeriesTakesItsSizesFromItsOwnLink()
    {
        ChartPlot plot = Scatter(bubbles: true);

        plot.Series.Count.ShouldBe(1);
        plot.Series[0].Values[0].ShouldBe(42.0);
        plot.Series[0].SizeValues.ShouldNotBeNull();
        plot.Series[0].SizeValues!.Count.ShouldBe(1);
        plot.Series[0].SizeValues![0].ShouldBe(7.0);
    }

    /// <summary>A scatter series states the same link and nothing reads it.</summary>
    /// <remarks>
    /// <c>EXC_CHPROP_ROLE_SIZEVALUES</c> is offered to chart2 whatever the group draws
    /// (<c>xichart.cxx</c>:2062) and only <c>BubbleChart</c> ever asks for it, so a scatter
    /// series carrying sizes must be drawn exactly as one that carries none.
    /// </remarks>
    [Fact]
    public void AScatterSeriesIgnoresTheBubbleLink()
        => Scatter(bubbles: false).Series[0].SizeValues.ShouldBeNull();

    /// <summary>A chart whose one type group is a <c>CHPIE</c> stating the hole given.</summary>
    private static ChartPlot Pie(int hole) => Chart(
        Substream(
        [
            .. Group(ChSeries, new byte[8], SeriesLink()),
            .. Group(ChAxesSet, AxesSetHeader(0),
                Group(ChTypeGroup, TypeGroupHeader(0), PieRecord((ushort)hole))),
        ]),
        withData: true);

    /// <summary>A chart whose one type group is a <c>CHSCATTER</c>, with a bubble-size link.</summary>
    private static ChartPlot Scatter(bool bubbles) => Chart(
        Substream(
        [
            .. Group(ChSeries, new byte[8],
                SeriesLink(),
                SeriesLink(destination: BubbleSizes, row: 1)),
            .. Group(ChAxesSet, AxesSetHeader(0),
                Group(ChTypeGroup, TypeGroupHeader(0), ScatterRecord(bubbles))),
        ]),
        withData: true);

    /// <summary><c>EXC_CHSRCLINK_BUBBLES</c>.</summary>
    private const byte BubbleSizes = 3;

    /// <summary>A <c>CHAXESSET</c> header: its id, then the rectangle nothing here reads.</summary>
    private static byte[] AxesSetHeader(ushort id) => [.. Word(id), .. new byte[16]];

    /// <summary>A <c>CHTYPEGROUP</c> header: sixteen ignored bytes, the flags, then the index.</summary>
    private static byte[] TypeGroupHeader(ushort index)
        => [.. new byte[16], .. Word(0), .. Word(index)];

    /// <summary>A BIFF8 <c>CHPIE</c>: the first wedge's angle, the hole, then the flags.</summary>
    private static byte[] PieRecord(ushort hole)
        => Record(ChPie, [.. Word(0), .. Word(hole), .. Word(0)]);

    /// <summary>
    /// A BIFF8 <c>CHSCATTER</c>: the bubble scale, the size interpretation, then the flags.
    /// </summary>
    /// <remarks>
    /// The first two are written as the values Excel's own defaults are, 100 and
    /// <c>EXC_CHSCATTER_AREA</c>, because the reference reads both into <c>XclChTypeData</c> and
    /// passes neither on — so a case that varied them could only assert that nothing happens.
    /// </remarks>
    private static byte[] ScatterRecord(bool bubbles)
        => Record(ChScatter, [.. Word(100), .. Word(1), .. Word((ushort)(bubbles ? 1 : 0))]);
}
