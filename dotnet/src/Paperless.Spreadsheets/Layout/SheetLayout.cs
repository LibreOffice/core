using Paperless.Core.Extraction;

namespace Paperless.Spreadsheets.Layout;

/// <summary>
/// Everything laying one sheet out needs: its print setup, its geometry, and its cells.
/// </summary>
/// <remarks>
/// <para>
/// The three readers converge on this. What they read differs completely — ODF states the page
/// in a <c>style:page-layout</c> and the widths on <c>table:table-column</c>, SpreadsheetML in
/// <c>pageSetup</c> and <c>cols</c>, BIFF in <c>SETUP</c> and <c>COLINFO</c> — but pagination
/// asks the same questions of all three, so the questions are asked once here.
/// </para>
/// <para>
/// The cells come from the content tree rather than from a second parse. That is not a
/// shortcut: the tree already holds every cell's position, its typed value and the text the
/// number format produced, which is exactly what a page needs to draw, and re-reading the sheet
/// to get them again would be a second chance to disagree with what was extracted.
/// </para>
/// <para>
/// A class rather than a record, although it is otherwise shaped like one: it memoises its used
/// range and a cell index, and a record's generated equality would compare those caches — so two
/// layouts describing the same sheet would stop being equal the moment one of them was drawn.
/// </para>
/// </remarks>
public sealed class SheetLayout
{
    /// <summary>The sheet's name, as shown on its tab.</summary>
    public required string Name { get; init; }

    /// <summary>Its position in the workbook, zero-based.</summary>
    public int Index { get; init; }

    /// <summary>True when the sheet is hidden, and therefore not printed.</summary>
    public bool IsHidden { get; init; }

    /// <summary>The sheet's print setup, which is its page geometry.</summary>
    public SheetPrintSetup Setup { get; init; } = SheetPrintSetup.Default;

    /// <summary>
    /// Its column widths and row heights, exactly as the file states them.
    /// </summary>
    /// <remarks>
    /// A row that asks for an optimal height is <em>not</em> recomputed here, although Calc
    /// recomputes one on load and although the runs carry
    /// <see cref="SheetSizeRun.IsOptimalSize"/> so that a reader can tell which rows those are.
    /// The reason is measured and is recorded in the module's TODO: Calc's own measurement of a
    /// cell for that purpose is coarser than the one it draws with, so reproducing the formula
    /// with an accurate measurement disagrees with the height the file already holds.
    /// </remarks>
    public SheetGrid Grid { get; init; } = SheetGrid.Standard;

    /// <summary>The sheet's cells, or null when it holds none.</summary>
    public ContentTable? Cells { get; init; }

    /// <summary>
    /// What is painted behind and around the cells: their fills and their borders.
    /// </summary>
    /// <remarks>
    /// Beside the cells rather than on them, because the two are stored separately in all three
    /// formats and for the same reason: a fill applied to a whole column belongs to sixteen
    /// thousand cells that do not exist. It also keeps extraction from paying for any of it —
    /// the content tree is unchanged, and a caller that only wants text never looks here.
    /// </remarks>
    public SheetFormatting Formatting { get; init; } = SheetFormatting.Empty;

    /// <summary>The document's own file name, for the <c>&amp;F</c> header field.</summary>
    /// <remarks>
    /// Carried on the sheet because the field is resolved while a page is drawn and nothing
    /// else in the layout knows where the document came from. Empty when it was read from a
    /// stream with no name, which is what a header holding <c>&amp;F</c> then prints.
    /// </remarks>
    public string FileName { get; init; } = string.Empty;

    /// <summary>
    /// The formats its cells are drawn in, kept apart from the cells themselves.
    /// </summary>
    /// <remarks>
    /// Separate because a spreadsheet stores it separately: formatting is a run-length structure
    /// keyed by row rather than a property of a cell, and a sheet with one uniformly-formatted
    /// million-cell region stays cheap only while that holds. Extraction never reads it — a font
    /// changes nothing about what a cell says — so a reader that has not been taught to fill it in
    /// yields <see cref="SheetCellFormats.Empty"/> and every cell draws in the default face.
    /// </remarks>
    public SheetCellFormats Formats { get; init; } = SheetCellFormats.Empty;

    /// <summary>
    /// The cells whose text is not all in one format, if any.
    /// </summary>
    /// <remarks>
    /// Beside <see cref="Formats"/> rather than inside it because the two have opposite shapes: a
    /// cell format is shared by thousands of cells and is pooled, while a rich cell's portions
    /// belong to that cell alone and almost no cell has any. See <see cref="SheetRichText"/>.
    /// </remarks>
    public SheetRichText RichText { get; init; } = SheetRichText.Empty;

    /// <summary>
    /// The pictures and charts anchored on the sheet, back to front.
    /// </summary>
    /// <remarks>
    /// Beside the cells because a drawing is not in one: it is fastened to the grid by a cell and
    /// an offset and floats over whatever is under it, so it belongs to the sheet rather than to
    /// any cell. See <see cref="SheetDrawings"/>.
    /// </remarks>
    public SheetDrawings Drawings { get; init; } = SheetDrawings.Empty;

    /// <summary>
    /// The block of cells the sheet holds, from the sheet's origin.
    /// </summary>
    /// <remarks>
    /// From A1 rather than from the first cell with something in it, which is what
    /// <c>ScPrintFunc::AdjustPrintArea(true)</c> does: it sets the start to column zero and row
    /// zero and searches only for the end (<c>printfun.cxx:700</c>). A sheet whose data begins
    /// at C3 therefore still prints columns A and B, blank — and printing from C3 instead would
    /// shift every column on every page.
    /// </remarks>
    public SheetRange UsedRange
    {
        get
        {
            if (_usedRange is { } cached) return cached;

            int lastColumn = -1;
            int lastRow = -1;

            foreach (ContentTableRow row in (Cells?.Children ?? []).OfType<ContentTableRow>())
            {
                foreach (ContentTableCell cell in row.Children.OfType<ContentTableCell>())
                {
                    // A blank cell carrying only a style is not content. Calc's own used-area
                    // search does count formatted-but-empty cells; the content tree does not
                    // record formatting, so this is the narrower of the two answers and is
                    // recorded in the module's TODO as a known difference.
                    if (cell.Value is null && cell.GetText().Length == 0) continue;

                    int columnEnd = cell.Column + Math.Max(1, cell.ColumnSpan) - 1;
                    int rowEnd = cell.Row + Math.Max(1, cell.RowSpan) - 1;
                    if (columnEnd > lastColumn) lastColumn = columnEnd;
                    if (rowEnd > lastRow) lastRow = rowEnd;
                }
            }

            SheetRange range = new(0, 0, lastColumn, lastRow);
            _usedRange = range;
            return range;
        }

        init => _usedRange = value;
    }

    private SheetRange? _usedRange;

    /// <summary>
    /// The block the sheet actually prints: its used range, widened for the drawings floating
    /// over it and then for overflowing text.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Distinct from <see cref="UsedRange"/> because Calc's own print-area search widens the range
    /// it found twice before paginating it, and the two widenings are different rules applied in a
    /// fixed order. <c>ScDocument::GetPrintArea</c> takes the maximum of the cells' own extent and
    /// the drawing layer's bounding box (<c>sc/source/core/data/documen2.cxx:644-664</c>), so a
    /// chart anchored right of the last cell adds the columns it covers — see
    /// <see cref="SheetDrawingArea"/>. Only then does <c>ScTable::ExtendPrintArea</c> widen it
    /// again for a string too wide for its column, which spills into the empty cells beside it and
    /// is printed whole (<c>sc/source/core/data/table1.cxx:2127</c>). On
    /// <c>xls-features.xls</c> the second of those is the difference between three pages and the
    /// four LibreOffice prints.
    /// </para>
    /// <para>
    /// The text widening applies only when the sheet declares no print range of its own. A declared
    /// range is honoured as written, and Calc agrees: it widens only the axis the search chose,
    /// which is neither of them once the range came from the file. The drawing widening is applied
    /// either way, because a declared whole-column range is cut back to this range and Calc cuts it
    /// back to the drawing-widened one (<c>AdjustPrintArea(false)</c>,
    /// <c>printfun.cxx:735-741</c>).
    /// </para>
    /// </remarks>
    public SheetRange PrintedRange
    {
        get
        {
            if (_printedRange is { } cached) return cached;

            SheetRange used = SheetDrawingArea.Extend(UsedRange, Drawings, Grid);
            SheetRange printed = used.IsValid && Setup.PrintAreas.Count == 0
                ? used with { LastColumn = SheetTextOverflow.ExtendedLastColumn(this, used) }
                : used;

            _printedRange = printed;
            return printed;
        }
    }

    private SheetRange? _printedRange;

    /// <summary>The cell at a position, or null when the sheet has nothing there.</summary>
    /// <remarks>
    /// Indexed on first use rather than walked, because a page asks this once per cell in its
    /// block and the rows are a list: walking would make drawing a page quadratic in the sheet's
    /// height.
    /// </remarks>
    /// <param name="row">The zero-based row.</param>
    /// <param name="column">The zero-based column.</param>
    public ContentTableCell? CellAt(int row, int column)
    {
        _index ??= BuildIndex();
        return _index.GetValueOrDefault((row, column));
    }

    /// <summary>
    /// True when a position lies inside a merged block, as its origin or covered by it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Asked of the <em>position</em> rather than of the cell, because the cells a merge covers
    /// are not in the tree at all: a reader keeps the block's origin and drops the rest, so
    /// <see cref="CellAt"/> answers null for them and a test of "is there a cell here" cannot tell
    /// a covered position from an empty one. Calc keeps the distinction in an attribute —
    /// <c>ATTR_MERGE</c> on the origin and <c>ATTR_MERGE_FLAG</c>'s overlapped bit on the rest —
    /// and <c>ScOutputData::IsAvailable</c> (<c>sc/source/ui/view/output2.cxx:1178-1191</c>) reads
    /// both to decide whether a neighbour's long string may run through.
    /// </para>
    /// <para>
    /// A list of ranges walked linearly rather than a set of covered positions, because a merge is
    /// <em>stated</em> as a range and expanding one is unbounded: a sheet may merge a whole
    /// column, which is a million positions to record and one range to test. Sheets carry few
    /// merges, and the case of none — nearly every sheet — costs a count check.
    /// </para>
    /// </remarks>
    /// <param name="row">The zero-based row.</param>
    /// <param name="column">The zero-based column.</param>
    public bool IsMerged(int row, int column)
    {
        _index ??= BuildIndex();
        if (_merges is not { Count: > 0 } merges) return false;

        foreach (SheetRange merge in merges)
        {
            if (row >= merge.FirstRow && row <= merge.LastRow
                && column >= merge.FirstColumn && column <= merge.LastColumn)
            {
                return true;
            }
        }

        return false;
    }

    private Dictionary<(int Row, int Column), ContentTableCell>? _index;
    private List<SheetRange>? _merges;

    private Dictionary<(int, int), ContentTableCell> BuildIndex()
    {
        Dictionary<(int, int), ContentTableCell> index = [];
        List<SheetRange> merges = [];

        foreach (ContentTableRow row in (Cells?.Children ?? []).OfType<ContentTableRow>())
        {
            foreach (ContentTableCell cell in row.Children.OfType<ContentTableCell>())
            {
                index[(cell.Row, cell.Column)] = cell;

                int columns = Math.Max(1, cell.ColumnSpan);
                int rows = Math.Max(1, cell.RowSpan);
                if (columns > 1 || rows > 1)
                {
                    merges.Add(new SheetRange(
                        cell.Column, cell.Row, cell.Column + columns - 1, cell.Row + rows - 1));
                }
            }
        }

        _merges = merges;
        return index;
    }
}
