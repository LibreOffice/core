using Paperless.Core.Extraction;
using Paperless.Core.Units;

namespace Paperless.Spreadsheets.Layout;

/// <summary>
/// Drops the pages a printout of a sheet would leave blank.
/// </summary>
/// <remarks>
/// <para>
/// Pagination divides the whole printed block into pages, and Calc then throws away the ones with
/// nothing on them. It is not a nicety: the printed block runs from A1 to the far corner of
/// whatever the sheet reaches, so a sheet whose only content sits five hundred rows down is paged
/// into ten sheets of paper of which nine are white. Calc prints one.
/// <c>ScPrintPageRangesProvider</c> drops a whole band of rows when
/// <c>ScDocument::IsPrintEmpty</c> is true across it, and <c>lcl_SetHidden</c> then hides the
/// individual pages inside a band that survived (<c>sc/source/ui/view/printfun.cxx:3174, :3138</c>)
/// — both asking the same question of the same kind of block, so one test per page gives the same
/// answer as their two passes. The switch is <c>bSkipEmpty</c>, which comes from the print options
/// and is on by default.
/// </para>
/// <para>
/// <strong>Three things keep a page, and only one of them is cells</strong>
/// (<c>ScDocument::IsPrintEmpty</c>, <c>sc/source/core/data/documen9.cxx:449-484</c>): a cell with
/// something in it; a border anywhere in the block — "we want to print sheets with borders even if
/// there is no cell content"; and any drawing whose bounding rectangle <em>overlaps</em> the block,
/// through <c>HasAnyDraw</c>, which tests every object on the page and not merely the ones anchored
/// inside it. The third is why this had to arrive with the shapes: a form control anchored at row
/// 516 of <c>sc/qa/unit/data/xlsx/singlecontrol.xlsx</c> — a sheet with no cells at all — is the
/// only thing on it, and it turned that sheet from nothing into ten pages until this dropped the
/// nine it does not touch. LibreOffice prints one.
/// </para>
/// <para>
/// <strong>This is deliberately narrower than Calc's.</strong> A page is dropped only when nothing
/// in the block is a cell <em>at all</em> — not when its cells exist and are empty — and only when
/// no cell lies to its left on any of its rows. Calc instead re-runs <c>ExtendPrintArea</c> from
/// the start column to see whether a long string in an earlier column reaches in
/// (<c>documen9.cxx:486-500</c>), and carries <c>bLeftIsEmpty</c> across the row so it can skip
/// that work. The narrower rule needs no measurement and cannot drop a page that overflowing text
/// would have reached, at the cost of keeping a blank page in the middle of a wide sparse sheet.
/// Widening it means measuring, and measuring is what the whole-corpus sweep is sensitive to.
/// </para>
/// </remarks>
internal static class SheetEmptyPages
{
    /// <summary>The placements worth printing, in order.</summary>
    /// <param name="sheet">The sheet being paginated.</param>
    /// <param name="placements">Every page the block divides into.</param>
    public static List<SheetPagePlacement> Occupied(
        SheetLayout sheet, IReadOnlyList<SheetPagePlacement> placements)
    {
        ArgumentNullException.ThrowIfNull(sheet);
        ArgumentNullException.ThrowIfNull(placements);

        List<SheetPagePlacement> kept = [];
        foreach (SheetPagePlacement placement in placements)
        {
            if (!IsBlank(sheet, placement)) kept.Add(placement);
        }

        // Never all of them. A sheet whose every page is blank still prints one, because Calc got
        // that far only by finding a print area for it, and a workbook that silently loses a sheet
        // is worse than one that prints an empty page for it.
        return kept.Count == 0 && placements.Count > 0 ? [placements[0]] : kept;
    }

    private static bool IsBlank(SheetLayout sheet, SheetPagePlacement placement)
    {
        SheetRange block = placement.Cells;

        // The repeated bands print on every page and are part of what is on it.
        if (placement.RepeatColumns is not null || placement.RepeatRows is not null) return false;

        for (int row = block.FirstRow; row <= block.LastRow; row++)
        {
            for (int column = block.FirstColumn; column <= block.LastColumn; column++)
            {
                if (sheet.CellAt(row, column) is not null) return false;
                if (sheet.IsMerged(row, column)) return false;
                if (!sheet.Formatting.IsEmpty && !sheet.Formatting.At(row, column).IsNone)
                    return false;
            }

            // Anything to the left on this row may run through into the block, and deciding
            // whether it does needs the string measured. Keeping the page instead is the
            // conservative answer and costs a blank page nobody has.
            for (int column = 0; column < block.FirstColumn; column++)
                if (sheet.CellAt(row, column) is not null) return false;
        }

        return !TouchedByADrawing(sheet, block);
    }

    /// <summary>
    /// True when any drawing's rectangle overlaps the block's.
    /// </summary>
    /// <remarks>
    /// Overlap rather than containment, and every drawing on the sheet rather than the ones
    /// anchored in the block: <c>ScDocument::HasAnyDraw</c> walks the whole page and asks
    /// <c>GetCurrentBoundRect().Overlaps(rMMRect)</c> of each (<c>documen9.cxx:381-403</c>). A
    /// chart spanning four pages therefore keeps all four, which is what
    /// <c>sc/qa/unit/data/xlsb/universal-content.xlsb</c> shows: one hidden shape reaching column
    /// 12 and row 50, and four pages of paper for it.
    /// </remarks>
    private static bool TouchedByADrawing(SheetLayout sheet, SheetRange block)
    {
        if (sheet.Drawings.IsEmpty) return false;

        long left = Start(block.FirstColumn, sheet.Grid.Columns);
        long top = Start(block.FirstRow, sheet.Grid.Rows);
        long right = left + sheet.Grid.Columns
            .TotalPrintedSize(block.FirstColumn, block.LastColumn).Twips;
        long bottom = top + sheet.Grid.Rows
            .TotalPrintedSize(block.FirstRow, block.LastRow).Twips;

        foreach (SheetDrawing drawing in sheet.Drawings.Items)
        {
            (long x, long y, long x2, long y2) = Bounds(drawing, sheet.Grid);
            if (x2 >= left && x <= right && y2 >= top && y <= bottom) return true;
        }

        return false;
    }

    /// <summary>A drawing's rectangle, in twips from the sheet's origin.</summary>
    private static (long Left, long Top, long Right, long Bottom) Bounds(
        SheetDrawing drawing, SheetGrid grid)
    {
        if (drawing.Anchor == SheetAnchorKind.Absolute)
        {
            long x = drawing.Position.X.Twips;
            long y = drawing.Position.Y.Twips;
            return (x, y, x + drawing.Extent.Width.Twips, y + drawing.Extent.Height.Twips);
        }

        long left = Start(drawing.From.Column, grid.Columns) + drawing.From.ColumnOffset.Twips;
        long top = Start(drawing.From.Row, grid.Rows) + drawing.From.RowOffset.Twips;

        if (drawing.Anchor == SheetAnchorKind.OneCell)
        {
            return (left, top,
                    left + drawing.Extent.Width.Twips, top + drawing.Extent.Height.Twips);
        }

        long right = Start(drawing.To.Column, grid.Columns) + drawing.To.ColumnOffset.Twips;
        long bottom = Start(drawing.To.Row, grid.Rows) + drawing.To.RowOffset.Twips;
        return (left, top, Math.Max(left, right), Math.Max(top, bottom));
    }

    private static long Start(int index, SheetAxis axis)
        => index <= 0 ? 0 : axis.TotalPrintedSize(0, index - 1).Twips;
}
