using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Layout;
using Shouldly;

namespace Paperless.Text.Tests;

/// <summary>
/// Tests the fixed distance a run puts between its characters — <c>EE_CHAR_KERNING</c>, which
/// DrawingML spells <c>a:rPr/@spc</c> and a word processor spells <c>w:spacing</c>.
/// </summary>
/// <remarks>
/// <para>
/// It is measured here rather than through a document because what matters is the arithmetic: a
/// tracked run is <em>n − 1</em> gaps wide, not <em>n</em>, and the difference between those two
/// readings is invisible on any one line and decides a break on a long one. Half the slides
/// corpus's PPTX files state the attribute and the commonest value is −0.2 pt, which over a
/// fifty-character line is ten points.
/// </para>
/// <para>
/// The reference is <c>SvxFont::QuickGetTextSize</c>
/// (<c>editeng/source/items/svxfont.cxx:481-500</c>), which walks the run's DX array adding one
/// kern per distinct advance and then takes the trailing one back off again.
/// </para>
/// </remarks>
public class CharacterTrackingTests
{
    private static OpenTypeFace Face()
    {
        string? path = FindFont("LiberationSans-Regular.ttf");
        Assert.SkipWhen(path is null, "Liberation Sans is not installed; see check-env.sh");
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

    private static readonly Length Size = Length.FromPoints(12);

    private static MeasuredParagraph Measured(string text, Length tracking)
        => MeasuredParagraph.Measure(
            text, [new FormattedRun(0, text.Length, Face(), Size, Tracking: tracking)]);

    /// <summary>
    /// A tracked paragraph is exactly one gap per character-boundary wider than an untracked one.
    /// </summary>
    [Theory]
    [InlineData("Requirement for research", 24)]
    [InlineData("ab", 2)]
    [InlineData("a", 1)]
    public void ATrackedRunIsOneGapPerBoundaryWider(string text, int length)
    {
        text.Length.ShouldBe(length);

        Length tracking = Length.FromPoints(0.25);
        Length plain = Measured(text, Length.Zero).WidthBetween(0, text.Length);
        Length tracked = Measured(text, tracking).WidthBetween(0, text.Length);

        (tracked - plain).ShouldBe(tracking * (text.Length - 1));
    }

    /// <summary>
    /// A negative value — which is what a deck's designer actually writes — pulls the run in.
    /// </summary>
    [Fact]
    public void ANegativeValuePullsTheRunIn()
    {
        const string text = "Critical analysis of evidence used to support points";

        Length plain = Measured(text, Length.Zero).WidthBetween(0, text.Length);
        Length tracked = Measured(text, Length.FromPoints(-0.2)).WidthBetween(0, text.Length);

        tracked.ShouldBeLessThan(plain);

        // 51 gaps at a fifth of a point: a whole word's worth of a line, which is why ignoring it
        // breaks a tracked line one word early.
        (plain - tracked).Points.ShouldBe(10.2, 0.001);
    }

    /// <summary>
    /// The gap is charged before each character, so the first of a run pays nothing.
    /// </summary>
    /// <remarks>
    /// Which is what makes the whole-paragraph width right: charge it after each character
    /// instead and every measured paragraph is one gap too wide, uniformly, which looks like a
    /// font-metric error rather than an off-by-one.
    /// </remarks>
    [Fact]
    public void TheFirstCharacterPaysNoGap()
    {
        MeasuredParagraph plain = Measured("abcd", Length.Zero);
        MeasuredParagraph tracked = Measured("abcd", Length.FromPoints(1));

        tracked.WidthBetween(0, 1).ShouldBe(plain.WidthBetween(0, 1));
        (tracked.WidthBetween(0, 2) - plain.WidthBetween(0, 2)).ShouldBe(Length.FromPoints(1));
        (tracked.WidthBetween(0, 4) - plain.WidthBetween(0, 4)).ShouldBe(Length.FromPoints(3));
    }

    /// <summary>
    /// Tracking that outruns a character's own advance does not walk the prefix table backwards.
    /// </summary>
    /// <remarks>
    /// Every width read out of the table is a difference of two of its entries, so one entry
    /// smaller than the one before it makes some range measure negative — and a negative width
    /// fits any line, which turns a wildly tracked run into a single line that runs off the page.
    /// </remarks>
    [Fact]
    public void ATrackingWiderThanTheCharacterKeepsTheTableMonotonic()
    {
        MeasuredParagraph measured = Measured("abcdef", Length.FromPoints(-100));

        for (int i = 1; i <= 6; i++)
        {
            measured.WidthBetween(0, i).ShouldBeGreaterThanOrEqualTo(measured.WidthBetween(0, i - 1));
        }
    }

    /// <summary>
    /// A run that states no tracking measures exactly as it did before tracking existed.
    /// </summary>
    [Fact]
    public void AnUntrackedRunIsUnchanged()
    {
        const string text = "The quick brown fox";

        Measured(text, Length.Zero).WidthBetween(0, text.Length)
            .ShouldBe(
                MeasuredParagraph
                    .Measure(text, [new FormattedRun(0, text.Length, Face(), Size)])
                    .WidthBetween(0, text.Length));
    }
}
