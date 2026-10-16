using XRay.Office.Core.Documents;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A wrapping cell that is one hyperlink field breaks only by chopping, never at a separator.
/// </summary>
/// <remarks>
/// <para>
/// The fixture is six wrap-enabled cells in one 30-character column, the same three strings once
/// plain and once carrying a sheet-level hyperlink, at a row height that leaves room for every
/// line. It exists to separate two explanations of the same symptom, and only one of them
/// survives contact with the reference: if the character-break fallback were missing, every
/// space-free token would fail; if the field were atomic, only the linked arm would.
/// </para>
/// <para>
/// Measured against the installed LibreOffice 26.2.4.2 — see
/// <c>dotnet/probes/sheets-wrap-01/results.md</c>, which holds the reference's own lines for all
/// six rows. The expectations below are LibreOffice's output, not ours.
/// </para>
/// <para>
/// The mechanism is in <see cref="SheetFieldBreaker"/>: Calc's import replaces a hyperlinked
/// string cell with an edit cell whose content node is a single <c>EE_FEATURE_FIELD</c>
/// character, and EditEngine hands <em>that</em> to the break iterator. A one-character node
/// offers nothing, so every line comes from <c>// No separator in line =&gt; Chop!</c>.
/// </para>
/// </remarks>
public sealed class SheetFieldChopTests
{
    private const string Url =
        "https://www.bsp.gov.ph/Regulations/Published%20Issuances/Images/M-2024-039.pdf";

    private static List<string> Drawn()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-field-chop.xlsx"));

        RecordingDrawingSink sink = new();
        ((SpreadsheetPages)document.Layout()).Pages[0].Draw(sink);

        // Trailing blanks are kept on a wrapped line by design — Calc's own output shows them —
        // and they are not what any of this is about.
        return [.. sink.Pages[0].Runs.Select(r => r.Text.TrimEnd())];
    }

    /// <summary>The cells the fixture says are links are the cells that are fields.</summary>
    /// <remarks>
    /// The control on everything below: rows 2, 4 and 6 are hyperlinked and rows 1, 3 and 5 hold
    /// the identical strings unlinked, so nothing but the field can separate them.
    /// </remarks>
    [Fact]
    public void EveryOtherRowIsAField()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-field-chop.xlsx"));

        SheetLayout sheet = ((SpreadsheetPages)document.Layout()).Sheets[0];

        for (int row = 0; row < 6; row++) sheet.HoldsField(row, 0).ShouldBe(row % 2 == 1);
    }

    /// <summary>
    /// A hyperlinked URL wraps, and its breaks land mid-token rather than after a solidus.
    /// </summary>
    /// <remarks>
    /// This is the defect the fixture was written for. X-Ray.Office drew the whole URL on one clipped
    /// line, which cost <c>Published_Issuances_2024.xlsx</c> the second line of all 22 of its rows
    /// and 22 extractable words. The three lines below are character for character what
    /// LibreOffice draws.
    /// </remarks>
    [Fact]
    public void ALinkedUrlIsChoppedWhereItStopsFitting()
    {
        List<string> drawn = Drawn();

        drawn.ShouldContain("https://www.bsp.gov.ph/Regulation");
        drawn.ShouldContain("s/Published%20Issuances/Images/M");
        drawn.ShouldContain("-2024-039.pdf");

        // And never whole: one clipped line is the bug.
        drawn.ShouldNotContain(Url);
    }

    /// <summary>
    /// The same URL unlinked breaks after its solidi instead, so the field is what decided it.
    /// </summary>
    /// <remarks>
    /// Deliberately weaker than the linked assertion. Where the *plain* path puts its last two
    /// breaks is a separate question from this one — X-Ray.Office and LibreOffice already disagree by
    /// one break there, ours cutting after the hyphen of <c>M-</c> where the reference cuts before
    /// the <c>M</c> — and pinning it here would tie this test to a defect it is not about.
    /// </remarks>
    [Fact]
    public void TheSameUrlUnlinkedBreaksAfterASolidus()
    {
        List<string> drawn = Drawn();

        drawn.ShouldContain("https://www.bsp.gov.ph/");
        drawn.ShouldContain("Regulations/");
    }

    /// <summary>
    /// A space inside a field is not a break opportunity either.
    /// </summary>
    /// <remarks>
    /// The sharpest of the six rows, and the one that rules out every explanation but the node.
    /// A break-opportunity rule of any kind — Unicode's, a URL-aware one, a punctuation-aware one
    /// — would break at the blank one character away. LibreOffice cuts <c>foxtrot</c> in half.
    /// </remarks>
    [Fact]
    public void ALinkedSentenceIsChoppedMidWordRatherThanAtItsBlanks()
    {
        List<string> drawn = Drawn();

        drawn.ShouldContain("alpha bravo charlie delta echo foxtr");
        drawn.ShouldContain("ot golf hotel india juliet kilo lima");
    }

    /// <summary>The same sentence unlinked breaks at a blank, as any text does.</summary>
    [Fact]
    public void TheSameSentenceUnlinkedBreaksAtABlank()
    {
        List<string> drawn = Drawn();

        drawn.ShouldContain("alpha bravo charlie delta echo");
        drawn.ShouldContain("foxtrot golf hotel india juliet kilo");
        drawn.ShouldContain("lima");
    }

    /// <summary>
    /// A field's lines are set closer together than any other cell's — by its ascent, not its
    /// line height.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The two space-free cells are the measurement, because they wrap into the same three lines
    /// with the same glyphs, so the only thing that can differ between them is the pitch. Carlito
    /// at 10 pt: ascent 1950/2048 em = 9.52 pt, ascent plus descent 12.21 pt, and the reference
    /// puts the linked cell's lines 9.50 pt apart against the unlinked cell's 12.19.
    /// </para>
    /// <para>
    /// Sixteen single-cell workbooks — four faces at four sizes, in
    /// <c>dotnet/probes/sheets-wrap-01</c> — put every field pitch on the face's <c>hhea</c>
    /// ascent, within the tenth of a point the reference's device quantises to. Before this was
    /// reproduced the gap was 1.7 to 5.4 pt, which on a real workbook is a wrapped line landing in
    /// the row beneath.
    /// </para>
    /// </remarks>
    [Fact]
    public void AFieldsLinesArePitchedByItsAscentAndAPlainCellsByItsLineHeight()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-field-chop.xlsx"));

        RecordingDrawingSink sink = new();
        ((SpreadsheetPages)document.Layout()).Pages[0].Draw(sink);

        // The two runs of 'A'…'P' — row 3 plain, row 4 linked — ordered down the page.
        List<DrawnGlyphRun> block = [.. sink.Pages[0].Runs
            .Where(run => run.Text.StartsWith("AAAABBBB", StringComparison.Ordinal)
                          || run.Text.StartsWith("GHHHH", StringComparison.Ordinal))
            .OrderBy(run => run.Origin.Y.Emu)];

        block.Count.ShouldBe(4, "two cells, each wrapping onto a first and a second line");

        Core.Units.Length plain = block[1].Origin.Y - block[0].Origin.Y;
        Core.Units.Length field = block[3].Origin.Y - block[2].Origin.Y;

        // Strictly closer together, and by roughly the descent — 12.21 against 9.52 in Carlito.
        field.ShouldBeLessThan(plain);
        (plain.Points / field.Points).ShouldBe(1.28, tolerance: 0.02);
    }

    /// <summary>
    /// A token with no break opportunity in it comes out identical linked and unlinked.
    /// </summary>
    /// <remarks>
    /// The arm that refutes "the character-break fallback is missing": the chop is all either path
    /// had to work with, so the two agree exactly. If this were the failing arm the seat would be
    /// in <c>XRay.Office.Text</c> rather than here.
    /// </remarks>
    [Fact]
    public void ATokenWithNoOpportunityChopsTheSameWayLinkedOrNot()
    {
        List<string> drawn = Drawn();

        foreach (string line in
                 (string[])["AAAABBBBCCCCDDDDEEEEFFFFGGG", "GHHHHIIIIJJJJKKKKLLLLMMMMNNN", "NOOOOPPPP"])
        {
            drawn.Count(t => t == line).ShouldBe(2, $"'{line}' should be drawn on both rows");
        }
    }
}
