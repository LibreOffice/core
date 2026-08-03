using Paperless.Core.Documents;
using Paperless.Core.Units;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// What survives a clip stays where it was standing.
/// </summary>
/// <remarks>
/// <para>
/// A string wider than the room it was given loses the characters it cannot show — "if the string
/// is clipped, make it shorter for better performance since drawing by HarfBuzz is quite
/// expensive" (<c>sc/source/ui/view/output2.cxx:2202</c>) — and the shortening is visible rather
/// than merely faster, because the PDF holds what was drawn. Dropping the <em>head</em> of a
/// right-aligned string leaves every remaining glyph exactly where it already was, so the tail
/// still ends at the cell's right margin and needs no compensating shift.
/// </para>
/// <para>
/// It was being shifted right by the width it had lost, which put the whole run over the cell's
/// right edge by that much. Measured on
/// <c>RVSM_Non_approved_list_2025_84c0b3f4ac.xlsx</c>, whose left-clipped dates then ran flush
/// into the next column's text with no gap between them: 419 extractable words against 445 — the
/// 26 missing ones were pairs the extractor could not tell apart — and 445 now.
/// </para>
/// <para>
/// Both directions are asserted from one fixture, because a fix that simply stopped shifting
/// anything must still be shown not to have moved the left-clipped case. Checked against
/// LibreOffice 24.2.7.2's own PDF, which draws <c>KLMNOP</c> ending at 208.76 pt and
/// <c>ABCDEF</c> beginning at 171.07 pt.
/// </para>
/// </remarks>
public sealed class SheetClippedAlignmentTests
{
    [Fact]
    public void AClippedTailEndsAtTheRightMarginAndAClippedHeadBeginsAtTheLeft()
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require("sheet-clipped-alignment.fods"));

        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();

        RecordingDrawingSink sink = new();
        pages.Pages[0].Draw(sink);

        List<DrawnGlyphRun> runs = [.. sink.Pages[0].Runs];

        // Sixteen characters in a 1.4 cm column: LibreOffice keeps six either way.
        DrawnGlyphRun tail = runs.First(r => r.Text.StartsWith('K'));
        DrawnGlyphRun head = runs.First(r => r.Text.StartsWith('A'));

        tail.Text.ShouldBe("KLMNOP");
        head.Text.ShouldBe("ABCDEF");

        // The two are in the same column, so the distance from where the left-aligned one starts
        // to where the right-aligned one ends is the column less its two margins — 37.69 pt in
        // the reference. The defect made it the width of the ten dropped characters longer.
        Length span = tail.Origin.X + tail.Width - head.Origin.X;
        span.Points.ShouldBe(37.69, 0.1);
    }
}
