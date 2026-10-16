using System.Globalization;
using System.Text.RegularExpressions;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A wrapping cell's lines stop where the row runs out, and the ones that stop are not drawn at
/// all — not clipped, not hidden, absent.
/// </summary>
/// <remarks>
/// <para>
/// <c>ScOutputData::DrawEditStandard</c> calls
/// <c>EnableSkipOutsideFormat(meVerJust==Top || meVerJust==Standard)</c>
/// (<c>sc/source/ui/view/output2.cxx:3115</c>) and the EditEngine then refuses to
/// <em>format</em> the lines past the paper height (<c>ImpEditEngine::CreateLines</c>,
/// <c>editeng/source/editeng/impedit3.cxx:1801-1806</c>). The paper is the cell's own,
/// <c>rAlignRect.GetHeight() - nTopM - nBottomM</c>, and only a wrapping cell has one at all —
/// everything else keeps <c>Size(1000000, 1000000)</c>.
/// </para>
/// <para>
/// <strong>This is the only rule on the cell-text path that a word count can see.</strong> A clip
/// cuts ink and leaves every glyph in the PDF's text layer, which is why
/// <see cref="SheetBlockClipTests"/>'s rule moved no word count in either direction. This one is
/// upstream of drawing. Measured on
/// <c>sheets/batch-011/xls/T0A0D0000090006XLSE.xls</c>, page-exact at 162/162: we drew
/// <strong>42471 words against the reference's 40382</strong>, and 13 491 more non-space
/// characters, because we drew every line of every over-full wrapped cell. With this rule the
/// same document reads 40379.
/// </para>
/// <para>
/// Every expectation here is read off LibreOffice 26.2.4.2's own output for
/// <c>sheet-vclip-row.fods</c>, whose header carries the full table and the arithmetic.
/// </para>
/// </remarks>
public sealed class SheetVerticalOverflowTests
{
    /// <summary>Tokens are `a001`… — the letter says which row, the number how far the text got.</summary>
    private static readonly Regex Token = new(@"([a-i])(\d{3})", RegexOptions.Compiled);

    /// <summary>How many lines each row drew, and the highest-numbered token on any of them.</summary>
    private static Dictionary<char, (int Lines, int LastToken)> Drawn()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-vclip-row.fods"));

        PlacedDrawingSink sink = new();
        foreach (SheetPage page in ((SpreadsheetPages)document.Layout()).Pages) page.Draw(sink);

        Dictionary<char, HashSet<long>> lines = [];
        Dictionary<char, int> last = [];
        foreach ((GlyphRun run, DocPoint origin) in sink.Runs)
        {
            foreach (Match match in Token.Matches(run.Text))
            {
                char row = match.Groups[1].Value[0];
                (lines.TryGetValue(row, out HashSet<long>? at) ? at : lines[row] = [])
                    .Add(origin.Y.Emu);
                int number = int.Parse(match.Groups[2].Value, CultureInfo.InvariantCulture);
                if (!last.TryGetValue(row, out int seen) || number > seen) last[row] = number;
            }
        }

        return lines.ToDictionary(pair => pair.Key, pair => (pair.Value.Count, last[pair.Key]));
    }

    [Fact]
    public void AWrappingTopAlignedCellDrawsOnlyTheLinesItsRowHasRoomFor()
    {
        Dictionary<char, (int Lines, int LastToken)> drawn = Drawn();

        // Row 3: a 3 cm row, so 83.06 pt of paper against a pitch of 11.20 — 7.42 pitches, and
        // the line that crosses the edge is drawn before the formatting stops. Eight lines in
        // 26.2.4.2, ending at c032; the cell holds sixty tokens.
        drawn['c'].Lines.ShouldBe(8);
        drawn['c'].LastToken.ShouldBe(32);
    }

    [Fact]
    public void ARowShorterThanFourLinesStillDrawsFour()
    {
        // "Format at least two lines though, in case something detects whether the text has been
        // wrapped or something similar" (impedit3.cxx:1799). Counted from outside the engine the
        // allowance is four: a 1 cm row holds 2.35 pitches and draws four lines all the same.
        // Without it a one-line row would lose a wrapped cell's text outright, which is the one
        // way this change could have done real damage.
        Dictionary<char, (int Lines, int LastToken)> drawn = Drawn();

        drawn['a'].Lines.ShouldBe(4);
        drawn['a'].LastToken.ShouldBe(16);
    }

    [Fact]
    public void RoomThatIsAnExactMultipleOfThePitchGetsOneLineMoreThanTheMultiple()
    {
        // 58 pt less 1.98 pt of margin is 56.02, which is 5.002 pitches. The engine's test is
        // `maPaperSize.Height() < nCurrentPosY`, strict, so the fifth line does not end the
        // formatting and a sixth is created. `ceil` would answer five and be wrong here, which is
        // why the port walks the lines instead of dividing.
        Dictionary<char, (int Lines, int LastToken)> drawn = Drawn();

        drawn['b'].Lines.ShouldBe(6);
        drawn['b'].LastToken.ShouldBe(24);
    }

    [Fact]
    public void ACellAlignedToTheBottomOrTheMiddleIsNotTruncated()
    {
        // The guard names Top and Standard and nothing else, so the same text in the same 1 cm row
        // keeps all sixty tokens on fifteen lines when the cell is bottom- or middle-aligned —
        // drawn far outside its row in both cases, exactly as the reference draws it.
        Dictionary<char, (int Lines, int LastToken)> drawn = Drawn();

        drawn['d'].ShouldBe((15, 60));
        drawn['e'].ShouldBe((15, 60));
    }

    [Fact]
    public void ACellThatDoesNotWrapIsNeverTruncatedHoweverManyBreaksItHolds()
    {
        // `calcPaperSize` is called only under `if (rParam.mbBreak)` (output2.cxx:3075-3085), so a
        // cell that does not wrap keeps the initial 1 000 000 and no paragraph of it is ever
        // dropped. Twenty hard-break paragraphs in a 1 cm row: the reference draws all twenty, on
        // twenty lines.
        //
        // Only the *token* count is asserted, and the omission is deliberate rather than a
        // weakening. We draw those twenty paragraphs on **one** line, because `Wrap` — which is
        // what splits at a hard break — is reached only for a cell that wraps, while Calc sends
        // any cell holding a break to an EditEngine whatever its wrap setting
        // (`DrawEditParam::hasLineBreak`, output2.cxx:2730). That is a real defect and a
        // pre-existing one; this fixture found it and `probes/sheets-b011-01/results.md` records
        // it. What this test is for is that the truncation rule does not reach such a cell, and
        // the token count is exactly that question.
        Dictionary<char, (int Lines, int LastToken)> drawn = Drawn();

        drawn['f'].LastToken.ShouldBe(20);
    }

    [Fact]
    public void AParagraphAfterTheRoomRanOutIsDroppedWholeRatherThanAllowedItsFourLines()
    {
        // The coarser guard one level up (impedit3.cxx:676-680) refuses a paragraph after the
        // first whose first line would start past the paper, and it is asked *before* the
        // four-line allowance — so six one-line paragraphs in a 1 cm row give three, not four.
        // A rule that counted the allowance per cell rather than per paragraph would say four.
        Dictionary<char, (int Lines, int LastToken)> drawn = Drawn();

        drawn['g'].ShouldBe((3, 3));
    }

    [Fact]
    public void ACellWithNoStatedVerticalAlignmentIsTruncatedAndStillPlacedFromTheBottom()
    {
        // Standard is in the guard although it draws from the bottom of the row, which reads like
        // a contradiction and is what the reference does: the text is cut to four lines and those
        // four are then bottom-aligned, so the cell's *first* line is what survives and it is
        // drawn above the row rather than in it.
        Dictionary<char, (int Lines, int LastToken)> drawn = Drawn();

        drawn['h'].ShouldBe((4, 16));
    }

    [Fact]
    public void ACellThatFitsItsRowKeepsEveryLine()
    {
        // The control. Nothing about this change may touch a cell with room to spare, and the
        // three tokens of row 9 are one line in a 3 cm row.
        Dictionary<char, (int Lines, int LastToken)> drawn = Drawn();

        drawn['i'].ShouldBe((1, 3));
    }
}
