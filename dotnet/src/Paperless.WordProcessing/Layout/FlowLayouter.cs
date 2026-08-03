using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.Text.Layout;

namespace Paperless.WordProcessing.Layout;

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
    /// <remarks>
    /// Nothing is clipped and nothing overflows into a second rectangle: content taller than the area is
    /// placed anyway and runs past its bottom, which is what Writer does with a fixed-height header whose
    /// text does not fit. A stated offset is honoured even then, so an overflowing footer grows downwards
    /// rather than climbing into the body.
    /// </remarks>
    public static PlacedFlow? LayOut(
        IReadOnlyList<PageBlock> blocks,
        DocRect area,
        Length? offsetFromTop,
        int nesting = 0,
        bool collapsesSpacing = false)
    {
        ArgumentNullException.ThrowIfNull(blocks);

        if (blocks.Count == 0 || area.Width <= Length.Zero) return null;

        List<PlacedLine> placed = [];
        List<PlacedTable> tables = [];
        Length top = Length.Zero;

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

                top += nested.SpaceBefore;

                // The flow's width is what a table stating none of its own is fitted to, which for a cell is
                // the cell and for a header the text area. It changes nothing for a table declaring a grid.
                (List<PlacedTableCell> cells, List<Length> rowHeights) = TableLayouter.LayOut(
                    nested,
                    new DocPoint(area.X, area.Y + top),
                    nesting + 1,
                    area.Width,
                    collapsesSpacing);

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

                top += height + nested.SpaceAfter;

                // A table hands no leading down: `GetSpacingValuesOfFrame` reports a line spacing only
                // for a text frame. Nor does it collapse against the paragraph after it — its space-after
                // is a table property rather than a paragraph's, and the formats keep the two apart.
                leading = Length.Zero;
                previousSpaceAfter = null;
                continue;
            }

            PageParagraph paragraph = (PageParagraph)blocks[i];
            ParagraphLayouter layouter = new(paragraph.Face, breaker: null, paragraph.Metrics);
            ParagraphFormat? previous = i > 0 && blocks[i - 1] is PageParagraph before
                ? before.Format
                : null;

            LaidOutParagraph layout =
                paragraph.HasRuns || paragraph.HasInlineObjects || paragraph.LabelRaisesFirstLine
                ? layouter.Layout(
                    paragraph.Measure(),
                    paragraph.Format,
                    area.Width,
                    paragraph.Language,
                    previous)
                : layouter.Layout(
                    paragraph.Text,
                    paragraph.Format,
                    paragraph.EmSize,
                    area.Width,
                    paragraph.Language,
                    previous,
                    paragraph.Shaping);

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

            top += above + leading;

            for (int line = 0; line < layout.Lines.Count; line++)
            {
                // A paragraph's first line loses the leading above its text — it belongs to the paragraph
                // above and has just been added to the gap — and so does the flow's first line, which is
                // the same rule the first line of a page's body follows: the space is part of the upper
                // margin and is dropped at the top of a frame, and each of these three is a frame.
                LineBox box = ParagraphLeading.AsDrawn(
                    layout.Lines[line],
                    isFirstOfParagraph: line == 0,
                    isFirstInFrame: placed.Count == 0 && tables.Count == 0);

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

            top += layout.SpaceAfter;
            leading = ParagraphLeading.Below(layout);
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
        Length shift = offsetFromTop ?? (area.Height - top);

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
            Advance = top,
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
        IReadOnlyList<PageBlock> blocks, Length width, int nesting = 0, bool collapsesSpacing = false)
    {
        PlacedFlow? flow = LayOut(
            blocks,
            new DocRect(Length.Zero, Length.Zero, width, Length.Zero),
            Length.Zero,
            nesting,
            collapsesSpacing);

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
}
