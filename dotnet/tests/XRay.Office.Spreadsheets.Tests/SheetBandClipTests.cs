using XRay.Office.Core.Documents;
using XRay.Office.Core.Graphics;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A page paints only the borders its own column band states, never a neighbour's.
/// </summary>
/// <remarks>
/// <para>
/// Inside a band the two cells meeting at an edge argue for it and the heavier wins
/// (<c>svx::frame::Array::GetCellStyleLeft</c>, <c>svx/source/dialog/framelinkarray.cxx:797</c>).
/// At the two ends of the band that rule does not apply: the array's clip range gets the first
/// column's own left style and, one past the last column, the last column's own right style
/// (<c>:786-793</c>). <c>ScOutputData::DrawFrame</c> sets that clip range to the printed band
/// whenever it draws in page mode (<c>sc/source/ui/view/output.cxx:1567</c>), and
/// <c>ScPrintFunc::PrintPage</c> is a page-mode caller
/// (<c>sc/source/ui/view/printfun.cxx:1612-1614</c>) — so it is in force for every printed page,
/// not only for page-break preview.
/// </para>
/// <para>
/// Without it a page paints borders belonging to a column it is not printing. Measured on page 2
/// of <c>7-memento-2015-transports-aeriens-b.xls</c>, where the off-page column 2's
/// <c>#003366</c> left edge took the trailing vertical for 32 rows.
/// </para>
/// <para>
/// The fixture's expectations are read off LibreOffice 26.2.4.2's own PDF of it: page 1 carries
/// one red stroke at x 340.10 spanning row 1, page 2 one blue stroke at x 56.66 spanning row 2,
/// and page 3 nothing at all.
/// </para>
/// </remarks>
public sealed class SheetBandClipTests
{
    private static IReadOnlyList<DrawnPage> Draw()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-band-clip.fods"));

        RecordingDrawingSink sink = new();
        foreach (SheetPage page in ((SpreadsheetPages)document.Layout()).Pages) page.Draw(sink);
        return sink.Pages;
    }

    private static IReadOnlyList<Colour> BorderColours(DrawnPage page)
        => [.. page.StrokedPaths
            .Where(stroke => stroke.Bounds.Width < stroke.Bounds.Height)
            .Select(stroke => stroke.Stroke.Paint)
            .OfType<SolidPaint>()
            .Select(paint => paint.Colour)];

    [Fact]
    public void TheLastColumnOfABandStatesItsOwnTrailingEdge()
    {
        // Page 1 is column A. A1 states the red right border and A2 states none; B2's blue left
        // border is on the same sheet edge but in a column this page does not print.
        BorderColours(Draw()[0]).ShouldBe([Colour.FromRgb(0xFF0000)]);
    }

    [Fact]
    public void TheFirstColumnOfABandStatesItsOwnLeadingEdge()
    {
        // Page 2 is column B, and the mirror image: B2's blue is drawn, A1's red is not.
        BorderColours(Draw()[1]).ShouldBe([Colour.FromRgb(0x0000FF)]);
    }

    [Fact]
    public void APageWhoseColumnStatesNothingDrawsNothing()
    {
        // Page 3 is column C, which touches neither border. The control: without it, a rule that
        // simply dropped every band-edge border would pass the two cases above.
        Draw()[2].StrokedPaths.ShouldBeEmpty();
    }
}
