using System.Buffers.Binary;
using Paperless.Core.Extraction;
using Paperless.Core.Graphics;

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

            ReadOnlySpan<byte> descriptor = span[(descriptorsAt + (i * CellDescriptorSize))..];

            // Twenty bytes: sixteen bits of flags, two reserved, then four four-byte border codes in
            // WW8's own side order. The two reserved bytes are what makes the newer descriptor twenty
            // rather than eighteen, and skipping them reads the flags as the top border.
            ushort flags = BinaryPrimitives.ReadUInt16LittleEndian(descriptor);

            cells[i] = new Ww8CellDefinition(
                IsFirstMerged: (flags & 0x0001) != 0,
                IsMerged: (flags & 0x0002) != 0,
                IsVerticallyMerged: (flags & 0x0020) != 0,
                StartsVerticalMerge: (flags & 0x0040) != 0,
                Borders: BordersIn(descriptor));
        }

        return new Ww8TableDefinition(edges, cells);
    }

    /// <summary>
    /// The four border codes one cell descriptor carries.
    /// </summary>
    /// <remarks>
    /// A cell that states none leaves all four unset, which is not the same as stating that it has none —
    /// see <see cref="Ww8Border.IsUnstated"/>. Only the newer four-byte form is read: the WW6 descriptor is
    /// ten bytes with two-byte codes, and a WW6 table gets no borders rather than wrong ones.
    /// </remarks>
    private static Ww8CellBorders BordersIn(ReadOnlySpan<byte> descriptor)
    {
        const int firstBorder = 4;

        return new Ww8CellBorders(
            Side(descriptor, 0), Side(descriptor, 1), Side(descriptor, 2), Side(descriptor, 3));

        static Ww8Border Side(ReadOnlySpan<byte> descriptor, int index)
            => Ww8Border.ReadShort(descriptor[Math.Min(
                   descriptor.Length, firstBorder + (index * Ww8Border.ShortLength))..])
               ?? default;
    }

    /// <summary>Which cell <c>sprmTDefTableShd2nd</c>'s shading starts at.</summary>
    /// <remarks>
    /// Twenty-two, which is neither a round number nor a third of Word's sixty-three column limit — it is
    /// simply how many ten-byte entries fit in the operand before Word has to split it. A reader taking the
    /// second and third sprms from cell zero shades the first columns three times and the rest never.
    /// </remarks>
    private const int SecondShadingCell = 22;

    /// <summary>Which cell <c>sprmTDefTableShd3rd</c>'s shading starts at.</summary>
    private const int ThirdShadingCell = 44;

    /// <summary>
    /// Parses a <c>sprmTSetBrc</c> or <c>sprmTSetBrc80</c> operand.
    /// </summary>
    /// <param name="operand">The operand: a first cell, a limit, a flag byte, then one border code.</param>
    /// <param name="isLongForm">
    /// True for <c>sprmTSetBrc</c>, whose border code is the eight-byte form. The older sprm has the same
    /// three-byte header and a four-byte code, and a document carries both.
    /// </param>
    /// <remarks>
    /// A range and not a cell, which is what makes this the largest of the four formats' border reads: one
    /// sprm can set the left edge of every cell in the row, and four of them describe a fully bordered row.
    /// An entry whose first cell is past its limit is dropped rather than clamped — it names no cells, so
    /// there is nothing to apply it to.
    /// </remarks>
    private static Ww8BorderOverride? ReadBorderOverride(ReadOnlyMemory<byte> operand, bool isLongForm)
    {
        const int headerLength = 3;

        ReadOnlySpan<byte> bytes = operand.Span;
        if (bytes.Length < headerLength) return null;

        int first = bytes[0];
        int limit = Math.Min((int)bytes[1], MaxTableColumns + 1);
        if (first >= limit) return null;

        Ww8Border? border = isLongForm
            ? Ww8Border.ReadLong(bytes[headerLength..])
            : Ww8Border.ReadShort(bytes[headerLength..]);

        return border is { } stated
            ? new Ww8BorderOverride(first, limit, bytes[2], stated)
            : null;
    }

    /// <summary>
    /// One shading list laid over another, entry by entry, with the newer entries winning where they say
    /// anything.
    /// </summary>
    /// <remarks>
    /// The three shading sprms describe disjoint ranges of cells, so in practice each fills in where the
    /// others were silent — but they are merged rather than concatenated because nothing guarantees a
    /// document writes them in order, or writes all three.
    /// </remarks>
    private static List<Colour?> Overlay(IReadOnlyList<Colour?>? under, List<Colour?> over)
    {
        if (under is null) return over;

        List<Colour?> merged = [.. under];
        while (merged.Count < over.Count) merged.Add(null);

        for (int i = 0; i < over.Count; i++)
        {
            if (over[i] is { } stated) merged[i] = stated;
        }

        return merged;
    }

    /// <summary>
    /// The list a paragraph at a nesting level belongs in: an open cell's content, or the section.
    /// </summary>
    private static IList<ContentNode> Destination(WalkState state, int level)
    {
        if (level <= 0) return state.Target.Children;

        EnsureLevels(state, level);
        return state.Levels[level - 1].CellContent;
    }

    private static void EnsureLevels(WalkState state, int level)
    {
        while (state.Levels.Count < level) state.Levels.Add(new Ww8TableLevel());
    }

    /// <summary>
    /// Materialises every table nested deeper than a level, innermost first.
    /// </summary>
    /// <remarks>
    /// A finished inner table goes into the cell of the table that encloses it, which is why this has
    /// to run innermost first: the inner table must exist before the cell holding it is closed. WW8
    /// marks no end to a table, so the only signal is content arriving at a shallower level.
    /// </remarks>
    private static void CloseTablesDeeperThan(WalkState state, int level)
    {
        for (int deeper = state.Levels.Count; deeper > level; deeper--)
        {
            if (FinishTable(state, deeper) is { } table) Destination(state, deeper - 1).Add(table);
            state.Levels.RemoveAt(deeper - 1);
        }
    }

    private static void FinishCell(WalkState state, int level)
    {
        EnsureLevels(state, level);
        Ww8TableLevel table = state.Levels[level - 1];

        Ww8CellDraft cell = new();
        cell.Content.AddRange(table.CellContent);
        table.CellContent.Clear();
        table.RowCells.Add(cell);
    }

    /// <summary>
    /// Closes a row, attaching the geometry its row-end paragraph declared.
    /// </summary>
    /// <param name="state">The walk in progress.</param>
    /// <param name="format">
    /// The row-end paragraph's properties, which carry the geometry. Passed in rather than read from
    /// the walk state, because the state's copy is cleared when that paragraph is finished.
    /// </param>
    /// <param name="level">Which nesting level's table the row belongs to.</param>
    /// <remarks>
    /// The geometry arrives with the row's <em>end</em>, so it can only be applied once every cell
    /// has been collected — which is also why a cell cannot know its own column while it is being
    /// read.
    /// </remarks>
    private static void FinishRow(WalkState state, Ww8ParagraphFormat format, int level)
    {
        EnsureLevels(state, level);
        Ww8TableLevel table = state.Levels[level - 1];

        if (table.CellContent.Count > 0) FinishCell(state, level);
        if (table.RowCells.Count == 0) return;

        Ww8TableDefinition? definition = format.TableDefinition;
        for (int i = 0; i < table.RowCells.Count; i++)
        {
            table.RowCells[i].RightEdge = definition?.RightEdgeOf(i) ?? 0;
            Ww8CellDefinition cell = definition?.CellAt(i) ?? default;
            table.RowCells[i].IsHorizontallyMerged = cell.IsMerged;
            table.RowCells[i].ContinuesMergeAbove = cell.IsVerticallyMerged && !cell.StartsVerticalMerge;
        }

        ApplyExplicitMerges(table);

        Ww8RowDraft row = new()
        {
            Index = table.Rows.Count,
            LeftEdge = definition?.LeftEdge ?? 0,
            IsHeader = format.IsTableHeaderRow,
        };
        row.Cells.AddRange(table.RowCells);
        table.Rows.Add(row);

        table.RowCells.Clear();
    }

    /// <summary>
    /// Folds cells the definition marks as merged into the cell they were merged with.
    /// </summary>
    /// <remarks>
    /// Only the merges a producer flags. A geometric merge cannot be recognised one row at a time —
    /// it needs the whole table's grid — so it is resolved later.
    /// </remarks>
    private static void ApplyExplicitMerges(Ww8TableLevel table)
    {
        for (int index = table.RowCells.Count - 1; index >= 1; index--)
        {
            if (!table.RowCells[index].IsHorizontallyMerged) continue;

            Ww8CellDraft owner = table.RowCells[index - 1];
            owner.RightEdge = Math.Max(owner.RightEdge, table.RowCells[index].RightEdge);
            owner.Content.AddRange(table.RowCells[index].Content);
            table.RowCells.RemoveAt(index);
        }
    }

    /// <summary>
    /// Materialises one level's rows as a table, or null when that level holds none.
    /// </summary>
    private static ContentTable? FinishTable(WalkState state, int level)
    {
        if (level <= 0 || level > state.Levels.Count) return null;

        Ww8TableLevel level_ = state.Levels[level - 1];

        // Content collected but never closed by a row mark: a truncated table, whose cells are still
        // worth keeping.
        if (level_.CellContent.Count > 0 || level_.RowCells.Count > 0)
            FinishRow(state, default, level);

        if (level_.Rows.Count == 0) return null;

        AssignColumns(level_.Rows);
        ResolveVerticalMerges(level_.Rows);

        ContentTable table = new()
        {
            ColumnCount = level_.Rows.Max(
                r => r.Cells.Count == 0 ? 0 : r.Cells.Max(c => c.ColumnStart + c.ColumnSpan)),
            // Only the header rows at the top count: sprmTTableHeader on a row further down does
            // not make the rows above it headers, and Word does write it that way.
            HeaderRowCount = level_.Rows.TakeWhile(r => r.IsHeader).Count(),
        };

        foreach (Ww8RowDraft row in level_.Rows)
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

        level_.Rows.Clear();
        return table;
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

        /// <summary>
        /// The cell's blocks with the formatting layout needs, beside the content nodes.
        /// </summary>
        /// <remarks>
        /// One draft type serving two walks, which is what lets the column grid, the spans and the vertical
        /// merges be resolved once — <c>AssignColumns</c> and <c>ResolveVerticalMerges</c> care only about
        /// the edges. Whichever walk built the draft fills its own list and leaves the other empty. Blocks
        /// rather than paragraphs because a cell can hold a table, which is what <c>sprmPItap</c>'s depth
        /// expresses.
        /// </remarks>
        public List<Ww8LayoutBlock> LayoutBlocks { get; } = [];

        /// <summary>The cell's right edge in twips, which is what places it in the grid.</summary>
        public int RightEdge { get; set; }

        /// <summary>The gap between the cell's edges and its text, from the row's padding sprms.</summary>
        public Layout.CellPadding Padding { get; set; }

        /// <summary>Its four edges, from the cell descriptor and whatever <c>sprmTSetBrc</c> laid over it.</summary>
        public Layout.CellBorders Borders { get; set; }

        /// <summary>Its background, or null when the row's shading sprms give it none.</summary>
        public Colour? Shading { get; set; }

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

        /// <summary>Its declared height in twips, signed: negative means exact rather than a floor.</summary>
        public int HeightTwips { get; init; }

        public List<Ww8CellDraft> Cells { get; } = [];
    }

    /// <summary>One table under construction, at one nesting level.</summary>
    private sealed class Ww8TableLevel
    {
        public List<ContentNode> CellContent { get; } = [];
        public List<Ww8CellDraft> RowCells { get; } = [];
        public List<Ww8RowDraft> Rows { get; } = [];
    }
}

/// <summary>One cell's merge flags from a table definition's descriptor.</summary>
/// <param name="IsFirstMerged">The cell begins a horizontal merge.</param>
/// <param name="IsMerged">The cell is swallowed by the merge that began to its left.</param>
/// <param name="IsVerticallyMerged">The cell takes part in a vertical merge.</param>
/// <param name="StartsVerticalMerge">
/// The cell is the top of that vertical merge rather than a continuation of it.
/// </param>
/// <param name="Borders">Its four border codes, as the descriptor states them.</param>
public readonly record struct Ww8CellDefinition(
    bool IsFirstMerged,
    bool IsMerged,
    bool IsVerticallyMerged,
    bool StartsVerticalMerge,
    Ww8CellBorders Borders = default);

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
