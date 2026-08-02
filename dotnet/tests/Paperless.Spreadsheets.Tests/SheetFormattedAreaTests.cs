using Paperless.Core.Documents;
using Paperless.Core.Graphics;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// A ruled but empty cell is inside the printed area.
/// </summary>
/// <remarks>
/// <para>
/// <c>ScTable::GetPrintArea</c> runs two passes over the same columns: the first finds the last
/// row and column holding data, and the second — headed <c>// Test attribute</c>
/// (<c>sc/source/core/data/table1.cxx:710</c>) — asks each column for its last visibly attributed
/// row. A workbook of blank forms has almost no data and a great deal of ruling, so without the
/// second pass its boxes are outside the block and never placed on a page at all.
/// </para>
/// <para>
/// The scan is bounded, and the bound is what makes it usable: below the last data row, attribute
/// runs are followed only while each run of visually equal rows is shorter than
/// <c>SC_VISATTR_STOP</c> = 84 (<c>ScAttrArray::GetLastVisibleAttr</c>,
/// <c>attarray.cxx:1922-1975</c>). Formatting reaches the end of a sheet far more often than data
/// does, so an unbounded scan would put the print area at the last row of every workbook that
/// states a column style.
/// </para>
/// </remarks>
public sealed class SheetFormattedAreaTests
{
    private static SheetFormatting Ruled(params (int Row, int Column)[] cells)
    {
        SheetFormatting formatting = new();
        int border = formatting.Intern(new SheetCellDecoration(
            null,
            new SheetCellBorders(
                SheetBorder.Line(Core.Units.Length.FromTwips(15), Colour.Black),
                SheetBorder.None, SheetBorder.None, SheetBorder.None)));

        foreach ((int row, int column) in cells) formatting.SetCell(row, column, border);
        return formatting;
    }

    [Fact]
    public void ARuledEmptyCellExtendsThePrintedArea()
    {
        SheetRange used = new(0, 0, 8, 0);
        SheetRange extended = SheetDecorationArea.Extend(used, Ruled((13, 4), (13, 5)));

        extended.LastRow.ShouldBe(13, "the ruled row is inside the block Calc prints");
        extended.LastColumn.ShouldBe(8, "it adds no column: column 5 is already inside");
    }

    /// <summary>
    /// A ruled cell right of the data widens the block sideways as well.
    /// </summary>
    /// <remarks>
    /// Calc's second pass sets <c>nMaxX</c> from the column index alone, with no row guard on it.
    /// </remarks>
    [Fact]
    public void ARuledCellRightOfTheDataWidensTheBlock()
    {
        SheetRange extended = SheetDecorationArea.Extend(new SheetRange(0, 0, 2, 0), Ruled((1, 7)));
        extended.LastColumn.ShouldBe(7);
    }

    /// <summary>
    /// A gap of more than eighty-four rows ends the scan, and everything past it is ignored.
    /// </summary>
    /// <remarks>
    /// The two cells straddle the limit deliberately: the first is inside it and must be taken,
    /// the second is one row past and must not, so a renderer cannot satisfy the test by choosing
    /// a side.
    /// </remarks>
    [Fact]
    public void FormattingFarBelowTheDataIsIgnored()
    {
        SheetRange used = new(0, 0, 3, 10);

        SheetDecorationArea.Extend(used, Ruled((10 + SheetDecorationArea.VisibleAttributeStop, 1)))
            .LastRow.ShouldBe(10 + SheetDecorationArea.VisibleAttributeStop, "inside the run limit");

        SheetDecorationArea.Extend(used, Ruled((11 + SheetDecorationArea.VisibleAttributeStop, 1)))
            .LastRow.ShouldBe(10, "one row past it, so the scan stops before it");
    }

    /// <summary>
    /// A cell that states a style painting nothing extends nothing.
    /// </summary>
    /// <remarks>
    /// <c>ScPatternAttr::CalcVisible</c> asks for a non-transparent background, a border edge, a
    /// diagonal or a shadow and nothing else (<c>patattr.cxx:1584</c>), so the great majority of
    /// styled cells — a font, an alignment, a number format — are invisible to this pass.
    /// </remarks>
    [Fact]
    public void APlainStyleExtendsNothing()
    {
        SheetFormatting formatting = new();
        formatting.SetCell(50, 3, formatting.Intern(SheetCellDecoration.None));

        SheetDecorationArea.Extend(new SheetRange(0, 0, 3, 0), formatting).LastRow.ShouldBe(0);
    }

    /// <summary>
    /// The whole of it, on a document: the ruled row reaches the page and is drawn.
    /// </summary>
    /// <remarks>
    /// <c>sheet-decor-xlsx.xlsx</c> would not show it — its ruling is on cells that also hold
    /// values — so the assertion is made on the corpus workbook the defect was found on instead,
    /// through the page's own drawing commands.
    /// </remarks>
    [Fact]
    public void ARuledRowBelowTheDataIsPlacedOnAPage()
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require("sheet-ruled-form.fods"));

        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();
        pages.Sheets[0].PrintedRange.LastRow.ShouldBe(13, "the ruled row is row 14");

        RecordingDrawingSink sink = new();
        pages.Pages[0].Draw(sink);

        sink.Pages[0].Strokes.Count.ShouldBeGreaterThan(0, "the ruling reaches the page");
    }
}
