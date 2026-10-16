using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// What a category axis running <em>down</em> the page may do about labels that collide, and what
/// it may not.
/// </summary>
/// <remarks>
/// <para>
/// <strong>It is not the horizontal rule on the other axis.</strong>
/// <c>canAutoAdjustLabelPlacement</c> (<c>chart2/source/view/axes/VCartesianAxis.cxx:539-556</c>)
/// is the joint prerequisite for auto-rotation and auto-staggering, and its last three lines say
/// so outright — "automatic adjusting labels only works for horizontal axis with horizontal text
/// or vertical axis with vertical text". Ordinary horizontal type on an axis running down the
/// page therefore never turns 45 degrees and never staggers; the one move left is to thin the
/// labels out. Line breaking survives (<c>isBreakOfLabelsAllowed</c>, <c>:513-535</c>, returns
/// <c>bIsVerticalAxis</c> for a swapped chart) but its limit is not the tick spacing:
/// <c>:768-773</c> replaces it with the whole band between the chart's own edge and the axis, and
/// takes no five per cent off it.
/// </para>
/// <para>
/// <strong>Measured against 26.2.4.2 on decks built for it</strong> — <c>probes/chart-layout/</c>,
/// a bar chart whose category count and label size are both swept:
/// </para>
/// <list type="number">
/// <item><description><strong>No rotation and no staggering, ever.</strong> Eight categories whose
/// names are 184 pt wide, at 8, 16, 24, 32, 40, 48 and 56 categories, and the same names written
/// as one unbreakable word: 15 renderings, every label upright, no second row in any of
/// them.</description></item>
/// <item><description><strong>Thinning, on the label's height along the axis.</strong> All 24
/// labels are drawn at a 13.35 pt slot and every second at 11.44, which brackets a 10 pt label's
/// height at <strong>(11.437, 11.848]</strong>; sweeping the size gives (9.156, 9.410] at 8 pt,
/// (10.672, 11.054] at 9, (12.812, 13.351] at 11 <em>and at 12</em>, and (16.029, 16.865] at 14.
/// That 11 and 12 pt come out identical is the point: a fixed fraction of the em would put them
/// 1.15 pt apart, and the two brackets for 9 and 10 pt do not even intersect. The height is
/// quantised by <c>chart2</c>'s 96 dpi device, exactly as <c>probes/chart-vertical</c>
/// found.</description></item>
/// <item><description><strong>The wrap limit is the band, and it is its own fixed point.</strong>
/// On an automatically laid-out chart the plot gives up exactly the width the widest label needs,
/// so the limit is never binding and nothing ever breaks — which is why all 15 of those decks
/// draw one line per label however long the names are. Fix the plot with a
/// <c>c:manualLayout</c> at 0.10 of the frame and the same labels break onto four lines whose
/// longest is 58.7 pt against the stated band's 59.0, and the axis thins to every second, third
/// and fifth label at 8, 16 and 32 categories.</description></item>
/// </list>
/// <para>
/// <strong>One thing this does not close.</strong> The measured height bracket sits 0.19 to
/// 0.60 pt <em>above</em> the device line height the tree computes (11.25 pt at 10 pt), at every
/// one of the six sizes. Half a device pixel and the face's unrounded external leading both fit
/// all six and this round did not separate them, so no constant is invented here: the arrangement
/// asks the measurer for the height it already uses everywhere else. The cost is confined to a
/// slot within about 1.6% of the boundary, where this draws every label and the reference draws
/// every second one.
/// </para>
/// </remarks>
public class ChartVerticalAxisLabelTests
{
    /// <summary>A tenth of an em a character wide and 1.15 em tall, so a count is a length.</summary>
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
            => new(size * (0.1 * text.Length), size * 1.15);
    }

    private static readonly Length Size = Length.FromPoints(10);
    private static readonly ChartText Text = new(new Ruler(), null);

    /// <summary>The label height the ruler gives: 11.5 pt at 10 pt.</summary>
    private static readonly Length LabelHeight = Length.FromPoints(11.5);

    private static ChartAxisLabelLayout Resolve(
        string?[] texts,
        double spacing,
        ChartAxisDirection direction,
        ChartAxisText? stated = null,
        double room = 0.0)
    {
        Length[] centres = new Length[texts.Length];
        for (int at = 0; at < texts.Length; at++)
            centres[at] = Length.FromPoints(20.0 + (at * spacing));

        return ChartAxisLabels.Resolve(
            texts, centres, stated ?? new ChartAxisText(), Size, Text, false, direction,
            Length.FromPoints(room));
    }

    private static string?[] Labels(int count, string label)
    {
        string?[] texts = new string?[count];
        for (int at = 0; at < count; at++) texts[at] = label;
        return texts;
    }

    /// <summary>
    /// A vertical axis whose labels fit is left exactly as an uncrowded axis has always been.
    /// </summary>
    [Fact]
    public void AVerticalAxisWhoseLabelsFitIsLeftAlone()
    {
        ChartAxisLabelLayout layout =
            Resolve(Labels(8, "January"), 20.0, ChartAxisDirection.Vertical);

        layout.Rotation.ShouldBe(0.0);
        layout.Rhythm.ShouldBe(1);
        layout.Staggered.ShouldBeFalse();
        layout.Texts.ShouldBeNull();
    }

    /// <summary>
    /// The same crowded axis turns 45 degrees running along the page and only thins running down
    /// it.
    /// </summary>
    /// <remarks>
    /// The two arms are the measurement. A horizontal axis' labels collide when their
    /// <em>widths</em> meet, and it escapes by rotating; a vertical axis' collide when their
    /// heights do, and it has no escape but the rhythm. An implementation that mirrors the
    /// horizontal rule onto the other axis passes the second arm's rhythm and fails its rotation;
    /// one that leaves the vertical axis unarranged fails the rhythm.
    /// </remarks>
    [Fact]
    public void ACrowdedAxisRotatesAlongThePageAndThinsDownIt()
    {
        ChartAxisLabelLayout along =
            Resolve(Labels(12, "January"), 5.0, ChartAxisDirection.Horizontal);

        along.Rotation.ShouldBe(Math.PI / 4.0, 1e-12);

        ChartAxisLabelLayout down =
            Resolve(Labels(12, "January"), 5.0, ChartAxisDirection.Vertical);

        down.Rotation.ShouldBe(0.0);
        down.Staggered.ShouldBeFalse();

        // 11.5 pt of label against a 5 pt slot: two slots are 10 and still collide, three are 15
        // and do not.
        down.Rhythm.ShouldBe(3);
    }

    /// <summary>
    /// A vertical axis reserves its labels' <em>width</em>, where a horizontal one reserves their
    /// height.
    /// </summary>
    /// <remarks>
    /// <c>ShapeFactory::getSizeAfterRotation</c> measured away from the axis, which is across the
    /// page for one and along it for the other. It is what the plot rectangle gives up, so
    /// getting it the wrong way round reserves 11.5 pt where a seven-character name needs 7.
    /// </remarks>
    [Fact]
    public void AVerticalAxisReservesTheLabelsWidth()
    {
        ChartAxisLabelLayout down =
            Resolve(Labels(6, "January"), 30.0, ChartAxisDirection.Vertical);

        down.Reserved.Points.ShouldBe(7.0, 0.01);

        ChartAxisLabelLayout along =
            Resolve(Labels(6, "January"), 30.0, ChartAxisDirection.Horizontal);

        along.Reserved.Points.ShouldBe(LabelHeight.Points, 0.01);
    }

    /// <summary>
    /// A vertical axis breaks its labels at the band beside it and not at its tick spacing.
    /// </summary>
    /// <remarks>
    /// The three arms separate the two rules. The spacing is 40 pt throughout, so the horizontal
    /// limit — 0.95 of it, 38 pt — never breaks a 21 pt label; the band does when it is 15 pt and
    /// does not when it is 30. Reading the vertical limit off the spacing would leave all three
    /// unbroken.
    /// </remarks>
    [Theory]
    [InlineData(15.0, true)]
    [InlineData(30.0, false)]
    public void AVerticalAxisBreaksAtTheBandBesideIt(double room, bool breaks)
    {
        ChartAxisLabelLayout layout = Resolve(
            Labels(6, "AAAAAAAAAA BBBBBBBBBB"), 40.0, ChartAxisDirection.Vertical,
            new ChartAxisText(LineBreakAllowed: true), room);

        if (breaks)
        {
            layout.Texts.ShouldNotBeNull();
            layout.Texts![0].ShouldBe("AAAAAAAAAA\nBBBBBBBBBB");

            // The band it reserves is the widest line, not the joined run.
            layout.Reserved.Points.ShouldBe(10.0, 0.01);
        }
        else
        {
            layout.Texts.ShouldBeNull();
            layout.Reserved.Points.ShouldBe(21.0, 0.01);
        }
    }

    /// <summary>The same label on a horizontal axis of the same spacing does not break.</summary>
    /// <remarks>
    /// The control for the theory above: 0.95 of a 40 pt tick is 38 pt and the label is 21, so a
    /// limit read from the spacing leaves it whole whatever the band is.
    /// </remarks>
    [Fact]
    public void AHorizontalAxisOfTheSameSpacingDoesNotBreakIt()
    {
        ChartAxisLabelLayout layout = Resolve(
            Labels(6, "AAAAAAAAAA BBBBBBBBBB"), 40.0, ChartAxisDirection.Horizontal,
            new ChartAxisText(LineBreakAllowed: true), 15.0);

        layout.Texts.ShouldBeNull();
    }

    /// <summary>
    /// A broken label's own height is what decides the rhythm on a vertical axis.
    /// </summary>
    /// <remarks>
    /// The interaction between the two halves, and the reason the manual-layout probe thins so
    /// hard: a label broken onto two lines is twice as tall, so an axis that had room for every
    /// label loses two thirds of them. Measured on the probe deck at 0.10 of the frame — eight
    /// categories, four lines each, every second label drawn.
    /// </remarks>
    [Fact]
    public void ABrokenLabelIsTwiceAsTallAndThinsTheAxisTwiceAsHard()
    {
        // 15 pt of slot against one 11.5 pt line: nothing collides.
        ChartAxisLabelLayout whole = Resolve(
            Labels(8, "AAAAAAAAAA BBBBBBBBBB"), 15.0, ChartAxisDirection.Vertical,
            new ChartAxisText(LineBreakAllowed: true), 30.0);

        whole.Texts.ShouldBeNull();
        whole.Rhythm.ShouldBe(1);

        // The same axis with only 15 pt of band: two lines, 23 pt, and every second label.
        ChartAxisLabelLayout broken = Resolve(
            Labels(8, "AAAAAAAAAA BBBBBBBBBB"), 15.0, ChartAxisDirection.Vertical,
            new ChartAxisText(LineBreakAllowed: true), 15.0);

        broken.Texts.ShouldNotBeNull();
        broken.Rhythm.ShouldBe(2);
        broken.Rotation.ShouldBe(0.0);
    }
}
