using XRay.Office.Core.Units;

namespace XRay.Office.Spreadsheets.Layout;

/// <summary>
/// Where a drawing's bounding rectangle falls on the sheet, in twips from its origin.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Two questions about a drawing decide pages, and both want the same rectangle.</strong>
/// <c>ScDrawLayer::GetPrintArea</c> widens the printed block to cover every object
/// (<c>sc/source/core/data/drwlayer.cxx:1400-1424</c>) and <c>ScDocument::HasAnyDraw</c> keeps a
/// page an object overlaps (<c>documen9.cxx:382-404</c>); both ask <c>GetCurrentBoundRect</c>, so
/// they are one computation with two callers — see <see cref="SheetDrawingArea"/> and
/// <see cref="SheetEmptyPages"/>.
/// </para>
/// <para>
/// <strong>The bounding rectangle is not the anchor.</strong> A drawing's anchor states the frame
/// its shapes are laid out in; its bound rect is the union of what those shapes actually cover,
/// and a <em>turned</em> shape covers more than its own box. The distinction is invisible on a
/// plain picture and decides four pages on <c>SIL_TDB648.xlsx</c>, whose ten sheets each carry a
/// group of seven watermark pictures turned 27°: the union of their turned boxes reaches
/// <strong>4.2% further down</strong> than the group's frame and stops <strong>1.1% short</strong>
/// of its right edge. Reading the frame instead put the print area one band of rows too shallow on
/// two sheets — LibreOffice prints a blank fifth band of <c>TerrDB Verification</c> and a fourth of
/// <c>RAAS</c>, four pages we did not — and one column too wide on a third, where we printed three
/// blank pages of <c>RUNWAYS</c> that LibreOffice drops. Checked against LibreOffice's own answer
/// by exporting the workbook to flat ODF and reading the <c>table:end-cell-address</c> it wrote for
/// each group: <strong>10 of 10 columns and 9 of 10 rows exact</strong>, the tenth one row out.
/// </para>
/// <para>
/// <strong>The turn has to be applied after the group is resized, not before.</strong> A group
/// stretched to an anchor scales its children by different factors horizontally and vertically, and
/// the bounding box of a turned rectangle is not linear in those factors — scaling the box and then
/// turning it gives a different answer from turning it and then scaling. Measured on the same
/// workbook: the wrong order makes each watermark 197 pt tall where the reference draws it 255, and
/// the right order gives 252. That is why <see cref="SheetDrawing.Parts"/> carries the shapes rather
/// than a fixed inset the reader could have folded away.
/// </para>
/// </remarks>
internal static class SheetDrawingBounds
{
    /// <summary>A drawing's bounding rectangle, in twips from the sheet's origin.</summary>
    /// <param name="drawing">The drawing.</param>
    /// <param name="grid">The geometry to place its anchor against.</param>
    public static (long Left, long Top, long Right, long Bottom) Of(
        SheetDrawing drawing, SheetGrid grid)
    {
        (long left, long top, long right, long bottom) = Frame(drawing, grid);
        if (drawing.Parts.Count == 0) return (left, top, right, bottom);

        double width = right - left;
        double height = bottom - top;

        double minX = double.MaxValue;
        double minY = double.MaxValue;
        double maxX = double.MinValue;
        double maxY = double.MinValue;

        foreach (SheetDrawingPart part in drawing.Parts)
        {
            double partWidth = part.Width * width;
            double partHeight = part.Height * height;
            double centreX = left + (part.X + (part.Width / 2)) * width;
            double centreY = top + (part.Y + (part.Height / 2)) * height;

            double radians = part.Degrees * Math.PI / 180.0;
            double cos = Math.Abs(Math.Cos(radians));
            double sin = Math.Abs(Math.Sin(radians));
            double boxWidth = (partWidth * cos) + (partHeight * sin);
            double boxHeight = (partWidth * sin) + (partHeight * cos);

            minX = Math.Min(minX, centreX - (boxWidth / 2));
            maxX = Math.Max(maxX, centreX + (boxWidth / 2));
            minY = Math.Min(minY, centreY - (boxHeight / 2));
            maxY = Math.Max(maxY, centreY + (boxHeight / 2));
        }

        if (minX > maxX || minY > maxY) return (left, top, right, bottom);

        return ((long)Math.Round(minX), (long)Math.Round(minY),
                (long)Math.Round(maxX), (long)Math.Round(maxY));
    }

    /// <summary>The rectangle the drawing's anchor states, in twips from the sheet's origin.</summary>
    private static (long Left, long Top, long Right, long Bottom) Frame(
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

    /// <summary>Where a column or row starts, in twips, hidden ones contributing nothing.</summary>
    private static long Start(int index, SheetAxis axis)
        => index <= 0 ? 0 : axis.TotalPrintedSize(0, index - 1).Twips;
}
