using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Spreadsheets.Layout;

/// <summary>
/// Places and paints the pictures and charts anchored on one page.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Drawn after the grid, which is where Calc draws them.</strong>
/// <c>ScPrintFunc::PrintPage</c> runs the back drawing layer, the backgrounds, the borders, the
/// strings, the grid and only then <c>PrintDrawingLayer(SC_LAYER_FRONT)</c>
/// (<c>sc/source/ui/view/printfun.cxx:1651-1703</c>), and a picture goes on the front layer. So a
/// logo covers the gridlines under it rather than being crossed by them, which is visible on any
/// sheet that prints its grid.
/// </para>
/// <para>
/// <strong>The rectangle is the anchor's, resolved against this page's own columns.</strong> A
/// two-cell anchor states two cells and two offsets and the drawing spans whatever lies between
/// them, so inserting a column moves it — that is the whole point of the anchor and it is why a
/// picture cannot be placed until the grid is known
/// (<c>ShapeAnchor::calcAnchorRectEmu</c>, <c>sc/source/filter/oox/drawingbase.cxx:190</c>). The
/// span is measured through the same <see cref="SheetDeviceUnits"/> the cells go through, or a
/// picture two columns wide would not line up with the column it ends in.
/// </para>
/// <para>
/// <strong>A drawing belongs to the page holding its top-left cell.</strong> Calc positions the
/// drawing layer in document coordinates and clips it to the printed block, so a picture
/// straddling a page break appears on both pages, cut. Anchoring it to one page and clipping is
/// the same answer for everything that does not straddle and a simpler one for what does; the
/// difference is recorded in the module's TODO.
/// </para>
/// </remarks>
internal sealed class SheetPageGraphics(SheetLayout sheet, double scale)
{
    /// <summary>Paints the sheet's pictures that belong to this page.</summary>
    /// <param name="sink">Receives the drawing commands.</param>
    /// <param name="columns">The columns on the page, with their positions.</param>
    /// <param name="rows">The rows on the page.</param>
    public void Draw(IDrawingSink sink, List<PlacedColumn> columns, List<PlacedRow> rows)
    {
        ArgumentNullException.ThrowIfNull(sink);
        ArgumentNullException.ThrowIfNull(columns);
        ArgumentNullException.ThrowIfNull(rows);

        if (sheet.Drawings.IsEmpty || columns.Count == 0 || rows.Count == 0) return;

        Dictionary<int, PlacedColumn> byColumn = [];
        foreach (PlacedColumn column in columns) byColumn.TryAdd(column.Column, column);

        Dictionary<int, PlacedRow> byRow = [];
        foreach (PlacedRow row in rows) byRow.TryAdd(row.Row, row);

        foreach (SheetDrawing drawing in sheet.Drawings.Items)
        {
            if (drawing.IsHidden) continue;
            if (drawing.Image is null && drawing.Chart is null) continue;
            if (Place(drawing, byColumn, byRow) is not { } box) continue;
            if (box.Width <= Length.Zero || box.Height <= Length.Zero) continue;

            if (drawing.Image is { } image) sink.DrawImage(image, box);
            else if (drawing.Chart is { } chart) SheetChart.Draw(sink, chart, box, scale);
        }
    }

    /// <summary>Where a drawing lands on this page, or null when it does not.</summary>
    private DocRect? Place(
        SheetDrawing drawing,
        Dictionary<int, PlacedColumn> columns,
        Dictionary<int, PlacedRow> rows)
    {
        if (drawing.Anchor == SheetAnchorKind.Absolute)
        {
            // Measured from the sheet's own origin, so it lands on the page holding A1 and
            // nowhere else. Calc treats it the same way: the position is a document coordinate
            // and the first page is the one whose block contains it.
            if (!columns.TryGetValue(0, out PlacedColumn first)) return null;
            if (!rows.TryGetValue(0, out PlacedRow top)) return null;

            return new DocRect(
                first.X + (SheetDeviceUnits.Snap(drawing.Position.X) * scale),
                top.Y + (SheetDeviceUnits.Snap(drawing.Position.Y) * scale),
                SheetDeviceUnits.Snap(drawing.Extent.Width) * scale,
                SheetDeviceUnits.Snap(drawing.Extent.Height) * scale);
        }

        if (ColumnX(drawing.From.Column, columns) is not { } columnX) return null;
        if (RowY(drawing.From.Row, rows) is not { } rowY) return null;

        Length x = columnX + (SheetDeviceUnits.Snap(drawing.From.ColumnOffset) * scale);
        Length y = rowY + (SheetDeviceUnits.Snap(drawing.From.RowOffset) * scale);

        if (drawing.Anchor == SheetAnchorKind.OneCell)
        {
            return new DocRect(
                x, y,
                SheetDeviceUnits.Snap(drawing.Extent.Width) * scale,
                SheetDeviceUnits.Snap(drawing.Extent.Height) * scale);
        }

        Length right = Edge(
            drawing.From.Column, drawing.From.ColumnOffset,
            drawing.To.Column, drawing.To.ColumnOffset,
            sheet.Grid.Columns);

        Length bottom = Edge(
            drawing.From.Row, drawing.From.RowOffset,
            drawing.To.Row, drawing.To.RowOffset,
            sheet.Grid.Rows);

        return new DocRect(x, y, right * scale, bottom * scale);
    }

    /// <summary>
    /// Where a column starts on the page, continuing past the last one the page prints.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A drawing may be anchored outside the print area, and Calc still prints it.</strong>
    /// The print area is computed from the <em>cells</em> — <c>ScTable::GetPrintArea</c>
    /// (<c>sc/source/core/data/table1.cxx:657</c>) tests data, notes, sparklines and attributes and
    /// never asks the drawing layer — but the drawing layer is then painted in document
    /// coordinates and clipped to the paper, not to the used range
    /// (<c>PrintDrawingLayer(SC_LAYER_FRONT)</c>, <c>printfun.cxx:1699</c>). So a chart anchored
    /// three rows below the last number prints, and looking its anchor up in the page's own placed
    /// rows finds nothing.
    /// </para>
    /// <para>
    /// Measured: <c>chart-bar-sheet.ods</c> holds four rows of data in <c>A1:C5</c> and anchors its
    /// chart in row 7, so every row the chart needs is outside the used range and the whole picture
    /// was dropped. Walking on from the last printed column or row through the grid — snapped per
    /// index, as <see cref="Edge"/> does — puts it where Calc puts it.
    /// </para>
    /// </remarks>
    private Length? ColumnX(int column, Dictionary<int, PlacedColumn> columns)
    {
        if (columns.TryGetValue(column, out PlacedColumn placed)) return placed.X;

        int last = -1;
        Length x = Length.Zero;

        foreach (PlacedColumn candidate in columns.Values)
        {
            if (candidate.Column <= last) continue;
            last = candidate.Column;
            x = candidate.Right;
        }

        if (last < 0 || column <= last) return null;

        for (int at = last + 1; at < column; at++)
            x += SheetDeviceUnits.Snap(sheet.Grid.Columns.PrintedSizeAt(at)) * scale;

        return x;
    }

    /// <summary>Where a row starts on the page, continuing past the last one it prints.</summary>
    /// <remarks>See <see cref="ColumnX"/>; the reason is the same and it is the row axis that
    /// the corpus exercises.</remarks>
    private Length? RowY(int row, Dictionary<int, PlacedRow> rows)
    {
        if (rows.TryGetValue(row, out PlacedRow placed)) return placed.Y;

        int last = -1;
        Length y = Length.Zero;

        foreach (PlacedRow candidate in rows.Values)
        {
            if (candidate.Row <= last) continue;
            last = candidate.Row;
            y = candidate.Bottom;
        }

        if (last < 0 || row <= last) return null;

        for (int at = last + 1; at < row; at++)
            y += SheetDeviceUnits.Snap(sheet.Grid.Rows.PrintedSizeAt(at)) * scale;

        return y;
    }

    /// <summary>
    /// How far a two-cell anchor reaches along one axis, from its start offset to its end offset.
    /// </summary>
    /// <remarks>
    /// Summed run by run through the axis so that a picture spanning a hidden column collapses
    /// with it, which is what a hidden column does to everything else on the page, and snapped
    /// per column so that the far edge lands on the same device unit the column's own gridline
    /// does.
    /// </remarks>
    private static Length Edge(
        int from, Length fromOffset, int to, Length toOffset, SheetAxis axis)
    {
        if (to < from) return Length.Zero;

        if (to == from)
        {
            Length span = toOffset - fromOffset;
            return span > Length.Zero ? SheetDeviceUnits.Snap(span) : Length.Zero;
        }

        Length total = SheetDeviceUnits.Snap(axis.PrintedSizeAt(from))
                       - SheetDeviceUnits.Snap(fromOffset);

        for (int at = from + 1; at < to; at++)
            total += SheetDeviceUnits.Snap(axis.PrintedSizeAt(at));

        return total + SheetDeviceUnits.Snap(toOffset);
    }
}
