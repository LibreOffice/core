using XRay.Office.Core.Documents;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// The ODF spelling of "Comments: at end of sheet", and the two inputs it needs.
/// </summary>
/// <remarks>
/// <para>
/// The pagination this drives is <see cref="SheetPrintNotesTests"/>' — the same
/// <c>ScPrintFunc::DoNotes</c> port, the same <c>"GW99999:"</c> mark column, the same column-major
/// order and the same 200 twips between notes. What was missing on the ODF path was neither of
/// those: it was the two <em>inputs</em>, and each hides in a different place.
/// </para>
/// <para>
/// <strong>The flag is a token inside <c>style:print</c>.</strong> ODF states what a page layout
/// prints as a space-separated list, and <c>annotations</c> is the word Calc's
/// <c>ATTR_PAGE_NOTES</c> arrives as — <c>PROP_PrintAnnotations</c> is mapped from that token by
/// <c>XMLPMPropHdl_Print(XML_ANNOTATIONS)</c>
/// (<c>xmloff/source/style/PageMasterStyleMap.cxx:80</c>,
/// <c>PageMasterPropHdlFactory.cxx:85</c>), and <c>ScPrintFunc</c> reads the item into
/// <c>aTableParam.bNotes</c> (<c>sc/source/ui/view/printfun.cxx:944</c>).
/// </para>
/// <para>
/// <strong>The notes themselves are fastened to their cells by containment</strong>, as
/// <c>office:annotation</c> children of the <c>table:table-cell</c> — so the address comes from
/// the walk and not from an attribute, and the reader has to be a second walk of the table rather
/// than a read of the content tree, which hoists an annotation out of its cell.
/// </para>
/// <para>
/// <strong>And the author line is inside the text rather than in <c>dc:creator</c></strong>, which
/// says <c>Unknown Author</c> on every note this fixture holds while the printed page opens each
/// one with <c>Probe:</c>. A reader that composed the two would print the wrong name.
/// </para>
/// <para>
/// The fixture is 26.2.4.2's own <c>--convert-to fods</c> of <c>sheet-print-notes.xls</c>, so the
/// markup is what LibreOffice itself writes for a workbook asking for note pages, and its own
/// rendering of the converted file is the ground truth: <strong>two pages</strong>, the second
/// listing <c>A1:</c>, <c>A2:</c>, <c>C1:</c> in that order. Column-major is the discriminating
/// half — reading order would put <c>C1</c> second.
/// </para>
/// <para>
/// Reach is <strong>2 of the 307 converted <c>.ods</c></strong>, and both were failing:
/// <c>Hazard Analysis Template.ods</c> at 2 pages against 3 and 2131 characters against 3313, and
/// <c>RMP 2011-2014 and Inventory.ods</c> at 36 pages against 38. Both are page-exact after this,
/// the second character-exact as well.
/// </para>
/// </remarks>
public sealed class SheetOdsPrintNotesTests
{
    private const string Fixture = "sheet-print-notes.fods";

    private static SpreadsheetPages Pages()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require(Fixture));

        return (SpreadsheetPages)document.Layout();
    }

    [Fact]
    public void TheAnnotationsTokenOfStylePrintReachesTheSheet()
    {
        Pages().Sheets[0].Setup.PrintsNotes.ShouldBeTrue(
            "style:print lists \"annotations\" for Excel's \"Comments: at end of sheet\"");
    }

    [Fact]
    public void EachAnnotationIsJoinedToTheCellThatContainsIt()
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
    public void TheAuthorLineComesFromTheTextRatherThanFromDublinCore()
    {
        // Every annotation in the fixture states `<dc:creator>Unknown Author</dc:creator>`, and
        // the reference prints none of them: the name it does print is the one the note's own
        // first paragraph carries.
        string text = Pages().Sheets[0].Notes.Items[0].Text;

        text.ShouldContain("Probe:");
        text.ShouldNotContain("Unknown Author");
    }

    [Fact]
    public void TheNotesAreListedOnAPageOfTheirOwnAfterTheCells()
    {
        SpreadsheetPages pages = Pages();

        pages.Pages.Count.ShouldBe(2, "one page of cells and one of notes");
        pages.Pages[0].IsNotePage.ShouldBeFalse();
        pages.Pages[1].IsNotePage.ShouldBeTrue();
    }

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
}
