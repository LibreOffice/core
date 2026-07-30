using System.Xml.Linq;
using Paperless.Core.Extraction;

namespace Paperless.WordProcessing.Ooxml;

/// <content>Table reading for WordprocessingML.</content>
/// <remarks>
/// WordprocessingML states spans differently from ODF in both directions, and both need
/// translating rather than copying:
/// <list type="bullet">
///   <item><description>
///     A horizontal span is <c>w:gridSpan</c> on the cell, a count like ODF's — straightforward.
///   </description></item>
///   <item><description>
///     A vertical span is not a count anywhere. The top cell carries
///     <c>w:vMerge w:val="restart"</c> and every cell below it carries a bare <c>w:vMerge</c>,
///     so the span's height is only knowable by looking at the rows that follow. That is why
///     rows are drafted first and materialised afterwards.
///   </description></item>
/// </list>
/// </remarks>
public sealed partial class DocxContentReader
{
    /// <summary>The widest row Paperless materialises, matching Word's own column limit.</summary>
    public const int MaxTableColumns = 63;

    private void ReadTable(XElement table, ContentNode target)
    {
        if (!EnterDepth()) return;
        target.Children.Add(ReadTableElement(table));
        _depth--;
    }

    /// <summary>Reads a <c>w:tbl</c> into a <see cref="ContentTable"/>.</summary>
    public ContentTable ReadTableElement(XElement table)
    {
        ArgumentNullException.ThrowIfNull(table);

        List<RowDraft> rows = [];
        int headerRowCount = 0;

        foreach (XElement rowElement in table.Elements())
        {
            if (!Word.Is(rowElement, "tr"))
            {
                // w:tblPr, w:tblGrid, and the tracked-change and content-control wrappers that
                // may sit between a table and its rows.
                if (Word.Is(rowElement, "sdt") && Word.Child(rowElement, "sdtContent") is { } content)
                {
                    foreach (XElement nested in Word.Children(content, "tr"))
                        rows.Add(ReadRow(nested, rows.Count));
                }
                else if (Word.Is(rowElement, "ins") || Word.Is(rowElement, "moveTo"))
                {
                    foreach (XElement nested in Word.Children(rowElement, "tr"))
                        rows.Add(ReadRow(nested, rows.Count));
                }
                continue;
            }

            RowDraft row = ReadRow(rowElement, rows.Count);
            rows.Add(row);

            // Header rows are the leading run of rows marked as such; a marked row further down
            // is a repeat marker Word ignores too.
            if (row.IsHeader && headerRowCount == rows.Count - 1) headerRowCount = rows.Count;
        }

        ResolveVerticalMerges(rows);

        ContentTable result = new()
        {
            ColumnCount = rows.Count == 0 ? 0 : rows.Max(r => r.Width),
            HeaderRowCount = headerRowCount,
        };

        foreach (RowDraft row in rows)
        {
            ContentTableRow contentRow = new() { Index = row.Index };
            foreach (CellDraft cell in row.Cells)
            {
                // A cell continuing a vertical merge is not a cell of its own: its content
                // belongs to the one that started the merge.
                if (cell.ContinuesMergeAbove) continue;

                ContentTableCell contentCell = new()
                {
                    Row = row.Index,
                    Column = cell.Column,
                    ColumnSpan = cell.ColumnSpan,
                    RowSpan = cell.RowSpan,
                };
                foreach (ContentNode node in cell.Content) contentCell.Children.Add(node);
                contentRow.Children.Add(contentCell);
            }
            result.Children.Add(contentRow);
        }

        return result;
    }

    private RowDraft ReadRow(XElement rowElement, int index)
    {
        XElement? rowProperties = Word.Child(rowElement, "trPr");
        RowDraft row = new()
        {
            Index = index,
            IsHeader = Word.Child(rowProperties, "tblHeader") is { } header && Word.IsOn(header),
        };

        // A row deleted by a tracked change is not part of the document.
        if (Word.Child(rowProperties, "del") is not null) return row;

        int column = 0;
        foreach (XElement cellElement in rowElement.Elements())
        {
            if (!Word.Is(cellElement, "tc")) continue;
            if (column >= MaxTableColumns) break;

            XElement? cellProperties = Word.Child(cellElement, "tcPr");
            int span = Math.Clamp(
                int.TryParse(Word.Value(cellProperties, "gridSpan"), out int parsed) ? parsed : 1,
                1, MaxTableColumns - column);

            XElement? verticalMerge = Word.Child(cellProperties, "vMerge");
            bool continues = verticalMerge is not null
                             && Word.Attribute(verticalMerge, "val") is not "restart";

            CellDraft cell = new()
            {
                Column = column,
                ColumnSpan = span,
                ContinuesMergeAbove = continues,
            };

            if (!continues)
            {
                ContentSection scratch = new() { Kind = SectionKind.Body };
                ReadBlocks(cellElement, scratch);
                cell.Content.AddRange(scratch.Children);
            }

            row.Cells.Add(cell);
            column += span;
        }

        return row;
    }

    /// <summary>
    /// Turns the top-and-continuation encoding of a vertical merge into a row span on the cell
    /// that starts it.
    /// </summary>
    /// <remarks>
    /// Matched by column position rather than by cell index, because a row inside a merge may
    /// have a different number of cells from the row that started it.
    /// </remarks>
    private static void ResolveVerticalMerges(List<RowDraft> rows)
    {
        for (int rowIndex = 0; rowIndex < rows.Count; rowIndex++)
        {
            foreach (CellDraft cell in rows[rowIndex].Cells)
            {
                if (!cell.ContinuesMergeAbove) continue;

                // Walk up to the nearest cell in the same column that is not itself a
                // continuation: that is the one the merge belongs to.
                for (int above = rowIndex - 1; above >= 0; above--)
                {
                    CellDraft? owner = rows[above].Cells
                        .FirstOrDefault(c => c.Column == cell.Column);
                    if (owner is null) break;
                    if (owner.ContinuesMergeAbove) continue;

                    owner.RowSpan++;
                    break;
                }
            }
        }
    }

    private sealed class RowDraft
    {
        public int Index { get; init; }
        public bool IsHeader { get; init; }
        public List<CellDraft> Cells { get; } = [];

        public int Width
        {
            get
            {
                int width = 0;
                foreach (CellDraft cell in Cells) width = Math.Max(width, cell.Column + cell.ColumnSpan);
                return width;
            }
        }
    }

    private sealed class CellDraft
    {
        public int Column { get; init; }
        public int ColumnSpan { get; init; } = 1;
        public bool ContinuesMergeAbove { get; init; }
        public int RowSpan { get; set; } = 1;
        public List<ContentNode> Content { get; } = [];
    }
}
