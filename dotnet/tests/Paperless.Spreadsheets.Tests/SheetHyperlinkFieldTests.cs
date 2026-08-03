using Paperless.Core.Documents;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// A hyperlink is not decoration on a cell — it replaces the cell's content with one field.
/// </summary>
/// <remarks>
/// <para>
/// <c>WorksheetGlobals::insertHyperlink</c>
/// (<c>sc/source/filter/oox/worksheethelper.cxx:1062-1080</c>) builds an <c>SvxURLField</c> whose
/// representation is the string the cell held and stores the result as an edit cell. A field is
/// one indivisible portion, so it is never broken across lines: "Fields aren't wrapped, so
/// clipping is enabled to prevent a field from being drawn beyond the cell size"
/// (<c>DrawEditParam::readCellContent</c>, <c>sc/source/ui/view/output2.cxx:2560-2567</c>).
/// </para>
/// <para>
/// The consequence is not cosmetic, because a URL is exactly the string a line breaker will
/// happily split at every solidus. A wrapping column of links measured four or five lines a row
/// instead of one, which is a row height, which is a page count. 33 of the 171 documents in the
/// sheets corpus carry cell hyperlinks; measured on <c>Published_Issuances_2024.xlsx</c>, whose
/// last column is a wrapping column of links: 482 extractable words against 458, and 458 now.
/// </para>
/// <para>
/// The fixture's second row is the control — the same URL, one character different, with no
/// hyperlink on it — and LibreOffice 24.2.7.2 wraps it into five lines while leaving the linked
/// one whole. Without the control the test would pass for a reader that simply stopped wrapping.
/// </para>
/// </remarks>
public sealed class SheetHyperlinkFieldTests
{
    private static SheetLayout Sheet()
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require("sheet-hyperlink-field.xlsx"));

        return ((SpreadsheetPages)document.Layout()).Sheets[0];
    }

    [Fact]
    public void TheLinkedCellIsAFieldAndTheUnlinkedOneIsNot()
    {
        SheetLayout sheet = Sheet();

        sheet.HyperlinkRanges.Count.ShouldBe(1);
        sheet.HoldsField(0, 0).ShouldBeTrue();
        sheet.HoldsField(1, 0).ShouldBeFalse();
    }

    /// <summary>
    /// The linked cell reaches the page as one run and the unlinked one as five.
    /// </summary>
    /// <remarks>
    /// Both cells state <c>wrapText</c> and hold a URL of the same length in the same column, so
    /// the only thing that can separate them is the field.
    /// </remarks>
    [Fact]
    public void AFieldDoesNotWrapAndTheSameStringWithoutOneDoes()
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require("sheet-hyperlink-field.xlsx"));

        RecordingDrawingSink sink = new();
        ((SpreadsheetPages)document.Layout()).Pages[0].Draw(sink);

        List<string> drawn = [.. sink.Pages[0].Runs.Select(r => r.Text)];

        drawn.ShouldContain("https://www.example.org/regulations/published/images/circular-1206.pdf");
        drawn.ShouldNotContain("https://www.example.org/regulations/published/images/circular-1205.pdf");

        // The unlinked one breaks after each solidus, which is what LibreOffice's own PDF shows.
        drawn.ShouldContain("https://");
        drawn.Count(t => t.Contains("1205", StringComparison.Ordinal)).ShouldBe(1);
    }

    /// <summary>
    /// The row holding a link is measured at one line, not at the five a broken URL needs.
    /// </summary>
    /// <remarks>
    /// This is the half that moves page counts, and it is measured on the resolved grid rather
    /// than on the drawing, because the height is decided before anything is drawn.
    /// </remarks>
    [Fact]
    public void TheLinkedRowIsMeasuredAtOneLine()
    {
        SheetLayout sheet = Sheet();

        Core.Units.Length linked = sheet.Grid.Rows.PrintedSizeAt(0);
        Core.Units.Length plain = sheet.Grid.Rows.PrintedSizeAt(1);

        plain.ShouldBeGreaterThan(linked * 3);
    }
}
