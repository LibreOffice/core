using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Spreadsheets.Layout;

/// <summary>
/// Places and paints the pictures, charts and shape text anchored on one page.
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
/// <strong>A drawing does not belong to a page; it belongs to the sheet, and every page it reaches
/// prints the part that lands on the paper.</strong> <c>ScOutputData::PrePrintDrawingLayer</c>
/// (<c>sc/source/ui/view/output3.cxx:40-104</c>) sets a map-mode offset of minus the width of the
/// columns and the height of the rows <em>before</em> the page's first, and
/// <c>PrintDrawingLayer</c> (<c>:138</c>) then paints the whole drawing page through it, so a
/// picture straddling a break appears on both pages, cut. The anchor is therefore resolved against
/// the page's own columns wherever it can be and walked out through the grid in <em>either</em>
/// direction where it cannot: a drawing anchored left of the band starts at a negative offset and
/// shows its right-hand part, exactly as one anchored past the last printed row shows its top.
/// Measured on <c>Air_Boss_Master_List.xlsx</c>, whose note box is anchored in column E and
/// straddles the column break: LibreOffice prints its left half on page 1 and its right half on
/// page 3, and anchoring it to one page lost the second half.
/// </para>
/// <para>
/// <strong>What bounds it is the page's own cell block, not the paper.</strong> The rectangle
/// <c>PrePrintDrawingLayer</c> hands to <c>BeginDrawLayers</c> runs exactly from the page's first
/// printed column to its last, so a drawing anchored in a band this page does not print is culled
/// even where the margin would have left room for it. Without that test every column band of a
/// sheet several bands wide carries every drawing on the sheet — see
/// <see cref="ReachesTheBlock"/>.
/// </para>
/// </remarks>
internal sealed class SheetPageGraphics(SheetLayout sheet, double scale)
{
    /// <summary>Paints the sheet's drawings that belong to this page.</summary>
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
            if (drawing.Image is null && drawing.Vector is null && drawing.Chart is null
                && drawing.Text is null)
            {
                continue;
            }

            if (Place(drawing, byColumn, byRow) is not { } box) continue;
            if (box.Width <= Length.Zero || box.Height <= Length.Zero) continue;
            if (!ReachesTheBlock(box, columns, rows)) continue;

            // The vector before the raster, since a shape carrying both means the DrawingML `svgBlip`
            // case where the raster is the fallback. `VectorImage.Draw` maps the picture's own frame
            // onto the box and clips to it — the same stretch `DrawImage` gives a raster, and not the
            // extent of the picture's ink, which would put a logo with margins outside its anchor.
            if (drawing.Vector is { } vector && !vector.Value.IsEmpty) vector.Value.Draw(sink, box);
            else if (drawing.Image is { } image) sink.DrawImage(image, box);
            else if (drawing.Chart is { } chart) SheetChart.Draw(sink, chart, box, scale);

            // Not an `else`: a shape may carry both a picture and a caption, and the text goes
            // over the fill rather than instead of it.
            if (drawing.Text is { } text) SheetShapePainter.Draw(sink, text, box, scale);
        }
    }

    /// <summary>Whether any part of a placed rectangle falls inside the page's own cell block.</summary>
    /// <remarks>
    /// <para>
    /// <strong>The clip is the block, not the paper</strong>, and the difference is the whole
    /// question of which page a drawing appears on. <c>PrePrintDrawingLayer</c> builds a rectangle
    /// running from the width of the columns before the page's first to the width of the columns it
    /// prints, and the same on the row axis, and hands <em>that</em> to
    /// <c>BeginDrawLayers</c> as the paint region (<c>sc/source/ui/view/output3.cxx:41-95</c>).
    /// So a drawing anchored in a column band the page does not print is culled even when the
    /// margin would have left room for it on the paper.
    /// </para>
    /// <para>
    /// Measured on <c>Part_375_Operators.xlsx</c>, whose two table slicers sit in columns E and F —
    /// the third of its three column bands. The bands are narrow enough that the second band's
    /// right edge stops well short of the right margin, so both slicers fitted on the paper of the
    /// first band's pages as well: LibreOffice draws them once, on page 19, and we drew them on
    /// pages 1, 10 and 19 — 2251 words against 2197.
    /// </para>
    /// </remarks>
    private static bool ReachesTheBlock(
        DocRect box, List<PlacedColumn> columns, List<PlacedRow> rows)
    {
        Length left = columns[0].X;
        Length right = columns[0].Right;
        foreach (PlacedColumn column in columns)
        {
            if (column.X < left) left = column.X;
            if (column.Right > right) right = column.Right;
        }

        Length top = rows[0].Y;
        Length bottom = rows[0].Bottom;
        foreach (PlacedRow row in rows)
        {
            if (row.Y < top) top = row.Y;
            if (row.Bottom > bottom) bottom = row.Bottom;
        }

        // Inclusive on both edges, because Calc's is: `aRect` is a `tools::Rectangle`, whose
        // `Right()` and `Bottom()` are the last coordinates *inside* it, so a drawing whose left
        // edge sits exactly on the block's right edge still overlaps it by one unit. Measured on
        // `sheet-shape-clip.xlsx`, whose box is anchored in the first column of the second band and
        // which LibreOffice prints on both pages.
        return box.X + box.Width >= left && box.X <= right
               && box.Y + box.Height >= top && box.Y <= bottom;
    }

    /// <summary>Where a drawing lands on this page, or null when it does not.</summary>
    private DocRect? Place(
        SheetDrawing drawing,
        Dictionary<int, PlacedColumn> columns,
        Dictionary<int, PlacedRow> rows)
    {
        if (drawing.Anchor == SheetAnchorKind.Absolute)
        {
            // Measured from the sheet's own origin, which is where A1 would sit on this page —
            // off to the left of a band that does not start at column A, and off the top of one
            // that does not start at row 1. Resolving it through the same walk a cell anchor uses
            // is what puts an absolutely-positioned shape on every page it reaches instead of only
            // on the first.
            if (ColumnX(0, columns) is not { } sheetX) return null;
            if (RowY(0, rows) is not { } sheetY) return null;

            return new DocRect(
                sheetX + (SheetDeviceUnits.Snap(drawing.Position.X) * scale),
                sheetY + (SheetDeviceUnits.Snap(drawing.Position.Y) * scale),
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
    /// <para>
    /// <strong>And backwards, for the same reason.</strong> A page whose band starts at column C is
    /// still a window onto one sheet, so a drawing anchored in column A sits at a negative offset on
    /// it and shows whatever reaches past the left margin. Returning null for that case — which is
    /// what this did — lost the right-hand half of every drawing straddling a column break.
    /// A column inside the band that has no placement is a <em>hidden</em> column, which occupies
    /// nothing: it is left to the caller as null rather than resolved to the next column's start,
    /// because a drawing anchored in a hidden column is one Calc collapses away.
    /// </para>
    /// </remarks>
    private Length? ColumnX(int column, Dictionary<int, PlacedColumn> columns)
    {
        if (columns.TryGetValue(column, out PlacedColumn placed)) return placed.X;

        int first = int.MaxValue;
        int last = -1;
        Length left = Length.Zero;
        Length right = Length.Zero;

        foreach (PlacedColumn candidate in columns.Values)
        {
            if (candidate.Column > last) { last = candidate.Column; right = candidate.Right; }
            if (candidate.Column < first) { first = candidate.Column; left = candidate.X; }
        }

        if (last < 0) return null;

        if (column > last)
        {
            for (int at = last + 1; at < column; at++)
                right += SheetDeviceUnits.Snap(sheet.Grid.Columns.PrintedSizeAt(at)) * scale;
            return right;
        }

        if (column > first) return null;

        for (int at = column; at < first; at++)
            left -= SheetDeviceUnits.Snap(sheet.Grid.Columns.PrintedSizeAt(at)) * scale;

        return left;
    }

    /// <summary>
    /// Where a row starts on the page, continuing past the rows it prints in either direction.
    /// </summary>
    /// <remarks>See <see cref="ColumnX"/>; the rules are the same on both axes.</remarks>
    private Length? RowY(int row, Dictionary<int, PlacedRow> rows)
    {
        if (rows.TryGetValue(row, out PlacedRow placed)) return placed.Y;

        int first = int.MaxValue;
        int last = -1;
        Length top = Length.Zero;
        Length bottom = Length.Zero;

        foreach (PlacedRow candidate in rows.Values)
        {
            if (candidate.Row > last) { last = candidate.Row; bottom = candidate.Bottom; }
            if (candidate.Row < first) { first = candidate.Row; top = candidate.Y; }
        }

        if (last < 0) return null;

        if (row > last)
        {
            for (int at = last + 1; at < row; at++)
                bottom += SheetDeviceUnits.Snap(sheet.Grid.Rows.PrintedSizeAt(at)) * scale;
            return bottom;
        }

        if (row > first) return null;

        for (int at = row; at < first; at++)
            top -= SheetDeviceUnits.Snap(sheet.Grid.Rows.PrintedSizeAt(at)) * scale;

        return top;
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
