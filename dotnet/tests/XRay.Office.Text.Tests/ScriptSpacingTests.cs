using XRay.Office.Core.Units;
using XRay.Office.Text.Layout;
using Shouldly;

namespace XRay.Office.Text.Tests;

/// <summary>
/// Writer's extra space between East Asian and Western text.
/// </summary>
/// <remarks>
/// <para>
/// A fifth of the font size at a script change — <c>SwTextFormatter::BuildPortions</c>,
/// <c>sw/source/core/text/itrform2.cxx</c>:707-734, whose value is
/// <c>rInf.GetFont()-&gt;GetHeight()/5</c>. Measured on <c>手机免提系统TSB.doc</c>: LibreOffice sets
/// <c>长安福特技术服务公告</c> ending at 331.800 pt and <c>CAF</c> beginning at 334.200, a gap of
/// exactly 2.400 pt at 12 pt, and repeats it at every one of that document's 38 script changes.
/// </para>
/// <para>
/// The two exclusions in <c>fnRequireKerningAtPosition</c> (<c>:486-521</c>) are what keep this from
/// being a general script-change rule, and they are tested here as carefully as the rule itself: one
/// side must be East Asian (tdf#89288), and neither side may be Hangul (tdf#136663).
/// </para>
/// </remarks>
public class ScriptSpacingTests
{
    [Fact]
    public void TheGapIsAFifthOfTheSize()
    {
        // 12 pt is 240 twips; a fifth is 48 twips, which is the 2.400 pt LibreOffice drew.
        ScriptSpacing.GapFor(Length.FromPoints(12)).Twips.ShouldBe(48);
        ScriptSpacing.GapFor(Length.FromPoints(12)).Points.ShouldBe(2.4, 0.0001);
    }

    [Fact]
    public void TheGapTruncatesRatherThanRounding()
    {
        // Integer division in the C++. 10.5 pt is 210 twips and a fifth is 42 exactly; 11 pt is 220
        // and a fifth is 44 exactly; 5.5 pt is 110, whose fifth is 22. The case that separates the two
        // rules is a size whose twip count is not a multiple of five — 7.9 pt, 158 twips, 31.6.
        ScriptSpacing.GapFor(Length.FromTwips(158)).Twips.ShouldBe(31);
        ScriptSpacing.GapFor(Length.FromTwips(159)).Twips.ShouldBe(31);
        ScriptSpacing.GapFor(Length.FromTwips(160)).Twips.ShouldBe(32);
    }

    [Fact]
    public void ANegativeOrZeroSizeOpensNoGap()
    {
        ScriptSpacing.GapFor(Length.Zero).ShouldBe(Length.Zero);
        ScriptSpacing.GapFor(Length.FromTwips(-240)).ShouldBe(Length.Zero);
    }

    [Theory]
    // Han beside Latin, in both directions — the case the corpus document is made of.
    [InlineData("公告CAF", 2, true)]
    [InlineData("CAF公告", 3, true)]
    // Han beside a digit, which is the same change: "（1根）" opens one between the 1 and the 根.
    [InlineData("1根", 1, true)]
    [InlineData("秒5", 1, true)]
    // Kana and bopomofo are East Asian too.
    [InlineData("かなAbc", 2, true)]
    [InlineData("Abcかな", 3, true)]
    public void AChangeWithEastAsianTextOnOneSideOpensAGap(string text, int index, bool opens)
        => ScriptSpacing.Opens(text, index).ShouldBe(opens);

    [Theory]
    // tdf#89288: only between CJK and non-CJK. Latin beside Hebrew or Arabic is a script change and
    // gets nothing, which is the whole reason this rule is safe to switch on for every Word document.
    [InlineData("abcאבג", 3)]
    [InlineData("אבגabc", 3)]
    [InlineData("abcمرحبا", 3)]
    [InlineData("ΑΒΓabc", 3)]
    public void AChangeWithNoEastAsianTextOpensNothing(string text, int index)
        => ScriptSpacing.Opens(text, index).ShouldBeFalse();

    [Theory]
    // tdf#136663: the space is a Chinese and Japanese convention, not a Korean one.
    [InlineData("한글abc", 3)]
    [InlineData("abc한글", 3)]
    public void HangulBesideWesternTextOpensNothing(string text, int index)
        => ScriptSpacing.Opens(text, index).ShouldBeFalse();

    [Theory]
    // "we do not want a kerning portion if any end would be a punctuation character" — both ends must
    // be a letter or a digit. This is what stops a gap opening beside 。， （ ） or a space.
    [InlineData("公告。CAF", 3)]
    [InlineData("公告，CAF", 3)]
    [InlineData("公告 CAF", 3)]
    [InlineData("公告（CAF", 3)]
    [InlineData("CAF、公告", 4)]
    public void PunctuationOnEitherSideOpensNothing(string text, int index)
        => ScriptSpacing.Opens(text, index).ShouldBeFalse();

    [Theory]
    [InlineData("公告公告", 2)]
    [InlineData("abcdef", 3)]
    public void NoChangeOfScriptOpensNothing(string text, int index)
        => ScriptSpacing.Opens(text, index).ShouldBeFalse();

    [Fact]
    public void NothingOpensAtTheStartOrPastTheEnd()
    {
        // `fnRequireKerningAtPosition` returns false at index 0 outright, and the C++ also refuses a
        // boundary at the very end of the text — there is no portion after it to space away from.
        ScriptSpacing.Opens("公告CAF", 0).ShouldBeFalse();
        ScriptSpacing.Opens("公告CAF", 5).ShouldBeFalse();
        ScriptSpacing.Opens("公告CAF", 99).ShouldBeFalse();
        ScriptSpacing.Opens("", 0).ShouldBeFalse();
    }

    [Fact]
    public void EveryBoundaryInASentenceIsFound()
    {
        // "本次TSB所涉及" — two changes, one on each side of the acronym. LibreOffice splits this into
        // three drawn words on the corpus document where we drew one before this existed.
        ScriptSpacing.Boundaries("本次TSB所涉及").ShouldBe([2, 5]);

        // And a sentence with none.
        ScriptSpacing.Boundaries("由于部分车辆手机免提系统模块的故障。").ShouldBeEmpty();
    }
}
