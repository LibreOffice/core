using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Globalization;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// The gap two category labels have to keep clear of one another, which is not the whole tick
/// spacing.
/// </summary>
/// <remarks>
/// <para>
/// <c>doesOverlap</c> (<c>VCartesianAxis.cxx:186-207</c>) intersects the two label shapes, and
/// reading chart2 says a label's shape is exactly its text — so two labels ought to collide when
/// their mean width reaches the tick spacing. 26.2.4.2 collides them <strong>earlier</strong>,
/// and the difference is the same five per cent <c>createTextShapes</c> already takes off the
/// wrap limit "to have a visible distance between the labels".
/// </para>
/// <para>
/// <strong>The discriminator is a pair of labels of different widths.</strong> With equal labels
/// the wrap limit (0.95 of the spacing) and the collision threshold are within half a per cent of
/// one another and only the wrap is ever reached, which is why three rounds of one-word ladders
/// could not see this at all. Alternating a long label with a short one along one axis and
/// sweeping the chart frame's width puts the collision on its own:
/// <c>probes/chart-collide-r110/results.md</c> §3, four tick pitches from 54.2 to 121.7 pt, every
/// label a run of one letter so that no ligature or kern pair can enter the width. A
/// <em>constant</em> reserve is refuted outright (2.9 pt at pitch 54 against 6.4 at pitch 122);
/// the measured fraction is about <strong>5.3 %</strong> and the source's own <c>nReduce</c> is
/// 5.0 %, which is what is implemented — §3.4 of that page says why and §7 carries the
/// 0.3 %-of-a-tick difference as open.
/// </para>
/// <para>
/// Each case below fails at the round's base in a stated direction, and the two directions are
/// opposite: without the reserve the first two axes stay upright, and a reserve applied to a
/// vertical axis — which has no such reduction in its own wrap limit — would thin one that the
/// reference leaves alone.
/// </para>
/// </remarks>
public class ChartAxisCollisionReserveTests
{
    /// <summary>A tenth of an em a character, so a width in points is a character count.</summary>
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
            => new(size * (0.1 * text.Length), size * 1.15);
    }

    private static readonly Length Size = Length.FromPoints(10);
    private static readonly ChartText Text = new(new Ruler(), null);

    private static ChartAxisLabelLayout Resolve(
        string?[] texts,
        double spacing,
        ChartAxisDirection direction = ChartAxisDirection.Horizontal,
        Length room = default)
    {
        Length[] centres = new Length[texts.Length];
        for (int at = 0; at < texts.Length; at++)
            centres[at] = Length.FromPoints(20.0 + (at * spacing));

        return ChartAxisLabels.Resolve(
            texts, centres, new ChartAxisText(LineBreakAllowed: true), Size, Text,
            direction: direction, room: room, hyphenator: Hyphenators.None);
    }

    /// <summary>A run of <paramref name="width"/> points, one character to the point.</summary>
    private static string Run(int width) => new('n', width);

    private static string?[] Alternating(int count, int longWidth, int shortWidth)
    {
        string?[] texts = new string?[count];
        for (int at = 0; at < count; at++)
            texts[at] = Run(at % 2 == 0 ? longWidth : shortWidth);
        return texts;
    }

    /// <summary>
    /// Two labels whose mean is 95.5 points on a 100 point pitch are turned, and 91 are not.
    /// </summary>
    /// <remarks>
    /// The long label is 98 points, past the 95 point wrap limit, so line breaking is off in both
    /// cases and the only thing that separates them is the collision. Against the tick spacing
    /// itself neither pair collides — 95.5 and 91 are both under 100 — so the base leaves both
    /// upright; against 0.95 of it, one does and one does not.
    /// </remarks>
    [Fact]
    public void APairIsCollidedByFiveHundredthsOfTheSpacingItDoesNotOccupy()
    {
        Resolve(Alternating(9, 98, 93), 100.0).Rotation
            .ShouldBe(Math.PI / 4.0, 1e-9);

        Resolve(Alternating(9, 98, 84), 100.0).Rotation.ShouldBe(0.0);
    }

    /// <summary>The reserve is a fraction of the pitch and not a constant.</summary>
    /// <remarks>
    /// The same pair scaled by two and a half, which a <em>constant</em> reserve cannot follow: a
    /// mean of 238.5 on a 250 point pitch is 11.5 short of the tick, so five points of reserve
    /// leaves it upright and twelve and a half — the same five per cent — turns it.
    /// </remarks>
    [Fact]
    public void TheReserveFollowsTheTickSpacing()
    {
        Resolve(Alternating(9, 245, 232), 250.0).Rotation
            .ShouldBe(Math.PI / 4.0, 1e-9);

        Resolve(Alternating(9, 245, 210), 250.0).Rotation.ShouldBe(0.0);
    }

    /// <summary>
    /// A single word between the wrap limit and the pitch is turned, where it used to be upright.
    /// </summary>
    /// <remarks>
    /// This is <c>probes/chart-geom-r108</c>'s <c>fine-width.tsv</c> in one assertion: fifteen
    /// one-word labels from 51.702 to 54.267 pt on a 54.202 pt pitch, of which 26.2.4.2 turns
    /// fifteen and this tree turned the one that was wider than the pitch. The label restarts the
    /// wrap either way — it is past 0.95 of the tick — and what decides it is whether the
    /// single-line labels then collide.
    /// </remarks>
    [Fact]
    public void AWordPastTheWrapLimitButInsideTheTickIsTurned()
    {
        Resolve(Enumerable.Repeat<string?>(Run(96), 9).ToArray(), 100.0).Rotation
            .ShouldBe(Math.PI / 4.0, 1e-9);
    }

    /// <summary>And a word that does not reach the wrap limit is still left alone.</summary>
    /// <remarks>
    /// The control that keeps the reserve from becoming a licence to turn every crowded axis: at
    /// 94 points the label never restarts the wrap, so line breaking stays on, and
    /// <c>canAutoAdjustLabelPlacement</c> refuses to rotate while it is
    /// (<c>VCartesianAxis.cxx:544-545</c>) whatever the boxes do.
    /// </remarks>
    [Fact]
    public void AWordInsideTheWrapLimitIsNotTurned()
    {
        ChartAxisLabelLayout layout =
            Resolve(Enumerable.Repeat<string?>(Run(94), 9).ToArray(), 100.0);

        layout.Rotation.ShouldBe(0.0);
        layout.Rhythm.ShouldBe(1);
    }

    /// <summary>A vertical category axis gets no reserve, because its wrap limit has none.</summary>
    /// <remarks>
    /// <c>createTextShapes</c> replaces the limit outright for a swapped chart —
    /// <c>nLimitedSpaceForText = pTickFactory-&gt;getXaxisStartPos().getX()</c>,
    /// <c>VCartesianAxis.cxx:768-773</c> — and takes no five per cent off it, so there is no
    /// <c>nReduce</c> to reserve and nothing was measured. Labels 11.5 points deep on a 12 point
    /// pitch are the case: they clear one another, and a reserve of 0.6 would thin them to every
    /// second.
    /// </remarks>
    [Fact]
    public void AVerticalAxisKeepsNoReserve()
    {
        ChartAxisLabelLayout layout = Resolve(
            Enumerable.Repeat<string?>(Run(30), 9).ToArray(), 12.0,
            ChartAxisDirection.Vertical, Length.FromPoints(200));

        layout.Rhythm.ShouldBe(1);
    }

    /// <summary>
    /// Widening the box does not deepen the band the axis reserves.
    /// </summary>
    /// <remarks>
    /// What is measured is the separation the reference keeps between two labels, not that its
    /// shape is genuinely wider than its text — and the two are separable, because the band a
    /// horizontal axis gives up is the label's <em>height</em>. A 40 point label at 10 pt on the
    /// ruler here is 11.5 points deep, and it stays 11.5 whatever the pitch, so nothing that this
    /// reserve does can move a plot rectangle on an upright axis.
    /// </remarks>
    [Fact]
    public void TheReserveDoesNotReachTheBandTheAxisGivesUp()
    {
        Length narrow = Resolve(Enumerable.Repeat<string?>(Run(40), 9).ToArray(), 100.0).Reserved;
        Length wide = Resolve(Enumerable.Repeat<string?>(Run(40), 9).ToArray(), 300.0).Reserved;

        narrow.ShouldBe(wide);
        narrow.ShouldBe(Size * 1.15);
    }
}
