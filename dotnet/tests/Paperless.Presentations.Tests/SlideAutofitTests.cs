using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Presentations.Layout;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// Shrink-to-fit: the sizes LibreOffice's own search arrives at, pinned one box at a time.
/// </summary>
/// <remarks>
/// <para>
/// Every expectation here is a measurement, not a derivation. The fixture is a probe deck of
/// plain text boxes — one shape per box height, an <c>a:normAutofit</c> on each, a throwaway
/// shape first so nothing measures the reference's shared-outliner state leak — converted by
/// <c>soffice --convert-to pdf</c>, with the drawn em size read back out of the content stream's
/// <c>Tf</c> operator. Across 227 such boxes at 25, 32 and 40 pt in four faces, Paperless now
/// agrees with the reference on 225.
/// </para>
/// <para>
/// The sizes look arbitrary and are not. The search bisects a font scale between nothing and
/// one, ten halvings deep, snapping each candidate down to a tenth of a point of a twelve-point
/// grid, and keeps the <em>closest fit at or above one</em> it saw anywhere — not the last one it
/// tried. So the answer is not monotonic in the box: a 28 pt box keeps 30 pt by tightening its
/// lines to four-fifths, and a 32 pt box, which has more room, drops to 27 pt at full spacing
/// because that is a closer fit than anything else the bisection visited. Both are what
/// LibreOffice draws.
/// </para>
/// <para>
/// The sizes are reported in hundredths of a millimetre rather than in points because that is
/// what they are: a whole number of points converted into the draw layer's own unit, so 27 pt is
/// 953 and the line it sits on is 1144. Comparing in exact points hides the one unit that decides
/// two of these cases — see <c>SlideTextLayout.Spacing</c>.
/// </para>
/// </remarks>
public class SlideAutofitTests
{
    /// <summary>The probe deck's box width, which is wide enough that nothing wraps.</summary>
    private static readonly Length Width = Length.FromPoints(60);

    /// <summary>
    /// One 40 pt line, in boxes from 20 to 48 pt, comes out at the sizes LibreOffice draws.
    /// </summary>
    /// <remarks>
    /// 48 pt is where the shrinking stops. A 40 pt line is 1.2 em — 48 pt — of box, and the
    /// comparison allows it 50 hundredths of a millimetre of slack
    /// (<c>aCurrentTextBoxSize.extendBy(0, -50)</c>), so 40 pt survives a box that is 1.417 pt
    /// short of its own line.
    /// </remarks>
    [Theory]
    [InlineData(20, 670)]
    [InlineData(24, 741)]
    [InlineData(28, 1058)]
    [InlineData(32, 953)]
    [InlineData(36, 1094)]
    [InlineData(40, 1199)]
    [InlineData(44, 1305)]
    [InlineData(48, 1411)]
    [InlineData(60, 1411)]
    public void OneLineShrinksToTheSizeTheReferenceDraws(double boxHeightPoints, long expectedMm100)
    {
        Drawn(Body(40, lines: 1), boxHeightPoints).Mm100.ShouldBe(expectedMm100);
    }

    /// <summary>
    /// Two 40 pt lines need twice the box, and shrink on the same grid.
    /// </summary>
    /// <remarks>
    /// The second line doubles the height the search compares but not the grid it walks, so the
    /// answers are not simply half the one-line ones: a 60 pt box keeps 25 pt where a 30 pt box
    /// on one line keeps 26.
    /// </remarks>
    [Theory]
    [InlineData(60, 882)]
    [InlineData(72, 1058)]
    [InlineData(96, 1411)]
    public void TwoLinesShrinkToTheSizeTheReferenceDraws(double boxHeightPoints, long expectedMm100)
    {
        Drawn(Body(40, lines: 2), boxHeightPoints).Mm100.ShouldBe(expectedMm100);
    }

    /// <summary>
    /// A body that does not ask for the fit keeps its size however small the box.
    /// </summary>
    /// <remarks>
    /// <c>a:normAutofit</c> is a choice in <c>EG_TextAutofit</c>, so a body stating
    /// <c>a:noAutofit</c> or nothing at all overflows its shape instead — which is what
    /// LibreOffice draws, and what makes the flag rather than the geometry the trigger.
    /// </remarks>
    [Fact]
    public void WithoutTheFlagNothingShrinks()
    {
        Drawn(Body(40, lines: 2) with { AutoFit = false }, boxHeightPoints: 20)
            .ShouldBe(Length.FromPoints(40));
    }

    /// <summary>
    /// A stated <c>fontScale</c> is applied only where no fit is solved.
    /// </summary>
    /// <remarks>
    /// The two are alternatives rather than a product: the reference reads
    /// <c>a:normAutofit/@fontScale</c> into a field it never reads again and searches for its own
    /// answer, so a body carrying both takes the search's. A body carrying only the scale — the
    /// ODF path, and a hand-built body — takes the scale, and it is not rounded to a whole point
    /// because nothing turned that rounding on.
    /// </remarks>
    [Fact]
    public void AStatedScaleAppliesOnlyWhenNoFitIsSolved()
    {
        SlideTextBody stated = Body(40, lines: 1) with { AutoFit = false, FontScale = 0.5 };

        Drawn(stated, boxHeightPoints: 200).ShouldBe(Length.FromPoints(20));

        // The same body asking for the fit in a box that needs none ignores the stated scale.
        Drawn(stated with { AutoFit = true }, boxHeightPoints: 200)
            .ShouldBe(Length.FromPoints(40));
    }

    /// <summary>
    /// A box with no height to give solves no fit rather than shrinking to nothing.
    /// </summary>
    [Fact]
    public void AnEmptyBoxLeavesTheTextAlone()
    {
        Drawn(Body(40, lines: 1), boxHeightPoints: 0).ShouldBe(Length.FromPoints(40));
    }

    /// <summary>
    /// Empty paragraphs at the end of a body are not part of what the fit measures.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>autoFitTextForCompatibility</c> measures with <c>Outliner::CalcTextSizeNTP</c>, and the
    /// <em>NTP</em> — no trailing paragraphs — is literal: <c>Calc1ColumnTextHeight</c> records the
    /// running bottom offset only <c>if (pHeightNTP &amp;&amp; !rInfo.rPortion.IsEmpty())</c>
    /// (<c>editeng/source/editeng/impedit2.cxx:3509</c> in 24.2), and <c>ParaPortion::IsEmpty</c>
    /// is one text portion of zero length (<c>editeng/inc/editdoc.hxx:640</c>).
    /// </para>
    /// <para>
    /// Measured, because the source alone would not have been believed: the eighth slide of
    /// <c>slides/batch-002/ppt/gfopportunitiesforlinkagespres_2010_en.ppt</c> carries four empty
    /// paragraphs after three bullets, and LibreOffice 24.2.7.2 fits its 26 pt text at 25 pt.
    /// Deleting those four paragraphs from LibreOffice's own flat-ODF export of the deck changes
    /// nothing at all; moving three of them into the middle of the body drops the same
    /// LibreOffice to 21 pt at nine-tenths spacing.
    /// </para>
    /// </remarks>
    [Fact]
    public void TrailingEmptyParagraphsAreNotMeasuredButInnerOnesAre()
    {
        // Three 40 pt lines need 144 pt of box; two need 96. A 100 pt box therefore fits the body
        // only if the third paragraph is not counted.
        SlideTextBody trailing = Body(40, lines: 2) with
        {
            Paragraphs = [.. Body(40, lines: 2).Paragraphs, Empty(40)],
        };

        Drawn(trailing, boxHeightPoints: 100).ShouldBe(Length.FromPoints(40));

        // The same empty paragraph between the two others is measured, so the body no longer fits
        // and the search shrinks it.
        SlideTextBody inner = trailing with
        {
            Paragraphs =
            [
                Body(40, lines: 1).Paragraphs[0], Empty(40), Body(40, lines: 1).Paragraphs[0],
            ],
        };

        Drawn(inner, boxHeightPoints: 100).ShouldBeLessThan(Length.FromPoints(40));
    }

    /// <summary>
    /// A fitted body is anchored by the height the fit measured, so a trailing blank line does
    /// not push it up; an unfitted one is anchored by all of its text.
    /// </summary>
    /// <remarks>
    /// Measured on the bottom-anchored subtitle of
    /// <c>slides/batch-001/pptx/BMFE-06-03 (Gerflor) Smoke Density and Toxicity.pptx</c>, whose
    /// three paragraphs end in an empty one. Deleting that paragraph from LibreOffice's flat-ODF
    /// export leaves the remaining line at byte-identical coordinates while the shape autofits,
    /// and moves it 33 pt once <c>style:shrink-to-fit</c> is turned off — so the exclusion belongs
    /// to the fit and not to empty paragraphs in general.
    /// </remarks>
    [Fact]
    public void OnlyAFittedBodyAnchorsByItsHeightToTheLastNonEmptyParagraph()
    {
        SlideTextBody body = Body(40, lines: 1) with
        {
            Anchor = TextAnchor.Bottom,
            Paragraphs = [.. Body(40, lines: 1).Paragraphs, Empty(40)],
        };

        // A 200 pt box needs no shrinking either way, so only the anchoring differs. The fitted
        // body puts its one line on the bottom of the box; the unfitted one leaves the blank line
        // below it and sits a line higher.
        Length fitted = Baseline(body, boxHeightPoints: 200);
        Length unfitted = Baseline(body with { AutoFit = false }, boxHeightPoints: 200);

        (fitted - unfitted).Points.ShouldBe(48.0, tolerance: 0.5);
    }

    /// <summary>Where the first glyph run's baseline lands.</summary>
    private static Length Baseline(SlideTextBody body, double boxHeightPoints)
    {
        DocRect area = new(
            Length.Zero, Length.Zero, Width, Length.FromPoints(boxHeightPoints));

        List<PlacedGlyphRun> placed = SlideTextLayout.Place(body, area, new SlideFonts());
        placed.ShouldNotBeEmpty();
        return placed[0].Run.Origin.Y;
    }

    /// <summary>An empty paragraph carrying one run, as every reader emits for a blank line.</summary>
    private static SlideParagraph Empty(double points) => new(
        string.Empty,
        [new SlideTextRun(0, 0, "Liberation Sans", Length.FromPoints(points), 400, false, Colour.Black)]);

    /// <summary>The em size the first glyph run is drawn at.</summary>
    private static Length Drawn(SlideTextBody body, double boxHeightPoints)
    {
        DocRect area = new(
            Length.Zero, Length.Zero, Width, Length.FromPoints(boxHeightPoints));

        List<PlacedGlyphRun> placed = SlideTextLayout.Place(body, area, new SlideFonts());

        placed.ShouldNotBeEmpty();
        return placed[0].Run.FontSize;
    }

    /// <summary>A body of single-character paragraphs at one size, asking for the fit.</summary>
    private static SlideTextBody Body(double points, int lines) => new()
    {
        AutoFit = true,
        Insets = new Margins(Length.Zero, Length.Zero, Length.Zero, Length.Zero),
        Paragraphs =
        [
            .. Enumerable.Range(0, lines).Select(_ => new SlideParagraph(
                "A",
                [
                    new SlideTextRun(
                        0, 1, "Liberation Sans", Length.FromPoints(points), 400, false,
                        Colour.Black),
                ])),
        ],
    };
}
