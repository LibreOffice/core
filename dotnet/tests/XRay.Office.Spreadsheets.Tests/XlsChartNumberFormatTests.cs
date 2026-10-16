using XRay.Office.Core.Charts;
using Shouldly;
using static XRay.Office.Spreadsheets.Tests.BiffChartFixture;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// Which number format a BIFF chart's value axis writes its tick labels through.
/// </summary>
/// <remarks>
/// <para>
/// <strong>The defect this pins was one decimal place on every tick.</strong>
/// <c>Template Pilot Logbook JAR-FCL V3.0.xls</c> page 17 draws a value axis the reference labels
/// <c>1200.0 1000.0 800.0 … 0.0</c> and we labelled <c>1200 1000 800 … 0</c> — the shortest
/// round-trip form, which is what <see cref="ChartDataLabel.Write"/> falls back to when no format
/// reaches it. The number-format engine was moved down into <c>XRay.Office.Core</c> precisely so a
/// chart axis composed in <c>Core/Charts</c> could call it; the OOXML and ODF readers set
/// <see cref="ChartPlot.ValueFormat"/> from an axis element their markup carries, and the BIFF
/// reader set nothing, so on this path the engine was reachable and never called.
/// </para>
/// <para>
/// <strong>BIFF states it in two places and the order matters.</strong>
/// <c>XclImpChAxis::Convert</c> (<c>sc/source/filter/excel/xichart.cxx:3363-3377</c>) takes the
/// axis' own <c>CHFORMAT</c> when it resolves and turns <c>LinkNumberFormatToSource</c>
/// <em>off</em>; otherwise the axis links to its source and
/// <c>AxisHelper::getExplicitNumberFormatKeyForAxis</c>
/// (<c>chart2/source/tools/AxisHelper.cxx:135-310</c>) asks the value sequence for
/// <c>getNumberFormatKeyByIndex(-1)</c> — "the format of the first non-empty numeric cell"
/// (<c>sc/source/ui/unoobj/chart2uno.cxx:3257-3277</c>).
/// </para>
/// <para>
/// <strong>Both branches are reachable on this corpus, which is why both are read.</strong> A
/// census of every OLE2 file in all three tracks (<c>probes/sheets-chart-01/census.py</c>) finds
/// six documents holding a chart substream and fifteen substreams between them: two state a
/// <c>CHFORMAT</c> on the value axis (<c>ifmt</c> 1 and 3, the built-in <c>0</c> and
/// <c>#,##0</c>) and two resolve <c>0.0</c> through their source cells. The other eleven resolve
/// to General and are unchanged.
/// </para>
/// <para>
/// <strong>What is deliberately not read is <c>CHSOURCELINK</c>'s own <c>ifmt</c>.</strong> It is
/// the field that looks like the answer, sits on the same record as the range, and is wrong: it
/// feeds a data label (<c>XclImpChText::ConvertNumFmt</c>, <c>xichart.cxx:1684</c>). The target
/// document settles it — its second chart's value link states <c>ifmt</c> 370, an index no
/// <c>FORMAT</c> record in that workbook defines, while the cells the link names carry
/// <c>ifmt</c> 175 = <c>0.0</c>, which is what the reference draws.
/// </para>
/// </remarks>
public sealed class XlsChartNumberFormatTests
{
    /// <summary>
    /// An axis with no <c>CHFORMAT</c> takes the format of the cells its series plots.
    /// </summary>
    /// <remarks>
    /// This is the target document's case: the format is stated nowhere in the chart substream at
    /// all, and only the workbook's own <c>XF</c> for cell A1 has it.
    /// </remarks>
    [Fact]
    public void AnAxisWithNoFormatRecordTakesTheSourceCellsFormat()
    {
        Chart(Substream([.. Series()]), withData: true, cellFormat: OneDecimal)
            .ValueFormat?.Code.ShouldBe("0.0");
    }

    /// <summary>
    /// A source cell whose format is General leaves the axis on General, which is null here.
    /// </summary>
    /// <remarks>
    /// Null rather than a <c>NumberFormatCode</c> holding <c>General</c>, because General is not
    /// a format code at all — <c>convertNumberFormat</c> asks the number formats supplier for its
    /// standard index instead of converting a string (<c>objectformatter.cxx:1132-1134</c>), and
    /// <see cref="ChartDataLabel.Write"/> takes the same branch on null.
    /// </remarks>
    [Fact]
    public void AGeneralSourceCellLeavesTheAxisUnformatted()
    {
        Chart(Substream([.. Series()]), withData: true).ValueFormat.ShouldBeNull();
    }

    /// <summary>The axis' own <c>CHFORMAT</c> wins over the format its source cells carry.</summary>
    /// <remarks>
    /// The two disagree here on purpose. That is the whole of <c>bLinkNumberFmtToSource</c>: a
    /// stated format is a statement that the source's is not wanted, and a reader that took the
    /// source's anyway would be right on thirteen of the corpus's fifteen substreams and wrong on
    /// the two that state one.
    /// </remarks>
    [Fact]
    public void TheAxisOwnFormatRecordOutranksTheSource()
    {
        Chart(Substream([.. Series(), .. ValueAxis(Percentage)]),
                withData: true, cellFormat: OneDecimal)
            .ValueFormat?.Code.ShouldBe("0.00%");
    }

    /// <summary>
    /// A <c>CHFORMAT</c> naming an index the workbook never defined falls back to the source.
    /// </summary>
    /// <remarks>
    /// <c>GetScFormat</c> answers <c>NUMBERFORMAT_ENTRY_NOT_FOUND</c> for one and
    /// <c>XclImpChAxis::Convert</c> then leaves <c>bLinkNumberFmtToSource</c> true, so an
    /// unresolvable index is not a statement of "no format" — it is no statement at all. Index
    /// 370 is the one the target document's <c>CHSOURCELINK</c> states, and it defines no format
    /// there either.
    /// </remarks>
    [Fact]
    public void AnUnresolvableFormatIndexFallsBackToTheSource()
    {
        Chart(Substream([.. Series(), .. ValueAxis(370)]), withData: true, cellFormat: OneDecimal)
            .ValueFormat?.Code.ShouldBe("0.0");
    }

    /// <summary>
    /// A <c>CHFORMAT</c> on the <em>category</em> axis does not become the value axis' format.
    /// </summary>
    /// <remarks>
    /// Which axis a record belongs to comes from the <c>CHAXIS</c> that opened the group and from
    /// nothing else, exactly as a <c>CHFONT</c>'s meaning does. Without that test a date axis'
    /// own format would be applied to the value ticks, which on the target document would label
    /// them as dates.
    /// </remarks>
    [Fact]
    public void ACategoryAxisFormatIsNotTheValueAxisFormat()
    {
        Chart(Substream([.. Series(), .. CategoryAxis(Percentage)]), withData: true)
            .ValueFormat.ShouldBeNull();
    }

    /// <summary>
    /// A chart whose series link resolves to nothing leaves the axis unformatted.
    /// </summary>
    /// <remarks>
    /// Without <c>withData</c> the series is dropped — a series with no numbers is not drawn — so
    /// there are no source cells to take a format from. It is the state every BIFF chart was in
    /// before this was read, and it has to stay reachable rather than throwing.
    /// </remarks>
    [Fact]
    public void AChartWithNoResolvableSeriesHasNoValueFormat()
    {
        Chart(Substream([.. Series()])).ValueFormat.ShouldBeNull();
    }

    /// <summary>
    /// The tick label the format actually produces, end to end.
    /// </summary>
    /// <remarks>
    /// The cases above pin what the reader put on the model; this one pins that the model reaches
    /// the engine, which is the half that was missing. <c>1200</c> through <c>0.0</c> is the
    /// reference's own label on <c>Template Pilot Logbook JAR-FCL V3.0.xls</c> page 17, read out
    /// of the banked PDF with <c>pdftotext</c>.
    /// </remarks>
    [Fact]
    public void TheFormatIsWhatWritesTheTick()
    {
        ChartPlot plot = Chart(
            Substream([.. Series()]), withData: true, cellFormat: OneDecimal);

        ChartDataLabel.Write(1200.0, plot.ValueFormat).ShouldBe("1200.0");
        ChartDataLabel.Write(0.0, plot.ValueFormat).ShouldBe("0.0");
    }

    /// <summary>A <c>CHAXIS</c> of the value dimension carrying one <c>CHFORMAT</c>.</summary>
    private static byte[] ValueAxis(ushort format)
        => Group(ChAxis, [.. Word(ValueAxisType), .. new byte[16]], Record(ChFormat, Word(format)));

    /// <summary>The same, on the category dimension.</summary>
    private static byte[] CategoryAxis(ushort format)
        => Group(ChAxis, [.. Word(CategoryAxisType), .. new byte[16]], Record(ChFormat, Word(format)));

    /// <summary>One series whose value link resolves to the fixture's single cell.</summary>
    private static byte[] Series() => Group(ChSeries, new byte[8], SeriesLink());

    /// <summary>An <c>ifmt</c> the fixture writes a <c>FORMAT</c> record for.</summary>
    private const ushort OneDecimal = 200;

    /// <summary>A built-in index, which needs no <c>FORMAT</c> record — <c>0.00%</c>.</summary>
    private const ushort Percentage = 10;

    private const ushort CategoryAxisType = 0;
    private const ushort ValueAxisType = 1;
}
