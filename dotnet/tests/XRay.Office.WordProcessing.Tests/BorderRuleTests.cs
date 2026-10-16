using XRay.Office.Core.Units;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A border's <c>w:sz</c> is the width of <em>a</em> line; the style says how many there are and how
/// far apart, so the room it takes and the strokes it draws are both derived rather than stated.
/// </summary>
/// <remarks>
/// The figures come from <c>probes/words-border-style/rules.py</c>, which writes one document per
/// style and size, reads the strokes out of a 300 dpi raster of 24.2.7.2's own PDF and the room out of
/// the following paragraph's y, and compares both against ours. The arithmetic under test is
/// <c>editeng</c>'s rather than a fit to those numbers — see <see cref="BorderRules"/> — so these
/// assertions are the port's contract with the measurement.
/// </remarks>
public sealed class BorderRuleTests
{
    /// <summary>Three points, which is <c>w:sz="24"</c>, the size the probe's table is quoted at.</summary>
    private static readonly Length ThreePoints = Length.FromPoints(3);

    /// <summary>A plain rule is drawn at the width it states, in one stroke.</summary>
    [Fact]
    public void ASingleRuleIsOneStrokeOfTheStatedWidth()
    {
        (BorderLine line, Length width) = BorderRules.FromWord(1, ThreePoints)!.Value;

        line.ShouldBe(BorderLine.Solid);
        width.ShouldBe(ThreePoints);

        BorderBands bands = BorderRules.Bands(line, width);
        bands.Outer.ShouldBe(ThreePoints);
        bands.HasTwoRules.ShouldBeFalse();
        bands.Total.ShouldBe(ThreePoints);
    }

    /// <summary>
    /// A double rule is three bands of the stated width: line, gap, line. It therefore takes three times
    /// the room, which is the half of this that decides pagination.
    /// </summary>
    /// <remarks>
    /// The reference draws a 3 pt <c>double</c> as strokes at 85.44 and 91.44 pt, each 3.12 pt at 300
    /// dpi, and puts the text 6 pt lower than the same document's <c>single</c> does.
    /// </remarks>
    [Fact]
    public void ADoubleRuleIsThreeBandsOfTheStatedWidth()
    {
        (BorderLine line, Length width) = BorderRules.FromWord(3, ThreePoints)!.Value;

        line.ShouldBe(BorderLine.Doubled);
        width.ShouldBe(Length.FromPoints(9));

        BorderBands bands = BorderRules.Bands(line, width);
        bands.Outer.ShouldBe(ThreePoints);
        bands.Gap.ShouldBe(ThreePoints);
        bands.Inner.ShouldBe(ThreePoints);
    }

    /// <summary>Writer has no triple rule and draws one as a double, gap and all.</summary>
    [Fact]
    public void ATripleRuleIsDrawnAsADouble()
        => BorderRules.FromWord(10, ThreePoints).ShouldBe(BorderRules.FromWord(3, ThreePoints));

    /// <summary><c>thick</c> is a single rule of twice the stated width, not a style of its own.</summary>
    [Fact]
    public void AThickRuleIsOneStrokeOfTwiceTheStatedWidth()
    {
        (BorderLine line, Length width) = BorderRules.FromWord(2, ThreePoints)!.Value;

        line.ShouldBe(BorderLine.Solid);
        width.ShouldBe(Length.FromPoints(6));
        BorderRules.Bands(line, width).Outer.ShouldBe(Length.FromPoints(6));
    }

    /// <summary>
    /// The small-gap pair put a fixed 0.75 pt rule and a fixed 0.75 pt gap beside the stated width, and
    /// they differ only in which side the stated one is on.
    /// </summary>
    [Fact]
    public void TheSmallGapStylesKeepTheStatedWidthAndAddAFixedThinRule()
    {
        Length thin = Length.FromPoints(0.75);

        (BorderLine thinThick, Length width) = BorderRules.FromWord(11, ThreePoints)!.Value;
        width.ShouldBe(ThreePoints + thin + thin);

        BorderBands outerScales = BorderRules.Bands(thinThick, width);
        outerScales.Outer.ShouldBe(ThreePoints);
        outerScales.Gap.ShouldBe(thin);
        outerScales.Inner.ShouldBe(thin);

        (BorderLine thickThin, Length same) = BorderRules.FromWord(12, ThreePoints)!.Value;
        same.ShouldBe(width);

        BorderBands innerScales = BorderRules.Bands(thickThin, same);
        innerScales.Outer.ShouldBe(thin);
        innerScales.Gap.ShouldBe(thin);
        innerScales.Inner.ShouldBe(ThreePoints);
    }

    /// <summary>
    /// <c>outset</c> is a fixed thin rule outside a scaling one, and <c>inset</c> the same reversed.
    /// </summary>
    /// <remarks>
    /// The reference draws a 3 pt <c>outset</c> as 0.72 pt and then 2.64 pt at 300 dpi, and puts the
    /// text 3.05 pt lower than <c>single</c> does. Both of those odd figures are the rounding: the
    /// scaling rule is <c>(2w + 0.75)/2 − 0.75</c>, which is 2.625 pt exactly and 53 twips once
    /// <c>BorderWidthImpl</c> has rounded the half — so the whole border is 121 twips rather than the
    /// 120 that 2w would suggest, and the extra twentieth of a point is visible in the probe.
    /// </remarks>
    [Fact]
    public void OutsetAndInsetPutTheFixedRuleOnOppositeSides()
    {
        Length thin = Length.FromPoints(0.75);

        (BorderLine outset, Length width) = BorderRules.FromWord(26, ThreePoints)!.Value;
        width.ShouldBe(Length.FromTwips(121));

        BorderBands out_ = BorderRules.Bands(outset, width);
        out_.Outer.ShouldBe(thin);
        out_.Inner.ShouldBe(Length.FromTwips(53));
        out_.Total.ShouldBe(Length.FromTwips(121));

        (BorderLine inset, Length inWidth) = BorderRules.FromWord(27, ThreePoints)!.Value;
        BorderBands in_ = BorderRules.Bands(inset, inWidth);
        in_.Outer.ShouldBe(Length.FromTwips(53));
        in_.Inner.ShouldBe(thin);
    }

    /// <summary>
    /// A dotted or dashed rule takes exactly the room a single one does — the style changes the ink
    /// along the rule, not across it.
    /// </summary>
    [Theory]
    [InlineData(6)]
    [InlineData(7)]
    [InlineData(8)]
    [InlineData(9)]
    public void ABrokenRuleTakesTheRoomOfASingleOne(int wordStyle)
    {
        (BorderLine line, Length width) = BorderRules.FromWord(wordStyle, ThreePoints)!.Value;

        width.ShouldBe(ThreePoints);
        BorderRules.Bands(line, width).Total.ShouldBe(ThreePoints);
        BorderRules.Dashes(line).ShouldNotBeNull();
    }

    /// <summary>
    /// The dash lengths do not scale with the rule: a quarter-point dotted border and a three-point one
    /// are both half-point dots a point apart.
    /// </summary>
    [Fact]
    public void TheDashLengthsAreTheSameAtEveryWidth()
    {
        IReadOnlyList<Length> dots = BorderRules.Dashes(BorderLine.Dotted)!;

        dots.Count.ShouldBe(2);
        dots[0].ShouldBe(Length.FromPoints(0.5));
        dots[1].ShouldBe(Length.FromPoints(1));

        BorderRules.Dashes(BorderLine.Dashed)!.ShouldBe([Length.FromPoints(8), Length.FromPoints(2.5)]);
        BorderRules.Dashes(BorderLine.Solid).ShouldBeNull();
    }

    /// <summary>
    /// <c>dashSmallGap</c> is the one style with a floor on its width: under a point it is drawn at a
    /// point.
    /// </summary>
    /// <remarks>
    /// Which the probe sees directly — <c>w:sz</c> 4 and 8 both draw a 0.96 pt rule at 300 dpi, where
    /// every other style draws 0.48 for the first and 0.96 for the second.
    /// </remarks>
    [Fact]
    public void AFineDashedRuleIsNeverThinnerThanAPoint()
    {
        BorderRules.FromWord(22, Length.FromPoints(0.5))!.Value.Width.ShouldBe(Length.FromPoints(1));
        BorderRules.FromWord(22, Length.FromPoints(3))!.Value.Width.ShouldBe(Length.FromPoints(3));
    }

    /// <summary>
    /// <c>none</c>, <c>nil</c> and every art border draw nothing, and an unknown name is read as one of
    /// them rather than guessed at as a plain rule.
    /// </summary>
    [Theory]
    [InlineData("none")]
    [InlineData("nil")]
    [InlineData("apples")]
    [InlineData("zanyTriangles")]
    [InlineData("somethingWordHasNotInventedYet")]
    public void AStyleWithNoLineDrawsNothing(string value)
        => BorderRules.FromWord(BorderRules.WordStyleOf(value), ThreePoints).ShouldBeNull();

    /// <summary>A border with no stated width is three quarters of a point, not nothing.</summary>
    /// <remarks><c>fdo#68779</c>, which is what makes an RTF <c>\brdrs</c> without a <c>\brdrw</c> draw.</remarks>
    [Fact]
    public void AStatedWidthOfNothingIsThreeQuartersOfAPoint()
        => BorderRules.FromWord(1, Length.Zero)!.Value.Width.ShouldBe(Length.FromPoints(0.75));
}
