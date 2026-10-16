using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Presentations.Layout;
using XRay.Office.Text.Layout;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// Shrink-to-fit: the sizes LibreOffice's own search arrives at, pinned one box at a time.
/// </summary>
/// <remarks>
/// <para>
/// Every expectation here is a measurement, not a derivation. The fixture is a probe deck of
/// plain text boxes — one shape per box height, an <c>a:normAutofit</c> on each, a throwaway
/// shape first so nothing measures the reference's shared-outliner state leak — converted by
/// <c>soffice --convert-to pdf</c>, with the drawn em size read back out of the content stream's
/// <c>Tf</c> operator. Across 227 such boxes at 25, 32 and 40 pt in four faces, XRay.Office agrees
/// with the reference on 225; across a further 66 that wrap and space their lines at 80 per cent
/// (<c>research/probes/slides-r15</c>), on 62.
/// </para>
/// <para>
/// <strong>Every expectation in this file was re-measured against 26.2.4.2 on 2026-08-20</strong>,
/// and fourteen of them moved. They had been measured against 24.2.7.2, whose fit was a bisection
/// over a font-scale grid; 25.2 replaced it with a walk down <c>constScaleLevels</c>, so the
/// sizes the reference can answer with are now eleven and not a continuum. The old values are
/// kept in the remarks beside each theory rather than deleted, because they are correct
/// measurements of a binary this project no longer renders against and the difference between
/// the two is the round's result.
/// </para>
/// <para>
/// The sizes still look arbitrary and still are not: they are <c>stated × level</c> rounded to a
/// whole point in the hundredth-of-a-millimetre domain, which is why a stated 30 pt gives 25 at
/// level 0.850 (25.5 rounding down) and 17 at level 0.550 (16.5 rounding up). The answer is
/// monotonic in the box, which the bisection's was not.
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
    /// Forty points as the draw layer holds it: <strong>1411</strong> hundredths of a millimetre,
    /// which is 39.9969 pt and not 40.
    /// </summary>
    /// <remarks>
    /// Every "nothing shrank" assertion below is written against this rather than against
    /// <c>Length.FromPoints(40)</c>, because a body that is not shrunk is still drawn at the size
    /// the model can hold — see <see cref="AnUnshrunkEmIsOnTheDrawLayersOwnGrid"/>.
    /// </remarks>
    private const long UnshrunkForty = 1411;

    /// <summary>
    /// One 40 pt line, in boxes from 20 to 48 pt, comes out at the sizes LibreOffice draws.
    /// </summary>
    /// <remarks>
    /// <para>
    /// 44 pt is where the shrinking stops, and it stops there rather than at 48 because
    /// <c>constScaleLevels</c>' first row keeps the font at one and takes the line spacing to
    /// nine-tenths: 40 pt of text on a 47.96 pt line does not fit a 44 pt box, and on a 43.17 pt
    /// line it does.
    /// </para>
    /// <para>
    /// <strong>Re-measured against the installed 26.2.4.2 on 2026-08-20.</strong> The previous
    /// expectations — 670, 741, 1058, 953, 1094, 1199, 1305, 1411, 1411 — were read off
    /// 24.2.7.2's own PDFs and are correct for the bisection that binary ran; they are not what
    /// this container's reference draws. The fixture is nine one-slide decks, one box each, a
    /// single "A" at 40 pt in a 60 pt-wide <c>a:normAutofit</c> box with zero insets
    /// (<c>dotnet/probes/slides-r52/make-fit-probe.py --text A --width 60</c>), and the values
    /// below are the <c>Tf</c> operators of <c>soffice --convert-to pdf</c>, converted to the
    /// draw layer's unit. Only 20, 48 and 60 pt are unchanged.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData(20, 670)]
    [InlineData(24, 882)]
    [InlineData(28, 988)]
    [InlineData(32, 1094)]
    [InlineData(36, 1199)]
    [InlineData(40, 1305)]
    [InlineData(44, 1411)]
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
    /// The second line doubles the height compared but not the table walked, so the answers are
    /// the one-line answers at half the box — 60 pt on two lines and 32 pt on one both land on
    /// 1094 — which is what a table walk gives and a search over a font-height grid did not.
    /// <strong>Re-measured against 26.2.4.2 on the same nine-deck fixture</strong>; the previous
    /// 882, 1058, 1411 were 24.2.7.2's.
    /// </remarks>
    [Theory]
    [InlineData(60, 1094)]
    [InlineData(72, 1199)]
    [InlineData(96, 1411)]
    public void TwoLinesShrinkToTheSizeTheReferenceDraws(double boxHeightPoints, long expectedMm100)
    {
        Drawn(Body(40, lines: 2), boxHeightPoints).Mm100.ShouldBe(expectedMm100);
    }

    /// <summary>
    /// A body nothing shrinks is still drawn on the draw layer's 1/100 mm grid, not at the exact
    /// number of points the file states.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The character height lives in an <c>SvxFontHeightItem</c> in the model's map unit, and for
    /// a draw object that unit is a hundredth of a millimetre — so a 20 pt run is 706 units and is
    /// drawn at <strong>20.0126 pt</strong>. Every advance width the reference measures, every
    /// line break it takes and every height the shrink-to-fit search compares is taken at that
    /// size.
    /// </para>
    /// <para>
    /// The expectations are the sizes LibreOffice 24.2.7.2's own PDFs carry, read off the
    /// <c>Tf</c> operator with <c>research/probes/slides-r17/size-census.py</c>: `20.01`, `24.01`
    /// and `28.01` where we wrote `20`, `24` and `28`. Over the forty documents
    /// <c>mm100-grid.py</c> checked, 82.27% of the reference's show operators sit on this grid
    /// against 45.81% of ours, and every one of the fifteen commonest offending sizes was a whole
    /// number of points.
    /// </para>
    /// <para>
    /// 13.33 pt is the case that separates the conversion the property setter performs — points to
    /// twips to hundredths of a millimetre — from a direct ratio. 13.33 pt is 267 twips and
    /// therefore 471 units; the direct ratio gives 470. See <c>SlideAutofit.Quantised</c>.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData(20, 706)]
    [InlineData(24, 847)]
    [InlineData(28, 988)]
    [InlineData(40, 1411)]
    [InlineData(13.33, 471)]
    public void AnUnshrunkEmIsOnTheDrawLayersOwnGrid(double statedPoints, long expectedMm100)
    {
        // A box far taller than the text, so the search returns before it shrinks anything and
        // the only thing under test is the size the run is drawn at.
        Length drawn = Drawn(Body(statedPoints, lines: 1), boxHeightPoints: 400);

        drawn.Mm100.ShouldBe(expectedMm100);
        drawn.Emu.ShouldBe(expectedMm100 * Length.EmuPerMm100);
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
            .Mm100.ShouldBe(UnshrunkForty);
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

        Drawn(stated, boxHeightPoints: 200).Mm100.ShouldBe(706);

        // The same body asking for the fit in a box that needs none ignores the stated scale.
        Drawn(stated with { AutoFit = true }, boxHeightPoints: 200).Mm100.ShouldBe(UnshrunkForty);
    }

    /// <summary>
    /// A box with no height to give solves no fit rather than shrinking to nothing.
    /// </summary>
    [Fact]
    public void AnEmptyBoxLeavesTheTextAlone()
    {
        Drawn(Body(40, lines: 1), boxHeightPoints: 0).Mm100.ShouldBe(UnshrunkForty);
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

        Drawn(trailing, boxHeightPoints: 100).Mm100.ShouldBe(UnshrunkForty);

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

    /// <summary>
    /// A wrapping body at 80 per cent line spacing lands on the size and the spacing scale the
    /// reference chooses, including the four boxes a whole-point candidate grid gets wrong.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The fixture is <c>tests/corpus/features/slide-autofit-grid.pptx</c>, one 360 pt-wide box
    /// per slide holding three paragraphs of the same sentence at 20 pt with
    /// <c>a:lnSpc/a:spcPct val="80000"</c> and <c>a:normAutofit</c>. Every expectation below is
    /// read out of <c>soffice --convert-to pdf</c>'s own content stream — the em size from
    /// <c>Tf</c> and the spacing scale from the baseline pitch over
    /// <c>1.2 x size x 0.8</c> — with <c>research/probes/slides-r15/read-autofit.py</c>.
    /// </para>
    /// <para>
    /// <strong>The seven heights are one per level the fixture can reach, and two of them are the
    /// pair that separates a level table from anything else.</strong> 120 pt and 135 pt both draw
    /// <em>17 pt</em> — level 0.850 — and differ only in the spacing that comes with it, 0.80 and
    /// 0.90, because <c>constScaleLevels</c> holds 0.850 twice. No search over a font scale can
    /// produce two different spacings at one size, so this pair fails against every reading of the
    /// old bisection and against a font-scale table without the second column.
    /// </para>
    /// <para>
    /// The whole fixture was re-measured on 26.2.4.2 and all 23 of its slides agree with us on
    /// both size and pitch to 0.0007 pt. The previous six expectations — 90/459/10.006,
    /// 110/494/12.103, 135/600/14.683, 150/600/16.327, 175/670/14.598, 200/670/16.412 — are
    /// 24.2.7.2's, and three of the six heights it used (135, 150, 175) now give the same answer,
    /// which is why the heights moved as well as the values.
    /// </para>
    /// <para>
    /// <strong>The pitch is asserted to a thousandth of a point, which is the precision the
    /// reference values are recorded to, and it is a second independent assertion rather than a
    /// loose one.</strong> It used to be allowed a twentieth of a point, because a proportional
    /// line height went through whole twips and then took the fit's spacing scale in a separate
    /// rounding. Both are now one rounding of the product in hundredths of a millimetre — see
    /// <c>SlideTextLayout.Proportioned</c> — and the residual error across the six cases is at
    /// most 0.00094 pt.
    /// </para>
    /// <para>
    /// So this tolerance is what distinguishes the three candidate arithmetics, and it was
    /// verified by putting each back:
    /// </para>
    /// <list type="bullet">
    /// <item><description>whole twips, as shipped before: worst case 0.032 pt out (135 pt box),
    /// none of the six inside a thousandth.</description></item>
    /// <item><description>hundredths of a millimetre but rounding the two factors separately:
    /// worst case 0.017 pt (175 pt box), one of the six inside a thousandth — and *worse* than
    /// whole twips on two of them.</description></item>
    /// <item><description>one rounding of the product: all six inside a thousandth.</description></item>
    /// </list>
    /// </remarks>
    [Theory]
    [InlineData(90, 388, 8.447)]
    [InlineData(95, 459, 10.006)]
    [InlineData(100, 494, 10.771)]
    [InlineData(115, 564, 12.274)]
    [InlineData(120, 600, 13.067)]
    [InlineData(135, 600, 14.683)]
    [InlineData(200, 670, 16.412)]
    public void AWrappingBodyLandsOnTheReferencesSizeAndSpacing(
        double boxHeightPoints, long expectedMm100, double expectedPitchPoints)
    {
        SlideTextBody body = Wrapping(20, paragraphs: 3, proportion: 0.8);
        DocRect area = new(
            Length.Zero, Length.Zero, Length.FromPoints(360), Length.FromPoints(boxHeightPoints));

        List<PlacedGlyphRun> placed = SlideTextLayout.Place(body, area, new SlideFonts());
        placed.ShouldNotBeEmpty();

        placed[0].Run.FontSize.Mm100.ShouldBe(expectedMm100);
        Pitch(placed).Points.ShouldBe(expectedPitchPoints, tolerance: 0.001);
    }

    /// <summary>
    /// The fit's spacing scale reaches a paragraph's own space, not only its lines.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The wiring of <c>SlideTextLayout.ScaledSpace</c>, so a machine with no LibreOffice still
    /// covers it. What the scale <em>should</em> be is measured against the reference's own PDF
    /// in <c>SlideAutofitParagraphSpaceComparisonTests</c>.
    /// </para>
    /// <para>
    /// The body is <see cref="AWrappingBodyLandsOnTheReferencesSizeAndSpacing"/>'s — three
    /// wrapping paragraphs at 20 pt stating 80 per cent line spacing — with a 12 pt space above
    /// each. <strong>The three box heights are one per spacing scale a level can carry</strong>,
    /// which is what makes this able to fail: at 300 pt nothing overflows and the space is
    /// untouched, at 220 pt the walk stops on a nine-tenths row and at 120 pt on a four-fifths
    /// row. A box at full spacing alone would pass under either reading. The heights moved with
    /// the level table (they were 175, 220 and 200); the three expectations did not, because they
    /// are the wiring and not the fit.
    /// </para>
    /// <para>
    /// 12 pt is 423.33 hundredths of a millimetre; the draw layer holds it as <strong>423</strong>
    /// and the two scaled values are <c>trunc(423 x 0.9) = 380</c> and <c>trunc(423 x 0.8) = 338</c>.
    /// The gap between the last line of one paragraph and the first of the next is one line plus
    /// that space, so subtracting the pitch leaves the space alone.
    /// </para>
    /// <para>
    /// <strong>The first two of those moved in round 85, and both halves of the move are the same
    /// citation.</strong> They were 424 and 381 — an unquantised space, and a scale that rounded.
    /// <c>ImpEditEngine::CalcHeight</c> reads the item into an integer of the model's own unit and
    /// then assigns the scaled <c>double</c> to another one —
    /// <c>sal_uInt16 nUpper = scaleYSpacingValue(rULItem.GetUpper())</c>,
    /// <c>editeng/source/editeng/impedit2.cxx</c>:4792-4802 — so the value is a whole unit before
    /// the scale and the fraction is dropped after it. Measured at the reference on page 2 of
    /// <c>2015-Civil-Rights-Website-training.ppt</c>, whose 282-unit space comes back as
    /// <strong>253</strong> and not 254; over that slide's seven gaps the difference is 0.198 pt
    /// in where the last baseline lands. The 120 pt row is the control that does not move, because
    /// <c>423 x 0.8</c> is 338.4 and both readings answer 338.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData(300, 423)]
    [InlineData(220, 380)]
    [InlineData(120, 338)]
    public void TheFitsSpacingScaleReachesAParagraphsOwnSpace(
        double boxHeightPoints, long expectedSpaceMm100)
    {
        SlideTextBody body = Wrapping(20, paragraphs: 3, proportion: 0.8) with { };
        body = body with
        {
            Paragraphs = [.. body.Paragraphs.Select(
                p => p with { SpaceBefore = Length.FromPoints(12) })],
        };

        DocRect area = new(
            Length.Zero, Length.Zero, Length.FromPoints(360), Length.FromPoints(boxHeightPoints));

        List<PlacedGlyphRun> placed = SlideTextLayout.Place(body, area, new SlideFonts());
        placed.ShouldNotBeEmpty();

        List<long> baselines = [.. placed.Select(p => p.Run.Origin.Y.Mm100).Distinct().Order()];
        baselines.Count.ShouldBeGreaterThan(3);

        long pitch = Pitch(placed).Mm100;
        long widest = 0;
        for (int i = 1; i < baselines.Count; i++)
        {
            widest = Math.Max(widest, baselines[i] - baselines[i - 1]);
        }

        (widest - pitch).ShouldBe(expectedSpaceMm100, $"the space above a paragraph in a {boxHeightPoints} pt box");
    }

    /// <summary>
    /// A body that overflows its box many times over is drawn at a quarter of its stated size,
    /// not at whatever the search's interval happens to reach.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>Without the floor this is not "too small" — it is nothing at all.</strong> The
    /// bisection's interval is <c>[0, 1]</c> and a body that overflows by a factor of twenty
    /// drives it into the thousandths, where <see cref="SlideTextLayout"/>'s rounding of a scaled
    /// em to a whole point rounds it to <em>zero</em>: every run is laid out and drawn at an em of
    /// nothing and the page receives no text-showing operator for the body. Measured on
    /// <c>NWD-GLA-Community-Outreach-Day-Oct-2025.pptx</c>, whose slides 5, 6 and 12 drew their
    /// titles and no subtitle whatsoever — 529 extractable words against the reference's 638.
    /// </para>
    /// <para>
    /// Every expected value is read off the banked 26.2.4.2 rendering of that deck rather than
    /// derived here. Its subtitle placeholders are 1152128 EMU tall — 90.7 pt — and the sizes
    /// below are the ones they state: 52 and 60 pt on slide 5, 60, 72 and 88 on slide 6, 77 on
    /// slide 12. The reference draws each at stated × 0.250, the last row of
    /// <c>constScaleLevels</c>, and lets the text overflow: <c>/F1 18.992 Tf</c> for the 77 pt
    /// runs, and glyph boxes of 26.8, 22.0, 18.3 and 15.9 units for the 88, 72, 60 and 52 pt ones.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData(52, 459)]
    [InlineData(60, 529)]
    [InlineData(72, 635)]
    [InlineData(77, 670)]
    [InlineData(88, 776)]
    public void AnOverflowingBodyStopsAtAQuarterAndOverflows(double points, long expectedMm100)
    {
        Drawn(Body(points, lines: 17), boxHeightPoints: 90.7).Mm100.ShouldBe(expectedMm100);
    }

    /// <summary>
    /// However badly a body overflows, the em it is drawn at is never zero.
    /// </summary>
    /// <remarks>
    /// The invariant the floor exists for, asserted over the whole range rather than at the five
    /// geometries the corpus happened to hold. A drawn em of zero is not a small rendering of the
    /// text; it is the silent loss of it, and nothing downstream of the fit can tell the two
    /// apart — the runs are still there, still shaped, still placed, and draw no ink.
    /// </remarks>
    [Fact]
    public void NoBodyIsEverScaledToNothing()
    {
        for (int lines = 1; lines <= 60; lines++)
        {
            foreach (double points in new[] { 12.0, 28.0, 44.0, 60.0, 88.0 })
            {
                Length drawn = Drawn(Body(points, lines), boxHeightPoints: 90.7);

                drawn.ShouldBeGreaterThan(
                    Length.Zero, $"{lines} lines of {points} pt in a 90.7 pt box");

                double scale = (double)drawn.Mm100 / Quantised(points);
                scale.ShouldBeGreaterThan(0.24, $"{lines} lines of {points} pt");
            }
        }
    }

    /// <summary>The draw layer's holding of a whole number of points, as the fit's grid sees it.</summary>
    private static double Quantised(double points)
        => (((long)((points * 20.0) + 0.5) * 127) + 36) / 72;

    /// <summary>The smallest gap between two distinct baselines, which is one line's height.</summary>
    /// <remarks>
    /// The smallest rather than the first, because a paragraph boundary carries the paragraph's
    /// own spacing on top of the line and would measure that instead.
    /// </remarks>
    private static Length Pitch(List<PlacedGlyphRun> placed)
    {
        List<double> baselines = [.. placed.Select(p => p.Run.Origin.Y.Emu).Distinct().Order()
            .Select(y => (double)y)];

        baselines.Count.ShouldBeGreaterThan(1);

        double smallest = double.MaxValue;
        for (int i = 1; i < baselines.Count; i++)
        {
            smallest = Math.Min(smallest, baselines[i] - baselines[i - 1]);
        }

        return Length.FromEmu((long)smallest);
    }

    /// <summary>
    /// The probe deck's body: three paragraphs of one long sentence at one size, wrapping.
    /// </summary>
    /// <remarks>
    /// The sentence and the face are the fixture's, so the line counts are the ones both
    /// renderers were measured to agree on — six lines up to 12 pt, nine to 17 and twelve beyond.
    /// A wrapping body is what makes the fit search interesting: the line count changes under it,
    /// so the height is not linear in the size and the search's path decides the answer.
    /// </remarks>
    private static SlideTextBody Wrapping(double points, int paragraphs, double proportion)
    {
        const string sentence =
            "Proficient in more than one language and able to convey meaning "
            + "accurately between two parties without adding or omitting anything";

        return new SlideTextBody
        {
            AutoFit = true,
            FontIndependentLineSpacing = true,
            Insets = new Margins(Length.Zero, Length.Zero, Length.Zero, Length.Zero),
            Paragraphs =
            [
                .. Enumerable.Range(0, paragraphs).Select(_ => new SlideParagraph(
                    sentence,
                    [
                        new SlideTextRun(
                            0, sentence.Length, "Liberation Sans", Length.FromPoints(points), 400,
                            false, Colour.Black),
                    ],
                    LineSpacing: LineSpacingRule.Multiple(proportion))),
            ],
        };
    }

    /// <summary>
    /// An autofitted body measures its lines at the reference device's realisation of the em; a
    /// plain one measures them at the em.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Every expected value is read out of a reference PDF —
    /// <c>research/probes/slides-r21/make-pitch-probe.py</c>, one slide per size, the same three
    /// paragraphs in an <c>a:noAutofit</c> box and in an <c>a:normAutofit</c> box far too tall to
    /// shrink, so the fit settles on scale 1 in every case. Over the probe's 53 sizes the plain
    /// column is <c>fround(em × 1.2)</c> every time and the autofitted column differs on 34.
    /// </para>
    /// <para>
    /// <strong>12 pt is the control and it passes either way</strong>: 423 units through the
    /// 600 dpi grid comes back 423, so its two columns agree and no reading of this method can
    /// make it fail. The other four bite in both directions — the autofitted line is longer at 8
    /// and 20 pt and shorter at 10 and 28 — which is what rules out a missing multiplier.
    /// </para>
    /// <para>
    /// The <c>plain</c> column also covers the twip round trip <c>SlideTextLayout.Spacing</c> used
    /// to take: 8, 10 and 28 pt are three of the sizes it moved, to 338.67, 423.33 and 1185.2.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData(8, 338, 341)]
    [InlineData(10, 424, 421)]
    [InlineData(12, 508, 508)]
    [InlineData(20, 847, 848)]
    [InlineData(28, 1186, 1183)]
    public void AnAutofittedBodyMeasuresItsLinesOnTheDevicesGrid(
        double points, long plainMm100, long autofittedMm100)
    {
        SlideTextBody plain = Body(points, lines: 4) with
        {
            AutoFit = false, FontIndependentLineSpacing = true,
        };
        SlideTextBody autofitted = Body(points, lines: 4) with
        {
            AutoFit = true, FontIndependentLineSpacing = true,
        };

        // Tall enough that the search cannot want to shrink either of them.
        const double boxHeightPoints = 400;

        Pitch(Placed(plain, boxHeightPoints)).Mm100.ShouldBe(plainMm100, $"a plain {points} pt line");
        Pitch(Placed(autofitted, boxHeightPoints)).Mm100
            .ShouldBe(autofittedMm100, $"an autofitted {points} pt line");
    }

    /// <summary>The glyph runs a body lays out in a box of the given height.</summary>
    private static List<PlacedGlyphRun> Placed(SlideTextBody body, double boxHeightPoints)
    {
        DocRect area = new(
            Length.Zero, Length.Zero, Width, Length.FromPoints(boxHeightPoints));

        List<PlacedGlyphRun> placed = SlideTextLayout.Place(body, area, new SlideFonts());
        placed.ShouldNotBeEmpty();
        return placed;
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
