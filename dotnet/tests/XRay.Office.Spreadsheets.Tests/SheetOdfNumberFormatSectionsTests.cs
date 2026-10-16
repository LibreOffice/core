using XRay.Office.Core.Documents;
using XRay.Office.Core.Graphics;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// The ODF half of a multi-section number format, drawn: what each cell says and what colour it
/// is in, against 26.2.4.2's own rendering of the same file.
/// </summary>
/// <remarks>
/// <para>
/// <c>features/sheet-odf-numfmt-sections.ods</c> is 26.2.4.2's own ODS export of a workbook
/// stating six format codes, so the markup under test is the exporter's. Six columns, one per
/// format; four rows — 150, −100, 0 and the text <c>text</c> — so a cell's address states which
/// arm it is.
/// </para>
/// <para>
/// Every expectation below was read out of the reference's own PDF of this file before it was
/// written down. On the three rows that matter the two renderings agree span for span and colour
/// for colour; the assertions here are on the cells the section rule decides.
/// </para>
/// </remarks>
public sealed class SheetOdfNumberFormatSectionsTests
{
    private const string Fixture = "sheet-odf-numfmt-sections.ods";

    private static List<DrawnGlyphRun> Runs()
    {
        using IPaginatedDocument document =
            (IPaginatedDocument)OfficeDocument.Open(Corpus.Require(Fixture));

        RecordingDrawingSink sink = new();
        ((SpreadsheetPages)document.Layout()).Pages[0].Draw(sink);
        return sink.Pages[0].Runs;
    }

    /// <summary>What one row draws, as sorted <c>text #rrggbb</c> pairs.</summary>
    /// <remarks>
    /// Keyed on the baseline rather than on the text, because four of the six columns draw
    /// <c>0.0</c> or <c>150.0</c> on some row and a test that picked a run by its text would be
    /// asserting on whichever one came first.
    /// </remarks>
    private static List<string> Row(int index)
    {
        List<DrawnGlyphRun> runs = [.. Runs()
            .Where(r => r.Text.Trim().Length > 0)
            .OrderBy(r => r.Origin.Y.Points)];

        // Clustered rather than rounded: two cells of one row can differ by a fraction of a point
        // where their formats differ, and a rounding boundary between them silently drops one of
        // the six from the comparison.
        List<List<DrawnGlyphRun>> rows = [];
        foreach (DrawnGlyphRun run in runs)
        {
            if (rows.Count == 0
                || Math.Abs(run.Origin.Y.Points - rows[^1][0].Origin.Y.Points) > 1.0)
            {
                rows.Add([]);
            }

            rows[^1].Add(run);
        }

        return [.. rows[index]
            .Select(r => $"{r.Text.Trim()} #{((SolidPaint)r.Paint).Colour.ToArgb() & 0xFFFFFF:x6}")
            .Order(StringComparer.Ordinal)];
    }

    [Fact]
    public void TheNegativeRowIsDrawnSectionForSectionAsTheReferenceDrawsIt()
        // Read out of 26.2.4.2's own PDF of this file, which draws exactly these six.
        // `(100)` in red is column B's second section and `(100)` in black is column C's; a
        // reader that compiled the named element alone drew neither, and drew the parentheses on
        // the positive row instead.
        => Row(1).ShouldBe([
            "(100) #000000",
            "(100) #ff0000",
            "-100.0 #000000",
            "-100.0 #ff0000",
            "-100.00 #000000",
            "neg 100 #000000",
        ]);

    [Fact]
    public void TheZeroRowTakesTheOwningStylesOwnBodyAndItsColour()
        // Column A's element carries the ZERO section and its two maps carry positive and
        // negative, so the blue belongs here and nowhere else. Column E is `[COLOR10]0.0`, whose
        // `fo:color="#dddddd"` is not one of the ten keyword colours and is dropped by the
        // reference on re-import as well — hence a second, black `0.0` rather than a pale one.
        => Row(2).ShouldBe([
            "- #000000",
            "0 #000000",
            "0.0 #000000",
            "0.0 #000000",
            "0.0 #0000ff",
            "0.00 #000000",
        ]);

    [Fact]
    public void ThePositiveRowTakesTheMappedSectionsAndIsUncoloured()
        => Row(0).ShouldBe([
            "150 #000000",
            "150 #000000",
            "150.0 #000000",
            "150.0 #000000",
            "150.00 #000000",
            "big 150 #000000",
        ]);

    [Fact]
    public void ATextValueTakesTheTextStyleOwnersOwnBody()
        // The accounting format's owner is the `number:text-style`, so the fourth row is the one
        // row where the element the cell names is what draws — and all six columns draw the
        // string through it or through General.
        => Row(3).ShouldBe([
            "text #000000", "text #000000", "text #000000",
            "text #000000", "text #000000", "text #000000",
        ]);
}
