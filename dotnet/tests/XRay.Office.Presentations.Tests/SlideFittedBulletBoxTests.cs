using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Presentations.Layout;
using XRay.Office.Text.Layout;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// A fitted paragraph centres a <em>small</em> bullet in a box sized for the bullet it would
/// have had unscaled.
/// </summary>
/// <remarks>
/// <para>
/// <c>Outliner::ImpCalcBulletArea</c> takes the box's height from <c>ImplGetBulletSize</c>, which
/// caches it on the paragraph (<c>editeng/source/outliner/outliner.cxx</c>:1315-1355), and the
/// autofit search formats the same outliner once unscaled before walking
/// <c>constScaleLevels</c> (<c>editeng/source/editeng/impedit3.cxx</c>:303-333) — so the call
/// that fills the cache is the unscaled one. The bullet is still drawn at the scaled size,
/// because <c>StripBullet</c> asks <c>ImpCalcBulletFont</c> again (<c>:906-913</c>) and that
/// does multiply by <c>getScalingParameters().fFontY</c> (<c>:851-855</c>).
/// </para>
/// <para>
/// <strong>This checkout would not do it and 26.2.4.2 does.</strong> Here the cache is keyed on
/// the scaling parameters (<c>include/editeng/outliner.hxx</c>:154-172) and the checkout declares
/// <c>27.2.0.0.alpha0+</c>, so the source leg is a later version's. The measurement is
/// <c>probes/slides-r100/make-bullet-fit-probe.py</c>: one 24 pt bulleted body per slide in a box
/// swept 60…420 pt so the search answers a different row each time, read out of 26.2.4.2's own
/// PDF. On the ten slides it does not scale the bullet offset agrees to <strong>0.057 pt</strong>;
/// on the six it does, the box-centred rule puts the bullet 1.615 to 6.533 pt too high and the
/// residual is <c>(1 − fontScale) × unscaledBulletHeight / 2</c> at every one, over five distinct
/// font scales.
/// </para>
/// <para>
/// The body below is that in miniature. A 24 pt paragraph fitted to 0.400 draws its bullet at
/// <c>fround(847 × 0.4) = 339</c> hundredths of a millimetre; OpenSymbol's typographic ascent and
/// descent are 1420 and 442 on a 2048 em, so its own box is 305 units and the unscaled one is
/// 770. The line is <c>H = fround(fround(1.2 × 339) × 0.8) = 339</c> and
/// <c>TH = 424</c>, so the reference's
/// <c>H − TH + TH/2 − box/2 + box − 1 − descent</c> is
/// <c>339 − 424 + 212 − 385 + 770 − 1 − 72 = 439</c> units below the block's top against the
/// box-centred <c>208</c>, and the ascent it is measured from is 339 — so the bullet sits
/// <strong>2.83 pt below</strong> the text's baseline where the old rule put it 3.71 above.
/// </para>
/// </remarks>
public class SlideFittedBulletBoxTests
{
    private const string Face = "Liberation Sans";

    private static (double Bullet, double Body) Baselines(double boxHeightPoints)
    {
        SlideTextBody body = new()
        {
            Insets = new Margins(Length.Zero, Length.Zero, Length.Zero, Length.Zero),
            Wraps = true,
            AutoFit = true,
            Anchor = TextAnchor.Top,
            FontIndependentLineSpacing = true,
            Paragraphs =
            [
                .. Enumerable.Range(0, 6).Select(i => new SlideParagraph(
                    $"Hxy {i}",
                    [new SlideTextRun(0, 5, Face, Length.FromPoints(24), 400, false, Colour.Black)],
                    TextAlignment.Start)
                {
                    Marker = new SlideMarker("\u2022", "OpenSymbol", 1.0, null, IsSymbol: true),
                    StartIndent = Length.FromPoints(36),
                    FirstLineIndent = -Length.FromPoints(36),
                }),
            ],
        };

        List<PlacedGlyphRun> placed = SlideTextLayout.Place(
            body,
            new DocRect(Length.Zero, Length.Zero, Length.FromPoints(400),
                        Length.FromPoints(boxHeightPoints)),
            new SlideFonts());

        PlacedGlyphRun bullet = placed.First(r => r.Run.Font.FamilyName.Contains("OpenSymbol"));
        PlacedGlyphRun text = placed.First(r => !r.Run.Font.FamilyName.Contains("OpenSymbol"));

        return (bullet.Run.Origin.Y.Points, text.Run.Origin.Y.Points);
    }

    /// <summary>The box is far too small, so the fit answers 0.400 and the bullet drops below the
    /// text's own baseline.</summary>
    [Fact]
    public void AFittedBulletHangsBelowItsTextsBaseline()
    {
        (double bullet, double text) = Baselines(60);

        (bullet - text).ShouldBe(2.83, 0.06);
    }

    /// <summary>The control: a box big enough that nothing is scaled, where the rule reduces to
    /// centring the bullet against the line's text.</summary>
    [Fact]
    public void AnUnfittedBulletIsCentredAgainstTheLine()
    {
        (double bullet, double text) = Baselines(420);

        (bullet - text).ShouldBe(-3.88, 0.06);
    }
}
