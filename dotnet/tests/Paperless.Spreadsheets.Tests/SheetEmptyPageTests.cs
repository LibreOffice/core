using Paperless.Core.Extraction;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Spreadsheets.Layout;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// Which of a sheet's pages are printed, and which are dropped for having nothing on them.
/// </summary>
/// <remarks>
/// <para>
/// The printed block runs from A1 to the far corner of whatever the sheet reaches, so pagination
/// alone gives a sheet whose only content sits five hundred rows down ten sheets of paper of which
/// nine are white. Calc drops those (<c>bSkipEmpty</c>, on by default,
/// <c>sc/source/ui/view/printfun.cxx:3174</c>) and it is not cosmetic: on
/// <c>sc/qa/unit/data/xlsx/singlecontrol.xlsx</c> — a sheet with no cells at all and one form
/// control anchored at row 516 — it is the difference between ten pages and LibreOffice's one.
/// </para>
/// <para>
/// Assembled here rather than taken from the corpus, and that is the point: every corpus
/// spreadsheet is small and dense, so not one of them has a blank page to drop and not one of them
/// can see this rule either way. That is also why it is safe — the whole-corpus sweep stayed at 223
/// of 231 across it.
/// </para>
/// </remarks>
public sealed class SheetEmptyPageTests
{
    /// <summary>A sheet of the given cells on the standard grid, A4 portrait.</summary>
    private static SheetLayout Sheet(
        IEnumerable<(int Row, int Column, string Text)> cells,
        SheetDrawings? drawings = null)
    {
        ContentTable table = new();
        foreach ((int row, int column, string text) in cells.GroupBy(c => c.Row)
                                                            .SelectMany(g => g))
        {
            ContentTableRow? existing = table.Children.OfType<ContentTableRow>()
                                             .FirstOrDefault(r => r.Index == row);
            if (existing is null)
            {
                existing = new ContentTableRow { Index = row };
                table.Children.Add(existing);
            }

            ContentTableCell cell = new() { Row = row, Column = column, Value = text };
            ContentParagraph paragraph = new();
            paragraph.Children.Add(new ContentRun { Text = text });
            cell.Children.Add(paragraph);
            existing.Children.Add(cell);
        }

        return new SheetLayout
        {
            Name = "Sheet1",
            Cells = table,
            Drawings = drawings ?? SheetDrawings.Empty,
        };
    }

    private static int PageCount(SheetLayout sheet)
        => new SpreadsheetPages([sheet]).Pages.Count;

    /// <summary>
    /// A cell to the left keeps a page only when its text really reaches it.
    /// </summary>
    /// <remarks>
    /// The fourth of <c>IsPrintEmpty</c>'s tests, and the one it needs a font for: Calc re-runs
    /// <c>ExtendPrintArea</c> over the page's own rows from column zero and keeps the page when
    /// the extension reaches it (<c>documen9.cxx:486-500</c>). Answering it with "is there any
    /// cell at all to the left" instead keeps every column band of every row that has anything in
    /// column A, which on a wide sheet is most of the paper — measured on
    /// <c>RCO_VOR_Master_List_082824.xlsx</c> as 183 pages of which 103 were blank, against
    /// LibreOffice's 80 with none.
    /// </remarks>
    [Fact]
    public void AShortStringToTheLeftDoesNotKeepAPageAndALongOneDoes()
    {
        // Column BZ is far enough right to be a second column band on A4; the cell in column A
        // is what decides whether the band's first page is printed.
        SheetLayout brief = Sheet([(0, 0, "short"), (0, 77, "far right")]);
        SheetLayout verbose = Sheet(
            [(0, 0, new string('W', 4000)), (0, 77, "far right")]);

        // Both sheets span the same block, so any difference in page count is this rule alone.
        brief.PrintedRange.LastColumn.ShouldBe(77);

        PageCount(brief).ShouldBeLessThan(PageCount(verbose));
    }

    /// <summary>
    /// A border keeps a page and a background does not.
    /// </summary>
    /// <remarks>
    /// Calc asks <c>HasAttrib(..., HasAttrFlags::Lines)</c>, and that flag tests the four edges of
    /// <c>ATTR_BORDER</c> and nothing else (<c>attarray.cxx:1279-1284</c>) — "we want to print
    /// sheets with borders even if there is no cell content". A fill is not a border, and treating
    /// it as one keeps every page of a shaded region: <c>grants-2005.xls</c> came out at 1170
    /// pages, 949 of them blank, against LibreOffice's 220.
    /// </remarks>
    [Fact]
    public void ABorderKeepsAPageAndABackgroundDoesNot()
    {
        static SheetLayout Decorated(SheetCellDecoration decoration)
        {
            SheetFormatting formatting = new();
            formatting.SetCell(400, 0, formatting.Intern(decoration));

            SheetLayout sheet = Sheet([(0, 0, "top"), (600, 0, "bottom")]);
            return new SheetLayout
            {
                Name = sheet.Name,
                Cells = sheet.Cells,
                Formatting = formatting,
            };
        }

        SheetCellDecoration ruled = new(
            null,
            new SheetCellBorders(
                SheetBorder.Line(Length.FromTwips(20), Colour.Black), SheetBorder.None,
                SheetBorder.None, SheetBorder.None));

        SheetCellDecoration shaded = new(Colour.FromRgb(0xFFFF00), SheetCellBorders.None);

        // The ruled cell sits on a page between the two cells and keeps it; the shaded one does
        // not, so that sheet prints one page fewer.
        PageCount(Decorated(shaded)).ShouldBe(2);
        PageCount(Decorated(ruled)).ShouldBe(3);
    }

    [Fact]
    public void APageWithNothingOnItIsNotPrinted()
    {
        // One cell a long way down. Everything between A1 and it is blank paper.
        SheetLayout sheet = Sheet([(516, 1, "form control inside cell b517")]);

        sheet.PrintedRange.LastRow.ShouldBe(516);
        PageCount(sheet).ShouldBe(1);
    }

    [Fact]
    public void EveryPageThatHoldsACellIsKept()
    {
        // Rows 0 and 516: the first page and the last, and nothing in between.
        SheetLayout sheet = Sheet([(0, 0, "top"), (516, 0, "bottom")]);

        PageCount(sheet).ShouldBe(2);
    }

    /// <summary>
    /// A repeated title row does not keep a page that is otherwise blank.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The obvious reading is the wrong one: the band does print on the page, so counting it as
    /// content looks right and disables the whole rule for any sheet declaring
    /// <c>_xlnm.Print_Titles</c>. Calc asks <c>IsPrintEmpty</c> for the page's own block alone —
    /// <c>IsPrintEmpty(getStartColumn(), nPageStartRow, getEndColumn(), nRow-1, …)</c>
    /// (<c>sc/source/ui/view/printfun.cxx:3174</c>) — and <c>PrintPage</c> adds the repeated band
    /// afterwards, so it never enters the question.
    /// </para>
    /// <para>
    /// Measured on <c>fy20-may20-sep20.xlsx</c>, which repeats row 1 and whose column F reaches
    /// only row 76: LibreOffice prints two pages of the second column band and Paperless printed
    /// 103 blank ones, 233 pages against 96.
    /// </para>
    /// </remarks>
    [Fact]
    public void ARepeatedRowDoesNotKeepABlankPage()
    {
        SheetLayout sheet = Sheet([(0, 0, "title"), (1, 0, "top"), (516, 0, "bottom")]);
        SheetLayout titled = new()
        {
            Name = sheet.Name,
            Cells = sheet.Cells,
            Drawings = sheet.Drawings,
            Setup = sheet.Setup with { RepeatRows = new SheetRange(0, 0, 0, 0) },
        };

        // The same two pages as without the repeated band: the rows between 1 and 516 are blank,
        // and a title repeated onto them is not content of theirs.
        PageCount(titled).ShouldBe(2);
    }

    /// <summary>
    /// A drawing keeps every page its rectangle touches, cells or no cells.
    /// </summary>
    /// <remarks>
    /// <c>ScDocument::IsPrintEmpty</c> asks <c>HasAnyDraw</c> whether any object <em>overlaps</em>
    /// the page's rectangle, walking the whole drawing page rather than the objects anchored inside
    /// the block (<c>documen9.cxx:381-403, :483</c>). So a chart spanning four pages keeps all
    /// four, which is what <c>sc/qa/unit/data/xlsb/universal-content.xlsb</c> does with one hidden
    /// shape reaching column 12 and row 50.
    /// </remarks>
    [Fact]
    public void ADrawingKeepsThePagesItCovers()
    {
        SheetDrawing tall = new()
        {
            Anchor = SheetAnchorKind.TwoCell,
            From = new SheetCellPoint(0, Length.Zero, 0, Length.Zero),
            To = new SheetCellPoint(2, Length.Zero, 516, Length.Zero),
        };

        SheetLayout sheet = Sheet([], new SheetDrawings([tall]));

        // Whatever the exact page count, it is the whole block rather than the one page a cell
        // would have kept — the drawing reaches every one of them.
        PageCount(sheet).ShouldBeGreaterThan(2);
    }

    /// <summary>
    /// A cell that exists and holds nothing is not content, and does not keep a page.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>ScTable::IsBlockEmpty</c> asks each column for <c>IsEmptyData</c>
    /// (<c>sc/source/core/data/table2.cxx:2432-2452</c>), which reads the cell store alone: a cell
    /// carrying nothing but a style index is not in it, and its attributes reach the question only
    /// through the separate <c>HasAttrFlags::Lines</c> test. Every format writes such cells in
    /// quantity — SpreadsheetML a <c>&lt;c r="I1" s="13"/&gt;</c>, BIFF a <c>MULBLANK</c> across a
    /// whole formatted row — so reading "a cell record exists here" as content keeps a page for
    /// every band of styled-but-empty columns.
    /// </para>
    /// <para>
    /// Measured on <c>Bulletin-37-Appendix-2-immediate-detriment-data-request.xlsx</c>, whose
    /// columns I to P carry a style on every cell of rows 1 to 15 and nothing else: LibreOffice
    /// drops that column band entirely and prints five pages where Paperless printed six.
    /// </para>
    /// </remarks>
    [Fact]
    public void AnEmptyCellIsNotContentAndDoesNotKeepAPage()
    {
        // Content in column A and in column BZ, which on A4 are the first and the last of about a
        // dozen column bands. Every column between them carries a cell that holds no value and no
        // text — what a formatted-but-blank column looks like once it has been read.
        SheetLayout sparse = Sheet([(0, 0, "left"), (0, 77, "far right")]);

        ContentTable table = new();
        ContentTableRow row = new() { Index = 0 };
        foreach (ContentTableCell cell in sparse.Cells!.Children.OfType<ContentTableRow>()
                                                .SelectMany(r => r.Children.OfType<ContentTableCell>()))
        {
            while (row.Children.OfType<ContentTableCell>().Count() < cell.Column)
            {
                int gap = row.Children.OfType<ContentTableCell>().Count();
                row.Children.Add(new ContentTableCell { Row = 0, Column = gap });
            }
            row.Children.Add(cell);
        }
        table.Children.Add(row);

        SheetLayout styled = new() { Name = "Sheet1", Cells = table };

        // The block is the same either way, so any difference in page count is this rule alone.
        styled.PrintedRange.LastColumn.ShouldBe(77);
        PageCount(styled).ShouldBe(PageCount(sparse));

        // And that count is the two bands that hold something, not a page per band of the block.
        PageCount(styled).ShouldBe(2);
    }

    [Fact]
    public void ASheetWithNothingOnItStillPrintsOnePage()
    {
        // A drawing gives the sheet a print area; nothing keeps any page. Losing the sheet
        // silently would be worse than printing it blank, and Calc got this far only by finding
        // a print area for it.
        SheetDrawing dot = new()
        {
            Anchor = SheetAnchorKind.Absolute,
            Position = new DocPoint(Length.Zero, Length.Zero),
            Extent = new DocSize(Length.Zero, Length.Zero),
        };

        PageCount(Sheet([], new SheetDrawings([dot]))).ShouldBe(1);
    }
}
