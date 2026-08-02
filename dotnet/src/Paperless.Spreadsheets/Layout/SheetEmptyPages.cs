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
/// <strong>The question is whether the block holds cell <em>content</em>, not whether it holds a
/// cell.</strong> <c>ScTable::IsBlockEmpty</c> (<c>sc/source/core/data/table2.cxx:2432-2452</c>)
/// asks each column for <c>IsEmptyData</c>, which reads the cell store alone; a cell carrying
/// nothing but a style index is not in that store, and its attributes are consulted only by the
/// separate <c>HasAttrFlags::Lines</c> test below. Reading "a cell record exists here" as content
/// instead keeps a page for every band of styled-but-empty columns — measured on
/// <c>Bulletin-37-Appendix-2-immediate-detriment-data-request.xlsx</c>, whose columns I to P carry
/// a style on every cell of rows 1 to 15 and nothing else: LibreOffice drops that whole column
/// band and prints five pages where we printed six.
/// </para>
/// <para>
/// The fourth test — whether a long string in a column to the left reaches into the block — is
/// Calc's own, measured the way Calc measures it; see <see cref="ReachedFromTheLeft"/>.
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

        // The repeated bands are deliberately *not* consulted, though they print on this page and
        // reading them as content is the obvious thing to do. Calc asks `IsPrintEmpty` for the
        // page's own block alone — `IsPrintEmpty(getStartColumn(), nPageStartRow, getEndColumn(),
        // nRow-1, …)` and the same range through `lcl_SetHidden`
        // (`sc/source/ui/view/printfun.cxx:3174, :3053`) — and a repeated band is added by
        // `PrintPage` afterwards, so it never enters the question. Counting it kept every page of
        // every sheet that declares `_xlnm.Print_Titles`, which disabled this whole class for them:
        // `fy20-may20-sep20.xlsx` states `Print_Titles` for row 1, and its column F reaches only
        // row 76, so Calc prints two pages of the second column band and we printed 103 blank ones
        // — 233 pages against 96.
        for (int row = block.FirstRow; row <= block.LastRow; row++)
        {
            for (int column = block.FirstColumn; column <= block.LastColumn; column++)
            {
                if (!SheetTextLayout.IsAvailable(sheet.CellAt(row, column))) return false;
                if (sheet.IsMerged(row, column)) return false;

                // A border keeps the page and a background does not, which reads as an oversight
                // and is not one: Calc asks for `HasAttrFlags::Lines`, and that flag tests the
                // four edges of ATTR_BORDER and nothing else (attarray.cxx:1279-1284). Asking
                // whether the cell is decorated at all instead keeps every page of a sheet with a
                // banded or shaded region — measured on grants-2005.xls, whose shading reaches
                // far past its cells: 1170 pages of which 949 were blank, against 220.
                if (!sheet.Formatting.IsEmpty
                    && !sheet.Formatting.At(row, column).Borders.IsNone)
                {
                    return false;
                }
            }
        }

        if (TouchedByADrawing(sheet, block)) return false;

        return !ReachedFromTheLeft(sheet, block);
    }

    /// <summary>
    /// True when a string in a column left of the block overflows far enough to reach into it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The last of <c>IsPrintEmpty</c>'s four tests and the only one that needs a font
    /// (<c>sc/source/core/data/documen9.cxx:486-500</c>): Calc re-runs <c>ExtendPrintArea</c> over
    /// the block's own rows, from column zero up to the column before the block, and keeps the
    /// page when the extension reaches the block. So a page holding no cells is still printed
    /// when a long label two columns to its left spills onto it — and dropped when it does not.
    /// </para>
    /// <para>
    /// It was previously answered by "is there any cell at all to the left", which is the
    /// conservative side of the same question and needs no measurement. That was the right
    /// trade while the measurement it depends on was untrustworthy; it is the wrong one now,
    /// because on a sheet several column bands wide it keeps <em>every</em> band of every row
    /// that has anything in column A. Measured on <c>RCO_VOR_Master_List_082824.xlsx</c>:
    /// 183 pages of which 103 were blank, against LibreOffice's 80 with none.
    /// </para>
    /// <para>
    /// The cheap half of Calc's own short-circuit is kept: it carries <c>bLeftIsEmpty</c> across
    /// the row band so that a band with nothing to its left is never measured at all. Asking
    /// whether any cell there holds content first is the same saving without the bookkeeping, and
    /// it is the same question <c>bLeftIsEmpty</c> answers: the flag is <c>IsPrintEmpty</c>'s own
    /// verdict on the band to the left, and that verdict starts at <c>IsBlockEmpty</c>.
    /// </para>
    /// </remarks>
    private static bool ReachedFromTheLeft(SheetLayout sheet, SheetRange block)
    {
        if (block.FirstColumn <= 0) return false;

        bool anythingLeft = false;
        for (int row = block.FirstRow; row <= block.LastRow && !anythingLeft; row++)
        {
            for (int column = 0; column < block.FirstColumn; column++)
            {
                if (SheetTextLayout.IsAvailable(sheet.CellAt(row, column))) continue;
                anythingLeft = true;
                break;
            }
        }

        if (!anythingLeft) return false;

        SheetRange left = new(0, block.FirstRow, block.FirstColumn - 1, block.LastRow);
        return SheetTextOverflow.ExtendedLastColumn(sheet, left) >= block.FirstColumn;
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
