using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Layout;
using Shouldly;

namespace XRay.Office.Text.Tests;

/// <summary>
/// Where the baseline sits when proportional line spacing has shrunk the box below the text.
/// </summary>
/// <remarks>
/// <para>
/// Four fifths of the shrunk box, and <em>not</em> the face's own ascent. Writer shrinks the line and
/// then overrides the ascent outright rather than scaling it with the box —
/// <c>SwTextFormatter::CalcRealHeight</c>'s <c>SvxLineSpaceRule::Auto</c> arm,
/// <c>sw/source/core/text/itrform2.cxx</c>:
/// </para>
/// <code>
/// if (nTmp &lt; 100) { nTmp *= nLineHeight; nTmp /= 100; nLineHeight = nTmp;
///                     SwTwips nAsc = (4 * nLineHeight) / 5; m_pCurr-&gt;SetAscent(nAsc); }
/// </code>
/// <para>
/// It is the same fraction <see cref="LineSpacingMode.Exact"/> has always used here, in a different
/// arm of the same C++ function, which is why it was missed: reading the exact-spacing rule off the
/// source does not tell you the auto arm shares it.
/// </para>
/// <para>
/// <strong>Measured, not inferred.</strong> Twenty pages, one <c>w:line</c> value each from 50% to
/// 97.5%, eleven lines per page, regressing the first line's ink position on the line height:
/// the reference's slope is <strong>0.8030</strong> and ours was <strong>0.0000</strong> — our first
/// baseline sat in exactly the same place at every spacing, because the natural ascent fell through
/// untouched. Predicting each shrink as <c>naturalAscent − (4 × height) / 5</c> in truncating integer
/// arithmetic is exact on all twenty, residual nought:
/// </para>
/// <code>
///   height tw   4h/5   predicted   measured        height tw   4h/5   predicted   measured
///         126    100       5.250      5.250              189    151       2.700      2.700
///         159    127       3.900      3.900              220    176       1.450      1.450
///         247    197       0.400      0.400              253      —       0.000      0.000
/// </code>
/// <para>
/// After the fix our slope is 0.7985, and the residual that remains is inherited from the line
/// <em>height</em> being one to three twips out — a separate open question recorded in
/// <c>probes/proportional-spacing-subunity/</c>. Where our height is exact, so is the baseline.
/// </para>
/// <para>
/// This moves the text inside the box and not the box, so it changes no page break. That is exactly
/// why it survived so long: no column of the corpus gate can see it.
/// </para>
/// </remarks>
public class SubUnitySpacingAscentTests
{
    private static readonly Length Eleven = Length.FromPoints(11);

    /// <summary>The rule: a shrunk box puts its baseline at four fifths, not at the font's ascent.</summary>
    [Fact]
    public void AShrunkProportionalLineTakesFourFifthsOfTheBoxAsItsAscent()
    {
        LaidOutParagraph laid = Lay(LineSpacingRule.Multiple(0.5));

        laid.Lines.Count.ShouldBe(1);
        laid.Lines[0].Baseline.ShouldBe(Length.FromTwips(4 * laid.Lines[0].Height.Twips / 5));
    }

    /// <summary>
    /// And it is a slope rather than a single point: a second, milder shrink moves the baseline with
    /// the box.
    /// </summary>
    /// <remarks>
    /// One ratio would also be satisfied by an implementation that happened to return some constant
    /// near four fifths of that particular box. Two fix the line, which is the whole content of the
    /// 0.8030-against-0.0000 measurement.
    /// </remarks>
    [Fact]
    public void TheBaselineTracksTheBoxRatherThanSittingAtOneHeight()
    {
        LaidOutParagraph half = Lay(LineSpacingRule.Multiple(0.5));
        LaidOutParagraph most = Lay(LineSpacingRule.Multiple(0.9));

        most.Lines[0].Height.ShouldBeGreaterThan(half.Lines[0].Height);
        most.Lines[0].Baseline.ShouldBeGreaterThan(half.Lines[0].Baseline);

        foreach (LaidOutParagraph laid in new[] { half, most })
        {
            laid.Lines[0].Baseline.ShouldBe(Length.FromTwips(4 * laid.Lines[0].Height.Twips / 5));
        }
    }

    /// <summary>
    /// Full spacing is left alone, which is the boundary the C++ tests with <c>nTmp &lt; 100</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The reference's 100% page is the one page in the probe that does not lie on the 0.8 line, and
    /// treating it as though it did is what made the first reading of that probe report a slope of
    /// 0.83. A line that is not shrunk keeps the face's own ascent.
    /// </para>
    /// <para>
    /// The control is a paragraph with no spacing rule at all rather than a fraction of the box,
    /// because for this face the two are close enough to be confused: Liberation Serif's ascent is
    /// 77.5% of its line height, so "four fifths" and "the font's own" differ by about a third of a
    /// point at 11 pt — and in the direction that makes the shrunk baseline sit <em>lower</em> in its
    /// box, not higher. An assertion written as an inequality against 4/5 gets the sign wrong.
    /// </para>
    /// </remarks>
    [Fact]
    public void FullSpacingKeepsTheFacesOwnAscent()
    {
        LaidOutParagraph laid = Lay(LineSpacingRule.Multiple(1.0));
        LaidOutParagraph unspaced = Lay(ParagraphFormat.Default.LineSpacing);

        laid.Lines[0].Baseline.ShouldBe(unspaced.Lines[0].Baseline);
        laid.Lines[0].Baseline.ShouldNotBe(Length.FromTwips(4 * laid.Lines[0].Height.Twips / 5));
    }

    /// <summary>
    /// Spacing <em>above</em> one keeps putting its slack over the text, which this must not disturb.
    /// </summary>
    /// <remarks>
    /// The change adds an arm below the existing <c>extra &gt; 0</c> test rather than replacing it, and
    /// this is the guard on that: at 150% the baseline is the natural ascent plus the whole of the
    /// added slack, so every twip still lands between the previous line and this one.
    /// </remarks>
    [Fact]
    public void SpacingAboveOneStillPutsAllItsSlackAboveTheText()
    {
        LaidOutParagraph single = Lay(LineSpacingRule.Multiple(1.0));
        LaidOutParagraph loose = Lay(LineSpacingRule.Multiple(1.5));

        Length slack = loose.Lines[0].Height - single.Lines[0].Height;
        slack.ShouldBeGreaterThan(Length.Zero);

        loose.Lines[0].Baseline.ShouldBe(single.Lines[0].Baseline + slack);
    }

    private static LaidOutParagraph Lay(LineSpacingRule spacing)
    {
        OpenTypeFace face = LiberationSerif();
        const string Text = "x";

        return new ParagraphLayouter(face).Layout(
            MeasuredParagraph.Measure(Text, [new FormattedRun(0, 1, face, Eleven)]),
            ParagraphFormat.Default with { LineSpacing = spacing },
            Length.FromMillimetres(170));
    }

    private static OpenTypeFace LiberationSerif()
    {
        string? path = FindFont("LiberationSerif-Regular.ttf");
        Assert.SkipWhen(path is null, "Liberation Serif is not installed; see check-env.sh");
        return OpenTypeFace.ReadFile(path!).ShouldNotBeNull();
    }

    private static string? FindFont(string fileName)
    {
        foreach (string directory in new[]
                 {
                     "/usr/share/fonts/truetype/liberation",
                     "/usr/share/fonts/truetype/crosextra",
                     "/usr/share/fonts",
                 })
        {
            if (!Directory.Exists(directory)) continue;

            string[] found = Directory.GetFiles(directory, fileName, SearchOption.AllDirectories);
            if (found.Length > 0) return found[0];
        }

        return null;
    }
}
