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

    /// <summary>
    /// Thirty equally-formatted columns behind the data end the block before them.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The sideways twin of the run limit, and the two straddle their constants the same way:
    /// twenty-nine identical ruled columns are all kept and thirty are all dropped, so a
    /// renderer cannot satisfy this by choosing a side. <c>SC_COLUMNS_STOP</c> = 30,
    /// <c>ScTable::GetPrintArea</c> (<c>table1.cxx:737-757</c>).
    /// </para>
    /// <para>
    /// The row the columns reached is kept either way, because Calc's cut-back touches
    /// <c>nMaxX</c> alone and <c>nMaxY</c> was already set by the pass that found them.
    /// </para>
    /// </remarks>
    [Fact]
    public void ThirtyEquallyRuledColumnsBehindTheDataAreDropped()
    {
        SheetRange used = new(0, 0, 3, 10);

        SheetRange kept = SheetDecorationArea.Extend(used, RuledBand(4, 29));
        kept.LastColumn.ShouldBe(32, "twenty-nine is one short of the limit");
        kept.LastRow.ShouldBe(25);

        SheetRange cut = SheetDecorationArea.Extend(used, RuledBand(4, 30));
        cut.LastColumn.ShouldBe(3, "thirty ends the block before the whole band");
        cut.LastRow.ShouldBe(25, "the cut is sideways only");
    }

    /// <summary>A band of identically ruled, entirely empty columns.</summary>
    private static SheetFormatting RuledBand(int first, int count)
    {
        (int Row, int Column)[] cells =
            [.. Enumerable.Range(first, count).SelectMany(
                c => Enumerable.Range(20, 6).Select(r => (r, c)))];

        return Ruled(cells);
    }

    /// <summary>
    /// A filled cell keyed to a column, for the per-column scan below.
    /// </summary>
    private static (SheetFormatting Formatting, Dictionary<int, int> LastData) Filled(
        (int Column, int First, int Last, bool HeadDiffers)[] bands,
        params (int Column, int LastDataRow)[] data)
    {
        SheetFormatting formatting = new();
        int fill = formatting.Intern(new SheetCellDecoration(Colour.FromRgb(0xDCE6F1), default));
        int head = formatting.Intern(new SheetCellDecoration(
            Colour.FromRgb(0xDCE6F1),
            new SheetCellBorders(
                SheetBorder.None,
                SheetBorder.None,
                SheetBorder.None,
                SheetBorder.Line(Core.Units.Length.FromTwips(20), Colour.Black))));

        foreach ((int column, int first, int last, bool headDiffers) in bands)
        {
            for (int row = first; row <= last; row++)
                formatting.SetCell(row, column, headDiffers && row == first ? head : fill);
        }

        return (formatting, data.ToDictionary(x => x.Column, x => x.LastDataRow));
    }

    /// <summary>
    /// An empty column's scan starts at its own last data row, which Calc reads as row zero.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>ScColumn::GetLastVisibleAttr</c> passes that column's own <c>GetLastDataPos()</c>, "0 if
    /// none" (<c>sc/inc/column.hxx:892-897</c>). Column 4 here is filled on rows 1 to 100 and holds
    /// nothing, and everything Calc uses from it is its <em>first</em> row: the 99 rows below are
    /// one run past <c>SC_VISATTR_STOP</c> and end the scan. Starting the scan at the sheet's last
    /// data row instead puts that row above the start and loses the column outright.
    /// </para>
    /// <para>
    /// Column 2 is the other half. Filled on rows 1 to 84, it is inside the limit only because
    /// <c>ScAttrArray::GetLastVisibleAttr</c> clamps a run's start to <c>nLastData + 1</c>
    /// (<c>attarray.cxx:1961</c>) and so measures 83 rather than 84 — which is what makes "0 if
    /// none" different from "no data at all", one row apart and worth one column of print area.
    /// </para>
    /// </remarks>
    [Fact]
    public void AnEmptyColumnIsScannedFromItsOwnLastDataRow()
    {
        (SheetFormatting formatting, Dictionary<int, int> lastData) = Filled(
            [(2, 0, 83, false), (4, 0, 99, true)],
            (0, 9), (1, 9));

        SheetRange extended = SheetDecorationArea.Extend(new SheetRange(0, 0, 1, 9), formatting, lastData);

        extended.LastColumn.ShouldBe(4, "column 4's first row is inside its own scan");
        extended.LastRow.ShouldBe(83, "column 2's run measures 83 rows, one inside the limit");
    }

    /// <summary>
    /// The same sheet as a document: the band Calc keeps is drawn, and it was not before.
    /// </summary>
    /// <remarks>
    /// LibreOffice's own rendering of <c>sheet-empty-column-band.fods</c> confirms the column
    /// half — it paints column E's fill to the right edge of the block, 481 pt into a 596 pt page,
    /// where columns A to D end at 397. The row half is not visible in a PDF from either side,
    /// because the page rows 58 to 84 would add holds no data and Calc suppresses it.
    /// </remarks>
    [Fact]
    public void AFilledColumnWithNoDataIsInsideThePrintedArea()
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require("sheet-empty-column-band.fods"));

        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();
        SheetRange printed = pages.Sheets[0].PrintedRange;

        printed.LastColumn.ShouldBe(4, "column E is filled on every row and holds nothing");
        printed.LastRow.ShouldBe(83, "column C's fill stops one row inside the run limit");
    }
}
