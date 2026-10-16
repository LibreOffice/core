using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Vector;

namespace XRay.Office.Spreadsheets.Layout;

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
/// <para>
/// <strong>And that rectangle is a clip as well as a cull, which is the half that was missing.</strong>
/// <c>BeginDrawLayers</c> stores the region on the paint window
/// (<c>SdrPageWindow::PrepareRedraw</c>, <c>svx/source/svdraw/sdrpagewindow.cxx</c>:212-224),
/// <c>RedrawAll</c>/<c>RedrawLayer</c> put it on the <c>DisplayInfo</c> (<c>:347</c>, <c>:404</c>),
/// and <c>ObjectContactOfPageView::DoProcessDisplay</c> pushes it onto the device —
/// <c>pOutDev-&gt;IntersectClipRegion(rRedrawArea)</c>,
/// <c>svx/source/sdr/contact/objectcontactofpageview.cxx</c>:163-171. So an object straddling a
/// page break is not merely drawn on both pages: on each one it is <em>cut at the block's edge</em>,
/// and the part belonging to the other page never reaches the paper even when the margin has room
/// for it. Measured on an authored two-band, three-strip workbook
/// (<c>probes/ink-pass-r92/make-clip-probe.py</c>): 26.2.4.2 emits <c>q 50.4 58.28 466.186 729.609
/// re W* n</c> — exactly six 14-character columns by fifty-seven rows, the block and not the
/// 494.9 pt text area — round every one of the six pages' copies of the rectangle, and this tree
/// emitted no clip at all on any of them.
/// </para>
/// <para>
/// <strong>The clip cuts the ink and keeps the words</strong>, so it is
/// <see cref="IDrawingSink.ClipPathKeepingText"/>. LibreOffice clips the output device, and its PDF
/// writer goes on emitting the text objects inside the clip: on
/// <c>044_Cash_flow_forecast_Use_this_template</c> page 4 the reference paints only the last three
/// of a chart's twelve category labels and <c>pdftotext</c> reads all twelve. So no gate column can
/// see this either way, which is why it survived.
/// </para>
/// <para>
/// It is taken only when the drawing actually leaves the block, for the same reason
/// <see cref="DrawPicture"/>'s crop clip is conditional: an unconditional one would put a
/// <c>q</c>/<c>W n</c>/<c>Q</c> pair round every picture in the corpus and change every one of
/// those renderings to no effect.
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

        // The block is a clip as well as a cull -- but the clip is taken per drawing, only where
        // one actually leaves the block, and as `ClipPathKeepingText` so the covered glyphs stay
        // in the text layer as 26.2.4.2's do. See `DrawInside` and `LeavesTheBlock`. An earlier,
        // independently written version of this rule clipped here instead: unconditionally, and
        // with `ClipPath`, which would have cut those glyphs out of the text layer and moved the
        // gate's own character count. Both survived a clean auto-merge; this one is the measured
        // one (`probes/ink-pass-r92`, 74 renderings, summed |ink| 316.40 -> 227.20).
        DrawInside(sink, columns, rows);
    }

    private void DrawInside(IDrawingSink sink, List<PlacedColumn> columns, List<PlacedRow> rows)
    {
        Dictionary<int, PlacedColumn> byColumn = [];
        foreach (PlacedColumn column in columns) byColumn.TryAdd(column.Column, column);

        Dictionary<int, PlacedRow> byRow = [];
        foreach (PlacedRow row in rows) byRow.TryAdd(row.Row, row);

        DocRect block = BlockOf(columns, rows);

        foreach (SheetDrawing drawing in sheet.Drawings.Items)
        {
            if (drawing.IsHidden) continue;

            // An object the file says does not print keeps its place in the model — it still
            // widens the printed block, exactly as a hidden one does — and puts no ink on the
            // page. See `SheetDrawing.IsPrintable` for why the two flags are not one.
            if (!drawing.IsPrintable) continue;

            bool paintsParts = false;
            foreach (SheetDrawingPart part in drawing.Parts)
            {
                if (part.HasPicture) { paintsParts = true; break; }
            }

            bool paintsPartInk = false;
            foreach (SheetDrawingPart part in drawing.Parts)
            {
                if (part.HasInk) { paintsPartInk = true; break; }
            }

            if (drawing.Image is null && drawing.Vector is null && drawing.Chart is null
                && drawing.Text is null && !drawing.HasInk
                && !paintsParts && !paintsPartInk)
            {
                continue;
            }

            if (Place(drawing, byColumn, byRow) is not { } placed) continue;

            // The anchor of a shape turned through a quarter describes it *after* the turn, so
            // the rectangle is reflected in `y = x` before anything is drawn in it. See
            // `SheetDrawing.QuarterTurnedAnchor`.
            DocRect box = drawing.QuarterTurnedAnchor ? Reflected(placed) : placed;
            if (box.Width <= Length.Zero || box.Height <= Length.Zero) continue;
            if (!ReachesTheBlock(box, block)) continue;

            bool cut = LeavesTheBlock(box, block);
            if (cut)
            {
                sink.Save();
                sink.ClipPathKeepingText(GraphicsPath.Rectangle(block));
            }

            try
            {
                DrawInside(sink, drawing, box, paintsParts, paintsPartInk);
            }
            finally
            {
                if (cut) sink.Restore();
            }
        }
    }

    /// <summary>Everything one drawing puts on the page, inside whatever clip is in force.</summary>
    private void DrawInside(
        IDrawingSink sink, SheetDrawing drawing, DocRect box, bool paintsParts, bool paintsPartInk)
    {
        // The box before anything inside it. A shape carrying a fill paints it under its own
        // text rather than instead of it, and under the cells' text rather than over it: the
        // internal layer is drawn after the strings, so a shown comment covers what it sits on
        // (`PrintDrawingLayer(SC_LAYER_INTERN)`, printfun.cxx:1713).
        //
        // Through the shape's own preset rather than the anchor's rectangle -- 438 of the
        // corpus's 644 worksheet shapes name a preset that is not `rect`. See
        // `SheetShapeInk`.
        SheetShapeInk.Draw(
            sink, box, drawing.Fill, drawing.Gradient, drawing.Texture, drawing.Stroke,
            drawing.StrokeWidth, drawing.Preset, drawing.Adjustments,
            drawing.FlipHorizontal, drawing.FlipVertical, scale);

        // The vector before the raster, since a shape carrying both means the DrawingML `svgBlip`
        // case where the raster is the fallback. `VectorImage.Draw` maps the picture's own frame
        // onto the box and clips to it — the same stretch `DrawImage` gives a raster, and not the
        // extent of the picture's ink, which would put a logo with margins outside its anchor.
        if (drawing.Vector is { } vector && !vector.Value.IsEmpty)
            DrawPicture(sink, box, drawing.Crop, vector, null, drawing.Opacity);
        else if (drawing.Image is { } image)
            DrawPicture(sink, box, drawing.Crop, null, image, drawing.Opacity);
        else if (drawing.Chart is { } chart) SheetChart.Draw(sink, chart, box, scale);

        // Not an `else`: a shape may carry both a picture and a caption, and the text goes
        // over the fill rather than instead of it.
        if (drawing.Text is { } text) SheetShapePainter.Draw(sink, text, box, scale);

        if (paintsParts || paintsPartInk) DrawParts(sink, box, drawing.Parts);
    }

    /// <summary>
    /// Draws the pictures held by the leaf shapes of a group, each in its own part of the box.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>An anchor holding an <c>xdr:grpSp</c> carries no picture of its own, so the whole
    /// group was skipped here and painted nothing.</strong> See
    /// <see cref="SheetDrawingPart.Image"/>: the parts were already read, for the bounding
    /// rectangle, and this draws them.
    /// </para>
    /// <para>
    /// A part states its rectangle as fractions of the anchor's, with the group's
    /// <c>a:chOff</c>/<c>a:chExt</c> mapping already folded in, so resolving it is arithmetic on
    /// the box. Its <c>a:xfrm/@rot</c> turns it about its own centre, which is what DrawingML means
    /// by a rotation and what <c>SheetDrawingPart.Degrees</c> was carried for.
    /// </para>
    /// </remarks>
    private void DrawParts(
        IDrawingSink sink, DocRect box, IReadOnlyList<SheetDrawingPart> parts)
    {
        foreach (SheetDrawingPart part in parts)
        {
            if (!part.HasPicture && !part.HasInk) continue;

            DocRect where = new(
                box.X + Length.FromEmu((long)Math.Round(box.Width.Emu * part.X)),
                box.Y + Length.FromEmu((long)Math.Round(box.Height.Emu * part.Y)),
                Length.FromEmu((long)Math.Round(box.Width.Emu * part.Width)),
                Length.FromEmu((long)Math.Round(box.Height.Emu * part.Height)));

            if (where.Width <= Length.Zero || where.Height <= Length.Zero) continue;

            AffineTransform? turn = Turn(where, part.Degrees);
            if (turn is { } transform)
            {
                sink.Save();
                sink.Transform(transform);
            }

            try
            {
                // The ink first and the picture over it, which is the order a leaf shape states
                // them in: a `pic` with a fill behind it is a picture on a coloured ground.
                SheetShapeInk.Draw(
                    sink, where, part.Fill, part.Gradient, null, part.Stroke, part.StrokeWidth,
                    part.Preset, part.Adjustments,
                    part.FlipHorizontal, part.FlipVertical, scale);

                if (part.HasPicture)
                    DrawPicture(sink, where, part.Crop, part.Vector, part.Image, part.Opacity);
            }
            finally
            {
                if (turn is not null) sink.Restore();
            }
        }
    }

    /// <summary>
    /// A rectangle reflected in the line <c>y = x</c> about its own centre: the same centre, with
    /// its width and height exchanged.
    /// </summary>
    /// <remarks>
    /// <c>drawingfragment.cxx</c>:325-329 states it as two additions and a swap rather than as a
    /// reflection, and the two are the same thing — <c>X + (w − h)/2</c> keeps the centre where it
    /// was. See <see cref="SheetDrawing.QuarterTurnedAnchor"/> for when it applies.
    /// </remarks>
    private static DocRect Reflected(DocRect box)
        => new(
            box.X + ((box.Width - box.Height) / 2),
            box.Y + ((box.Height - box.Width) / 2),
            box.Height,
            box.Width);

    /// <summary>A rotation about a rectangle's centre, or null when there is no turn.</summary>
    /// <remarks>
    /// Null rather than the identity so an upright picture pays neither a <c>Save</c>/<c>Restore</c>
    /// pair nor a transform in the output — the same reason <c>PageDrawing.Turn</c> does it.
    /// </remarks>
    private static AffineTransform? Turn(DocRect box, double degrees)
    {
        if (degrees == 0) return null;

        double cx = box.X.Emu + (box.Width.Emu / 2.0);
        double cy = box.Y.Emu + (box.Height.Emu / 2.0);

        return AffineTransform.Concat(
            AffineTransform.Concat(
                AffineTransform.Translation(-cx, -cy),
                AffineTransform.Rotation(degrees * Math.PI / 180.0)),
            AffineTransform.Translation(cx, cy));
    }

    /// <summary>
    /// Draws a drawing's picture into its box, cropped when the shape says so.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A crop is a larger destination plus a clip, and the clip is the half that was
    /// missing here.</strong> The surviving fraction of the picture is what fills the anchor, so
    /// the whole of it is correspondingly bigger and everything outside the anchor is cut off.
    /// Nothing on this path clipped a picture before — <c>DrawImage</c> was called bare — so the
    /// larger rectangle without the clip would not crop a logo, it would spill the whole of it
    /// across the sheet.
    /// </para>
    /// <para>
    /// The clip is taken only when there is a crop. It is a no-op otherwise for both backends,
    /// since a raster is stretched onto exactly the box and <c>VectorImage.Draw</c> clips to its
    /// destination itself; making it unconditional would put a <c>q</c>/<c>W n</c>/<c>Q</c> pair
    /// into the PDF for every picture in the corpus and change every one of those renderings for
    /// no reason at all.
    /// </para>
    /// </remarks>
    private static void DrawPicture(
        IDrawingSink sink,
        DocRect box,
        PictureCropFractions crop,
        Lazy<VectorImage>? vector,
        RasterImage? image,
        double opacity)
    {
        DocRect destination = crop.Apply(box);

        if (destination == box)
        {
            PaintPicture(sink, box, vector, image, opacity);
            return;
        }

        sink.Save();
        try
        {
            sink.ClipPath(GraphicsPath.Rectangle(box));
            PaintPicture(sink, destination, vector, image, opacity);
        }
        finally
        {
            sink.Restore();
        }
    }

    /// <summary>Puts whichever of the two pictures there is into a rectangle.</summary>
    /// <remarks>
    /// A raster takes the opacity on the draw itself, which is one <c>gs</c> and no group; a
    /// display list has to be composited as a whole, because a constant alpha applied to each of
    /// its members would show every overlap inside the picture at full strength. That is the same
    /// distinction <c>IDrawingSink.BeginTransparencyGroup</c> exists for, and the group is opened
    /// only when there is something to fade — an unconditional one would put a form XObject round
    /// every metafile in the corpus.
    /// </remarks>
    private static void PaintPicture(
        IDrawingSink sink, DocRect where, Lazy<VectorImage>? vector, RasterImage? image,
        double opacity)
    {
        if (vector is null)
        {
            if (image is not null) sink.DrawImage(image, where, opacity);
            return;
        }

        if (opacity >= 1)
        {
            vector.Value.Draw(sink, where);
            return;
        }

        sink.BeginTransparencyGroup(opacity);
        try
        {
            vector.Value.Draw(sink, where);
        }
        finally
        {
            sink.EndTransparencyGroup();
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
    private static bool ReachesTheBlock(DocRect box, DocRect block)
        // Inclusive on both edges, because Calc's is: `aRect` is a `tools::Rectangle`, whose
        // `Right()` and `Bottom()` are the last coordinates *inside* it, so a drawing whose left
        // edge sits exactly on the block's right edge still overlaps it by one unit. Measured on
        // `sheet-shape-clip.xlsx`, whose box is anchored in the first column of the second band and
        // which LibreOffice prints on both pages.
        => box.X + box.Width >= block.X && box.X <= block.X + block.Width
           && box.Y + box.Height >= block.Y && box.Y <= block.Y + block.Height;

    /// <summary>Whether any part of a placed rectangle falls outside the page's own cell block.</summary>
    /// <remarks>
    /// The test that decides whether a clip is emitted at all. Strict, so a drawing whose edge sits
    /// exactly on the block's costs nothing — the clip would remove no ink and every such rendering
    /// would change for no reason.
    /// </remarks>
    private static bool LeavesTheBlock(DocRect box, DocRect block)
        => box.X < block.X || box.Y < block.Y
           || box.X + box.Width > block.X + block.Width
           || box.Y + box.Height > block.Y + block.Height;

    /// <summary>The rectangle the page's own printed cells cover.</summary>
    /// <remarks>
    /// <c>PrePrintDrawingLayer</c>'s <c>aRect</c>, in this page's coordinates: the columns
    /// <c>mnX1</c>…<c>mnX2</c> across and the rows <c>mnY1</c>…<c>mnY2</c> down
    /// (<c>sc/source/ui/view/output3.cxx</c>:43-78). Taken as the extremes of the placed columns
    /// and rows rather than as first and last, because neither list is promised to be in order and
    /// a right-to-left sheet places its first column on the right.
    /// </remarks>
    // Two rounds wrote this rectangle independently and named it `BlockOf` and `Block`; their
    // bodies were identical, and a clean auto-merge spliced `ReachesTheBlock`'s body into this
    // one's signature. Collapsed onto the surviving implementation rather than kept twice.
    private static DocRect BlockOf(List<PlacedColumn> columns, List<PlacedRow> rows)
        => Block(columns, rows);

    /// <summary>
    /// The page's own cell block: the rectangle the drawing layer is both culled and clipped to.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>It is a clip and not only a cull, and reading it as a cull alone lets a shape run
    /// off the edge of the printed block.</strong> `PrePrintDrawingLayer` builds this rectangle
    /// and hands it to <c>SdrView::BeginDrawLayers</c> as the paint <em>region</em>
    /// (<c>sc/source/ui/view/output3.cxx</c>:41-102), so every object the drawing layer paints
    /// afterwards is clipped to it; <c>ScPrintFunc::PrintArea</c> calls the pair once per printed
    /// area (<c>printfun.cxx</c>:1641, 1651-1713).
    /// </para>
    /// <para>
    /// It shows only once ink is being painted, which is why it surfaced with round 92's BIFF
    /// fill rather than before it. <c>PC1000.xls</c>' <c>Rectangle 16</c> is 244 pt wide and
    /// starts 6 pt inside the sheet's last column, so 62 pt of it hang past the block's right
    /// edge, and on pages 2 onwards all but a 4 pt sliver of it hangs off the top. 26.2.4.2 emits
    /// the whole rectangle into the content stream and then <c>W* n</c>s it away
    /// — <c>q 55.389 552.019 681.846 23.981 re W* n</c> is that page's own clip, read out of the
    /// reference's page 2 — so a reader counting the PDF's <em>path</em> operators sees a shape
    /// the reference is not showing. Count the pixels.
    /// </para>
    /// </remarks>
    private static DocRect Block(List<PlacedColumn> columns, List<PlacedRow> rows)
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

        return new DocRect(left, top, right - left, bottom - top);
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

        // A shown comment's caption hangs off the cell it belongs to rather than off the cell its
        // VML anchor names, so it is looked up there — see `SheetDrawing.NoteCell`.
        int fromColumn = drawing.NoteCell?.Column ?? drawing.From.Column;
        int fromRow = drawing.NoteCell?.Row ?? drawing.From.Row;

        if (ColumnX(fromColumn, columns) is not { } columnX) return null;
        if (RowY(fromRow, rows) is not { } rowY) return null;

        Length x = columnX
                   + (Delta(fromColumn, drawing.From.Column, drawing.From.ColumnOffset,
                            sheet.Grid.Columns) * scale);
        Length y = rowY
                   + (Delta(fromRow, drawing.From.Row, drawing.From.RowOffset,
                            sheet.Grid.Rows) * scale);

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
    /// coordinates, addressed from the sheet's own origin rather than from the used range
    /// (<c>PrintDrawingLayer(SC_LAYER_FRONT)</c>, <c>printfun.cxx:1699</c>). So a chart anchored
    /// three rows below the last number prints, and looking its anchor up in the page's own placed
    /// rows finds nothing. What bounds it is the *printed block* — which
    /// <see cref="SheetDrawingArea"/> has already grown to cover that chart — and the class remarks
    /// say where that clip comes from.
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
    /// <summary>
    /// How far a drawing's own start sits from the cell it is placed against.
    /// </summary>
    /// <remarks>
    /// Zero plus the offset for every drawing but a comment caption, where the two cells differ and
    /// the distance between them has to be walked. Snapped per index, as <see cref="Edge"/> is and
    /// for the same reason: the placed columns and rows accumulate snapped sizes, so a distance
    /// summed any other way lands between two of them.
    /// </remarks>
    /// <param name="cell">The cell the drawing is placed against.</param>
    /// <param name="from">The cell its anchor names.</param>
    /// <param name="fromOffset">How far into that cell the anchor starts.</param>
    /// <param name="axis">The columns or the rows.</param>
    private static Length Delta(int cell, int from, Length fromOffset, SheetAxis axis)
    {
        Length offset = SheetDeviceUnits.Snap(fromOffset);
        if (cell == from) return offset;

        // A caption is anchored within a few cells of its own; anything further is a file saying
        // something this cannot mean, and walking a million rows to honour it would cost more than
        // the drawing is worth.
        const int Reach = 1024;
        if (Math.Abs((long)from - cell) > Reach) return offset;

        Length total = Length.Zero;
        if (from > cell)
        {
            for (int at = cell; at < from; at++)
                total += SheetDeviceUnits.Snap(axis.PrintedSizeAt(at));
            return total + offset;
        }

        for (int at = from; at < cell; at++)
            total += SheetDeviceUnits.Snap(axis.PrintedSizeAt(at));
        return offset - total;
    }

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
