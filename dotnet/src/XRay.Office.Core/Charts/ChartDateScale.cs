using XRay.Office.Core.Numbers;

namespace XRay.Office.Core.Charts;

/// <summary>The unit a date axis counts its ticks in.</summary>
/// <remarks>
/// <c>css::chart::TimeUnit</c>, whose three values are ordered coarsest-last and are compared as
/// numbers by <c>ScaleAutomatism</c> — <c>MajorTimeInterval.TimeUnit &lt; TimeResolution</c> raises
/// the interval to the resolution — so the numbering is part of the contract, not a detail.
/// </remarks>
public enum ChartTimeUnit
{
    /// <summary>One day.</summary>
    Day = 0,

    /// <summary>One calendar month.</summary>
    Month = 1,

    /// <summary>One calendar year.</summary>
    Year = 2,
}

/// <summary>How far apart a date axis' ticks are: a count and the unit it counts.</summary>
/// <param name="Number">How many of the unit, always at least one.</param>
/// <param name="Unit">Which unit.</param>
public readonly record struct ChartTimeInterval(int Number, ChartTimeUnit Unit);

/// <summary>
/// A category axis that is a <em>date</em> axis: a continuous serial-number scale with calendar
/// ticks on it, rather than a run of equal category slots.
/// </summary>
/// <remarks>
/// <para>
/// The distinction is not cosmetic. On a category axis the <em>n</em>th point sits at ordinal
/// <em>n</em> whatever its date is; on a date axis it sits where its date falls between the axis'
/// two ends, so a series clustered at one end of a long range is drawn clustered. Measured on
/// <c>Template Pilot Logbook JAR-FCL V3.0.xls</c>, whose 799 categories hold 25 dates: on the
/// category axis its points are at 75–78% of the plot width and on the date axis at 91.8–100%,
/// and the reference draws the second.
/// </para>
/// <para>
/// Resolved once, by the reader, from what the file states and what the category cells hold —
/// none of it depends on the geometry, so <see cref="ChartLayout"/> gets an answer rather than
/// an algorithm.
/// </para>
/// </remarks>
/// <param name="Minimum">The serial the axis starts at.</param>
/// <param name="Maximum">The serial it ends at.</param>
/// <param name="TimeResolution">The finest unit the data distinguishes.</param>
/// <param name="MajorInterval">The distance between major ticks.</param>
/// <param name="Ticks">Every major tick's serial, in order, the first at <paramref name="Minimum"/>.</param>
/// <param name="CategoryValues">
/// Each category's serial, or null where the category has no value and the chart leaves a gap.
/// </param>
/// <param name="Format">The number format the tick labels are written through.</param>
/// <param name="ShiftedCategories">
/// Whether a category occupies the unit <em>after</em> its own date rather than straddling it —
/// <c>ScaleData.ShiftedCategoryPosition</c>, which OOXML states as the crossing value axis'
/// <c>c:crossBetween</c>. It also decides whether a point exactly at
/// <paramref name="Maximum"/> is drawn: <c>isStrongLowerRequested</c> makes the upper bound
/// strict for a shifted axis (<c>chart2/source/view/inc/PlottingPositionHelper.hxx</c>:321-338).
/// </param>
public sealed record ChartDateAxis(
    double Minimum,
    double Maximum,
    ChartTimeUnit TimeResolution,
    ChartTimeInterval MajorInterval,
    IReadOnlyList<double> Ticks,
    IReadOnlyList<double?> CategoryValues,
    NumberFormatCode? Format,
    bool ShiftedCategories = false)
{
    /// <summary>The span the axis covers, never zero.</summary>
    public double Span => Maximum - Minimum == 0.0 ? 1.0 : Maximum - Minimum;

    /// <summary>Where a serial sits along the axis, 0 at its start and 1 at its end.</summary>
    /// <remarks>
    /// <para>
    /// <strong>A date axis is linear in its own resolution unit and not in days.</strong>
    /// <c>DateScaling::doScaling</c> (<c>chart2/source/view/axes/DateScaling.cxx</c>:56-91, this
    /// tree) returns the serial itself only at <see cref="ChartTimeUnit.Day"/>; at
    /// <see cref="ChartTimeUnit.Month"/> and <see cref="ChartTimeUnit.Year"/> it returns
    /// <c>year × 12 + month</c> plus the fraction of the month elapsed, under the comment
    /// <em>"assuming equal count of months in each year"</em>. Months are not of equal length, so
    /// the two mappings genuinely differ.
    /// </para>
    /// <para>
    /// <strong>How much depends on the span, which is why it hid.</strong> On
    /// <c>171128IPAP.pptx</c>, 132 months, the difference is under a tenth of a percent and
    /// invisible. On <c>044_Cash_flow_forecast</c>, twelve monthly points over eleven months,
    /// 26.2.4.2 spaces the bars at a flat <strong>35.26 pt</strong> and days would give 30.2 to
    /// 33.4 — measured on the reference's own rendering of the file and of a variant whose axis
    /// states its range explicitly.
    /// </para>
    /// </remarks>
    public double Fraction(double value)
    {
        double from = Units(Minimum);
        double span = Units(Maximum) - from;

        return span == 0.0 ? 0.0 : (Units(value) - from) / span;
    }

    /// <summary>
    /// Where the category at <paramref name="index"/> sits, or null when it has no value.
    /// </summary>
    /// <remarks>
    /// A null is a genuine gap in the domain, and a caller drawing a polyline has to break it
    /// there — exactly as it breaks on a missing Y.
    /// </remarks>
    public double? FractionOf(int index)
        => index >= 0 && index < CategoryValues.Count && CategoryValues[index] is { } value
            ? Fraction(value)
            : null;

    /// <summary>The label a tick carries.</summary>
    public string LabelOf(double tick) => ChartDateScale.Label(tick, Format);

    /// <summary>
    /// One category's width on this axis, as a fraction of its whole span.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A date axis' category is <strong>one unit of its own time resolution</strong>, not one
    /// n-th of the points: <c>PlottingPositionHelper::setTimeResolution</c> sets the category
    /// width to 1 and, at <see cref="ChartTimeUnit.Year"/>, to 12
    /// (<c>chart2/source/view/main/PlottingPositionHelper.cxx</c>:670-690, this tree) — twelve
    /// because the axis' own scaling counts months, under that function's comment
    /// <em>"assuming equal count of months in each year"</em>. Both cases are therefore one
    /// resolution unit, and this is that unit divided by the span measured in the same unit.
    /// </para>
    /// <para>
    /// Measured on <c>171128IPAP.pptx</c> page 40, whose axis states
    /// <c>c:min="39814"</c> and <c>c:max="43831"</c> — 132 months — and whose bars 26.2.4.2 draws
    /// <strong>2.21 pt wide on a 288 pt plot</strong>, which is 288/132 = 2.18 at the chart's own
    /// gap width. A bar one n-th of 145 points wide would be 1.99 and would be drawn at the wrong
    /// place besides.
    /// </para>
    /// </remarks>
    public double SlotFraction
    {
        get
        {
            double units = Units(Maximum) - Units(Minimum);
            return units > 0.0 ? 1.0 / units : 1.0;
        }
    }

    /// <summary>The axis' span, in its own resolution unit.</summary>
    internal double UnitSpan => Units(Maximum) - Units(Minimum);

    /// <summary>
    /// Where a point's own date puts the start of its category, or null when it is off the axis.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>BarChart::createShapes</c> rasterises the point's x to the axis' resolution and then
    /// drops it outright when it falls outside the axis' range — three consecutive
    /// <c>continue</c>s, before any geometry
    /// (<c>chart2/source/view/charttypes/BarChart.cxx</c>:694-704). A point on the axis is placed
    /// at that rasterised date and its category is centred there
    /// (<c>CategoryPositionHelper::getScaledSlotPos</c>), which is the difference between a date
    /// axis and an index one.
    /// </para>
    /// <para>
    /// <strong>Where the category sits relative to the point was solved from the reference's own
    /// renderings, not derived.</strong> <c>CategoryPositionHelper::getScaledSlotPos</c>
    /// subtracts half a category width unconditionally, but on a shifted axis something adds it
    /// back — <c>AllowShiftXAxisPos</c> and <c>isStrongLowerRequested</c> are the territory and
    /// the exact line is not pinned here. What is measured is the result:
    /// <c>044_Cash_flow_forecast</c> rendered by 26.2.4.2 three times — as authored, with its
    /// range stated as the data's own, and with a month's margin either side — puts the plot's
    /// left edge at <strong>414.31 pt in all three</strong> under the leading-edge reading and
    /// at two different values under the centred one, and its first bar lands within
    /// <strong>0.02 pt</strong> of the leading-edge prediction. <c>171128IPAP.pptx</c>'s page-40
    /// chart states <c>midCat</c> instead and is centred, its bars agreeing to 0.06 pt.
    /// </para>
    /// <para>
    /// The rasterisation is <c>DateHelper::RasterizeDateValue</c>
    /// (<c>chart2/source/view/axes/DateHelper.cxx</c>:69-89): at
    /// <see cref="ChartTimeUnit.Month"/> the day of month becomes 1 and at
    /// <see cref="ChartTimeUnit.Year"/> the month does too. It is not cosmetic — a quarterly
    /// series whose dates are month-ends would otherwise sit most of a category out of place.
    /// </para>
    /// </remarks>
    /// <param name="serial">The point's own date, in the workbook's serial numbering.</param>
    /// <returns>Where the category's leading edge sits, 0 at the axis' start and 1 at its end.</returns>
    public double? SlotStart(double serial)
    {
        if (!double.IsFinite(serial)) return null;

        double rasterised = ChartDateScale.Rasterise(serial, TimeResolution);
        if (rasterised < Minimum) return null;

        // A shifted axis excludes a point sitting exactly on its maximum, because that maximum
        // is the extra interval the shift added and nothing belongs in it.
        if (ShiftedCategories ? rasterised >= Maximum : rasterised > Maximum) return null;

        double at = Fraction(rasterised);

        // A shifted category runs from its own date; an unshifted one straddles it. Measured on
        // 26.2.4.2 rather than derived — see the remarks on this member.
        return ShiftedCategories ? at : at - (SlotFraction / 2.0);
    }

    /// <summary>A serial expressed in the axis' own resolution unit, as the scaling counts it.</summary>
    /// <remarks>
    /// <c>DateScaling::doScaling</c> (<c>chart2/source/view/axes/DateScaling.cxx</c>:56-91): a day
    /// axis counts days, and a month or year axis counts <c>year × 12 + month</c> plus the
    /// fraction of the month elapsed. A year axis counts months too and takes its width as
    /// twelve of them, which is why this divides by twelve rather than counting years.
    /// </remarks>
    private double Units(double serial)
    {
        if (TimeResolution == ChartTimeUnit.Day) return serial;

        double months = ChartDateScale.Months(serial);
        return TimeResolution == ChartTimeUnit.Year ? months / 12.0 : months;
    }
}

/// <summary>
/// Resolves a date axis' scale and its ticks — a port of LibreOffice's
/// <c>ScaleAutomatism::calculateExplicitIncrementAndScaleForDateTimeAxis</c> and
/// <c>DateTickFactory::getAllTicks</c>.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Everything here was established by measuring the installed 26.2.4.2, not by reading the
/// tree</strong>, and three of the four rules would not have been guessed. On the corpus's one
/// date-axis workbook the reference draws 38 labels reading <c>30/12/99</c>, <c>02/01/03</c>,
/// <c>02/01/06</c> … — a first tick at serial <em>zero</em> on data that starts at 37935, an
/// apparent three-year step that is not a whole number of years from the first label, and a day
/// of month that is 02 forever. All three fall out of the same three rules:
/// </para>
/// <list type="number">
/// <item><description>
/// <strong>A date axis is never told how much room it has.</strong>
/// <c>VCoordinateSystem::prepareAutomaticAxisScaling</c> returns early for a date X axis, before
/// the call that would narrow <c>m_nMaximumAutoMainIncrementCount</c> from its constructed value,
/// and that value for an axis of type DATE is <c>MAXIMUM_MANUAL_INCREMENT_COUNT</c> =
/// <see cref="MaximumAutoIntervalCount"/>. So the interval is always
/// <c>(days spanned) / 499</c> — 82 days on that workbook, which is
/// <see cref="ChartTimeUnit.Month"/> and <c>floor(82/31)</c> = 2 of them.
/// </description></item>
/// <item><description>
/// <strong>Ticks are calendar additions with roll-over, not clamping.</strong>
/// <c>comphelper::date::normalize</c> turns 30 February 1900 into 2 <em>March</em> — it subtracts
/// the month's length and carries — where every modern date library clamps to the 28th. That one
/// difference is the whole of the day-02 pattern, and .NET's <c>DateOnly.AddMonths</c> gives the
/// other answer, which is why <see cref="AddMonths"/> exists.
/// </description></item>
/// <item><description>
/// <strong>The thinning is not here.</strong> All 679 ticks are generated and
/// <see cref="ChartAxisLabels"/>' collision ladder decides that every 18th carries a label. An
/// axis that produced 38 ticks directly would put them in the wrong places, because 18 two-month
/// steps from 30 December 1899 is 2 January 1903 and three years is not.
/// </description></item>
/// </list>
/// <para>
/// <strong>What decides the axis <em>minimum</em> is a plotter, not the scale.</strong>
/// <c>AreaChart::addSeries</c> (<c>chart2/source/view/charttypes/AreaChart.cxx:136-143</c>)
/// silently promotes a series' <c>LEAVE_GAP</c> to <c>USE_ZERO</c> for any <em>area</em> chart, so
/// a blank category cell counts as serial 0 and drags the axis back to 30 December 1899. Measured
/// by single-variable probes: the same 799 categories as a line chart, as a bar chart, or with
/// <c>dispBlanksAs="span"</c>, take the data minimum instead. That resolution belongs to the
/// reader, which knows the chart's kind; this class is handed the values it should use.
/// </para>
/// </remarks>
public static class ChartDateScale
{
    /// <summary>
    /// The most automatic intervals a date axis is ever divided into.
    /// </summary>
    /// <remarks>
    /// <c>MAXIMUM_MANUAL_INCREMENT_COUNT</c> (<c>ScaleAutomatism.cxx:39</c>), which
    /// <c>lcl_getMaximumAutoIncrementCount</c> returns for a DATE axis where every other type gets
    /// ten. It is used unreduced because nothing narrows it — see the remarks on this class.
    /// </remarks>
    public const int MaximumAutoIntervalCount = 500;

    /// <summary>How many ticks an axis is ever asked to produce.</summary>
    /// <remarks>
    /// The interval is chosen against <see cref="MaximumAutoIntervalCount"/> but the floor in
    /// <c>floor(intervalDays / daysPerUnit)</c> can halve it, so twice that count is reachable and
    /// is not a defect. The cap is a guard against a corrupt minimum and maximum spanning
    /// millennia at a one-day step, not a rule of LibreOffice's.
    /// </remarks>
    public const int MaximumTickCount = 4000;

    /// <summary>Calc's null date: serial 0 is 30 December 1899.</summary>
    private static readonly DateOnly Null1900 = new(1899, 12, 30);

    /// <summary>The 1904 system's null date.</summary>
    private static readonly DateOnly Null1904 = new(1904, 1, 1);

    /// <summary>The serial a date sits at in a workbook's own numbering.</summary>
    public static double SerialOf(DateOnly date, SpreadsheetDateSystem system)
        => date.DayNumber - NullDateOf(system).DayNumber;

    /// <summary>The date a serial names, counting plain days from the null date.</summary>
    public static DateOnly DateOf(double serial, SpreadsheetDateSystem system)
    {
        double days = Math.Floor(serial);
        DateOnly origin = NullDateOf(system);
        double number = origin.DayNumber + days;

        if (number < DateOnly.MinValue.DayNumber) return DateOnly.MinValue;
        if (number > DateOnly.MaxValue.DayNumber) return DateOnly.MaxValue;

        return DateOnly.FromDayNumber((int)number);
    }

    /// <summary>
    /// The text a tick carries, written through the axis' number format.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The raw serial goes through, which is what <c>FixedNumberFormatter::getFormattedString</c>
    /// is handed. It matters that no correction is applied here: the axis' first tick and the
    /// cells it came from are the same number, and a nudge in one place and not the other would
    /// make one document disagree with itself.
    /// </para>
    /// <para>
    /// <strong>It leaves one label a day late, and the day belongs to a different defect.</strong>
    /// <see cref="SpreadsheetDate.FromSerial"/> reproduces Excel's phantom 29 February 1900, so it
    /// reads serial 0 as 31 December 1899; LibreOffice 26.2.4.2 has no such rule and draws
    /// 30 December, in a cell and on an axis alike — measured in both <c>.xls</c> and
    /// <c>.xlsx</c> with a workbook holding serials 0, 1, 2 and 58 to 62. That is a defect in the
    /// shared converter, it reaches every date cell below serial 61 in the corpus, and it is not
    /// this file's to fix.
    /// </para>
    /// </remarks>
    public static string Label(double tick, NumberFormatCode? format)
        => ChartDataLabel.Write(tick, format);

    /// <summary>
    /// A serial snapped back to the start of its own resolution unit.
    /// </summary>
    /// <remarks>
    /// <c>DateHelper::RasterizeDateValue</c> (<c>chart2/source/view/axes/DateHelper.cxx</c>
    /// :69-89): a day axis leaves the value alone, a month axis sets the day of month to 1, and a
    /// year axis sets the month to January as well. The 1900 null date is assumed, which is what
    /// every corpus chart uses; a 1904 workbook would shift each snap by the same four years in
    /// both directions and the axis' own minimum with it, so the placement is unaffected and only
    /// a date exactly on a month boundary could land differently.
    /// </remarks>
    public static double Rasterise(double serial, ChartTimeUnit resolution)
    {
        if (resolution == ChartTimeUnit.Day || !double.IsFinite(serial)) return serial;

        DateOnly date = DateOf(serial, SpreadsheetDateSystem.Date1900);
        DateOnly start = resolution == ChartTimeUnit.Year
            ? new DateOnly(date.Year, 1, 1)
            : new DateOnly(date.Year, date.Month, 1);

        return SerialOf(start, SpreadsheetDateSystem.Date1900);
    }

    /// <summary>
    /// A serial counted in months, as <c>DateScaling::doScaling</c> counts them.
    /// </summary>
    /// <remarks>
    /// <c>year × 12 + month</c> plus the fraction of the month elapsed
    /// (<c>chart2/source/view/axes/DateScaling.cxx</c>:68-89). The months are not of equal
    /// length, so this is not a linear function of the serial and a span measured in it is not
    /// the span in days over 30.44.
    /// </remarks>
    public static double Months(double serial)
    {
        if (!double.IsFinite(serial)) return serial;

        DateOnly date = DateOf(serial, SpreadsheetDateSystem.Date1900);
        double months = (date.Year * 12.0) + date.Month;

        return months + ((date.Day - 1.0) / DateTime.DaysInMonth(date.Year, date.Month));
    }

    /// <summary>
    /// The finest unit two of the axis' dates fall inside — LibreOffice's automatic time
    /// resolution.
    /// </summary>
    /// <remarks>
    /// <c>VSeriesPlotter::calculateTimeResolutionOnXAxis</c>
    /// (<c>chart2/source/view/charttypes/VSeriesPlotter.cxx:1617-1660</c>): start at
    /// <see cref="ChartTimeUnit.Year"/>, walk the <em>sorted</em> dates, drop to
    /// <see cref="ChartTimeUnit.Month"/> the first time two consecutive ones share a year and to
    /// <see cref="ChartTimeUnit.Day"/> the first time two share a month. The sort matters: the
    /// categories are sorted before this runs, so a column of dates in any order gives the same
    /// answer.
    /// </remarks>
    public static ChartTimeUnit ResolveTimeResolution(
        IEnumerable<double?> values, SpreadsheetDateSystem system = SpreadsheetDateSystem.Date1900)
    {
        ArgumentNullException.ThrowIfNull(values);

        List<double> dates = [];
        foreach (double? value in values)
        {
            if (value is { } serial && double.IsFinite(serial)) dates.Add(serial);
        }

        dates.Sort();
        if (dates.Count == 0) return ChartTimeUnit.Year;

        ChartTimeUnit unit = ChartTimeUnit.Year;
        DateOnly previous = DateOf(dates[0], system);

        for (int at = 1; at < dates.Count; at++)
        {
            DateOnly current = DateOf(dates[at], system);

            if (unit == ChartTimeUnit.Year && previous.Year == current.Year)
                unit = ChartTimeUnit.Month;

            if (unit == ChartTimeUnit.Month
                && previous.Year == current.Year && previous.Month == current.Month)
            {
                return ChartTimeUnit.Day;
            }

            previous = current;
        }

        return unit;
    }

    /// <summary>
    /// Resolves a date axis from the values its categories hold and what the file states.
    /// </summary>
    /// <param name="categoryValues">
    /// Each category's serial, or null where it has none. The caller has already applied the
    /// chart's missing-value treatment — see the remarks on this class for why that is not
    /// decided here.
    /// </param>
    /// <param name="format">The format the tick labels are written through.</param>
    /// <param name="statedMinimum">The stated minimum serial, or null for automatic.</param>
    /// <param name="statedMaximum">The stated maximum serial, or null for automatic.</param>
    /// <param name="statedInterval">The stated major interval, or null for automatic.</param>
    /// <param name="statedResolution">The stated base unit, or null for automatic.</param>
    /// <param name="system">The workbook's date epoch.</param>
    /// <param name="shifted">
    /// Whether the categories sit <em>between</em> the ticks rather than on them, which is
    /// <c>ScaleData.ShiftedCategoryPosition</c>. It adds one resolution unit to the axis'
    /// maximum, stated or automatic alike — <c>"for explicit scales we need one interval more
    /// (maximum excluded)"</c>, <c>ScaleAutomatism.cxx</c>:565-597 — and it does so
    /// <em>before</em> the major interval is chosen from the span, so it moves every tick too.
    /// OOXML states it as the value axis' <c>c:crossBetween</c>, which
    /// <c>AxisConverter::convertFromModel</c> reads as <c>== XML_between</c> and otherwise
    /// defaults to true for a bar, line or stock group
    /// (<c>oox/source/drawingml/chart/axisconverter.cxx</c>:292-301).
    /// <strong>That default belongs to the format reader and not here</strong>, so this
    /// parameter defaults to false: a caller that does not know keeps the geometry it had. The
    /// DrawingML reader passes the real value, fallback included; the BIFF reader does not yet,
    /// because <c>CHVALUERANGE</c>'s own crossing flag has not been measured against the
    /// reference and guessing it would move every legacy date chart on a hunch.
    /// </param>
    /// <returns>The axis, or null when no category holds a value at all.</returns>
    public static ChartDateAxis? Resolve(
        IReadOnlyList<double?> categoryValues,
        NumberFormatCode? format = null,
        double? statedMinimum = null,
        double? statedMaximum = null,
        ChartTimeInterval? statedInterval = null,
        ChartTimeUnit? statedResolution = null,
        SpreadsheetDateSystem system = SpreadsheetDateSystem.Date1900,
        bool shifted = false)
    {
        ArgumentNullException.ThrowIfNull(categoryValues);

        double dataMinimum = double.PositiveInfinity;
        double dataMaximum = double.NegativeInfinity;

        foreach (double? value in categoryValues)
        {
            if (value is not { } serial || !double.IsFinite(serial)) continue;
            if (serial < dataMinimum) dataMinimum = serial;
            if (serial > dataMaximum) dataMaximum = serial;
        }

        if (double.IsInfinity(dataMinimum)) return null;

        bool autoMinimum = statedMinimum is null;
        bool autoMaximum = statedMaximum is null;

        DateOnly minimum = DateOf(statedMinimum ?? dataMinimum, system);
        DateOnly maximum = DateOf(statedMaximum ?? dataMaximum, system);

        if (minimum > maximum) (minimum, maximum) = (maximum, minimum);

        ChartTimeUnit resolution =
            statedResolution ?? ResolveTimeResolution(categoryValues, system);

        // Snapping the limits to the resolution, and widening a range shorter than one of its own
        // units so that the axis has somewhere to put a tick (ScaleAutomatism.cxx:565-597).
        switch (resolution)
        {
            case ChartTimeUnit.Day:
                if (shifted) maximum = maximum.AddDays(1);
                break;

            case ChartTimeUnit.Month:
                minimum = new DateOnly(minimum.Year, minimum.Month, 1);
                maximum = new DateOnly(maximum.Year, maximum.Month, 1);
                if (shifted) maximum = AddMonths(maximum, 1);
                if (maximum < AddMonths(minimum, 1))
                {
                    if (autoMaximum || !autoMinimum) maximum = AddMonths(minimum, 1);
                    else minimum = AddMonths(maximum, -1);
                }

                break;

            case ChartTimeUnit.Year:
                minimum = new DateOnly(minimum.Year, 1, 1);
                maximum = new DateOnly(maximum.Year, 1, 1);
                if (shifted) maximum = AddYears(maximum, 1);
                if (maximum < AddYears(minimum, 1))
                {
                    if (autoMaximum || !autoMinimum) maximum = AddYears(minimum, 1);
                    else minimum = AddYears(maximum, -1);
                }

                break;
        }

        ChartTimeInterval interval =
            MajorIntervalOf(minimum, maximum, resolution, statedInterval);

        DateOnly origin = NullDateOf(system);
        double axisMinimum = minimum.DayNumber - origin.DayNumber;
        double axisMaximum = maximum.DayNumber - origin.DayNumber;

        List<double> ticks = [];
        DateOnly at = minimum;

        while (at <= maximum && ticks.Count < MaximumTickCount)
        {
            ticks.Add(at.DayNumber - origin.DayNumber);

            DateOnly next = interval.Unit switch
            {
                ChartTimeUnit.Day => at.AddDays(interval.Number),
                ChartTimeUnit.Year => AddYears(at, interval.Number),
                _ => AddMonths(at, interval.Number),
            };

            if (next <= at) break;
            at = next;
        }

        return new ChartDateAxis(
            axisMinimum, axisMaximum, resolution, interval, ticks, categoryValues, format,
            shifted);
    }

    /// <summary>
    /// Puts a date axis' points in date order, which is not the order the cells are in.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>AreaChart::createShapes</c> calls <c>pSeries-&gt;doSortByXValues()</c> for every series
    /// on a date category axis (<c>AreaChart.cxx:604-620</c>,
    /// <c>VDataSeries::doSortByXValues</c>), and it is not a tidiness measure: the polyline joins
    /// consecutive <em>points</em>, so a series whose categories run newest-first is drawn as a
    /// mirror image of the reference. Measured on
    /// <c>southern-classic-kennesaw-state-university-final.pptx</c>, whose 254 daily closes are
    /// stored 12 January 2017 down to 12 January 2016: the reference's axis reads
    /// <c>Jan-16 … Jan-17</c> left to right and ours read <c>Jan-17 … Jan-16</c> until this ran.
    /// </para>
    /// <para>
    /// Sorted by the reader rather than by <see cref="ChartLayout"/> because it is a property of
    /// the data and not of the geometry, and because sorting the categories, their values and
    /// every series with one permutation keeps the three indexed by the same thing — which is
    /// what every consumer of <c>ChartPlot</c> already assumes.
    /// </para>
    /// <para>
    /// <strong>Only the numbers move.</strong> A per-point label or fill is looked up on the
    /// model by index — <c>VDataSeries</c> sorts <c>m_aValues_X</c> and <c>m_aValues_Y</c> and
    /// asks <c>getDataPointProperties</c> for everything else — so <c>c:dLbl idx="253"</c> lands
    /// on the last point drawn and not on the point whose cell was 253rd. That is measurable on
    /// the same document: its three threshold labels sit at the right-hand end of the reference's
    /// plot, which is where the *newest* date is and where cell 253 is *not*.
    /// </para>
    /// <para>
    /// A category with no value keeps its place at the end, because <c>lcl_LessXOfPoint</c>
    /// compares with <c>&lt;</c> and a NaN answers false to every comparison it is in.
    /// </para>
    /// </remarks>
    /// <param name="axis">The resolved date axis, whose category values are the sort key.</param>
    /// <param name="categories">The category labels, permuted with the values.</param>
    /// <param name="series">Every series, whose values are permuted with them.</param>
    public static (ChartDateAxis Axis, IReadOnlyList<string?> Categories,
        IReadOnlyList<ChartSeries> Series) SortByDate(
        ChartDateAxis axis, IReadOnlyList<string?> categories, IReadOnlyList<ChartSeries> series)
    {
        ArgumentNullException.ThrowIfNull(axis);
        ArgumentNullException.ThrowIfNull(categories);
        ArgumentNullException.ThrowIfNull(series);

        IReadOnlyList<double?> values = axis.CategoryValues;

        int[] order = [.. Enumerable.Range(0, values.Count)];
        Array.Sort(
            [.. values.Select(value => value ?? double.MaxValue)], order);

        bool moved = false;
        for (int at = 0; at < order.Length; at++)
        {
            if (order[at] != at) { moved = true; break; }
        }

        if (!moved) return (axis, categories, series);

        double?[] sortedValues = new double?[order.Length];
        string?[] sortedCategories = new string?[order.Length];

        for (int at = 0; at < order.Length; at++)
        {
            sortedValues[at] = values[order[at]];
            sortedCategories[at] =
                order[at] < categories.Count ? categories[order[at]] : null;
        }

        List<ChartSeries> sortedSeries = [];
        foreach (ChartSeries one in series)
        {
            double?[] numbers = new double?[order.Length];
            for (int at = 0; at < order.Length; at++)
            {
                numbers[at] = order[at] < one.Values.Count ? one.Values[order[at]] : null;
            }

            sortedSeries.Add(one with { Values = numbers });
        }

        return (axis with { CategoryValues = sortedValues }, sortedCategories, sortedSeries);
    }

    /// <summary>
    /// The distance between major ticks, stated or derived.
    /// </summary>
    /// <remarks>
    /// <c>ScaleAutomatism.cxx:605-673</c>. A stated interval is honoured only while it produces no
    /// more intervals than the ceiling; past that it is discarded and the automatic rule runs, and
    /// the automatic rule works in <em>nominal</em> days — 31 to a month and 365 to a year — which
    /// is why a two-month interval over 41292 days yields 679 ticks rather than the 499 the
    /// ceiling suggests.
    /// </remarks>
    private static ChartTimeInterval MajorIntervalOf(
        DateOnly minimum, DateOnly maximum, ChartTimeUnit resolution,
        ChartTimeInterval? statedInterval)
    {
        int dayCount = maximum.DayNumber - minimum.DayNumber;
        int ceiling = MaximumAutoIntervalCount - 1;

        if (statedInterval is { } stated && stated.Number > 0)
        {
            ChartTimeUnit unit = stated.Unit < resolution ? resolution : stated.Unit;
            int days = stated.Number * NominalDays(unit);
            if (days > 0 && dayCount / days <= ceiling) return new ChartTimeInterval(stated.Number, unit);
        }

        int intervalDays = ceiling <= 0 ? dayCount : dayCount / ceiling;

        ChartTimeUnit chosen;
        double perInterval;

        if (intervalDays > 365 || resolution == ChartTimeUnit.Year)
        {
            chosen = ChartTimeUnit.Year;
            perInterval = 365.0;
        }
        else if (intervalDays > 31 || resolution == ChartTimeUnit.Month)
        {
            chosen = ChartTimeUnit.Month;
            perInterval = 31.0;
        }
        else
        {
            chosen = ChartTimeUnit.Day;
            perInterval = 1.0;
        }

        int number = (int)Math.Floor(intervalDays / perInterval);
        if (number <= 0) number = 1;

        if (chosen == ChartTimeUnit.Day)
        {
            // A step of three to six days is rounded up to a week, and anything past a week
            // becomes a month — LibreOffice would rather label Mondays than every fifth day.
            if (number is > 2 and < 7)
            {
                number = 7;
            }
            else if (number > 7)
            {
                chosen = ChartTimeUnit.Month;
                number = (int)Math.Floor(intervalDays / 31.0);
                if (number <= 0) number = 1;
            }
        }

        return new ChartTimeInterval(number, chosen);
    }

    /// <summary>The day count LibreOffice charges a unit when it compares intervals.</summary>
    /// <remarks>
    /// 31 and 365, both carrying the same <c>//todo: maybe different for other calendars</c> in
    /// the source. They are not the average lengths and the difference is what makes the tick
    /// count exceed the interval ceiling.
    /// </remarks>
    private static int NominalDays(ChartTimeUnit unit) => unit switch
    {
        ChartTimeUnit.Year => 365,
        ChartTimeUnit.Month => 31,
        _ => 1,
    };

    /// <summary>The null date of a workbook's date system.</summary>
    private static DateOnly NullDateOf(SpreadsheetDateSystem system)
        => system == SpreadsheetDateSystem.Date1904 ? Null1904 : Null1900;

    /// <summary>
    /// <c>Date::AddMonths</c>: add calendar months and normalise by <em>rolling over</em>.
    /// </summary>
    /// <remarks>
    /// 30 December plus two months is 30 February, which
    /// <c>comphelper::date::normalize</c> resolves by subtracting the month's length and carrying
    /// into the next month — 2 March in a common year, 1 March in a leap one. .NET's
    /// <c>DateOnly.AddMonths</c> clamps to the 28th instead, which is the ordinary convention and
    /// the wrong one here: every tick after the first on the corpus's date-axis workbook is placed
    /// by this difference.
    /// </remarks>
    public static DateOnly AddMonths(DateOnly date, int months)
    {
        int total = date.Month + months;
        int month = total % 12;
        int year = date.Year + (total / 12);

        if (total <= 0 || month == 0) year--;
        if (month <= 0) month += 12;

        return Normalise(date.Day, month, year);
    }

    /// <summary><c>Date::AddYears</c>: 29 February in a common year rolls to 1 March.</summary>
    public static DateOnly AddYears(DateOnly date, int years)
        => Normalise(date.Day, date.Month, date.Year + years);

    /// <summary>Resolves an out-of-range day of month by carrying into the following months.</summary>
    private static DateOnly Normalise(int day, int month, int year)
    {
        if (year < 1) year = 1;
        if (year > 9999) year = 9999;

        while (true)
        {
            int length = DateTime.DaysInMonth(year, month);
            if (day <= length) break;

            day -= length;

            if (month < 12)
            {
                month++;
            }
            else if (year < 9999)
            {
                year++;
                month = 1;
            }
            else
            {
                return DateOnly.MaxValue;
            }
        }

        return new DateOnly(year, month, day);
    }
}
