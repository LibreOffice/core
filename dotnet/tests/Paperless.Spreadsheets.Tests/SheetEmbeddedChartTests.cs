using Paperless.Core.Charts;
using Paperless.Core.Documents;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// A chart on an <c>.xls</c> worksheet of its own still prints, and the chart it prints is the
/// one its substream describes.
/// </summary>
/// <remarks>
/// <para>
/// The fixture pair is generated: <c>sheet-chart-only-page.fods</c> is the committed
/// <c>chart-bar-sheet.fods</c> with its chart moved onto a second sheet, and
/// <c>sheet-chart-only-page.xls</c> is LibreOffice's own BIFF8 export of it. **The move is what
/// makes the fixture discriminating.** A chart beside its data is kept alive by the data's print
/// area, so a reader that drops chart objects entirely still prints that page and the defect is
/// invisible; a chart on a sheet with no cells at all is the only content there, and dropping it
/// takes the whole sheet with it.
/// </para>
/// <para>
/// LibreOffice prints this workbook as <strong>two</strong> pages, one per sheet, which is the
/// number asserted below.
/// </para>
/// <para>
/// What is <em>not</em> asserted is the chart's scale. An <c>.xls</c> chart states its series as
/// cell ranges on another sheet and nothing resolves them yet, so the value axis is drawn from an
/// empty series and reads 0 to 12 where the reference reads 0 to 180. That is a real and separate
/// gap, recorded in the module's TODO; asserting the wrong scale here would freeze it.
/// </para>
/// </remarks>
public sealed class SheetEmbeddedChartTests
{
    private static SpreadsheetPages Pages()
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require("sheet-chart-only-page.xls"));

        return (SpreadsheetPages)document.Layout();
    }

    [Fact]
    public void ASheetHoldingNothingButAChartStillPrintsIt()
    {
        SpreadsheetPages pages = Pages();

        pages.Sheets.Count.ShouldBe(2);
        pages.Sheets[1].Name.ShouldBe("Graph");

        // Without the chart the second sheet has no cells, no drawings and so no printed range at
        // all — not one blank page but none, because `SheetEmptyPages`'s "a sheet always prints
        // one" floor has no placement to keep.
        pages.Pages.Count(page => page.Sheet.Name == "Graph")
            .ShouldBe(1, "LibreOffice prints one page for the chart sheet");
        pages.Pages.Count.ShouldBe(2, "LibreOffice prints two pages for this workbook");
    }

    [Fact]
    public void TheChartSubstreamReachesTheDrawingThatOpenedIt()
    {
        SheetDrawings drawings = Pages().Sheets[1].Drawings;

        drawings.Items.Count.ShouldBe(1);
        SheetDrawing chart = drawings.Items[0];

        chart.IsChart.ShouldBeTrue();

        // The substream is read rather than stepped over, and the plot proves which substream:
        // the title is the chart's own and appears nowhere in the sheet's cells.
        ChartPlot plot = chart.Chart.ShouldNotBeNull();
        plot.Title.ShouldBe("Regional revenue");
    }

    [Fact]
    public void TheDataSheetIsUnaffectedAndKeepsItsCells()
    {
        // The control. A change that made every sheet print a chart page, or that consumed the
        // worksheet's own records while reading the chart's substream, would show here first.
        SheetLayout revenue = Pages().Sheets[0];

        revenue.Name.ShouldBe("Revenue");
        revenue.Drawings.IsEmpty.ShouldBeTrue("the chart is on the other sheet");
        revenue.CellAt(0, 1).ShouldNotBeNull().Text.ShouldBe("North");
        revenue.CellAt(4, 2).ShouldNotBeNull().Text.ShouldBe("121");
    }
}
