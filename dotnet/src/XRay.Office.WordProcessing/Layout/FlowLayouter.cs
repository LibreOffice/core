using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.Text.Layout;

namespace XRay.Office.WordProcessing.Layout;

/// <summary>
/// Lays a run of blocks out into one rectangle, as a header, a footer or a table cell.
/// </summary>
/// <remarks>
/// <para>
/// The operation all three share, and the reason <see cref="PlacedFlow"/> is one type: stack the blocks at
/// the rectangle's width, with their spacing between them, and report where each line and each table
/// landed. What none of the three does is <em>flow</em> — nothing here splits across a page, because a
/// header is furniture and a cell belongs to its row. The body's own blocks go through
/// <see cref="Paginator"/> instead, which is the same stacking plus everything that makes a page end.
/// </para>
/// <para>
/// Height is discovered rather than given, which is what a table needs: a row is as tall as its tallest
/// cell, so the cells have to be laid out before the row's height is known, and laying one out cannot
/// require it.
/// </para>
/// </remarks>
public static class FlowLayouter
{
    /// <summary>
    /// How deeply tables may nest inside one another before the innermost is dropped.
    /// </summary>
    /// <remarks>
    /// A guard on untrusted input, and one that matters more here than most: a cell holds a flow, a flow
    /// holds a table, and that table's cells hold flows — so a file claiming a hundred levels of nesting
    /// would recurse a hundred deep for every cell of every level. Real documents nest two or three.
    /// </remarks>
    public const int MaxNesting = 16;

    /// <summary>
    /// Lays blocks out into a rectangle, or returns null when there is nothing to place.
    /// </summary>
    /// <param name="blocks">The paragraphs and tables, in order.</param>
    /// <param name="area">The rectangle to fill, whose width decides the line breaks.</param>
    /// <param name="offsetFromTop">
    /// Where the first line goes: zero to grow downwards from the area's top, a value to start that far
    /// below it, and null to <em>bottom-align</em> the whole block so that its last line rests on the
    /// area's bottom. Null is what a Word footer does; see <see cref="Model.PageGeometry.FooterOffset"/>.
    /// </param>
    /// <param name="nesting">How many tables enclose this flow, for the recursion guard.</param>
    /// <param name="collapsesSpacing">
    /// Whether the gap between two paragraphs is the larger of the previous one's space-after and the next
    /// one's space-before rather than their sum — <see cref="PaginationOptions.CollapsesSpacing"/>, which
    /// is Writer's <c>PARA_SPACE_MAX</c> read the other way round. The same rule the body follows, because
    /// <c>SwFlowFrame::CalcUpperSpace</c> is what measures the gap above <em>every</em> text frame and
    /// knows nothing about whether it sits in a page, a cell or a running head. Defaults to adding, which
    /// is what an ODF document asks for.
    /// </param>
    /// <param name="addsCellLineSpacing">
    /// Whether a table cell inside this flow grows by its last paragraph's proportional line spacing —
    /// Writer's <c>AddParaLineSpacingToTableCells</c>, forwarded to <see cref="TableLayouter.LayOut"/>
    /// so that a table nested in a cell, a header or a text frame follows the same rule the body's does.
    /// </param>
    /// <param name="floatsPositionedTables">
    /// Whether a <see cref="PageTable.IsPositioned"/> table is taken out of the flow — placed where the
    /// flow has reached but leaving it there, so the blocks after it start where it started rather than
    /// below it, and contributing its own bottom edge plus <see cref="PageTable.LowerSpacing"/> to the
    /// flow's height instead of stacking. True for a running head or foot and false everywhere else; see
    /// the remarks for the measurement that decides it.
    /// </param>
    /// <remarks>
    /// <para>
    /// Nothing is clipped and nothing overflows into a second rectangle: content taller than the area is
    /// placed anyway and runs past its bottom, which is what Writer does with a fixed-height header whose
    /// text does not fit. A stated offset is honoured even then, so an overflowing footer grows downwards
    /// rather than climbing into the body.
    /// </para>
    /// <para>
    /// <b>A positioned table in a running head is a frame, and its anchor paragraph does not move out of
    /// its way.</b> Writer's DOCX importer turns a <c>w:tblpPr</c> table into a fly holding a table —
    /// visible in <c>--convert-to fodt</c> as a <c>draw:frame</c> whose style carries <c>w:bottomFromText</c>
    /// as <c>fo:margin-bottom</c> — and in a header <c>SwTextFly</c> does not wrap the anchor's text around
    /// it. Measured on the installed 26.2.4.2 by perturbing that flat XML and re-rendering, on
    /// <c>words/batch-010/docx/5709.16 ch.40_mgfinal.docx</c>: the body's first line moves one for one with
    /// the frame's lower spacing (0 → 114.54 pt, 403 twips → 134.69, 1 in → 186.54), does not move with the
    /// frame's <em>upper</em> spacing, and does not move when the anchor paragraph grows from 8 pt to 20 pt
    /// — only when the paragraph grows taller than the frame does it take over. Put text in that paragraph
    /// and it draws at the very top of the header, overlapping the frame.
    /// </para>
    /// <para>
    /// So the head's height is <c>max(in-flow content height, frame bottom + the frame's lower spacing)</c>,
    /// which is what this computes. Stacking the table instead made this document's header 10.95 pt short
    /// — the table's height plus a 9.20 pt empty paragraph, where Writer takes the table's height plus a
    /// 20.15 pt lower spacing — and that one step, repeated on all 31 pages, is what cost it a page.
    /// </para>
    /// <para>
    /// The body is deliberately not given the same treatment. There <c>bWrapAllowed</c> is true
    /// (<c>sw/source/core/text/txtfly.cxx</c>, the <c>!IsInFootnote() &amp;&amp; !bFooterHeader</c> arm), so
    /// the anchor's text goes <em>below</em> the fly — which is what stacking already approximates — and no
    /// measurement was taken there. 21 corpus documents hold a positioned table in the body against 4 in a
    /// header or foot.
    /// </para>
    /// </remarks>
    /// <param name="anchored">
    /// The frames the flow's own paragraphs anchor, with the page coordinates its origin corresponds to,
    /// or null when the caller does not know where the flow sits — which is every caller but a table
    /// cell's. See <see cref="AnchoredObstacles"/> for why a cell needs its own route to them.
    /// </param>
    public static PlacedFlow? LayOut(
        IReadOnlyList<PageBlock> blocks,
        DocRect area,
        Length? offsetFromTop,
        int nesting = 0,
        bool collapsesSpacing = false,
        bool addsCellLineSpacing = false,
        bool floatsPositionedTables = false,
        AnchoredObstacles? anchored = null)
    {
        ArgumentNullException.ThrowIfNull(blocks);

        if (blocks.Count == 0 || area.Width <= Length.Zero) return null;

        List<PlacedLine> placed = [];
        List<PlacedTable> tables = [];
        Length top = Length.Zero;

        // How far down a floated table reaches, its own lower spacing included. The flow is as tall as the
        // lower of its stacked content and this, which is the `max` the remarks above measured.
        Length floated = Length.Zero;

        // Tables the flow actually stacked, which a floated one is not. Only these decide whether a
        // paragraph is the first thing in the frame.
        int stacked = 0;

        // What the paragraph last placed hands down to the next one's first line. See
        // <see cref="ParagraphLeading"/>: the leading proportional line spacing adds above a first line
        // is the previous paragraph's, measured against the height of *its* last line.
        Length leading = Length.Zero;

        // The space-after already added to `top` by the paragraph just placed, which is what a collapsing
        // gap is measured against: adding only the part of the next paragraph's space-before that exceeds
        // it leaves the larger of the two between them. Null after a table or before the first block,
        // since neither collapses against a paragraph.
        Length? previousSpaceAfter = null;

        for (int i = 0; i < blocks.Count; i++)
        {
            if (blocks[i] is PageTable nested)
            {
                if (nesting >= MaxNesting) continue;

                // A floated table is placed where the flow has reached and leaves it there. Its own
                // space-before is a table property and belongs to the flow it was taken out of, so it is
                // not applied either — the frame's position is what the fly carries, and only the
                // horizontal half of that is read.
                bool floats = floatsPositionedTables && nested.IsPositioned;

                // The paragraph above's leading goes to the table, exactly as it goes to a paragraph:
                // `SwFlowFrame::CalcUpperSpace` adds `nPrevLineSpacing` before it looks at what the
                // frame below is (`flowfrm.cxx`:1655-1739). A floated one takes none of it, for the
                // same reason it takes no space-before — it is not in the flow the leading belongs to.
                if (!floats) top += nested.SpaceBefore + leading;

                // The flow's width is what a table stating none of its own is fitted to, which for a cell is
                // the cell and for a header the text area. It changes nothing for a table declaring a grid.
                (List<PlacedTableCell> cells, List<Length> rowHeights) = TableLayouter.LayOut(
                    nested,
                    new DocPoint(area.X, area.Y + top),
                    nesting + 1,
                    area.Width,
                    collapsesSpacing,
                    addsCellLineSpacing,
                    anchored?.Below(new DocPoint(Length.Zero, top)));

                Length height = Length.Zero;
                foreach (Length row in rowHeights) height += row;

                tables.Add(new PlacedTable
                {
                    Table = nested,
                    Area = new DocRect(
                        area.X + nested.LeftIndent,
                        area.Y + top,
                        nested.WidthWithin(area.Width),
                        height),
                    Cells = cells,
                    FirstRow = 0,
                    RowEnd = rowHeights.Count,
                });

                if (floats)
                {
                    floated = Length.Max(floated, top + height + nested.LowerSpacing);
                }
                else
                {
                    top += height + nested.SpaceAfter;
                    stacked++;
                }

                // A table hands no leading down: `GetSpacingValuesOfFrame` reports a line spacing only
                // for a text frame. Nor does it collapse against the paragraph after it — its space-after
                // is a table property rather than a paragraph's, and the formats keep the two apart.
                // The leading it was *given* is spent either way, floated or not, so it is cleared here
                // rather than in the branch above.
                leading = Length.Zero;
                previousSpaceAfter = null;
                continue;
            }

            PageParagraph paragraph = (PageParagraph)blocks[i];
            ParagraphLayouter layouter = new(
                paragraph.Face, breaker: null, paragraph.Metrics, WriterLineBox.LeadingAboveText);
            ParagraphFormat? previous = i > 0 && blocks[i - 1] is PageParagraph before
                ? before.Format
                : null;

            // The same four conditions the paginator routes on, and for the same reasons — a cell, a
            // header and a text box reach layout through here instead and must break their lines in the
            // same places. `NeedsGlyphFallback` is the one that is not about height: it says the
            // paragraph's own face cannot draw its own text, so only the per-run path measures it in the
            // face the drawing pass will actually use.
            // The frames this paragraph itself anchors, which for a flow is the only way it can meet
            // one: a cell's obstacles are keyed by nothing the paginator can look a cell paragraph up
            // by. Null for every flow but a table cell's, and for every cell holding no floating frame.
            ILineObstacles? obstacles = anchored?.For(paragraph, top);

            LaidOutParagraph layout =
                paragraph.HasRuns || paragraph.HasInlineObjects || paragraph.LabelRaisesFirstLine
                || paragraph.NeedsGlyphFallback || paragraph.HasScriptSpace
                // And a character width, which is the one property of the shortcut's own
                // arguments that cannot express it: the text overload measures from a face, a
                // size and the shaping, and a scale is none of those.
                || paragraph.IsHorizontallyScaled
                ? layouter.Layout(
                    paragraph.Measure(),
                    paragraph.Format,
                    area.Width,
                    paragraph.Language,
                    previous,
                    obstacles,
                    emSize: paragraph.EmSize)
                : layouter.Layout(
                    paragraph.Text,
                    paragraph.Format,
                    paragraph.EmSize,
                    area.Width,
                    paragraph.Language,
                    previous,
                    paragraph.EffectiveShaping,
                    obstacles);

            // Collapsing: the gap between two paragraphs is the larger of the two spacings rather than
            // their sum, so only the part of this paragraph's space-before that exceeds the space-after
            // already added for the one above is added again. Contextual spacing goes further and
            // suppresses the gap outright, which means taking that space-after back off.
            Length above =
                previousSpaceAfter is { } settled
                    && ParagraphLayouter.SharesContextualSpacing(previous, paragraph.Format)
                    ? Length.Zero - settled
                    : collapsesSpacing && previousSpaceAfter is { } after
                        ? Length.Max(Length.Zero, layout.SpaceBefore - after)
                        : layout.SpaceBefore;

            // The top border's room, outside the collapse: a rule's distance from its own text is not
            // spacing between two paragraphs, and nothing above can pay for it.
            top += above + leading + paragraph.BorderAbove;

            for (int line = 0; line < layout.Lines.Count; line++)
            {
                // A paragraph's first line loses the leading above its text — it belongs to the paragraph
                // above and has just been added to the gap — and so does the flow's first line, which is
                // the same rule the first line of a page's body follows: the space is part of the upper
                // margin and is dropped at the top of a frame, and each of these three is a frame.
                // A floated table is not content the flow has passed: it left `top` where it was, so a
                // paragraph beside it is still the first thing in the frame and still drops its leading.
                LineBox box = ParagraphLeading.AsDrawn(
                    layout.Lines[line],
                    isFirstOfParagraph: line == 0,
                    isFirstInFrame: placed.Count == 0 && stacked == 0,
                    paragraph.Format.LineSpacing);

                // `above` and not `above + leading`: the leading is the paragraph above's, and Writer's
                // `GetTopForObjPos` keeps it in a paragraph-anchored frame's origin. See
                // `PlacedLine.ParagraphTop`.
                placed.Add(new PlacedLine(
                    i, line, box, top, Column: 0, UpperSpace: line == 0 ? above : Length.Zero));

                // A box that shares its line with the next leaves the pen where it is: a line beside a
                // floating frame clear of both margins is two stretches on one baseline, and the line's
                // height is counted once, at its last stretch. Never true for a flow with no obstacles,
                // which is every header and footer, so this changes nothing for them.
                if (!box.SharesLineWithNext) top += box.Height;
            }

            top += layout.SpaceAfter + paragraph.BorderBelow;
            leading = ParagraphLeading.Below(layout, paragraph.Format.LineSpacing);
            previousSpaceAfter = layout.SpaceAfter;
        }

        if (placed.Count == 0 && tables.Count == 0) return null;

        // Where the block as a whole goes. A bottom-aligned one rests its last line on the area's bottom
        // whether or not it fits, so a footer that outgrows the room reserved for it grows *upwards* into
        // the body — which is what Word does and what Writer's dynamic-height footer frame does, since
        // the frame's lower edge is fixed at the footer distance and only its top moves. Clamping the
        // shift at nought instead pushed such a footer down past the page's bottom edge, and on a Word
        // document whose `w:bottom` equals its `w:footer` that is every footer it has. A stated offset is
        // taken as given either way.
        //
        // The flow is as tall as the lower of the two edges — what it stacked, and what a floated table
        // reaches with its lower spacing. A running foot holding a positioned table is bottom-aligned
        // against the second of those exactly as it would be against the first.
        Length reach = Length.Max(top, floated);
        Length shift = offsetFromTop ?? (area.Height - reach);

        if (shift != Length.Zero)
        {
            for (int i = 0; i < placed.Count; i++)
            {
                placed[i] = placed[i] with { Top = placed[i].Top + shift };
            }

            // A table's cells carry page coordinates rather than flow-relative ones, so they move with the
            // rectangle rather than with the line tops. Forgetting this leaves a bottom-aligned footer's
            // table where the flow would have been had it not moved.
            for (int i = 0; i < tables.Count; i++)
            {
                tables[i] = tables[i] with
                {
                    Area = Shift(tables[i].Area, shift),
                    Cells = TableLayouter.Offset(tables[i].Cells, Length.Zero, shift),
                };
            }
        }

        return new PlacedFlow
        {
            Blocks = blocks,
            Lines = placed,
            Tables = tables,
            Area = area,
            Advance = reach,

            // Whatever the last paragraph would have handed down. `leading` holds it because the loop
            // leaves it there for a paragraph that never came; see PlacedFlow.TrailingLineSpacing for
            // why it is reported rather than added.
            TrailingLineSpacing = leading,
        };
    }

    /// <summary>
    /// How tall the blocks are at a given width, without placing them anywhere.
    /// </summary>
    /// <remarks>
    /// What a table's rows are sized from: a cell's height is its content's, and the row's is the tallest
    /// cell's. Measured by laying the flow out into a rectangle of unbounded height and asking where it
    /// ended, because that is the only answer that agrees with where the lines will actually be drawn —
    /// summing estimated line heights instead would drift from the real result exactly where it matters.
    /// </remarks>
    public static Length HeightOf(
        IReadOnlyList<PageBlock> blocks,
        Length width,
        int nesting = 0,
        bool collapsesSpacing = false,
        bool addsCellLineSpacing = false)
    {
        PlacedFlow? flow = LayOut(
            blocks,
            new DocRect(Length.Zero, Length.Zero, width, Length.Zero),
            Length.Zero,
            nesting,
            collapsesSpacing,
            addsCellLineSpacing);

        return flow is null ? Length.Zero : Extent(flow);
    }

    /// <summary>
    /// How far down a flow's content reaches, measured from its area's top.
    /// </summary>
    /// <remarks>
    /// The lower of the two edges, because a flow can end with either: a header of two paragraphs ends at
    /// its last line, and one that ends with a table ends at the table's bottom. Taking only the lines
    /// would size a cell whose last block is a table as though the table were not there.
    /// </remarks>
    public static Length Extent(PlacedFlow flow)
    {
        ArgumentNullException.ThrowIfNull(flow);

        Length bottom = flow.Lines.Count == 0
            ? Length.Zero
            : flow.Lines[^1].Top + flow.Lines[^1].Box.Height;

        foreach (PlacedTable table in flow.Tables)
        {
            Length reach = table.Area.Bottom - flow.Area.Y;
            bottom = Length.Max(bottom, reach);
        }

        return bottom;
    }

    private static DocRect Shift(DocRect area, Length dy)
        => new(area.X, area.Y + dy, area.Width, area.Height);

    /// <summary>
    /// A flow cut down to what fits in a shape of a stated height, the way Writer formats one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A text box whose height the file states rather than grows does not overflow and is not clipped:
    /// Writer stops formatting when the next line would start below the box, and the lines after that
    /// are never laid out at all. The difference from clipping is measurable — the text is absent from
    /// the PDF's text-showing operators, so <c>pdftotext</c> does not find it — and it is why a
    /// four-paragraph running head 15 pt tall extracts as one line and not four.
    /// </para>
    /// <para>
    /// <strong>The rule is the measured one</strong>, from the 60 authored boxes in
    /// <c>dotnet/probes/words-extra-01/</c>: keep an item whose top is <em>strictly less</em> than
    /// <paramref name="height"/>, and always keep the first one however short the box. Keeping only
    /// items that fit entirely is a different rule and is refuted — a 10 pt box with no insets draws
    /// two lines of a face taller than 5 pt.
    /// </para>
    /// <para>
    /// Tables are kept or dropped whole. LibreOffice truncates a table in a box by <em>row</em> on this
    /// same rule, so a table straddling the boundary keeps rows we would drop; no corpus document in
    /// the group that motivated this has one, and doing it properly means re-running
    /// <see cref="TableLayouter"/> against a height it does not currently take.
    /// </para>
    /// </remarks>
    /// <param name="flow">The flow as it was laid out with no height limit.</param>
    /// <param name="height">The shape's content height — its own less its text insets.</param>
    public static PlacedFlow Truncated(PlacedFlow flow, Length height)
    {
        ArgumentNullException.ThrowIfNull(flow);

        List<PlacedLine> lines = [];
        foreach (PlacedLine line in flow.Lines)
        {
            if (line.Top < height) lines.Add(line);
        }

        List<PlacedTable> tables = [];
        foreach (PlacedTable table in flow.Tables)
        {
            if (table.Area.Y - flow.Area.Y < height) tables.Add(table);
        }

        // The first thing always survives. Writer formats one line into a box too short for any, which
        // is what makes a 15 pt head still say "Document reference:" rather than nothing at all.
        if (lines.Count == 0 && tables.Count == 0)
        {
            if (flow.Lines.Count > 0) lines.Add(flow.Lines[0]);
            else if (flow.Tables.Count > 0) tables.Add(flow.Tables[0]);
        }

        if (lines.Count == flow.Lines.Count && tables.Count == flow.Tables.Count) return flow;

        PlacedFlow cut = flow with { Lines = lines, Tables = tables };
        return cut with { Advance = Length.Min(flow.Advance, Extent(cut)) };
    }
}
