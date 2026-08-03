using Paperless.Core.Diagnostics;
using Paperless.Core.Extraction;
using Paperless.Core.Numbers;
using Paperless.Spreadsheets.Layout;
using Paperless.Spreadsheets.MsBinary;

namespace Paperless.Spreadsheets.Xlsb;

/// <summary>
/// Turns one BIFF12 worksheet part into a <see cref="ContentTable"/> of the shape the other
/// three readers produce.
/// </summary>
/// <remarks>
/// <para>
/// The cell records fall into three families and the difference between them is only how the
/// column is stated. A <c>CELL_*</c> record names its column outright; a <c>MULTCELL_*</c>
/// carries no column at all and continues from the previous cell — the format's run-length
/// compression, and the reason a cursor has to be kept across records rather than each record
/// being self-describing; and a <c>FORMULA_*</c> names its column and carries a cached result
/// followed by a token array (<c>SheetDataContext::readCellHeader</c>,
/// <c>sc/source/filter/oox/sheetdatacontext.cxx:452-459</c>).
/// </para>
/// <para>
/// <strong>The cached result is what is read, and the formula is not decoded.</strong> That is
/// the same decision the BIFF8 reader records: a BIFF12 formula is an RPN token array whose
/// operators come in reference, value and array forms under three different opcodes, and
/// decoding it would buy a <c>Formula</c> string and change no displayed value. So an XLSB cell
/// carries its value and its text and a null formula, exactly as an XLS cell does — and unlike
/// an XLSX one, which has the expression written out for free.
/// </para>
/// <para>
/// The gap-filling and truncation rules are <see cref="Ooxml.XlsxSheetReader"/>'s, because the
/// compression is the same: BIFF12 omits a cell rather than repeating it, so interior gaps have
/// to be materialised for a row's tab-separated text to keep its columns and trailing ones must
/// not be.
/// </para>
/// </remarks>
internal sealed class XlsbSheetReader(XlsbFile file, List<Diagnostic> diagnostics)
{
    /// <summary>The widest row materialised — SpreadsheetML's own column limit.</summary>
    public const int MaxColumns = 16384;

    /// <summary>How many consecutive empty rows are materialised inside a used range.</summary>
    public const int MaxConsecutiveEmptyRows = 4096;

    private readonly XlsbFile _file = file;
    private readonly List<Diagnostic> _diagnostics = diagnostics;
    private bool _reportedTruncation;

    /// <summary>
    /// The merged ranges the last <see cref="ReadSheet"/> found, for
    /// <see cref="SheetLayout.StatedMerges"/>.
    /// </summary>
    /// <remarks>
    /// Handed back rather than re-read, because finding them is a full pass over the part's
    /// records and the caller wants exactly the ones the cells were built from. Sheets are read
    /// one at a time and the layout is assembled immediately after, so there is one live answer.
    /// </remarks>
    public IReadOnlyList<SheetRange> SheetMerges { get; private set; } = [];

    /// <summary>
    /// The hyperlinked ranges the last <see cref="ReadSheet"/> found, for
    /// <see cref="SheetLayout.HyperlinkRanges"/>.
    /// </summary>
    public IReadOnlyList<SheetRange> SheetHyperlinks { get; private set; } = [];

    /// <summary>Reads a worksheet part's cells.</summary>
    /// <param name="part">The part's bytes, or null when it did not load.</param>
    public ContentTable ReadSheet(byte[]? part)
    {
        SheetMerges = [];
        SheetHyperlinks = [];
        if (part is null) return new ContentTable();

        XlsbMerges merges = XlsbMerges.Read(part);
        SheetMerges = merges.Ranges;
        SheetHyperlinks = ReadHyperlinks(part);

        List<ContentTableRow> rows = [];
        List<ContentTableCell> cells = [];
        int rowIndex = -1;
        int column = -1;
        int columnCount = 0;
        int pendingEmptyRows = 0;
        int expectedRow = 0;
        bool inSheetData = false;

        foreach (Biff12Record record in Biff12Stream.Records(part))
        {
            switch (record.Id)
            {
                case Biff12.SheetData:
                    inSheetData = true;
                    continue;

                case Biff12.SheetData + 1:
                    inSheetData = false;
                    continue;
            }

            if (!inSheetData) continue;

            if (record.Id == Biff12.Row)
            {
                Flush(rows, cells, rowIndex, ref columnCount, ref pendingEmptyRows);

                Biff12Cursor cursor = new(record.Data.Span);
                rowIndex = cursor.ReadInt32();
                if (rowIndex < 0) rowIndex = expectedRow;
                if (rowIndex > expectedRow) pendingEmptyRows += rowIndex - expectedRow;
                expectedRow = rowIndex + 1;

                // −1 rather than LibreOffice's 0 (`importRow`, sheetdatacontext.cxx:424). The two
                // differ only for a row whose *first* record is a MULTCELL_*, which puts the cell
                // in column B there and column A here — and a run has to begin with a record that
                // states its column, so no writer produces one. Recorded rather than reproduced:
                // the deviation is from a case that cannot occur, not from the rule.
                column = -1;
                continue;
            }

            if (rowIndex < 0) continue;
            if (ReadCell(record, rowIndex, ref column, merges) is not { } cell) continue;

            if (cell.Column >= MaxColumns)
            {
                ReportTruncation("columns");
                continue;
            }

            for (int gap = cells.Count == 0 ? 0 : cells[^1].Column + 1; gap < cell.Column; gap++)
            {
                if (merges.IsCovered(rowIndex, gap)) continue;
                cells.Add(new ContentTableCell { Row = rowIndex, Column = gap });
            }
            cells.Add(cell);
        }

        Flush(rows, cells, rowIndex, ref columnCount, ref pendingEmptyRows);

        ContentTable table = new() { ColumnCount = columnCount };
        foreach (ContentTableRow row in rows) table.Children.Add(row);
        return table;
    }

    /// <summary>Ends the row being accumulated, dropping it when it holds nothing.</summary>
    private void Flush(
        List<ContentTableRow> rows,
        List<ContentTableCell> cells,
        int rowIndex,
        ref int columnCount,
        ref int pendingEmptyRows)
    {
        if (rowIndex < 0 || cells.Count == 0)
        {
            if (rowIndex >= 0) pendingEmptyRows++;
            cells.Clear();
            return;
        }

        // Trailing empties are padding a writer left behind; interior ones are the row's shape.
        int last = -1;
        for (int at = 0; at < cells.Count; at++)
            if (!IsEmpty(cells[at])) last = at;

        if (last < 0)
        {
            pendingEmptyRows++;
            cells.Clear();
            return;
        }

        FlushEmptyRows(rows, rowIndex, ref pendingEmptyRows);

        ContentTableRow row = new() { Index = rowIndex };
        for (int at = 0; at <= last; at++)
        {
            row.Children.Add(cells[at]);
            columnCount = Math.Max(columnCount, cells[at].Column + cells[at].ColumnSpan);
        }
        rows.Add(row);
        cells.Clear();
    }

    private void FlushEmptyRows(List<ContentTableRow> rows, int beforeRow, ref int pending)
    {
        if (pending == 0) return;

        int count = pending;
        if (count > MaxConsecutiveEmptyRows)
        {
            count = MaxConsecutiveEmptyRows;
            ReportTruncation("consecutive empty rows");
        }

        int first = beforeRow - count;
        for (int offset = 0; offset < count; offset++)
        {
            if (first + offset < 0) continue;
            rows.Add(new ContentTableRow { Index = first + offset });
        }
        pending = 0;
    }

    private static bool IsEmpty(ContentTableCell cell)
        => cell.Value is null && cell.Formula is null && cell.Children.Count == 0;

    /// <summary>
    /// One cell record, or null when the record is not a cell.
    /// </summary>
    /// <remarks>
    /// The column cursor is advanced here rather than by the caller because a
    /// <c>MULTCELL_*</c>'s column <em>is</em> the advance: <c>readCellHeader</c> reads an int for
    /// the other two families and pre-increments for this one, so the two cannot be separated.
    /// </remarks>
    private ContentTableCell? ReadCell(
        Biff12Record record, int row, ref int column, XlsbMerges merges)
    {
        CellShape shape = ShapeOf(record.Id);
        if (shape == CellShape.NotACell) return null;

        Biff12Cursor cursor = new(record.Data.Span);
        column = shape == CellShape.Multi ? column + 1 : cursor.ReadInt32();
        if (column < 0) return null;

        // The low 24 bits are the cell format index; the top byte carries the phonetic flag.
        int styleIndex = (int)(cursor.ReadUInt32() & 0x00FFFFFF);
        if (merges.IsCovered(row, column)) return null;

        NumberFormatCode format = _file.Styles.FormatFor(styleIndex);
        (object? value, string display) = ReadValue(record.Id, ref cursor, format);
        (int columnSpan, int rowSpan) = merges.SpanAt(row, column);

        ContentTableCell cell = new()
        {
            Row = row,
            Column = column,
            ColumnSpan = columnSpan,
            RowSpan = rowSpan,
            Value = value,
        };

        AddText(cell, display);
        return cell;
    }

    private (object? Value, string Display) ReadValue(
        int id, ref Biff12Cursor cursor, NumberFormatCode format)
    {
        switch (id)
        {
            case Biff12.CellBlank or Biff12.MultCellBlank:
                return (null, string.Empty);

            case Biff12.CellRk or Biff12.MultCellRk:
            {
                double number = BiffRecordReader.RkValue(cursor.ReadInt32());
                return (TypedNumber(number, format),
                        NumberFormatter.Format(format, number, _file.DateSystem));
            }

            case Biff12.CellDouble or Biff12.MultCellDouble
                 or Biff12.FormulaDouble:
            {
                double number = cursor.ReadDouble();
                return (TypedNumber(number, format),
                        NumberFormatter.Format(format, number, _file.DateSystem));
            }

            case Biff12.CellBool or Biff12.MultCellBool or Biff12.FormulaBool:
            {
                bool flag = cursor.ReadByte() != 0;

                // A boolean has no number of its own to format, so General means the words
                // rather than the 1 or 0 the file stores.
                string display = format.IsGeneral
                    ? flag ? "TRUE" : "FALSE"
                    : NumberFormatter.Format(format, flag ? 1.0 : 0.0, _file.DateSystem);
                return (flag, display);
            }

            case Biff12.CellError or Biff12.MultCellError or Biff12.FormulaError:
            {
                CellError error = BiffErrors.ToCellError(cursor.ReadByte());
                return (error, BiffErrors.Text(error));
            }

            case Biff12.CellSi or Biff12.MultCellSi:
            {
                int index = cursor.ReadInt32();
                if (index < 0 || index >= _file.SharedStrings.Count)
                {
                    _diagnostics.Add(new Diagnostic(
                        DiagnosticSeverity.Warning, "PL2143",
                        $"A cell refers to shared string {index}, which the string table does "
                        + "not contain; the cell has been read as empty."));
                    return (null, string.Empty);
                }

                string shared = _file.SharedStrings[index];
                return (shared, NumberFormatter.Format(format, shared));
            }

            case Biff12.CellString or Biff12.MultCellString or Biff12.FormulaString:
            {
                // Written in place rather than shared, and *not* preceded by a flag byte —
                // unlike CELL_RSTRING, which is the same payload with the rich-text tails.
                string text = XlsbSharedStrings.ReadRichString(ref cursor, rich: false);
                return text.Length == 0
                    ? (null, string.Empty)
                    : (text, NumberFormatter.Format(format, text));
            }

            case Biff12.CellRString or Biff12.MultCellRString:
            {
                string text = XlsbSharedStrings.ReadRichString(ref cursor, rich: true);
                return text.Length == 0
                    ? (null, string.Empty)
                    : (text, NumberFormatter.Format(format, text));
            }

            default:
                return (null, string.Empty);
        }
    }

    /// <summary>
    /// Surfaces a number as the type its format says it is — the same rule the other readers use.
    /// </summary>
    private object TypedNumber(double number, NumberFormatCode format)
    {
        if (!format.IsDateTime) return number;

        if (format.IsTimeOnly)
        {
            bool elapsed = format.Sections[0].HasElapsed;
            return SpreadsheetDate.ToTimeOfDay(number, keepWholeDays: elapsed);
        }

        return SpreadsheetDate.FromSerial(number, _file.DateSystem) is { } moment ? moment : number;
    }

    private static void AddText(ContentNode target, string text)
    {
        if (text.Length == 0) return;

        foreach (string line in text.Split('\n'))
        {
            ContentParagraph paragraph = new();
            paragraph.Children.Add(new ContentRun { Text = line.TrimEnd('\r') });
            target.Children.Add(paragraph);
        }
    }

    private void ReportTruncation(string what)
    {
        if (_reportedTruncation) return;
        _reportedTruncation = true;
        _diagnostics.Add(new Diagnostic(
            DiagnosticSeverity.Warning, "PL2145",
            $"A sheet addresses more {what} than Paperless materialises; the excess has been "
            + "dropped. This is a guard against a sheet that addresses its full extent rather "
            + "than its content."));
    }

    private enum CellShape
    {
        NotACell,
        Single,
        Multi,
    }

    private static CellShape ShapeOf(int id) => id switch
    {
        Biff12.CellBlank or Biff12.CellRk or Biff12.CellError or Biff12.CellBool
            or Biff12.CellDouble or Biff12.CellString or Biff12.CellSi or Biff12.CellRString
            or Biff12.FormulaString or Biff12.FormulaDouble or Biff12.FormulaBool
            or Biff12.FormulaError => CellShape.Single,

        Biff12.MultCellBlank or Biff12.MultCellRk or Biff12.MultCellError or Biff12.MultCellBool
            or Biff12.MultCellDouble or Biff12.MultCellString or Biff12.MultCellSi
            or Biff12.MultCellRString => CellShape.Multi,

        _ => CellShape.NotACell,
    };

    /// <summary>
    /// The blocks a <c>BrtHLink</c> covers, in a pass of its own.
    /// </summary>
    /// <remarks>
    /// The record is a <c>BinRange</c> followed by the relationship id, the location, the tooltip
    /// and the display text (<c>WorksheetFragment::importHyperlink</c>,
    /// <c>sc/source/filter/oox/worksheetfragment.cxx:846-857</c>). Only the first two decide
    /// whether a URL results, which is what makes the cell a field — see
    /// <see cref="SheetLayout.HyperlinkRanges"/>.
    /// </remarks>
    private static List<SheetRange> ReadHyperlinks(byte[] part)
    {
        List<SheetRange> links = [];
        foreach (Biff12Record record in Biff12Stream.Records(part))
        {
            if (record.Id != Biff12.HLink) continue;

            Biff12Cursor cursor = new(record.Data.Span);
            (int firstRow, int lastRow, int firstColumn, int lastColumn) = cursor.ReadRange();
            if (lastColumn < firstColumn || lastRow < firstRow) continue;
            if (firstRow < 0 || firstColumn < 0) continue;

            string relationship = cursor.ReadString();
            string location = cursor.ReadString();
            if (relationship.Length == 0 && location.Length == 0) continue;

            links.Add(new SheetRange(firstColumn, firstRow, lastColumn, lastRow));
        }

        return links;
    }
}

/// <summary>
/// The sheet's merged ranges, as a lookup from cell to span.
/// </summary>
/// <remarks>
/// Declared once in <c>MERGECELLS</c> rather than on the cells, so a reader that only walks
/// <c>SHEETDATA</c> sees a merged block as one populated cell followed by several empty ones and
/// reports no span at all. Read in a pass of its own because <c>MERGECELLS</c> comes <em>after</em>
/// the cell data in every file Excel writes.
/// </remarks>
internal sealed class XlsbMerges
{
    private readonly Dictionary<(int Row, int Column), (int Columns, int Rows)> _anchors = [];
    private readonly HashSet<(int Row, int Column)> _covered = [];
    private readonly List<SheetRange> _ranges = [];

    /// <summary>Every merged block, as the sheet states it.</summary>
    public IReadOnlyList<SheetRange> Ranges => _ranges;

    public static XlsbMerges Read(byte[] part)
    {
        XlsbMerges map = new();
        foreach (Biff12Record record in Biff12Stream.Records(part))
        {
            if (record.Id != Biff12.MergeCell) continue;

            Biff12Cursor cursor = new(record.Data.Span);
            (int firstRow, int lastRow, int firstColumn, int lastColumn) = cursor.ReadRange();
            if (lastColumn < firstColumn || lastRow < firstRow) continue;
            if (firstRow < 0 || firstColumn < 0) continue;

            int columns = lastColumn - firstColumn + 1;
            int rows = lastRow - firstRow + 1;
            if (columns == 1 && rows == 1) continue;

            // A merge covering a whole sheet would otherwise enumerate every cell in it.
            if ((long)columns * rows > 1_000_000) continue;

            map._anchors[(firstRow, firstColumn)] = (columns, rows);
            map._ranges.Add(new SheetRange(firstColumn, firstRow, lastColumn, lastRow));
            for (int row = firstRow; row <= lastRow; row++)
            {
                for (int column = firstColumn; column <= lastColumn; column++)
                {
                    if (row == firstRow && column == firstColumn) continue;
                    map._covered.Add((row, column));
                }
            }
        }
        return map;
    }

    public bool IsCovered(int row, int column) => _covered.Contains((row, column));

    public (int Columns, int Rows) SpanAt(int row, int column)
        => _anchors.TryGetValue((row, column), out (int Columns, int Rows) span) ? span : (1, 1);
}
