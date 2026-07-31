using Paperless.Core.Documents;
using Paperless.Core.Units;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// Tests the print setup each reader recovers, and the pages it derives from it.
/// </summary>
/// <remarks>
/// <para>
/// The page count is the assertion that matters most and it is asserted first, because it is
/// the single number that catches a wrong scale, a wrong print area or a wrong break. Every
/// count here was measured: <c>soffice --convert-to pdf</c> on the same corpus file, read back
/// with <c>pdfinfo</c>. The comparison against a live LibreOffice lives in
/// <c>Paperless.Fidelity.Tests</c>; these are the same numbers frozen, so that the suite still
/// covers pagination on a machine with no LibreOffice on it.
/// </para>
/// <para>
/// The geometry assertions below the counts are what make a failure diagnosable. A wrong page
/// count says only that something is wrong; a margin read in the wrong unit, a header band that
/// swallowed the top margin or a column width converted with the wrong digit says which.
/// </para>
/// </remarks>
public class SheetPaginationTests
{
    private static IPaginatedDocument Open(string name)
        => (IPaginatedDocument)new SpreadsheetReader().Read(
            DocumentSource.FromFile(Corpus.Require(name)));

    [Theory]
    [InlineData("sheet-features.ods", 2)]
    [InlineData("sheet-ooxml-features.xlsx", 3)]
    [InlineData("xls-features.xls", 4)]
    public void AWorkbookPaginatesToTheNumberOfPagesLibreOfficePrints(string name, int expected)
    {
        using IPaginatedDocument document = Open(name);
        IPageSequence pages = document.Layout();

        pages.Count.ShouldBe(expected);
    }

    [Theory]
    [InlineData("sheet-features.ods")]
    [InlineData("sheet-ooxml-features.xlsx")]
    [InlineData("xls-features.xls")]
    public void EveryPageIsA4Portrait(string name)
    {
        using IPaginatedDocument document = Open(name);
        IPageSequence pages = document.Layout();

        for (int at = 0; at < pages.Count; at++)
        {
            // 595.3 x 841.9 points, which is what pdfinfo reports for all three references.
            pages[at].Size.Width.Points.ShouldBe(595.3, 0.5);
            pages[at].Size.Height.Points.ShouldBe(841.9, 0.5);
        }
    }

    [Fact]
    public void AHiddenSheetIsExtractedButNeverPrinted()
    {
        // The one place layout and extraction deliberately disagree: extraction keeps a hidden
        // sheet and flags it, and Calc's printout has no page for it at all.
        using IPaginatedDocument document = Open("sheet-features.ods");
        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();

        pages.Sheets.ShouldContain(sheet => sheet.Name == "Hidden" && sheet.IsHidden);
        pages.Pages.ShouldNotContain(page => page.Sheet.Name == "Hidden");
    }

    [Fact]
    public void TheOdfPageStyleFallsBackToCalcsOwnDefaults()
    {
        // sheet-features.ods declares no page size and no margin anywhere: its style:page-layout
        // holds a writing mode and two bands, nothing else. LibreOffice still renders it on A4
        // with two-centimetre margins, because those come from the page style Calc creates
        // before any file is read.
        using IPaginatedDocument document = Open("sheet-features.ods");
        SheetLayout sales = ((SpreadsheetPages)document.Layout()).Sheets[0];

        sales.Setup.LeftMargin.Twips.ShouldBe(1134);
        sales.Setup.TopMargin.Twips.ShouldBe(1134);
        sales.Setup.PageSize.Width.Twips.ShouldBe(11906);

        // The header band is the 0.75 cm the page layout declares, and its 0.25 cm gap to the
        // first row is *inside* that rather than added to it: Calc's aHdr.nHeight is
        // ATTR_PAGE_SIZE's height and aHdr.nDistance is subtracted from it to get the rectangle
        // the header text is laid out in (lcl_FillHFParam, printfun.cxx:664, and PrintHF,
        // printfun.cxx:1808). That is what puts the first row of cells at the 78 points down
        // the page this test always claimed: 1134 + 425 twips is 77.95 pt, where 1134 + 567
        // would be 85.05. Measured on sheet-decor-ods.ods, LibreOffice clips its header text to
        // a rectangle 14.099 pt tall and starts the first row 21.11 pt below the top margin.
        sales.Setup.HeaderHeight.Twips.ShouldBe(425);
        sales.Setup.HeaderGap.Twips.ShouldBe(142);
    }

    [Fact]
    public void AnOoxmlTopMarginSurvivesTheHeaderBandConversion()
    {
        // SpreadsheetML measures its top margin to the first row and its header margin to the
        // header; Calc measures the top margin to the header and holds the band separately. The
        // invariant the conversion has to preserve is that the two still add up.
        using IPaginatedDocument document = Open("sheet-ooxml-features.xlsx");
        SheetLayout sales = ((SpreadsheetPages)document.Layout()).Sheets[0];

        // top="1.05277777777778" header="0.7875" in the file.
        (sales.Setup.TopMargin + sales.Setup.HeaderHeight).Inches.ShouldBe(1.0528, 0.001);
        sales.Setup.TopMargin.Inches.ShouldBe(0.7875, 0.001);
        sales.Setup.LeftMargin.Inches.ShouldBe(0.7875, 0.001);
    }

    [Fact]
    public void AnOoxmlColumnWidthIsDigitsOfTheDefaultFont()
    {
        // width="20.76" in the file, and LibreOffice's rendering puts the columns 115.2 points
        // apart — 2304 twips, which is 20.76 digits of 111 twips each.
        using IPaginatedDocument document = Open("sheet-ooxml-features.xlsx");
        SheetLayout sales = ((SpreadsheetPages)document.Layout()).Sheets[0];

        sales.Grid.Columns.SizeAt(0).Twips.ShouldBe(2304);
        sales.Grid.Rows.SizeAt(0).Points.ShouldBe(12.75, 0.01);
    }

    [Fact]
    public void AnOdfColumnWidthComesFromItsColumnStyle()
    {
        // style:column-width="0.9839in" — 1417 twips, and the reference PDF puts "Region" and
        // "Units" 70.84 points apart.
        using IPaginatedDocument document = Open("sheet-features.ods");
        SheetLayout sales = ((SpreadsheetPages)document.Layout()).Sheets[0];

        sales.Grid.Columns.SizeAt(0).Twips.ShouldBe(1417);
        sales.Grid.Rows.SizeAt(0).Twips.ShouldBe(255);
    }

    [Fact]
    public void APageKnowsWhichCellsAreOnIt()
    {
        using IPaginatedDocument document = Open("sheet-features.ods");
        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();

        // The whole of the first sheet fits one page, so the page's block is the used range —
        // which starts at A1 whether or not A1 holds anything, because Calc's own search for the
        // print area fixes the start at the sheet's origin and only looks for the end.
        SheetPagePlacement first = pages.Pages[0].Placement;
        first.Cells.FirstColumn.ShouldBe(0);
        first.Cells.FirstRow.ShouldBe(0);
        first.Cells.LastColumn.ShouldBe(3);
        first.ZoomPercentage.ShouldBe(100);
    }

    [Theory]
    [InlineData("sheet-print-ods.ods")]
    [InlineData("sheet-print-xlsx.xlsx")]
    [InlineData("sheet-print-xls.xls")]
    public void ThePrintSetupWorkbookPaginatesTheSameWayFromAllThreeFormats(string name)
    {
        // Fourteen pages, and the same fourteen from all three: six from the sheet with repeated
        // headers, one from the sheet limited to a print range, one from the sheet fitted to a
        // single page wide, two from the sheet with a manual break, and four from the sheet
        // printed across before down. LibreOffice prints fourteen from each of the three too.
        using IPaginatedDocument document = Open(name);
        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();

        pages.Count.ShouldBe(14);

        int[] perSheet = [.. pages.Sheets
            .Where(sheet => SpreadsheetPages.IsPrinted(sheet, true))
            .Select(sheet => pages.Pages.Count(page => page.Sheet == sheet))];

        perSheet.ShouldBe([6, 1, 1, 2, 4]);
    }

    [Theory]
    [InlineData("sheet-print-ods.ods")]
    [InlineData("sheet-print-xlsx.xlsx")]
    [InlineData("sheet-print-xls.xls")]
    public void ARepeatedHeaderBandIsOnEveryPageButTheOneItStartsOn(string name)
    {
        using IPaginatedDocument document = Open(name);
        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();
        List<SheetPage> wide = [.. pages.Pages.Where(page => page.Sheet.Name == "Wide")];

        // Column A and row 1 are the repeated bands, so the first page of each column band
        // holds them as ordinary content and every page below repeats them.
        wide[0].Placement.RepeatRows.ShouldBeNull();
        wide[0].Placement.RepeatColumns.ShouldBeNull();
        wide[1].Placement.RepeatRows.ShouldNotBeNull();
        wide[2].Placement.RepeatColumns.ShouldNotBeNull();

        // The page after the first column band starts at column one, not at column zero: the
        // repeated column is printed but is not part of the band's own range.
        wide[2].Placement.Cells.FirstColumn.ShouldBeGreaterThan(0);
    }

    [Theory]
    [InlineData("sheet-print-ods.ods")]
    [InlineData("sheet-print-xlsx.xlsx")]
    [InlineData("sheet-print-xls.xls")]
    public void APrintRangeIsThePageRatherThanTheUsedArea(string name)
    {
        // The Area sheet holds eight columns and forty rows and declares B2:D10 as its print
        // range. One page, and it starts at B2 rather than at A1.
        using IPaginatedDocument document = Open(name);
        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();
        SheetPage area = pages.Pages.Single(page => page.Sheet.Name == "Area");

        area.Placement.Cells.ShouldBe(new SheetRange(1, 1, 3, 9));
    }

    [Theory]
    [InlineData("sheet-print-ods.ods")]
    [InlineData("sheet-print-xlsx.xlsx")]
    [InlineData("sheet-print-xls.xls")]
    public void FittingToOnePageWideShrinksUntilItFits(string name)
    {
        // Ten one-inch columns need two pages across on A4 at full size; the Fit sheet asks for
        // one, so the search has to find a scale under a hundred per cent that reaches it.
        using IPaginatedDocument document = Open(name);
        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();
        SheetPage fit = pages.Pages.Single(page => page.Sheet.Name == "Fit");

        fit.Placement.ZoomPercentage.ShouldBeLessThan(100);
        fit.Placement.ZoomPercentage.ShouldBeGreaterThan(SheetPagination.MinimumZoom);
        fit.Placement.Cells.LastColumn.ShouldBe(9);
    }

    [Theory]
    [InlineData("sheet-print-ods.ods")]
    [InlineData("sheet-print-xlsx.xlsx")]
    [InlineData("sheet-print-xls.xls")]
    public void AManualBreakSplitsAPageThatWouldOtherwiseFit(string name)
    {
        // Thirty rows fit one A4 page comfortably. The Breaks sheet forces a break before row
        // eleven, so the second page starts at row index ten.
        using IPaginatedDocument document = Open(name);
        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();
        List<SheetPage> breaks = [.. pages.Pages.Where(page => page.Sheet.Name == "Breaks")];

        breaks.Count.ShouldBe(2);
        breaks[0].Placement.Cells.LastRow.ShouldBe(9);
        breaks[1].Placement.Cells.FirstRow.ShouldBe(10);
    }

    [Theory]
    [InlineData("sheet-print-ods.ods")]
    [InlineData("sheet-print-xlsx.xlsx")]
    [InlineData("sheet-print-xls.xls")]
    public void ThePageOrderDecidesWhichBlockIsPageTwo(string name)
    {
        // The Across sheet prints left to right rather than top down, so its second page is the
        // block to the right of the first rather than the one below it. Getting this backwards
        // reorders every page of a sheet that spills both ways without changing how many there
        // are, which is why the count alone does not catch it.
        using IPaginatedDocument document = Open(name);
        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();
        List<SheetPage> across = [.. pages.Pages.Where(page => page.Sheet.Name == "Across")];

        across.Count.ShouldBe(4);
        across[0].Sheet.Setup.PageOrder.ShouldBe(PagePrintOrder.AcrossThenDown);
        across[1].Placement.Cells.FirstColumn.ShouldBeGreaterThan(0);
        across[1].Placement.Cells.FirstRow.ShouldBe(across[0].Placement.Cells.FirstRow);
        across[2].Placement.Cells.FirstColumn.ShouldBe(0);
        across[2].Placement.Cells.FirstRow.ShouldBeGreaterThan(0);
    }

    [Fact]
    public void APageDrawsTheCellsItHolds()
    {
        // The pages are the point of the work and the ink is not yet, but a page that places
        // cells and draws nothing is indistinguishable from one that placed none — so this
        // asserts that every cell of the block reaches the sink, in the order it is placed, and
        // that a repeated band is drawn above the block rather than after it.
        using IPaginatedDocument document = Open("sheet-print-ods.ods");
        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();
        SheetPage second = pages.Pages.First(
            page => page.Sheet.Name == "Wide" && page.Placement.RepeatRows is not null);

        RecordingDrawingSink sink = new();
        second.Draw(sink);

        sink.Pages.Count.ShouldBe(1);
        sink.UnclosedPages.ShouldBe(0);

        List<string> drawn = [.. sink.Pages[0].Runs.Select(run => run.Text)];
        drawn.ShouldNotBeEmpty();

        // The repeated row is drawn first and above everything else on the page.
        drawn[0].ShouldStartWith("C");
        sink.Pages[0].Runs[0].Origin.Y.ShouldBeLessThan(sink.Pages[0].Runs[^1].Origin.Y);

        // And the block's own first row follows it.
        drawn.ShouldContain($"R{second.Placement.Cells.FirstRow:00}C00");
    }

    [Fact]
    public void ABiffTopMarginSurvivesTheHeaderBandConversionToo()
    {
        // The same conversion as the OOXML one, out of TOPMARGIN and SETUP's header margin. It
        // is asserted separately because the page count cannot catch it: getting the header
        // wrong moves the first row and the band by the same amount in opposite directions, so
        // the printable height — and therefore the number of pages — is unchanged.
        using IPaginatedDocument document = Open("xls-features.xls");
        SheetLayout sales = ((SpreadsheetPages)document.Layout()).Sheets[0];

        sales.Setup.HeaderText.ShouldNotBeNullOrEmpty();
        sales.Setup.TopMargin.Inches.ShouldBe(0.7875, 0.001);

        // The sum is 1.0828 in and not the file's own 1.0528, because a BIFF header band has a
        // floor the OOXML one does not: the BIFF filter never puts an ATTR_PAGE_SIZE on the
        // header's item set, only a distance (XclImpPageSettings::Finalize,
        // sc/source/filter/excel/xipage.cxx:310-331), so nManHeight stays at the 0.75 cm a
        // fresh page style carries (ScStyleSheet::GetItemSet, stlsheet.cxx:184) and
        // UpdateHFHeight will not go below it. Measured on this very file: LibreOffice puts the
        // first cell's baseline 86.63 pt down the page, which is a band of 20.25 pt — nearer
        // the 21.26 the floor gives than the 19.1 the margins alone would.
        (sales.Setup.TopMargin + sales.Setup.HeaderHeight).Inches.ShouldBe(1.0828, 0.001);

        // A COLINFO of 2953 256ths of a character at 111 twips a digit, which is 1280.4 — and
        // 1279, not 1280, because LibreOffice's conversion takes half a twip off before
        // truncating rather than rounding (XclTools::GetScColumnWidth). Measured: its rendering
        // puts "Region" at 58.677 points and "Units" at 122.627, which is 63.95 points apart.
        sales.Grid.Columns.SizeAt(0).Twips.ShouldBe(1279);
    }
}
