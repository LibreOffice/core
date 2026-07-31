using Paperless.Core.Extraction;
using Paperless.Core.Units;
using Paperless.WordProcessing.Layout;

namespace Paperless.WordProcessing.Rtf;

/// <content>Table reading for RTF.</content>
/// <remarks>
/// <para>
/// RTF has no table element. A row is a run of paragraphs marked <c>\intbl</c>, preceded by a
/// <c>\trowd</c> row definition that declares each cell by its <em>right edge</em> in twips with
/// <c>\cellx</c>, with <c>\cell</c> ending each cell's content and <c>\row</c> ending the row.
/// Consecutive rows form a table only by being adjacent, which is why the table is closed by the
/// first thing that is not another row rather than by any marker.
/// </para>
/// <para>
/// <strong>A horizontal span is usually not marked at all.</strong> <c>\clmgf</c> and
/// <c>\clmrg</c> exist, and Word writes them, but LibreOffice expresses a merge purely by
/// geometry: the merged cell's <c>\cellx</c> simply reaches the edge where two columns end in the
/// rows around it. So the span has to be derived from the column grid — the set of every right
/// edge the whole table uses — rather than read off the cell. Trusting the flags alone reports
/// every LibreOffice-written merge as an ordinary cell.
/// </para>
/// <para>
/// A <strong>nested</strong> table is written inside the enclosing cell with <c>\itap</c> giving its
/// depth, <c>\nestcell</c> and <c>\nestrow</c> in place of <c>\cell</c> and <c>\row</c>, and its row
/// definition <em>after</em> its cells rather than before, inside <c>{\*\nesttableprops}</c>. So the
/// declarations cannot be matched to cells as each cell closes; they are applied when the row does,
/// which works for both orders.
/// </para>
/// </remarks>
public sealed partial class RtfDocumentReader
{
    /// <summary>
    /// The widest row Paperless materialises. Word's own limit is 63 columns, and a row declaring
    /// more is malformed.
    /// </summary>
    public const int MaxTableColumns = 63;

    /// <summary>How deeply tables may nest before the document is treated as malformed.</summary>
    /// <remarks>
    /// <c>\itap</c> is an arbitrary number from an untrusted file and each level is a live builder, so
    /// without a cap a document claiming a depth of two billion would allocate that many.
    /// </remarks>
    public const int MaxTableDepth = 32;

    /// <summary>The nesting level the paragraphs being read belong to.</summary>
    /// <remarks>
    /// <c>\itap</c> states it, but a producer predating nested tables writes only <c>\intbl</c> — so
    /// "in a table at no stated depth" means the top level rather than no level at all.
    /// </remarks>
    private static int LevelOf(Flow flow)
        => flow.TableLevelIndex > 0 ? Math.Min(flow.TableLevelIndex, MaxTableDepth)
        : flow.InTable ? 1
        : 0;

    private static TableLevel LevelAt(Flow flow, int level)
    {
        while (flow.Levels.Count < level) flow.Levels.Add(new TableLevel());
        return flow.Levels[level - 1];
    }

    /// <summary>
    /// Begins a row definition at <c>\trowd</c>.
    /// </summary>
    /// <remarks>
    /// The level is remembered, because a nested row's definition sits in a group that says nothing
    /// about depth and its <c>\nestrow</c> has to close the same table its cells went into.
    /// <c>\trowd</c> may be repeated to restate a row before it is filled, so an open definition is
    /// replaced rather than nested.
    /// </remarks>
    private static void BeginRowDefinition(Flow flow)
    {
        int level = Math.Max(1, LevelOf(flow));
        flow.DefinitionLevel = level;

        TableLevel table = LevelAt(flow, level);
        table.CellDefinitions.Clear();
        table.RowLeftEdge = 0;
        table.RowIsHeader = false;
        table.RowHalfGap = null;
        table.RowHeight = 0;
        Array.Clear(table.RowPadding);
        ClearPendingCellFlags(table);
    }

    /// <summary>The level a row-definition control word applies to.</summary>
    private static TableLevel DefinitionTarget(Flow flow)
        => LevelAt(flow, Math.Max(1, flow.DefinitionLevel));

    private static void AddCellDefinition(Flow flow, int? rightEdge)
    {
        TableLevel table = DefinitionTarget(flow);

        if (table.CellDefinitions.Count >= MaxTableColumns)
        {
            ClearPendingCellFlags(table);
            return;
        }

        table.CellDefinitions.Add(new CellDefinition(
            rightEdge ?? 0,
            table.PendingCellMergesFirst,
            table.PendingCellMerged,
            table.PendingCellVerticalFirst,
            table.PendingCellVerticalMerged,
            [.. table.PendingCellPadding],
            table.PendingCellAlignment));
        ClearPendingCellFlags(table);
    }

    /// <summary>
    /// Which side a <c>\clpad</c> control word actually sets.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>Top and left are swapped.</strong> <c>\clpadl</c> sets the <em>top</em> margin and
    /// <c>\clpadt</c> the <em>left</em>; bottom and right are not swapped. This is not a guess and not a
    /// bug reproduced blindly — LibreOffice's own importer does it deliberately, with the comment "Top and
    /// left is swapped, that's what Word does"
    /// (<c>sw/source/writerfilter/rtftok/rtfdispatchvalue.cxx</c>, <c>RTFKeyword::CLPADL</c>). Word is what
    /// defines RTF in practice, so it is the specification's reading of these two words that is wrong.
    /// </para>
    /// <para>
    /// Caught by measurement rather than by reading. LibreOffice's own RTF export of a table with 0.6 cm of
    /// left padding writes <c>\clpadt340</c>, and its own rendering of that file puts the text 340 twips
    /// from the cell's <em>left</em> edge. A reader taking the words at face value indents the text
    /// downwards instead of rightwards, which is exactly what this reader did first.
    /// </para>
    /// </remarks>
    private static int CellPaddingSide(char word) => word switch
    {
        't' => 0,
        'r' => 1,
        'l' => 2,
        _ => 3,
    };

    /// <summary>Which side a <c>\trpadd</c> control word sets, which is the side it names.</summary>
    /// <remarks>
    /// Not swapped, unlike <see cref="CellPaddingSide"/> — the same source maps these four straight
    /// through. Two spellings of one quantity disagreeing about which side is which is the sort of thing
    /// only a comparison finds.
    /// </remarks>
    private static int RowPaddingSide(char word) => word switch
    {
        'l' => 0,
        'r' => 1,
        't' => 2,
        _ => 3,
    };

    /// <summary>
    /// Records one side of a padding block.
    /// </summary>
    /// <remarks>
    /// Zero is a real value and not "unstated": a cell can legitimately ask for no padding at all, which is
    /// why the arrays hold nulls rather than zeroes for the sides nothing was said about.
    /// </remarks>
    private static void SetPadding(int?[] sides, int index, int? twips)
        => sides[index] = Math.Max(0, twips ?? 0);

    private static void ClearPendingCellFlags(TableLevel table)
    {
        table.PendingCellMergesFirst = false;
        table.PendingCellMerged = false;
        table.PendingCellVerticalFirst = false;
        table.PendingCellVerticalMerged = false;
        table.PendingCellAlignment = CellVerticalAlignment.Top;
        Array.Clear(table.PendingCellPadding);
    }

    /// <summary>
    /// The list a finished paragraph belongs in: an open cell's content, or the section itself.
    /// </summary>
    /// <remarks>
    /// Anything at a shallower level than the open tables closes them first, since RTF marks no end to
    /// a table — a paragraph back at the enclosing level is what says the nested one finished.
    /// </remarks>
    private IList<ContentNode> Destination(Flow flow, int level)
    {
        CloseTablesDeeperThan(flow, level);
        return level <= 0 ? flow.Target.Children : LevelAt(flow, level).CellContent;
    }

    /// <summary>Materialises every table nested deeper than a level, innermost first.</summary>
    /// <remarks>
    /// Innermost first, because a finished inner table goes into a cell of the table that encloses it
    /// and so must exist before that cell is closed.
    /// </remarks>
    private void CloseTablesDeeperThan(Flow flow, int level)
    {
        for (int deeper = flow.Levels.Count; deeper > Math.Max(0, level); deeper--)
        {
            ContentTable? table = FinishTable(flow, deeper);
            flow.Levels.RemoveAt(deeper - 1);

            if (table is null) continue;
            if (deeper == 1) flow.Target.Children.Add(table);
            else LevelAt(flow, deeper - 1).CellContent.Add(table);
        }
    }

    /// <summary>Ends a cell at <c>\cell</c> or <c>\nestcell</c>.</summary>
    private void EndCell(GroupState state)
    {
        if (state.Destination is not RtfDestination.Body) return;

        Flow flow = CurrentFlow;
        int level = LevelOf(flow);

        // A cell mark with no table around it: keep the content as body text rather than losing it.
        if (level <= 0)
        {
            FinishParagraph(flow, force: true);
            return;
        }

        TableLevel table = LevelAt(flow, level);

        // The cell's last paragraph usually has no \par of its own — the cell mark ends it. The group
        // state is passed because it is the paragraph's own formatting, and without it the paragraph
        // reaches the content tree but never the layout pass: a cell's text would extract and not draw.
        FinishParagraph(flow, state, force: table.CellContent.Count == 0);
        CollectCell(table);
    }

    private static void CollectCell(TableLevel table)
    {
        CellDraft cell = new();
        cell.Content.AddRange(table.CellContent);
        cell.LayoutParagraphs.AddRange(table.CellLayout);
        table.CellContent.Clear();
        table.CellLayout.Clear();
        table.RowCells.Add(cell);
    }

    /// <summary>Ends a row at <c>\row</c> or <c>\nestrow</c>.</summary>
    /// <remarks>
    /// The level comes from the paragraphs the row is made of, not from the row definition: the
    /// definition of a nested row is the last one seen, so a <c>\row</c> closing the enclosing table
    /// afterwards would otherwise close the nested one a second time.
    /// </remarks>
    private void EndRow(GroupState state)
    {
        if (state.Destination is not RtfDestination.Body) return;

        Flow flow = CurrentFlow;
        int level = Math.Max(1, LevelOf(flow));
        TableLevel table = LevelAt(flow, level);

        // Content written after the last cell mark but before the row's end still belongs to a cell.
        if (table.CellContent.Count > 0)
        {
            FinishParagraph(flow, state, force: false);
            if (table.CellContent.Count > 0) CollectCell(table);
        }

        if (table.RowCells.Count == 0) return;

        ApplyDefinitions(table);
        ApplyExplicitMerges(table);

        RowDraft row = new()
        {
            Index = table.TableRows.Count,
            LeftEdge = table.RowLeftEdge,
            IsHeader = table.RowIsHeader,
            Height = table.RowHeight,
        };
        row.Cells.AddRange(table.RowCells);
        table.TableRows.Add(row);

        table.RowCells.Clear();
        table.CellContent.Clear();
    }

    /// <summary>
    /// Attaches the row definition's declarations to the cells that were collected, by index.
    /// </summary>
    /// <remarks>
    /// Once the row is closed rather than as each cell ends: doing it per cell works for a top-level
    /// table and fails for a nested one, whose definition arrives only after its cells.
    /// </remarks>
    private static void ApplyDefinitions(TableLevel table)
    {
        for (int index = 0; index < table.RowCells.Count; index++)
        {
            CellDefinition definition = index < table.CellDefinitions.Count
                ? table.CellDefinitions[index]
                : default;

            table.RowCells[index].RightEdge = definition.RightEdge;
            table.RowCells[index].IsHorizontallyMerged = definition.Merged;
            table.RowCells[index].ContinuesMergeAbove = definition.VerticalMerged;
            table.RowCells[index].Padding = PaddingOf(definition, table);
            table.RowCells[index].VerticalAlignment = definition.VerticalAlignment;
        }
    }

    /// <summary>
    /// Folds cells marked <c>\clmrg</c> into the cell they were merged with.
    /// </summary>
    /// <remarks>
    /// Only the explicitly flagged merges Word writes. LibreOffice's geometric merges are resolved
    /// later, from the column grid, because they cannot be recognised one row at a time.
    /// </remarks>
    private static void ApplyExplicitMerges(TableLevel table)
    {
        for (int index = table.RowCells.Count - 1; index >= 1; index--)
        {
            if (!table.RowCells[index].IsHorizontallyMerged) continue;

            CellDraft owner = table.RowCells[index - 1];
            // The merged cell's right edge becomes the owner's: that is what the pair covers.
            owner.RightEdge = Math.Max(owner.RightEdge, table.RowCells[index].RightEdge);
            owner.Content.AddRange(table.RowCells[index].Content);
            owner.LayoutParagraphs.AddRange(table.RowCells[index].LayoutParagraphs);
            table.RowCells.RemoveAt(index);
        }
    }

    /// <summary>
    /// Materialises one level's accumulated rows as a table, or null when that level has none.
    /// </summary>
    private ContentTable? FinishTable(Flow flow, int level)
    {
        if (level <= 0 || level > flow.Levels.Count) return null;

        TableLevel table = flow.Levels[level - 1];
        if (table.TableRows.Count == 0) return null;

        AssignColumns(table.TableRows);
        ResolveVerticalMerges(table.TableRows);

        // The layout copy, taken before the rows are cleared. Only the body's outermost tables: a nested
        // one is laid out as part of its parent cell's flow, which has no grid to hold it.
        if (level == 1 && ReferenceEquals(flow, _flows[0])
            && LayoutTableOf(table.TableRows) is { } laid)
        {
            _layoutBlocks.Add(laid);
        }

        ContentTable content = new()
        {
            ColumnCount = table.TableRows.Max(
                r => r.Cells.Count == 0 ? 0 : r.Cells.Max(c => c.ColumnStart + c.ColumnSpan)),
            // Only the run of header rows at the top counts: \trhdr on a row further down does not
            // make the rows above it headers.
            HeaderRowCount = table.TableRows.TakeWhile(r => r.IsHeader).Count(),
        };

        foreach (RowDraft row in table.TableRows)
        {
            ContentTableRow contentRow = new() { Index = row.Index };
            foreach (CellDraft cell in row.Cells)
            {
                if (cell.ContinuesMergeAbove) continue;

                ContentTableCell contentCell = new()
                {
                    Row = row.Index,
                    Column = cell.ColumnStart,
                    ColumnSpan = cell.ColumnSpan,
                    RowSpan = cell.RowSpan,
                };
                foreach (ContentNode node in cell.Content) contentCell.Children.Add(node);
                contentRow.Children.Add(contentCell);
            }
            content.Children.Add(contentRow);
        }

        table.TableRows.Clear();
        return content;
    }

    /// <summary>
    /// Derives each cell's column and span from the table's column grid.
    /// </summary>
    /// <remarks>
    /// The grid is every distinct right edge any row declares. A cell then occupies the grid
    /// columns between the previous cell's edge and its own — so a cell reaching an edge two
    /// columns along spans two columns, which is how a LibreOffice-written merge is recognised
    /// without any merge flag being present.
    /// </remarks>
    private static void AssignColumns(List<RowDraft> rows)
    {
        List<int> grid = [.. rows.SelectMany(r => r.Cells).Select(c => c.RightEdge).Distinct().Order()];

        // A table whose cells declare no edges at all — malformed, or written by a producer that
        // omits \cellx — falls back to counting cells, which is what the positions would have
        // been anyway.
        bool hasGeometry = grid.Count > 0 && grid[^1] > 0;

        foreach (RowDraft row in rows)
        {
            int previousEdge = row.LeftEdge;
            int sequential = 0;

            foreach (CellDraft cell in row.Cells)
            {
                if (!hasGeometry)
                {
                    cell.ColumnStart = sequential;
                    cell.ColumnSpan = 1;
                    sequential++;
                    continue;
                }

                int start = CountEdgesUpTo(grid, previousEdge);
                int end = CountEdgesUpTo(grid, cell.RightEdge);

                cell.ColumnStart = start;
                cell.ColumnSpan = Math.Max(1, end - start);
                previousEdge = cell.RightEdge;
            }
        }

        // How many grid columns end at or before an edge: the cell's position in the grid.
        static int CountEdgesUpTo(List<int> grid, int edge)
        {
            int count = 0;
            foreach (int boundary in grid)
            {
                if (boundary <= edge) count++;
                else break;
            }
            return count;
        }
    }

    /// <summary>
    /// A cell's padding: its own <c>\clpad*</c>, then the row's <c>\trpadd*</c>, then <c>\trgaph</c>.
    /// </summary>
    /// <remarks>
    /// Three spellings of one quantity, in order of decreasing specificity. Only the first of them names
    /// its sides the way it means them — see <see cref="CellPaddingSide"/>.
    /// </remarks>
    private static CellPadding PaddingOf(CellDefinition definition, TableLevel table)
    {
        int?[] cell = definition.Padding ?? [null, null, null, null];
        int?[] row = table.RowPadding;
        Length halfGap = Length.FromTwips(table.RowHalfGap ?? 0);

        return new CellPadding(
            Side(0) ?? halfGap,
            Side(1) ?? halfGap,
            Side(2) ?? Length.Zero,
            Side(3) ?? Length.Zero);

        // The cell's own value, then the row's default, then the caller's fallback. \trgaph is half the gap
        // between two cells, so it is the padding on each side of one and says nothing about top or bottom.
        Length? Side(int index)
        {
            if (cell[index] is { } stated) return Length.FromTwips(stated);
            if (row[index] is { } declared) return Length.FromTwips(declared);
            return null;
        }
    }

    /// <summary>
    /// The table's rows and column grid as layout wants them, or null when it has no geometry.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Built from the same edges <see cref="AssignColumns"/> derived the columns from, so the two cannot
    /// disagree: a column's width is the distance between two consecutive edges, measured from the row's
    /// own left. RTF states edges and the layout engine wants widths, and this is the one place the two
    /// meet.
    /// </para>
    /// <para>
    /// The left indent comes from the first row's <c>\trleft</c>. A table whose rows are indented
    /// differently is legal and rare; taking the first row's is what makes the common case — every row
    /// sharing one indent — exact, and leaves the rare one out by the difference rather than by the whole
    /// indent.
    /// </para>
    /// </remarks>
    private static RtfLayoutTable? LayoutTableOf(List<RowDraft> rows)
    {
        List<int> edges = [.. rows.SelectMany(r => r.Cells).Select(c => c.RightEdge).Distinct().Order()];
        if (edges.Count == 0 || edges[^1] <= 0) return null;

        int left = rows.Count > 0 ? rows[0].LeftEdge : 0;

        List<Length> widths = new(edges.Count);
        int previous = left;
        foreach (int edge in edges)
        {
            widths.Add(Length.FromTwips(Math.Max(0, edge - previous)));
            previous = edge;
        }

        List<RtfLayoutRow> layoutRows = new(rows.Count);
        foreach (RowDraft row in rows)
        {
            List<RtfLayoutCell> cells = [];
            foreach (CellDraft cell in row.Cells)
            {
                if (cell.ContinuesMergeAbove) continue;

                cells.Add(new RtfLayoutCell(
                    cell.ColumnStart,
                    cell.ColumnSpan,
                    cell.RowSpan,
                    cell.Padding,
                    cell.VerticalAlignment,
                    [.. cell.LayoutParagraphs]));
            }

            layoutRows.Add(new RtfLayoutRow(cells, Length.FromTwips(row.Height), row.IsHeader));
        }

        return new RtfLayoutTable(
            widths,
            layoutRows,
            rows.TakeWhile(r => r.IsHeader).Count(),
            Length.FromTwips(left));
    }

    private static void ResolveVerticalMerges(List<RowDraft> rows)
    {
        for (int rowIndex = 0; rowIndex < rows.Count; rowIndex++)
        {
            foreach (CellDraft cell in rows[rowIndex].Cells)
            {
                if (!cell.ContinuesMergeAbove) continue;

                for (int above = rowIndex - 1; above >= 0; above--)
                {
                    CellDraft? owner = rows[above].Cells
                        .FirstOrDefault(c => c.ColumnStart == cell.ColumnStart);
                    if (owner is null) break;
                    if (owner.ContinuesMergeAbove) continue;

                    owner.RowSpan++;
                    break;
                }
            }
        }
    }
}
