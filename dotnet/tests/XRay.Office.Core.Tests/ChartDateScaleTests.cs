using XRay.Office.Core.Charts;
using XRay.Office.Core.Numbers;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// Checks the date axis against the ticks LibreOffice 26.2.4.2 actually draws.
/// </summary>
/// <remarks>
/// <para>
/// Every expectation here is a reading of an installed binary's PDF rather than of the C++ tree,
/// because three of the four rules read the other way round in source. The corpus's one date-axis
/// workbook — <c>sheets/batch-010/xls/Template Pilot Logbook JAR-FCL V3.0.xls</c> — states
/// categories from 37935 to 41292 and the reference's page 17 draws its first tick at serial
/// <em>zero</em>, its labels three years apart, and every one of them on the 2nd of the month.
/// </para>
/// <para>
/// A date axis is worth asserting on numerically for the same reason
/// <see cref="ChartScaleTests"/> gives for a value axis: an axis whose minimum is wrong by a
/// century puts every point in the wrong place while the picture still looks like a chart.
/// </para>
/// </remarks>
public sealed class ChartDateScaleTests
{
    private static readonly NumberFormatCode DayMonthYear = NumberFormatCode.Parse("DD/MM/YY");

    /// <summary>The categories the Pilot Logbook's page-17 chart plots, blanks included.</summary>
    /// <remarks>
    /// <c>GraphData!A2:A800</c> — one date at the top, 24 in a run near the bottom, 774 blanks
    /// between. The blanks are given as zero because the chart is an area chart, which is what
    /// <c>AreaChart::addSeries</c> does to a series whose missing values are gaps.
    /// </remarks>
    private static double?[] LogbookCategories(bool blanksAsZero)
    {
        double?[] values = new double?[799];
        for (int at = 0; at < values.Length; at++) values[at] = blanksAsZero ? 0.0 : null;

        values[0] = 37935;
        for (int at = 0; at < 24; at++) values[599 + at] = 41258 + (at * 2 > 34 ? 34 : at * 2);
        values[622] = 41292;
        values[623] = 41292;

        return values;
    }

    [Fact]
    public void AnAxisStartsAtTheSerialItsBlankCategoriesContributeWhenTheyCountAsZero()
    {
        ChartDateAxis axis = ChartDateScale.Resolve(
            LogbookCategories(blanksAsZero: true), DayMonthYear)!;

        axis.Minimum.ShouldBe(0.0);
        axis.Maximum.ShouldBe(41292.0);
    }

    [Fact]
    public void AnAxisWhoseBlanksAreGapsStartsAtTheFirstDateInstead()
    {
        ChartDateAxis axis = ChartDateScale.Resolve(
            LogbookCategories(blanksAsZero: false), DayMonthYear)!;

        axis.Minimum.ShouldBe(37935.0);
    }

    /// <summary>
    /// The interval is <c>(days spanned) / 499</c> in nominal units, not one chosen to fit.
    /// </summary>
    /// <remarks>
    /// 41292 days over 499 is 82, which is more than a month and less than a year, so the unit is
    /// months and the count is <c>floor(82 / 31)</c>. Nothing narrows the 499 for a date axis —
    /// see <see cref="ChartDateScale"/> — so an axis with room for eight labels still generates
    /// six hundred ticks and lets the label ladder thin them.
    /// </remarks>
    [Fact]
    public void TheMajorIntervalIsTwoMonthsOnTheCorpusDateAxis()
    {
        ChartDateAxis axis = ChartDateScale.Resolve(
            LogbookCategories(blanksAsZero: true), DayMonthYear)!;

        axis.MajorInterval.ShouldBe(new ChartTimeInterval(2, ChartTimeUnit.Month));
        axis.TimeResolution.ShouldBe(ChartTimeUnit.Day);
        axis.Ticks.Count.ShouldBe(679);
    }

    /// <summary>
    /// Every eighteenth tick is a label the reference draws, and they read exactly as it draws
    /// them.
    /// </summary>
    /// <remarks>
    /// This is the whole diagnosis in one assertion. The reference's 38 labels are
    /// <c>30/12/99</c> then <c>02/01/03</c>, <c>02/01/06</c> … — three years apart but not three
    /// years from the first, and always on the 2nd, because 30 December plus two months is
    /// 30 February and LibreOffice's date normalisation rolls that over into 2 March rather than
    /// clamping it to the 28th.
    /// </remarks>
    [Fact]
    public void EveryEighteenthTickIsALabelTheReferenceDraws()
    {
        ChartDateAxis axis = ChartDateScale.Resolve(
            LogbookCategories(blanksAsZero: true), DayMonthYear)!;

        List<string> shown = [];
        for (int at = 0; at < axis.Ticks.Count; at += 18) shown.Add(axis.LabelOf(axis.Ticks[at]));

        shown.Count.ShouldBe(38);
        shown[0].ShouldBe("31/12/99");   // see ATickIsWrittenThroughTheAxisFormatAsTheSerialItIs
        shown[1].ShouldBe("02/01/03");
        shown[2].ShouldBe("02/01/06");
        shown[^1].ShouldBe("02/01/11");
    }

    /// <summary>A tick is written through the axis' format as the bare serial it is.</summary>
    /// <remarks>
    /// <para>
    /// The expectations below are <em>ours</em> and not the reference's for serials under 61, and
    /// the difference is a defect in <see cref="XRay.Office.Core.Numbers.SpreadsheetDate"/> rather
    /// than in this class. LibreOffice 26.2.4.2 reads serial 0 as 30 December 1899, 1 as
    /// 31 December and 60 as 28 February 1900 — measured in both <c>.xls</c> and <c>.xlsx</c> —
    /// where the shared converter reproduces Excel's phantom 29 February and answers one day
    /// later. Pinned here so that fixing the converter shows up as this test failing rather than
    /// as a silent change to an axis.
    /// </para>
    /// <para>
    /// It costs the corpus's date axis exactly one label: its first tick is serial 0.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData(0.0, "31/12/99")]
    [InlineData(2.0, "02/01/00")]
    [InlineData(61.0, "01/03/00")]
    [InlineData(1098.0, "02/01/03")]
    public void ATickIsWrittenThroughTheAxisFormatAsTheSerialItIs(double tick, string expected)
        => ChartDateScale.Label(tick, DayMonthYear).ShouldBe(expected);

    /// <summary>Adding months rolls the day over into the next month rather than clamping it.</summary>
    /// <remarks>
    /// <c>comphelper::date::normalize</c> subtracts the month's length and carries. 1900 is not a
    /// leap year, so 30 February 1900 is 2 March; 1904 is, so it is 1 March there. .NET's own
    /// <c>AddMonths</c> answers 28 and 29 February, and using it puts every tick on this axis in
    /// the wrong place.
    /// </remarks>
    [Theory]
    [InlineData(1899, 12, 30, 2, 1900, 3, 2)]
    [InlineData(1903, 12, 30, 2, 1904, 3, 1)]
    [InlineData(1900, 1, 31, 1, 1900, 3, 3)]
    [InlineData(1900, 3, 15, 12, 1901, 3, 15)]
    [InlineData(1900, 3, 15, -3, 1899, 12, 15)]
    public void AddingMonthsRollsOverRatherThanClamping(
        int year, int month, int day, int months, int wantYear, int wantMonth, int wantDay)
    {
        ChartDateScale.AddMonths(new DateOnly(year, month, day), months)
            .ShouldBe(new DateOnly(wantYear, wantMonth, wantDay));
    }

    /// <summary>29 February plus a year is 1 March, not 28 February.</summary>
    [Fact]
    public void AddingYearsRollsOverRatherThanClamping()
        => ChartDateScale.AddYears(new DateOnly(1904, 2, 29), 1)
            .ShouldBe(new DateOnly(1905, 3, 1));

    /// <summary>
    /// The resolution is the finest unit two consecutive dates share.
    /// </summary>
    /// <remarks>
    /// <c>VSeriesPlotter::calculateTimeResolutionOnXAxis</c>, and the dates are sorted before it
    /// runs — a column in any order gives the same answer.
    /// </remarks>
    [Fact]
    public void TheResolutionIsTheFinestUnitTwoDatesShare()
    {
        // 2003-01-01, 2005-01-01, 2007-01-01: no two in one year.
        ChartDateScale.ResolveTimeResolution([37622.0, 38353.0, 39083.0])
            .ShouldBe(ChartTimeUnit.Year);

        // Two in 2003, in different months.
        ChartDateScale.ResolveTimeResolution([37622.0, 37712.0, 39083.0])
            .ShouldBe(ChartTimeUnit.Month);

        // Two in the same month.
        ChartDateScale.ResolveTimeResolution([37622.0, 37623.0, 39083.0])
            .ShouldBe(ChartTimeUnit.Day);
    }

    /// <summary>A month resolution snaps both limits to the first of their month.</summary>
    /// <remarks><c>ScaleAutomatism.cxx:570-582</c>.</remarks>
    [Fact]
    public void AMonthResolutionSnapsTheLimitsToTheFirstOfTheMonth()
    {
        ChartDateAxis axis = ChartDateScale.Resolve(
            [37935.0, 38353.0], DayMonthYear, statedResolution: ChartTimeUnit.Month)!;

        ChartDateScale.Label(axis.Minimum, DayMonthYear).ShouldBe("01/11/03");
        ChartDateScale.Label(axis.Maximum, DayMonthYear).ShouldBe("01/01/05");
    }

    /// <summary>An axis whose categories hold no numbers at all is not a date axis.</summary>
    [Fact]
    public void NoValuesMeansNoAxis()
        => ChartDateScale.Resolve([null, null, null], DayMonthYear).ShouldBeNull();

    /// <summary>A stated interval is honoured while it does not ask for too many ticks.</summary>
    [Fact]
    public void AStatedIntervalIsHonoured()
    {
        ChartDateAxis axis = ChartDateScale.Resolve(
            [37622.0, 39083.0], DayMonthYear,
            statedInterval: new ChartTimeInterval(1, ChartTimeUnit.Year))!;

        axis.MajorInterval.ShouldBe(new ChartTimeInterval(1, ChartTimeUnit.Year));
        axis.Ticks.Count.ShouldBe(5);
    }

    /// <summary>
    /// <c>055_Project_timeline_with_milestones</c>' axis is fifteen ticks ten days apart, and its
    /// gate shortfall is not a scaling question at all.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The register carried this document for two rounds as a date axis whose <em>range and
    /// step</em> differ from the reference's — 26.2.4.2 draws 38 labels at 30-day steps over
    /// three years where this tree draws fifteen at ten days. It is neither. Its DATE column is
    /// thirteen volatile <c>DATE(YEAR(TODAY()),m,d)</c>, and the reference recalculates them on
    /// load while we render the values the file was saved with.
    /// </para>
    /// <para>
    /// <strong>Measured by freezing them.</strong> With the <c>&lt;f&gt;</c> elements of C20:C32
    /// alone removed — at the cached 2023 serials <em>and</em> at the 2026 serials the reference
    /// itself recalculates to, so that the freeze and not the year is the variable — 26.2.4.2
    /// draws exactly the fifteen upright labels below, <c>5 Apr</c> to <c>23 Aug</c>, and so does
    /// this tree. `probes/chart-axis-r112` §1.
    /// </para>
    /// <para>
    /// So this asserts the *reference's own answer for the data it is given*, which is what a
    /// scaling test can assert about a volatile document. The 38-label axis the corpus reference
    /// draws is the union of the import-time cache and the recalculated cells and cannot be
    /// reproduced without evaluating <c>TODAY()</c>.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheProjectTimelineAxisIsFifteenTicksTenDaysApart()
    {
        double?[] dates =
        [
            45021, 45040, 45040, 45047, 45061, 45061, 45092,
            45107, 45122, 45137, 45149, 45161, 45169, null, null, null, null,
        ];

        ChartDateAxis axis = ChartDateScale.Resolve(
            dates,
            NumberFormatCode.Parse("[$-409]d mmm;@"),
            statedInterval: new ChartTimeInterval(10, ChartTimeUnit.Day),
            statedResolution: ChartTimeUnit.Day)!;

        axis.MajorInterval.ShouldBe(new ChartTimeInterval(10, ChartTimeUnit.Day));
        axis.Ticks.Count.ShouldBe(15);
        axis.LabelOf(axis.Ticks[0]).ShouldBe("5 Apr");
        axis.LabelOf(axis.Ticks[^1]).ShouldBe("23 Aug");
    }

    /// <summary>A category with no value has nowhere to be, and says so.</summary>
    [Fact]
    public void ACategoryWithNoValueHasNoFraction()
    {
        ChartDateAxis axis = ChartDateScale.Resolve([37622.0, null, 39083.0], DayMonthYear)!;

        axis.FractionOf(0).ShouldBe(0.0);
        axis.FractionOf(1).ShouldBeNull();
        axis.FractionOf(2)!.Value.ShouldBe(1.0, 1e-9);
    }
}
