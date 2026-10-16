using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A wrapping cell narrower than its own text still wraps — a character to a line — and draws
/// nothing at all only when its text begins outside its column.
/// </summary>
/// <remarks>
/// <para>
/// Two rules of one mechanism, and getting either half alone wrong loses a whole cell's text.
/// Only a wrapping cell has a paper: <c>DrawEditParam::calcPaperSize</c>
/// (<c>sc/source/ui/view/output2.cxx</c>:2684-2700) gives the EditEngine
/// <c>rAlignRect.GetWidth() − nLeftM − nRightM</c> under <c>if (rParam.mbBreak)</c> alone, and a
/// column narrower than its margins makes that negative.
/// <c>ImpEditEngine::calculateMaxLineWidth</c> (<c>editeng/source/editeng/impedit3.cxx</c>:530-545)
/// clamps it to one unit rather than giving up, so every character takes a line of its own; and
/// the block that results is drawn unless it begins past the column's right edge, where
/// <c>DrawText_ToRectangle</c> strips its portions against the cell's rectangle and emits none
/// (<c>impedit3.cxx</c>:3408-3440).
/// </para>
/// <para>
/// This tree used to answer neither: a cell with no room fell back to one unbroken line written
/// across whatever was beside it. On <c>075_Idea_planner_tasks_f44ebd73.xlsx</c>, whose <c>B6</c>
/// holds <c>" Task Status Indicator"</c> in a column one character wide with one indent level,
/// that put nineteen alphanumeric characters into the PDF's text layer that 26.2.4.2 does not
/// draw, which is the whole of that document's gate failure — 503 characters against 484.
/// </para>
/// <para>
/// The numbers below are LibreOffice 26.2.4.2's own PDF of the fixture, read with
/// <c>pdftotext -bbox</c>: six single-character lines at x = 57.685, tops 29.525, 40.721, 51.918,
/// 63.115, 74.312 and 85.509. The step is measured rather than asserted from arithmetic —
/// <c>probes/xlsx-chart-r84/</c> holds the twelve renderings of the corpus document that place it
/// between column widths of 5.25 pt and 6.11 pt against a margin and indent of 5.70 pt.
/// </para>
/// </remarks>
public sealed class SheetNarrowWrapTests
{
    /// <summary>The left edge of the narrow column's text, in both renderings.</summary>
    private const double NarrowTextLeftPoints = 57.685;

    [Fact]
    public void AWrappingCellNarrowerThanOneCharacterTakesACharacterPerLine()
    {
        List<DrawnGlyphRun> runs = Draw();

        string.Concat(NarrowLines(runs).Select(run => run.Text))
            .ShouldBe("NARROW", "one character to a line, as the reference draws it");
    }

    [Fact]
    public void ItsLinesFallWhereTheReferenceDrawsThem()
    {
        List<DrawnGlyphRun> runs = Draw();

        // The reference's own tops for the six lines. `pdftotext` reports the ink's box and a
        // recorded run its baseline, so only the pitch and the first line are compared here.
        List<DrawnGlyphRun> narrow = NarrowLines(runs);

        narrow.Count.ShouldBe(6, "six characters, six lines");

        for (int at = 1; at < narrow.Count; at++)
        {
            (narrow[at].Origin.Y.Points - narrow[at - 1].Origin.Y.Points)
                .ShouldBe(11.196, 0.05, "the reference's own line pitch, 11.196 pt");
        }
    }

    [Fact]
    public void ACellThatDoesNotWrapIsUnmovedAndStillSpills()
    {
        List<DrawnGlyphRun> runs = Draw();

        // The control that says this is the wrap flag rather than the column width: the same
        // 1.42 pt column draws its whole string when the cell does not wrap, and the reference
        // draws it at the same x.
        DrawnGlyphRun spilled = runs.First(run => run.Text.StartsWith("SPILLED", StringComparison.Ordinal));

        spilled.Origin.X.Points.ShouldBe(NarrowTextLeftPoints, 0.05, "where 26.2.4.2 draws it");
    }

    [Fact]
    public void TheWideNeighboursAreUntouched()
    {
        List<DrawnGlyphRun> runs = Draw();

        // 26.2.4.2 draws both at 59.074; nothing about a narrow neighbour may move a cell that
        // has room of its own.
        foreach (string word in new[] { "CONTROL", "BLOCKER" })
        {
            runs.First(run => run.Text.StartsWith(word, StringComparison.Ordinal))
                .Origin.X.Points.ShouldBe(59.074, 0.05, $"{word} is where the reference draws it");
        }
    }

    [Fact]
    public void TextThatBeginsAtOrPastTheColumnsRightEdgeIsNotDrawnAtAll()
    {
        // The predicate itself, at the values the corpus document's twelve renderings pin it to:
        // a 5.70 pt margin-and-indent in a 5.25 pt column draws nothing and in a 6.11 pt column
        // draws everything.
        SheetTextLayout.StartsOutsideItsCell(Length.FromPoints(5.25), Length.FromPoints(5.70))
            .ShouldBeTrue("the block begins past the column's right edge");
        SheetTextLayout.StartsOutsideItsCell(Length.FromPoints(6.11), Length.FromPoints(5.70))
            .ShouldBeFalse("it begins inside, so every line of it is drawn");

        // And a negative paper is not the test: the fixture's narrow column is 1.42 pt against
        // margins of 0.99 pt a side, so its paper is negative and 26.2.4.2 draws all six lines.
        SheetTextLayout.StartsOutsideItsCell(Length.FromPoints(1.42), Length.FromPoints(0.99))
            .ShouldBeFalse("a negative paper still starts inside the cell");
    }

    /// <summary>The narrow cell's own lines: one character each, at the column's own x.</summary>
    private static List<DrawnGlyphRun> NarrowLines(List<DrawnGlyphRun> runs) =>
        [.. runs
            .Where(run => run.Text.Length == 1
                          && Math.Abs(run.Origin.X.Points - NarrowTextLeftPoints) < 0.05)
            .OrderBy(run => run.Origin.Y.Points)];

    private static List<DrawnGlyphRun> Draw()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-narrow-wrap.fods"));

        RecordingDrawingSink sink = new();
        ((SpreadsheetPages)document.Layout()).Pages[0].Draw(sink);

        return [.. sink.Pages[0].Runs];
    }
}
