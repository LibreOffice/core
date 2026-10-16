using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;

namespace XRay.Office.Spreadsheets.Layout;

/// <summary>One column as a page places it.</summary>
/// <param name="Column">Its zero-based index in the sheet.</param>
/// <param name="X">Its left edge on the page.</param>
/// <param name="Width">How wide it is at the print scale.</param>
internal readonly record struct PlacedColumn(int Column, Length X, Length Width)
{
    /// <summary>Its right edge.</summary>
    public Length Right => X + Width;
}

/// <summary>One row as a page places it.</summary>
/// <param name="Row">Its zero-based index in the sheet.</param>
/// <param name="Y">Its top edge on the page.</param>
/// <param name="Height">How tall it is at the print scale.</param>
internal readonly record struct PlacedRow(int Row, Length Y, Length Height)
{
    /// <summary>Its bottom edge.</summary>
    public Length Bottom => Y + Height;
}

/// <summary>The face one run of a header or footer is drawn in.</summary>
/// <remarks>
/// A band's face has three parts that arrive from two places — the workbook's own default cell
/// font, and the band's own <c>&amp;"Family,Style"</c>, <c>&amp;B</c> and <c>&amp;I</c> codes —
/// and every one of the four calls that lays a run out needs all three. Passing them as a triple
/// rather than as three parameters keeps the resolution in one place
/// (<see cref="SheetBandHeight.FaceOf"/>) rather than at each call.
/// </remarks>
/// <param name="Family">The family name, or null for the application's own default.</param>
/// <param name="Bold">Whether the bold face is wanted.</param>
/// <param name="Italic">Whether the italic face is wanted.</param>
internal readonly record struct SheetBandFace(string? Family, bool Bold, bool Italic);

/// <summary>
/// Everything a printed sheet draws that is not the text of a cell.
/// </summary>
/// <remarks>
/// <para>
/// The order is <c>ScPrintFunc</c>'s and it is not arbitrary
/// (<c>sc/source/ui/view/printfun.cxx:1679-1695</c> and <c>:2344-2404</c>): backgrounds, then
/// borders, then the cells' text, then the grid, then the row and column headings, and last the
/// single rectangle round the lot. Backgrounds are opaque, so anything drawn before them
/// vanishes; the grid is drawn <em>over</em> the text because a hairline under a glyph would
/// otherwise disappear under an opaque fill.
/// </para>
/// <para>
/// Split out from the page's own drawing rather than folded into it because none of it is cell
/// text: this walks the same placed columns and rows, but what it needs from a cell is its fill
/// and its four edges, which live beside the content tree rather than in it.
/// </para>
/// </remarks>
internal sealed class SheetPageDecoration(SheetLayout sheet, SheetPagePlacement placement)
{
    /// <summary>
    /// How far inside its cell a data bar is painted, on every edge.
    /// </summary>
    /// <remarks>
    /// Two pixels of the PDF writer's own 720 dpi device — <c>2 * nOneX</c> in
    /// <c>drawDataBars</c> — which is 4 twips. See <see cref="DrawDataBars"/> for the
    /// measurements this was checked against.
    /// </remarks>
    private static readonly Length BarInset = Length.FromPoints(0.2);

    /// <summary>
    /// How many columns past the printed block a merged cell's fill may reach.
    /// </summary>
    /// <remarks>
    /// One. <c>ScOutputData::DrawBackground</c>'s width accumulation breaks at
    /// <c>nCol &gt; mnX2 + 2</c> while adding the width of column <c>nCol - 1</c>, so the last
    /// column it can add is <c>mnX2 + 1</c>. See <see cref="DrawBackgrounds"/> for the 26.2.4.2
    /// measurement that settles it, which is what the bound rests on — a wider merge is cut here
    /// and not merely clipped by the paper.
    /// </remarks>
    private const int OverflowLimit = 1;

    /// <summary>
    /// One dash of a data bar's axis, and one gap: <c>LineInfo</c>'s 3 logic units.
    /// </summary>
    /// <remarks>
    /// <strong>The logic unit is a hundredth of a millimetre, not a twip</strong>, and the axis is
    /// what says so: <c>LineInfo(LineStyle::Dash, 1)</c> with <c>SetDashLen(3)</c> and
    /// <c>SetDistance(3)</c> comes out of 26.2.4.2's PDF of
    /// <c>sheet-cf-data-bar-negative.xlsx</c> as <c>0.02835 w</c> and a dash array of eight
    /// <c>0.08504</c>, which are exactly 1 and 3 hundredths of a millimetre. That settles
    /// <see cref="BarInset"/> too: one pixel of the writer's 720 dpi device is 2540/720 = 3.5278
    /// of the same unit, so <c>2 * nOneX</c> is 7.0556, which is 0.19999 pt.
    /// </remarks>
    private static readonly Length AxisDash = Length.FromPoints(3 * 2.54 / 100 * 72 / 25.4);

    /// <summary>The axis line's own width, which is one of those units.</summary>
    private static readonly Length AxisWidth = Length.FromPoints(2.54 / 100 * 72 / 25.4);

    /// <summary>
    /// One unit of a border's dash table: ten twips, half a point.
    /// </summary>
    /// <remarks>
    /// <c>svtools::GetLineDashing</c>'s table is in units the caller scales, and the caller is
    /// <c>CreateBorderPrimitives</c> with <c>PatternScale() * 10.0</c>
    /// (<c>svx/source/sdr/primitive2d/sdrframeborderprimitive2d.cxx</c>:599). A Calc cell's
    /// pattern scale is <c>fColScale</c>, which for a print or PDF export is the constant
    /// twips-to-1/100 mm factor 2540/1440 = 1.76389 — so one table unit is
    /// <c>10 × 1.76389</c> hundredths of a millimetre, which is ten twips exactly. The print
    /// zoom multiplies it afterwards, as it multiplies the width.
    /// </remarks>
    private static readonly Length DashUnit = Length.FromTwips(10);

    /// <summary>One centimetre, the width of the printed row headings.</summary>
    /// <remarks><c>PRINT_HEADER_WIDTH</c>, <c>sc/source/ui/inc/printfun.hxx:45</c>.</remarks>
    public static Length HeadingWidth { get; } = Length.FromTwips(567);

    /// <summary>12.8 points, the height of the printed column headings.</summary>
    public static Length HeadingHeight { get; } = Length.FromTwips(256);

    /// <summary>
    /// The colour the printed grid and the headings are drawn in: black.
    /// </summary>
    /// <remarks>
    /// Black, not the pale grey the grid has on screen. <c>ScPrintFunc::PrintPage</c> starts from
    /// <c>Color aGridColor(COL_BLACK)</c> and only replaces it when the printout is asked to use
    /// the screen's style colours, which a PDF export is not
    /// (<c>printfun.cxx:1662</c> and <c>:2340</c>). Measured: every grid line in LibreOffice's
    /// PDF of <c>sheet-decor-ods.ods</c> is written under <c>0 0 0 RG</c>.
    /// </remarks>
    private static readonly Colour FurnitureColour = Colour.Black;

    /// <summary>
    /// The pen the grid, the headings and the outer frame are drawn with.
    /// </summary>
    /// <remarks>
    /// A tenth of a point, which is what LibreOffice's PDF export writes for a hairline — the
    /// grid is drawn with <c>DrawLine</c> and no width at all, and the export's initial
    /// <c>0.1 w</c> is what it comes out as.
    /// </remarks>
    private static readonly Length HairlineWidth = Length.FromPoints(0.1);

    private readonly double _scale = placement.Scale;

    /// <summary>Paints the cells' fills.</summary>
    /// <remarks>
    /// <para>
    /// Every fill before any border and any text, rather than each cell's fill before its own
    /// border: a fill is opaque and a border runs through the centre of a shared edge, so half of
    /// every border would be painted over by the neighbour drawn after it.
    /// </para>
    /// <para>
    /// A merged block takes its fill from its origin cell and covers the whole block, which is why
    /// this asks <see cref="DecorationAt"/> rather than the formatting directly. Inside the printed
    /// block, painting the origin's colour into each covered cell's own rectangle covers the same
    /// area with the same ink as <c>ScOutputData::DrawBackground</c>'s single extended run
    /// (<c>sc/source/ui/view/output.cxx:1148-1172</c> in this tree, which is 27.2.0.0.alpha0+ and
    /// not the reference binary's source).
    /// </para>
    /// <para>
    /// <strong>What it does not cover is a merge running off the right of the block, which the
    /// reference paints past it anyway — up to a bound of its own rather than the paper's.</strong>
    /// The run's width is accumulated
    /// over the merged columns and the accumulation stops at <c>nCol &gt; mnX2 + 2</c>, where the
    /// column whose width is added is <c>nCol - 1</c> — so the fill reaches one column past the
    /// block's last and no further, however wide the merge is. <see cref="OverflowLimit"/> is that
    /// bound. Measured at 26.2.4.2 rather than read off the loop, on
    /// <c>tests/corpus/features/sheet-merge-fill-overflow.fods</c>: a 2 cm column A alone on page 1
    /// with a merge of A:D (11 cm) and one of A:C (8 cm) is painted <strong>5 cm wide in both
    /// cases</strong>, which is A + B exactly, while the unmerged control keeps its 2 cm. On page 2,
    /// where the origin is off the block to the left, the same two merges are painted B + C + D and
    /// B + C — their covered columns' own widths, which is what this already draws.
    /// </para>
    /// </remarks>
    /// <param name="columns">The columns on the page.</param>
    /// <param name="rows">The rows on the page.</param>
    /// <param name="sink">Receives the drawing commands.</param>
    public void DrawBackgrounds(
        IReadOnlyList<PlacedColumn> columns, IReadOnlyList<PlacedRow> rows, IDrawingSink sink)
    {
        SheetFormatting formatting = sheet.Formatting;
        if (formatting.IsEmpty) return;

        SheetMerges merges = sheet.Merges;
        int lastPrinted = placement.Cells.LastColumn;

        foreach (PlacedRow row in rows)
        {
            foreach (PlacedColumn column in columns)
            {
                if (DecorationAt(row.Row, column.Column).Background is not { } colour) continue;

                // The repeated print-title columns are in `columns` too, at their own band's X
                // and with their own fixed width, so only a column of the page's own block may be
                // widened — a merge beginning inside a repeat band is drawn as it always was.
                Length width = column.Width;
                if (!merges.IsEmpty
                    && column.Column >= placement.Cells.FirstColumn
                    && column.Column <= lastPrinted)
                {
                    width += Overflow(merges, row.Row, column.Column, lastPrinted);
                }

                Fill(new DocRect(column.X, row.Y, width, row.Height), colour, sink);
            }
        }
    }

    /// <summary>
    /// How much of a merge runs off the right of the printed block and is painted anyway.
    /// </summary>
    /// <remarks>
    /// Zero unless the merge's own <em>origin</em> is the position asked about and the merge reaches
    /// past <paramref name="lastPrinted"/>. The origin is what carries <c>ATTR_MERGE</c> and
    /// therefore the column count the run is extended by; a covered cell carries
    /// <c>ATTR_MERGE_FLAG</c> and no count, so a merge whose origin is off the left of the block
    /// extends nothing and each of its covered columns paints its own rectangle as before. Measured
    /// as well as read: on page 2 and page 27 of <c>TOGAF9-Tool-ConfReqts-CSQ.xls</c> the reference
    /// draws exactly the covered columns' own widths for merges that begin on an earlier page
    /// column, and widening those two costs 0.01 and 0.65 of ink.
    /// </remarks>
    /// <param name="merges">The sheet's merged blocks.</param>
    /// <param name="row">The zero-based row.</param>
    /// <param name="column">The zero-based column.</param>
    /// <param name="lastPrinted">The last column of the page's printed block.</param>
    private Length Overflow(SheetMerges merges, int row, int column, int lastPrinted)
    {
        if (merges.Covering(row, column) is not { } merge) return Length.Zero;
        if (merge.LastColumn <= lastPrinted) return Length.Zero;
        if (column != merge.FirstColumn) return Length.Zero;

        Length extra = Length.Zero;
        int limit = Math.Min(merge.LastColumn, lastPrinted + OverflowLimit);
        for (int at = column + 1; at <= limit; at++)
        {
            if (sheet.Grid.Columns.IsHidden(at)) continue;
            extra += SheetDeviceUnits.Snap(sheet.Grid.Columns.SizeAt(at)) * _scale;
        }

        return extra;
    }

    /// <summary>Paints the bars a <c>dataBar</c> conditional format draws over its cells.</summary>
    /// <remarks>
    /// <para>
    /// <c>drawDataBars</c> (<c>sc/source/ui/view/output.cxx</c>:883-953) is the whole of the
    /// geometry and it is short. The cell's rectangle is inset by two device pixels on every
    /// edge; the axis sits at <see cref="SheetDataBar.Zero"/> per cent of what is left; and the
    /// bar runs from there to <see cref="SheetDataBar.Length"/> per cent of the remaining width
    /// on whichever side its sign names. **A length of zero returns before painting anything**,
    /// which is why a cell sitting exactly on an automatic minimum shows no bar at all rather
    /// than a hairline.
    /// </para>
    /// <para>
    /// <strong>The inset is a device pixel and the device is the PDF writer's, at 720 dpi</strong>
    /// — <c>vcl::PDFWriter</c>'s reference resolution, so <c>PixelToLogic(Size(1,1))</c> is two
    /// twips and <c>2 * nOneX</c> is <see cref="BarInset"/>. Measured rather than derived from
    /// that: on 26.2.4.2's own PDF of <c>088_To-do_list_with_progress_tracker</c> the full-length
    /// bar spans 518.54-600.55 inside a 518.35-600.75 cell and stands 20.62 pt in a 21.01 pt row,
    /// which is 0.195 pt a side, and on
    /// <c>tests/corpus/features/sheet-cf-data-bar-auto.xlsx</c> the rows give 0.21. The residual
    /// against 0.2 is under a fortieth of a point.
    /// </para>
    /// <para>
    /// The axis is drawn across the <em>whole</em> cell and not across the inset rectangle —
    /// <c>Point aPoint1(nPosZero, rRect.Top())</c> — which the negative fixture shows directly:
    /// its first row's bar stands y 71.0-85.49 and its axis runs 70.8-85.69.
    /// </para>
    /// </remarks>
    /// <param name="columns">The columns on the page.</param>
    /// <param name="rows">The rows on the page.</param>
    /// <param name="sink">Receives the drawing commands.</param>
    public void DrawDataBars(
        IReadOnlyList<PlacedColumn> columns, IReadOnlyList<PlacedRow> rows, IDrawingSink sink)
    {
        ArgumentNullException.ThrowIfNull(columns);
        ArgumentNullException.ThrowIfNull(rows);

        SheetFormatting formatting = sheet.Formatting;
        if (formatting.IsEmpty) return;

        foreach (PlacedRow row in rows)
        {
            foreach (PlacedColumn column in columns)
            {
                if (formatting.BarAt(row.Row, column.Column) is not { } bar) continue;
                if (bar.Length == 0) continue;

                Length left = column.X + BarInset;
                Length right = column.Right - BarInset;
                Length top = row.Y + BarInset;
                Length bottom = row.Bottom - BarInset;
                if (right <= left || bottom <= top) continue;

                Length span = right - left;
                Length zero = left + (span * (bar.Zero / 100.0));

                Length from;
                Length to;
                if (bar.Length < 0)
                {
                    to = zero;
                    from = zero + ((zero - left) * (bar.Length / 100.0));
                }
                else
                {
                    from = zero;
                    to = zero + ((right - zero) * (bar.Length / 100.0));
                }

                if (to > from) Fill(new DocRect(from, top, to - from, bottom - top), bar.Colour, sink);

                if (bar.HasAxis) AxisLine(zero, row, bar.AxisColour, sink);
            }
        }
    }

    /// <summary>Paints the icons an <c>iconSet</c> conditional format draws over its cells.</summary>
    /// <remarks>
    /// <para>
    /// <c>drawIconSets</c> (<c>sc/source/ui/view/output.cxx</c>:960-989) is four statements, and
    /// three of them are geometry:
    /// </para>
    /// <list type="bullet">
    ///   <item><description>
    ///     <strong>The icon is as tall as the cell's own font, not a fixed size.</strong> The ten
    ///     points at <c>:967</c> is a fallback for a null <c>mnHeight</c>, and
    ///     <c>GetIconSetInfo</c> <em>always</em> sets that field, from the cell's
    ///     <c>ATTR_FONT_HEIGHT</c> (<c>sc/source/core/data/colorscale.cxx</c>:1222-1224), so the
    ///     branch at <c>:969-980</c> always wins. Measured on 26.2.4.2's own renderings of the
    ///     corpus, the per-icon area runs from 19.4 pt² on <c>066_Agile_Gantt_chart</c> to
    ///     103.9 pt² on <c>069_Blue_modern_balance_sheet</c> — a range of more than five to one,
    ///     where a constant ten points would be 100 pt² throughout.
    ///   </description></item>
    ///   <item><description>
    ///     <strong>It is square</strong>, because its width is the bitmap's own aspect ratio times
    ///     that height (<c>:982-984</c>) and every icon-set asset the reference ships is 16 × 16.
    ///   </description></item>
    ///   <item><description>
    ///     <strong>It sits in the bottom-left corner, two device pixels in on both axes</strong> —
    ///     <c>Point(rRect.Left() + 2 * nOneX, rRect.Bottom() - 2 * nOneY - aHeight)</c> at
    ///     <c>:988</c> — the same inset as a data bar's, which is why this shares
    ///     <see cref="BarInset"/> rather than restating it. It is clipped to the cell
    ///     (<c>:987</c>), so an icon taller than its row is cut rather than spilling.
    ///   </description></item>
    /// </list>
    /// <para>
    /// The font height is the cell's <em>stated</em> one: <c>GetIconSetInfo</c> reads
    /// <c>mrDoc.GetPattern(rAddr)</c>, the cell's own attribute pattern, which a conditional
    /// format's <c>dxf</c> does not reach — Calc applies a condition's font as a style at fill
    /// time and not to the pattern the icon measures itself against. It scales with the print
    /// scale because <c>aHeight</c> is a logic length in a mapped-out output device.
    /// </para>
    /// </remarks>
    /// <param name="columns">The columns on the page.</param>
    /// <param name="rows">The rows on the page.</param>
    /// <param name="sink">Receives the drawing commands.</param>
    public void DrawIconSets(
        IReadOnlyList<PlacedColumn> columns, IReadOnlyList<PlacedRow> rows, IDrawingSink sink)
    {
        ArgumentNullException.ThrowIfNull(columns);
        ArgumentNullException.ThrowIfNull(rows);
        ArgumentNullException.ThrowIfNull(sink);

        SheetFormatting formatting = sheet.Formatting;
        if (formatting.IsEmpty) return;

        foreach (PlacedRow row in rows)
        {
            foreach (PlacedColumn column in columns)
            {
                if (formatting.IconAt(row.Row, column.Column) is not { } icon) continue;
                if (icon.Glyph == SheetIconGlyph.Unpainted) continue;

                // The stated size times the print scale, and no device-pixel round trip:
                // `drawIconSets` converts `mnHeight` straight from twips to hundredths of a
                // millimetre (`:977`) and hands it to `DrawBitmap`, so unlike a *face* it is
                // never selected at whole pixels. The conversion's own truncation is under a
                // thousandth of a millimetre and below anything measurable in a PDF.
                Length side = sheet.Formats.At(row.Row, column.Column).FontSize * _scale;
                if (side <= Length.Zero) continue;

                DocRect box = new(
                    column.X + BarInset, row.Bottom - BarInset - side, side, side);

                sink.Save();
                sink.ClipPath(GraphicsPath.Rectangle(
                    new DocRect(column.X, row.Y, column.Width, row.Height)));
                SheetIconArtwork.Draw(sink, box, icon.Glyph);
                sink.Restore();
            }
        }
    }

    /// <summary>The dashed vertical a data bar draws at its zero, when it has one inside the cell.</summary>
    /// <remarks>
    /// <c>LineInfo(LineStyle::Dash, 1)</c> with four dashes of three logic units and three
    /// between them, over the cell's own full height. **No corpus rule reaches it** — all nine
    /// resolve a minimum at or above zero, so their zero sits on the left edge and
    /// <c>drawDataBars</c> returns before the axis — so the only witness is
    /// <c>tests/corpus/features/sheet-cf-data-bar-negative.xlsx</c>, on which 26.2.4.2 draws four
    /// of them, one per row that has a bar and none on the row whose length is zero.
    /// </remarks>
    private static void AxisLine(Length at, PlacedRow row, Colour colour, IDrawingSink sink)
    {
        sink.StrokePath(
            new GraphicsPath().MoveTo(new DocPoint(at, row.Y)).LineTo(new DocPoint(at, row.Bottom)),
            new Stroke(
                Paint.Solid(colour),
                AxisWidth,
                LineCap.Butt,
                LineJoin.Round,
                // Four dashes and four gaps rather than one of each, because `SetDashCount(4)`
                // is what the reference states and its PDF carries all eight entries. The two
                // spellings paint the same line; this one matches operator for operator.
                DashPattern: [
                    AxisDash, AxisDash, AxisDash, AxisDash,
                    AxisDash, AxisDash, AxisDash, AxisDash,
                ]));
    }

    /// <summary>
    /// The decoration a position draws with: its own, or its merged block's origin's.
    /// </summary>
    /// <remarks>
    /// A covered cell's own fill and borders are never drawn — measured on
    /// <c>probes/sheets-r37/merge-decor.fods</c>, whose covered cells state a green fill and a
    /// magenta box and whose reference PDF holds neither colour anywhere. See
    /// <see cref="SheetMerges.OriginOf"/> for the mechanism.
    /// </remarks>
    private SheetCellDecoration DecorationAt(int row, int column)
    {
        SheetMerges merges = sheet.Merges;
        if (merges.IsEmpty) return sheet.Formatting.At(row, column);

        (int originRow, int originColumn) = merges.OriginOf(row, column);
        return sheet.Formatting.At(originRow, originColumn);
    }

    /// <summary>
    /// Draws the cells' borders, coalesced into one stroke per run of identically styled edges.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>Array::CreateB2DPrimitiveRange</c> emits one primitive for each cell's top and left edge,
    /// and the bottom and right only for the last row and column
    /// (<c>svx/source/dialog/framelinkarray.cxx:1490-1537</c>) — but that is not what reaches the
    /// page. <c>SdrFrameBorderPrimitive2D::create2DDecomposition</c>
    /// (<c>svx/source/sdr/primitive2d/sdrframeborderprimitive2d.cxx:782-841</c>) decomposes them
    /// all and then folds each new segment into any already-emitted one it can, through
    /// <c>tryMergeBorderLinePrimitive2D</c>
    /// (<c>drawinglayer/source/primitive2d/borderlineprimitive2d.cxx:300-417</c>). So a grid line
    /// arrives as **one** stroke per maximal run of identically styled cell edges.
    /// </para>
    /// <para>
    /// **Measured, not inferred.** <c>tests/corpus/features/sheet-border-runs.fods</c> is fourteen
    /// four-cell runs each varying one property, rendered by LibreOffice 26.2.4.2 itself; the
    /// stroke census is in <c>probes/sheets-d-01/results.md</c>. Four identically styled cells give
    /// one stroke spanning all four. A change of colour, of width, of pattern, or of the number of
    /// sub-lines gives two. A cell that states nothing gives two, and they do not bridge the hole.
    /// A perpendicular border crossing an interior joint does <em>not</em> break the run. A hidden
    /// row or column does not break it either — Calc's array holds visible lines only.
    /// </para>
    /// <para>
    /// The correction this supersedes, recorded because the measurement behind it was real and
    /// the conclusion drawn from it was not: <c>sheet-decor-ods.ods</c>'s B4 box does arrive as
    /// four separate <c>m … l S</c> pairs, and that says nothing about coalescing, because a
    /// cell's four edges run in four directions and <c>tryMergeBorderLinePrimitive2D</c> refuses a
    /// non-zero cross product by construction. Calc merges collinear runs and only those.
    /// </para>
    /// <para>
    /// A run may not cross a band. <c>ScPrintFunc::PrintPage</c> makes up to four separate
    /// <c>PrintArea</c> calls per page — the repeated corner, the repeated column band, the
    /// repeated row band and the data (<c>sc/source/ui/view/printfun.cxx:2303-2335</c>) — each
    /// with its own <c>ScOutputData</c>, hence its own array and its own primitive group, so two
    /// segments from different bands can never meet in the merge loop.
    /// </para>
    /// <para>
    /// Which border a shared edge gets is <see cref="SheetCellBorders.Resolve"/>: the heavier of
    /// the two cells' facing edges. Both neighbours are consulted even at the page's own edges,
    /// because Calc's border array is built one column and one row wider than the page on every
    /// side (<c>ScDocument::FillInfo</c> loops <c>nCol1-1</c> to <c>nCol2+1</c>,
    /// <c>sc/source/core/data/fillinfo.cxx:1019</c>) and sets no clipping range when printing.
    /// </para>
    /// <para>
    /// A merged block contributes its <em>origin's</em> four edges and nothing else: every style
    /// read here goes through <see cref="DecorationAt"/>, and an edge interior to a block is not
    /// drawn at all. <c>Array::GetCellStyleTop</c> and its three siblings return
    /// <c>OBJ_STYLE_NONE</c> when <c>IsMergedOverlapped*</c> and otherwise read from
    /// <c>GetMergedStyleSourceCell</c> (<c>svx/source/dialog/framelinkarray.cxx:782-856</c>).
    /// A block three rows tall still emits its left edge <em>per row</em>, all three carrying the
    /// origin's style, because only <c>mbOverlapX</c> suppresses a left edge — and the three are
    /// now folded back into one line, which is what the reference draws.
    /// </para>
    /// </remarks>
    /// <param name="columns">The columns on the page.</param>
    /// <param name="rows">The rows on the page.</param>
    /// <param name="sink">Receives the drawing commands.</param>
    public void DrawBorders(
        IReadOnlyList<PlacedColumn> columns, IReadOnlyList<PlacedRow> rows, IDrawingSink sink)
    {
        SheetFormatting formatting = sheet.Formatting;
        if (formatting.IsEmpty || columns.Count == 0 || rows.Count == 0) return;

        Edges edges = Edges.Build(
            DecorationAt,
            sheet.Merges,
            columns,
            rows,
            placement.RepeatColumns is { IsValid: true } rc ? rc.LastColumn : int.MinValue,
            placement.RepeatRows is { IsValid: true } rr ? rr.LastRow : int.MinValue);

        foreach (Edge edge in edges.All) Stroke(edge, edges, sink);
    }

    /// <summary>
    /// Draws the faint rules between cells, when the sheet prints them.
    /// </summary>
    /// <remarks>
    /// <para>
    /// At the <em>far</em> edge of each column and row rather than the near one, which is what
    /// <c>ScOutputData::DrawGrid</c> does: it advances the pen by the column's width and then
    /// draws (<c>sc/source/ui/view/output.cxx:420-424</c>), so there is no line down the left of
    /// the first column and none along the top of the first row. Measured on
    /// <c>sheet-decor-ods.ods</c>, whose three columns start at 85.039 pt: the verticals are at
    /// 148.904, 212.882 and 276.86 and there is none at 85.039. The block's own left and top
    /// edges come from the outer frame instead, which is why the two are drawn together.
    /// </para>
    /// <para>
    /// The grid stops at a merged block, in both directions: <c>ScOutputData::DrawGrid</c> skips a
    /// vertical wherever the cell to its right is <c>bHOverlapped</c> and a horizontal wherever the
    /// cell below is <c>bVOverlapped</c>. Measured on <c>probes/sheets-r37/merge-grid.fods</c>,
    /// whose interior vertical is drawn above and below its 2 × 3 block and not through it, and
    /// whose interior horizontals run to the block's left and right only.
    /// </para>
    /// </remarks>
    /// <param name="columns">The columns on the page.</param>
    /// <param name="rows">The rows on the page.</param>
    /// <param name="sink">Receives the drawing commands.</param>
    public void DrawGrid(
        IReadOnlyList<PlacedColumn> columns, IReadOnlyList<PlacedRow> rows, IDrawingSink sink)
    {
        if (!sheet.Setup.PrintsGrid || columns.Count == 0 || rows.Count == 0) return;

        SheetMerges merges = sheet.Merges;

        // Runs rather than one line per column, because a merged block interrupts a rule and the
        // two ends of it are one stroke each. With no merge on the sheet — nearly every sheet —
        // the run is the whole block and this is the single line the unmerged case always drew.
        foreach (PlacedColumn column in columns)
        {
            Run(
                rows.Count,
                i => merges.IsOverlappedLeft(rows[i].Row, column.Column + 1),
                i => rows[i].Y,
                i => rows[i].Bottom,
                (from, to) => Rule(column.Right, from, column.Right, to, sink));
        }

        foreach (PlacedRow row in rows)
        {
            Run(
                columns.Count,
                i => merges.IsOverlappedTop(row.Row + 1, columns[i].Column),
                i => columns[i].X,
                i => columns[i].Right,
                (from, to) => Rule(from, row.Bottom, to, row.Bottom, sink));
        }
    }

    /// <summary>Emits one line per maximal run of positions the grid is not suppressed at.</summary>
    private static void Run(
        int count,
        Func<int, bool> suppressed,
        Func<int, Length> start,
        Func<int, Length> end,
        Action<Length, Length> emit)
    {
        int from = -1;

        for (int i = 0; i < count; i++)
        {
            if (suppressed(i))
            {
                if (from >= 0) emit(start(from), end(i - 1));
                from = -1;
                continue;
            }

            if (from < 0) from = i;
        }

        if (from >= 0) emit(start(from), end(count - 1));
    }

    /// <summary>
    /// Draws the row and column headings, and the frame round the printed block.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The headings are a boxed cell each, holding the column's letter or the row's number
    /// centred both ways (<c>ScPrintFunc::PrintColHdr</c> and <c>PrintRowHdr</c>,
    /// <c>printfun.cxx:1417</c> and <c>:1466</c>). The strip is one centimetre wide and
    /// 12.8 points tall, taken off the printable area before any cell is placed.
    /// </para>
    /// <para>
    /// The frame is drawn whenever <em>either</em> the grid or the headings print, not only with
    /// the headings — <c>if (bDoPrint &amp;&amp; (aTableParam.bGrid || aTableParam.bHeaders))</c>
    /// (<c>printfun.cxx:2384</c>) — and it encloses the headings as well as the cells. Measured
    /// on <c>sheet-decor-ods.ods</c>: one rectangle from 56.58 pt to 276.889 pt across, which is
    /// the row headings' left edge to the last column's right edge.
    /// </para>
    /// </remarks>
    /// <param name="origin">Where the block starts, headings included.</param>
    /// <param name="columns">The columns on the page.</param>
    /// <param name="rows">The rows on the page.</param>
    /// <param name="sink">Receives the drawing commands.</param>
    public void DrawHeadings(
        DocPoint origin,
        IReadOnlyList<PlacedColumn> columns,
        IReadOnlyList<PlacedRow> rows,
        IDrawingSink sink)
    {
        SheetPrintSetup setup = sheet.Setup;
        if (!setup.PrintsHeadings && !setup.PrintsGrid) return;
        if (columns.Count == 0 || rows.Count == 0) return;

        Length headingWidth = setup.PrintsHeadings ? HeadingWidth * _scale : Length.Zero;
        Length headingHeight = setup.PrintsHeadings ? HeadingHeight * _scale : Length.Zero;

        if (setup.PrintsHeadings)
        {
            Length top = origin.Y;
            foreach (PlacedColumn column in columns)
            {
                Box(new DocRect(column.X, top, column.Width, headingHeight),
                    ColumnName(column.Column), _scale, sink);
            }

            Length left = origin.X;
            foreach (PlacedRow row in rows)
            {
                Box(new DocRect(left, row.Y, headingWidth, row.Height),
                    (row.Row + 1).ToString(System.Globalization.CultureInfo.CurrentCulture),
                    _scale, sink);
            }
        }

        DocRect frame = new(
            origin.X,
            origin.Y,
            columns[^1].Right - origin.X,
            rows[^1].Bottom - origin.Y);

        Outline(frame, sink);
    }

    /// <summary>
    /// Draws the header and the footer: three pieces of text in one band each.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The band runs from the page's left margin plus the band's own left margin to the mirror of
    /// that on the right (<c>ScPrintFunc::PrintHF</c>, <c>printfun.cxx:1800-1802</c>), its text
    /// occupies the top <c>height - gap</c> of it, and each of the three parts is drawn into that
    /// same rectangle — left-aligned, centred and right-aligned. They overlap when they are long,
    /// which is what LibreOffice shows and why this is three runs rather than one line with tab
    /// stops.
    /// </para>
    /// <para>
    /// Vertically the text is centred in what is left: <c>aDraw.Y += (paperHeight - textHeight)
    /// / 2</c>. Measured on <c>sheet-decor-ods.ods</c>, whose header band gives its text
    /// 14.099 pt and whose single line measures 11.1 pt, that puts the baseline 10.55 pt below
    /// the top margin — 1.5 pt of centring plus a 9.05 pt ascent.
    /// </para>
    /// </remarks>
    /// <param name="context">What the fields resolve to on this page.</param>
    /// <param name="sink">Receives the drawing commands.</param>
    public void DrawHeaderAndFooter(SheetHeaderContext context, IDrawingSink sink)
    {
        SheetPrintSetup setup = sheet.Setup;
        DocSize page = setup.PageSize;

        // **A band of no height at all draws nothing, and the claim that stood here said the
        // opposite.** It read: a zero-height VCL rectangle has no bottom edge, so `PrintHF`'s
        // clip is unbounded and a zero band suppresses the space and not the ink — measured on
        // `2012-GA-Survey-Chapter-6-Tables-16Dec2013-V2.xls` against LibreOffice **24.2.7.2**,
        // which drew `Page 6 - 2` at y 575.95.
        //
        // Re-measured on **26.2.4.2**, the binary this tree is scored against, that same
        // document's reference PDF contains **no footer at all** and ours contained four
        // `Page 6 - N` lines — twelve words the reference does not have, on a document passing by
        // 0.48 of its band. A second, independent measurement agrees: six authored margin
        // variants of `020_Free_Blood_Pressure_Chart…xlsx` differing only in `bottom` and
        // `footer`, where the reference draws the footer at every stated band above zero and
        // draws nothing at a stated band of exactly zero.
        //
        // So the guard below is the *measured* behaviour of the reference binary rather than a
        // reading of the C++, and it is a reminder that a stored figure is evidence about an
        // environment: the mechanism the old note identified survived, the number attached to it
        // did not.
        //
        // [24.2.7-audit: FIXED 2026-08-21, round sheets-r56 — was WRONG (round 55): the
        // zero-band half was right, the "above zero" half was wrong, and the rule that replaces
        // it is implemented below rather than reported.] Round 55 re-checked this on 26.2.4.2
        // with `probes/sheets-r55/audit_pagedecoration.py`, found the reference drew nothing at
        // 0.72 or 1.44 pt of 8 pt text, and recorded a "text-fit threshold" scaling at about
        // 0.27x the point size.
        //
        // **There is no threshold. There is a clip rectangle**, and the apparent threshold is
        // just how far below a line's top its ink starts. See `DrawBand`, which carries the
        // citations and the bisection that measures it — and which reproduces round 55's own 8 pt
        // bracket with nothing fitted, while refuting its 20 pt one (4.32 pt draws).
        //
        // What survives unchanged: a band of exactly zero draws nothing, and so does every
        // negative band. Both now fall out of the rectangle rather than being asserted.
        //
        // `differentFirst` swaps the pair on the sheet's first page, and swapping it to *nothing*
        // is the case the corpus holds: every workbook here that sets the flag supplies no
        // first-page content, so the first page prints bare. The band is unchanged either way —
        // Calc reserves max(odd, even, first) on every page — so nothing below moves.
        bool first = setup.DifferentFirstPage && context.IsFirstPageOfSheet;
        SheetHeaderFooter? bandHeader = first ? setup.FirstHeader : setup.Header;
        SheetHeaderFooter? bandFooter = first ? setup.FirstFooter : setup.Footer;

        if (bandHeader is { IsEmpty: false } header && setup.HeaderHeight > Length.Zero)
        {
            DrawBand(
                header,
                context,
                setup.LeftMargin + setup.HeaderLeftMargin,
                page.Width - setup.RightMargin - setup.HeaderRightMargin,
                setup.TopMargin,
                setup.HeaderHeight - setup.HeaderGap,
                setup.TopMargin,
                setup.HeaderIsDynamic,
                false,
                setup.BandFont ?? SheetDefaultFont.Calc,
                sink);
        }

        if (bandFooter is { IsEmpty: false } footer && setup.FooterHeight > Length.Zero)
        {
            // The footer's gap sits at the *top* of its band, between the last row and the text,
            // so the text starts that far below the band's top rather than at it.
            DrawBand(
                footer,
                context,
                setup.LeftMargin + setup.FooterLeftMargin,
                page.Width - setup.RightMargin - setup.FooterRightMargin,
                page.Height - setup.BottomMargin - setup.FooterHeight + setup.FooterGap,
                setup.FooterHeight - setup.FooterGap,

                // The band's own top edge, which is the gap-free figure and not the text
                // rectangle's. The two differ by exactly the gap, and the clamp inside `DrawBand`
                // has to be against the band — a gap that is not the filter's own `nDistance`
                // would otherwise move a footer that was already right.
                page.Height - setup.BottomMargin - setup.FooterHeight,
                setup.FooterIsDynamic,
                true,
                setup.BandFont ?? SheetDefaultFont.Calc,
                sink);
        }
    }

    /// <summary>
    /// Lays one band's three areas into the rectangle the band gives them, and clips.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>The rectangle is the whole of the rule that decides whether a short band draws
    /// anything.</strong> <c>ScPrintFunc::PrintHF</c> sets a clip region of exactly
    /// <c>Rectangle(aStart, Size(nLineWidth, nHeight - nDistance))</c> before it draws the three
    /// areas (<c>sc/source/ui/view/printfun.cxx:1870</c>) — which is
    /// <paramref name="left"/>..<paramref name="right"/> by <paramref name="top"/> plus
    /// <paramref name="height"/>, the two figures this method is already given. Each area then
    /// goes through <c>ImpEditEngine::DrawText_ToPosition</c>, which takes the area's whole
    /// primitive range and <em>returns having emitted nothing at all</em> when that range does
    /// not meet the clip (<c>editeng/source/editeng/impedit3.cxx:3367-3379</c>). When the two
    /// overlap only partly it wraps the area in a <c>MaskPrimitive2D</c> of the clip polygon
    /// (<c>:3380-3389</c>), which keeps every line in the primitive tree and <em>cuts</em> all of
    /// them when they are rendered. So the decision to draw at all is per area and never per
    /// line, and the cut that follows it is geometric and lands wherever the band's edge is.
    /// </para>
    /// <para>
    /// A band is only ever shorter than its text when the file's own margins make it so, and both
    /// Excel readers already model that: <c>convertHeaderFooterData</c> sets
    /// <c>mbDynamicHeight = false</c> and pins <c>mnHeight</c> at the stated band when
    /// <c>statedBand - textHeight</c> is negative (<c>pagesettings.cxx:1030-1041</c>), and
    /// <c>UpdateHFHeight</c> returns before it can grow a pinned band
    /// (<c>printfun.cxx:789-793</c>). See <see cref="SheetBandHeight"/>, which is that port.
    /// </para>
    /// <para>
    /// <strong>Measured, and it replaces a rule that was fitted.</strong>
    /// <c>probes/sheets-r56/probe-bandclip.py</c> bisects the band at which 26.2.4.2 begins to
    /// draw, in 0.1 pt steps: 8 pt text turns over between 1.59 and 1.70 pt, 11 pt between 2.21
    /// and 2.30, 20 pt between 4.11 and 4.20. Those are not a threshold — they are
    /// <c>ascent - inkAscent</c>, the distance from a line's top to the top of its ink, and
    /// <see cref="SheetBandText.CapHeightAt(Length, string?)"/> carries that measurement. The same probe's decisive
    /// case is one a threshold could never have produced:
    /// <c>FAA-2019-0995-0002_attachment_2</c>'s <c>ACC list</c> sheet has a
    /// <strong>5.67 pt</strong> band — comfortably above every bracket — whose header is
    /// <c>&amp;R</c>, seven empty lines, then <c>PAGE </c> and <c>&amp;P OF &amp;N</c> at 9 pt.
    /// The ink lands 90 pt below the band, the reference draws none of it, and we drew twenty
    /// tokens across five pages that the reference does not have.
    /// </para>
    /// </remarks>
    /// <param name="band">The three areas and their codes.</param>
    /// <param name="context">What the fields resolve to on this page.</param>
    /// <param name="left">The band's left edge.</param>
    /// <param name="right">The band's right edge.</param>
    /// <param name="top">The top of the band's <em>text</em> rectangle, which is Calc's <c>aStart.Y</c>.</param>
    /// <param name="height">That rectangle's height, which is Calc's <c>nHeight - nDistance</c>.</param>
    /// <param name="bandTopEdge">The band's own top edge, gap included.</param>
    /// <param name="dynamic">Whether the band grew to fit its text.</param>
    /// <param name="fromBottom">Whether the band hangs from the page's bottom, as a footer does.</param>
    /// <param name="bandFont">The face an area's pieces are drawn in when they name none.</param>
    /// <param name="sink">Receives the drawing commands.</param>
    private void DrawBand(
        SheetHeaderFooter band,
        SheetHeaderContext context,
        Length left,
        Length right,
        Length top,
        Length height,
        Length bandTopEdge,
        bool dynamic,
        bool fromBottom,
        SheetDefaultFont bandFont,
        IDrawingSink sink)
    {
        if (right <= left || height < Length.Zero) return;

        // The band is drawn at the page's own zoom. `ScPrintFunc::PrintHF` switches the device to
        // `aTwipMode`, which carries the zoom as its scale fraction
        // (`InitModes`, sc/source/ui/view/printfun.cxx:2645), so a header on a sheet printed at
        // 33% is drawn at a third of its stated size along with everything else.
        double zoom = placement.Scale;

        // The three parts share one band and are each centred in it, which is why the band's own
        // height is the tallest of the three rather than each part's own: `PrintHF` gives the
        // EditEngine one `aPaperSize` and computes `nDif` per area against it
        // (sc/source/ui/view/printfun.cxx:1876-1912), and `UpdateHFHeight` has set that height to
        // the greatest of the three (`:820-834`). Measured on `sheet-outline-collapse.xlsx`,
        // whose footer holds two 8 pt lines on the left and one on the right: LibreOffice puts
        // the left part's last line hard against the footer margin and the right part's single
        // line 3.35 pt above it, which is half the difference between the two.
        Length bandText = Length.Zero;
        foreach (SheetHeaderPart part in (SheetHeaderPart[])[band.Left, band.Centre, band.Right])
            bandText = Length.Max(
                bandText, SheetBandHeight.PartHeight(part, context, zoom, bandFont));

        if (bandText <= Length.Zero) return;

        // What the three areas are centred *in*, which is `PrintHF`'s `aPaperSize.Height()` and
        // never the text's own height: `nDif = paperHeight - textHeight`, applied only when it is
        // positive (printfun.cxx:1876-1912). For a band that fits, `UpdateHFHeight` has already
        // made the two equal, which is why a short area is lifted half the difference between
        // itself and the tallest of the three — measured on `sheet-outline-collapse.xlsx`.
        //
        // **The `Min` is what makes a pinned band behave**, and it is measured rather than
        // reasoned. `HeaderIsDynamic` is set true for every SpreadsheetML band, correctly, because
        // Calc's own flag is about where the text sits rather than about whether the band grew;
        // but on a band Calc has *pinned* the paper height is the stated band and not the text, so
        // `nDif` is negative and nothing is centred at all. Without this clamp a short area in a
        // pinned band is pushed down by half the tallest area's overhang.
        // `tests/corpus/features/sheet-band-clip-xlsx.xlsx` sheet `Areas` is that case: a 7.2 pt
        // band whose right area is eight lines and whose left is one. 26.2.4.2 puts `KEEPLEFT`'s
        // ink at y 21.576 against a band top of 21.60; centring it in the right area's 124 pt put
        // it 54 pt lower, and then the clip below deleted it.
        //
        // **All three of those figures have to be in one space and two of them were not.**
        // `bandText` is the text at the print scale — `SheetBandHeight.SizeOf` multiplies every
        // em size by the zoom — while `height` is the band as the file states it, unscaled. Calc has no such
        // split: `PrintHF` does its whole arithmetic in *logical* twips and lets the map mode
        // apply the zoom to all of it at once (`printfun.cxx:1867`, `:2645`), so `nDif` is
        // `(paperHeight - textHeight)` logical and reaches the paper as
        // `(paperHeight × zoom - textHeight × zoom) / 2`. Comparing a scaled text against an
        // unscaled band overstates <c>nDif</c> by `height × (1 - zoom)`, which is nothing at all
        // on an unscaled sheet and 12.3 pt on a sheet printed at 35 %.
        //
        // Measured on `sheets/done-014/xls/TICAPCapability_Final.xls`, whose sheets print at 57 %
        // and 35 %: at 35 % the band is 30.16 pt stated, the text 7.82 pt drawn, and the mixed
        // arithmetic put the header's pen 6.17 pt below the band's top — far enough that its ink
        // began at 47.9 against a band ending at 46.6, so the whole area missed the clip
        // rectangle and six pages lost their header and footer outright. The reference draws that
        // header hard against the band's top.
        Length scaled = height * zoom;
        Length drawn = dynamic ? Length.Min(bandText, scaled) : scaled;

        // The text rectangle's top, which is `PrintHF`'s `aStart`. For a footer that is the band's
        // own bottom edge — the one figure the scaling leaves fixed — less the rectangle; a
        // dynamic band measures that rectangle by its own text, which is the figure this line has
        // always carried and which `SheetBandLinesTests` and `SheetSmallBandTests` pin, and a
        // pinned one measures it by the stated band at the print scale.
        //
        // On an unscaled sheet `scaled` is `height`, so the pinned arm is `top` — exactly what
        // stood here — and nothing without a print scale can move.
        Length bandTop = fromBottom
            ? top + height - (dynamic ? bandText : scaled)
            : top;

        // A footer sits on its own margin line, but **never above the top of its own band**:
        // `PrintHF` offsets the text by `nDif = paperHeight - textHeight` and only when that is
        // positive (`sc/source/ui/view/printfun.cxx:1876-1912`), so a band shorter than its own
        // text starts at the top and overflows downwards rather than being lifted clear.
        //
        // Measured on two authored variants of `020_Free_Blood_Pressure_Chart…xlsx`: at a stated
        // band of 3.6 pt the reference's first footer glyph tops out at 770.37 pt against a band
        // top of 770.40, and at 7.2 pt at 766.77 against 766.80. Bottom-aligning them instead put
        // our text 7.5 pt and 3.9 pt low.
        //
        // Clamping against `top` rather than the band's own edge is the version of this that does
        // not work, and it is worth saying why. `top` is the *text rectangle's* top, which is the
        // band's top plus the gap — and `height` is the rectangle, not the band. A reader whose
        // gap is not the filter's own `nDistance` therefore has `height < bandText` on bands that
        // fit perfectly well, and clamping against `top` moves them. One probe caught it: a
        // 14.4 pt band went from 0.53 pt of the reference to 2.72 pt out. Against the band edge
        // the clamp fires only when the text genuinely does not fit inside the band.
        //
        // The edge is the band's own top at the print scale, anchored — like everything else in a
        // footer — on the bottom edge the scaling leaves fixed. `bandTopEdge` is stated unscaled,
        // and the whole band below it, the gap included, is scaled: `top + height` is that bottom
        // edge and `height + top - bandTopEdge` is the band with its gap. On an unscaled sheet
        // this is `bandTopEdge` exactly, whatever the band is doing.
        Length bandEdge = (top + height) - ((height + top - bandTopEdge) * zoom);
        if (fromBottom && bandTop < bandEdge) bandTop = bandEdge;

        // `PrintHF`'s own clip rectangle, which every one of the three areas is drawn through:
        // `Rectangle(aStart, Size(nLineWidth, nHeight - nDistance))`
        // (`sc/source/ui/view/printfun.cxx:1870`).
        //
        // **It is as tall as the band at the print scale and as wide as the band at full size**,
        // which is the same asymmetry <see cref="SheetPrintSetup.PrintableAreaAt"/> already
        // carries: `aPageRect` is in *document* twips, so `nLineWidth` comes off it and arrives
        // unscaled while `nHeight - nDistance` is added whole and arrives at `nHeight × zoom`
        // (`ScPrintFunc::GetDocPageSize`, `printfun.cxx:3002`, over the map mode's zoom fraction
        // at `:2645`). At an unscaled sheet the two are the same figure and nothing here moves.
        //
        // Both halves read straight off `FY2023-AIP-grants.xlsx` page 3, whose print scale is
        // 43 %: the reference's content stream opens `17.995 576.503 755.94 13.91 re W* n` for a
        // 32.4 pt header band 756 pt wide, and `17.995 21.674 755.94 5.74 re W* n` for a footer
        // whose dynamic band holds one 13.4 pt line.
        //
        // **Its origin is `bandTop` and is not derived a second time.** `aStart` is both the clip
        // region's corner and the point the three areas are drawn from — `PrintHF` passes the one
        // variable to `SetClipRegion` and to each `DrawText_ToPosition`
        // (`printfun.cxx:1870-1912`) — so computing it twice is what let the window drift out of
        // step with where the text had actually been put. An area placed outside a window it is
        // then tested against is not clipped, it is **rejected**, and `TICAPCapability_Final.xls`
        // lost its header and its footer on six pages that way.
        Length clipHeight = scaled;
        Length clipTop = bandTop;

        Place(band.Left, _ => left);
        Place(band.Centre, width => left + ((right - left - width) / 2));
        Place(band.Right, width => right - width);

        void Place(SheetHeaderPart part, Func<Length, Length> position)
        {
            if (part.IsEmpty) return;

            IReadOnlyList<IReadOnlyList<SheetHeaderPiece>> lines = part.Lines(context);
            if (lines.Count == 0) return;

            Length text = Length.Zero;
            foreach (IReadOnlyList<SheetHeaderPiece> line in lines)
                text += SheetBandHeight.LineHeight(line, zoom, bandFont);

            Length spare = drawn - text;
            Length pen = bandTop + (spare > Length.Zero ? spare / 2 : Length.Zero);

            // Everything the area would draw, laid out but not yet emitted, together with the
            // rectangle its ink occupies. `DrawText_ToPosition` asks that question of the area as
            // a whole, so it has to be answered before a single run is written.
            List<(BandRun Run, DocPoint Origin)> placed = [];
            Length inkLeft = Length.Zero;
            Length inkRight = Length.Zero;
            Length inkTop = Length.Zero;
            Length inkBottom = Length.Zero;
            bool anyInk = false;

            foreach (IReadOnlyList<SheetHeaderPiece> line in lines)
            {
                Length lineHeight = SheetBandHeight.LineHeight(line, zoom, bandFont);
                if (line.Count == 0)
                {
                    pen += lineHeight;
                    continue;
                }

                Length width = Length.Zero;
                List<(BandRun Run, Length Size, SheetBandFace Face)> runs = [];
                foreach (SheetHeaderPiece piece in line)
                {
                    Length size = SheetBandHeight.SizeOf(piece, zoom, bandFont);
                    SheetBandFace face = SheetBandHeight.FaceOf(piece, bandFont);
                    if (SheetBandText.Shape(piece.Text, size, face.Family, face.Bold, face.Italic)
                        is not { } run)
                    {
                        continue;
                    }

                    runs.Add((run, size, face));
                    width += run.Width;
                }

                if (runs.Count > 0)
                {
                    Length ascent = Length.Zero;
                    foreach ((_, Length size, SheetBandFace face) in runs)
                    {
                        ascent = Length.Max(
                            ascent,
                            SheetBandText.AscentAt(size, face.Family, face.Bold, face.Italic));
                    }

                    Length x = position(width);
                    foreach ((BandRun run, Length size, SheetBandFace face) in runs)
                    {
                        Length baseline = pen + ascent;
                        placed.Add((run, new DocPoint(x, baseline)));

                        Length runTop = baseline
                                        - SheetBandText.CapHeightAt(
                                            size, face.Family, face.Bold, face.Italic);
                        Length runBottom =
                            baseline
                            + SheetBandText.LineHeightAt(size, face.Family, face.Bold, face.Italic)
                            - SheetBandText.AscentAt(size, face.Family, face.Bold, face.Italic);

                        if (!anyInk)
                        {
                            (inkLeft, inkRight, inkTop, inkBottom) =
                                (x, x + run.Width, runTop, runBottom);
                            anyInk = true;
                        }
                        else
                        {
                            inkLeft = Length.Min(inkLeft, x);
                            inkRight = Length.Max(inkRight, x + run.Width);
                            inkTop = Length.Min(inkTop, runTop);
                            inkBottom = Length.Max(inkBottom, runBottom);
                        }

                        x += run.Width;
                    }
                }

                pen += lineHeight;
            }

            if (!anyInk) return;

            // The clip. `PrintHF`'s rectangle is `[left, right] x [top, top + height]`, and an
            // area whose ink misses it entirely is not drawn — not the ink and not the text.
            bool overlaps = inkLeft < right
                            && inkRight > left
                            && inkTop < clipTop + clipHeight
                            && inkBottom > clipTop;

            if (!overlaps) return;

            // Ink that is only *partly* inside is cut at the band's edge, and the remark above
            // said the opposite until round `clip`. `DrawText_ToPosition` has three branches, not
            // two: no overlap returns having drawn nothing, wholly inside draws unwrapped, and
            // anything else embeds the area in a `MaskPrimitive2D` of the clip polygon
            // (`editeng/source/editeng/impedit3.cxx:3367-3389`). A mask keeps every line in the
            // primitive tree — which is what "keeps every line" meant and why it read as no clip
            // at all — and cuts every one of them geometrically when it is rendered.
            //
            // `ClipPathKeepingText`, because a band's words are the document's own: the reference
            // cuts the ink at the same edge and leaves the glyphs in its PDF's text layer. On
            // `sheets/done-011/xlsx/FY2023-AIP-grants.xlsx` the header is three centred lines in
            // a band that holds two — `pdftotext` reads all three off the reference's page 3 while
            // its content stream opens with `17.995 576.503 755.94 13.91 re W* n` and the third
            // line's ink stops just inside that rectangle's bottom edge.
            //
            // **The rectangle is as tall as the band at the print scale and as wide as the band
            // at full size**, which is the same asymmetry `SheetPrintSetup.PrintableAreaAt`
            // already carries: `aPageRect` is in *document* twips, so `nLineWidth` comes off it
            // and arrives unscaled while `aPaperSize.Height() = nHeight - nDistance` is added
            // whole and arrives at `nHeight × zoom` (`ScPrintFunc::GetDocPageSize`,
            // `printfun.cxx:3002`, over the map mode's zoom fraction at `:2645`). Both halves
            // read straight off that one document: the header's clip is
            // `(32.4 - 0) × 0.4293 = 13.91` pt tall and `774 - 18 = 756` pt wide, and the
            // footer's — a dynamic band, so its own text height — is `13.4 × 0.4293 = 5.74`.
            bool inside = inkLeft >= left
                          && inkRight <= right
                          && inkTop >= clipTop
                          && inkBottom <= clipTop + clipHeight;

            if (!inside)
            {
                sink.Save();
                sink.ClipPathKeepingText(
                    Rectangle(new DocRect(left, clipTop, right - left, clipHeight)));
            }

            try
            {
                foreach ((BandRun run, DocPoint origin) in placed)
                    sink.DrawGlyphRun(run.At(origin), Paint.Solid(Colour.Black));
            }
            finally
            {
                if (!inside) sink.Restore();
            }
        }
    }

    /// <summary>A rectangle as a closed path, for a clip.</summary>
    private static GraphicsPath Rectangle(DocRect rect)
        => new GraphicsPath()
           .MoveTo(new DocPoint(rect.X, rect.Y))
           .LineTo(new DocPoint(rect.Right, rect.Y))
           .LineTo(new DocPoint(rect.Right, rect.Bottom))
           .LineTo(new DocPoint(rect.X, rect.Bottom))
           .Close();

    /// <summary>
    /// How tall one part of a band is, how tall one of its lines is, and what face and size a
    /// piece is drawn at, all of which live in <see cref="SheetBandHeight"/>.
    /// </summary>
    /// <remarks>
    /// They moved there because the band's <em>height</em> and its <em>drawing</em> ask the same
    /// question and must not answer it twice: <c>ScPrintFunc::UpdateHFHeight</c> measures the
    /// areas to size the band and <c>ScPrintFunc::PrintHF</c> measures them again to centre the
    /// text in it, from one EditEngine with one set of defaults
    /// (<c>sc/source/ui/view/printfun.cxx:817-836</c>, <c>:1876-1912</c>). While the ODF reader
    /// took the declared height alone there was no caller for the first half, and the drawing
    /// path was the only one measuring anything.
    /// </remarks>
    /// <summary>One stroke of a border, with its ends extended to meet what it crosses.</summary>
    /// <remarks>
    /// <para>
    /// The extension is half the width of the perpendicular border it meets, and nothing at all
    /// where it meets none — which is the whole of what <c>getExtends</c> computes for the simple
    /// case (<c>svx/source/sdr/primitive2d/sdrframeborderprimitive2d.cxx:310</c>): the line is cut
    /// against the far edge of the other line's band, and that edge is half its width past the
    /// crossing point.
    /// </para>
    /// <para>
    /// Measured on <c>sheet-decor-ods.ods</c>: the red 2.5 pt vertical in row 2 meets no
    /// horizontal at either end and runs 12.784 pt against a row 12.813 pt tall, while the blue
    /// vertical in row 3 ends on B4's one-point box and overshoots it by 0.509 pt. A rule that
    /// always overshot by half its <em>own</em> width — which is what a Writer table does —
    /// would put the red one 2.5 pt too long.
    /// </para>
    /// </remarks>
    private void Stroke(Edge edge, Edges edges, IDrawingSink sink)
    {
        SheetBorder border = edge.Border;
        Length start = edge.From - (edges.ExtensionAt(edge, edge.From) * _scale);
        Length end = edge.To + (edges.ExtensionAt(edge, edge.To) * _scale);

        // A double rule is two lines about the centre, the gap between them untouched.
        if (border.IsDouble)
        {
            Length half = (border.Primary + border.Distance + border.Secondary) * _scale / 2;
            Line(edge, start, end, half - (border.Primary * _scale / 2), border.Primary, sink);
            Line(edge, start, end, (border.Secondary * _scale / 2) - half, border.Secondary, sink);
            return;
        }

        Line(edge, start, end, Length.Zero, border.Primary, sink);
    }

    /// <summary>
    /// The width one line of a border is stroked at: the stated width times the print scale,
    /// floored at a tenth of a point.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A cell border is scaled by the sheet's print scale exactly as its geometry is,
    /// and this drew every one of them at full size.</strong> Calc hands
    /// <c>svx::frame::Style</c> a width in twips times <c>nScaleX</c>
    /// (<c>ScDocument::FillInfo</c>, <c>sc/source/core/data/fillinfo.cxx:1144-1147</c>), and for
    /// a print or a PDF export <c>nScaleX</c> is the constant twips-to-1/100 mm factor
    /// (<c>ScPrintFunc::InitModes</c>, <c>printfun.cxx:2627-2628</c>) — the zoom arrives later,
    /// as the fractional scale of the <c>Map100thMM</c> map mode the whole page is drawn
    /// through (<c>:2639-2641</c>), which multiplies the widths along with the coordinates.
    /// </para>
    /// <para>
    /// Measured at 26.2.4.2 rather than read off that: one workbook of thirteen cells, one
    /// border style each, exported at nine stated print scales
    /// (<c>probes/sheet-border-r115/make.py</c>). The drawn <c>w</c> operators are the stated
    /// width times the scale at every one of them — <c>thin</c> 0.75, 0.5625, 0.375, 0.315,
    /// 0.1875 and 3.0 pt at 100, 75, 50, 42, 25 and 400 per cent — and the same nine numbers
    /// come back out of the file saved as <c>.xls</c>, so the two filters share this.
    /// </para>
    /// <para>
    /// <strong>The floor is a tenth of a point</strong>, which is one pixel of the PDF export's
    /// own 720 dpi device: <c>PDFPage::appendLineInfo</c> writes <c>72/DPIX</c> for anything
    /// that does not survive as a whole logic unit (<c>vcl/source/pdf/PDFPage.cxx:497-505</c>).
    /// Measured: <c>hair</c> is 1 twip and is drawn at 0.1 pt at 100 per cent and at 0.19956 at
    /// 400, and <c>thin</c> at a scale of 10 per cent is drawn at 0.1 rather than 0.075.
    /// </para>
    /// <para>
    /// <strong>A patterned line's width goes through a whole 1/100 mm first.</strong> The dashed
    /// styles reach the metafile as a <c>LineInfo</c> whose width is
    /// <c>std::round(getTransformedLineWidth(...))</c> in logic units
    /// (<c>drawinglayer/source/processor2d/vclmetafileprocessor2d.cxx:1818-1819</c>) where a
    /// solid one keeps its double, so a dotted <c>thin</c> is 26 rather than 26.46 hundredths of
    /// a millimetre. Measured at 0.737 against a solid 0.75004, and 1.75745 against 1.75008 for
    /// the medium ones — and the rounding happens before the zoom, since at 42 per cent the same
    /// lines are 0.30956 and 0.73817.
    /// </para>
    /// </remarks>
    /// <param name="border">The border the line belongs to, for its pattern.</param>
    /// <param name="width">One line's stated width.</param>
    private Length Drawn(SheetBorder border, Length width)
    {
        if (width <= Length.Zero) return Length.Zero;

        Length stated = border.Pattern == SheetBorderPattern.Solid
            ? width
            : Length.FromMm100(Math.Max(1, width.Mm100));

        return Length.Max(stated * _scale, HairlineWidth);
    }

    private void Line(
        Edge edge, Length start, Length end, Length offset, Length width, IDrawingSink sink)
    {
        if (width <= Length.Zero) return;

        Length at = edge.At + offset;
        Length drawn = Drawn(edge.Border, width);

        // Butt caps and round joins, which is what LibreOffice's own export writes for a border:
        // "q 2.49983 w 0 J 1 j". A square cap would add half a width at each end and undo the
        // extension arithmetic above.
        Stroke pen = new(
            Paint.Solid(edge.Border.Colour), drawn, LineCap.Butt, LineJoin.Round,
            DashPattern: Dashes(edge.Border));

        GraphicsPath path = edge.IsHorizontal
            ? new GraphicsPath().MoveTo(new DocPoint(start, at)).LineTo(new DocPoint(end, at))
            : new GraphicsPath().MoveTo(new DocPoint(at, start)).LineTo(new DocPoint(at, end));

        sink.StrokePath(path, pen);
    }

    /// <summary>
    /// The dash pattern a border pattern draws with, or null for a solid line.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>Absolute, and not a multiple of the line's width.</strong> A border's dashing is a
    /// fixed table scaled by the border's own <em>pattern scale</em>, which for a Calc cell is the
    /// twips-to-1/100 mm factor and nothing to do with how thick the line is:
    /// <c>ScDocument::FillInfo</c> builds every <c>svx::frame::Style</c> as
    /// <c>Style(pBox-&gt;GetLeft(), fColScale)</c> (<c>sc/source/core/data/fillinfo.cxx</c>:1144-1147),
    /// which lands in <c>mfPatternScale</c>; <c>CreateBorderPrimitives</c> then asks for
    /// <c>svtools::GetLineDashing(type, PatternScale() * 10.0)</c>
    /// (<c>svx/source/sdr/primitive2d/sdrframeborderprimitive2d.cxx</c>:599-601) and
    /// <c>GetDashing</c> is a table of small integers — 1/2 dotted, 6/2 fine-dashed, 16/5 dashed,
    /// 16/5/5/5 dash-dot and 16/5/5/5/5/5 dash-dot-dot
    /// (<c>svtools/source/control/ctrlbox.cxx</c>:248-283). Ten of those units times 1.76389
    /// hundredths of a millimetre is <see cref="DashUnit"/>: ten twips, half a point.
    /// </para>
    /// <para>
    /// Measured at 26.2.4.2 rather than read off that. On a thirteen-style fixture the arrays are
    /// the same five whatever the border's width — a <c>thin</c> dotted rule and a <c>medium</c>
    /// dotted one both draw <c>[0.49999 0.99998]</c> — and they take the print scale like
    /// everything else, coming out as <c>[0.21001 0.42002]</c> at 42 per cent
    /// (<c>probes/sheet-border-r115/results.md</c> §4). Confirmed again on a corpus document
    /// rather than a fixture: page 1 of <c>079_Org_charts_visual</c> is drawn at a fitted scale of
    /// 0.40002 and its dotted borders come out <c>0.29481 w</c> with <c>[.20001 .40001] 0</c> —
    /// which is <c>26/100 mm × 0.40002</c> for the width and <c>[0.5 1.0] pt × 0.40002</c> for the
    /// dash, to five figures and with no free parameter.
    /// </para>
    /// <para>
    /// <strong>The dash lengths are not rounded to a whole logic unit and the width is.</strong>
    /// <c>VclMetafileProcessor2D::processPolygonStrokePrimitive2D</c> writes
    /// <c>std::round(getTransformedLineWidth(...))</c> for the width and a bare
    /// <c>getTransformedLineWidth(...)</c> for <c>SetDashLen</c> and <c>SetDistance</c>
    /// (<c>drawinglayer/source/processor2d/vclmetafileprocessor2d.cxx</c>:1818-1874), which is
    /// why <see cref="Drawn"/> rounds and this does not.
    /// </para>
    /// </remarks>
    /// <param name="border">The border whose pattern is wanted.</param>
    private Length[]? Dashes(SheetBorder border)
    {
        double[]? units = border.Pattern switch
        {
            SheetBorderPattern.Dotted => [1, 2],
            SheetBorderPattern.Dashed => [16, 5],
            SheetBorderPattern.FineDashed => [6, 2],
            SheetBorderPattern.DashDot => [16, 5, 5, 5],
            SheetBorderPattern.DashDotDot => [16, 5, 5, 5, 5, 5],
            _ => null,
        };

        if (units is null) return null;

        Length[] dashes = new Length[units.Length];
        for (int at = 0; at < units.Length; at++) dashes[at] = DashUnit * units[at] * _scale;
        return dashes;
    }

    private static void Rule(Length x1, Length y1, Length x2, Length y2, IDrawingSink sink)
        => sink.StrokePath(
            new GraphicsPath().MoveTo(new DocPoint(x1, y1)).LineTo(new DocPoint(x2, y2)),
            new Stroke(Paint.Solid(FurnitureColour), HairlineWidth));

    /// <summary>A heading cell: its box, and its label centred inside it.</summary>
    /// <remarks>
    /// <para>
    /// <strong>The label is drawn at the page's own print scale, not at a fixed ten point.</strong>
    /// <c>ScPrintFunc::PrintPage</c> sets the heading font on the device while its map mode is
    /// <c>aOffsetMode</c> (<c>sc/source/ui/view/printfun.cxx:2350-2357</c>), and <c>InitModes</c>
    /// builds that map mode with the zoom as its scale fraction (<c>:2642</c>), so
    /// <c>PrintColHdr</c>'s <c>DrawText</c> is scaled along with the cells. The strip itself was
    /// already scaled by the caller — <c>HeadingWidth * _scale</c> — so only the text was out.
    /// </para>
    /// <para>
    /// Measured on a probe stating <c>printOptions headings="1"</c> through LibreOffice 24.2.7.2:
    /// at <c>pageSetup scale="30"</c> the reference's column heading <c>A</c> is 3.35 pt tall and
    /// this drew it 11.17 pt tall, while at <c>scale="100"</c> both are 11.17 pt. The corpus
    /// witness is <c>Application_Compliance_Checklist_5_Apr_2021.xlsx</c>, whose sheets print at
    /// 28, 50 and 59 per cent with headings switched on: its row numbers were drawn three times
    /// over the cells.
    /// </para>
    /// </remarks>
    /// <param name="area">The heading cell's rectangle.</param>
    /// <param name="label">The column letter or row number.</param>
    /// <param name="scale">The page's print scale, as a fraction.</param>
    /// <param name="sink">Receives the drawing commands.</param>
    private static void Box(DocRect area, string label, double scale, IDrawingSink sink)
    {
        Outline(area, sink);

        Length size = SheetBandText.DefaultSize * scale;
        if (SheetBandText.Shape(label, size) is not { } run) return;

        Length x = area.X + ((area.Width - run.Width) / 2);
        Length y = area.Y + ((area.Height - SheetBandText.LineHeightAt(size)) / 2)
                   + SheetBandText.AscentAt(size);

        sink.DrawGlyphRun(run.At(new DocPoint(x, y)), Paint.Solid(FurnitureColour));
    }

    /// <summary>
    /// A column's heading: A, B … Z, AA, AB and so on.
    /// </summary>
    /// <remarks>
    /// Bijective base 26, which is the part that catches people out: there is no zero digit, so
    /// column 26 is AA rather than BA and the remainder has to be taken before the division.
    /// <c>ScColToAlpha</c> (<c>sc/source/core/tool/address.cxx</c>) writes the same loop.
    /// </remarks>
    private static string ColumnName(int column)
    {
        if (column < 0) return string.Empty;

        Span<char> letters = stackalloc char[8];
        int at = letters.Length;
        int value = column;

        do
        {
            letters[--at] = (char)('A' + (value % 26));
            value = (value / 26) - 1;
        }
        while (value >= 0 && at > 0);

        return new string(letters[at..]);
    }

    private static void Outline(DocRect area, IDrawingSink sink)
    {
        if (area.Width <= Length.Zero || area.Height <= Length.Zero) return;

        sink.StrokePath(
            new GraphicsPath()
                .MoveTo(new DocPoint(area.X, area.Y))
                .LineTo(new DocPoint(area.Right, area.Y))
                .LineTo(new DocPoint(area.Right, area.Bottom))
                .LineTo(new DocPoint(area.X, area.Bottom))
                .Close(),
            new Stroke(Paint.Solid(FurnitureColour), HairlineWidth));
    }

    private static void Fill(DocRect area, Colour colour, IDrawingSink sink)
    {
        if (area.Width <= Length.Zero || area.Height <= Length.Zero) return;

        sink.FillPath(
            new GraphicsPath()
                .MoveTo(new DocPoint(area.X, area.Y))
                .LineTo(new DocPoint(area.Right, area.Y))
                .LineTo(new DocPoint(area.Right, area.Bottom))
                .LineTo(new DocPoint(area.X, area.Bottom))
                .Close(),
            Paint.Solid(colour));
    }

    /// <summary>One drawn border: where it sits, how far it runs, and what it is.</summary>
    /// <param name="IsHorizontal">True when it runs across the page.</param>
    /// <param name="At">Where it sits on the other axis.</param>
    /// <param name="From">Where it starts along its own axis.</param>
    /// <param name="To">Where it ends.</param>
    /// <param name="Border">The resolved border.</param>
    private readonly record struct Edge(
        bool IsHorizontal, Length At, Length From, Length To, SheetBorder Border);

    /// <summary>
    /// Which run a cell edge belongs to: everything but its position along that run.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Two edges coalesce when their keys are equal and their <c>Index</c> values are adjacent.
    /// </para>
    /// <para>
    /// <c>Line</c> identifies the grid line by <em>which edge of which placed row or column</em>
    /// stated it — <c>2 × index</c> for a top or left edge and <c>2 × index + 1</c> for a bottom
    /// or right one — and not by its coordinate. Keying on the coordinate is wrong and cost this
    /// round a measured regression: a workbook with a zero-height row puts two grid lines at the
    /// same y, so the pieces of two different lines land in one bucket with duplicate indices, and
    /// what came out of the run builder was five mutually overlapping segments where there should
    /// have been two coincident ones. Measured on page 2 of
    /// <c>6880ac7361ca1b99a9230811_ST Capability List Rev.16 - Web.xlsx</c>, where the overhang
    /// double-ink went 251 pt → 710 pt against a reference of 1.8 pt. With the line identity in
    /// the key an index cannot repeat, by construction.
    /// </para>
    /// </remarks>
    /// <param name="IsHorizontal">True when the run goes across the page.</param>
    /// <param name="Line">Which grid line stated it, as an edge of a placed row or column.</param>
    /// <param name="Band">Which printed band drew it; runs never cross one.</param>
    /// <param name="Border">The resolved border, compared in full — width, gap, colour, pattern.</param>
    private readonly record struct RunKey(
        bool IsHorizontal, int Line, int Band, SheetBorder Border);

    /// <summary>
    /// A page's borders, with the crossings indexed so an end can be extended.
    /// </summary>
    /// <remarks>
    /// Indexed on the coordinate in twips, because a corner is reached from two rectangles — a
    /// row's bottom and the next row's top — and the two can differ in the last EMU after the
    /// print scale has been applied.
    /// </remarks>
    private sealed class Edges
    {
        private readonly List<Edge> _runs = [];
        private readonly Dictionary<(bool Horizontal, long At, long Along), Length> _widths = [];
        private readonly Dictionary<RunKey, List<Piece>> _pieces = [];

        // Iterated rather than the dictionary, so the order strokes reach the page is the order
        // the cells were walked in and does not depend on a hash. Two renders of one document
        // have to be byte-identical: that is how this project measures a change's reach.
        private readonly List<RunKey> _keys = [];

        public IReadOnlyList<Edge> All => _runs;

        public static Edges Build(
            Func<int, int, SheetCellDecoration> decoration,
            SheetMerges merges,
            IReadOnlyList<PlacedColumn> columns,
            IReadOnlyList<PlacedRow> rows,
            int lastRepeatedColumn,
            int lastRepeatedRow)
        {
            Edges edges = new();

            for (int r = 0; r < rows.Count; r++)
            {
                PlacedRow row = rows[r];

                // Which of the page's up-to-four printed areas this cell was drawn by. A repeated
                // band is a separate ScPrintFunc::PrintArea call with its own ScOutputData, so a
                // border run may not cross into it; SheetPagePlacement only carries a repeat range
                // when the page's own cells start past it, so the test cannot misfire on the page
                // that holds the band itself.
                int rowBand = row.Row <= lastRepeatedRow ? 0 : 1;

                for (int c = 0; c < columns.Count; c++)
                {
                    PlacedColumn column = columns[c];
                    int columnBand = column.Column <= lastRepeatedColumn ? 0 : 1;
                    SheetCellBorders own = decoration(row.Row, column.Column).Borders;
                    bool firstRowOfBand = r == 0 || rows[r - 1].Row != row.Row - 1;
                    bool firstColumnOfBand = c == 0 || columns[c - 1].Column != column.Column - 1;

                    if (!merges.IsOverlappedTop(row.Row, column.Column))
                    {
                        edges.Add(
                            true, 2 * r, columnBand, c,
                            row.Y, column.X, column.Right, firstRowOfBand
                            ? own.Top
                            : SheetCellBorders.Resolve(
                                own.Top, decoration(row.Row - 1, column.Column).Borders.Bottom));
                    }

                    if (!merges.IsOverlappedLeft(row.Row, column.Column))
                    {
                        edges.Add(
                            false, 2 * c, rowBand, r,
                            column.X, row.Y, row.Bottom, firstColumnOfBand
                            ? own.Left
                            : SheetCellBorders.Resolve(
                                own.Left, decoration(row.Row, column.Column - 1).Borders.Right));
                    }

                    // The far edges only where nothing follows to cover them, because every
                    // other one is already the next cell's near edge — which is what keeps two
                    // neighbours agreeing about a border from drawing it twice. "Nothing
                    // follows" is not the same as "last on the page": a repeated header band is
                    // placed above a block it is not adjacent to, and a hidden row leaves the
                    // same gap, so the test is whether the next placed row really is this row's
                    // successor in the sheet. Calc reaches the same answer by drawing each band
                    // through its own ScOutputData (ScPrintFunc::PrintPage, printfun.cxx:2300).
                    if ((r == rows.Count - 1 || rows[r + 1].Row != row.Row + 1)
                        && !merges.IsOverlappedBottom(row.Row, column.Column))
                    {
                        edges.Add(
                            true, (2 * r) + 1, columnBand, c,
                            row.Bottom, column.X, column.Right, own.Bottom);
                    }

                    if ((c == columns.Count - 1 || columns[c + 1].Column != column.Column + 1)
                        && !merges.IsOverlappedRight(row.Row, column.Column))
                    {
                        edges.Add(
                            false, (2 * c) + 1, rowBand, r,
                            column.Right, row.Y, row.Bottom, own.Right);
                    }
                }
            }

            edges.Coalesce();
            return edges;
        }

        /// <summary>How far an end reaches past its own line to meet what crosses it.</summary>
        /// <remarks>
        /// Asked of a coalesced run but answered from the <em>uncoalesced</em> crossing edges,
        /// which is what makes an interior joint disappear and an outer end keep its overshoot.
        /// A merged vertical run has endpoints only at the top and bottom of the whole run, so a
        /// horizontal in the middle of a table would find nothing to extend to if the index were
        /// rebuilt after coalescing — and the reference extends it: on page 3 of
        /// <c>T0A0D0000090006XLSE.xls</c> every one of the twelve horizontals starts at 53.433
        /// where the vertical sits at 53.802, half a 0.75 pt border away.
        /// </remarks>
        public Length ExtensionAt(Edge edge, Length end)
            => _widths.TryGetValue((!edge.IsHorizontal, end.Twips, edge.At.Twips), out Length width)
                ? width / 2
                : Length.Zero;

        private void Add(
            bool horizontal,
            int line,
            int band,
            int index,
            Length at,
            Length from,
            Length to,
            SheetBorder border)
        {
            if (border.IsNone) return;

            RunKey key = new(horizontal, line, band, border);
            if (!_pieces.TryGetValue(key, out List<Piece>? pieces))
            {
                _pieces[key] = pieces = [];
                _keys.Add(key);
            }

            pieces.Add(new Piece(index, at, from, to));

            // Recorded at both ends so that either end of a crossing line can find it. The
            // narrowest wins a tie, which is what getExtends does when several lines meet at one
            // point: it takes the minimum of the cut sets.
            Note(horizontal, at, from, border.Width);
            Note(horizontal, at, to, border.Width);
        }

        /// <summary>Folds each grid line's identically styled cell edges into maximal runs.</summary>
        /// <remarks>
        /// Adjacency is by <em>placed</em> index rather than by sheet row or column, and that is
        /// measured rather than assumed: sheet 2 of <c>sheet-border-runs.fods</c> hides a column
        /// in the middle of a four-cell top border and a row in the middle of a four-row left
        /// border, and LibreOffice draws one stroke for each — 56.665→226.743 across three visible
        /// columns, and 717.306→768.245 down three visible rows. A hidden line is not in Calc's
        /// border array at all, so it does not interrupt a run.
        /// </remarks>
        private void Coalesce()
        {
            foreach (RunKey key in _keys)
            {
                List<Piece> pieces = _pieces[key];
                pieces.Sort(static (a, b) => a.Index.CompareTo(b.Index));

                int at = 0;
                while (at < pieces.Count)
                {
                    int last = at;
                    while (last + 1 < pieces.Count
                           && pieces[last + 1].Index == pieces[last].Index + 1)
                    {
                        last++;
                    }

                    _runs.Add(new Edge(
                        key.IsHorizontal,
                        pieces[at].At,
                        pieces[at].From,
                        pieces[last].To,
                        key.Border));

                    at = last + 1;
                }
            }
        }

        /// <summary>One cell edge, before its run is folded together.</summary>
        /// <param name="Index">Where it sits along the run: the placed column or row index.</param>
        /// <param name="At">Its exact position on the other axis, kept rather than the key's twips.</param>
        /// <param name="From">Where it starts along its own axis.</param>
        /// <param name="To">Where it ends.</param>
        private readonly record struct Piece(int Index, Length At, Length From, Length To);

        private void Note(bool horizontal, Length at, Length along, Length width)
        {
            (bool, long, long) key = (horizontal, at.Twips, along.Twips);
            if (!_widths.TryGetValue(key, out Length existing) || width < existing)
                _widths[key] = width;
        }
    }
}
