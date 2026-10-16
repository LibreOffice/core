using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Presentations.Layout;
using XRay.Office.Text.Layout;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// A slide paragraph that states a line <em>height</em> rather than a proportion.
/// </summary>
/// <remarks>
/// <para>
/// EditEngine tests four line-spacing rules in order — <c>SvxLineSpaceRule::Min</c>,
/// <c>::Fix</c>, <c>SvxInterLineSpaceRule::Prop</c>, <c>::Off</c>
/// (<c>editeng/source/editeng/impedit3.cxx:1530-1602</c>) — and the first two set the height
/// <em>and move the ascent by the whole of its change</em>:
/// </para>
/// <code>
/// pLine-&gt;SetMaxAscent(pLine-&gt;GetMaxAscent() + (nFixHeight - nTxtHeight));
/// pLine-&gt;SetHeight(nFixHeight, nTxtHeight);
/// </code>
/// <para>
/// The expectations below are read off LibreOffice <strong>26.2.4.2</strong>'s own rendering of
/// <c>probes/slides-r53/make-linespace-probe.py</c> — four-line <c>a:noAutofit</c> boxes with
/// <c>tIns="0"</c>, so the first baseline's distance below the box top is the ascent. That deck
/// reports, for 12 pt Liberation Sans text:
/// </para>
/// <list type="table">
///   <item><description>stated 10 pt → first baseline 7.556 pt down, pitch 10.006</description></item>
///   <item><description>stated 24 pt → 21.559 pt down, pitch 24.009</description></item>
///   <item><description>stated 50 pt → 47.553 pt down, pitch 50.003</description></item>
/// </list>
/// <para>
/// The pitches are the tell that the unit is a hundredth of a millimetre rather than a twip: a
/// stated 24 pt line is drawn at <strong>24.009</strong>, which is 847 units, and the whole-twip
/// arithmetic this replaced gives a flat 24.000.
/// </para>
/// <para>
/// A hundredth of a point of tolerance on the pitch, and the project's usual tenth on the
/// baseline — the reference's PDF export shifts every pen by 0.0283 pt, which
/// <see cref="SlideTextPlacementTests"/> already records.
/// </para>
/// </remarks>
public class SlideStatedLineHeightTests
{
    private const string Face = "Liberation Sans";

    private static SlideTextBody Body(LineSpacingRule rule, double sizePoints)
    {
        Length size = Length.FromPoints(sizePoints);

        return new SlideTextBody
        {
            Insets = new Margins(Length.Zero, Length.Zero, Length.Zero, Length.Zero),
            Wraps = false,
            Anchor = TextAnchor.Top,
            FontIndependentLineSpacing = true,
            Paragraphs =
            [
                new SlideParagraph(
                    "Hxy0\u2028Hxy1\u2028Hxy2\u2028Hxy3",
                    [new SlideTextRun(0, 19, Face, size, 400, false, Colour.Black)],
                    TextAlignment.Start,
                    LineSpacing: rule),
            ],
        };
    }

    private static (double First, double Pitch) Baselines(SlideTextBody body)
    {
        List<PlacedGlyphRun> placed = SlideTextLayout.Place(
            body, new DocRect(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(400)),
            new SlideFonts());

        List<double> ys = placed.Select(run => run.Run.Origin.Y.Points).Distinct().Order().ToList();
        ys.Count.ShouldBe(4);

        return (ys[0], ys[1] - ys[0]);
    }

    [Theory]
    [InlineData(10.0, 7.556, 10.006)]
    [InlineData(24.0, 21.559, 24.009)]
    [InlineData(50.0, 47.553, 50.003)]
    public void AnExactHeightMovesTheAscentWithIt(double stated, double baseline, double pitch)
    {
        (double first, double gap) =
            Baselines(Body(LineSpacingRule.Exactly(Length.FromPoints(stated)), 12.0));

        first.ShouldBe(baseline, 0.1);
        gap.ShouldBe(pitch, 0.01);
    }

    /// <summary>
    /// A minimum height only grows a short line — the <c>::Min</c> arm's <c>if</c>, which is the
    /// only thing separating it from <c>::Fix</c>.
    /// </summary>
    /// <remarks>
    /// 12 pt text has a natural line of 14.4 pt, so a stated minimum of 10 leaves it alone and one
    /// of 24 takes over. Read off the same probe's <c>::Fix</c> rows, which is legitimate because
    /// the two arms are the same arithmetic once the guard has passed.
    /// </remarks>
    [Fact]
    public void AMinimumHeightBelowTheNaturalOneChangesNothing()
    {
        (double first, double gap) =
            Baselines(Body(LineSpacingRule.AtLeast(Length.FromPoints(10.0)), 12.0));

        gap.ShouldBe(14.4, 0.01);
        first.ShouldBe(12.0, 0.1);

        (double grown, double gapGrown) =
            Baselines(Body(LineSpacingRule.AtLeast(Length.FromPoints(24.0)), 12.0));

        gapGrown.ShouldBe(24.009, 0.01);
        grown.ShouldBe(21.559, 0.1);
    }

    /// <summary>
    /// Stating nothing is untouched, which is the control that says the new arm is not swallowing
    /// the ordinary case.
    /// </summary>
    [Fact]
    public void APlainParagraphIsUnchanged()
    {
        (double first, double gap) = Baselines(Body(LineSpacingRule.SingleSpaced, 12.0));

        gap.ShouldBe(14.4, 0.01);
        first.ShouldBe(12.0, 0.1);
    }
}
