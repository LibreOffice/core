using XRay.Office.Core.Charts;
using Shouldly;
using static XRay.Office.Spreadsheets.Tests.BiffChartFixture;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// What a chart substream's <em>second</em> <c>CHAXESSET</c> means, and the records that route a
/// series to it.
/// </summary>
/// <remarks>
/// <para>
/// <strong>A BIFF chart states its structure three records deep and this reader used to read
/// none of it.</strong> <c>CHAXESSET</c> says which of the two axes sets is open,
/// <c>CHTYPEGROUP</c> says which group is open and therefore which set it belongs to, and a
/// series' <c>CHSERGROUP</c> names its group — so a value axis, a chart type and a scale are all
/// properties of a group rather than of the chart. Reading a <c>CHVALUERANGE</c> into one field
/// leaves the chart drawn against whichever axes set the file happened to write last.
/// </para>
/// <para>
/// <strong>Measured on <c>014_Contextures_chart_sample_991ecfc5.xls</c>, the corpus's one chart
/// substream that states two axes sets.</strong> Its primary value axis says 0…800 in steps of
/// 80 and its secondary says 0…1.01 in steps of 0.1; drawn against the second, its currency ticks
/// read <c>$1 $1 $1 $1 $1 $1 $0 $0 $0 $0 $0</c> and its bars ran three hundred plot-heights off
/// the top of the page. See <c>probes/chart-secaxis/results.md</c>.
/// </para>
/// <para>
/// Synthetic, and it has to be: the corpus holds exactly one file of this shape, so only a
/// fixture that can move one record between two groups shows that the position is what decides.
/// </para>
/// </remarks>
public sealed class XlsChartAxesSetTests
{
    /// <summary>
    /// The primary axes set's value range survives a secondary axes set stating its own.
    /// </summary>
    /// <remarks>
    /// The two ranges are written in the order BIFF writes them — primary first — so a reader
    /// that keeps one field answers the <em>secondary</em>'s numbers for both. That is the whole
    /// of the Pareto chart's axis defect.
    /// </remarks>
    [Fact]
    public void ASecondaryAxesSetDoesNotOverwriteThePrimaryScale()
    {
        ChartPlot plot = TwoAxesSets();

        plot.ValueScale.Minimum.ShouldBe(0.0);
        plot.ValueScale.Maximum.ShouldBe(800.0);
        plot.ValueScale.MajorUnit.ShouldBe(80.0);

        plot.SecondaryValueScale.ShouldNotBeNull();
        plot.SecondaryValueScale!.Value.Maximum.ShouldBe(1.01);
        plot.SecondaryValueScale!.Value.MajorUnit.ShouldBe(0.1);
    }

    /// <summary>
    /// A series is measured against the axes set of the type group its <c>CHSERGROUP</c> names.
    /// </summary>
    /// <remarks>
    /// Not against the axes set the series is written inside — a <c>CHSERIES</c> sits directly
    /// under <c>CHCHART</c>, before either <c>CHAXESSET</c> is opened, so the group index is the
    /// only statement there is. <c>XclImpChChart::GetTypeGroup</c> looks it up in the primary set
    /// and then in the secondary (<c>sc/source/filter/excel/xichart.cxx</c>:3948-3954).
    /// </remarks>
    [Fact]
    public void ASeriesGroupPutsItsSeriesOnThatGroupsAxis()
    {
        ChartPlot plot = TwoAxesSets();

        plot.Series.Count.ShouldBe(2);
        plot.Series[0].AxisIndex.ShouldBe(0);
        plot.Series[1].AxisIndex.ShouldBe(1);
        plot.HasSecondaryAxis.ShouldBeTrue();
    }

    /// <summary>Each type group's own type record decides how its series are drawn.</summary>
    /// <remarks>
    /// The chart's own kind is still the first group's, which is what the axes are built for;
    /// the second group's series carry an override. Before this the second group's type record
    /// was dropped and its series were drawn as bars — and the Pareto chart's cumulative line
    /// states <c>EXC_PATT_NONE</c>, so those bars filled nothing at all.
    /// </remarks>
    [Fact]
    public void ASeriesTakesItsOwnTypeGroupsKind()
    {
        ChartPlot plot = TwoAxesSets();

        plot.Kind.ShouldBe(ChartPlotKind.Bar);
        plot.Series[0].Kind.ShouldBeNull();
        plot.Series[1].Kind.ShouldBe(ChartPlotKind.Line);
    }

    /// <summary>A chart with one axes set states no secondary scale, whatever its series say.</summary>
    /// <remarks>
    /// The control on the two cases above: the same series records with the second axes set
    /// removed must leave the model exactly as it was before any of this was read.
    /// </remarks>
    [Fact]
    public void OneAxesSetLeavesNoSecondaryScale()
    {
        ChartPlot plot = Chart(
            Substream(
            [
                .. Series(group: 0),
                .. Group(ChAxesSet, AxesSetHeader(0),
                    ValueAxis(ValueRange(0.0, 800.0, 80.0)),
                    Group(ChTypeGroup, TypeGroupHeader(0), BarRecord())),
            ]),
            withData: true);

        plot.ValueScale.Maximum.ShouldBe(800.0);
        plot.SecondaryValueScale.ShouldBeNull();
        plot.HasSecondaryAxis.ShouldBeFalse();
        plot.Series[0].AxisIndex.ShouldBe(0);
        plot.Series[0].Kind.ShouldBeNull();
    }

    /// <summary>
    /// <c>CHBAR</c>'s gap is a gap and its overlap is negated on the way to the model.
    /// </summary>
    /// <remarks>
    /// <c>XclImpChType::CreateChartType</c> hands chart2 <c>-mnOverlap</c> and <c>mnGap</c>
    /// (<c>xichart.cxx</c>:2404-2410) where <c>oox</c> hands it <c>c:overlap</c> unchanged
    /// (<c>typegroupconverter.cxx</c>:459-461), so the two formats count an overlap the opposite
    /// way round. Both fields were being skipped past, which drew every BIFF bar chart at the
    /// model's own 100 % gap: on the Pareto chart the file says nought and the reference's seven
    /// bars touch.
    /// </remarks>
    [Fact]
    public void ABarGroupStatesItsGapAndItsOverlap()
    {
        ChartPlot none = Chart(
            Substream([.. Group(ChTypeGroup, TypeGroupHeader(0), BarRecord(overlap: 0, gap: 0))]));

        none.GapWidth.ShouldBe(0.0);
        none.Overlap.ShouldBe(0.0);

        ChartPlot stacked = Chart(
            Substream(
                [.. Group(ChTypeGroup, TypeGroupHeader(0), BarRecord(overlap: 100, gap: 150))]));

        stacked.GapWidth.ShouldBe(150.0);
        stacked.Overlap.ShouldBe(-100.0);
    }

    /// <summary>A chart stating no <c>CHBAR</c> keeps the model's own gap and overlap.</summary>
    [Fact]
    public void AChartWithNoBarGroupKeepsTheDefaultGap()
    {
        ChartPlot plot = Chart(Substream([.. Series(group: 0)]), withData: true);

        plot.GapWidth.ShouldBe(new ChartPlot().GapWidth);
        plot.Overlap.ShouldBe(new ChartPlot().Overlap);
    }

    /// <summary>
    /// <c>SHOWVISIBLEONLY</c> drops a hidden cell out of the series rather than blanking it.
    /// </summary>
    /// <remarks>
    /// <c>ScChart2DataSequence::BuildDataCache</c> <c>continue</c>s past a hidden row
    /// (<c>sc/source/ui/unoobj/chart2uno.cxx</c>:2636-2646), so the points after it move up and
    /// the series is shorter. The flag comes from <c>CHPROPERTIES</c>, negated into the diagram's
    /// <c>IncludeHiddenCells</c> (<c>xichart.cxx</c>:4027-4028).
    /// </remarks>
    [Fact]
    public void AHiddenRowIsLeftOutWhenTheChartPlotsVisibleCellsOnly()
    {
        ChartPlot visible = Chart(
            Substream([.. Record(ChProperties, [.. Word(0x0002), 0, 0]), .. AreaSeries()]),
            withData: true,
            hiddenRow: true);

        visible.Series[0].Values.Count.ShouldBe(1);
        visible.Series[0].Values[0].ShouldBe(42.0);
    }

    /// <summary>
    /// The same file without the flag keeps both cells, and so does one whose rows are shown.
    /// </summary>
    /// <remarks>
    /// Two controls in one case, because each on its own could pass for the wrong reason: the
    /// first shows the flag is what decides and the second shows the <c>ROW</c> record is.
    /// </remarks>
    [Fact]
    public void AHiddenRowIsKeptWithoutTheFlagAndAShownRowIsAlwaysKept()
    {
        ChartPlot unflagged = Chart(
            Substream([.. Record(ChProperties, [.. Word(0x0000), 0, 0]), .. AreaSeries()]),
            withData: true,
            hiddenRow: true);

        unflagged.Series[0].Values.Count.ShouldBe(2);

        ChartPlot shown = Chart(
            Substream([.. Record(ChProperties, [.. Word(0x0002), 0, 0]), .. AreaSeries()]),
            withData: true,
            hiddenRow: false);

        shown.Series[0].Values.Count.ShouldBe(2);
        shown.Series[0].Values[1].ShouldBe(7.0);
    }

    /// <summary>
    /// A Pareto chart's shape: a bar group on the primary axes set and a line group on the
    /// secondary, with a series on each.
    /// </summary>
    private static ChartPlot TwoAxesSets() => Chart(
        Substream(
        [
            .. Series(group: 0),
            .. Series(group: 1),
            .. Group(ChAxesSet, AxesSetHeader(0),
                ValueAxis(ValueRange(0.0, 800.0, 80.0)),
                Group(ChTypeGroup, TypeGroupHeader(0), BarRecord())),
            .. Group(ChAxesSet, AxesSetHeader(1),
                ValueAxis(ValueRange(0.0, 1.01, 0.1)),
                Group(ChTypeGroup, TypeGroupHeader(1), Record(ChLine, Word(0)))),
        ]),
        withData: true);

    /// <summary>One series whose value link resolves, naming the type group it belongs to.</summary>
    private static byte[] Series(int group)
        => Group(ChSeries, new byte[8], SeriesLink(), Record(ChSeriesGroup, Word((ushort)group)));

    /// <summary>One series whose values are the rectangle A1:A2 rather than the cell A1.</summary>
    private static byte[] AreaSeries()
        => Group(ChSeries, new byte[8], SeriesAreaLink(), Record(ChSeriesGroup, Word(0)));

    /// <summary>A <c>CHAXESSET</c> header: its id, then the rectangle nothing here reads.</summary>
    private static byte[] AxesSetHeader(ushort id) => [.. Word(id), .. new byte[16]];

    /// <summary>
    /// A <c>CHTYPEGROUP</c> header: sixteen ignored bytes, the flags, then the group index.
    /// </summary>
    private static byte[] TypeGroupHeader(ushort index)
        => [.. new byte[16], .. Word(0), .. Word(index)];

    /// <summary>A <c>CHAXIS</c> of the value dimension holding the records given.</summary>
    private static byte[] ValueAxis(params byte[][] children)
        => Group(ChAxis, [.. Word(1), .. new byte[16]], children);

    /// <summary>A <c>CHVALUERANGE</c> stating a minimum, a maximum and a major step.</summary>
    /// <remarks>
    /// The flags are zero, which means every one of the three is stated rather than automatic —
    /// the automatic bits are 0x0001, 0x0002 and 0x0004.
    /// </remarks>
    private static byte[] ValueRange(double minimum, double maximum, double major) =>
        Record(ChValueRange,
        [
            .. BitConverter.GetBytes(minimum),
            .. BitConverter.GetBytes(maximum),
            .. BitConverter.GetBytes(major),
            .. new byte[16],
            .. Word(0),
        ]);

    /// <summary>A <c>CHBAR</c>: an overlap, a gap, then the flags that say column or bar.</summary>
    private static byte[] BarRecord(short overlap = 0, short gap = 0) => Record(ChBar,
    [
        .. BitConverter.GetBytes(overlap),
        .. BitConverter.GetBytes(gap),
        .. Word(0),
    ]);
}
