using Paperless.Core.Charts;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Shouldly;

namespace Paperless.Core.Tests;

/// <summary>
/// What a crowded category axis does about it: rotate, thin out, or neither.
/// </summary>
/// <remarks>
/// The whole feature is a decision cascade with two dead branches and one non-obvious entry
/// condition, so the tests are about <em>which</em> escape is taken rather than about the pixels.
/// Each of the three below is a case that produced a plausible-looking wrong picture at some point
/// on this run.
/// </remarks>
public class ChartAxisLabelTests
{
    /// <summary>Half an em per character, 1.15 em a line.</summary>
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family)
            => new(size * (0.5 * text.Length), size * 1.15);
    }

    private static readonly Length Size = Length.FromPoints(10);

    private static (string?[] Texts, Length[] Centres) Axis(int count, double spacing)
    {
        string?[] texts = new string?[count];
        Length[] centres = new Length[count];

        for (int at = 0; at < count; at++)
        {
            texts[at] = "September";
            centres[at] = Length.FromPoints(20.0 + (at * spacing));
        }

        return (texts, centres);
    }

    /// <summary>Labels that fit are left exactly alone.</summary>
    /// <remarks>
    /// The case every chart in the corpus is, and the one that must not move: an axis whose labels
    /// do not collide reserves one line, upright, every label drawn. If this changed, so would the
    /// plot rectangle of all six corpus charts at once.
    /// </remarks>
    [Fact]
    public void LabelsThatFitAreNotRotatedThinnedOrStaggered()
    {
        (string?[] texts, Length[] centres) = Axis(4, 120.0);

        ChartAxisLabelLayout layout = ChartAxisLabels.Resolve(
            texts, centres, new ChartAxisText(), Size, new ChartText(new Ruler(), null));

        layout.Rotation.ShouldBe(0.0);
        layout.Rhythm.ShouldBe(1);
        layout.Staggered.ShouldBeFalse();
    }

    /// <summary>
    /// Colliding labels on an axis that may not wrap are turned 45°.
    /// </summary>
    /// <remarks>
    /// <c>autoRotate45</c>. This is the state a <c>c:dateAx</c> starts in, because the OOXML
    /// importer's three label properties are set in the <c>else</c> of a test on <c>bDateAxis</c>
    /// and a date axis therefore keeps chart2's own defaults — wrapping <em>off</em>, which is what
    /// <c>canAutoAdjustLabelPlacement</c> requires. <c>bnc889755.pptx</c> is that file, and its
    /// sixteen month names are turned a quarter of a right angle in LibreOffice's own render.
    /// </remarks>
    [Fact]
    public void CollidingLabelsWithNoWrappingAreRotatedFortyFiveDegrees()
    {
        (string?[] texts, Length[] centres) = Axis(16, 36.0);

        ChartAxisLabelLayout layout = ChartAxisLabels.Resolve(
            texts, centres, new ChartAxisText(), Size, new ChartText(new Ruler(), null));

        layout.Rotation.ShouldBe(Math.PI / 4.0, 1e-12);
        layout.Rhythm.ShouldBe(1);
        layout.Reserved.ShouldBeGreaterThan(Size);
    }

    /// <summary>
    /// An axis that is allowed to overlap does none of it.
    /// </summary>
    /// <remarks>
    /// <c>m_bOverlapAllowed</c> short-circuits the whole cascade — OOXML sets it whenever the file
    /// states a rotation of exactly zero, which is its way of saying "I have already decided".
    /// </remarks>
    [Fact]
    public void AnAxisThatAllowsOverlapIsLeftAlone()
    {
        (string?[] texts, Length[] centres) = Axis(16, 36.0);

        ChartAxisLabelLayout layout = ChartAxisLabels.Resolve(
            texts, centres, new ChartAxisText(OverlapAllowed: true), Size, new ChartText(new Ruler(), null));

        layout.Rotation.ShouldBe(0.0);
        layout.Rhythm.ShouldBe(1);
    }

    /// <summary>
    /// An axis that may wrap thins its labels out rather than rotating them — unless a single
    /// word does not fit, which is the one thing that turns wrapping off.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>canAutoAdjustLabelPlacement</c> refuses while line breaking is on, so a plain
    /// <c>c:catAx</c> — whose importer turns it on — cannot rotate. The only route out is the wrap
    /// test in <c>createTextShapes</c>, and that fires on a break <em>inside</em> a word:
    /// <c>lcl_hasWordBreak</c> returns true only where a line starts somewhere a word does not.
    /// </para>
    /// <para>
    /// Both halves are asserted because the difference between them is the difference between
    /// <c>tdf106217.pptx</c> — "Netherlands", one word wider than its slot, rotated in the
    /// reference — and an axis of short words, which is thinned instead.
    /// </para>
    /// </remarks>
    [Fact]
    public void WrappingIsTurnedOffOnlyByAWordThatCannotFitAtAll()
    {
        ChartAxisText wrapping = new(LineBreakAllowed: true);

        // Ten characters at half an em is 50 pt in a 30 pt slot: one word, no way to break it.
        (string?[] unbreakable, Length[] centres) = Axis(8, 30.0);

        ChartAxisLabels.Resolve(unbreakable, centres, wrapping, Size, new ChartText(new Ruler(), null))
            .Rotation.ShouldBe(Math.PI / 4.0, 1e-12);

        // The same width split into two words, each of which fits: wrapping stays on, so the
        // rotation branch is closed and the labels are thinned out instead.
        string?[] breakable = [.. unbreakable.Select(_ => "Sep tem")];

        ChartAxisLabelLayout thinned =
            ChartAxisLabels.Resolve(breakable, centres, wrapping, Size, new ChartText(new Ruler(), null));

        thinned.Rotation.ShouldBe(0.0);
        thinned.Rhythm.ShouldBeGreaterThan(1);
    }

    /// <summary>
    /// A thinned axis draws every <em>n</em>th label, and always the first.
    /// </summary>
    /// <remarks>
    /// <c>nTick % nRhythm != 0</c> keeps tick zero whatever the rhythm, so a crowded axis never
    /// ends up with no labels at all — which is the failure that would look exactly like a bug in
    /// the reader.
    /// </remarks>
    [Fact]
    public void AThinnedAxisStillDrawsItsFirstLabel()
    {
        ChartPlot plot = new()
        {
            Categories = ["Sep tem", "Sep tem", "Sep tem", "Sep tem",
                          "Sep tem", "Sep tem", "Sep tem", "Sep tem"],
            Series = [new ChartSeries("North", [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0],
                Colour.FromRgb(0x99CCFF))],
            CategoryAxisText = new ChartAxisText(LineBreakAllowed: true),
        };

        ChartDrawing drawing = ChartLayout.Place(
            plot,
            new DocRect(Length.Zero, Length.Zero, Length.FromPoints(200), Length.FromPoints(150)),
            new Ruler());

        int drawn = drawing.Labels.Count(label => label.Text == "Sep tem");
        drawn.ShouldBeGreaterThan(0);
        drawn.ShouldBeLessThan(8);
    }

    /// <summary>
    /// Rotating the labels takes room from the plot area rather than being free.
    /// </summary>
    /// <remarks>
    /// The point of the whole exercise: a 45° label is <c>W·sin θ + H·cos θ</c> deep, which on
    /// long names is several lines, and the plot rectangle gives up exactly that. Without the
    /// feedback the labels are turned and then drawn over the bars beneath them.
    /// </remarks>
    [Fact]
    public void RotatedLabelsShortenThePlotArea()
    {
        ChartSeries series = new(
            "North", [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0], Colour.FromRgb(0x99CCFF));

        string?[] names =
            ["Netherlands", "Denmark", "Norway", "France",
             "Austria", "UK", "Germany", "Spain"];

        DocRect frame =
            new(Length.Zero, Length.Zero, Length.FromPoints(300), Length.FromPoints(280));

        ChartPlot crowded = new() { Categories = names, Series = [series] };
        ChartPlot roomy = crowded with { CategoryAxisText = new ChartAxisText(OverlapAllowed: true) };

        ChartDrawing turned = ChartLayout.Place(crowded, frame, new Ruler());
        ChartDrawing upright = ChartLayout.Place(roomy, frame, new Ruler());

        turned.Labels.ShouldContain(label => label.Text == "Netherlands" && label.Rotation != 0.0);
        upright.Labels.ShouldContain(label => label.Text == "Netherlands" && label.Rotation == 0.0);

        turned.PlotArea.Bottom.ShouldBeLessThan(upright.PlotArea.Bottom);
    }
}
