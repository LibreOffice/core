namespace Paperless.Core.Charts;

/// <summary>
/// What a chart states about one axis' scale, before anything is computed.
/// </summary>
/// <remarks>
/// Every member is nullable and null means "the file did not say", which is the distinction the
/// whole of <see cref="ChartScale"/> turns on: a stated maximum is honoured exactly, and an
/// unstated one is the output of an algorithm that has to agree with LibreOffice's to the last
/// tick. Both vocabularies reach this shape — OOXML from <c>c:scaling/c:min</c>,
/// <c>c:scaling/c:max</c> and <c>c:majorUnit</c>, ODF from <c>chart:minimum</c>,
/// <c>chart:maximum</c> and <c>chart:interval-major</c>.
/// </remarks>
/// <param name="Minimum">The stated minimum, or null for automatic.</param>
/// <param name="Maximum">The stated maximum, or null for automatic.</param>
/// <param name="MajorUnit">The stated distance between major ticks, or null for automatic.</param>
/// <param name="IsReversed">
/// True when the axis runs from maximum to minimum — <c>c:orientation val="maxMin"</c>.
/// </param>
public readonly record struct ChartScaleRequest(
    double? Minimum = null,
    double? Maximum = null,
    double? MajorUnit = null,
    bool IsReversed = false);

/// <summary>
/// An axis' scale after the automatic parts have been resolved: the range it covers and the
/// distance between its major ticks.
/// </summary>
/// <param name="Minimum">The value at the axis' start.</param>
/// <param name="Maximum">The value at its end.</param>
/// <param name="Distance">The distance between major ticks.</param>
/// <param name="IsReversed">True when the axis runs from maximum to minimum.</param>
public readonly record struct ChartScaleResult(
    double Minimum,
    double Maximum,
    double Distance,
    bool IsReversed = false)
{
    /// <summary>The span the axis covers, never zero.</summary>
    public double Span => Maximum - Minimum == 0.0 ? 1.0 : Maximum - Minimum;

    /// <summary>
    /// Where a value sits along the axis, 0 at the start and 1 at the end.
    /// </summary>
    /// <remarks>
    /// Not clamped. A point outside the scale genuinely is outside the plot area, and a caller
    /// drawing it wants to know that rather than to be handed a silently truncated bar.
    /// </remarks>
    public double Fraction(double value)
    {
        double along = (value - Minimum) / Span;
        return IsReversed ? 1.0 - along : along;
    }

    /// <summary>
    /// The major tick values, from the first multiple of <see cref="Distance"/> at or after
    /// <see cref="Minimum"/> to the last at or before <see cref="Maximum"/>.
    /// </summary>
    /// <remarks>
    /// Counted rather than accumulated. Adding <see cref="Distance"/> repeatedly accumulates
    /// binary error — nine additions of 0.1 reach 0.8999999999999999, which then prints as
    /// "0.9" but compares unequal to the maximum and drops the last tick — whereas
    /// <c>minimum + n × distance</c> has one rounding per tick and none of the drift.
    /// </remarks>
    public IEnumerable<double> MajorTicks()
    {
        if (Distance <= 0.0 || !double.IsFinite(Distance)) yield break;

        double span = Maximum - Minimum;
        if (span < 0.0 || !double.IsFinite(span)) yield break;

        // The count is floored with a relative tolerance rather than exactly, because the
        // maximum is itself the product of a multiply and lands a few ulps below the multiple it
        // was meant to be — which would otherwise cost the axis its topmost label.
        int count = (int)Math.Floor(span / Distance + 1e-9);
        if (count > MaximumTickCount) yield break;

        for (int at = 0; at <= count; at++) yield return Minimum + at * Distance;
    }

    /// <summary>How many major ticks an axis is ever asked to produce.</summary>
    /// <remarks>
    /// LibreOffice's own ceiling on a user-stated distance
    /// (<c>MAXIMUM_MANUAL_INCREMENT_COUNT</c>, <c>ScaleAutomatism.cxx:39</c>). A file may state
    /// <c>c:majorUnit val="1e-9"</c> over a range of thousands; the axis then has no meaningful
    /// ticks and drawing none is better than drawing 10^12.
    /// </remarks>
    public const int MaximumTickCount = 500;
}

/// <summary>
/// Resolves an axis' automatic scale — the range it covers and where its ticks fall.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Why this is the first thing to get right.</strong> An axis that runs 0–180 where the
/// reference runs 0–200 puts every bar at the wrong height while looking entirely reasonable,
/// and nothing downstream can detect it: the bars are in proportion to each other, the labels
/// are round numbers, and only a comparison against the reference shows the error. So this is a
/// port of LibreOffice's <c>ScaleAutomatism::calculateExplicitIncrementAndScaleForLinear</c>
/// (<c>chart2/source/view/axes/ScaleAutomatism.cxx:738-964</c>) step for step rather than a
/// plausible reimplementation of it.
/// </para>
/// <para>
/// <strong>The measurement that fixes it.</strong> <c>chart-bar-deck.pptx</c>'s two series peak
/// at 168 and its <c>c:valAx/c:scaling</c> states no minimum, maximum or major unit at all.
/// LibreOffice's PDF draws ten value-axis ticks over a plot area 241.994 pt tall, evenly spaced
/// 26.888 pt apart, labelled 0 to 180 — which this algorithm produces and a naive
/// "round the maximum up to a nice number" does not: 168 rounds up to 200 and gives nine ticks
/// of 25, putting the tallest bar at 84% of the plot height where the reference puts it at 93%.
/// </para>
/// <para>
/// <strong>The trap, named, because it costs an axis its whole lower half.</strong> The data
/// minimum here is 88, not 0 — no series contains a zero. Taking the range as
/// [88, 168] and rounding gives an axis from 80 to 170, on which a bar of 120 is 44% tall
/// instead of 67%. LibreOffice's step 2 is what prevents it:
/// <em>if the minimum is positive and less than five sixths of the maximum, the minimum becomes
/// zero</em> (<c>ScaleAutomatism.cxx:787-804</c>), gated on the plotter's
/// <c>isExpandWideValuesToZero</c>, which <c>VSeriesPlotter</c> returns true from for every Y
/// axis (<c>VSeriesPlotter.cxx:1742-1746</c>). 88/168 is 0.524, so the axis starts at zero. A
/// bar chart is the case that most needs it and the one where its absence is least visible.
/// </para>
/// </remarks>
public static class ChartScale
{
    /// <summary>
    /// LibreOffice's ceiling on the number of automatic intervals.
    /// </summary>
    /// <remarks>
    /// <c>lcl_getMaximumAutoIncrementCount</c> returns 10 for every axis type but a date axis
    /// (<c>ScaleAutomatism.cxx:43-49</c>), and <c>ScaleAutomatism::setMaximumAutoMainIncrementCount</c>
    /// clamps whatever the axis asks for into <c>[2, 10]</c> (<c>:143-151</c>). So ten is the most
    /// an axis ever gets and two the fewest.
    /// </remarks>
    public const int MaximumAutoIntervalCount = 10;

    /// <summary>The fewest automatic intervals an axis is given, whatever its size.</summary>
    /// <remarks><c>ScaleAutomatism.cxx:145-146</c>, commented <c>#i82006</c>.</remarks>
    public const int MinimumAutoIntervalCount = 2;

    /// <summary>
    /// Resolves a value axis' scale from what the file states and what the data holds.
    /// </summary>
    /// <param name="request">What the file states; every unstated member is computed.</param>
    /// <param name="dataMinimum">
    /// The smallest value any series contributes, or null when there are no numbers at all.
    /// </param>
    /// <param name="dataMaximum">The largest value any series contributes, or null for none.</param>
    /// <param name="expandToZero">
    /// Whether a wholly positive range is pulled down to zero — LibreOffice's
    /// <c>isExpandWideValuesToZero</c>. True for the value axis of every plotter in
    /// <c>VSeriesPlotter</c>'s default, which is what a bar, line, area or column chart gets.
    /// </param>
    /// <param name="maximumIntervals">
    /// The most automatic intervals the axis may have, clamped into
    /// <c>[<see cref="MinimumAutoIntervalCount"/>, <see cref="MaximumAutoIntervalCount"/>]</c>.
    /// <see cref="ChartLayout"/> derives it from the axis' length once it has measured a label;
    /// a caller with no geometry passes nothing and gets ten, which is LibreOffice's first pass.
    /// </param>
    public static ChartScaleResult Resolve(
        ChartScaleRequest request,
        double? dataMinimum,
        double? dataMaximum,
        bool expandToZero = true,
        int maximumIntervals = MaximumAutoIntervalCount)
    {
        int automaticIntervals = Math.Clamp(
            maximumIntervals, MinimumAutoIntervalCount, MaximumAutoIntervalCount);

        // A chart whose cache holds no numbers still has an axis, and LibreOffice's own
        // ExplicitScaleData is constructed as [0, 10] for exactly this case
        // (ScaleAutomatism.cxx:63-64). Drawing 0..10 is what the reference draws.
        double sourceMinimum = request.Minimum ?? dataMinimum ?? 0.0;
        double sourceMaximum = request.Maximum ?? dataMaximum ?? 10.0;

        if (!double.IsFinite(sourceMinimum)) sourceMinimum = 0.0;
        if (!double.IsFinite(sourceMaximum)) sourceMaximum = 10.0;

        bool autoMinimum = request.Minimum is null;
        bool autoMaximum = request.Maximum is null;

        // STEP 1: validate the range. A minimum above the maximum moves whichever limit the file
        // did not fix (ScaleAutomatism.cxx:755-762).
        if (sourceMinimum > sourceMaximum)
        {
            if (autoMaximum || !autoMinimum) sourceMaximum = sourceMinimum;
            else sourceMinimum = sourceMaximum;
        }

        // A wholly negative range is negated and swapped so that the rest of the algorithm only
        // ever sees a positive one, then swapped back at the end (ScaleAutomatism.cxx:764-777).
        // [0, 0] is excluded deliberately: swapping it would produce [-1, 0] rather than [0, 1].
        bool swapped = sourceMinimum < 0.0 && sourceMaximum <= 0.0;
        if (swapped)
        {
            (sourceMinimum, sourceMaximum) = (-sourceMaximum, -sourceMinimum);
            (autoMinimum, autoMaximum) = (autoMaximum, autoMinimum);
        }

        // STEP 2: the unrounded limits.
        double temporaryMinimum = sourceMinimum;
        double temporaryMaximum = sourceMaximum;

        if (autoMinimum && temporaryMinimum > 0.0)
        {
            if (temporaryMinimum == temporaryMaximum
                || temporaryMinimum / temporaryMaximum < 5.0 / 6.0)
            {
                // The wide case: the data does not fill the top sixth of its own range, so the
                // axis starts at zero. This is the branch chart-bar-deck takes.
                if (expandToZero) temporaryMinimum = 0.0;
            }
            else if (expandToZero)
            {
                // The narrow case: all the values are within a sixth of each other, so zeroing
                // the axis would flatten them into one indistinguishable band. Half the visible
                // range is added below instead (ScaleAutomatism.cxx:796-803). Gated on the same
                // flag because VSeriesPlotter returns the same answer for both
                // (VSeriesPlotter.cxx:1742-1752).
                temporaryMinimum -= (temporaryMaximum - temporaryMinimum) / 2.0;
            }
        }

        // A range that is still zero-sized gets space on whichever side is free.
        if (temporaryMinimum == temporaryMaximum)
        {
            if (autoMaximum || !autoMinimum)
                temporaryMaximum = temporaryMaximum == 0.0 ? 1.0 : temporaryMaximum * 2.0;
            else
                temporaryMinimum = temporaryMinimum == 0.0 ? -1.0 : temporaryMinimum / 2.0;
        }

        // STEP 3: the interval. The base value the intervals are anchored to is zero unless a
        // limit is fixed, in which case it is that limit (ScaleAutomatism.cxx:831-839).
        double baseValue = !autoMinimum ? temporaryMinimum : !autoMaximum ? temporaryMaximum : 0.0;

        bool autoDistance = request.MajorUnit is not { } stated || !(stated > 0.0);
        double distance = autoDistance ? 0.0 : request.MajorUnit!.Value;
        int intervalCeiling = autoDistance
            ? automaticIntervals
            : ChartScaleResult.MaximumTickCount;

        double magnitude = 0.0;
        double normalised = 0.0;
        bool haveNormalised = false;

        double axisMinimum = temporaryMinimum;
        double axisMaximum = temporaryMaximum;

        // The loop repeats with a coarser interval whenever rounding the limits outwards has
        // produced more intervals than are allowed (ScaleAutomatism.cxx:854-964). It terminates
        // because every pass either multiplies the magnitude by ten or moves the normalised
        // distance up the 1-2-5 ladder; the guard is belt and braces against a NaN limit.
        for (int pass = 0; pass < 64; pass++)
        {
            if (autoDistance)
            {
                if (!haveNormalised)
                {
                    double raw = (temporaryMaximum - temporaryMinimum) / intervalCeiling;

                    if (raw <= 1.0e-307)
                    {
                        normalised = 1.0;
                        magnitude = 1.0e-307;
                    }
                    else if (!double.IsFinite(raw))
                    {
                        normalised = 1.0;
                        magnitude = double.MaxValue;
                    }
                    else
                    {
                        // The magnitude is the power of ten below the raw interval and the
                        // normalised part is snapped to 1, 2 or 5 — the three multipliers that
                        // give round labels. 16.8 becomes 2 × 10, which is the corpus case.
                        magnitude = Math.Pow(10.0, Math.Floor(Math.Log10(raw)));
                        normalised = raw / magnitude;

                        if (normalised <= 1.0) normalised = 1.0;
                        else if (normalised <= 2.0) normalised = 2.0;
                        else if (normalised <= 5.0) normalised = 5.0;
                        else
                        {
                            normalised = 1.0;
                            magnitude *= 10.0;
                        }
                    }

                    haveNormalised = true;
                }
                else if (normalised == 1.0) normalised = 2.0;
                else if (normalised == 2.0) normalised = 5.0;
                else
                {
                    normalised = 1.0;
                    magnitude *= 10.0;
                }

                distance = normalised * magnitude;
            }

            // STEP 4: round the limits out to whole multiples of the interval, and add one more
            // interval when a data point would otherwise sit on the border.
            axisMinimum = temporaryMinimum;
            axisMaximum = temporaryMaximum;

            if (autoMinimum)
            {
                axisMinimum = FloorAt(axisMinimum, baseValue, distance);
                if (axisMinimum != 0.0
                    && (axisMaximum - sourceMinimum) / (axisMaximum - axisMinimum) > 20.0 / 21.0)
                {
                    axisMinimum -= distance;
                }
            }

            if (autoMaximum)
            {
                axisMaximum = CeilingAt(axisMaximum, baseValue, distance);
                if (axisMaximum != 0.0
                    && (sourceMaximum - axisMinimum) / (axisMaximum - axisMinimum) > 20.0 / 21.0)
                {
                    axisMaximum += distance;
                }
            }

            double intervals = Math.Floor((axisMaximum - axisMinimum) / distance);
            if (!(intervals > intervalCeiling)) break;

            // A stated distance that produces too many intervals is discarded rather than
            // honoured, which is what stops a c:majorUnit of 0.001 over a range of 200 from
            // asking for two hundred thousand gridlines.
            autoDistance = true;
            intervalCeiling = automaticIntervals;
        }

        return swapped
            ? new ChartScaleResult(-axisMaximum, -axisMinimum, distance, request.IsReversed)
            : new ChartScaleResult(axisMinimum, axisMaximum, distance, request.IsReversed);
    }

    /// <summary>
    /// The largest multiple of <paramref name="distance"/> from
    /// <paramref name="baseValue"/> that is at or below <paramref name="value"/>.
    /// </summary>
    /// <remarks>
    /// <c>EquidistantTickFactory::getMinimumAtIncrement</c>
    /// (<c>Tickmarks_Equidistant.cxx</c>). The extra comparison after the floor is not
    /// redundant: floating-point division can land a hair above the value it was meant to floor.
    /// </remarks>
    private static double FloorAt(double value, double baseValue, double distance)
    {
        if (distance <= 0.0 || !double.IsFinite(distance)) return value;

        double result = baseValue + Math.Floor((value - baseValue) / distance) * distance;
        if (result > value && !Approximately(result, value)) result -= distance;
        return result;
    }

    /// <summary>
    /// The smallest multiple of <paramref name="distance"/> from
    /// <paramref name="baseValue"/> that is at or above <paramref name="value"/>.
    /// </summary>
    /// <remarks><c>EquidistantTickFactory::getMaximumAtIncrement</c>.</remarks>
    private static double CeilingAt(double value, double baseValue, double distance)
    {
        if (distance <= 0.0 || !double.IsFinite(distance)) return value;

        double result = baseValue + Math.Floor((value - baseValue) / distance) * distance;
        if (result < value && !Approximately(result, value)) result += distance;
        return result;
    }

    /// <summary>
    /// LibreOffice's <c>rtl::math::approxEqual</c>: equal to within the last few bits.
    /// </summary>
    /// <remarks>
    /// A relative comparison rather than an absolute epsilon, because the values here span
    /// currency in millions and physical constants in nanometres in the same corpus, and a fixed
    /// tolerance is wrong for one of them.
    /// </remarks>
    private static bool Approximately(double left, double right)
    {
        if (left == right) return true;

        double difference = Math.Abs(left - right);
        double scale = Math.Max(Math.Abs(left), Math.Abs(right));
        return difference < scale * 1.0e-12;
    }
}
