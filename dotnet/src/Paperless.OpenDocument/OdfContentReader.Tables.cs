using System.Globalization;
using System.Xml.Linq;
using Paperless.Core.Diagnostics;
using Paperless.Core.Extraction;
using Paperless.OpenDocument.Styles;

namespace Paperless.OpenDocument;

/// <content>
/// Table reading, shared between word-processing tables and spreadsheet sheets.
/// </content>
/// <remarks>
/// The two are the same element with different conventions. A Writer table's cells hold
/// paragraphs; a Calc sheet's cells hold a typed value, a formula, and a cached rendering of
/// the value as paragraphs. Both use <c>table:number-columns-repeated</c> and
/// <c>table:number-rows-repeated</c> to compress runs of identical cells — which a
/// spreadsheet does aggressively, padding every row out to the sheet's full width and the
/// sheet out to its full height. Expanding those naively is the difference between reading a
/// 7 KB file and materialising sixteen billion cells, so trailing empties are dropped rather
/// than expanded.
/// </remarks>
public sealed partial class OdfContentReader
{
    /// <summary>
    /// The widest row Paperless materialises. Matches Calc's own column limit, so no real
    /// spreadsheet is truncated by it.
    /// </summary>
    public const int MaxColumns = 16384;

    /// <summary>
    /// How many identical rows a single <c>table:number-rows-repeated</c> may expand to when
    /// the row has content.
    /// </summary>
    /// <remarks>
    /// Repeated rows carrying content are rare — the attribute exists to compress
    /// <em>empty</em> rows, which are dropped rather than expanded — so a low cap costs
    /// nothing on real files while bounding what a hostile one can allocate.
    /// </remarks>
    public const int MaxRepeatedContentRows = 1024;

    /// <summary>
    /// How many consecutive empty rows are materialised inside a table's used range.
    /// </summary>
    /// <remarks>
    /// Empty rows between content are worth keeping: they are blank lines in the extracted
    /// text and they keep row numbering honest. A run longer than this is padding, not
    /// layout.
    /// </remarks>
    public const int MaxConsecutiveEmptyRows = 4096;

    private bool _reportedTableTruncation;

    /// <summary>
    /// Reads a <c>table:table</c> into a <see cref="ContentTable"/>.
    /// </summary>
    /// <remarks>
    /// One method serves both a Writer table and a Calc sheet, with no flag to choose
    /// between them: a cell's typed value is read when the file records one, and a Writer
    /// table's cells never do. The distinction is in the document, so it does not need to be
    /// in the API.
    /// </remarks>
    /// <param name="table">The <c>table:table</c> element.</param>
    public ContentTable ReadTableElement(XElement table)
    {
        ArgumentNullException.ThrowIfNull(table);

        TableBuilder builder = new(this);
        builder.WalkRows(table, isHeader: false, depth: 0);
        return builder.Build();
    }

    /// <summary>
    /// True when a sheet or table is hidden, per the <c>table:display</c> property of its
    /// table style.
    /// </summary>
    /// <remarks>
    /// A hidden sheet's content is still extracted — a caller indexing a document wants it —
    /// but the section is flagged so a caller reproducing what a reader sees can skip it.
    /// </remarks>
    public bool IsTableHidden(XElement table)
    {
        ArgumentNullException.ThrowIfNull(table);
        return _styles.ResolveProperty(
            Attribute(table, OdfNamespaces.Table, "style-name"),
            OdfStyleFamily.Table,
            OdfPropertyKind.Table,
            OdfNamespaces.Table,
            "display").AsBoolean() == false;
    }

    private void ReadTable(XElement table, ContentNode target)
        => target.Children.Add(ReadTableElement(table));

    private void ReportTableTruncation(string what)
    {
        if (_reportedTableTruncation) return;
        _reportedTableTruncation = true;
        _diagnostics.Add(new Diagnostic(
            DiagnosticSeverity.Warning, "PL2030",
            $"A table declares more {what} than Paperless materialises; the excess has been "
            + "dropped. This is a guard against repeat counts that describe an empty sheet's "
            + "full extent rather than its content."));
    }

    /// <summary>
    /// Accumulates a table's rows, expanding repeats and dropping trailing empties.
    /// </summary>
    private sealed class TableBuilder(OdfContentReader reader)
    {
        private readonly List<ContentTableRow> _rows = [];
        private int _rowIndex;
        private int _headerRowCount;
        private int _pendingEmptyRows;
        private int _columnCount;

        public void WalkRows(XElement container, bool isHeader, int depth)
        {
            // Rows may be wrapped in header-row, row-group and plain row containers, nested
            // arbitrarily; the wrappers carry no content of their own.
            if (depth > 16) return;

            foreach (XElement child in container.Elements())
            {
                if (child.Name.NamespaceName != OdfNamespaces.Table) continue;

                switch (child.Name.LocalName)
                {
                    case "table-row":
                        AddRow(child, isHeader);
                        break;

                    case "table-header-rows":
                        WalkRows(child, isHeader: true, depth + 1);
                        break;

                    case "table-rows" or "table-row-group":
                        WalkRows(child, isHeader, depth + 1);
                        break;

                    // Column declarations describe widths and visibility, not content.
                    default:
                        break;
                }
            }
        }

        public ContentTable Build()
        {
            ContentTable table = new()
            {
                ColumnCount = _columnCount,
                HeaderRowCount = _headerRowCount,
            };
            foreach (ContentTableRow row in _rows) table.Children.Add(row);
            return table;
        }

        private void AddRow(XElement rowElement, bool isHeader)
        {
            int repeat = Repeat(rowElement, "number-rows-repeated");

            // Read once to find out whether the row has anything in it. An empty row is not
            // materialised until something after it proves it was inside the used range.
            List<ContentTableCell> firstRow = ReadCells(rowElement, _rowIndex);
            if (firstRow.Count == 0)
            {
                _pendingEmptyRows += repeat;
                _rowIndex += repeat;
                return;
            }

            FlushPendingEmptyRows();

            int materialise = repeat;
            if (materialise > MaxRepeatedContentRows)
            {
                materialise = MaxRepeatedContentRows;
                reader.ReportTableTruncation("repeated rows with content");
            }

            for (int copy = 0; copy < materialise; copy++)
            {
                // The first copy is already read; later copies are re-read so each row owns
                // its own nodes rather than sharing them with a sibling.
                List<ContentTableCell> cells = copy == 0 ? firstRow : ReadCells(rowElement, _rowIndex);
                ContentTableRow row = new() { Index = _rowIndex };
                foreach (ContentTableCell cell in cells) row.Children.Add(cell);
                _rows.Add(row);

                if (isHeader) _headerRowCount = _rows.Count;
                _columnCount = Math.Max(_columnCount, WidthOf(cells));
                _rowIndex++;
            }

            // A repeat beyond the cap still advances the row numbering, so cells after it
            // keep their true addresses.
            _rowIndex += repeat - materialise;
        }

        private void FlushPendingEmptyRows()
        {
            if (_pendingEmptyRows == 0) return;

            int count = _pendingEmptyRows;
            if (count > MaxConsecutiveEmptyRows)
            {
                count = MaxConsecutiveEmptyRows;
                reader.ReportTableTruncation("consecutive empty rows");
            }

            int firstIndex = _rowIndex - _pendingEmptyRows;
            for (int offset = 0; offset < count; offset++)
                _rows.Add(new ContentTableRow { Index = firstIndex + offset });

            _pendingEmptyRows = 0;
        }

        private static int WidthOf(List<ContentTableCell> cells)
        {
            int width = 0;
            foreach (ContentTableCell cell in cells)
                width = Math.Max(width, cell.Column + cell.ColumnSpan);
            return width;
        }

        /// <summary>
        /// Reads one row's cells, expanding column repeats and dropping trailing empties.
        /// </summary>
        private List<ContentTableCell> ReadCells(XElement rowElement, int rowIndex)
        {
            List<ContentTableCell> cells = [];
            int column = 0;
            int lastWithContent = -1;

            foreach (XElement child in rowElement.Elements())
            {
                if (child.Name.NamespaceName != OdfNamespaces.Table) continue;

                bool covered = child.Name.LocalName == "covered-table-cell";
                if (!covered && child.Name.LocalName != "table-cell") continue;

                int repeat = Repeat(child, "number-columns-repeated");
                if (column + repeat > MaxColumns)
                {
                    repeat = Math.Max(0, MaxColumns - column);
                    if (repeat == 0)
                    {
                        reader.ReportTableTruncation("columns");
                        break;
                    }
                }

                if (covered)
                {
                    // A covered cell is the hidden remainder of a neighbour's span. It holds
                    // no content of its own, but it still occupies its columns.
                    column += repeat;
                    continue;
                }

                for (int copy = 0; copy < repeat; copy++)
                {
                    ContentTableCell cell = reader.ReadCell(child, rowIndex, column);
                    if (!IsEmpty(cell)) lastWithContent = cells.Count;
                    cells.Add(cell);
                    column++;
                }
            }

            // Trailing empty cells are padding: a spreadsheet row declares its full width
            // whether or not the cells hold anything.
            if (lastWithContent + 1 < cells.Count) cells.RemoveRange(lastWithContent + 1, cells.Count - lastWithContent - 1);
            return cells;
        }

        private static bool IsEmpty(ContentTableCell cell)
            => cell.Value is null && cell.Formula is null && cell.Children.Count == 0;

        private static int Repeat(XElement element, string attributeName)
        {
            int? declared = OdfValue.ParseInt(Attribute(element, OdfNamespaces.Table, attributeName));
            // A zero or negative repeat is meaningless; treat it as the single cell the
            // element itself represents rather than dropping the cell.
            return declared is null or < 1 ? 1 : declared.Value;
        }
    }

    /// <summary>
    /// Reads one cell: its typed value and formula for a spreadsheet, and its text for both.
    /// </summary>
    private ContentTableCell ReadCell(XElement cellElement, int row, int column)
    {
        int columnSpan = Math.Clamp(
            OdfValue.ParseInt(Attribute(cellElement, OdfNamespaces.Table, "number-columns-spanned")) ?? 1,
            1, MaxColumns);
        int rowSpan = Math.Max(
            1, OdfValue.ParseInt(Attribute(cellElement, OdfNamespaces.Table, "number-rows-spanned")) ?? 1);

        ContentTableCell cell = new()
        {
            Row = row,
            Column = column,
            ColumnSpan = columnSpan,
            RowSpan = rowSpan,
            Value = ReadCellValue(cellElement),
            Formula = NormaliseFormula(Attribute(cellElement, OdfNamespaces.Table, "formula")),
        };

        // The cell's paragraphs are its *displayed* text — for a spreadsheet, the value
        // already run through its number format. Keeping them rather than re-deriving the
        // text from the value is what makes extraction agree with what the authoring
        // application showed.
        ReadingState state = SuspendReading();
        ReadBlocks(cellElement, cell);
        ResumeReading(state);

        return cell;
    }

    /// <summary>
    /// Reads a cell's underlying value, or null when it has none.
    /// </summary>
    /// <remarks>
    /// A word-processing table cell has no <c>office:value-type</c> at all, so this returns
    /// null there and the cell is text only — which is exactly what
    /// <see cref="ContentTableCell.Value"/> documents.
    /// </remarks>
    private static object? ReadCellValue(XElement cellElement)
    {
        string? valueType = Attribute(cellElement, OdfNamespaces.Office, "value-type");
        if (valueType is null) return null;

        // LibreOffice records an error result in its own namespace, because ODF has no way
        // to say "this cell is an error": office:value-type stays whatever the formula would
        // have produced and the error text lives in the cell's paragraphs.
        if (Attribute(cellElement, OdfNamespaces.CalcExt, "value-type") == "error")
            return ParseCellError(cellElement.Value);

        switch (valueType)
        {
            case "float" or "percentage" or "currency":
                return OdfValue.ParseDouble(Attribute(cellElement, OdfNamespaces.Office, "value"));

            case "boolean":
                return OdfValue.ParseBoolean(Attribute(cellElement, OdfNamespaces.Office, "boolean-value"));

            case "date":
                DateTimeOffset? date =
                    OdfValue.ParseDateTime(Attribute(cellElement, OdfNamespaces.Office, "date-value"));
                return date?.DateTime;

            case "time":
                // ODF writes a time-of-day as a duration since midnight, which is also how
                // it represents a genuine elapsed time — the two are indistinguishable in
                // the file, and the number format is what tells them apart.
                return OdfValue.ParseDuration(Attribute(cellElement, OdfNamespaces.Office, "time-value"));

            case "string":
                // office:string-value is optional; when absent the paragraphs are the value.
                return Attribute(cellElement, OdfNamespaces.Office, "string-value")
                       ?? CellText(cellElement);

            default:
                return null;
        }
    }

    private static string CellText(XElement cellElement)
    {
        List<string> paragraphs = [];
        foreach (XElement paragraph in cellElement.Elements(XName.Get("p", OdfNamespaces.Text)))
            paragraphs.Add(paragraph.Value);
        return string.Join('\n', paragraphs);
    }

    /// <summary>
    /// Maps an error cell's displayed text onto <see cref="CellError"/>.
    /// </summary>
    /// <remarks>
    /// The displayed text is the only machine-readable statement of <em>which</em> error
    /// occurred — LibreOffice records the kind nowhere else — so it is matched against the
    /// error strings Calc produces. An unrecognised one becomes
    /// <see cref="CellError.Unknown"/> rather than being dropped: the cell is still in error.
    /// </remarks>
    private static CellError ParseCellError(string? displayed)
    {
        string text = (displayed ?? string.Empty).Trim();
        if (text.StartsWith("#DIV/0", StringComparison.OrdinalIgnoreCase)) return CellError.DivideByZero;
        if (text.StartsWith("#VALUE", StringComparison.OrdinalIgnoreCase)) return CellError.Value;
        if (text.StartsWith("#REF", StringComparison.OrdinalIgnoreCase)) return CellError.Reference;
        if (text.StartsWith("#NAME", StringComparison.OrdinalIgnoreCase)) return CellError.Name;
        if (text.StartsWith("#NUM", StringComparison.OrdinalIgnoreCase)) return CellError.Number;
        if (text.StartsWith("#N/A", StringComparison.OrdinalIgnoreCase)) return CellError.NotAvailable;
        if (text.StartsWith("#NULL", StringComparison.OrdinalIgnoreCase)) return CellError.Null;

        // Calc's own numbered errors. 522 is the circular-reference one; the rest do not map
        // onto a distinct CellError.
        if (text.StartsWith("Err:522", StringComparison.OrdinalIgnoreCase)) return CellError.Circular;
        return CellError.Unknown;
    }

    /// <summary>
    /// Strips the namespace prefix and leading <c>=</c> from a <c>table:formula</c>.
    /// </summary>
    /// <remarks>
    /// ODF prefixes a formula with the namespace of the language it is written in —
    /// <c>of:</c> for OpenFormula, <c>oooc:</c> for the older OpenOffice.org syntax — and the
    /// prefix is not part of the expression. What remains is left in the file's own syntax,
    /// bracket references and all, because translating it would be a lie about what the
    /// document says.
    /// </remarks>
    private static string? NormaliseFormula(string? formula)
    {
        if (string.IsNullOrWhiteSpace(formula)) return null;
        string text = formula.Trim();

        int colon = text.IndexOf(':', StringComparison.Ordinal);
        if (colon > 0 && text.AsSpan(0, colon).IndexOfAny("=[ .") < 0)
            text = text[(colon + 1)..];

        if (text.StartsWith('=')) text = text[1..];
        return text.Length == 0 ? null : text;
    }

    /// <summary>
    /// A sheet's name, or a generated one when the file omits it.
    /// </summary>
    /// <remarks>
    /// The generated name matches what Calc would have called the sheet, so a document
    /// missing the attribute still produces stable, recognisable section names.
    /// </remarks>
    public static string SheetName(XElement table, int index)
        => Attribute(table, OdfNamespaces.Table, "name")
           ?? string.Create(CultureInfo.InvariantCulture, $"Sheet{index + 1}");
}
