using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Presentations.Layout;
using XRay.Office.Text.Layout;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// A fitted paragraph's first line clears the bullet it would have had <em>unscaled</em>.
/// </summary>
/// <remarks>
/// <para>
/// The width half of the rule <see cref="SlideFittedBulletBoxTests"/> establishes for the height,
/// and it comes out of the same cache: <c>Outliner::ImplGetBulletSize</c> measures the bullet once
/// and stores the whole <c>Size</c> on the paragraph, and the autofit search formats the outliner
/// once unscaled before it walks <c>constScaleLevels</c> — so the call that fills the cache is the
/// unscaled one. EditEngine then takes that width for the first line alone,
/// <c>nStartX = max(textLeft + firstLineOffset, bulletX)</c>
/// (<c>editeng/source/editeng/impedit3.cxx</c>:846-851 over the <c>BulletX</c> set at
/// <c>:798-802</c>), while <c>StripBullet</c> asks <c>ImpCalcBulletFont</c> again and <em>does</em>
/// scale what it draws.
/// </para>
/// <para>
/// So a hanging indent wide enough for the shrunk bullet can still be too narrow for the cached
/// one, and the first line is pushed right by the difference while the bullet glyph is not.
/// </para>
/// <para>
/// Measured on <c>Sector_Skills_Insights_Advanced_Manufacturing_summary_slide_pack.pptx</c> page 13
/// against 26.2.4.2 (<c>probes/slidebullet-r191</c>), whose master states
/// <c>lvl2 marL="742950" indent="-285750"</c> and whose runs state 56 pt under a
/// <c>normAutofit</c> that draws them at 14: the reference starts every second-level first line
/// <strong>31.13 pt</strong> past the bullet, which is an en dash at 56 pt and not the 7.79 pt it
/// is at 14. Pinned over twelve one-attribute variants of that one master — four hanging indents,
/// two margins, three bullet characters and three stated run sizes, every one exact — and the
/// bullet's own stated size and the file's <c>fontScale</c> both decide nothing, the second because
/// LibreOffice recomputes the autofit rather than honouring it.
/// </para>
/// <para>
/// <strong>The two box heights below are the assertion, not the number.</strong> They fit to
/// different scales, so a reach taken at the drawn size would move between them and a reach taken
/// at the cached size cannot. The number is there as well because a reach that is wrong by a
/// constant would also be stable.
/// </para>
/// </remarks>
public class SlideFittedBulletReachTests
{
    private const string Face = "Liberation Sans";

    /// <summary>The level's margin, as <c>marL="742950"</c> states it.</summary>
    private const double MarginPoints = 58.5;

    /// <summary>Its hanging indent, as <c>indent="-285750"</c> states it.</summary>
    private const double HangingPoints = 22.5;

    /// <summary>
    /// Liberation Sans' en dash at 56 pt — <c>1139/2048</c> of an em, so 31.14 pt, against the
    /// 31.13 read off 26.2.4.2's own page.
    /// </summary>
    private const double ReachPoints = 31.14;

    /// <summary>
    /// The x of every line the body drew, and the scale the fit chose, in one pass.
    /// </summary>
    /// <remarks>
    /// The marker names its own typeface, so its width is Liberation Sans' rather than a
    /// substitute's — which is what makes <see cref="ReachPoints"/> a stated metric rather than a
    /// measurement of whatever the machine happened to resolve.
    /// </remarks>
    /// <param name="boxHeightPoints">The box's height, which is what decides the fit.</param>
    /// <param name="symbol">
    /// Whether the marker is a fixed character. A generated number is measured at the size it is
    /// drawn at instead, because its text differs per paragraph and refills the cache after the fit
    /// has a scale — see <c>SlideTextLayout.Unscaled</c>.
    /// </param>
    private static (List<double> Lefts, double Size) Lines(
        double boxHeightPoints, bool symbol = true)
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
                .. Enumerable.Range(0, 4).Select(_ => new SlideParagraph(
                    "One two three four five six seven eight nine ten eleven twelve thirteen",
                    [
                        new SlideTextRun(
                            0, 70, Face, Length.FromPoints(56), 400, false, Colour.Black),
                    ],
                    TextAlignment.Start)
                {
                    Marker = new SlideMarker("–", Face, 1.0, null, IsSymbol: symbol),
                    StartIndent = Length.FromPoints(MarginPoints),
                    FirstLineIndent = -Length.FromPoints(HangingPoints),
                }),
            ],
        };

        List<PlacedGlyphRun> placed = SlideTextLayout.Place(
            body,
            new DocRect(Length.Zero, Length.Zero, Length.FromPoints(300),
                        Length.FromPoints(boxHeightPoints)),
            new SlideFonts());

        // The bullet is one glyph of the same face at the same place on every paragraph; the lines
        // are everything else, in the order they were drawn.
        List<double> lefts = [.. placed.Where(run => run.Run.Glyphs.Count > 1)
                                       .Select(run => run.Run.Origin.X.Points)];

        double size = placed.Count == 0 ? 0.0 : placed[0].Run.FontSize.Points;
        return (lefts, size);
    }

    /// <summary>
    /// The first line of each paragraph starts a whole unscaled bullet past the bullet's own pen,
    /// and the lines after it start at the margin.
    /// </summary>
    [Theory]
    [InlineData(120)]
    [InlineData(200)]
    [InlineData(400)]
    public void AFittedParagraphsFirstLineClearsItsUnscaledBullet(double boxHeightPoints)
    {
        (List<double> lefts, _) = Lines(boxHeightPoints);

        lefts.Count.ShouldBeGreaterThan(4);

        double first = MarginPoints - HangingPoints + ReachPoints;

        // Every distinct left edge is one of the two, and both are present: the first lines at the
        // bullet's own reach and the continuations at the margin.
        foreach (double left in lefts)
        {
            (Math.Abs(left - first) < 0.1 || Math.Abs(left - MarginPoints) < 0.1)
                .ShouldBeTrue($"unexpected left edge {left:F2}");
        }

        lefts.ShouldContain(left => Math.Abs(left - first) < 0.1);
        lefts.ShouldContain(left => Math.Abs(left - MarginPoints) < 0.1);
    }

    /// <summary>
    /// A generated number's label is measured at the size it is drawn at, so a hanging indent wide
    /// enough for it keeps the first line at the margin.
    /// </summary>
    /// <remarks>
    /// The control that keeps the rule off the wrong markers, and one corpus deck needs it:
    /// <c>30-04-2021 merged NDoH and NICD_Presentation HBV BD meeting 05May2021_1.pptx</c> page 9
    /// numbers its second level and 26.2.4.2 starts those lines at the margin, where taking the
    /// unscaled width puts them 2.84 pt right of it.
    /// </remarks>
    [Fact]
    public void ANumberedParagraphsFirstLineStaysAtTheMargin()
    {
        (List<double> lefts, _) = Lines(120, symbol: false);

        lefts.ShouldAllBe(left => Math.Abs(left - MarginPoints) < 0.1);
    }

    /// <summary>
    /// The three boxes fit to more than one size, so the assertion above is a statement about the
    /// cached width rather than a coincidence.
    /// </summary>
    /// <remarks>
    /// More than one rather than three: the fit walks a fixed table of scale levels, so two box
    /// heights can land on the same row and the count is not the point — that the reach does not
    /// move between two rows is.
    /// </remarks>
    [Fact]
    public void TheBoxesFitToMoreThanOneSize()
    {
        double[] sizes = [Lines(120).Size, Lines(200).Size, Lines(400).Size];

        sizes.Distinct().Count().ShouldBeGreaterThan(1);
        sizes.ShouldAllBe(size => size > 0.0 && size < 56.0);
    }
}
