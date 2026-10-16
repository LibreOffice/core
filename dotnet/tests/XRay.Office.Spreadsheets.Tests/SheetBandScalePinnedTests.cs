using XRay.Office.Core.Documents;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// On a scaled sheet, a band with room to spare centres its text in the band <em>at the print
/// scale</em> — and is not rejected by the clip that is then tested against it.
/// </summary>
/// <remarks>
/// <para>
/// <c>PrintHF</c> does its whole arithmetic in logical twips and lets the map mode apply the zoom
/// to all of it at once (<c>sc/source/ui/view/printfun.cxx:1867</c>, <c>:2645</c>), so
/// <c>nDif = paperHeight - textHeight</c> reaches the paper as
/// <c>(paperHeight - textHeight) x zoom / 2</c>. <see cref="SheetPageDecoration"/> holds the text
/// height already scaled and the band as the file states it, and comparing the two overstates
/// <c>nDif</c> by <c>height x (1 - zoom)</c>.
/// </para>
/// <para>
/// That is invisible without a print scale and it cost whole bands with one.
/// <c>sheets/done-014/xls/TICAPCapability_Final.xls</c> prints at 35 %: its band is 30.16 pt
/// stated and its text 7.82 pt drawn, the mixed arithmetic put the pen 6.17 pt below the band's
/// top, and the ink then began below the clip rectangle — so the area missed the window
/// altogether and <strong>six pages lost their header and their footer</strong>, seventeen words
/// each, on a document that had been word-exact against the reference on every one of them.
/// Three more corpus documents lost 8, 18 and 195 words the same way.
/// </para>
/// <para>
/// <c>sheet-band-scale-pinned.fods</c> is authored to the smallest case that shows it: letter
/// portrait at <c>style:scale-to="35%"</c>, a 0.42 in band with no gap, and one short line in
/// each of the header and the footer, so both have room to spare. ODF is the format that can
/// state it — <c>XlsxPrintSetup</c> flags every SpreadsheetML band dynamic, which takes the other
/// arm. Both installed references put the header's ink box at <strong>39.340</strong> and the
/// footer's at <strong>748.702</strong>, and the body cell between them at 46.834.
/// </para>
/// </remarks>
public sealed class SheetBandScalePinnedTests
{
    private static IReadOnlyList<DrawnGlyphRun> Runs()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-band-scale-pinned.fods"));

        RecordingDrawingSink sink = new();
        foreach (SheetPage page in ((SpreadsheetPages)document.Layout()).Pages) page.Draw(sink);
        return [.. sink.Pages.SelectMany(page => page.Runs)];
    }

    [Fact]
    public void AScaledSheetsBandIsDrawnAtAll()
    {
        // The regression this file exists for. Both references draw both bands; we drew neither.
        IEnumerable<string> drawn = Runs().Select(run => run.Text);
        drawn.ShouldContain("PINNEDHEADER");
        drawn.ShouldContain("PINNEDFOOTER");
    }

    [Fact]
    public void AScaledSheetsHeaderIsCentredInTheBandAtThePrintScale()
    {
        // The references' ink box tops out at 39.340 on a band whose top edge is the 36 pt margin,
        // so the pen has come down by half of `(band - text) x zoom` and not by half of
        // `band - text x zoom`. `Origin` is the baseline, one ascent below that box.
        Runs().Single(run => run.Text == "PINNEDHEADER").Origin.Y.Points
            .ShouldBeInRange(41.0, 44.5);
    }

    [Fact]
    public void AScaledSheetsFooterStillSitsAboveItsFooterMargin()
    {
        // 748.702 in both references, against a page 792 pt tall and a 36 pt bottom margin.
        Runs().Single(run => run.Text == "PINNEDFOOTER").Origin.Y.Points
            .ShouldBeInRange(750.0, 753.5);
    }

    [Fact]
    public void TheBodyIsWhereTheBandLeavesIt()
    {
        // The control: a band arithmetic that moved the text without moving the body would put
        // the two in different spaces again, and this is the figure the reference agrees on
        // (46.834) whichever way the band is measured.
        Runs().Single(run => run.Text == "BODYCELL").Origin.Y.Points
            .ShouldBeInRange(48.0, 51.5);
    }
}
