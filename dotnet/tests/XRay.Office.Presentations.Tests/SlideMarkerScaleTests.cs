using XRay.Office.Core.Extraction;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.Presentations.Layout;
using XRay.Office.Presentations.MsBinary;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// A shrink-to-fit body draws its bullet and its text at <em>different</em> sizes, because only
/// the text goes through the round-to-whole-point.
/// </summary>
/// <remarks>
/// <para>
/// <c>Outliner::setRoundFontSizeToPt</c> — which the fit turns on and nothing else does — rounds a
/// run's scaled height to a whole point. <c>Outliner::ImpCalcBulletFont</c> never reaches it:
/// <c>fround(aStdFont.GetFontSize().Height() × GetBulletRelSize()/100 × fFontY)</c>, one
/// multiplication and one round, taken in the model's own hundredths of a millimetre
/// (<c>editeng/source/outliner/outliner.cxx:851-855</c>).
/// </para>
/// <para>
/// Measured on <c>slides/done-006/ppt/Lepore.ppt</c> page 2, one stated 24 pt body at a fit of
/// 0.850: the reference draws eleven text baselines at <strong>20.013 pt</strong> — that is
/// <c>round(24 × 0.85) = 20</c> — and six bullets at <strong>20.409 pt</strong>, which is
/// <c>fround(847 × 0.85) = 720</c> hundredths of a millimetre, 24 × 0.85 unrounded. The pair on
/// one page is what identifies the rule; either figure alone is consistent with several.
/// </para>
/// </remarks>
public class SlideMarkerScaleTests
{
    private const uint StatesFontHeight = 0x0002_0000;

    private static readonly string Text = string.Join(
        PptTextReader.ParagraphSeparator,
        Enumerable.Repeat(
            "Survey results of the economic impact of orbital debris standard compliance", 6));

    /// <summary>Six bulleted paragraphs of 24 pt text, in a body the fit has to shrink.</summary>
    private static SlideTextBody Body()
    {
        PptTextRun run = new(
            PptTextKind.Body,
            Text,
            [new PptParagraphRun(
                Text.Length + 1, Depth: 0, HasBullet: true, BulletCharacter: '•')],
            [new PptCharacterRun(
                Text.Length, RunEmphasis.None, RunEmphasis.None,
                Mask: StatesFontHeight, FontHeight: 24)]);

        SlideTextBody body = PptTextBody.Build(
            run, styles: null, PptColourScheme.Default, PptFontTable.Empty,
            SlideTextBody.DefaultInsets, TextAnchor.Top, wraps: true).ShouldNotBeNull();

        return body with { Insets = default, AutoFit = true };
    }

    /// <summary>The distinct <c>/Tf</c> sizes the body draws, in points, largest first.</summary>
    private static List<double> Sizes(SlideTextBody body, double heightPoints)
    {
        DocRect area = new(
            Length.FromPoints(0), Length.FromPoints(0),
            Length.FromPoints(300), Length.FromPoints(heightPoints));

        List<double> sizes = [];
        foreach (PlacedGlyphRun run in SlideTextLayout.Place(body, area, new SlideFonts()))
        {
            double size = run.Run.FontSize.Points;
            if (!sizes.Exists(s => Math.Abs(s - size) < 0.0005)) sizes.Add(size);
        }

        sizes.Sort();
        sizes.Reverse();
        return sizes;
    }

    /// <summary>
    /// Unfitted, the bullet and the text are the same size — the control that says the two sizes
    /// below come from the fit and not from the marker path in general.
    /// </summary>
    [Fact]
    public void AnUnfittedBodyDrawsItsBulletAtItsTextSize()
    {
        Sizes(Body() with { AutoFit = false }, 600).Count.ShouldBe(1);
    }

    /// <summary>
    /// Fitted, there are exactly two: the text rounded to a whole point and the bullet not.
    /// </summary>
    [Fact]
    public void AFittedBodyDrawsItsBulletUnroundedAndItsTextRounded()
    {
        // 150 pt of room for six paragraphs of 24 pt text drives the fit down the table.
        List<double> sizes = Sizes(Body(), 150);

        sizes.Count.ShouldBe(2);

        // Which of the two is the larger is not fixed, and asserting that it is was this test's
        // first cut and its first failure: the run's size is `round(stated x scale)`, so it lands
        // ABOVE the bullet's unrounded `stated x scale` whenever the fraction is a half or more.
        // On this fixture the fit answers 0.400 and 24 x 0.400 = 9.6 rounds UP to 10, so the
        // bullet is the smaller. What is invariant is that exactly one of them is a whole number
        // of points.
        double whole = sizes.Find(s => Math.Abs(s - Math.Round(s)) < 0.02);
        double fraction = sizes.Find(s => Math.Abs(s - Math.Round(s)) >= 0.02);

        whole.ShouldNotBe(0);
        fraction.ShouldNotBe(0);

        // And that they are within one rounding step of each other, so this cannot pass on a
        // body that happens to draw two unrelated sizes.
        Math.Abs(whole - fraction).ShouldBeLessThan(0.51);
    }
}
