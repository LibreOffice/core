using Paperless.Core.Documents;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// The seven characters that decide which of Calc's two text painters draws a cell.
/// </summary>
/// <remarks>
/// <para>
/// <c>ScDrawStringsVars::HasEditCharacters</c> (<c>sc/source/ui/view/output2.cxx:823-847</c>) is
/// consulted at <c>output2.cxx:1812</c>, before anything about the cell's output area has been
/// worked out. A no-break space, a soft hyphen, a zero-width space, either bidi mark, a
/// non-breaking hyphen or a word joiner sends the cell to <c>DrawEditStandard</c>, and the
/// difference that shows on paper is what happens when the string does not fit its column:
/// <c>DrawStrings</c> drops the characters it cannot show, while the EditEngine path sets a clip
/// of the cell and draws the whole string behind it.
/// </para>
/// <para>
/// Measured on <c>esurf-12-135-2024-t01.xlsx</c>, whose date column is written with no-break
/// spaces — <c>28 Oct 2012</c> — in a column one character too narrow for it: the
/// reference PDF's text layer holds all eleven characters and ours held ten, on eighteen of the
/// twenty-three rows. 113 extractable words against 124, and 123 now.
/// </para>
/// <para>
/// The fixture's first row is the control and is the assertion that matters most: a change that
/// simply stopped shortening would pass every other check here.
/// </para>
/// </remarks>
public sealed class SheetEditCharacterTests
{
    [Theory]
    [InlineData("plain text", "no edit characters", false)]
    [InlineData("no-break space", "28 Oct 2012", true)]
    [InlineData("soft hyphen", "co­operate", true)]
    [InlineData("zero-width space", "one​two", true)]
    [InlineData("left-to-right mark", "‎mark", true)]
    [InlineData("right-to-left mark", "‏mark", true)]
    [InlineData("non-breaking hyphen", "F‑117", true)]
    [InlineData("word joiner", "one⁠two", true)]
    // A thin space and an en dash are not on the list, and the corpus document that found this
    // holds both in its neighbouring columns — which the reference shortens.
    [InlineData("thin space", "5 km", false)]
    public void OnlyTheSevenNamedCodePointsCount(string _, string text, bool expected)
        => SheetTextLayout.HasEditCharacters(text).ShouldBe(expected);

    /// <summary>
    /// A no-break space is a thousands separator in half of Europe, so a repeat directive
    /// cancels it.
    /// </summary>
    /// <remarks>
    /// tdf#122676, stated in the comment beside the <c>CHAR_NBSP</c> case itself: "Ignore
    /// CHAR_NBSP (this is thousand separator in any number) if repeat character is set". None of
    /// the other six is excused.
    /// </remarks>
    [Fact]
    public void ARepeatDirectiveExcusesTheNoBreakSpaceAndNothingElse()
    {
        SheetTextLayout.HasEditCharacters("1 234", fillAt: 3).ShouldBeFalse();
        SheetTextLayout.HasEditCharacters("1 234", fillAt: -1).ShouldBeTrue();
        SheetTextLayout.HasEditCharacters("1​234", fillAt: 3).ShouldBeTrue();
    }

    /// <summary>
    /// The string reaches the page whole, and the control beside it does not.
    /// </summary>
    /// <remarks>
    /// Asserted against the drawn glyph runs rather than against the placement, because the
    /// defect was that the characters never reached the text layer at all — a PDF viewer showed
    /// the same clipped column either way and only a selection or an extraction could see it.
    /// Checked against LibreOffice 24.2.7.2's own PDF of the same fixture, whose text layer holds
    /// <c>Plain wide h</c>, <c>Nobreak space here</c>, <c>Softhyphen-wide-text</c> and
    /// <c>Wordjoinerwidetext</c>.
    /// </remarks>
    [Fact]
    public void ACellHoldingOneIsClippedRatherThanShortened()
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require("sheet-edit-characters.fods"));

        RecordingDrawingSink sink = new();
        ((SpreadsheetPages)document.Layout()).Pages[0].Draw(sink);

        List<string> drawn = [.. sink.Pages[0].Runs.Select(r => r.Text)];

        // The control: plain text in the same column, shortened to what the column can show.
        drawn.ShouldContain(t => t.StartsWith("Plain wide", StringComparison.Ordinal));
        drawn.ShouldNotContain("Plain wide heading");

        // The three that hold an edit character keep every one of theirs.
        drawn.ShouldContain("Nobreak space here");
        drawn.ShouldContain("Softhyphen­wide­text");
        drawn.ShouldContain("Wordjoiner⁠wide⁠text");
    }
}
