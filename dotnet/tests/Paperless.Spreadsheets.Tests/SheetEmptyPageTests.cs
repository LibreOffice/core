using Paperless.Core.Extraction;
using Paperless.Core.Geometry;
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
