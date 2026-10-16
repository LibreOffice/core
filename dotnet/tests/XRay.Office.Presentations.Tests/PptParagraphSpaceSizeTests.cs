using XRay.Office.Core.Extraction;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.Presentations.Layout;
using XRay.Office.Presentations.MsBinary;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// A binary PowerPoint paragraph whose space above is a <em>percentage</em> resolves that
/// percentage against the size of its <strong>last</strong> portion, not its first.
/// </summary>
/// <remarks>
/// <para>
/// <c>PPTParagraphObj::ApplyTo</c> converts a positive <c>PPT_ParaAttr_UpperDist</c> into master
/// units before it becomes an <c>SvxULSpaceItem</c>, and the height it multiplies by comes off the
/// back of the portion list (<c>filter/source/msfilter/svdfppt.cxx</c>:6296-6306, this tree):
/// </para>
/// <code>
/// m_PortionList.back()->GetAttrib(PPT_CharAttr_FontHeight, nFontHeight, nDestinationInstance);
/// if (static_cast&lt;sal_Int16&gt;(nUpperDist) > 0)
///     nUpperDist = -static_cast&lt;sal_Int16&gt;((nFontHeight * nUpperDist * 100) / 1000);
/// </code>
/// <para>
/// Eighty master units make a point, so the whole of it is <c>size × percentage / 80</c> — and the
/// size is the paragraph's last run's, whatever the first run or the largest run is set in.
/// </para>
/// <para>
/// <strong>Measured at 26.2.4.2, not only read out of a later tree.</strong> Its own flat ODP of
/// <c>slides/done-012/ppt/gillikin_online_user_mtg_2010.ppt</c> gives page 2's five bullets
/// <c>fo:margin-top</c> of 0.3 cm, 0.3, <strong>0.318</strong>, 0.3 and <strong>0.212</strong>,
/// which are 20/80 of 34 pt, 34, <strong>36</strong>, 34 and <strong>24</strong> — the last
/// portion of each paragraph, where the third and the fifth both open on a 34 pt portion. Over the
/// whole 51-document <c>.ppt</c> column the last portion is admitted for 147 of 178 multi-size
/// paragraphs against base rates of 24.7 % for the first portion and 34.3 % for the largest
/// (<c>probes/slides-r108/ulpct2.py</c>).
/// </para>
/// </remarks>
public class PptParagraphSpaceSizeTests
{
    /// <summary>The mask bit for a stated font height — <c>PPT_CharAttr_FontHeight</c>.</summary>
    private const uint StatesFontHeight = 0x0002_0000;

    /// <summary>The mask bit for a stated space above — <c>PPT_ParaAttr_UpperDist</c>.</summary>
    private const uint StatesSpaceBefore = 0x0000_2000;

    /// <summary>The percentage PowerPoint's own body style states, and the corpus's commonest.</summary>
    private const short TwentyPerCent = 20;

    private const string First = "Maria Collins: DOCLINE and the ";
    private const string Second = "Emergency Access Initiative";

    /// <summary>
    /// One body of two paragraphs, the second of which runs <paramref name="opening"/> point text
    /// into <paramref name="closing"/> point text and states a percentage space above.
    /// </summary>
    private static SlideTextBody Body(ushort opening, ushort closing)
    {
        string text = string.Join(PptTextReader.ParagraphSeparator, "Questions", First + Second);

        // The paragraph the space is measured on is the second, so the first paragraph's own
        // space-before is the one PptTextBody zeroes and this fixture never looks at.
        PptParagraphRun[] paragraphs =
        [
            new PptParagraphRun("Questions".Length + 1, Depth: 0, HasBullet: null,
                                BulletCharacter: null),
            new PptParagraphRun(First.Length + Second.Length, Depth: 0, HasBullet: null,
                                BulletCharacter: null,
                                Mask: StatesSpaceBefore, SpaceBefore: TwentyPerCent),
        ];

        PptCharacterRun[] characters =
        [
            new PptCharacterRun("Questions".Length + 1 + First.Length, RunEmphasis.None,
                                RunEmphasis.None, Mask: StatesFontHeight, FontHeight: opening),
            new PptCharacterRun(Second.Length, RunEmphasis.None, RunEmphasis.None,
                                Mask: StatesFontHeight, FontHeight: closing),
        ];

        SlideTextBody body = PptTextBody.Build(
            new PptTextRun(PptTextKind.Body, text, paragraphs, characters),
            styles: null,
            PptColourScheme.Default,
            PptFontTable.Empty,
            SlideTextBody.DefaultInsets,
            TextAnchor.Top,
            wraps: true).ShouldNotBeNull();

        return body with { Insets = default, AutoFit = false };
    }

    /// <summary>
    /// A paragraph that ends smaller than it starts takes the smaller gap, and one that ends
    /// larger takes the larger — in both directions, so a rule reading the first run, the largest
    /// run or the smallest run fails one of the two.
    /// </summary>
    /// <remarks>
    /// The asymmetry is the assertion. 34 pt into 24 pt and 24 pt into 34 pt hold the same two
    /// sizes and the same 20 %, and only "the last portion" answers 6 pt for the first and 8.5 pt
    /// for the second.
    /// </remarks>
    [Theory]
    [InlineData(34, 24, 6.0)]
    [InlineData(24, 34, 8.5)]
    [InlineData(34, 36, 9.0)]
    [InlineData(34, 34, 8.5)]
    public void ThePercentageIsResolvedAgainstTheLastPortion(
        ushort opening, ushort closing, double expected)
    {
        SlideTextBody body = Body(opening, closing);

        body.Paragraphs.Count.ShouldBe(2);
        body.Paragraphs[1].Runs.Count.ShouldBe(2);
        body.Paragraphs[1].Runs[0].Size.Points.ShouldBe(opening, 0.001);
        body.Paragraphs[1].Runs[^1].Size.Points.ShouldBe(closing, 0.001);

        // size x 20/80, which is a quarter of the last portion's em.
        body.Paragraphs[1].SpaceBefore.Points.ShouldBe(expected, 0.001);
    }

    /// <summary>
    /// A negative value is a distance in master units and no portion's size reaches it.
    /// </summary>
    /// <remarks>
    /// The control for the arm above: <c>svdfppt.cxx</c>:6311-6314 takes
    /// <c>convertMasterUnitToMm100(-nVal2)</c> whenever the value is already at or below zero, and
    /// the percentage branch is never entered — so changing which portion the percentage reads
    /// must not move a paragraph that states an absolute gap.
    /// </remarks>
    [Fact]
    public void AnAbsoluteSpaceIsNotResolvedAgainstAnyPortion()
    {
        string text = string.Join(PptTextReader.ParagraphSeparator, "Questions", First + Second);

        PptParagraphRun[] paragraphs =
        [
            new PptParagraphRun("Questions".Length + 1, Depth: 0, HasBullet: null,
                                BulletCharacter: null),
            // -160 master units: 160/576 inch, which is two points.
            new PptParagraphRun(First.Length + Second.Length, Depth: 0, HasBullet: null,
                                BulletCharacter: null,
                                Mask: StatesSpaceBefore, SpaceBefore: -160),
        ];

        PptCharacterRun[] characters =
        [
            new PptCharacterRun("Questions".Length + 1 + First.Length, RunEmphasis.None,
                                RunEmphasis.None, Mask: StatesFontHeight, FontHeight: 34),
            new PptCharacterRun(Second.Length, RunEmphasis.None, RunEmphasis.None,
                                Mask: StatesFontHeight, FontHeight: 24),
        ];

        SlideTextBody body = PptTextBody.Build(
            new PptTextRun(PptTextKind.Body, text, paragraphs, characters),
            styles: null,
            PptColourScheme.Default,
            PptFontTable.Empty,
            SlideTextBody.DefaultInsets,
            TextAnchor.Top,
            wraps: true).ShouldNotBeNull();

        body.Paragraphs[1].SpaceBefore.Points.ShouldBe(20.0, 0.001);
    }

    /// <summary>
    /// And the gap the fit measures moves with it: the same body in the same box takes a different
    /// <c>constScaleLevels</c> row depending on which portion the percentage was read off.
    /// </summary>
    /// <remarks>
    /// This is the property the corpus moved on. <c>gillikin_online_user_mtg_2010.ppt</c> page 2
    /// misses row <c>{1.000, 0.900}</c> by 55 units of a hundredth of a millimetre when the
    /// percentage is read off the first portion and fits it by 9 when it is read off the last, and
    /// the whole slide is drawn 34 pt against 31 pt on that one comparison.
    /// </remarks>
    [Fact]
    public void TheGapTheFitMeasuresMovesWithIt()
    {
        // Two paragraphs whose second ends 10 pt smaller than it starts, in a box just too short
        // for the larger gap: 68 pt of text plus a 8.5 pt gap overflows and plus a 6 pt gap does
        // not, so the two readings answer different rows.
        SlideTextBody small = Body(34, 24) with { AutoFit = true };
        SlideTextBody large = Body(34, 34) with { AutoFit = true };

        small.Paragraphs[1].SpaceBefore.ShouldBeLessThan(large.Paragraphs[1].SpaceBefore);

        double Em(SlideTextBody body, double height)
        {
            DocRect area = new(
                Length.FromPoints(0), Length.FromPoints(0),
                Length.FromPoints(600), Length.FromPoints(height));
            List<PlacedGlyphRun> runs = SlideTextLayout.Place(body, area, new SlideFonts());
            runs.Count.ShouldBeGreaterThan(0);
            return runs[0].Run.FontSize.Points;
        }

        // A box that both fit unscaled, so the two agree wherever the gap is not what decides.
        Em(small, 400).ShouldBe(Em(large, 400), 0.001);

        // And one where the 2.5 pt of gap is the whole difference.
        double edge = (small.Paragraphs[1].SpaceBefore + large.Paragraphs[1].SpaceBefore).Points / 2
                      + (34 * 1.2 * 2);
        Em(small, edge).ShouldBeGreaterThan(Em(large, edge));
    }
}
