using Paperless.Core.Extraction;

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
/// </remarks>
public sealed partial class RtfDocumentReader
{
    /// <summary>
    /// The widest row Paperless materialises. Word's own limit is 63 columns, and a row declaring
    /// more is malformed.
    /// </summary>
    public const int MaxTableColumns = 63;

    private static void BeginRowDefinition(Flow flow)
    {
        // \trowd may be repeated to restate a row's definition before it is filled, so an open
        // definition is replaced rather than nested.
        flow.InRowDefinition = true;
        flow.CellDefinitions.Clear();
        flow.RowLeftEdge = 0;
        ClearPendingCellFlags(flow);
    }

    private static void AddCellDefinition(Flow flow, int? rightEdge)
    {
        if (flow.CellDefinitions.Count >= MaxTableColumns)
        {
            ClearPendingCellFlags(flow);
            return;
        }

        flow.CellDefinitions.Add(new CellDefinition(
            rightEdge ?? 0,
            flow.PendingCellMergesFirst,
            flow.PendingCellMerged,
            flow.PendingCellVerticalFirst,
            flow.PendingCellVerticalMerged));
        ClearPendingCellFlags(flow);
    }

    private static void ClearPendingCellFlags(Flow flow)
    {
        flow.PendingCellMergesFirst = false;
        flow.PendingCellMerged = false;
        flow.PendingCellVerticalFirst = false;
        flow.PendingCellVerticalMerged = false;
    }

    /// <summary>Ends a cell at a <c>\cell</c>.</summary>
    private void EndCell(GroupState state)
    {
        if (state.Destination is not RtfDestination.Body) return;

        Flow flow = CurrentFlow;

        // A cell whose row definition never arrived: treat the content as body text rather than
        // losing it.
        if (!flow.InRowDefinition)
        {
            FinishParagraph(flow, force: true);
            return;
        }

        // The cell's last paragraph usually has no \par of its own — \cell ends it.
        FinishParagraph(flow, force: flow.CellContent.Count == 0);

        int index = flow.RowCells.Count;
        CellDefinition definition = index < flow.CellDefinitions.Count
            ? flow.CellDefinitions[index]
            : default;

        CellDraft cell = new()
        {
            RightEdge = definition.RightEdge,
            IsHorizontallyMerged = definition.Merged,
            ContinuesMergeAbove = definition.VerticalMerged,
        };
        cell.Content.AddRange(flow.CellContent);
        flow.CellContent.Clear();
        flow.RowCells.Add(cell);
    }

    /// <summary>Ends a row at a <c>\row</c>.</summary>
    private void EndRow(GroupState state)
    {
        if (state.Destination is not RtfDestination.Body) return;

        Flow flow = CurrentFlow;
        if (!flow.InRowDefinition) return;

        // Content written after the last \cell but before \row still belongs to a cell.
        if (flow.CellContent.Count > 0) EndCell(state);

        ApplyExplicitMerges(flow);

        RowDraft row = new() { Index = flow.TableRows.Count, LeftEdge = flow.RowLeftEdge };
        row.Cells.AddRange(flow.RowCells);
        flow.TableRows.Add(row);

        flow.RowCells.Clear();
        flow.CellContent.Clear();
        flow.InRowDefinition = false;
    }

    /// <summary>
    /// Folds cells marked <c>\clmrg</c> into the cell they were merged with.
    /// </summary>
    /// <remarks>
    /// Only the explicitly flagged merges Word writes. LibreOffice's geometric merges are resolved
    /// later, from the column grid, because they cannot be recognised one row at a time.
    /// </remarks>
    private static void ApplyExplicitMerges(Flow flow)
    {
        for (int index = flow.RowCells.Count - 1; index >= 1; index--)
        {
            if (!flow.RowCells[index].IsHorizontallyMerged) continue;

            CellDraft owner = flow.RowCells[index - 1];
            // The merged cell's right edge becomes the owner's: that is what the pair covers.
            owner.RightEdge = Math.Max(owner.RightEdge, flow.RowCells[index].RightEdge);
            owner.Content.AddRange(flow.RowCells[index].Content);
            flow.RowCells.RemoveAt(index);
        }
    }

    /// <summary>
    /// Materialises the accumulated rows as a table, if there are any.
    /// </summary>
    /// <remarks>
    /// Called when something other than a row follows, and again when the flow ends: a table at
    /// the very end of a document has nothing after it to close it.
    /// </remarks>
    private static void FinishTable(Flow flow)
    {
        if (flow.TableRows.Count == 0) return;

        AssignColumns(flow.TableRows);
        ResolveVerticalMerges(flow.TableRows);

        ContentTable table = new()
        {
            ColumnCount = flow.TableRows.Max(
                r => r.Cells.Count == 0 ? 0 : r.Cells.Max(c => c.ColumnStart + c.ColumnSpan)),
            // RTF marks a repeated header row with \trhdr, which Paperless does not read yet;
            // reporting a count it has not established would be a guess.
            HeaderRowCount = 0,
        };

        foreach (RowDraft row in flow.TableRows)
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
            table.Children.Add(contentRow);
        }

        flow.TableRows.Clear();
        flow.Target.Children.Add(table);
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
