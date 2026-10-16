using System.Diagnostics;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Graphics;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A repeat count is stored as the rectangle it describes, not as the cells it covers.
/// </summary>
/// <remarks>
/// <para>
/// Calc's own ODF export pads every sheet out to its full extent with
/// <c>table:number-rows-repeated</c> and <c>table:number-columns-repeated</c>, and the padding
/// names a styled cell rather than the default one — a 34 KB corpus workbook ends nine of its
/// sheets with <c>number-rows-repeated="1048567"</c> around
/// <c>number-columns-repeated="16381"</c>, which is seventeen billion cells per sheet.
/// </para>
/// <para>
/// The importer's answer is not to ignore a large repeat. <c>ScXMLTableRowContext</c> clamps the
/// row count to the sheet's own (<c>min(declared, GetSheetLimits().GetMaxRowCount())</c>,
/// <c>sc/source/filter/xml/xmlrowi.cxx</c>:73-82) and <c>ScXMLTableRowCellContext</c> clamps the
/// column count the same way (<c>xmlcelli.cxx</c>:186-195); a repeated <em>empty</em> cell then
/// does no per-cell work at all — the column loop breaks out after the first iteration,
/// <c>"if nothing else useful can happen in the loop, just exit early"</c>
/// (<c>xmlcelli.cxx</c>:1291-1297) — and the block's <em>style</em> is recorded as one
/// <c>ScRange</c> (<c>xmlcelli.cxx</c>:1368-1377), merged per style name into an
/// <c>ScRangeList</c> and applied over ranges at the end of the import. The store underneath is
/// <c>ScAttrArray</c>, a list of runs.
/// </para>
/// <para>
/// So the repeat is honoured up to the sheet's bounds and costs one entry, and the three things
/// that follow from that are what these tests assert.
/// </para>
/// </remarks>
public sealed class SheetRepeatBlockTests
{
    private static readonly Colour Padding = Colour.FromRgb(0xFFFFFF);
    private static readonly Colour Small = Colour.FromRgb(0xFFD966);

    private static (IPaginatedDocument Document, SheetLayout Sheet) Padded()
    {
        IPaginatedDocument document =
            (IPaginatedDocument)new SpreadsheetReader().Read(
                DocumentSource.FromFile(Corpus.Require("sheet-repeat-padding.fods")));

        return (document, ((SpreadsheetPages)document.Layout()).Sheets[0]);
    }

    /// <summary>
    /// A repeat that runs past the sheet's own last row and column reaches them and stops.
    /// </summary>
    /// <remarks>
    /// The document's last row element declares 1048576 repeats starting at row 4 and its padding
    /// cell 16384 starting at column 3, so both overrun by a few. Calc clamps rather than drops:
    /// the cell at the sheet's last row and last column is inside the block and takes its format.
    /// </remarks>
    [Fact]
    public void ARepeatPastTheSheetBoundsIsHonouredUpToThem()
    {
        (IPaginatedDocument document, SheetLayout sheet) = Padded();
        using (document)
        {
            sheet.Formatting.At(SheetAddress.MaxRow, SheetAddress.MaxColumn).Background
                 .ShouldBe(Padding, "the padding block reaches the sheet's last cell");
            sheet.Formatting.At(500_000, 8_000).Background
                 .ShouldBe(Padding, "and everything between");

            // The first three columns are outside the padding cell's own run, so they answer
            // whatever the sheet does rather than the block's fill.
            sheet.Formatting.At(500_000, 0).Background.ShouldNotBe(Padding);
        }
    }

    /// <summary>
    /// A repeated styled empty row does not paginate the sheet to its last row.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The padding's fill is white, and white is <em>visible</em> by Calc's own test — only
    /// <c>COL_TRANSPARENT</c> is not (<c>ScPatternAttr::CalcVisible</c>,
    /// <c>sc/source/core/data/patattr.cxx</c>:1584-1612) — so the attribute pass of
    /// <c>ScTable::GetPrintArea</c> can see it. What stops it is the run limit:
    /// <c>ScAttrArray::GetLastVisibleAttr</c> ignores the first run of
    /// <c>SC_VISATTR_STOP = 84</c> visually equal rows below the last data cell and everything
    /// under it (<c>attarray.cxx</c>:1920, 1967-1969).
    /// </para>
    /// <para>
    /// Measured on the document itself: LibreOffice 26.2.4.2 renders it as one page.
    /// </para>
    /// </remarks>
    [Fact]
    public void ARepeatedStyledEmptyRowDoesNotExtendThePrintedArea()
    {
        (IPaginatedDocument document, SheetLayout sheet) = Padded();
        using (document)
        {
            // The three-row amber block is a run of three and is taken; the million-row white one
            // is a run past the limit and ends the scan.
            sheet.PrintedRange.LastRow.ShouldBe(3);
            sheet.PrintedRange.LastColumn.ShouldBe(2);

            document.Layout().Count.ShouldBe(1);
        }
    }

    /// <summary>
    /// A repeat that legitimately describes a handful of cells still formats all of them.
    /// </summary>
    [Fact]
    public void ARepeatOfAHandfulOfCellsFormatsEveryOneOfThem()
    {
        (IPaginatedDocument document, SheetLayout sheet) = Padded();
        using (document)
        {
            // Rows 1 to 3, columns 0 and 1 — one row element repeated three times holding one
            // cell element repeated twice, so six cells from two attributes.
            for (int row = 1; row <= 3; row++)
            {
                for (int column = 0; column <= 1; column++)
                {
                    sheet.Formatting.At(row, column).Background
                         .ShouldBe(Small, $"({row}, {column}) is inside the repeated run");
                }
            }

            sheet.Formatting.At(1, 2).Background.ShouldNotBe(Small, "column 2 is outside the run");
            sheet.Formatting.At(4, 0).Background.ShouldNotBe(Small, "row 4 is below it");
        }
    }

    /// <summary>
    /// Reading the padded sheet costs neither the time nor the memory of its cell count.
    /// </summary>
    /// <remarks>
    /// The guard rather than the mechanism, and it is the reason the mechanism exists: the corpus
    /// document this was reduced from took 176 seconds to an out-of-memory abort while the same
    /// workbook as <c>.xlsx</c> rendered in one second. A generous bound, because a test host
    /// under load is slow — what it excludes is materialising the block, which is three orders of
    /// magnitude away from it.
    /// </remarks>
    [Fact]
    public void ThePaddedSheetIsReadWithoutMaterialisingIt()
    {
        Stopwatch clock = Stopwatch.StartNew();
        (IPaginatedDocument document, SheetLayout sheet) = Padded();
        using (document)
        {
            sheet.PrintedRange.LastRow.ShouldBe(3);
            clock.Elapsed.TotalSeconds.ShouldBeLessThan(20);
        }
    }

    /// <summary>
    /// A block resolves after a cell and before a row, and is clamped to the sheet.
    /// </summary>
    /// <remarks>
    /// The lookup order is Calc's own — a cell's own pattern, then the block it sits in, then the
    /// row's default, then the column's, then the sheet's — and it is asserted here rather than
    /// through a document because a document can only demonstrate one layer at a time.
    /// </remarks>
    [Fact]
    public void ABlockIsBeatenByACellAndBeatsARowAndIsClampedToTheSheet()
    {
        SheetCellFormats.Builder builder = new();
        int cell = builder.Intern(new SheetCellFormat { FontWeight = 700 });
        int block = builder.Intern(new SheetCellFormat { IsItalic = true });
        int row = builder.Intern(new SheetCellFormat { Underline = SheetUnderline.SingleLine });

        builder.SetRow(4, row);
        builder.SetCells(2, int.MaxValue, 1, int.MaxValue, block);
        builder.SetCell(4, 1, cell);

        SheetCellFormats formats = builder.Build();

        formats.At(4, 1).FontWeight.ShouldBe(700, "a cell of its own beats the block");
        formats.At(4, 2).IsItalic.ShouldBeTrue("the block beats the row's default");
        formats.At(4, 0).Underline.ShouldBe(SheetUnderline.SingleLine, "outside it, the row answers");
        formats.At(1, 2).IsItalic.ShouldBeFalse("above it, nothing does");

        // The two ends the repeat overran are clamped onto the sheet rather than dropped.
        formats.At(SheetAddress.MaxRow, SheetAddress.MaxColumn).IsItalic.ShouldBeTrue();

        // And unfolding it is bounded by what the caller asks for, not by what it covers.
        formats.CellsIn(new SheetRange(1, 2, 3, 4)).Count().ShouldBe(9);
    }
}
