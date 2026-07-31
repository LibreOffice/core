using System.Buffers.Binary;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.WordProcessing.Layout;

namespace Paperless.WordProcessing.Ww8;

/// <content>
/// Assembling a DOC's tables for layout, as the layout walk passes over them.
/// </content>
/// <remarks>
/// <para>
/// A second assembly of the same tables the content walk builds, for the same reason there are two walks
/// at all: the content tree keeps no font sizes or indents, and a cell's text has to break at the cell's
/// width. What is <em>not</em> duplicated is the hard part — the column grid, the spans and the vertical
/// merges are resolved by the very same <c>AssignColumns</c> and <c>ResolveVerticalMerges</c> the content
/// walk uses, because those work on the row drafts and care nothing for what a cell holds. So a cell draft
/// carries both a content list and a layout list, and whichever walk built it fills its own.
/// </para>
/// <para>
/// Only the outermost table is assembled. A nested one reuses the paragraph mark rather than U+0007 and
/// would have to be laid out inside its parent cell, which is a flow and has no grid to put a table in.
/// </para>
/// </remarks>
public sealed partial class Ww8DocumentReader
{
    /// <summary>
    /// Word's default cell padding, for a table whose rows declare none.
    /// </summary>
    /// <remarks>
    /// 108 twips at the sides and nothing vertically, which is what Word's own table dialogue starts at and
    /// what a DOC written without a padding sprm means. It comes out of the cell's width, so defaulting it
    /// to zero breaks a narrow cell's text one word late.
    /// </remarks>
    private static readonly CellPadding DefaultCellPadding = CellPadding.Word;

    /// <summary>
    /// Parses a <c>sprmTCellPadding</c> or <c>sprmTCellPaddingDefault</c> operand.
    /// </summary>
    /// <param name="operand">The six operand bytes.</param>
    /// <param name="isDefault">True for the default form, whose first two bytes are not a range.</param>
    /// <remarks>
    /// <para>
    /// Six bytes either way, and the same six: a first cell, a limit, a side mask, a size type, and a
    /// sixteen-bit value. What differs is only the first two — the default form writes cell zero and a byte
    /// LibreOffice's own reader calls "unknown", and means every cell of the row, while the specific form
    /// names a half-open range of cells.
    /// </para>
    /// <para>
    /// The fourth byte differs too, and this is the part that is easy to get wrong because the two operands
    /// look identical. In the specific form it is an <c>Fts</c> size type which must be 3,
    /// <c>FtsDxa</c> — LibreOffice rejects any other outright, "the size is wrong (or unconverted) and MUST
    /// be ignored" (<c>ww8par2.cxx</c>, <c>ProcessSpecificSpacing</c>). In the <em>default</em> form the
    /// same byte carries nothing at all: LibreOffice's <c>ProcessSpacing</c> skips it as "unknown" and never
    /// tests it. Requiring 3 there rejects every default a real document writes, which leaves a table with
    /// Word's 108-twip padding instead of its own — a visible three points per column.
    /// </para>
    /// </remarks>
    private static Ww8CellPadding? ReadCellPadding(ReadOnlyMemory<byte> operand, bool isDefault)
    {
        if (operand.Length < 6) return null;

        ReadOnlySpan<byte> bytes = operand.Span;

        int first = isDefault ? 0 : bytes[0];
        int limit = isDefault ? 0 : bytes[1];
        int sides = bytes[2];
        int sizeType = bytes[3];

        if (!isDefault && sizeType != FtsDxa) return null;
        if (!isDefault && (first >= limit || limit > MaxTableColumns + 1)) return null;

        return new Ww8CellPadding(
            first, limit, sides, BinaryPrimitives.ReadUInt16LittleEndian(bytes[4..]));
    }

    /// <summary>The only <c>Fts</c> width type that means twips.</summary>
    private const int FtsDxa = 3;

    /// <summary>
    /// Collects a table as the layout walk passes over its paragraphs.
    /// </summary>
    /// <remarks>
    /// Fed one paragraph at a time with the properties of the mark that ended it, which is where WW8 keeps
    /// everything that matters: whether the mark ended a cell or a row, whether the row is a heading, and
    /// the row's own column edges. None of it is knowable from the character alone — U+0007 ends both a cell
    /// and a row, and only <c>sprmPFTtp</c> says which.
    /// </remarks>
    private sealed class LayoutTableAssembler
    {
        private readonly List<Ww8LayoutBlock> _blocks = [];
        private readonly List<Level> _levels = [];

        /// <summary>Takes one paragraph, with the properties of the mark that ended it.</summary>
        /// <param name="paragraph">The paragraph.</param>
        /// <param name="format">Its mark's properties.</param>
        /// <param name="endsCell">
        /// True when the mark was U+0007. Only the outermost table uses that character; a nested table
        /// reuses the paragraph mark and says what it means with <c>sprmPFInnerTableCell</c> and
        /// <c>sprmPFInnerTtp</c> instead — which is why the depth cannot be read off the character.
        /// </param>
        public void Add(Ww8LayoutParagraph paragraph, Ww8ParagraphFormat format, bool endsCell)
        {
            // U+0007 always belongs to the outermost table. A paragraph mark belongs to whichever level its
            // own sprms name, and a mark that ends a nested cell is at least level two whatever the depth
            // sprm reads — the flag means "a table inside a cell", so it cannot be the outermost.
            int level = endsCell
                ? 1
                : format.IsInnerTableCell ? Math.Max(2, format.Level) : format.Level;

            if (level <= 0)
            {
                // The first paragraph outside every table closes them all, innermost first: consecutive
                // rows form a table only by being adjacent, and WW8 marks no end.
                CloseDeeperThan(0);
                _blocks.Add(new Ww8LayoutBlock(paragraph));
                return;
            }

            // Anything at a shallower level than the open tables closes the deeper ones first, and a
            // finished inner table lands in the cell of the level that encloses it.
            CloseDeeperThan(level);
            Level open = LevelAt(level);
            open.Section = paragraph.SectionIndex;

            bool endsRow = endsCell ? format.IsTableRowEnd : format.IsInnerTableRowEnd;

            if (endsRow)
            {
                // The row-end paragraph is the terminator rather than content, so it is dropped — but it
                // still closes whatever cell was open, and it carries the row's geometry.
                FinishRow(open, format);
                return;
            }

            bool closesCell = endsCell || format.IsInnerTableCell;
            if (paragraph.Text.Length > 0 || closesCell) open.Cell.Add(new Ww8LayoutBlock(paragraph));
            if (closesCell) FinishCell(open);
        }

        /// <summary>Everything collected, with any unterminated tables closed.</summary>
        public List<Ww8LayoutBlock> Finished()
        {
            CloseDeeperThan(0);
            return _blocks;
        }

        private Level LevelAt(int level)
        {
            while (_levels.Count < level) _levels.Add(new Level());
            return _levels[level - 1];
        }

        private static void FinishCell(Level level)
        {
            Ww8CellDraft cell = new();
            cell.LayoutBlocks.AddRange(level.Cell);
            level.Cell.Clear();
            level.RowCells.Add(cell);
        }

        private static void FinishRow(Level level, Ww8ParagraphFormat format)
        {
            if (level.Cell.Count > 0) FinishCell(level);
            if (level.RowCells.Count == 0) return;

            Ww8TableDefinition? definition = format.TableDefinition;

            for (int i = 0; i < level.RowCells.Count; i++)
            {
                level.RowCells[i].RightEdge = definition?.RightEdgeOf(i) ?? 0;
                level.RowCells[i].Padding = PaddingOf(format, i);
                level.RowCells[i].Shading = ShadingOf(format, i);

                Ww8CellDefinition cell = definition?.CellAt(i) ?? default;
                level.RowCells[i].IsHorizontallyMerged = cell.IsMerged;
                level.RowCells[i].ContinuesMergeAbove = cell.IsVerticallyMerged && !cell.StartsVerticalMerge;
                level.RowCells[i].Borders = BordersOf(format, cell.Borders, i);
            }

            ApplyExplicitMerges(level.RowCells);

            Ww8RowDraft row = new()
            {
                Index = level.Rows.Count,
                LeftEdge = definition?.LeftEdge ?? 0,
                IsHeader = format.IsTableHeaderRow,
                HeightTwips = format.RowHeightTwips,
                DefaultBorders = format.TableBorders,
            };
            row.Cells.AddRange(level.RowCells);
            level.Rows.Add(row);

            level.RowCells.Clear();
        }

        /// <summary>
        /// Materialises every open level deeper than one, innermost first.
        /// </summary>
        /// <remarks>
        /// Innermost first, because a finished inner table goes into a cell of the table that encloses it
        /// and so has to exist before that cell is closed. The outermost level's table goes into the body's
        /// own block list instead, which is what makes it a block the paginator sees.
        /// </remarks>
        private void CloseDeeperThan(int level)
        {
            for (int deeper = _levels.Count; deeper > Math.Max(0, level); deeper--)
            {
                Level open = _levels[deeper - 1];

                // Cells collected but never closed by a row mark: a truncated table, still worth keeping.
                if (open.Cell.Count > 0) FinishCell(open);
                if (open.RowCells.Count > 0) FinishRow(open, default);

                if (open.Rows.Count > 0)
                {
                    AssignColumns(open.Rows);
                    ResolveVerticalMerges(open.Rows);

                    if (LayoutTableOf(open.Rows, open.Section) is { } table)
                    {
                        if (deeper == 1) _blocks.Add(new Ww8LayoutBlock(table));
                        else LevelAt(deeper - 1).Cell.Add(new Ww8LayoutBlock(table));
                    }
                }

                _levels.RemoveAt(deeper - 1);
            }
        }

        /// <summary>One table under construction, at one nesting level.</summary>
        private sealed class Level
        {
            public List<Ww8LayoutBlock> Cell { get; } = [];

            public List<Ww8CellDraft> RowCells { get; } = [];

            public List<Ww8RowDraft> Rows { get; } = [];

            /// <summary>The section its rows were in, taken from the paragraphs that made them.</summary>
            public int Section { get; set; }
        }
    }

    /// <summary>
    /// The rows and column grid as layout wants them, or null when the table has no geometry.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Built from the same edges <c>AssignColumns</c> derived the columns from, so the two cannot disagree.
    /// WW8 states absolute positions in twips — the first edge is the row's left and each of the rest is a
    /// cell's right — and the layout engine wants widths, so this is where the two conventions meet.
    /// </para>
    /// <para>
    /// The edges are measured from the text area's left, which is where a row's left edge of zero means the
    /// margin. A row indented from it carries that in its own first edge, so the table's indent is the
    /// first row's left edge and the column widths are the gaps after it.
    /// </para>
    /// </remarks>
    private static Ww8LayoutTable? LayoutTableOf(List<Ww8RowDraft> rows, int section)
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

        List<Ww8LayoutRow> layoutRows = new(rows.Count);
        foreach (Ww8RowDraft row in rows)
        {
            List<Ww8LayoutCell> cells = [];
            foreach (Ww8CellDraft cell in row.Cells)
            {
                if (cell.ContinuesMergeAbove) continue;

                cells.Add(new Ww8LayoutCell(
                    cell.ColumnStart,
                    cell.ColumnSpan,
                    cell.RowSpan,
                    cell.Padding,
                    [.. cell.LayoutBlocks],
                    cell.Shading,
                    // Resolved here rather than as the row was closed, because the defaults a missing
                    // side falls back to depend on the cell's place in the whole table — and a row being
                    // built does not know whether another follows it.
                    ResolveBorders(
                        cell.Borders,
                        row.DefaultBorders,
                        isFirstRow: row.Index == 0,
                        isLastRow: row.Index == rows.Count - 1,
                        isFirstCell: cell.ColumnStart == 0,
                        isLastCell: cell.ColumnStart + cell.ColumnSpan >= widths.Count)));
            }

            layoutRows.Add(new Ww8LayoutRow(
                cells,
                row.IsHeader,
                Length.FromTwips(Math.Abs(row.HeightTwips)),
                row.HeightTwips < 0));
        }

        return new Ww8LayoutTable(
            widths,
            layoutRows,
            rows.TakeWhile(r => r.IsHeader).Count(),
            Length.FromTwips(left),
            section);
    }

    /// <summary>
    /// One cell's padding, from the sprms its row-end paragraph carries.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Per row rather than per table, because that is where DOC puts it: the row-end paragraph carries a
    /// default for every cell of the row and any number of overrides naming a range of cells and a set of
    /// sides. So a table whose second cell has a wider left inset is one default plus one override, and a
    /// reader taking only the default puts that cell's text where its neighbours' is.
    /// </para>
    /// <para>
    /// The bit order is WW8's own — top, left, bottom, right — which is neither the order the sides are
    /// usually written in nor OOXML's. Getting it wrong swaps a cell's left inset for its top, which moves
    /// the text down instead of across.
    /// </para>
    /// </remarks>
    private static CellPadding PaddingOf(Ww8ParagraphFormat format, int cell)
    {
        IReadOnlyList<Ww8CellPadding> stated = format.CellPaddings ?? [];
        CellPadding fallback = DefaultCellPadding;

        return new CellPadding(
            Side(Ww8CellPadding.Left) ?? fallback.Left,
            Side(Ww8CellPadding.Right) ?? fallback.Right,
            Side(Ww8CellPadding.Top) ?? fallback.Top,
            Side(Ww8CellPadding.Bottom) ?? fallback.Bottom);

        // An entry naming this cell in particular wins over one covering every cell, whichever order they
        // appear in — which is what LibreOffice does, applying its override values over its defaults. Within
        // each kind the last wins, since a row restating a side means the later value.
        Length? Side(int side)
            => Find(side, specific: true) ?? Find(side, specific: false);

        Length? Find(int side, bool specific)
        {
            for (int i = stated.Count - 1; i >= 0; i--)
            {
                if (specific != stated[i].CellLimit > 0) continue;
                if (!stated[i].Covers(cell)) continue;
                if (stated[i].For(side) is { } twips) return Length.FromTwips(twips);
            }

            return null;
        }
    }

    /// <summary>
    /// One cell's shading, from the two arrays its row states.
    /// </summary>
    /// <remarks>
    /// The RGB array wins wherever it names a colour and the palette array fills in behind it, per cell
    /// rather than per row — which is exactly what LibreOffice's <c>SetTabShades</c> does, testing
    /// <c>pNewSHDs[i] != COL_AUTO</c> before falling back to <c>pSHDs[i]</c>. It matters because the two
    /// disagree: the corpus document's grey is <c>#CCCCCC</c> in the RGB array and <c>#C0C0C0</c> —
    /// Word's nearest palette entry — in the older one.
    /// </remarks>
    private static Colour? ShadingOf(Ww8ParagraphFormat format, int cell)
    {
        if (format.CellShading is { } rgb && cell < rgb.Count && rgb[cell] is { } stated) return stated;

        IReadOnlyList<Colour?>? palette = format.PaletteCellShading;
        return palette is not null && cell < palette.Count ? palette[cell] : null;
    }

    /// <summary>
    /// One cell's borders: what its descriptor said, with every <c>sprmTSetBrc</c> covering it applied.
    /// </summary>
    /// <remarks>
    /// In order and unconditionally, both of which matter. A later sprm restating a side wins over an
    /// earlier one, which is how the RGB form supersedes the palette form; and a sprm carrying a BRC that
    /// states nothing <em>clears</em> the descriptor's border rather than being ignored, which is how a
    /// row removes a border its table's defaults would otherwise put back.
    /// </remarks>
    private static Ww8CellBorders BordersOf(
        Ww8ParagraphFormat format, Ww8CellBorders stated, int cell)
    {
        foreach (Ww8CellBorderChange change in format.CellBorderChanges ?? [])
        {
            if (change.Covers(cell)) stated = stated.With(change.Sides, change.Border);
        }

        return stated;
    }

    /// <summary>
    /// A cell's four edges as the layout engine wants them, with the table's defaults filling the gaps.
    /// </summary>
    /// <remarks>
    /// The fall-back is positional, which is the whole reason a table states six defaults rather than
    /// four: a cell's top is the table's outline in the first row and the inside horizontal everywhere
    /// else. A side left unstated with no defaults at all has no border, which is how a Word table with
    /// no grid comes out.
    /// </remarks>
    private static CellBorders ResolveBorders(
        Ww8CellBorders stated,
        Ww8TableBorders? defaults,
        bool isFirstRow,
        bool isLastRow,
        bool isFirstCell,
        bool isLastCell)
    {
        return new CellBorders(
            Side(stated.Left, Ww8CellBorders.LeftSide),
            Side(stated.Right, Ww8CellBorders.RightSide),
            Side(stated.Top, Ww8CellBorders.TopSide),
            Side(stated.Bottom, Ww8CellBorders.BottomSide));

        TableBorder Side(Ww8Border? border, int side)
        {
            border ??= defaults?.For(side, isFirstRow, isLastRow, isFirstCell, isLastCell);
            return border?.Resolved ?? default;
        }
    }

    /// <summary>
    /// Folds cells the definition marks as merged into the cell they were merged with.
    /// </summary>
    /// <remarks>
    /// The layout counterpart of the content walk's own fold, which works on the same draft type and
    /// carries the other of its two content lists. Kept separate rather than parameterised because the
    /// two differ in exactly one line and sharing it would mean a draft knowing which walk built it.
    /// </remarks>
    private static void ApplyExplicitMerges(List<Ww8CellDraft> cells)
    {
        for (int index = cells.Count - 1; index >= 1; index--)
        {
            if (!cells[index].IsHorizontallyMerged) continue;

            Ww8CellDraft owner = cells[index - 1];
            owner.RightEdge = Math.Max(owner.RightEdge, cells[index].RightEdge);
            owner.LayoutBlocks.AddRange(cells[index].LayoutBlocks);

            // The swallowed cell's right edge is now the owner's, so its right border is too — the same
            // hand-over LibreOffice makes when it folds a merged cell (`ww8par2.cxx`, `ReadDef`).
            owner.Borders = owner.Borders with { Right = cells[index].Borders.Right };
            cells.RemoveAt(index);
        }
    }
}

/// <summary>
/// One block of a DOC's body as layout sees it: a paragraph or a table, never both.
/// </summary>
/// <remarks>
/// A wrapper struct rather than a class hierarchy, because <see cref="Ww8DocumentReader.Ww8LayoutParagraph"/>
/// is a struct and making it a class to gain a base type would allocate one per paragraph of every document
/// read. Exactly one of the two is non-null.
/// </remarks>
public readonly record struct Ww8LayoutBlock
{
    /// <summary>Wraps a paragraph.</summary>
    public Ww8LayoutBlock(Ww8DocumentReader.Ww8LayoutParagraph paragraph) => Paragraph = paragraph;

    /// <summary>Wraps a table.</summary>
    public Ww8LayoutBlock(Ww8LayoutTable table) => Table = table;

    /// <summary>The paragraph, when this block is one.</summary>
    public Ww8DocumentReader.Ww8LayoutParagraph? Paragraph { get; }

    /// <summary>The table, when this block is one.</summary>
    public Ww8LayoutTable? Table { get; }
}

/// <summary>
/// A footnote or endnote as layout sees it: where it is cited, and the blocks of its body.
/// </summary>
/// <remarks>
/// The body comes from a different subdocument than the citation, which is what makes WW8's notes easier than
/// RTF's and harder than ODF's: there is nothing to nest and nothing to buffer, but the two halves are found
/// by two different tables and matched up by ordinal.
/// </remarks>
/// <param name="Offset">Where the citation sits in the citing paragraph's text.</param>
/// <param name="IsEndnote">True for an endnote, which collects at the end of the document.</param>
/// <param name="Blocks">The note's body.</param>
/// <param name="Placement">Where it collects, which for an endnote the DOP's <c>epc</c> decides.</param>
public sealed record Ww8LayoutNote(
    int Offset,
    bool IsEndnote,
    IReadOnlyList<Ww8LayoutBlock> Blocks,
    Layout.NotePlacement Placement = Layout.NotePlacement.PageBottom);

/// <summary>A DOC table as layout sees it: the column grid in twips, and cells holding paragraphs.</summary>
/// <param name="ColumnWidths">The grid's column widths, left to right.</param>
/// <param name="Rows">The rows, top to bottom.</param>
/// <param name="HeaderRowCount">How many rows at the top repeat across a page break.</param>
/// <param name="LeftIndent">How far the table's left edge sits from the text area's.</param>
/// <param name="SectionIndex">Which of the document's sections the table sits in.</param>
public sealed record Ww8LayoutTable(
    IReadOnlyList<Length> ColumnWidths,
    IReadOnlyList<Ww8LayoutRow> Rows,
    int HeaderRowCount,
    Length LeftIndent,
    int SectionIndex = 0);

/// <summary>One row of a DOC table.</summary>
/// <param name="Cells">Its cells, left to right; one covered by a merge above is absent.</param>
/// <param name="IsHeader">True when <c>sprmTTableHeader</c> marked it a heading row.</param>
/// <param name="MinHeight">Its declared height, as a magnitude.</param>
/// <param name="HasExactHeight">
/// True when <c>sprmTDyaRowHeight</c>'s operand was negative, which is how WW8 says the height is exact
/// rather than a floor — the row is that tall and content past it is clipped.
/// </param>
public sealed record Ww8LayoutRow(
    IReadOnlyList<Ww8LayoutCell> Cells,
    bool IsHeader,
    Length MinHeight = default,
    bool HasExactHeight = false);

/// <summary>One cell of a DOC table.</summary>
/// <param name="Column">The grid column it starts at.</param>
/// <param name="ColumnSpan">How many grid columns it covers.</param>
/// <param name="RowSpan">How many rows it covers downwards.</param>
/// <param name="Padding">The gap between its edges and its text.</param>
/// <param name="Blocks">The blocks inside it, in order — paragraphs, and any table nested in it.</param>
/// <param name="Shading">The colour behind its text, or null when it is not shaded.</param>
/// <param name="Borders">Its four edges, with the table's defaults already applied.</param>
public sealed record Ww8LayoutCell(
    int Column,
    int ColumnSpan,
    int RowSpan,
    CellPadding Padding,
    IReadOnlyList<Ww8LayoutBlock> Blocks,
    Colour? Shading = null,
    CellBorders Borders = default);
