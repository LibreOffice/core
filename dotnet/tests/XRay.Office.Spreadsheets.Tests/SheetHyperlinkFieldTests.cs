using XRay.Office.Core.Documents;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A hyperlink is not decoration on a cell — it replaces the cell's content with one field.
/// </summary>
/// <remarks>
/// <para>
/// <c>WorksheetGlobals::insertHyperlink</c>
/// (<c>sc/source/filter/oox/worksheethelper.cxx:1062-1080</c>) builds an <c>SvxURLField</c> whose
/// representation is the string the cell held and stores the result as an edit cell.
/// </para>
/// <para>
/// <strong>What follows from that is a rule about the row's height, and it is not the same rule as
/// the one about its lines.</strong> Calc measures a field cell at one line however narrow the
/// column is, which is what this class pins; the drawing path wraps the field anyway and lets it
/// overflow the row it did not size. Reading the two as one rule is what put the whole of a URL on
/// one clipped line — see <see cref="SheetFieldChopTests"/>, and the measurement in
/// <c>dotnet/probes/sheets-wrap-01/results.md</c>.
/// </para>
/// <para>
/// The height half is not cosmetic, because a URL is exactly the string a line breaker will
/// happily split at every solidus. A wrapping column of links measured four or five lines a row
/// instead of one, which is a row height, which is a page count. 33 of the 171 documents in the
/// sheets corpus carry cell hyperlinks.
/// </para>
/// <para>
/// The fixture's second row is the control — the same URL, one character different, with no
/// hyperlink on it. Without the control the test would pass for a reader that simply stopped
/// wrapping.
/// </para>
/// </remarks>
public sealed class SheetHyperlinkFieldTests
{
    private static SheetLayout Sheet()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
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
    /// Both cells wrap, and they wrap by different rules.
    /// </summary>
    /// <remarks>
    /// Both state <c>wrapText</c> and hold a URL of the same length in the same column, so the only
    /// thing that can separate them is the field — and it does, in the direction the *breaking*
    /// goes rather than in whether there is any. The unlinked one breaks after each solidus; the
    /// linked one is cut wherever it stops fitting, which lands inside a token.
    /// </remarks>
    [Fact]
    public void AFieldWrapsByChoppingAndTheSameStringWithoutOneBreaksAtItsSolidi()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-hyperlink-field.xlsx"));

        RecordingDrawingSink sink = new();
        ((SpreadsheetPages)document.Layout()).Pages[0].Draw(sink);

        List<string> drawn = [.. sink.Pages[0].Runs.Select(r => r.Text.TrimEnd())];

        // Neither URL survives whole: the linked one is chopped, the unlinked one is broken.
        drawn.ShouldNotContain("https://www.example.org/regulations/published/images/circular-1206.pdf");
        drawn.ShouldNotContain("https://www.example.org/regulations/published/images/circular-1205.pdf");

        // The unlinked one breaks after each solidus, which is what LibreOffice's own PDF shows.
        drawn.ShouldContain("https://");
        drawn.Count(t => t.Contains("1205", StringComparison.Ordinal)).ShouldBe(1);

        // The linked one is cut at the fitting limit, so its first line ends inside a token —
        // it neither ends with a solidus nor is the whole URL.
        string first = drawn.First(t => t.StartsWith("https://www", StringComparison.Ordinal));
        first.ShouldNotEndWith("/");
        first.Length.ShouldBeLessThan(
            "https://www.example.org/regulations/published/images/circular-1206.pdf".Length);
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
