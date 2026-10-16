using XRay.Office.Core.Extraction;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.Presentations.Layout;
using XRay.Office.Presentations.MsBinary;
using XRay.Office.Text.Layout;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// A binary PowerPoint paragraph that <em>states</em> its line spacing is exempt from the
/// shrink-to-fit's line-spacing reduction, and one that states none is not.
/// </summary>
/// <remarks>
/// <para>
/// EditEngine tests four rules in order and stops at the first that applies
/// (<c>editeng/source/editeng/impedit3.cxx:1528-1602</c>): <c>SvxLineSpaceRule::Min</c>,
/// <c>::Fix</c>, <c>SvxInterLineSpaceRule::Prop</c>, <c>::Off</c>. The <c>Prop</c> arm does
/// nothing at all when the proportion is exactly 100, so the <c>::Off</c> arm — the only place a
/// paragraph picks up the fit's <c>fSpacingY</c> — is unreachable for a paragraph that states
/// exactly 100%. "States exactly 100%" and "states nothing" are different answers.
/// </para>
/// <para>
/// It is binary-only. Every OOXML and ODF line spacing reaches the item through
/// <c>SvxLineSpacingItem::PutValue</c> (<c>editeng/source/items/paraitem.cxx:194-202</c>), which
/// writes <c>eInterLineSpaceRule = Off</c> when the height is exactly 100; the <c>.ppt</c>
/// importer calls <c>SetPropLineSpace(100)</c> directly
/// (<c>filter/source/msfilter/svdfppt.cxx:6285-6288</c>) and <c>lspcitem.hxx:86-91</c> shows that
/// setter writes <c>Prop</c> unconditionally.
/// </para>
/// <para>
/// Measured on an authored known-answer deck (<c>probes/slides-r54/</c>): fifteen fitted boxes of
/// one text, the <c>.pptx</c> half drawing a baseline pitch of <c>1.2 × 0.8 × em</c> and the
/// <c>.ppt</c> half <c>1.2 × em</c>.
/// </para>
/// </remarks>
public class PptStatedLineSpacingTests
{
    /// <summary>The mask bit for a stated font height — <c>PPT_CharAttr_FontHeight</c>.</summary>
    private const uint StatesFontHeight = 0x0002_0000;

    /// <summary>The mask bit for a stated typeface index — <c>PPT_CharAttr_Font</c>.</summary>
    private const uint StatesFontIndex = 0x0001_0000;

    /// <summary>The mask bit for a stated line feed — <c>PPT_ParaAttr_LineFeed</c>.</summary>
    private const uint StatesLineFeed = 0x0000_1000;

    private static readonly string Text = string.Join(
        PptTextReader.ParagraphSeparator,
        "Proficient in more than one language and able to convey meaning accurately",
        "Proficient in more than one language and able to convey meaning accurately",
        "Proficient in more than one language and able to convey meaning accurately");

    /// <summary>
    /// One outline body, three paragraphs of 40 pt text, with the character run stating a typeface
    /// index or not and the paragraph stating a line feed or not.
    /// </summary>
    private static SlideTextBody Body(bool statesFontIndex, bool statesLineFeed)
    {
        uint charMask = StatesFontHeight | (statesFontIndex ? StatesFontIndex : 0);
        PptParagraphRun[] paragraphs = statesLineFeed
            ? [new PptParagraphRun(
                Text.Length + 1, Depth: 0, HasBullet: null, BulletCharacter: null,
                Mask: StatesLineFeed, LineFeed: 100)]
            : [];

        PptTextRun run = new(
            // Body, so PptSlideLayout.Autofits would turn the fit on; the flag is passed
            // explicitly below because this fixture has no Escher shape.
            PptTextKind.Body,
            Text,
            paragraphs,
            [new PptCharacterRun(
                Text.Length, RunEmphasis.None, RunEmphasis.None,
                Mask: charMask, FontIndex: 1, FontHeight: 40)]);

        SlideTextBody body = PptTextBody.Build(
            run,
            styles: null,
            PptColourScheme.Default,
            PptFontTable.Empty,
            SlideTextBody.DefaultInsets,
            TextAnchor.Top,
            wraps: true).ShouldNotBeNull();

        return body with { Insets = default, AutoFit = true };
    }

    /// <summary>The distance between the first two baselines the body draws, in points.</summary>
    private static double Pitch(SlideTextBody body)
    {
        // 360 x 150 pt, the box the authored probe's tenth slide uses: wide enough for two lines a
        // paragraph and short enough that the fit has to shrink.
        DocRect area = new(
            Length.FromPoints(0), Length.FromPoints(0),
            Length.FromPoints(360), Length.FromPoints(150));

        List<PlacedGlyphRun> runs = SlideTextLayout.Place(body, area, new SlideFonts());
        List<double> baselines = [];
        foreach (PlacedGlyphRun run in runs)
        {
            double y = run.Run.Origin.Y.Points;
            if (!baselines.Exists(b => Math.Abs(b - y) < 0.001)) baselines.Add(y);
        }

        baselines.Sort();
        baselines.Count.ShouldBeGreaterThan(2);

        return baselines[1] - baselines[0];
    }

    /// <summary>The em the fit answered with, in points.</summary>
    private static double Em(SlideTextBody body)
    {
        DocRect area = new(
            Length.FromPoints(0), Length.FromPoints(0),
            Length.FromPoints(360), Length.FromPoints(150));

        List<PlacedGlyphRun> runs = SlideTextLayout.Place(body, area, new SlideFonts());
        runs.Count.ShouldBeGreaterThan(0);

        return runs[0].Run.FontSize.Points;
    }

    /// <summary>
    /// Every binary paragraph states its line spacing, whatever its record says.
    /// </summary>
    /// <remarks>
    /// The record's own two bits — <c>PPT_ParaAttr_LineFeed</c> on the paragraph and
    /// <c>PPT_CharAttr_Font</c> on its first portion — are what <c>svdfppt.cxx:6266-6271</c>
    /// tests, and implementing exactly that disjunction was measured over the whole slides track
    /// and beaten: 13 documents moved for −13.06 <c>abs_ink</c>, against −85.96 over 30
    /// improvements for "always". The theory is written over all four record shapes so that a
    /// future round narrowing the rule has to delete a case rather than quietly change one.
    /// </remarks>
    [Theory]
    [InlineData(false, false)]
    [InlineData(true, false)]
    [InlineData(false, true)]
    [InlineData(true, true)]
    public void EveryBinaryParagraphStatesItsLineSpacing(bool statesFontIndex, bool statesLineFeed)
    {
        SlideTextBody body = Body(statesFontIndex, statesLineFeed);

        foreach (SlideParagraph paragraph in body.Paragraphs)
        {
            paragraph.LineSpacingStated.ShouldBeTrue();
        }
    }

    /// <summary>
    /// A stated 100% and a stated nothing both resolve to single spacing, and the two draw
    /// different pitches once the fit has shrunk the body.
    /// </summary>
    /// <remarks>
    /// This is the property the whole change exists for, and it is stated as a ratio rather than
    /// as a length so it does not depend on which <c>constScaleLevels</c> row the fixture lands
    /// on. Both bodies resolve <see cref="LineSpacingRule"/> to proportional 1.0 — that is the
    /// point: the rule cannot tell them apart and <see cref="SlideParagraph.LineSpacingStated"/>
    /// can.
    /// </remarks>
    [Fact]
    public void AStatedHundredPerCentKeepsTheFullPitchWhereAStatedNothingIsReduced()
    {
        SlideTextBody stated = Body(statesFontIndex: true, statesLineFeed: false);

        // The same body as an OOXML or ODF reader would hand it over: the rule resolves to the
        // identical proportional 1.0 and only the flag differs. That is the whole point — the
        // rule cannot tell "states exactly 100%" from "states nothing" and the flag can.
        SlideTextBody unstated = stated with
        {
            Paragraphs = [.. stated.Paragraphs.Select(p => p with { LineSpacingStated = false })],
        };

        foreach (SlideParagraph paragraph in stated.Paragraphs.Concat(unstated.Paragraphs))
        {
            paragraph.LineSpacing.Mode.ShouldBe(LineSpacingMode.Proportional);
            paragraph.LineSpacing.Proportion.ShouldBe(1.0);
        }

        // 1.2 x em, the fixed-cell-height line, with no reduction.
        (Pitch(stated) / Em(stated)).ShouldBe(1.2, 0.01);

        // constScaleLevels pairs every font scale with 0.900 or 0.800, so the reduced pitch is
        // 1.08 or 0.96 ems and in either case strictly below 1.2.
        (Pitch(unstated) / Em(unstated)).ShouldBeLessThan(1.15);
    }

    /// <summary>
    /// The control: a paragraph that states a proportion other than 100 takes the <c>Prop</c> arm
    /// on both sides, and that arm multiplies by the fit's <c>fSpacingY</c> in the reference
    /// (<c>impedit3.cxx:1560-1576</c>) — so the flag must not change it.
    /// </summary>
    [Fact]
    public void AStatedNinetyPerCentIsUnaffectedByTheFlag()
    {
        PptTextRun run = new(
            PptTextKind.Body,
            Text,
            [new PptParagraphRun(
                Text.Length + 1, Depth: 0, HasBullet: null, BulletCharacter: null,
                Mask: StatesLineFeed, LineFeed: 90)],
            [new PptCharacterRun(
                Text.Length, RunEmphasis.None, RunEmphasis.None,
                Mask: StatesFontHeight, FontHeight: 40)]);

        SlideTextBody body = PptTextBody.Build(
            run, styles: null, PptColourScheme.Default, PptFontTable.Empty,
            SlideTextBody.DefaultInsets, TextAnchor.Top, wraps: true).ShouldNotBeNull();

        body.Paragraphs[0].LineSpacingStated.ShouldBeTrue();
        body.Paragraphs[0].LineSpacing.Proportion.ShouldBe(0.9, 0.001);

        // Not 1.2: the Prop arm applies the stated proportion whether or not the flag is set.
        SlideTextBody fitted = body with { Insets = default, AutoFit = true };
        (Pitch(fitted) / Em(fitted)).ShouldBeLessThan(1.15);
    }
}
