using Paperless.Core.Units;

namespace Paperless.Spreadsheets.Layout;

/// <summary>
/// Widens a sheet's print area to cover the drawings floating over it.
/// </summary>
/// <remarks>
/// <para>
/// A chart anchored to the right of the last cell still prints, and it prints on a page the cells
/// alone would never have produced. The reason is that Calc asks two different questions and only
/// one of them is about cells: <c>ScTable::GetPrintArea</c>
/// (<c>sc/source/core/data/table1.cxx:657</c>) tests data, notes, sparklines and attributes and
/// never looks at the drawing layer, but its caller <c>ScDocument::GetPrintArea</c>
/// (<c>sc/source/core/data/documen2.cxx:644-664</c>) then takes the maximum of that answer and
/// <c>ScDrawLayer::GetPrintArea</c>'s (<c>drwlayer.cxx:1344</c>), which is the bounding box of
/// every visible object on the sheet expressed back as a cell. <c>ScPrintFunc::AdjustPrintArea</c>
/// calls the document's, not the table's (<c>printfun.cxx:705</c>), so the widened range is what
/// pagination sees — and only afterwards is it widened again for overflowing text
/// (<c>ExtendPrintArea</c>, <c>printfun.cxx:759</c>), which is why this runs first.
/// </para>
/// <para>
/// This is a different rule from the one <see cref="SheetPageGraphics"/> already carries. That one
/// says a drawing outside the printed block is still <em>painted</em>, because the drawing layer is
/// clipped to the paper rather than to the cells; this one says the block itself grows, which is
/// what turns a one-page workbook into a two-page one. Measured on
/// <c>chart2/qa/extras/data/xlsx/bubble_chart_simple.xlsx</c>, four cells with a chart anchored at
/// column 11: one page here against LibreOffice's two, and 5 words against 26, with a complete
/// chart composed behind the right-hand edge of the only page produced.
/// </para>
/// <para>
/// <strong>Only the end is moved.</strong> <c>ScDocument::GetPrintArea</c> maxes the end column and
/// row and leaves the start alone, because the print area always begins at A1
/// (<c>AdjustPrintArea(true)</c> sets both starts to zero); a drawing above or left of A1 therefore
/// widens nothing. <c>GetDataStart</c> is the routine that does move the start, and printing does
/// not call it.
/// </para>
/// </remarks>
internal static class SheetDrawingArea
{
    /// <summary>
    /// The used range, widened to cover every visible drawing on the sheet.
    /// </summary>
    /// <param name="used">The block of cells the sheet holds.</param>
    /// <param name="drawings">The sheet's drawings.</param>
    /// <param name="grid">Its column widths and row heights.</param>
    public static SheetRange Extend(SheetRange used, SheetDrawings drawings, SheetGrid grid)
    {
        ArgumentNullException.ThrowIfNull(drawings);
        ArgumentNullException.ThrowIfNull(grid);

        if (drawings.IsEmpty) return used;

        long right = -1;
        long bottom = -1;

        foreach (SheetDrawing drawing in drawings.Items)
        {
            // Every object counts, hidden or not. This is the one place where "hidden" has to be
            // read carefully: `ScDrawLayer::GetPrintArea` skips an object only when it is on
            // `SC_LAYER_HIDDEN` (drwlayer.cxx:1408), and that layer holds exactly one thing — the
            // caption of a comment the user has not pinned open (postit.cxx:84). It is *not*
            // where a shape with `cNvPr hidden="1"` goes; that becomes an ordinary object with its
            // Visible property false, and the line above the layer test says so in as many words:
            // `//TODO: test Flags (hidden?)`. So a hidden shape paints nothing and still moves the
            // page break, which is what `sc/qa/unit/data/xlsb/universal-content.xlsb` shows: its
            // only drawing is a hidden comment shape spanning to column 12, and LibreOffice prints
            // four pages for it where we printed one.
            (long edgeRight, long edgeBottom) = Edges(drawing, grid);
            if (edgeRight > right) right = edgeRight;
            if (edgeBottom > bottom) bottom = edgeBottom;
        }

        if (right < 0 || bottom < 0) return used;

        int lastColumn = Math.Max(used.LastColumn, IndexAt(right, grid.Columns, SheetAddress.MaxColumn));
        int lastRow = Math.Max(used.LastRow, IndexAt(bottom, grid.Rows, SheetAddress.MaxRow));

        // A sheet holding nothing but a chart has no valid used range at all, and Calc prints it:
        // ScTable::GetPrintArea leaves its outputs at zero when it finds nothing and the document's
        // maximum is then taken against those zeroes rather than against "no area".
        return new SheetRange(0, 0, lastColumn, lastRow);
    }

    /// <summary>
    /// A drawing's right and bottom edges, in twips from the sheet's origin.
    /// </summary>
    /// <remarks>
    /// Whole twips because that is the unit the comparison is made in — Calc converts the object's
    /// hundredth-of-a-millimetre bound rect back to twips before walking the columns
    /// (<c>o3tl::toTwips</c>, <c>drwlayer.cxx:1439</c>) — and because pagination's own arithmetic
    /// is in twips for the same reason. The offsets are not snapped through
    /// <see cref="SheetDeviceUnits"/>: this decides which cell an edge falls in, not where a pen
    /// goes, and a hundredth of a millimetre never moves a cell boundary.
    /// </remarks>
    private static (long Right, long Bottom) Edges(SheetDrawing drawing, SheetGrid grid)
    {
        if (drawing.Anchor == SheetAnchorKind.Absolute)
        {
            return (drawing.Position.X.Twips + drawing.Extent.Width.Twips,
                    drawing.Position.Y.Twips + drawing.Extent.Height.Twips);
        }

        long left = Start(drawing.From.Column, grid.Columns) + drawing.From.ColumnOffset.Twips;
        long top = Start(drawing.From.Row, grid.Rows) + drawing.From.RowOffset.Twips;

        if (drawing.Anchor == SheetAnchorKind.OneCell)
            return (left + drawing.Extent.Width.Twips, top + drawing.Extent.Height.Twips);

        return (Math.Max(left, Start(drawing.To.Column, grid.Columns) + drawing.To.ColumnOffset.Twips),
                Math.Max(top, Start(drawing.To.Row, grid.Rows) + drawing.To.RowOffset.Twips));
    }

    /// <summary>Where a column or row starts, in twips, hidden ones contributing nothing.</summary>
    private static long Start(int index, SheetAxis axis)
        => index <= 0 ? 0 : axis.TotalPrintedSize(0, index - 1).Twips;

    /// <summary>
    /// Which column or row an edge falls in: the first whose accumulated size passes it.
    /// </summary>
    /// <remarks>
    /// Strictly greater, which is what puts a two-cell anchor ending exactly on a column boundary
    /// in the column it stops at rather than the one after (<c>drwlayer.cxx:1458-1470</c>).
    /// Accumulated by run through <see cref="SheetAxis.TotalPrintedSize"/> rather than index by
    /// index, because an edge past the last stated run would otherwise cost a million additions on
    /// the way to the sheet's limit.
    /// </remarks>
    private static int IndexAt(long edge, SheetAxis axis, int limit)
    {
        if (edge <= 0) return 0;

        int at = 0;
        long total = 0;
        const int Step = 64;

        while (at <= limit)
        {
            int end = Math.Min(limit, at + Step - 1);
            long block = axis.TotalPrintedSize(at, end).Twips;

            if (total + block > edge)
            {
                for (int index = at; index <= end; index++)
                {
                    total += axis.PrintedSizeAt(index).Twips;
                    if (total > edge) return index;
                }
            }

            total += block;
            at = end + 1;
        }

        return limit;
    }
}
