using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.Text.Layout;

namespace XRay.Office.WordProcessing.Layout;

/// <summary>
/// Works out how tall a table's rows are, and where each of its cells goes.
/// </summary>
/// <remarks>
/// <para>
/// Two passes, because the answer is circular until it is not. A cell's height depends on its width, which
/// the grid states outright; a row's height is its tallest cell's; and a cell's <em>rectangle</em> depends
/// on the row heights. So the first pass lays every cell's text out at its own width and keeps the result,
/// and the second turns the heights into rectangles. Nothing is measured twice.
/// </para>
/// <para>
/// A cell spanning rows contributes to the last row it covers rather than to the first. That is what keeps
/// a tall merged cell from making its <em>first</em> row tall — the two-row merge in a real table is
/// usually beside two short ordinary cells, and charging its height to row one would leave row two empty
/// and the table the wrong shape. LibreOffice reaches the same answer by growing the merge's rows to fit;
/// charging the last row is the same result for the common case and cheaper than an iteration.
/// </para>
/// </remarks>
public static class TableLayouter
{
    /// <summary>
    /// Lays a table out at a stated origin, reporting the cells and the row heights.
    /// </summary>
    /// <param name="table">The table.</param>
    /// <param name="origin">
    /// Where the table's top-left corner goes, in page coordinates. Its left indent is applied here rather
    /// than by the caller, so a caller only has to know where the body area starts.
    /// </param>
    /// <param name="nesting">
    /// How many tables enclose this one, so that a file claiming absurd nesting stops rather than recursing.
    /// </param>
    /// <param name="available">
    /// How wide the area holding the table is — the body's text width, or the enclosing cell's inner width
    /// for a nested one. It changes nothing for a table that declares its grid, which is every table this
    /// engine has ever laid out; it is read only by one that left a column without a width and so has to be
    /// fitted to what it sits in. See <see cref="PageTable.ColumnFit"/>.
    /// </param>
    /// <param name="collapsesSpacing">
    /// Whether the paragraphs inside a cell collapse their spacing against one another rather than adding
    /// it — see <see cref="FlowLayouter.LayOut"/>. It decides a cell's height, and so a row's, so passing
    /// the document's answer matters as much here as it does in the body.
    /// </param>
    /// <param name="addsCellLineSpacing">
    /// Whether a cell grows by its last paragraph's proportional line spacing — Writer's
    /// <c>AddParaLineSpacingToTableCells</c>. See <see cref="CellLineSpacing"/>.
    /// </param>
    /// <returns>
    /// The cells with page-coordinate rectangles, and each row's height in order — the caller needs the
    /// heights to decide where the table ends and which rows fit on the page.
    /// </returns>
    /// <param name="anchored">
    /// The frames the flow's own paragraphs anchor, with the page coordinates its origin corresponds to,
    /// or null when the caller does not know where the flow sits — which is every caller but a table
    /// cell's. See <see cref="AnchoredObstacles"/> for why a cell needs its own route to them.
    /// </param>
    public static (List<PlacedTableCell> Cells, List<Length> RowHeights) LayOut(
        PageTable table,
        DocPoint origin,
        int nesting = 0,
        Length? available = null,
        bool collapsesSpacing = false,
        bool addsCellLineSpacing = false,
        AnchoredObstacles? anchored = null)
    {
        ArgumentNullException.ThrowIfNull(table);

        List<Length> lefts = ColumnLefts(table.WidthsWithin(available ?? table.Width));
        int rows = Math.Min(table.Rows.Count, PageTable.MaxRows);

        // Pass one: every cell's text, laid out at its own width, with the row it charges its height to.
        List<Measured> measured = [];
        List<Length> heights = [.. Enumerable.Repeat(Length.Zero, rows)];

        // Where the row about to be measured starts, in the table's own coordinates — the same running
        // sum pass two makes into `tops`, kept here because a cell that has to flow round a frame it
        // anchors needs to know where on the page it is *while* it is being measured. Only the rows above
        // it are needed and they are already settled, so nothing is being read before it is written.
        // The table's frame top IS the first boundary: the first row's band hangs downwards from it
        // rather than the first grid line sitting half a border below it. See `TopBand`.
        Length measuredRowTop = Length.Zero;

        for (int row = 0; row < rows; row++)
        {
            foreach (PageTableCell cell in table.Rows[row].Cells)
            {
                Length width = WidthOf(cell, lefts);
                if (width <= Length.Zero) continue;

                int last = Math.Min(row + Math.Max(1, cell.RowSpan), rows) - 1;

                // A turned cell breaks its text at the cell's *height*, which is exactly what this pass is
                // trying to work out — so it is not measured here at all. It does not have to be: it
                // contributes nothing to the row's height, so the circle never closes. See `Turned`.
                if (cell.IsTurned)
                {
                    measured.Add(new Measured(row, last, cell, width, null, Length.Zero));
                    continue;
                }

                Length inner = width - cell.Padding.Horizontal;
                PlacedFlow? content = inner > Length.Zero
                    ? FlowLayouter.LayOut(
                        cell.Blocks,
                        new DocRect(Length.Zero, Length.Zero, inner, Length.Zero),
                        Length.Zero,
                        nesting,
                        collapsesSpacing,
                        addsCellLineSpacing,
                        anchored: anchored?.Below(new DocPoint(
                            table.LeftWithin(available ?? table.Width)
                            + lefts[cell.Column] + cell.Padding.Left,
                            measuredRowTop + TopBand(row > 0 ? table.Rows[row - 1] : null, table.Rows[row])
                            + cell.Padding.Top)))
                    : null;

                // The advance rather than the ink: a cell is as tall as its content plus the space after
                // its last paragraph, which is what `AddParaSpacingToTableCells` — on for every Word
                // document — makes Writer do. See `PlacedFlow.Advance`.
                Length text = content is null ? Length.Zero : content.Advance;

                // And its last paragraph's proportional line spacing on top of that, which is the second
                // half of the same compatibility rule. See `CellLineSpacing`.
                if (addsCellLineSpacing) text += CellLineSpacing(cell.Blocks);

                measured.Add(new Measured(row, last, cell, width, content, text));

                // A merged cell charges only its last row, and only for what one row's worth of it needs.
                // Charging the whole height there would make that row as tall as the merge.
                //
                // The declared floor is deliberately *not* applied here. Two rounds found the same defect
                // independently — a `w:trHeight` `atLeast` row resting on its floor is its floor plus its
                // cell margins tall, not the greater of the floor and text-plus-margins — and wrote two
                // different fixes. This one, applied in the per-cell loop and ungated, is the weaker of
                // them: it reaches every format, and the setting it models is one the three Word filters
                // set and the ODF filter does not. Applying it to all five was measured to fix
                // `table-exact-row.{doc,docx,rtf}` and break `.odt` and `.fodt`.
                //
                // The floor is therefore applied once, per row, further down, gated on
                // `PageTable.MinHeightIncludesInsets` and carrying the border term as well. Both rounds'
                // evidence agrees and is recorded there: `template---tpr…docx`'s 460-twip rows with
                // 100-twip margins measure 33.00 pt, and `FAA 2025-26 Holdover Tables.docx`'s Table 54
                // measures 440 twips to the hundredth.
                if (last == row)
                {
                    heights[row] = Length.Max(heights[row], text + cell.Padding.Vertical);
                }
            }

            // The declared height, which is a floor unless the row says it is exact — in which case it is the
            // height, and content taller than it is clipped rather than growing the row. Applied per row
            // before the merge shortfall below, so that a merge spanning an exact row cannot stretch it.
            //
            // A border takes space, and a row owns *half* of each of the two grid lines it sits between — the
            // line runs through the border's centre, so the other half belongs to the neighbour. The two
            // outermost halves, which have no neighbour, are added to the last row once the rectangles are
            // built; see there.
            //
            // The `atLeast` floor sits *under* those borders rather than over them: it raises the content and
            // the borders are then added on top, so a row resting on its floor is one border taller than the
            // floor. Measured by sweeping the border width against a fixed w:trHeight — see
            // `dotnet/probes/words-pagination-01/row-min-height-border.py`, which reads 24.00 / 24.50 / 25.00 /
            // 26.00 / 27.00 pt out of the reference for w:sz 0 / 4 / 8 / 16 / 24 against a 24 pt floor, while
            // we read 24.00 throughout. That the gap tracks the border exactly is what rules out the other
            // reading of the same two corpus observations — both `ESPN-R - MCF - Manual` and the FAA Holdover
            // Tables draw a w:sz="4" grid, so a flat half point fits them just as well and is refuted here.
            //
            // `exact` is the other branch and measured the other way: at w:sz="16" both sides read 24.00, so a
            // clipped row's height really is the whole of it, borders included. Applying the border there too
            // would be the obvious symmetry and is wrong.
            //
            // **The cell margins go on top of the floor as well**, and that probe could not see it because its
            // cells had none. `lcl_CalcMinRowHeight` (`sw/source/core/layout/tabfrm.cxx`:5087-5097) is the
            // whole rule, under `MIN_ROW_HEIGHT_INCL_BORDER` — which both Word filters turn on and neither ODF
            // nor the Writer default does (`ww8par.cxx`:1966, `DomainMapper.cxx`:156, comment "handle MS Word
            // 'atLeast' oddities"):
            //
            //     nHeight  = trHeight
            //     nHeight += lcl_GetTopSpace(row)        // top border width *and* top margin
            //     nHeight += lcl_GetBottomLineDist(row)  // the bottom margin, and not the bottom border
            //
            // and `lcl_GetFixedRowHeight` (:5058) does the second of those two for an `exact` row.
            // Both maxima are over the row's cells, since one tall margin governs the row.
            //
            // `MIN_ROW_HEIGHT_INCL_BORDER` is a per-document setting and the three Word filters set it
            // while the ODF one does not, so this is gated on `PageTable.MinHeightIncludesInsets` rather
            // than applied to every table — measured, not assumed: applying it to all five formats fixed
            // the `table-exact-row` fidelity fixture's `.doc`, `.docx` and `.rtf` and broke its `.odt`
            // and `.fodt`, which is the setting's own split exactly.
            //
            // Measured on `FAA 2025-26 Holdover Tables.docx`, whose Table 54 rows state `w:trHeight
            // w:val="397"` and `w:tcMar` top 23, bottom 0, under a `w:sz="8"` grid: 397 + (20 + 23) + 0 = 440
            // twips = 22.00 pt, which is the reference's row pitch to the hundredth. Without the margins we
            // read 417 twips = 20.85, and eleven rows of 1.15 pt is the 12.7 pt that lets a three-line
            // paragraph onto a page the reference had to break before.
            Length topInset = table.MinHeightIncludesInsets ? TopInset(table.Rows[row]) : Length.Zero;
            Length bottomInset =
                table.MinHeightIncludesInsets ? BottomInset(table.Rows[row]) : Length.Zero;

            // The floor and the content do not carry the same border term, and that is why round 131
            // could fit no linear function of the two facing widths: a `w:trHeight` floor is raised by the
            // row's OWN STATED top rule while the content is pushed down by the RESOLVED band drawn at that
            // boundary. `probes/tablerow-r146/results.md` §3.4 measures the two arms that separate them.
            Length band = TopBand(row > 0 ? table.Rows[row - 1] : null, table.Rows[row]);
            Length statedRule = table.MinHeightIncludesInsets ? OwnTopRule(table.Rows[row]) : Length.Zero;

            heights[row] = table.Rows[row].HasExactHeight
                ? Length.Max(Length.Zero, table.Rows[row].MinHeight + bottomInset)
                : Length.Max(
                    heights[row] + band,
                    table.Rows[row].MinHeight + statedRule + topInset + bottomInset);

            measuredRowTop += heights[row];
        }

        // A merged cell may still need more room than the rows it covers add up to, so the last row it
        // covers takes the difference. Done after every row has its own floor, since the sum is what
        // decides whether there is a shortfall at all.
        foreach (Measured cell in measured)
        {
            if (cell.LastRow == cell.Row) continue;

            Length covered = Length.Zero;
            for (int row = cell.Row; row <= cell.LastRow; row++) covered += heights[row];

            // An exact row does not grow, so a merge ending in one has nowhere to put its shortfall and its
            // content overflows. Skipping the row rather than growing it is the whole point of "exact".
            if (table.Rows[cell.LastRow].HasExactHeight) continue;

            Length needed = cell.TextHeight + cell.Cell.Padding.Vertical;
            if (needed > covered) heights[cell.LastRow] += needed - covered;
        }

        // Pass two: the heights are settled, so every cell has a rectangle.
        List<Length> tops = [];

        // The table's top edge is the first boundary itself, and the first row's band hangs downwards from
        // it into the row -- which is why the row paid for that band above. [bin] `align.docx`'s outer top
        // rule is a band 82.401..83.401 on a table whose frame top is 82.401.
        Length top = Length.Zero;

        for (int row = 0; row < rows; row++)
        {
            tops.Add(top);
            top += heights[row];
        }

        List<PlacedTableCell> placed = new(measured.Count);
        foreach (Measured cell in measured)
        {
            Length height = tops[cell.LastRow] + heights[cell.LastRow] - tops[cell.Row];

            DocRect area = new(
                origin.X + table.LeftWithin(available ?? table.Width) + lefts[cell.Cell.Column],
                origin.Y + tops[cell.Row],
                cell.Width,
                height);

            // The whole band is inside this row, at its top; the row below pays its own, and the table's
            // outer bottom band hangs below the last row rather than inside it. So nothing is charged here.
            Length bandAbove = TopBand(cell.Row > 0 ? table.Rows[cell.Row - 1] : null, table.Rows[cell.Row]);
            Length bandBelow = Length.Zero;

            if (cell.Cell.IsTurned)
            {
                (PlacedFlow? turned, AffineTransform? onto) = Turned(
                    cell.Cell, area, bandAbove, bandBelow, nesting, collapsesSpacing, addsCellLineSpacing);

                placed.Add(new PlacedTableCell
                {
                    Cell = cell.Cell,
                    Area = area,
                    Content = turned,
                    ContentTransform = onto,
                    Row = cell.Row,
                });

                continue;
            }

            placed.Add(new PlacedTableCell
            {
                Cell = cell.Cell,
                Area = area,
                Content = Positioned(cell, area, bandAbove, bandBelow),
                Row = cell.Row,
            });
        }

        // The one band no rectangle covers: the table's outer bottom rule, which hangs below the last row
        // because there is no row beneath it to pay for it. The outer top rule needs no counterpart — it is
        // the first row's own `TopBand` and is already inside `heights[0]`. Charged after the rectangles are
        // built, so the last row's cells keep their own extent and only the table's total height grows.
        if (rows > 0) heights[rows - 1] += BottomBand(table.Rows[rows - 1]);

        return (placed, heights);
    }

    /// <summary>
    /// One row's cells restricted to the part of the row that goes on a single page.
    /// </summary>
    /// <remarks>
    /// Positioned with the part's own top at nought and the table's left edge at nought, exactly as
    /// <see cref="LayOut"/> leaves a whole table, so the caller moves it onto the page with
    /// <see cref="Offset"/> and nothing here needs to know where the page is.
    /// </remarks>
    /// <param name="Cells">The cells, each holding only the lines that belong to this part.</param>
    /// <param name="Height">How tall the part is, borders included.</param>
    /// <param name="Cut">
    /// The row-relative depth everything above which is now drawn. Handed back to
    /// <see cref="SliceRow"/> for the next page's part, and equal to <see cref="Height"/> only by
    /// coincidence — the part is as tall as its tallest cell needs, and the cut is where the text stopped.
    /// </param>
    /// <param name="IsComplete">True when nothing of the row is left over.</param>
    public readonly record struct RowSlice(
        List<PlacedTableCell> Cells, Length Height, Length Cut, bool IsComplete);

    /// <summary>
    /// Takes the part of a row that fits in a given height, starting below what is already drawn.
    /// </summary>
    /// <remarks>
    /// <para>
    /// What lets a table row cross a page break, which Writer does through a <em>follow flow line</em>:
    /// <c>SwTabFrame::Split</c> keeps the rows that fit in the master table and hands the first one that
    /// does not to <c>lcl_InsertNewFollowFlowLine</c>, which builds a second frame for the same row on the
    /// next page and moves whatever text did not fit into it
    /// (<c>sw/source/core/layout/tabfrm.cxx</c>). This is that in one function: the lines above the cut
    /// stay, the rest are the next page's problem.
    /// </para>
    /// <para>
    /// The cut is one depth for the whole row rather than a per-cell allowance, which is the point rather
    /// than a simplification — a row has one bottom edge, and choosing per cell would let one cell's text
    /// run past the edge another cell's stopped at. So the candidate cuts are the line bottoms of every
    /// cell together, and the deepest one whose part still fits is taken.
    /// </para>
    /// <para>
    /// Returns null when the row must move whole: a cell holding a nested table (Writer's
    /// <c>bTableLayoutTooComplex</c>), a cell merged across this row, a row of a stated exact height, or a
    /// cut that would leave nothing on either side of it. The last is what stops a split that gains
    /// nothing — a page ending in an empty row followed by the same row again does not terminate.
    /// </para>
    /// </remarks>
    /// <param name="row">The row, for its borders and its height rule.</param>
    /// <param name="cells">Its cells as <see cref="LayOut"/> placed them.</param>
    /// <param name="drawn">How far into the row an earlier page already reached; nought at its first part.</param>
    /// <param name="room">How much height is left on this page.</param>
    /// <param name="acrossRowSpans">
    /// Whether a cell merged down past this row may be cut as well. False everywhere but the one case
    /// where declining hides content — see the remark on the row-span test below.
    /// </param>
    /// <param name="keepsSpacingAtPages">
    /// The document's <c>PARA_SPACE_MAX_AT_PAGES</c> — <see cref="UpperSpaceAbove"/>, which is the only
    /// thing it reaches here. A document without it gives a follow part no upper space at all.
    /// </param>
    /// <param name="bandAbove">
    /// The resolved band at the boundary above this row, which the part occupies as well as its own
    /// content — <see cref="BoundaryBand"/>, because this method is handed one row and cannot resolve it.
    /// Nought is safe and is what a caller measuring a borderless table passes; it costs a part one
    /// line's worth of room only where the boundary is really ruled.
    /// </param>
    public static RowSlice? SliceRow(
        PageTableRow row,
        IReadOnlyList<PlacedTableCell> cells,
        Length drawn,
        Length room,
        bool acrossRowSpans = false,
        bool keepsSpacingAtPages = true,
        Length bandAbove = default)
    {
        ArgumentNullException.ThrowIfNull(row);
        ArgumentNullException.ThrowIfNull(cells);

        if (cells.Count == 0 || row.HasExactHeight) return null;

        Length rowTop = cells[0].Area.Y;
        foreach (PlacedTableCell cell in cells)
        {
            // A cell covering more than this row is normally not cut here: its text belongs to a row
            // further down, and half of its rectangle would be drawn on each of two pages.
            //
            // The exception is the row that is taller than a whole column, where declining is worse than
            // the artefact. Writer allows that row to split "regardless of setting, otherwise it has
            // hidden content and that makes no sense" (`SwTabFrame::Split`, `tabfrm.cxx`:1161) and
            // re-formats the spanned cells with `lcl_AdjustRowSpanCells`; declining leaves the row to be
            // placed whole on a page it cannot fit, which draws it past the bottom of the paper and
            // wastes the page before it. Measured on `ESPN-R - MCF - RA - Ed1.docx`, whose "Engine -
            // Flight" row is 440.5 pt under a 422.2 pt landscape body and whose first cell is merged
            // down into the rows after it: about 356 pt of page 27 went blank and the row was then drawn
            // to y = 30.6 on a page whose bottom margin is at 56.7.
            if (Math.Max(1, cell.Cell.RowSpan) > 1 && !acrossRowSpans) return null;
            rowTop = Length.Min(rowTop, cell.Area.Y);
        }

        // What a PART of this row pays for its rules, and at a cut that is the row's own BOTTOM rule
        // rather than its top.
        //
        // [src] `lcl_IsFirstRowInFollowTableWithoutRepeatedHeadlines`
        // (`sw/source/core/layout/paintfrm.cxx`:2815-2833, used at :3089): the first row of a follow
        // table draws its **top** line from the cell's **bottom** border style. The master's own foot
        // draws that same bottom rule, so both halves of a split row are ruled by it and each pays for
        // it once. [bin] `probes/tablerow-r146/results.md` §4 on `split.docx`, whose cells state
        // `w:top nil` and a 1 pt `w:bottom`: 26.2.4.2 rules the foot of page 1 at 759.201..760.201 and
        // the head of page 2 at 70.901..71.901, both 1 pt, both hanging downwards from the frame edge,
        // and the follow's first line starts at 71.901 -- so the follow really is charged the whole of
        // it. Charging the row's own TOP here instead, which is what this did until round 152, charges
        // nothing at all for a table ruled the way this one is, and one line too many then fits above
        // the cut.
        Length border = BottomBand(row);

        // What the part pays at its HEAD, and the two parts of a split row do not pay the same thing.
        // The row's FIRST part begins on the row's own boundary, so it occupies the band there --
        // `bandAbove`, which the caller resolves because this method is handed one row. A FOLLOW part
        // begins at the top of a page with no boundary above it, and what is ruled at its head is the
        // CUT's rule: [src] `lcl_IsFirstRowInFollowTableWithoutRepeatedHeadlines` has the first row of a
        // follow table draw its top line from the cell's BOTTOM border style. The two coincide on
        // `split.docx`, whose every boundary is 1 pt, which is why charging `bandAbove` to both looked
        // right there and cost `review-welsh-...-mandelson.docx` a page it should not have.
        Length head = drawn > Length.Zero ? border : bandAbove;
        Length above = rowTop + drawn;

        // The spans a cut may not fall inside: a table nested in a cell is placed as one rectangle and
        // this function has no machinery to divide one, so a cut through it would draw the top half on
        // one page and nothing on the other. See `Nested`.
        List<(Length Top, Length Bottom)> whole = [];
        foreach (PlacedTableCell cell in cells)
        {
            if (cell.ContentTransform is not null || cell.Content is not { } flow) continue;
            foreach (PlacedTable nested in flow.Tables) whole.Add((nested.Area.Y, nested.Area.Bottom));
        }

        // Every line that is not yet drawn, as the depth its bottom sits at. These are the only places the
        // row may be cut, since a cut between two of them would draw half a line — plus the bottom of each
        // nested table, which is a legal cut and often the only one a densely nested cell offers.
        List<Length> candidates = [];
        foreach (PlacedTableCell cell in cells)
        {
            // A turned cell offers no cuts: its lines run across the row rather than down it, so there is
            // no depth at which cutting one would divide its text. It rides on the row's first part.
            if (cell.ContentTransform is not null) continue;
            if (cell.Content is not { } flow) continue;

            foreach (PlacedLine line in flow.Lines)
            {
                Length bottom = flow.Area.Y + line.Top + line.Box.Height;
                if (bottom > above) candidates.Add(bottom);
            }

            foreach (PlacedTable nested in flow.Tables)
            {
                if (nested.Area.Bottom > above) candidates.Add(nested.Area.Bottom);
            }
        }

        candidates.RemoveAll(cut => Crosses(whole, cut));

        if (candidates.Count == 0) return null;

        candidates.Sort();

        // Whether anything is left over, asked of the content rather than of the candidate list: a cut a
        // nested table forbade is absent from that list, and reading the deepest *legal* cut as the
        // deepest content would report a part complete while a whole nested table still waited below it.
        //
        // Hoisted above the search because `Fits` needs it: it does not depend on the cut.
        Length last = Length.Zero;
        foreach (PlacedTableCell cell in cells)
        {
            if (cell.ContentTransform is not null || cell.Content is not { } flow) continue;

            foreach (PlacedLine line in flow.Lines)
            {
                last = Length.Max(last, flow.Area.Y + line.Top + line.Box.Height);
            }

            foreach (PlacedTable nested in flow.Tables) last = Length.Max(last, nested.Area.Bottom);
        }

        // What a part cut here costs, or null when it does not fit.
        //
        // A part that does NOT finish the row is measured twice. The reference prefers to carry the
        // trailing `w:spacing w:after` of the cells that finished above the cut, and drops it whole —
        // never clipped — when the part would not otherwise fit, taking the split anyway rather than
        // moving the row. A part that DOES finish the row is not a split: it owes that spacing exactly
        // as `LayOut` charged the row for it, so it is measured once and moves whole if it will not fit.
        //
        // [bin] 26.2.4.2 on a three-row two-column fixture with `w:spacing before=40 line=233`, `after`
        // swept and the bottom margin swept in 10 twip steps so the room varies continuously: the master
        // part measures 26.90 (before + line + after + border) while the room allows it and 14.90
        // (before + line + border) once it does not, with no intermediate value anywhere in the sweep.
        // The same all-or-nothing shape at `after=40`. See `probes/orphan3-r163/`.
        //
        // Both measures are non-decreasing in the cut, so `break` still ends the search at the first
        // candidate that fails.
        Length? Fits(Length candidate)
        {
            Length needed = head + HeightAt(
                cells, rowTop, above, candidate, border, keepsSpacingAtPages, chargeTrailing: true);
            if (needed <= room) return needed;

            if (candidate >= last) return null;

            needed = head + HeightAt(
                cells, rowTop, above, candidate, border, keepsSpacingAtPages, chargeTrailing: false);
            return needed <= room ? needed : null;
        }

        // The deepest cut whose part still fits. The height is not decreasing in the cut, so the first
        // candidate that does not fit ends the search.
        Length? chosen = null;
        Length height = Length.Zero;

        foreach (Length candidate in candidates)
        {
            if (chosen is { } already && already == candidate) continue;
            if (Fits(candidate) is not { } needed) break;

            chosen = candidate;
            height = needed;
        }

        // Nothing was cuttable at this level. Descend into the nested tables and look for a cut inside
        // one, which is the only thing that lets a row split when a cell holds a nested table taller than
        // the page it must fit on.
        //
        // **Only when the ordinary search found nothing**, which confines this to exactly the rows that
        // are broken today: a row with no legal cut is placed whole on a page that cannot hold it, and
        // whatever runs past the bottom is neither drawn nor carried. Every row that splits today finds
        // its cut in the list above and never reaches here, so no document that works can be moved by it.
        // That is the same argument the row-span exception above already makes — declining is worse than
        // the artefact when declining hides content.
        //
        // Measured on `May 25 bulletin focus on carers in the workplace.docx`, a chain of single-row,
        // single-cell wrapper tables nested five deep. Every `SliceRow` call on it reported one cell, one
        // nested span of 0.0..1126.4 and a single candidate at that span's bottom — against a body 697.9
        // pt tall, so the one legal cut was unreachable even on a completely empty page and the row could
        // never be split at all. 86 words fell off the end. See `probes/nested-table-slice/`.
        //
        // Writer never needs this: `bTableLayoutTooComplex` (`sw/source/core/layout/tabfrm.cxx`:586-594,
        // gating the split at :611-613) makes it refuse to cut such a row too, but its nested table is a
        // frame in its own right and splits across the page through its own follow. Ours is a rectangle
        // inside a cell's flow, placed whole or not at all — so the recursion has to happen here instead.
        bool fromDeepSearch = false;

        if (chosen is null)
        {
            List<Length> deep = [];
            foreach (PlacedTableCell cell in cells)
            {
                if (cell.ContentTransform is not null || cell.Content is not { } flow) continue;

                foreach (PlacedTable nested in flow.Tables) DeepCuts(nested, above, deep);
            }

            deep.Sort();

            foreach (Length candidate in deep)
            {
                if (chosen is { } already && already == candidate) continue;
                if (Fits(candidate) is not { } needed) break;

                chosen = candidate;
                height = needed;
                fromDeepSearch = true;
            }
        }

        if (chosen is not { } cut) return null;

        // A nested table contributes its whole rectangle's bottom to `last`, which is right while a cut
        // can only fall between nested tables: the part is incomplete exactly when one is still below it.
        // A cut *inside* one breaks that reading — the rectangle's bottom stays where it always was
        // however much of the table has been drawn, so the last part reports itself incomplete, the
        // caller asks for one more page, and the band left over holds no lines and draws nothing.
        //
        // That is where the bulletin's spurious fifth page came from. Asked of the lines instead the
        // answer is right, and it is asked only of a cut the deep search chose, so a row that splits the
        // ordinary way keeps the rectangle test exactly as it was.
        bool complete = last <= cut || (fromDeepSearch && !HasLinesBelow(cells, cut));

        // A part holding every remaining line is not a split at all; the caller places the whole row.
        if (complete && drawn <= Length.Zero) return null;

        // Only a part the page CUTS pays a band at its foot. A part that finishes the row pays none:
        // the boundary below it is an ordinary one and the row after it pays for it as `TopBand`, so
        // charging it here as well rules that boundary twice, one band apart. [bin] on `split.docx`
        // that drew bands at 118.050 and 119.050 where 26.2.4.2 draws one at 118.101, and pushed every
        // later row down with it.
        if (complete) height -= border;

        // The cells are built to the part's DRAWN extent and the part reports the whole of it, which
        // differ by the band at the cut: that band hangs below the last line rather than inside the
        // rectangle, exactly as the table's own outer bottom band does in `LayOut` -- charged after the
        // rectangles are built so the caller advances past it while no cell claims it. Building the
        // rectangles to the full height instead draws the cut's rule one band too low and starts the
        // follow part one band too far down, which is the +0.999 this round measured before fixing it.
        return new RowSlice(
            Sliced(cells, rowTop, above, cut, complete ? height : height - border, keepsSpacingAtPages),
            height, cut - rowTop, complete);
    }

    /// <summary>
    /// How tall the row's part is when it is cut at a stated depth.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The same sum <see cref="LayOut"/> makes a whole row from — the tallest cell's text plus its
    /// padding, plus the row's borders — over the lines this part holds. The row's declared floor is
    /// deliberately <em>not</em> applied: it is a floor on the row and the parts add up to the row, so
    /// imposing it on each would make a split row twice as tall as an unsplit one. A row whose floor
    /// exceeds its text never reaches here anyway, since every line then fits in one part and
    /// <see cref="SliceRow"/> declines a split that leaves nothing over.
    /// </para>
    /// <para>
    /// The two ends of the run are measured as <see cref="LayOut"/> measures them rather than as ink, and
    /// that is what makes the two agree. A cell's flow does not begin at its first line — a first
    /// paragraph's space-before sits above it — and it does not end at its last, since
    /// <see cref="PlacedFlow.Advance"/> carries the last paragraph's space-after, which is exactly what
    /// a row's height is built from. So the row's own first part starts at the flow's top, and a part
    /// holding a cell's last line ends at its advance. Measuring both from the ink instead makes the sum
    /// of a row's parts shorter than the row, and then a row whose text fits the page but whose *height*
    /// does not is judged unbreakable by one measure and too tall by the other: it moves whole and leaves
    /// the difference blank. Measured on
    /// <c>f445896eb008d14c1746fc37d412dc22.docx</c>, where 205.8 pt of a page went empty because the row
    /// was 211.8 pt tall and its lines measured 202.5.
    /// </para>
    /// <para>
    /// A <em>follow</em> part starts at the top of the block its first line belongs to and not at the top
    /// of that line — see <see cref="UpperSpaceAbove"/>. That is one line of arithmetic and it is the
    /// difference between a split row's parts adding up to the row and adding up to a point or two less.
    /// </para>
    /// </remarks>
    private static Length HeightAt(
        IReadOnlyList<PlacedTableCell> cells,
        Length rowTop,
        Length above,
        Length cut,
        Length border,
        bool keepsSpacingAtPages,
        bool chargeTrailing)
    {
        // The row's own first part keeps the offset it was laid out with — see `Sliced` — so its cells
        // begin at the top of their flow and not at their first line.
        bool isFirst = above <= rowTop;
        Length text = Length.Zero;

        foreach (PlacedTableCell cell in cells)
        {
            // Charges the part nothing, exactly as it charged the row nothing. See `Turned`.
            if (cell.ContentTransform is not null) continue;
            if (cell.Content is not { } flow) continue;

            Length? top = null;
            Length bottom = Length.Zero;
            PlacedLine? following = null;

            foreach (PlacedLine line in flow.Lines)
            {
                Length end = flow.Area.Y + line.Top + line.Box.Height;
                if (end <= above) continue;

                if (end > cut)
                {
                    following ??= line;
                    continue;
                }

                top ??= isFirst
                    ? flow.Area.Y
                    : flow.Area.Y + line.Top - UpperSpaceAbove(flow, line, keepsSpacingAtPages);
                bottom = end;
            }

            // A nested table this part holds whole is as much of the cell's height as a line is, and it
            // can be the *only* thing in the part — a cell that opens with a picture in a one-cell table
            // has no line above it at all, so the part would otherwise measure nothing and be rejected.
            bool tableBelow = false;
            foreach (PlacedTable nested in flow.Tables)
            {
                if (nested.Area.Bottom > cut)
                {
                    // A nested table the cut falls *inside* contributes the part above the cut. Without
                    // this the part measures nothing at a deep cut — a wrapper cell has no lines of its
                    // own — so every candidate would appear to fit and the deepest would be chosen,
                    // which is a part taller than the page. Only reachable through the deep-cut search
                    // below; the ordinary candidate list never crosses a nested table.
                    if (nested.Area.Y < cut)
                    {
                        Length straddleTop = isFirst ? flow.Area.Y : Length.Max(nested.Area.Y, above);
                        top = top is { } sofar ? Length.Min(sofar, straddleTop) : straddleTop;
                        bottom = Length.Max(bottom, cut);
                    }

                    tableBelow = true;
                    continue;
                }

                if (nested.Area.Bottom <= above) continue;
                if (nested.Area.Y < above && SliceNested(nested, above, cut) is null) continue;

                Length nestedTop = isFirst ? flow.Area.Y : nested.Area.Y;
                top = top is { } already ? Length.Min(already, nestedTop) : nestedTop;
                bottom = Length.Max(bottom, nested.Area.Bottom);
            }

            if (top is not { } start) continue;

            if (following is { } next)
            {
                // A part ends where the next block begins, not where its own last line does — so a part
                // whose last line completes a paragraph is charged that paragraph's space-after, the same
                // way `Advance` charges the cell for its final one. Within a paragraph the next line's
                // block top *is* this line's bottom and this changes nothing.
                bottom = Length.Max(bottom, flow.Area.Y + next.Top - next.UpperSpace);
            }
            else if (chargeTrailing
                     && !tableBelow
                     && flow.Lines.Count > 0
                     && flow.Area.Y + flow.Lines[^1].Top + flow.Lines[^1].Box.Height <= cut)
            {
                // Nothing of this cell is left over, so its part is as tall as the cell — the trailing
                // spacing included, which is what `LayOut` charged the row for. `tableBelow` is what
                // keeps that from firing on a cell whose *lines* all fit while a nested table below them
                // does not: charging the whole cell's advance there makes the part as tall as the row
                // and nothing ever fits.
                //
                // `chargeTrailing` is false on the second measure a split takes when the first did not
                // fit: the reference drops this spacing whole rather than move the row. See `Fits`.
                bottom = Length.Max(bottom, flow.Area.Y + flow.Advance);
            }

            text = Length.Max(text, bottom - start + cell.Cell.Padding.Vertical);
        }

        return text + border;
    }

    /// <summary>
    /// The upper spacing a follow part re-applies above its first line.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A part of a split row that is not the row's first begins at the top of the <em>block</em> its first
    /// line belongs to, which is one paragraph's space-before above the line. So a paragraph cut in half
    /// by a page break pays its space-before twice — once on each part — and a paragraph that merely
    /// starts a follow part pays it there rather than losing it.
    /// </para>
    /// <para>
    /// Both halves of that are measured rather than reasoned, on 12 authored probes rendered through
    /// LibreOffice 26.2.4.2 (<c>dotnet/probes/words-regress-01/probe-rowsplit-spacing.py</c> and
    /// <c>probe-rowsplit-paras.py</c>). Sweeping <c>w:spacing w:before</c> and <c>w:after</c>
    /// independently over a row cut across a page, the reference's follow part is
    /// <c>before + remaining lines + after + border</c> in every combination, and the sum of a split
    /// row's two parts therefore exceeds the unsplit row by exactly <c>before</c>. This is Writer's
    /// <c>AddParaSpacingToTableCells</c> compatibility setting seen from the other end:
    /// <see cref="PlacedFlow.Advance"/> already charges a cell for its last paragraph's space-after
    /// because of it, and <c>SwFlowFrame::CalcUpperSpace</c> is why a follow keeps its upper space in a
    /// table cell where an ordinary body paragraph would drop it.
    /// </para>
    /// <para>
    /// <strong>It is the paragraph's own <c>w:spacing w:before</c> and not the gap the flow was laid out
    /// with, and the two differ whenever the paragraph above it states a space-after.</strong>
    /// <c>SwFlowFrame::CalcUpperSpace</c> (<c>sw/source/core/layout/flowfrm.cxx:1589,1744-1755</c>) asks
    /// <c>GetPrevFrameForUpperSpaceCalc_</c> for the frame above and takes the collapsing branch only
    /// when it finds one. The paragraph that opens a follow part has none — the paragraph above it is in
    /// the master cell frame on the page before — so it falls to the <c>else if</c>, where
    /// <c>nUpper = pAttrs-&gt;GetULSpace().GetUpper()</c> is the paragraph's whole upper space, with
    /// neither the collapse against the previous frame's lower space nor contextual spacing consulted at
    /// all. <c>HasParaSpaceAtPages</c> (<c>:1438</c>) grants it unconditionally to anything
    /// <c>IsInTab</c>, and <c>lcl_PartiallyCollapseUpper</c> (<c>:1541</c>) returns early for a frame in
    /// a table, so nothing takes it away again.
    /// </para>
    /// <para>
    /// Measured on <c>dotnet/probes/words-clip-rowsplit/</c>, a four-paragraph cell cut between two of
    /// its paragraphs, as the distance from the follow part's own top rule to the first baseline on
    /// page 2. Both installed references agree on every row:
    /// </para>
    /// <code>
    ///   before  after   24.2.7.2   26.2.4.2   this engine before
    ///      240      0      12.76      12.76      12.51
    ///        0    240       0.76       0.76       0.51
    ///      240    240      12.76      12.76       0.51
    ///      240    120      12.76      12.76       6.51
    ///      120    240       6.76       6.76       0.51
    /// </code>
    /// <para>
    /// The reference is the paragraph's own <c>before</c> in all five; the old reading was
    /// <c>max(before − after, 0)</c>, which agrees only on the two rows where nothing collapses. The
    /// constant 0.25 pt is where the part's top rule sits and not this spacing — it is there on the
    /// agreeing rows too. On <c>mde087077~283.docx</c> the missing 6 pt put every page after a split row
    /// 6 pt high and fitted one bulletin row too many onto page 3.
    /// </para>
    /// <para>
    /// Only a follow part is affected: the row's own first part starts at the flow's top, which already
    /// holds the first paragraph's space-before.
    /// </para>
    /// </remarks>
    /// <param name="flow">The cell's flow, for the paragraph the line belongs to.</param>
    /// <param name="line">The first line the part holds.</param>
    /// <param name="keepsSpacingAtPages">
    /// The document's <c>PARA_SPACE_MAX_AT_PAGES</c>, which is the gate on the whole <c>else if</c>
    /// branch above. Both OOXML and ODF default it on, so this is false only for a document that turns
    /// it off by hand.
    /// </param>
    private static Length UpperSpaceAbove(
        PlacedFlow flow, PlacedLine line, bool keepsSpacingAtPages)
    {
        if (!keepsSpacingAtPages) return Length.Zero;

        return line.ParagraphIndex >= 0
               && line.ParagraphIndex < flow.Blocks.Count
               && flow.Blocks[line.ParagraphIndex] is PageParagraph paragraph
            ? paragraph.Format.SpaceBefore
            : Length.Zero;
    }

    /// <summary>
    /// Whether a cut would fall strictly inside one of the spans that must not be divided.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Writer's <c>bTableLayoutTooComplex</c> refuses to split a row holding a nested table at all; this
    /// refuses only the cuts that would go <em>through</em> one, which is the same guarantee with the
    /// rest of the row still available. Strict at both ends on purpose: a cut exactly at a nested
    /// table's bottom leaves it whole on the part above, and one exactly at its top leaves it whole on
    /// the part below, so both are legal and one of them is frequently the only cut a nested cell has.
    /// </para>
    /// <para>
    /// It is not the whole of the follow-flow story. Writer builds a follow frame for the nested table
    /// too and divides it in turn, so a document whose *one* nested table is taller than a page still
    /// paginates differently from us. What this removes is the case where content vanished: a row that
    /// cannot be cut anywhere is placed whole on a page of its own and everything past the page bottom
    /// is never drawn. Measured on a two-file probe differing in one nested table — 90 body paragraphs
    /// in a one-cell table render 2 pages and 90 lines both ways without it, and 1 page and 64 lines
    /// with it, the last paragraph gone.
    /// </para>
    /// </remarks>
    /// <summary>
    /// Every depth at which a nested table could be divided, gathered through the tables inside it.
    /// </summary>
    /// <remarks>
    /// The bottom of each of its lines, and the bottom of each table nested inside it — the same two
    /// kinds of candidate <see cref="SliceRow"/> collects at the top level, read one level down. A
    /// wrapper table holding a single row and a single cell offers no cut of its own, which is exactly
    /// the shape this exists for, so the recursion has to reach the lines rather than stopping at the
    /// rows.
    /// </remarks>
    /// <param name="nested">The table to look inside.</param>
    /// <param name="above">Depths at or above this one are already drawn and are not candidates.</param>
    /// <param name="into">The list being gathered into.</param>
    /// <param name="depth">How far the recursion has gone; its guard.</param>
    private static void DeepCuts(PlacedTable nested, Length above, List<Length> into, int depth = 0)
    {
        if (depth > FlowLayouter.MaxNesting) return;

        foreach (PlacedTableCell cell in nested.Cells)
        {
            // A turned cell offers no cuts, for the reason `SliceRow` gives: its lines run across the row
            // rather than down it.
            if (cell.ContentTransform is not null || cell.Content is not { } flow) continue;

            foreach (PlacedLine line in flow.Lines)
            {
                Length bottom = flow.Area.Y + line.Top + line.Box.Height;
                if (bottom > above) into.Add(bottom);
            }

            foreach (PlacedTable inner in flow.Tables)
            {
                if (inner.Area.Bottom > above) into.Add(inner.Area.Bottom);

                DeepCuts(inner, above, into, depth + 1);
            }
        }
    }

    /// <summary>
    /// The part of a nested table lying between two depths, or null when none of it does.
    /// </summary>
    /// <remarks>
    /// The counterpart of <see cref="Sliced"/> one level down, and it keeps page coordinates: the caller
    /// shifts the whole thing by the same <c>dy</c> it shifts a nested table it kept whole, so nothing
    /// here has to know where the part will land.
    /// </remarks>
    /// <param name="nested">The table being divided.</param>
    /// <param name="above">The top of the band to keep.</param>
    /// <param name="cut">Its bottom.</param>
    /// <param name="depth">How far the recursion has gone; its guard.</param>
    private static PlacedTable? SliceNested(PlacedTable nested, Length above, Length cut, int depth = 0)
    {
        if (depth > FlowLayouter.MaxNesting) return null;

        List<PlacedTableCell> kept = [];

        foreach (PlacedTableCell cell in nested.Cells)
        {
            // Undividable, so it rides on whichever part holds its top.
            if (cell.ContentTransform is not null)
            {
                if (cell.Area.Y >= above && cell.Area.Y < cut) kept.Add(cell);
                continue;
            }

            List<PlacedLine> lines = [];
            List<PlacedTable> tables = [];

            if (cell.Content is { } flow)
            {
                foreach (PlacedLine line in flow.Lines)
                {
                    Length bottom = flow.Area.Y + line.Top + line.Box.Height;
                    if (bottom > above && bottom <= cut) lines.Add(line);
                }

                foreach (PlacedTable inner in flow.Tables)
                {
                    if (inner.Area.Bottom <= above || inner.Area.Y >= cut) continue;

                    PlacedTable? part = inner.Area.Y >= above && inner.Area.Bottom <= cut
                        ? inner
                        : SliceNested(inner, above, cut, depth + 1);

                    if (part is not null) tables.Add(part);
                }
            }

            if (lines.Count == 0 && tables.Count == 0) continue;

            Length top = Length.Max(cell.Area.Y, above);
            Length bottom2 = Length.Min(cell.Area.Bottom, cut);

            kept.Add(cell with
            {
                Area = new DocRect(cell.Area.X, top, cell.Area.Width, bottom2 - top),
                Content = cell.Content is { } text
                    ? text with { Lines = lines, Tables = tables }
                    : null,
            });
        }

        if (kept.Count == 0) return null;

        Length areaTop = Length.Max(nested.Area.Y, above);
        Length areaBottom = Length.Min(nested.Area.Bottom, cut);

        return nested with
        {
            Area = new DocRect(nested.Area.X, areaTop, nested.Area.Width, areaBottom - areaTop),
            Cells = kept,
        };
    }

    /// <summary>Whether any line anywhere inside these cells ends below a depth.</summary>
    /// <remarks>
    /// The content test that <see cref="SliceRow"/>'s completeness check falls back on when the cut was
    /// taken inside a nested table. Lines rather than rectangles, because a nested table's rectangle
    /// tells you where it was laid out and not how much of it is still waiting.
    /// </remarks>
    private static bool HasLinesBelow(IReadOnlyList<PlacedTableCell> cells, Length cut, int depth = 0)
    {
        if (depth > FlowLayouter.MaxNesting) return false;

        foreach (PlacedTableCell cell in cells)
        {
            if (cell.Content is not { } flow) continue;

            foreach (PlacedLine line in flow.Lines)
            {
                if (flow.Area.Y + line.Top + line.Box.Height > cut) return true;
            }

            foreach (PlacedTable nested in flow.Tables)
            {
                if (HasLinesBelow(nested.Cells, cut, depth + 1)) return true;
            }
        }

        return false;
    }

    private static bool Crosses(List<(Length Top, Length Bottom)> spans, Length cut)
    {
        foreach ((Length top, Length bottom) in spans)
        {
            if (cut > top && cut < bottom) return true;
        }

        return false;
    }

    /// <summary>The cells of one part, holding its lines and positioned from the part's own top.</summary>
    private static List<PlacedTableCell> Sliced(
        IReadOnlyList<PlacedTableCell> cells,
        Length rowTop,
        Length above,
        Length cut,
        Length height,
        bool keepsSpacingAtPages)
    {
        List<PlacedTableCell> sliced = new(cells.Count);

        foreach (PlacedTableCell cell in cells)
        {
            // A turned cell goes whole onto the row's first part and is absent from every later one. It
            // cannot be divided — see `SliceRow` — and drawing it twice would print the label on both
            // pages. Its own rectangle still becomes the part's, so the borders round it are right.
            if (cell.ContentTransform is { } onto)
            {
                bool isFirstPart = above <= rowTop;

                sliced.Add(cell with
                {
                    Area = new DocRect(cell.Area.X, Length.Zero, cell.Area.Width, height),
                    Content = isFirstPart ? cell.Content : null,
                    ContentTransform = isFirstPart
                        ? onto with { F = onto.F - rowTop.Emu }
                        : null,
                });

                continue;
            }

            List<PlacedLine> kept = [];
            List<PlacedTable> keptTables = [];
            Length? first = null;

            if (cell.Content is { } flow)
            {
                foreach (PlacedLine line in flow.Lines)
                {
                    Length end = flow.Area.Y + line.Top + line.Box.Height;
                    if (end <= above || end > cut) continue;

                    // Less the space the part is charged for above it, so the text lands where the height
                    // `HeightAt` reported puts it. See `UpperSpaceAbove`.
                    first ??= line.Top - UpperSpaceAbove(flow, line, keepsSpacingAtPages);
                    kept.Add(line);
                }

                // A nested table belongs to exactly one part — the one whose span holds it whole. `SliceRow`
                // never chooses a cut that crosses one, so every nested table is above the cut or below it
                // and none is drawn twice or lost between the two.
                foreach (PlacedTable nested in flow.Tables)
                {
                    // Wholly outside the band this part covers.
                    if (nested.Area.Bottom <= above || nested.Area.Y >= cut) continue;

                    // Whole inside it, or divided by one end of it. The second case is new and is only
                    // reachable through the deep-cut search in `SliceRow`: the ordinary candidate list
                    // never crosses a nested table, so before that search existed every nested table was
                    // wholly above the cut or wholly below it.
                    PlacedTable? part = nested.Area.Y >= above && nested.Area.Bottom <= cut
                        ? nested
                        : SliceNested(nested, above, cut);

                    if (part is null) continue;

                    keptTables.Add(part);
                    Length start = part.Area.Y - flow.Area.Y;
                    first = first is { } already ? Length.Min(already, start) : start;
                }
            }

            DocRect area = new(cell.Area.X, Length.Zero, cell.Area.Width, height);
            PlacedFlow? content = null;

            if (cell.Content is { } text && (kept.Count > 0 || keptTables.Count > 0))
            {
                // The remaining text starts at the top of the part rather than where it was measured, which
                // is what a follow flow line is: the cell begins again on the next page. The first part
                // keeps the offset it was laid out with, so a short cell beside a long one stays where its
                // vertical alignment put it.
                Length top = above <= rowTop
                    ? text.Area.Y - rowTop
                    : cell.Cell.Padding.Top - first!.Value;

                // A line is positioned relative to the flow's rectangle and a nested table is not — its
                // cells carry page coordinates, exactly as `ShiftFlow` records — so moving the flow's top
                // takes the lines along and leaves the tables where the pre-layout pass put them. They are
                // shifted by the same amount explicitly.
                Length dy = top - text.Area.Y;

                content = text with
                {
                    Area = new DocRect(text.Area.X, top, text.Area.Width, height),
                    Lines = kept,
                    Tables = keptTables.Count == 0
                        ? []
                        : [.. keptTables.Select(nested => nested with
                        {
                            Area = Shift(nested.Area, Length.Zero, dy),
                            Cells = Offset(nested.Cells, Length.Zero, dy),
                        })],
                };
            }

            sliced.Add(cell with { Area = area, Content = content });
        }

        return sliced;
    }

    /// <summary>
    /// The same cells, shifted.
    /// </summary>
    /// <remarks>
    /// What lets a table be laid out once and placed many times: a cell's lines are positioned relative to
    /// its content rectangle, so moving the rectangle takes the text with it and nothing needs measuring
    /// again. Used by the paginator both to move a table onto a page and to draw a repeated heading row
    /// part way down one.
    /// </remarks>
    /// <param name="cells">The cells to shift.</param>
    /// <param name="dx">How far right.</param>
    /// <param name="dy">How far down.</param>
    public static List<PlacedTableCell> Offset(
        IEnumerable<PlacedTableCell> cells, Length dx, Length dy)
    {
        ArgumentNullException.ThrowIfNull(cells);

        List<PlacedTableCell> moved = [];
        foreach (PlacedTableCell cell in cells)
        {
            // A turned cell's content is in the cell's own coordinates, so moving the cell moves its
            // *transform* and must not touch the flow: shifting both would move the text twice.
            if (cell.ContentTransform is { } onto)
            {
                moved.Add(cell with
                {
                    Area = Shift(cell.Area, dx, dy),
                    ContentTransform = onto with { E = onto.E + dx.Emu, F = onto.F + dy.Emu },
                });

                continue;
            }

            moved.Add(cell with
            {
                Area = Shift(cell.Area, dx, dy),
                Content = ShiftFlow(cell.Content, dx, dy),
            });
        }

        return moved;
    }

    /// <summary>
    /// Moves a cell's whole content: its rectangle, and any table nested inside it.
    /// </summary>
    /// <remarks>
    /// The lines need no attention — they are positioned relative to the flow's rectangle, so moving the
    /// rectangle takes them along. A nested table is the exception and the reason this exists: its cells
    /// carry page coordinates rather than flow-relative ones, so they have to be moved by the same amount
    /// explicitly. Missing that leaves a nested table wherever the pre-layout pass put it — which is near
    /// the page's top-left corner, since a table is laid out once at the origin and placed later.
    /// </remarks>
    private static PlacedFlow? ShiftFlow(PlacedFlow? flow, Length dx, Length dy)
    {
        if (flow is null) return null;
        if (dx == Length.Zero && dy == Length.Zero) return flow;

        return flow with
        {
            Area = Shift(flow.Area, dx, dy),
            Tables = [.. flow.Tables.Select(table => table with
            {
                Area = Shift(table.Area, dx, dy),
                Cells = Offset(table.Cells, dx, dy),
            })],
        };
    }

    /// <summary>
    /// The band of border a row pays for: the whole of the resolved rule on the boundary <em>above</em> it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>The row below a boundary pays the whole of its band and the row above pays nothing</strong>, and
    /// the band's top edge sits on the boundary and hangs downwards into that row. [src]
    /// <c>SwTabFramePainter::Insert</c> (<c>sw/source/core/layout/paintfrm.cxx</c>:3061-3064) sets
    /// <c>RefMode::Begin</c> on a cell's two horizontal borders — <em>"drawn below the reference points"</em>,
    /// <c>mfRefModeOffset = +width/2</c> — and <c>Centered</c> on its two vertical ones, and
    /// <c>SwRowFrame::Format</c> (<c>tabfrm.cxx</c>:5338-5430) takes
    /// <c>nTopPrtMargin = max(lcl_GetTopSpace, pPreviousRow-&gt;GetBottomLineSize() + nTopLineDist)</c> while
    /// charging the bottom <em>padding</em> and never the bottom line.
    /// </para>
    /// <para>
    /// [bin] Measured on <c>probes/tablerow-r146/fixtures/height.docx</c>, whose arms vary only the pair of
    /// attributes facing across one boundary: a 3 pt edge stated by the upper row, by the lower row, by both,
    /// and beside a 0.5 pt one are <b>indistinguishable in every figure</b> — the same 3 pt band at the same y
    /// and the same 3.000 pt of table height — and the boundary does not move, so the row above pays not half
    /// but nothing. The width is the <em>resolved</em> rule, the maximum of the two facing statements, and a
    /// cell's own <c>nil</c> beats a table-level <c>w:insideH</c>.
    /// </para>
    /// <para>
    /// The thickest over the row's cells rather than each cell's own, because the row has one height: two
    /// cells disagreeing share the thicker one's band, which is what the drawing does when it consolidates.
    /// </para>
    /// <para>
    /// And over the cells the row <em>dropped</em> as well — see <see cref="PageTableRow.CoveredTopRule"/>.
    /// A covered cell's stated top is charged here and drawn nowhere, which is the one place in this file
    /// where the height and the ink are deliberately not the same set of statements.
    /// </para>
    /// <para>
    /// The model this replaced charged half of each of the two stated rules to each row and drew the bands
    /// centred, which is <em>algebraically identical</em> for as long as every horizontal rule in the table has
    /// one width and the table does not split — which is why the family seated as three separate defects
    /// (O83, O84, O85) rather than as one.
    /// </para>
    /// </remarks>
    /// <param name="above">The row above, or null for the table's first row, whose band is its own top rule.</param>
    /// <param name="row">The row that pays.</param>
    private static Length TopBand(PageTableRow? above, PageTableRow row)
    {
        Length band = row.CoveredTopRule;

        foreach (PageTableCell cell in row.Cells) band = Length.Max(band, cell.Borders.Top.Width);

        if (above is not null)
        {
            band = Length.Max(band, above.CoveredBottomRule);

            foreach (PageTableCell cell in above.Cells) band = Length.Max(band, cell.Borders.Bottom.Width);
        }

        return band;
    }

    /// <summary>
    /// The resolved band at the boundary above a row, for a caller that has the table and the index.
    /// </summary>
    /// <remarks>
    /// <see cref="SliceRow"/> is handed one row and its placed cells, so it cannot resolve this itself —
    /// and it needs it, because a part of a row occupies the boundary band above it as well as its own
    /// content. Without it a part measures only from its text and the row fits one line too many on the
    /// page. See <see cref="HeightAt"/>.
    /// </remarks>
    public static Length BoundaryBand(PageTable table, int row)
    {
        ArgumentNullException.ThrowIfNull(table);

        return row >= 0 && row < table.Rows.Count
            ? TopBand(row > 0 ? table.Rows[row - 1] : null, table.Rows[row])
            : Length.Zero;
    }

    /// <summary>
    /// The band below the table's last row, which belongs to no row because there is none below it.
    /// </summary>
    /// <remarks>
    /// [bin] <c>probes/tablerow-r146/results.md</c> §3.1: the outer bottom rule of <c>align.docx</c>'s table is
    /// a band at 109.501..110.501 below a last row ending at 109.501 — the same downward-hanging band as every
    /// interior boundary, so <em>there is no separate rule for an outer line</em>. The outer <em>top</em> band
    /// needs no counterpart here: it is the first row's own <see cref="TopBand"/> and sits inside its height.
    /// </remarks>
    private static Length BottomBand(PageTableRow row)
    {
        Length band = row.CoveredBottomRule;
        foreach (PageTableCell cell in row.Cells) band = Length.Max(band, cell.Borders.Bottom.Width);
        return band;
    }

    /// <summary>The widest top rule the row's own cells state, which is what a declared height is raised by.</summary>
    /// <remarks>
    /// [bin] <c>probes/tablerow-r146/results.md</c> §3.4, and it is the reason round 131 could find no linear
    /// fit: a <c>w:trHeight atLeast</c> floor is raised by the row's <b>own stated</b> top rule while the band
    /// drawn at that boundary is the <b>resolved</b> one. Two arms of <c>trheight.docx</c> draw the same 3 pt
    /// line at the same y and differ by 3.000 pt of height, and a third draws a 3 pt line and grows by 0.5 —
    /// its own statement, every time. [src] <c>lcl_CalcMinRowHeight</c> (<c>tabfrm.cxx</c>:5087-5097) adds
    /// <c>lcl_GetTopSpace(*_pRow)</c>, this row's own <c>CalcLineSpace(TOP, true)</c>, where the cell's print
    /// margin at :5399-5404 is the maximum of that and the row above's bottom line size.
    /// </remarks>
    private static Length OwnTopRule(PageTableRow row)
    {
        Length top = row.CoveredTopRule;
        foreach (PageTableCell cell in row.Cells) top = Length.Max(top, cell.Borders.Top.Width);
        return top;
    }

    /// <summary>
    /// The top margin a row's declared height is raised by, which is its cells' thickest.
    /// </summary>
    /// <remarks>
    /// <c>lcl_GetTopSpace</c> (<c>sw/source/core/layout/tabfrm.cxx</c>:5175) takes the maximum over the
    /// row's cells of the top border's <em>line space</em> — its width plus its distance — and adds the whole
    /// of it to a <c>w:trHeight</c> floor. The rule half of that is <see cref="OwnTopRule"/>, charged inside
    /// the same floor term, so what is left here is the distance: the cell's top margin. The two are
    /// separate because the floor is raised by the row's own <em>stated</em> rule while the band the layout
    /// makes room for is the <em>resolved</em> one, and <c>probes/tablerow-r146/results.md</c> §3.4 measures
    /// two arms that draw the same rule at the same y and differ by the whole of it in height.
    /// </remarks>
    private static Length TopInset(PageTableRow row)
    {
        Length top = Length.Zero;
        foreach (PageTableCell cell in row.Cells) top = Length.Max(top, cell.Padding.Top);
        return top;
    }

    /// <summary>
    /// The bottom margin a row's declared height is raised by, which is its cells' thickest.
    /// </summary>
    /// <remarks>
    /// <c>lcl_GetBottomLineDist</c> (<c>tabfrm.cxx</c>:5245) — the distance alone, with no part of the bottom
    /// border, which is the asymmetry the LibreOffice comment calls out: <em>"MS Word also adds the bottom
    /// border padding (but not the bottom border line)"</em>. It is charged for an <c>exact</c> row as well
    /// as for an <c>atLeast</c> one, which is the one thing <c>lcl_GetFixedRowHeight</c> does.
    /// </remarks>
    private static Length BottomInset(PageTableRow row)
    {
        Length bottom = Length.Zero;
        foreach (PageTableCell cell in row.Cells) bottom = Length.Max(bottom, cell.Padding.Bottom);
        return bottom;
    }

    /// <summary>
    /// How much a cell grows for the proportional line spacing on its last paragraph.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Writer's <c>AddParaLineSpacingToTableCells</c>, which <c>WriterFilter.cxx</c>:314 switches on for
    /// every document the OOXML, DOC and RTF importers read and which is off for a native ODF one. It is
    /// the companion of <c>AddParaSpacingToTableCells</c> — that one charges the cell for its last
    /// paragraph's space-after, this one for the space its line spacing would have handed to a paragraph
    /// below it. <c>SwFlowFrame::CalcAddLowerSpaceAsLastInTableCell</c>
    /// (<c>sw/source/core/layout/flowfrm.cxx</c>:1946) adds both, and only for the last flow frame in the
    /// cell, which is why this is measured from the last block rather than from every paragraph.
    /// </para>
    /// <para>
    /// The amount is not the leading the layout engine computes. <c>SwBorderAttrs::CalcLineSpacing_</c>
    /// (<c>sw/source/core/layout/frmtool.cxx</c>:2681) is
    /// <c>nFontSize × (prop − 100) × 1.15 / 100</c> in twips — the paragraph's <em>font size</em> rather
    /// than its measured line height, with a 1.15 fudge factor added for tdf#125300 that stands in for the
    /// ratio between the two. Reproduced here to the digit, including the truncation and the order of
    /// operations: the integer product is formed first and only then multiplied by a binary <c>1.15</c>
    /// slightly below the decimal one, which is what makes 125% come out at 68 twips rather than 69.
    /// Measured against LibreOffice on a one-cell fixture at 110, 115, 125, 150, 200 and 250 per cent,
    /// where every one of the six matched exactly.
    /// </para>
    /// <para>
    /// A cell ending in a nested table gets nothing: the attribute set consulted there is the table's,
    /// which carries no line spacing item at all.
    /// </para>
    /// </remarks>
    /// <param name="blocks">The cell's blocks.</param>
    private static Length CellLineSpacing(IReadOnlyList<PageBlock> blocks)
    {
        if (blocks.Count == 0 || blocks[^1] is not PageParagraph last) return Length.Zero;

        LineSpacingRule spacing = last.Format.LineSpacing;
        if (spacing.Mode != LineSpacingMode.Proportional) return Length.Zero;

        long percent = (long)Math.Round(spacing.Proportion * 100.0, MidpointRounding.AwayFromZero);
        if (percent <= 100) return Length.Zero;

        return Length.FromTwips((long)(last.EmSize.Twips * (percent - 100) * 1.15 / 100.0));
    }

    private static DocRect Shift(DocRect area, Length dx, Length dy)
        => new(area.X + dx, area.Y + dy, area.Width, area.Height);

    /// <summary>
    /// A cell's text moved from the origin into its own rectangle, and aligned inside it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The lines themselves do not move: they are positioned relative to the flow's area, so shifting the
    /// area takes them with it. What does change is the vertical alignment, which is only decidable now —
    /// it depends on the row's final height, and the row's height depended on this cell.
    /// </para>
    /// <para>
    /// A border is not only a line, it is a band the text may not enter: Writer insets a cell's content by
    /// the whole border width on top of the padding, so a 1 pt border moves the first line down 1 pt and
    /// everything after the table down 2. The rectangle here starts half a grid line above the content —
    /// the line is drawn through its middle — so what the content owes is the other half at each end.
    /// Measured against LibreOffice on a one-column fixture at 0, 1 and 2 pt borders, where the step was
    /// exactly half the border and did not depend on the number of rows.
    /// </para>
    /// </remarks>
    /// <param name="cell">The measured cell.</param>
    /// <param name="area">Its rectangle.</param>
    /// <param name="bandAbove">Half the grid line above the cell's first row.</param>
    /// <param name="bandBelow">Half the grid line below the cell's last row.</param>
    private static PlacedFlow? Positioned(
        Measured cell, DocRect area, Length bandAbove, Length bandBelow)
    {
        if (cell.Content is null) return null;

        CellPadding padding = cell.Cell.Padding;
        Length height = area.Height - padding.Vertical - bandAbove - bandBelow;
        Length spare = height - cell.TextHeight;

        Length offset = spare <= Length.Zero
            ? Length.Zero
            : cell.Cell.VerticalAlignment switch
            {
                VerticalTextAlignment.Middle => spare / 2,
                VerticalTextAlignment.Bottom => spare,
                _ => Length.Zero,
            };

        DocRect placed = new(
            area.X + padding.Left,
            area.Y + bandAbove + padding.Top + offset,
            area.Width - padding.Horizontal,
            height > Length.Zero ? height : Length.Zero);

        // From the origin the flow was laid out at to where the cell actually is.
        PlacedFlow moved = ShiftFlow(
            cell.Content,
            placed.X - cell.Content.Area.X,
            placed.Y - cell.Content.Area.Y)!;

        // The height comes from the cell rather than from the content, since the row may be taller.
        return moved with { Area = placed };
    }

    /// <summary>
    /// Lays out a turned cell's text and works out the quarter turn that puts it on the page.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Everything here was measured against the installed LibreOffice 26.2.4.2 on generated probes, read
    /// out of the PDF's own operators. Four facts, each of which the obvious implementation gets wrong:
    /// </para>
    /// <para>
    /// <strong>The line breaks at the cell's inner height, and it is the same inner box an upright cell
    /// uses</strong> — the height less the two half grid lines and less the <em>vertical</em> padding.
    /// Horizontal padding does not shorten the line. Pinned by a five-twip sweep of <c>w:trHeight</c>: the
    /// four-glyph to five-glyph boundary sits at exactly 500 twips in all three of {0.5 pt borders, no
    /// borders, 10 pt top and bottom cell margin}, whose frames are 25.5, 25.0 and 45.5 pt tall. Reading
    /// the turn as "swap the padding too" would have moved that boundary in two of the three.
    /// </para>
    /// <para>
    /// <strong>A turned cell contributes nothing to its row's height.</strong> Not one line's worth,
    /// nothing: a row holding only turned cells collapses to zero and LibreOffice draws neither its text
    /// nor its borders. That is what makes the apparent circularity — the line length is the cell height,
    /// the cell height is the tallest cell's line count — not a circle at all, and it is why this runs in
    /// the second pass with the row heights already settled.
    /// </para>
    /// <para>
    /// <strong>A line that starts outside the cell is not drawn at all.</strong> Dropped rather than
    /// clipped: there is no text-showing operator for it in the reference's PDF, so it is absent from the
    /// text layer as well as from the ink. A 50 pt column whose inner width is 38.7 pt draws four lines at
    /// 11.55 pt each — the fourth overhanging — and not the fifth, which would start past the edge.
    /// </para>
    /// <para>
    /// <strong><c>w:vAlign</c> places the line stack across the cell's width.</strong> Top puts the first
    /// line against the left edge, bottom the last against the right, centre in the middle — measured at
    /// 71.20, 148.80 and 110.00 pt on one fixture. It is the same axis in the cell's own frame, which is
    /// why it is still called a vertical alignment.
    /// </para>
    /// <para>
    /// What this deliberately does not do is measure the cell's <em>width</em> from the text. A turned
    /// label wants the column to be one line thick and the document says how thick; nothing in Writer
    /// widens a column to fit turned text either.
    /// </para>
    /// </remarks>
    /// <param name="cell">The cell.</param>
    /// <param name="area">Its rectangle on the page.</param>
    /// <param name="bandAbove">Half the grid line above it.</param>
    /// <param name="bandBelow">Half the grid line below it.</param>
    /// <param name="nesting">How many tables enclose the one this cell is in.</param>
    /// <param name="collapsesSpacing">Whether paragraphs collapse their spacing — see <see cref="LayOut"/>.</param>
    /// <param name="addsCellLineSpacing">Whether a cell grows by its last line spacing — see <see cref="LayOut"/>.</param>
    private static (PlacedFlow? Content, AffineTransform? Onto) Turned(
        PageTableCell cell,
        DocRect area,
        Length bandAbove,
        Length bandBelow,
        int nesting,
        bool collapsesSpacing,
        bool addsCellLineSpacing)
    {
        CellPadding padding = cell.Padding;

        // Along the text: the cell's inner height, which is what the lines break at.
        Length along = area.Height - padding.Vertical - bandAbove - bandBelow;

        // Across the stack: the cell's inner width, which is how many lines can be drawn.
        Length across = area.Width - padding.Horizontal;

        if (along <= Length.Zero || across <= Length.Zero) return (null, null);

        PlacedFlow? flow = FlowLayouter.LayOut(
            cell.Blocks,
            new DocRect(Length.Zero, Length.Zero, along, Length.Zero),
            Length.Zero,
            nesting,
            collapsesSpacing,
            addsCellLineSpacing);

        if (flow is null) return (null, null);

        // The lines that begin inside the cell. A line starting past the edge is not drawn — see above —
        // and the ones before it are kept whole even where they overhang, which is what the reference does.
        List<PlacedLine> kept = [];
        foreach (PlacedLine line in flow.Lines)
        {
            if (line.Top < across) kept.Add(line);
        }

        if (kept.Count == 0) return (null, null);

        // How much of the width the stack actually uses, for the alignment below. The last line's box
        // rather than the flow's advance: a turned cell has no space-after to charge across its width.
        Length used = kept[^1].Top + kept[^1].Box.Height;
        Length spare = across - used;

        Length offset = spare <= Length.Zero
            ? Length.Zero
            : cell.VerticalAlignment switch
            {
                VerticalTextAlignment.Middle => spare / 2,
                VerticalTextAlignment.Bottom => spare,
                _ => Length.Zero,
            };

        // Where the flow's own origin lands. The text starts at whichever end of the cell it runs *from*:
        // the bottom for a cell turned anticlockwise, the top for one turned clockwise.
        Length left = area.X + padding.Left;
        Length right = area.Right - padding.Right;
        Length top = area.Y + bandAbove + padding.Top;
        Length bottom = area.Bottom - bandBelow - padding.Bottom;

        (double radians, Length x, Length y) = cell.TextDirection switch
        {
            CellTextDirection.TopToBottomRightToLeft => (Math.PI / 2, right - offset, top),
            _ => (-Math.PI / 2, left + offset, bottom),
        };

        AffineTransform onto = AffineTransform.Concat(
            AffineTransform.Rotation(radians),
            AffineTransform.Translation(x.Emu, y.Emu));

        // The flow keeps its own coordinates — the transform is what moves it — but it is trimmed to the
        // lines that are drawn, and its rectangle is the whole inner box so that anything measuring the
        // flow sees the room it had rather than the room it used.
        return (
            flow with
            {
                Lines = kept,
                Area = new DocRect(Length.Zero, Length.Zero, along, across),
            },
            onto);
    }

    /// <summary>Where each grid column starts, measured from the table's left edge.</summary>
    /// <remarks>
    /// One entry per column plus a final one past the last, so a cell's width is the difference between
    /// two lookups whatever it spans — including a cell whose span runs off the end of the declared grid,
    /// which real documents contain.
    /// </remarks>
    private static List<Length> ColumnLefts(IReadOnlyList<Length> widths)
    {
        int columns = Math.Min(widths.Count, PageTable.MaxColumns);
        List<Length> lefts = new(columns + 1);

        Length at = Length.Zero;
        for (int column = 0; column < columns; column++)
        {
            lefts.Add(at);
            at += widths[column];
        }

        lefts.Add(at);
        return lefts;
    }

    /// <summary>How wide a cell is: the columns it spans, added up.</summary>
    /// <remarks>
    /// Clamped to the declared grid at both ends. A cell starting past the last column has no width and is
    /// dropped; one spanning past it stops at the edge, which is what LibreOffice's own importers do with a
    /// row whose cells overrun the grid rather than widening the table.
    /// </remarks>
    private static Length WidthOf(PageTableCell cell, List<Length> lefts)
    {
        int columns = lefts.Count - 1;
        if (cell.Column < 0 || cell.Column >= columns) return Length.Zero;

        int end = Math.Clamp(cell.ColumnEnd, cell.Column + 1, columns);
        return lefts[end] - lefts[cell.Column];
    }

    /// <summary>
    /// A cell after pass one: its text laid out, and which rows it charges.
    /// </summary>
    /// <param name="Row">The row it starts in.</param>
    /// <param name="LastRow">The last row it covers, which is where its height is charged.</param>
    /// <param name="Cell">The cell itself.</param>
    /// <param name="Width">Its outer width, padding included.</param>
    /// <param name="Content">Its text at the origin, to be moved once the rectangle is known.</param>
    /// <param name="TextHeight">How tall that text is.</param>
    private readonly record struct Measured(
        int Row,
        int LastRow,
        PageTableCell Cell,
        Length Width,
        PlacedFlow? Content,
        Length TextHeight);
}
