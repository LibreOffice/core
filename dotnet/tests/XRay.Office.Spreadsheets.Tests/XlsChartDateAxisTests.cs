using XRay.Office.Core.Charts;
using XRay.Office.Core.Numbers;
using Shouldly;
using static XRay.Office.Spreadsheets.Tests.BiffChartFixture;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// Which of two label rules a BIFF category axis takes, and what decides it.
/// </summary>
/// <remarks>
/// <para>
/// <c>XclImpChLabelRange::Convert</c> (<c>sc/source/filter/excel/xichart.cxx:3013-3047</c>) is an
/// <c>if</c> and an <c>else</c> over <c>CHDATERANGE</c>'s <c>DATEAXIS</c> flag, and it sets
/// <c>TEXTOVERLAP</c>, <c>TEXTBREAK</c> and <c>ARRANGEORDER</c> in the <c>else</c> alone. Reading
/// only the <c>else</c> — which is what this reader did — applies a date axis' overlap rule from a
/// record the reference never consults for one.
/// </para>
/// <para>
/// It matters because overlap is the first thing <see cref="ChartAxisLabels.Resolve"/> tests: an
/// axis that allows it returns before the auto-rotate ladder is reached, so a crowded date axis
/// came out upright where the reference turns it 45°. Measured on
/// <c>Template Pilot Logbook JAR-FCL V3.0.xls</c>, whose two charts both state
/// <c>CHDATERANGE</c> flags <c>0x00ff</c>: no rotated text at all in our PDF against 848 rotated
/// glyphs in the reference's.
/// </para>
/// </remarks>
public sealed class XlsChartDateAxisTests
{
    /// <summary><c>CHLABELRANGE</c>: a crossing, the label frequency, a tick frequency, flags.</summary>
    private static byte[] LabelRange(ushort frequency) => Record(
        ChLabelRange, [.. Word(1), .. Word(frequency), .. Word(1), .. Word(0)]);

    /// <summary><c>CHDATERANGE</c>: eight fields and then the flag word this is about.</summary>
    private static byte[] DateRange(bool dateAxis) => Record(
        ChDateRange,
        [
            .. Word(0), .. Word(0), .. Word(1), .. Word(0),
            .. Word(1), .. Word(0), .. Word(0), .. Word(0),
            .. Word(dateAxis ? (ushort)0x0010 : (ushort)0x0000),
        ]);

    /// <summary>An X-axis group holding the records given. Axis 0 is the categories.</summary>
    private static byte[] CategoryAxis(params byte[][] children)
        => Group(ChAxis, [.. Word(0), .. new byte[16]], children);

    /// <summary>
    /// Without a date axis, the label frequency decides — which is the rule we already had.
    /// </summary>
    /// <remarks>
    /// Kept as the control. If this moved, the fix would have replaced one blanket rule with
    /// another rather than split them.
    /// </remarks>
    [Fact]
    public void ANonDateAxisTakesItsOverlapRuleFromTheLabelFrequency()
    {
        ChartPlot every = Chart(Substream([.. CategoryAxis(LabelRange(1), DateRange(false))]));
        every.CategoryAxisText.OverlapAllowed.ShouldBeTrue();
        every.CategoryAxisText.LineBreakAllowed.ShouldBeTrue();

        ChartPlot thinned = Chart(Substream([.. CategoryAxis(LabelRange(3), DateRange(false))]));
        thinned.CategoryAxisText.OverlapAllowed.ShouldBeFalse();
        thinned.CategoryAxisText.LineBreakAllowed.ShouldBeFalse();
    }

    /// <summary>
    /// A date axis ignores the frequency and keeps chart2's defaults, so its labels can rotate.
    /// </summary>
    /// <remarks>
    /// The frequency is 1 here — "label every category", the value that turns overlap *on* for
    /// every other axis and the value the corpus's own chart states. A reader that applied it
    /// anyway would leave <c>OverlapAllowed</c> true and this would fail.
    /// </remarks>
    [Fact]
    public void ADateAxisKeepsTheDefaultsWhateverTheLabelFrequencySays()
    {
        ChartPlot plot = Chart(Substream([.. CategoryAxis(LabelRange(1), DateRange(true))]));

        plot.CategoryAxisText.OverlapAllowed.ShouldBeFalse();
        plot.CategoryAxisText.LineBreakAllowed.ShouldBeFalse();
        plot.CategoryAxisText.Stagger.ShouldBe(ChartLabelStagger.Auto);
    }

    /// <summary>
    /// The two records decide together however they are ordered.
    /// </summary>
    /// <remarks>
    /// BIFF does not fix their order and LibreOffice keeps both halves on one
    /// <c>XclImpChLabelRange</c> until <c>Convert</c> reads them together, so a reader that let
    /// whichever arrived last win would be right on the corpus and wrong in general — the corpus's
    /// one file writes <c>CHLABELRANGE</c> first.
    /// </remarks>
    [Fact]
    public void TheOrderOfTheTwoRecordsDoesNotChangeTheAnswer()
    {
        ChartPlot first = Chart(Substream([.. CategoryAxis(LabelRange(1), DateRange(true))]));
        ChartPlot second = Chart(Substream([.. CategoryAxis(DateRange(true), LabelRange(1))]));

        second.CategoryAxisText.ShouldBe(first.CategoryAxisText);
        second.CategoryAxisText.OverlapAllowed.ShouldBeFalse();
    }

    /// <summary>A <c>CHDATERANGE</c> on the value axis says nothing about the categories.</summary>
    /// <remarks>
    /// The record is read under <c>_axis == AxisX</c> only, the same guard <c>CHLABELRANGE</c>
    /// carries. Without it a value axis' own record would silently retune the category axis.
    /// </remarks>
    [Fact]
    public void ADateRangeOnTheValueAxisIsNotTheCategoryAxisRule()
    {
        ChartPlot plot = Chart(Substream(
        [
            .. CategoryAxis(LabelRange(1)),
            .. Group(ChAxis, [.. Word(1), .. new byte[16]], DateRange(true)),
        ]));

        plot.CategoryAxisText.OverlapAllowed.ShouldBeTrue();
    }

    /// <summary>
    /// A date axis is resolved only when the categories are dates as well as declared.
    /// </summary>
    /// <remarks>
    /// <c>EXC_CHDATERANGE_AUTODATE</c> becomes <c>ScaleData::AutoDateAxis</c>, and
    /// <c>AxisHelper::checkDateAxis</c> then asks the categories themselves through
    /// <c>ExplicitCategoriesProvider::isDateAxis</c>, which tests each cell's own number format
    /// (<c>lcl_fillDateCategories</c>). The corpus's one date-axis workbook states flags
    /// <c>0x00ff</c> — every automatic bit, <c>AUTODATE</c> among them — so this branch is the
    /// one that decides it, and a reader that took the <c>DATEAXIS</c> flag alone would put a
    /// date axis on any chart whose author once tried one.
    /// </remarks>
    [Fact]
    public void AnAutomaticDateAxisNeedsTheCategoriesToCarryADateFormat()
    {
        ChartPlot dated = Chart(
            Substream([.. DatedSeries(), .. CategoryAxis(LabelRange(1), AutoDateRange())]),
            withData: true, cellFormat: DateFormat, formatCode: "DD/MM/YY");

        dated.DateAxis.ShouldNotBeNull();

        ChartPlot plain = Chart(
            Substream([.. DatedSeries(), .. CategoryAxis(LabelRange(1), AutoDateRange())]),
            withData: true, cellFormat: OneDecimal, formatCode: "0.0");

        plain.DateAxis.ShouldBeNull();
    }

    /// <summary>An axis that does not declare itself a date axis never becomes one.</summary>
    [Fact]
    public void AnAxisWithoutTheDateFlagIsNotADateAxisHoweverItsCellsAreFormatted()
        => Chart(
            Substream([.. DatedSeries(), .. CategoryAxis(LabelRange(1), DateRange(false))]),
            withData: true, cellFormat: DateFormat, formatCode: "DD/MM/YY")
        .DateAxis.ShouldBeNull();

    /// <summary>
    /// The resolved axis snaps to whole years when one date is all there is to go on.
    /// </summary>
    /// <remarks>
    /// One category cannot put two dates in the same year, so
    /// <c>calculateTimeResolutionOnXAxis</c> stops at <c>YEAR</c>, both limits snap to 1 January
    /// and a range shorter than a year is widened to one (<c>ScaleAutomatism.cxx:586-597</c>).
    /// The fixture's cell holds 42, which is 10 February 1900, so the axis runs from serial 2 to
    /// serial 367 — 1 January 1900 to 1 January 1901 — and carries a tick at each end.
    /// </remarks>
    [Fact]
    public void OneDateGivesAYearlyAxisSnappedToJanuary()
    {
        ChartDateAxis axis = Chart(
            Substream([.. DatedSeries(), .. CategoryAxis(LabelRange(1), AutoDateRange())]),
            withData: true, cellFormat: DateFormat, formatCode: "DD/MM/YY").DateAxis!;

        axis.TimeResolution.ShouldBe(ChartTimeUnit.Year);
        axis.Minimum.ShouldBe(2.0);
        axis.Maximum.ShouldBe(367.0);
        axis.MajorInterval.ShouldBe(new ChartTimeInterval(1, ChartTimeUnit.Year));
        axis.Ticks.ShouldBe([2.0, 367.0]);
    }

    /// <summary>
    /// A stated minimum, maximum and step are honoured, and they are counted in the base unit.
    /// </summary>
    /// <remarks>
    /// <c>lclConvertTimeValue</c> and <c>lclConvertTimeInterval</c>
    /// (<c>xichart.cxx:2960-2988</c>) read every one of those fields as a count of the record's
    /// own base unit, so a minimum of 2 under base unit <em>years</em> is 30 December 1901 and
    /// not serial 2. The record here states years for all four and no automatic bits at all.
    /// </remarks>
    [Fact]
    public void AStatedRangeIsCountedInTheRecordsOwnBaseUnit()
    {
        ChartDateAxis axis = Chart(
            Substream(
            [
                .. DatedSeries(),
                .. CategoryAxis(LabelRange(1), StatedDateRange(minimum: 2, maximum: 6, step: 2)),
            ]),
            withData: true, cellFormat: DateFormat, formatCode: "DD/MM/YY").DateAxis!;

        axis.TimeResolution.ShouldBe(ChartTimeUnit.Year);
        axis.MajorInterval.ShouldBe(new ChartTimeInterval(2, ChartTimeUnit.Year));

        // Two and six years from the null date are 30 December 1901 and 1905, which a year
        // resolution then snaps back to the 1 January before each.
        axis.Ticks.Count.ShouldBe(3);
        ChartDateScale.DateOf(axis.Ticks[0], SpreadsheetDateSystem.Date1900)
            .ShouldBe(new DateOnly(1901, 1, 1));
        ChartDateScale.DateOf(axis.Ticks[^1], SpreadsheetDateSystem.Date1900)
            .ShouldBe(new DateOnly(1905, 1, 1));
    }

    /// <summary>A series naming both its values and its categories.</summary>
    private static byte[] DatedSeries()
        => Group(ChSeries, new byte[8], SeriesLink(), CategoryLink());

    /// <summary><c>CHDATERANGE</c> with every automatic bit set, which is what the corpus states.</summary>
    private static byte[] AutoDateRange() => Record(
        ChDateRange,
        [
            .. Word(0), .. Word(0), .. Word(1), .. Word(0),
            .. Word(1), .. Word(0), .. Word(0), .. Word(0),
            .. Word(0x00FF),
        ]);

    /// <summary>
    /// <c>CHDATERANGE</c> stating its limits and its step, in years, with no automatic bits.
    /// </summary>
    private static byte[] StatedDateRange(ushort minimum, ushort maximum, ushort step) => Record(
        ChDateRange,
        [
            .. Word(minimum), .. Word(maximum), .. Word(step), .. Word(YearsUnit),
            .. Word(1), .. Word(YearsUnit), .. Word(YearsUnit), .. Word(0),
            .. Word(0x0010),
        ]);

    /// <summary>An <c>ifmt</c> the fixture writes a <c>FORMAT</c> record for.</summary>
    private const ushort DateFormat = 201;
    private const ushort OneDecimal = 200;

    /// <summary><c>EXC_CHDATERANGE_YEARS</c>.</summary>
    private const ushort YearsUnit = 2;

    private const ushort ChSeries = 0x1003;
    private const ushort ChLabelRange = 0x1020;
    private const ushort ChDateRange = 0x1062;
}
