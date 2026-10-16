using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Layout;
using Shouldly;

namespace XRay.Office.Text.Tests;

/// <summary>
/// A blank or tab run is transparent to the line's <em>height</em> and opaque to the height
/// proportional line spacing takes its percentage of.
/// </summary>
/// <remarks>
/// <para>
/// The companion to <see cref="BlankLineHeightTests"/>, and it is the same rule read from the other
/// side. Writer keeps two maxima per line and they have different membership: `#i3952#` /
/// <c>IGNORE_TABS_AND_BLANKS_FOR_LINE_CALCULATION</c> decides <c>SwLineLayout::Height</c>, while
/// <c>m_nLineSpacingBaseHeight</c> is a second maximum with its own test, and
/// <c>SwTextFormatter::CalcRealHeight</c> adds <c>(prop − 100)%</c> of <em>that</em> to the line
/// height rather than scaling the line.
/// </para>
/// <para>
/// Measured against the installed 26.2.4.2 by <c>probes/words-w-pitch/mk.py</c>, six paragraphs per
/// group of a 10 pt Arial style at <c>w:line="288" w:lineRule="auto"</c> with
/// <c>w:contextualSpacing</c>, so the pitch is the line height plus the proportional gap and nothing
/// else. With one blank run in a face and size of its own:
/// </para>
/// <code>
///   blank run              line height   reference pitch   base it implies
///   none                         11.50             13.80             11.50
///   Calibri 11 pt tab            11.50             14.15             13.43
///   Calibri 11 pt space          11.50             14.15             13.43
///   Calibri 22 pt tab            11.50             16.85             26.86
///   Arial 20 pt tab              11.50             16.10             23.00
/// </code>
/// <para>
/// It is worth a page: <c>OM template for non-complex NCC operators</c> sets each contents entry
/// with a theme-font 11 pt run holding the tab between the number and the title, and drawing those
/// at 13.80 rather than 14.15 fitted 83 entries where the reference fits 79.
/// </para>
/// </remarks>
public class BlankLineSpacingBaseTests
{
    private static readonly Length Small = Length.FromPoints(8);
    private static readonly Length Large = Length.FromPoints(24);

    private const string Tabbed = "small\tsmall";

    [Fact]
    public void ATallTabRaisesTheSpacingBaseWithoutRaisingTheLine()
    {
        OpenTypeFace face = Carlito();

        (Length height, _, Length baseHeight) = MeasuredParagraph
            .Measure(Tabbed, LargeTab(face), blanksAreTransparentToHeight: true)
            .MeasureLine(0, Tabbed.Length);

        (Length small, _, _) = MeasuredParagraph
            .Measure("small", [new FormattedRun(0, 5, face, Small)])
            .MeasureLine(0, 5);

        (Length large, _, _) = MeasuredParagraph
            .Measure("x", [new FormattedRun(0, 1, face, Large)])
            .MeasureLine(0, 1);

        height.ShouldBe(small);
        baseHeight.ShouldBe(large);
    }

    /// <summary>A run of spaces does it too, which a tab alone would not prove.</summary>
    /// <remarks>
    /// The two are separate branches in <c>CalcLine</c> — a tab by portion type, a blank run by
    /// <c>lcl_HasOnlyBlanks</c> over its characters — and the probe measured both at 14.15.
    /// </remarks>
    [Fact]
    public void ARunOfTallSpacesRaisesTheSpacingBaseToo()
    {
        OpenTypeFace face = Carlito();
        const string Text = "small   small";

        List<FormattedRun> runs =
        [
            new FormattedRun(0, 5, face, Small),
            new FormattedRun(5, 3, face, Large),
            new FormattedRun(8, 5, face, Small),
        ];

        (Length height, _, Length baseHeight) = MeasuredParagraph
            .Measure(Text, runs, blanksAreTransparentToHeight: true)
            .MeasureLine(0, Text.Length);

        (Length small, _, _) = MeasuredParagraph
            .Measure("small", [new FormattedRun(0, 5, face, Small)])
            .MeasureLine(0, 5);

        (Length large, _, _) = MeasuredParagraph
            .Measure("x", [new FormattedRun(0, 1, face, Large)])
            .MeasureLine(0, 1);

        height.ShouldBe(small);
        baseHeight.ShouldBe(large);
    }

    /// <summary>
    /// The pitch a 120 % paragraph then gets: the line height plus a fifth of the tab's height.
    /// </summary>
    /// <remarks>
    /// The arithmetic the reference's 14.15 pt comes from, in the one place a caller can see it.
    /// Scaling the line instead gives <c>small × 1.2</c> and is what this closes.
    /// </remarks>
    [Fact]
    public void ProportionalSpacingTakesItsPercentageOfTheTabRatherThanOfTheLine()
    {
        OpenTypeFace face = Carlito();

        (Length height, _, Length baseHeight) = MeasuredParagraph
            .Measure(Tabbed, LargeTab(face), blanksAreTransparentToHeight: true)
            .MeasureLine(0, Tabbed.Length);

        LineSpacingRule spacing = LineSpacingRule.Multiple(1.2);

        spacing.Apply(height, baseHeight)
            .ShouldBe(Length.FromTwips(height.Twips + (20 * baseHeight.Twips / 100)));

        spacing.Apply(height, baseHeight).ShouldBeGreaterThan(spacing.Apply(height, height));
    }

    /// <summary>A blank no taller than the text leaves the base exactly where the text put it.</summary>
    /// <remarks>
    /// The rule raises the base towards the blanks; it must never lower it. Without this the tests
    /// above would pass against an implementation that measured the blank runs and nothing else.
    /// </remarks>
    [Fact]
    public void ASmallTabBetweenLargeTextLeavesTheBaseAtTheText()
    {
        OpenTypeFace face = Carlito();
        const string Text = "big\tbig";

        List<FormattedRun> runs =
        [
            new FormattedRun(0, 3, face, Large),
            new FormattedRun(3, 1, face, Small),
            new FormattedRun(4, 3, face, Large),
        ];

        (Length height, _, Length baseHeight) = MeasuredParagraph
            .Measure(Text, runs, blanksAreTransparentToHeight: true)
            .MeasureLine(0, Text.Length);

        (Length large, _, _) = MeasuredParagraph
            .Measure("big", [new FormattedRun(0, 3, face, Large)])
            .MeasureLine(0, 3);

        height.ShouldBe(large);
        baseHeight.ShouldBe(large);
    }

    /// <summary>
    /// An RTF or ODF document, which does not make blanks transparent, is untouched by any of this.
    /// </summary>
    /// <remarks>
    /// There the tab is measured into the line's own height, so the base and the height agree and the
    /// second fold never runs — the same answer the engine gave before this rule existed.
    /// </remarks>
    [Fact]
    public void WithoutTheWordRuleTheBaseAndTheLineAgree()
    {
        OpenTypeFace face = Carlito();

        (Length height, _, Length baseHeight) = MeasuredParagraph
            .Measure(Tabbed, LargeTab(face))
            .MeasureLine(0, Tabbed.Length);

        baseHeight.ShouldBe(height);
    }

    private static List<FormattedRun> LargeTab(OpenTypeFace face) =>
    [
        new FormattedRun(0, 5, face, Small),
        new FormattedRun(5, 1, face, Large),
        new FormattedRun(6, 5, face, Small),
    ];

    private static OpenTypeFace Carlito()
    {
        string? path = FindFont("Carlito-Regular.ttf");
        Assert.SkipWhen(path is null, "Carlito is not installed; see check-env.sh");
        return OpenTypeFace.ReadFile(path!).ShouldNotBeNull();
    }

    private static string? FindFont(string fileName)
    {
        foreach (string directory in new[]
                 {
                     "/usr/share/fonts/truetype/crosextra",
                     "/usr/share/fonts/truetype/dejavu",
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
