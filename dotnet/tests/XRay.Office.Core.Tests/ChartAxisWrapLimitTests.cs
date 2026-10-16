using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// How much of one tick's worth of axis a category label's word gets before it breaks inside
/// itself, and what breaking actually does.
/// </summary>
/// <remarks>
/// <para>
/// Round 63 measured this against the reference on 328 authored decks
/// (<c>probes/sheets-r63/</c>), sweeping the tick spacing continuously by the chart frame's own
/// width and reading LibreOffice's decision out of its own <c>chart:coordinate-region</c> and out
/// of whether the labels are still in the exported PDF's text layer. Two facts came out of it and
/// both are asserted here, because a plausible implementation can satisfy either alone:
/// </para>
/// <list type="number">
/// <item><description>the limit is <strong>0.95 of the tick spacing</strong>, not the whole of it
/// — <c>Middle Column</c> among twelve categories turns the axis at a spacing of 35.35 where
/// <c>Column</c> measures 33.60, and the same twelve at 11 pt turn at 40.78 where it measures
/// 38.77;</description></item>
/// <item><description>and <strong>breaking does not turn the axis</strong> — it turns line
/// breaking off, after which the axis turns only if the labels then collide as single lines. A
/// label that breaks at a <em>space</em> is at most one line's worth of the limit wide and comes
/// out upright on two lines; that is why round 30's decks — every one of them one-word — read the
/// limit as 1.000 and rejected the source's own 0.95. <strong>A one-word label past 0.95 of a
/// tick does turn</strong>, because the collision keeps the same five per cent clear that the
/// wrap does — round 110, and the case below asserted the opposite until
/// then.</description></item>
/// </list>
/// <para>
/// <strong>The upright half of each case used to assert a thinned axis and no longer does, and
/// that change is a correction rather than a relaxation.</strong> A label that breaks at a space
/// is at most one line's worth of the limit wide — 0.95 of a tick — so it cannot reach its
/// neighbour and the axis is never thinned: every label is drawn, on two lines. The old
/// expectation was reachable only because the wrap was computed and then discarded by a guard
/// that compared string lengths, and a newline is exactly as long as the blank it replaces. The
/// reference settles it: <c>033_Event_planning_tracker</c>'s six two-word categories are all
/// drawn on two lines, and we drew three of six on one.
/// </para>
/// </remarks>
public class ChartAxisWrapLimitTests
{
    /// <summary>A tenth of an em a character, so a width in points is a character count.</summary>
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
            => new(size * (0.1 * text.Length), size * 1.15);
    }

    private static readonly Length Size = Length.FromPoints(10);
    private static readonly ChartText Text = new(new Ruler(), null);

    private static ChartAxisLabelLayout Resolve(string?[] texts, double spacing)
    {
        Length[] centres = new Length[texts.Length];
        for (int at = 0; at < texts.Length; at++)
            centres[at] = Length.FromPoints(20.0 + (at * spacing));

        return ChartAxisLabels.Resolve(
            texts, centres, new ChartAxisText(LineBreakAllowed: true), Size, Text);
    }

    private static string?[] Labels(int count, string label, string? first = null)
    {
        string?[] texts = new string?[count];
        for (int at = 0; at < count; at++) texts[at] = label;
        if (first is not null) texts[0] = first;
        return texts;
    }

    /// <summary>
    /// A word 96 points wide breaks in a 100 point slot and one 95 points wide does not.
    /// </summary>
    /// <remarks>
    /// The two cases fail under the two rival readings <em>in opposite directions</em>: a limit of
    /// the whole spacing leaves both thinned, and a limit of 0.90 turns both. The label is two
    /// words so that the break shows at all — see the class remarks — and the whole of it is wider
    /// than a tick either way, so the axis is crowded in both cases and only the break differs.
    /// </remarks>
    [Theory]
    [InlineData(96, true)]
    [InlineData(95, false)]
    public void AWordBreaksAtNineteenTwentiethsOfTheTickSpacing(int width, bool turns)
    {
        ChartAxisLabelLayout layout = Resolve(Labels(8, "AAAAA " + new string('W', width)), 100.0);

        if (turns)
        {
            layout.Rotation.ShouldBe(Math.PI / 4.0, 1e-12);
        }
        else
        {
            layout.Rotation.ShouldBe(0.0);
            layout.Rhythm.ShouldBe(1);
            layout.Texts.ShouldNotBeNull();
        }
    }

    /// <summary>
    /// A single word between 0.95 of a tick and a whole one breaks, and the axis <em>turns</em>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>This case asserted the opposite until round 110 and the premise was wrong.</strong>
    /// It read: "the restarted pass finds a 96 pt label in a 100 pt slot, which does not collide,
    /// and returns it upright on one line" — true of <c>doesOverlap</c> as this tree's chart2
    /// reads, and not true of 26.2.4.2. <c>probes/chart-geom-r108/fine-width.tsv</c> is fifteen
    /// one-word labels from 51.702 to 54.267 pt on a 54.202 pt pitch, of which the reference turns
    /// <strong>fifteen</strong>; and <c>probes/chart-collide-r110</c> §3.3 sweeps the tick pitch
    /// continuously at five run lengths and puts the one-word turn threshold at
    /// <strong>0.95 of the pitch, to [0.94895, 0.95125)</strong> — the wrap limit, not the tick.
    /// </para>
    /// <para>
    /// What is between the two is a separation: the reference keeps the same <c>nReduce</c> clear
    /// between two adjacent labels that it takes off the wrap limit. So the one-word case has
    /// <em>one</em> boundary rather than two, and the case below is the control that keeps this
    /// from becoming "a break turns the axis" — 94 points never restarts the wrap at all, so line
    /// breaking stays on and <c>canAutoAdjustLabelPlacement</c> refuses to rotate whatever the
    /// boxes do. See <c>ChartAxisCollisionReserveTests</c>.
    /// </para>
    /// </remarks>
    [Fact]
    public void AOneWordLabelPastTheWrapLimitTurnsInsideTheTick()
    {
        Resolve(Labels(8, new string('W', 96)), 100.0).Rotation
            .ShouldBe(Math.PI / 4.0, 1e-12);

        ChartAxisLabelLayout inside = Resolve(Labels(8, new string('W', 94)), 100.0);
        inside.Rotation.ShouldBe(0.0);
        inside.Rhythm.ShouldBe(1);
    }

    /// <summary>The first label is not tested for a break.</summary>
    /// <remarks>
    /// <c>nTick > 0</c> guards the whole check. Round 63's <c>C</c> deck is the measurement: its
    /// widest word is its first label, <c>START</c> at 31.96 against a limit of 29.16, and
    /// LibreOffice leaves that axis upright. Here the over-wide word is moved into label zero and
    /// nowhere else, so the axis must not turn.
    /// </remarks>
    [Fact]
    public void TheFirstLabelIsNotTestedForABreak()
    {
        // Every label is 96 wide and the ticks are 90 apart, so the axis collides whatever
        // happens; the only over-wide *word* is the one in label zero.
        const string ordinary = "AAAAAAAAAAAAAAA " + "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW"
                                + "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW";
        const string overWide = "AAAAAAAAAAAAAAA " + "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW"
                                + "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW";

        ChartAxisLabelLayout first = Resolve(Labels(8, ordinary, overWide), 90.0);
        first.Rotation.ShouldBe(0.0);

        // Upright because every label broke at its own blank, which is the only reason an axis
        // this crowded stays upright — asserting the rotation alone would pass on an axis that
        // never wrapped at all.
        first.Texts.ShouldNotBeNull()[0].ShouldBe(overWide.Replace(' ', '\n'));
        first.Texts.ShouldNotBeNull()[1].ShouldBe(ordinary.Replace(' ', '\n'));
        first.Rhythm.ShouldBe(1);

        // The same over-wide word anywhere else does turn it, so this is the index and not the
        // width.
        string?[] later = Labels(8, ordinary);
        later[1] = overWide;
        Resolve(later, 90.0).Rotation.ShouldBe(Math.PI / 4.0, 1e-12);
    }

    /// <summary>A trailing blank hangs past the line and is not part of the word's width.</summary>
    /// <remarks>
    /// <c>WWW…W</c> at 95 points followed by a space is 96 points if the space is counted, and
    /// counting it turns an axis the reference leaves upright — round 63's <c>C</c> deck again,
    /// whose <c>Middle</c> is 28.72 against a limit of 29.16 and whose <c>Middle </c> would be
    /// 31.43. A hyphen is the other way and is kept, because the break comes after it and its
    /// width is on the line.
    /// </remarks>
    [Fact]
    public void ATrailingBlankIsNotCountedInAWordsWidthButAHyphenIs()
    {
        // Both labels are 106 wide in a 100 pt slot, so both axes collide; what differs is
        // whether the run before the separator measures 95 or 96.
        ChartAxisLabelLayout blank = Resolve(Labels(8, new string('W', 95) + " AAAAAAAAAA"), 100.0);
        blank.Rotation.ShouldBe(0.0);

        // Upright because the label broke at the blank, and the break is where the blank was —
        // which is the half of this the rotation cannot show.
        blank.Texts.ShouldNotBeNull()[0].ShouldBe(new string('W', 95) + "\nAAAAAAAAAA");
        blank.Rhythm.ShouldBe(1);

        Resolve(Labels(8, new string('W', 95) + "-AAAAAAAAAA"), 100.0)
            .Rotation.ShouldBe(Math.PI / 4.0, 1e-12);
    }
}
