using Paperless.Core.Charts;
using Shouldly;

namespace Paperless.Core.Tests;

/// <summary>
/// Checks the automatic axis scale against LibreOffice's own, which is the number every other
/// part of a chart is measured from.
/// </summary>
/// <remarks>
/// <para>
/// An axis that runs 0–180 where the reference runs 0–200 puts every bar at the wrong height
/// while looking entirely reasonable — the bars stay in proportion to each other, the labels
/// stay round, and nothing downstream can tell. That makes this the one part of a chart worth
/// asserting on numerically rather than by eye, and it is why <see cref="ChartScale"/> is a step
/// for step port of <c>ScaleAutomatism::calculateExplicitIncrementAndScaleForLinear</c> rather
/// than a reimplementation.
/// </para>
/// <para>
/// The headline expectation is quoted from LibreOffice's PDF for <c>chart-bar-deck.pptx</c>,
/// whose value axis it draws ten ticks on, labelled 0 to 180, over a plot area 241.994 pt tall
/// with 26.888 pt between neighbours.
/// </para>
/// </remarks>
public class ChartScaleTests
{
    [Fact]
    public void TheCorpusChartsRangeIsZeroToOneHundredAndEightyInStepsOfTwenty()
    {
        // chart-bar-deck's two series hold 120, 95, 143, 168 and 88, 132, 101, 121, and its
        // c:valAx/c:scaling states no minimum, no maximum and no major unit. Everything below
        // therefore comes out of the algorithm.
        ChartScaleResult scale = ChartScale.Resolve(default, 88, 168);

        scale.Minimum.ShouldBe(0.0);
        scale.Maximum.ShouldBe(180.0);
        scale.Distance.ShouldBe(20.0);

        scale.MajorTicks().ShouldBe([0, 20, 40, 60, 80, 100, 120, 140, 160, 180]);
    }

    [Fact]
    public void AWholelyPositiveRangeThatDoesNotFillItsTopSixthStartsAtZero()
    {
        // The trap this exists to prevent. 88/168 is 0.524, well under five sixths, so
        // LibreOffice pulls the minimum down to zero (ScaleAutomatism.cxx:787-795) and a bar of
        // 120 is two thirds of the plot's height. Keeping the data minimum instead would put the
        // axis at 80..170 and draw the same bar at 44%, which is a plausible-looking chart.
        ChartScale.Resolve(default, 88, 168).Minimum.ShouldBe(0.0);

        // And the flag that governs it is honoured: a plotter that does not expand to zero — a
        // scatter chart's value X axis — keeps its own minimum, rounded out to the increment.
        ChartScale.Resolve(default, 88, 168, expandToZero: false).Minimum.ShouldBe(80.0);
    }

    [Fact]
    public void ANarrowRangeIsHalvedTowardsZeroRatherThanFlattenedOntoIt()
    {
        // 158/168 is 0.940, above five sixths, so zeroing the axis would squeeze ten units of
        // difference into the top 6% of the plot. LibreOffice instead expands the minimum
        // downwards by half the visible range (ScaleAutomatism.cxx:796-803): 158 − 5 = 153,
        // which then rounds out to a whole increment.
        ChartScaleResult scale = ChartScale.Resolve(default, 158, 168);

        scale.Minimum.ShouldBeLessThan(158.0);
        scale.Minimum.ShouldBeGreaterThan(0.0);
        scale.Maximum.ShouldBeGreaterThanOrEqualTo(168.0);
    }

    [Fact]
    public void AStatedLimitIsHonouredExactlyAndAStatedIntervalWithIt()
    {
        // The measurement in the brief that turned out to be about a different file: the same
        // data can draw 0 20 … 180 or 0 50 … 200, and the difference is what the *file* says.
        // A chart part that states them gets them back unrounded.
        ChartScaleResult scale = ChartScale.Resolve(
            new ChartScaleRequest(Minimum: 0, Maximum: 200, MajorUnit: 50), 88, 168);

        scale.Minimum.ShouldBe(0.0);
        scale.Maximum.ShouldBe(200.0);
        scale.MajorTicks().ShouldBe([0, 50, 100, 150, 200]);
    }

    [Fact]
    public void AStatedIntervalThatWouldDrawFiveHundredTicksIsDiscarded()
    {
        // c:majorUnit is a double written by whoever produced the file. Honouring 0.001 over a
        // range of 200 asks for two hundred thousand gridlines, so LibreOffice falls back to the
        // automatic interval once the count passes MAXIMUM_MANUAL_INCREMENT_COUNT
        // (ScaleAutomatism.cxx:956-963).
        ChartScaleResult scale = ChartScale.Resolve(
            new ChartScaleRequest(MajorUnit: 0.001), 0, 200);

        scale.Distance.ShouldBeGreaterThan(0.001);
        scale.MajorTicks().Count().ShouldBeLessThanOrEqualTo(ChartScaleResult.MaximumTickCount + 1);
    }

    [Fact]
    public void AWhollyNegativeRangeIsNegatedSwappedAndSwappedBack()
    {
        // Both [2, 5] and [−5, −2] are processed as [2, 5] and the latter swapped back
        // (ScaleAutomatism.cxx:764-777), which is what keeps a chart of losses from getting an
        // axis whose rounding runs the wrong way.
        ChartScaleResult scale = ChartScale.Resolve(default, -168, -88);

        scale.Minimum.ShouldBe(-180.0);
        scale.Maximum.ShouldBe(0.0);
        scale.Distance.ShouldBe(20.0);
    }

    [Fact]
    public void ARangeThatIsExactlyZeroWideStillHasAnAxis()
    {
        // A single-point series, or several series that all hold the same number. Doubling the
        // maximum is what LibreOffice does (ScaleAutomatism.cxx:806-826), and zero becomes one
        // rather than staying zero.
        ChartScale.Resolve(default, 0, 0).Maximum.ShouldBeGreaterThan(0.0);
        ChartScale.Resolve(default, 50, 50).Maximum.ShouldBeGreaterThanOrEqualTo(50.0);

        // And a chart whose cache holds no numbers at all starts from LibreOffice's constructed
        // default of [0, 10] (ScaleAutomatism.cxx:63-64) rather than from a degenerate axis —
        // and then goes through the same rounding as any other range, which pushes the maximum
        // to 12: the data would otherwise sit exactly on the border, and the twentieth-of-a-
        // range rule adds one more increment (ScaleAutomatism.cxx:939-941).
        ChartScaleResult empty = ChartScale.Resolve(default, null, null);
        empty.Minimum.ShouldBe(0.0);
        empty.Maximum.ShouldBe(12.0);
    }

    [Fact]
    public void TheTicksAreCountedRatherThanAccumulatedSoTheLastOneSurvives()
    {
        // Adding the distance repeatedly drifts: nine additions of 0.1 reach
        // 0.8999999999999999, which compares greater than 0.9 is not and silently drops the
        // topmost label. Counting from the minimum has one rounding per tick and no drift.
        ChartScaleResult scale = new(0.0, 0.9, 0.1);
        scale.MajorTicks().Count().ShouldBe(10);
        scale.MajorTicks().Last().ShouldBe(0.9, 1e-12);
    }

    [Fact]
    public void AReversedAxisRunsFromTheMaximumDownwards()
    {
        // c:orientation val="maxMin". The scale is unchanged and only the mapping flips, which
        // is what keeps the labels reading the same way up.
        ChartScaleResult scale = new(0.0, 180.0, 20.0, IsReversed: true);

        scale.Fraction(0.0).ShouldBe(1.0, 1e-12);
        scale.Fraction(180.0).ShouldBe(0.0, 1e-12);
        scale.Fraction(90.0).ShouldBe(0.5, 1e-12);
    }
}
