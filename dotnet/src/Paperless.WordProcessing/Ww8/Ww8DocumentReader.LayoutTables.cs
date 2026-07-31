using System.Buffers.Binary;
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
        private readonly List<Ww8LayoutParagraph> _cell = [];
        private readonly List<Ww8CellDraft> _rowCells = [];
        private readonly List<Ww8RowDraft> _rows = [];

        /// <summary>The section the table's rows were in, taken from the paragraphs that made them.</summary>
        private int _section;

        /// <summary>Takes one paragraph, with the properties of the mark that ended it.</summary>
        /// <param name="paragraph">The paragraph.</param>
        /// <param name="format">Its mark's properties.</param>
        /// <param name="endsCell">
        /// True when the mark was U+0007, which ends a cell of the outermost table — or its row, when the
        /// properties say so. A paragraph mark inside a cell ends neither.
        /// </param>
        public void Add(Ww8LayoutParagraph paragraph, Ww8ParagraphFormat format, bool endsCell)
        {
            if (format.Level == 0)
            {
                // The first paragraph outside a table is what closes it: consecutive rows form a table
                // only by being adjacent, and WW8 marks no end.
                Close();
                _blocks.Add(new Ww8LayoutBlock(paragraph));
                return;
            }

            // A nested table's cells are not laid out, so neither are its paragraphs — putting them in the
            // enclosing cell instead would stack a whole inner table into one cell's flow.
            if (format.Level > 1) return;

            if (endsCell && format.IsTableRowEnd)
            {
                // The row-end paragraph is the terminator rather than content, so it is dropped — but it
                // still closes whatever cell was open, and it carries the row's geometry.
                FinishRow(format);
                return;
            }

            _section = paragraph.SectionIndex;

            if (paragraph.Text.Length > 0 || endsCell) _cell.Add(paragraph);
            if (endsCell) FinishCell();
        }

        /// <summary>Everything collected, with a trailing unterminated table closed.</summary>
        public List<Ww8LayoutBlock> Finished()
        {
            Close();
            return _blocks;
        }

        private void FinishCell()
        {
            Ww8CellDraft cell = new();
            cell.LayoutParagraphs.AddRange(_cell);
            _cell.Clear();
            _rowCells.Add(cell);
        }

        private void FinishRow(Ww8ParagraphFormat format)
        {
            if (_cell.Count > 0) FinishCell();
            if (_rowCells.Count == 0) return;

            Ww8TableDefinition? definition = format.TableDefinition;

            for (int i = 0; i < _rowCells.Count; i++)
            {
                _rowCells[i].RightEdge = definition?.RightEdgeOf(i) ?? 0;
                _rowCells[i].Padding = PaddingOf(format, i);

                Ww8CellDefinition cell = definition?.CellAt(i) ?? default;
                _rowCells[i].IsHorizontallyMerged = cell.IsMerged;
                _rowCells[i].ContinuesMergeAbove = cell.IsVerticallyMerged && !cell.StartsVerticalMerge;
            }

            ApplyExplicitMerges(_rowCells);

            Ww8RowDraft row = new()
            {
                Index = _rows.Count,
                LeftEdge = definition?.LeftEdge ?? 0,
                IsHeader = format.IsTableHeaderRow,
            };
            row.Cells.AddRange(_rowCells);
            _rows.Add(row);

            _rowCells.Clear();
        }

        /// <summary>Turns the rows collected so far into a table, and appends it.</summary>
        private void Close()
        {
            // Cells collected but never closed by a row mark: a truncated table, still worth keeping.
            if (_cell.Count > 0) FinishCell();
            if (_rowCells.Count > 0) FinishRow(default);

            if (_rows.Count == 0)
            {
                Reset();
                return;
            }

            AssignColumns(_rows);
            ResolveVerticalMerges(_rows);

            if (LayoutTableOf(_rows, _section) is { } table) _blocks.Add(new Ww8LayoutBlock(table));

            Reset();
        }

        private void Reset()
        {
            _cell.Clear();
            _rowCells.Clear();
            _rows.Clear();
            _section = 0;
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
                    [.. cell.LayoutParagraphs]));
            }

            layoutRows.Add(new Ww8LayoutRow(cells, row.IsHeader));
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
            owner.LayoutParagraphs.AddRange(cells[index].LayoutParagraphs);
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
public sealed record Ww8LayoutRow(IReadOnlyList<Ww8LayoutCell> Cells, bool IsHeader);

/// <summary>One cell of a DOC table.</summary>
/// <param name="Column">The grid column it starts at.</param>
/// <param name="ColumnSpan">How many grid columns it covers.</param>
/// <param name="RowSpan">How many rows it covers downwards.</param>
/// <param name="Padding">The gap between its edges and its text.</param>
/// <param name="Paragraphs">The paragraphs inside it, in order.</param>
public sealed record Ww8LayoutCell(
    int Column,
    int ColumnSpan,
    int RowSpan,
    CellPadding Padding,
    IReadOnlyList<Ww8DocumentReader.Ww8LayoutParagraph> Paragraphs);
