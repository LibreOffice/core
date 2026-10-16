using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A centred wrapping cell keeps the word its line begins with.
/// </summary>
/// <remarks>
/// <para>
/// <strong>This is a lost-text defect wearing an alignment defect's clothes.</strong> A wrapped
/// line keeps the spaces it broke after, because Calc's own output draws them
/// (<c>SheetTextLayout.Wrap</c> takes a line to its <c>End</c> and not to its <c>VisibleEnd</c>).
/// Centring the line against that width subtracts half the blank from the <em>left</em>, so a
/// line whose trailing blanks outweigh its text starts outside the cell — and the cell's own clip
/// then removes whatever begins there. Nothing warns; the characters are simply not on the page.
/// </para>
/// <para>
/// Measured on <c>Infotabelle_WLAN im Flugzeug.xlsx</c> page 2, a German airline price table
/// whose runs of spaces stand in for tab stops. Its second line carried 46 trailing spaces worth
/// 151 pt of a 436 pt line against 283.1 pt of room, so the line was placed at x = −25.2 pt in a
/// cell clipped from 50.4 pt, and the bold word <c>kostenlos</c> that began it was drawn entirely
/// outside. The PDF's text layer held <c>kostenlos</c> five times against the reference's six,
/// and it holds six now. The word was in the file, in <c>xray-office extract</c>, in the wrapped
/// line's own range and in the shaped run the whole time — only the placement lost it.
/// </para>
/// <para>
/// Against LibreOffice 26.2.4.2's own PDF of that page: the reference draws the two affected
/// lines at 52.044 pt and 52.611 pt and we now draw both at 51.392 pt, the remaining 0.7 pt being
/// the cell margin and the same on every cell in the file. Before, the second was not drawn.
/// </para>
/// <para>
/// The fixture is flat ODF and deliberately smaller than the corpus document: one 4 cm centred
/// wrapping cell holding <c>WESTERLY</c>, 44 spaces and <c>EASTERLY</c>, which is enough to make
/// the first line overflow by more than the width of the word on it.
/// </para>
/// <para>
/// <strong>The placement guard is still here and is no longer what makes this fixture right.</strong>
/// This class used to assert that the first line was the word plus all 44 of its blanks, 177.37 pt
/// of it on a 110 pt line, and that the guard then pulled it back to the cell's left margin. That
/// was the symptom clamped rather than the cause removed, and the class's own remark said so:
/// "EditEngine keeps only the blanks that fit". It now does — <see cref="XRay.Office.Text.Layout.LineFiller"/>
/// breaks behind the blank that overflows — so the same fixture breaks into three lines where it
/// used to break into two, and the guard has nothing left to correct.
/// </para>
/// <para>
/// Re-measured against LibreOffice 26.2.4.2's own PDF of this fixture, which is what the counts
/// below are: the reference draws <strong>29</strong> glyphs at x = 171.47, then <strong>23</strong>
/// blanks at x = 196.19, then <strong>8</strong> at x = 200.64. We draw 29, 23 and 8 at 171.07,
/// 194.80 and 200.63. Before the break rule changed we drew 52 glyphs and then 8, so the line
/// structure disagreed with the reference's even where the placement had been corrected to agree.
/// </para>
/// </remarks>
public sealed class SheetCentredWrapBlankTests
{
    /// <summary>Column A is 4 cm and the page margin 2 cm, so the centred cell begins here.</summary>
    private const double CellLeftPoints = 170.079;

    [Fact]
    public void ACentredLineWhoseTrailingBlanksOverflowStartsAtTheCellRatherThanOutsideIt()
    {
        List<DrawnGlyphRun> runs = Draw();

        DrawnGlyphRun first = runs.First(run => run.Text.StartsWith("WESTERLY", StringComparison.Ordinal));

        // Eight letters and the twenty-one blanks that fit, the last of them the one that
        // overflowed and is compressed away — which is why 113.43 pt may exceed the 110 pt the
        // cell has and the line is still not over-full.
        first.Text.Length.ShouldBe(29, "the word and the blanks that fit, and no more");
        first.Width.Points.ShouldBe(113.43, 0.05, "the line keeps the blanks it broke after");

        first.Origin.X.Points.ShouldBeGreaterThanOrEqualTo(
            CellLeftPoints, "a centred line never begins left of the cell it is in");

        // The reference places it at 171.468 pt, which is its left margin: its line fills the
        // width, so (nMaxLineWidth - nCenterWidth) / 2 is nought. Ours is now the same line.
        first.Origin.X.Points.ShouldBe(171.07, 0.05, "at the cell's left margin, as the reference");
    }

    [Fact]
    public void TheBlanksThatDidNotFitTakeALineOfTheirOwn()
    {
        List<DrawnGlyphRun> runs = Draw();

        int first = runs.FindIndex(run => run.Text.StartsWith("WESTERLY", StringComparison.Ordinal));
        int last = runs.FindIndex(run => run.Text.StartsWith("EASTERLY", StringComparison.Ordinal));

        // Three lines, not two: 44 blanks do not fit beside an eight-letter word in 110 pt, and
        // EditEngine cuts a run of them into line-fuls rather than letting it hang. The reference
        // draws 23 blanks on this line; a renderer that lets them hang draws none of it at all.
        last.ShouldBe(first + 2, "a line of blanks stands between the two words");
        runs[first + 1].Text.ShouldAllBe(character => character == ' ', "and holds nothing else");
        runs[first + 1].Text.Length.ShouldBe(23, "as many blanks as the reference draws there");
    }

    [Fact]
    public void ALineThatFitsIsStillCentredOnItsWholeWidth()
    {
        List<DrawnGlyphRun> runs = Draw();

        // The control, and the reason the fix is bounded at both ends rather than being "do not
        // count trailing blanks". The second line fits, so nothing about it moves — 200.635 pt
        // against the reference's 200.636 pt, which it also matched before this change.
        DrawnGlyphRun second = runs.First(run => run.Text.StartsWith("EASTERLY", StringComparison.Ordinal));

        second.Origin.X.Points.ShouldBe(200.636, 0.05, "a line that fits is centred as it always was");
    }

    [Fact]
    public void ARunsTrailingBlanksComeOffItsWidthAndNothingElseDoes()
    {
        // The measurement the placement is built on, taken directly rather than through a page.
        // Only U+0020 counts, which is what EditEngine tests for (tdf#168135,
        // editeng/source/editeng/impedit3.cxx:1650); a no-break space is not a blank here.
        SheetFace face = SheetText.DefaultFace!.Value;
        Length size = Length.FromPoints(10);

        SheetTextRun plain = SheetText.Shape("WESTERLY", face, size)!;
        SheetTextRun trailed = SheetText.Shape("WESTERLY    ", face, size)!;
        SheetTextRun protectedBlanks = SheetText.Shape("WESTERLY\u00A0\u00A0", face, size)!;

        trailed.Width.ShouldBeGreaterThan(plain.Width, "the spaces are in the run's width");
        trailed.WithoutTrailingBlanks.Points.ShouldBe(plain.Width.Points, 0.01, "and come off it again");
        plain.WithoutTrailingBlanks.Points.ShouldBe(plain.Width.Points, 0.01, "a run ending in a letter is unchanged");
        protectedBlanks.WithoutTrailingBlanks.Points.ShouldBe(
            protectedBlanks.Width.Points, 0.01, "a no-break space is not a blank");
    }

    private static List<DrawnGlyphRun> Draw()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-centred-wrap-blanks.fods"));

        RecordingDrawingSink sink = new();
        ((SpreadsheetPages)document.Layout()).Pages[0].Draw(sink);

        return [.. sink.Pages[0].Runs];
    }
}
