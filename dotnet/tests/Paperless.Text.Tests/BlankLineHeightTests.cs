using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Layout;
using Shouldly;

namespace Paperless.Text.Tests;

/// <summary>
/// Whether a tab or a run of spaces is allowed to make a line taller.
/// </summary>
/// <remarks>
/// <para>
/// Word says no, and Writer follows behind <c>IgnoreTabsAndBlanksForLineCalculation</c> (#i3952):
/// <c>SwLineLayout::CalcLine</c> skips a tab portion, a hole portion or an all-blank text portion
/// outright while any other portion is on the line
/// (<c>sw/source/core/text/porlay.cxx</c>:340), and falls back to it when none is
/// (<c>porlay.cxx</c>:601). The DOC and DOCX importers turn the setting on; RTF and ODF leave it off.
/// </para>
/// <para>
/// It bites where a document tabs between columns of small text: the tab takes whatever character
/// formatting covers it, which is usually the document's twelve-point default rather than the size of
/// the text either side. Measured on <c>prison-population-bulletin-june.doc</c>, a tabbed table set in
/// eight point, where counting the tabs gave a row pitch of 16.7 pt against the reference's 12.1 at
/// identical glyph sizes — two pages' worth over the document.
/// </para>
/// </remarks>
public class BlankLineHeightTests
{
    private static readonly Length Small = Length.FromPoints(8);
    private static readonly Length Large = Length.FromPoints(24);

    /// <summary>Text whose tab is the only large thing on the line, as a tabbed table's row is.</summary>
    private const string Tabbed = "small\tsmall";

    [Fact]
    public void ATabInALargerFaceDoesNotRaiseTheLineWhenBlanksAreTransparent()
    {
        OpenTypeFace face = Carlito();

        (Length height, _) = MeasuredParagraph
            .Measure(Tabbed, LargeTab(face), blanksAreTransparentToHeight: true)
            .HeightOf(0, Tabbed.Length);

        (Length small, _) = MeasuredParagraph
            .Measure("small", [new FormattedRun(0, 5, face, Small)])
            .HeightOf(0, 5);

        height.ShouldBe(small);
    }

    [Fact]
    public void ATabInALargerFaceRaisesTheLineWhenTheyAreNot()
    {
        OpenTypeFace face = Carlito();

        (Length height, _) = MeasuredParagraph
            .Measure(Tabbed, LargeTab(face))
            .HeightOf(0, Tabbed.Length);

        (Length small, _) = MeasuredParagraph
            .Measure("small", [new FormattedRun(0, 5, face, Small)])
            .HeightOf(0, 5);

        height.ShouldBeGreaterThan(small);
    }

    /// <summary>
    /// A run of spaces is skipped for the same reason a tab is, which a tab alone would not prove.
    /// </summary>
    /// <remarks>
    /// The two are separate branches in <c>CalcLine</c>: a tab is recognised by portion type and a blank
    /// run by <c>lcl_HasOnlyBlanks</c> over the portion's characters.
    /// </remarks>
    [Fact]
    public void ARunOfSpacesInALargerFaceDoesNotRaiseTheLineEither()
    {
        OpenTypeFace face = Carlito();
        const string Text = "small   small";

        List<FormattedRun> runs =
        [
            new FormattedRun(0, 5, face, Small),
            new FormattedRun(5, 3, face, Large),
            new FormattedRun(8, 5, face, Small),
        ];

        (Length height, _) = MeasuredParagraph
            .Measure(Text, runs, blanksAreTransparentToHeight: true)
            .HeightOf(0, Text.Length);

        (Length small, _) = MeasuredParagraph
            .Measure("small", [new FormattedRun(0, 5, face, Small)])
            .HeightOf(0, 5);

        height.ShouldBe(small);
    }

    /// <summary>
    /// A line of nothing but tabs and blanks takes their height, since there is nothing else to take.
    /// </summary>
    /// <remarks>
    /// <c>porlay.cxx</c>:601 — "Word increases line height if _only_ spaces and|or tabstops are in a
    /// line". Skipping them unconditionally would give such a line no height at all, which would collapse
    /// every blank tabbed line in a form.
    /// </remarks>
    [Fact]
    public void ALineOfOnlyBlanksIsAsTallAsTheBlanks()
    {
        OpenTypeFace face = Carlito();
        const string Text = "\t";

        (Length height, _) = MeasuredParagraph
            .Measure(Text, [new FormattedRun(0, 1, face, Large)], blanksAreTransparentToHeight: true)
            .HeightOf(0, Text.Length);

        (Length large, _) = MeasuredParagraph
            .Measure("x", [new FormattedRun(0, 1, face, Large)])
            .HeightOf(0, 1);

        height.ShouldBe(large);
    }

    /// <summary>A small tab between large text still leaves the line as tall as the text.</summary>
    /// <remarks>
    /// The rule takes height away from blanks; it must not take it away from anything else. Without this
    /// the first test would pass just as well against an implementation that measured only the first run.
    /// </remarks>
    [Fact]
    public void TheTextOnTheLineStillDecidesItsHeight()
    {
        OpenTypeFace face = Carlito();
        const string Text = "big\tbig";

        List<FormattedRun> runs =
        [
            new FormattedRun(0, 3, face, Large),
            new FormattedRun(3, 1, face, Small),
            new FormattedRun(4, 3, face, Large),
        ];

        (Length height, _) = MeasuredParagraph
            .Measure(Text, runs, blanksAreTransparentToHeight: true)
            .HeightOf(0, Text.Length);

        (Length large, _) = MeasuredParagraph
            .Measure("big", [new FormattedRun(0, 3, face, Large)])
            .HeightOf(0, 3);

        height.ShouldBe(large);
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
