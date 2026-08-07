using Paperless.Core.Documents;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// Tests that the page count the fit-to-page zoom search bisects on excludes empty row bands.
/// </summary>
/// <remarks>
/// <para>
/// <c>PrintPageRanges::calculate</c> increments <c>m_nPagesY</c> only for a band
/// <c>IsPrintEmpty</c> is false across — <c>sc/source/ui/view/printfun.cxx:3176</c> for each band
/// the break iterator ends, and <c>:3220</c> for the last one. So the count is not the number of
/// bands the geometry produces, and every predicate in <c>ScPrintFunc::CalcZoom</c> compares the
/// smaller number.
/// </para>
/// <para>
/// It shows up on the tdf#103516 nudge (<c>printfun.cxx:2955-2968</c>), which drops the scale by
/// two per cent when doing so removes a vertical page and reverts when it does not. A print area
/// reaching one row past the data gives a trailing band that Calc never counts: the nudge sees the
/// count fall and keeps the smaller scale, where counting the band makes the count look unchanged
/// and abandons it.
/// </para>
/// <para>
/// <c>sheet-fit-empty-band.xlsx</c> is built to that shape — A4 portrait, quarter-inch margins,
/// fit-to-width with the height unconstrained, 54 rows pinned at 15 pt and a print area of
/// <c>A1:B55</c>. The printable height is 16118 twips at zoom 100 and 16447 at 98, and the rows
/// sum to 16200, so the 54 rows fit only after the nudge and row 55 never fits either way.
/// LibreOffice 24.2.7.2 prints it on <strong>one</strong> page; counting the empty band gives two.
/// </para>
/// <para>
/// Verified by reintroducing the defect rather than by watching this pass: restoring the previous
/// call — <c>Paginate(setup, grid, range)</c> with no emptiness test threaded in, which is exactly
/// what the code did before — renders this document on two pages.
/// </para>
/// </remarks>
public class SheetFitEmptyBandTests
{
    private static IPaginatedDocument Open()
        => (IPaginatedDocument)new SpreadsheetReader().Read(
            DocumentSource.FromFile(Corpus.Require("sheet-fit-empty-band.xlsx")));

    [Fact]
    public void ATrailingEmptyRowBandDoesNotCountAgainstTheFitToWidthNudge()
    {
        using IPaginatedDocument document = Open();

        document.Layout().Count.ShouldBe(1);
    }

    [Fact]
    public void TheNudgedScaleIsTheOneTheSheetPrintsAt()
    {
        using IPaginatedDocument document = Open();
        IPageSequence pages = document.Layout();

        // The point of the assertion is that the scale moved off the fitting 100 at all: the
        // nudge is the only thing that lowers it, and it lowers it by exactly two per cent.
        SheetPage page = (SheetPage)pages[0];
        page.Placement.ZoomPercentage.ShouldBe(98);
    }
}
