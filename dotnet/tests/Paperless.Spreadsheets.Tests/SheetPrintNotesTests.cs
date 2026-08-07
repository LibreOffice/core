using Paperless.Core.Documents;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// A sheet asking for "Comments: at end of sheet" gets pages of its own after its cells.
/// </summary>
/// <remarks>
/// <para>
/// Excel's setting, <c>EXC_SETUP_PRINTNOTES</c> in a BIFF <c>SETUP</c> record
/// (<c>sc/source/filter/excel/xipage.cxx:84</c>, <c>:257</c>), which Calc prints as extra pages
/// listing each note against its cell's address: the marks flush left in a column the width of
/// <c>"GW99999:"</c>, the note beside them, 200 twips between two notes, and the sheet's own
/// header and footer on top (<c>ScPrintFunc::DoNotes</c>,
/// <c>sc/source/ui/view/printfun.cxx:1930-2001</c>).
/// </para>
/// <para>
/// <strong>The order is column-major, not reading order</strong> — <c>CountNotePages</c> walks
/// the print area's columns outermost (<c>printfun.cxx:2591-2600</c>), so the fixture puts its
/// three notes on A1, C1 and A2 to tell the two apart: column-major gives A1, A2, C1 and reading
/// order would give A1, C1, A2.
/// </para>
/// <para>
/// The fixture is LibreOffice 24.2.7.2's own <c>--convert-to xls</c> of a hand-authored flat ODS,
/// so the <c>NOTE</c>, <c>OBJ</c> and <c>TXO</c> records are the ones LibreOffice itself writes.
/// Its reference PDF is two pages — one of cells, one of notes — and Paperless's
/// layout-preserved text layer is identical to it.
/// </para>
/// <para>
/// <strong>Its three object identifiers were then permuted to 31, 11 and 21</strong>, and that is
/// the only edit. LibreOffice numbers the comment objects it writes 1, 2, 3 in the order it writes
/// their <c>NOTE</c> records, which makes "the n-th <c>OBJ</c> is the n-th <c>NOTE</c>" — the
/// pairing the rest of <c>XlsDrawingCollector</c> uses for shapes, and the plausible wrong
/// implementation here — indistinguishable from the identifier join the format specifies.
/// Measured: with the sequential identifiers, replacing the join with ordinal pairing left all six
/// of these tests green; with the permuted ones it fails five of them. Real workbooks number
/// comments arbitrarily — <c>RMP 2011-2014 and Inventory.xls</c> uses 1029, 1030, 1033, 1036,
/// 1037, 1124, 1125 and 1126 — so the permutation makes the fixture more like a real file rather
/// than less. LibreOffice's own PDF for the permuted file is identical to the one for the
/// original.
/// </para>
/// <para>
/// Measured on the corpus: exactly two of the 171 sheets-track documents ask for this.
/// <c>Hazard Analysis Template.xls</c> went from 2 pages against 3 and 460 words against 682 to
/// 3 against 3 and 682 against 682; <c>RMP 2011-2014 and Inventory.xls</c> gained the two note
/// pages the reference draws at its pages 22 and 38, with the same eight and four marks in the
/// same order, and keeps a one-page shortfall of its own on the first sheet's cells.
/// </para>
/// </remarks>
public sealed class SheetPrintNotesTests
{
    private const string Fixture = "sheet-print-notes.xls";

    private static SpreadsheetPages Pages()
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require(Fixture));

        return (SpreadsheetPages)document.Layout();
    }

    [Fact]
    public void TheSetupFlagReachesTheSheet()
    {
        Pages().Sheets[0].Setup.PrintsNotes.ShouldBeTrue(
            "the SETUP record's 0x0020 bit is Excel's \"Comments: at end of sheet\"");
    }

    /// <summary>
    /// The text of each note reaches the sheet, joined to its cell through the object identifier.
    /// </summary>
    /// <remarks>
    /// A <c>NOTE</c> record carries the cell and names an object; the characters are in that
    /// object's <c>TXO</c>. Neither record alone is enough, so this asserts the join rather than
    /// either half of it.
    /// </remarks>
    [Fact]
    public void EachNotesTextIsJoinedToItsCell()
    {
        IReadOnlyList<SheetNote> notes = Pages().Sheets[0].Notes.Items;

        notes.Count.ShouldBe(3);
        notes.ShouldContain(note => note.Column == 0 && note.Row == 0
                                    && note.Text.Contains("on A1", StringComparison.Ordinal));
        notes.ShouldContain(note => note.Column == 0 && note.Row == 1
                                    && note.Text.Contains("on A2", StringComparison.Ordinal));
        notes.ShouldContain(note => note.Column == 2 && note.Row == 0
                                    && note.Text.Contains("on C1", StringComparison.Ordinal));
    }

    [Fact]
    public void TheNotesAreListedOnAPageOfTheirOwnAfterTheCells()
    {
        SpreadsheetPages pages = Pages();

        pages.Pages.Count.ShouldBe(2, "one page of cells and one of notes");
        pages.Pages[0].IsNotePage.ShouldBeFalse();
        pages.Pages[1].IsNotePage.ShouldBeTrue();
    }

    /// <summary>
    /// The marks are drawn in column-major order, which is the whole of what the fixture is for.
    /// </summary>
    [Fact]
    public void TheMarksAreDrawnInColumnMajorOrder()
    {
        SpreadsheetPages pages = Pages();

        RecordingDrawingSink sink = new();
        pages.Pages[1].Draw(sink);

        List<string> marks = [.. sink.Pages[0].Runs
            .Select(run => run.Text)
            .Where(text => text.Length > 1 && text[^1] == ':' && char.IsAsciiLetterUpper(text[0])
                           && text[1..^1].Length > 0 && char.IsAsciiDigit(text[^2]))];

        marks.ShouldBe(["A1:", "A2:", "C1:"]);
    }

    /// <summary>
    /// Every note's text reaches the page, wrapped beside its mark.
    /// </summary>
    [Fact]
    public void EveryNotesTextIsDrawnOnTheNotePage()
    {
        SpreadsheetPages pages = Pages();

        RecordingDrawingSink sink = new();
        pages.Pages[1].Draw(sink);

        string drawn = string.Join(" ", sink.Pages[0].Runs.Select(run => run.Text));

        drawn.ShouldContain("the note on A1");
        drawn.ShouldContain("the note on A2");
        drawn.ShouldContain("the note on C1");
    }

    /// <summary>
    /// A sheet that holds notes and does not ask for them keeps its own page count.
    /// </summary>
    /// <remarks>
    /// The flag is the whole of the difference, and it is worth asserting separately because the
    /// notes are read whether or not it is set: a reader that listed them unconditionally would
    /// add a page to every workbook carrying a comment, which is 17 of the corpus's 534 documents
    /// against the 2 that ask for the pages.
    /// </remarks>
    [Fact]
    public void ASheetThatDoesNotAskForNotePagesGetsNone()
    {
        SheetLayout sheet = Pages().Sheets[0];

        SheetNotePages.Paginate(sheet).Count.ShouldBe(1);

        SheetLayout quiet = new()
        {
            Name = sheet.Name,
            Index = sheet.Index,
            Setup = sheet.Setup with { PrintsNotes = false },
            Grid = sheet.Grid,
            Cells = sheet.Cells,
            Notes = sheet.Notes,
        };

        SheetNotePages.Paginate(quiet).ShouldBeEmpty();
    }
}
