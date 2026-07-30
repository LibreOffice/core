using System.Buffers.Binary;
using Paperless.Core.Extraction;

namespace Paperless.WordProcessing.Ww8;

/// <content>Table reading for WW8.</content>
/// <remarks>
/// <para>
/// WW8 has no table element either. A row is a run of paragraphs whose properties say
/// <c>sprmPFInTable</c>, its cells separated by U+0007, and the row ends at the U+0007 whose
/// paragraph also says <c>sprmPFTtp</c>. Consecutive rows form a table only by being adjacent, which
/// is why the table is closed by the first thing that is not another row rather than by any marker.
/// </para>
/// <para>
/// The row's geometry comes from <c>sprmTDefTable</c> on that same row-end paragraph: a column count,
/// the column edges in twips, and a twenty-byte descriptor per cell carrying the merge flags. Both
/// halves matter, because a horizontal merge is expressed <em>either</em> way. Word writes the flags;
/// LibreOffice writes geometry alone — a merged cell's edge simply reaches where two columns end in
/// the rows around it — so the span has to be derived from the table's column grid as well as read
/// off the cell. This is the same split the RTF reader faces, for the same reason: one producer
/// marks the merge and the other implies it.
/// </para>
/// </remarks>
public sealed partial class Ww8DocumentReader
{
    /// <summary>
    /// The widest row Paperless materialises. Word's own limit is 63 columns, and a row declaring
    /// more is malformed.
    /// </summary>
    public const int MaxTableColumns = 63;

    /// <summary>The size of one cell descriptor in a table definition.</summary>
    private const int CellDescriptorSize = 20;

    /// <summary>
    /// Parses a <c>sprmTDefTable</c> operand: a column count, the edges, then the cell descriptors.
    /// </summary>
    /// <remarks>
    /// A definition may declare fewer descriptors than columns — the trailing ones are then defaults
    /// — so the descriptor count is derived from what is left of the operand rather than assumed to
    /// match. Returning null for a malformed operand leaves the row to be laid out by cell count,
    /// which is what the geometry would have said anyway.
    /// </remarks>
    private static Ww8TableDefinition? ReadTableDefinition(ReadOnlyMemory<byte> operand)
    {
        ReadOnlySpan<byte> span = operand.Span;
        if (span.Length < 1) return null;

        int columns = span[0];
        if (columns is <= 0 or > MaxTableColumns) return null;

        int edgesBytes = 2 * (columns + 1);
        if (1 + edgesBytes > span.Length) return null;

        int[] edges = new int[columns + 1];
        for (int i = 0; i <= columns; i++)
            edges[i] = BinaryPrimitives.ReadInt16LittleEndian(span[(1 + (2 * i))..]);

        Ww8CellDefinition[] cells = new Ww8CellDefinition[columns];
        int descriptorsAt = 1 + edgesBytes;
        int available = (span.Length - descriptorsAt) / CellDescriptorSize;

        for (int i = 0; i < columns; i++)
        {
            if (i >= available) break;

            // The flags are the descriptor's first sixteen bits; the rest is width and borders.
            ushort flags = BinaryPrimitives.ReadUInt16LittleEndian(
                span[(descriptorsAt + (i * CellDescriptorSize))..]);

            cells[i] = new Ww8CellDefinition(
                IsFirstMerged: (flags & 0x0001) != 0,
                IsMerged: (flags & 0x0002) != 0,
                IsVerticallyMerged: (flags & 0x0020) != 0,
                StartsVerticalMerge: (flags & 0x0040) != 0);
        }

        return new Ww8TableDefinition(edges, cells);
    }

    private static void FinishCell(WalkState state)
    {
        Ww8CellDraft cell = new();
        cell.Content.AddRange(state.CellContent);
        state.CellContent.Clear();
        state.RowCells.Add(cell);
    }

    /// <summary>
    /// Closes a row, attaching the geometry its row-end paragraph declared.
    /// </summary>
    /// <param name="state">The walk in progress.</param>
    /// <param name="format">
    /// The row-end paragraph's properties, which carry the geometry. Passed in rather than read from
    /// the walk state, because the state's copy is cleared when that paragraph is finished.
    /// </param>
    /// <remarks>
    /// The geometry arrives with the row's <em>end</em>, so it can only be applied once every cell
    /// has been collected — which is also why a cell cannot know its own column while it is being
    /// read.
    /// </remarks>
    private static void FinishRow(WalkState state, Ww8ParagraphFormat format)
    {
        if (state.CellContent.Count > 0) FinishCell(state);
        if (state.RowCells.Count == 0)
        {
            state.InCell = false;
            return;
        }

        Ww8TableDefinition? definition = format.TableDefinition;
        for (int i = 0; i < state.RowCells.Count; i++)
        {
            state.RowCells[i].RightEdge = definition?.RightEdgeOf(i) ?? 0;
            Ww8CellDefinition cell = definition?.CellAt(i) ?? default;
            state.RowCells[i].IsHorizontallyMerged = cell.IsMerged;
            state.RowCells[i].ContinuesMergeAbove = cell.IsVerticallyMerged && !cell.StartsVerticalMerge;
        }

        ApplyExplicitMerges(state);

        Ww8RowDraft row = new()
        {
            Index = state.Rows.Count,
            LeftEdge = definition?.LeftEdge ?? 0,
            IsHeader = format.IsTableHeaderRow,
        };
        row.Cells.AddRange(state.RowCells);
        state.Rows.Add(row);

        state.RowCells.Clear();
        state.InCell = false;
    }

    /// <summary>
    /// Folds cells the definition marks as merged into the cell they were merged with.
    /// </summary>
    /// <remarks>
    /// Only the merges a producer flags. A geometric merge cannot be recognised one row at a time —
    /// it needs the whole table's grid — so it is resolved later.
    /// </remarks>
    private static void ApplyExplicitMerges(WalkState state)
    {
        for (int index = state.RowCells.Count - 1; index >= 1; index--)
        {
            if (!state.RowCells[index].IsHorizontallyMerged) continue;

            Ww8CellDraft owner = state.RowCells[index - 1];
            owner.RightEdge = Math.Max(owner.RightEdge, state.RowCells[index].RightEdge);
            owner.Content.AddRange(state.RowCells[index].Content);
            state.RowCells.RemoveAt(index);
        }
    }

    private static void FinishTable(WalkState state)
    {
        if (state.Rows.Count == 0) return;

        AssignColumns(state.Rows);
        ResolveVerticalMerges(state.Rows);

        ContentTable table = new()
        {
            ColumnCount = state.Rows.Max(
                r => r.Cells.Count == 0 ? 0 : r.Cells.Max(c => c.ColumnStart + c.ColumnSpan)),
            // Only the header rows at the top count: sprmTTableHeader on a row further down does
            // not make the rows above it headers, and Word does write it that way.
            HeaderRowCount = state.Rows.TakeWhile(r => r.IsHeader).Count(),
        };

        foreach (Ww8RowDraft row in state.Rows)
        {
            ContentTableRow contentRow = new() { Index = row.Index };
            foreach (Ww8CellDraft cell in row.Cells)
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

        state.Rows.Clear();
        state.Target.Children.Add(table);
    }

    /// <summary>
    /// Derives each cell's column and span from the table's column grid.
    /// </summary>
    /// <remarks>
    /// The grid is every distinct edge any row declares. A cell occupies the grid columns between the
    /// previous cell's edge and its own — so a cell reaching an edge two columns along spans two
    /// columns, which is how a merge that carries no flag is recognised. A table whose rows declare
    /// no geometry falls back to counting cells.
    /// </remarks>
    private static void AssignColumns(List<Ww8RowDraft> rows)
    {
        List<int> grid = [.. rows.SelectMany(r => r.Cells).Select(c => c.RightEdge).Distinct().Order()];
        bool hasGeometry = grid.Count > 0 && grid[^1] > grid[0];

        foreach (Ww8RowDraft row in rows)
        {
            int previousEdge = row.LeftEdge;
            int sequential = 0;

            foreach (Ww8CellDraft cell in row.Cells)
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
    /// Turns each continuation cell into a row span on the cell that started the merge.
    /// </summary>
    /// <remarks>
    /// WW8 marks the continuation rather than the extent, exactly as DOCX's <c>w:vMerge</c> does, so
    /// the span has to be counted upwards from each continuation — which is only possible once every
    /// row's columns are known.
    /// </remarks>
    private static void ResolveVerticalMerges(List<Ww8RowDraft> rows)
    {
        for (int rowIndex = 0; rowIndex < rows.Count; rowIndex++)
        {
            foreach (Ww8CellDraft cell in rows[rowIndex].Cells)
            {
                if (!cell.ContinuesMergeAbove) continue;

                for (int above = rowIndex - 1; above >= 0; above--)
                {
                    Ww8CellDraft? owner = rows[above].Cells
                        .FirstOrDefault(c => c.ColumnStart == cell.ColumnStart);
                    if (owner is null) break;
                    if (owner.ContinuesMergeAbove) continue;

                    owner.RowSpan++;
                    break;
                }
            }
        }
    }

    private sealed class Ww8CellDraft
    {
        public List<ContentNode> Content { get; } = [];

        /// <summary>The cell's right edge in twips, which is what places it in the grid.</summary>
        public int RightEdge { get; set; }

        public bool IsHorizontallyMerged { get; set; }
        public bool ContinuesMergeAbove { get; set; }

        public int ColumnStart { get; set; }
        public int ColumnSpan { get; set; } = 1;
        public int RowSpan { get; set; } = 1;
    }

    private sealed class Ww8RowDraft
    {
        public int Index { get; init; }
        public int LeftEdge { get; init; }
        public bool IsHeader { get; init; }
        public List<Ww8CellDraft> Cells { get; } = [];
    }
}

/// <summary>One cell's merge flags from a table definition's descriptor.</summary>
/// <param name="IsFirstMerged">The cell begins a horizontal merge.</param>
/// <param name="IsMerged">The cell is swallowed by the merge that began to its left.</param>
/// <param name="IsVerticallyMerged">The cell takes part in a vertical merge.</param>
/// <param name="StartsVerticalMerge">
/// The cell is the top of that vertical merge rather than a continuation of it.
/// </param>
public readonly record struct Ww8CellDefinition(
    bool IsFirstMerged,
    bool IsMerged,
    bool IsVerticallyMerged,
    bool StartsVerticalMerge);

/// <summary>A table row's geometry, from <c>sprmTDefTable</c>.</summary>
/// <remarks>
/// The edges are absolute positions in twips, one more than there are columns: the first is the row's
/// left edge and each of the rest is a cell's right edge. Storing them rather than widths is what
/// lets rows with different cell counts be compared against one grid.
/// </remarks>
public sealed record Ww8TableDefinition(
    ReadOnlyMemory<int> Edges,
    ReadOnlyMemory<Ww8CellDefinition> Cells)
{
    /// <summary>The row's left edge.</summary>
    public int LeftEdge => Edges.Length > 0 ? Edges.Span[0] : 0;

    /// <summary>How many cells the definition describes.</summary>
    public int ColumnCount => Math.Max(0, Edges.Length - 1);

    /// <summary>A cell's right edge, or zero when the definition does not reach that cell.</summary>
    public int RightEdgeOf(int index)
        => index >= 0 && index + 1 < Edges.Length ? Edges.Span[index + 1] : 0;

    /// <summary>A cell's descriptor, or the default when the definition does not reach it.</summary>
    public Ww8CellDefinition CellAt(int index)
        => index >= 0 && index < Cells.Length ? Cells.Span[index] : default;
}
