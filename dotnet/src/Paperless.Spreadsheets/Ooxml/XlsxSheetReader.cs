using System.Globalization;
using System.Xml.Linq;
using Paperless.Core.Diagnostics;
using Paperless.Core.Extraction;
using Paperless.Spreadsheets.Numbers;

namespace Paperless.Spreadsheets.Ooxml;

/// <summary>
/// Turns one <c>worksheet</c> part into a <see cref="ContentTable"/> of the same shape the ODS
/// path produces: every cell carrying both its typed value and the text its authoring
/// application displayed.
/// </summary>
/// <remarks>
/// <para>
/// SpreadsheetML compresses a sheet by <em>omission</em> rather than by repeat counts: a row
/// with nothing in columns B and C simply has no <c>&lt;c&gt;</c> for them, and a sheet with
/// nothing in rows 2 to 400 has no <c>&lt;row&gt;</c> for them either. The gaps still have to
/// be materialised for the extracted text to keep its columns aligned — but only inside the
/// used range, and only up to a cap, because a row may legitimately address column 16384 and a
/// sheet row 1048576. Filling either out in full is the difference between a table and sixteen
/// billion cells.
/// </para>
/// <para>
/// Formatting stays out. A cell's style index is resolved to a number format and used to render
/// the display text, and nothing else about the style is carried into the content tree.
/// </para>
/// </remarks>
internal sealed class XlsxSheetReader(XlsxFile file, List<Diagnostic> diagnostics)
{
    /// <summary>
    /// The widest row materialised. Matches SpreadsheetML's own column limit, so no valid
    /// sheet is truncated by it.
    /// </summary>
    public const int MaxColumns = 16384;

    /// <summary>
    /// How many consecutive empty rows are materialised inside a sheet's used range.
    /// </summary>
    /// <remarks>
    /// Empty rows between content are worth keeping — they are blank lines in the extracted
    /// text and they keep row numbering honest — but a sheet whose next content is a hundred
    /// thousand rows down is describing a gap, not a layout.
    /// </remarks>
    public const int MaxConsecutiveEmptyRows = 4096;

    private readonly XlsxFile _file = file;
    private readonly List<Diagnostic> _diagnostics = diagnostics;
    private readonly Dictionary<int, SharedFormula> _sharedFormulas = [];
    private bool _reportedTruncation;

    /// <summary>Reads a sheet's cells.</summary>
    public ContentTable ReadSheet(XElement worksheet)
    {
        ArgumentNullException.ThrowIfNull(worksheet);

        _sharedFormulas.Clear();
        MergeMap merges = MergeMap.Read(worksheet);

        List<ContentTableRow> rows = [];
        int columnCount = 0;
        int expectedRow = 0;
        int pendingEmptyRows = 0;

        foreach (XElement rowElement in Xlsx.Children(Xlsx.Child(worksheet, "sheetData"), "row"))
        {
            // The r attribute is optional; without it a row simply follows the previous one,
            // which is how LibreOffice's importer treats it too
            // (sc/source/filter/oox/sheetdatacontext.cxx:291).
            int rowIndex = (Xlsx.Integer(rowElement, "r") - 1) ?? expectedRow;
            if (rowIndex < 0) rowIndex = expectedRow;
            if (rowIndex > expectedRow) pendingEmptyRows += rowIndex - expectedRow;
            expectedRow = rowIndex + 1;

            List<ContentTableCell> cells = ReadCells(rowElement, rowIndex, merges);
            if (cells.Count == 0)
            {
                pendingEmptyRows++;
                continue;
            }

            FlushEmptyRows(rows, rowIndex, ref pendingEmptyRows);

            ContentTableRow row = new() { Index = rowIndex };
            foreach (ContentTableCell cell in cells) row.Children.Add(cell);
            rows.Add(row);

            foreach (ContentTableCell cell in cells)
                columnCount = Math.Max(columnCount, cell.Column + cell.ColumnSpan);
        }

        ContentTable table = new() { ColumnCount = columnCount };
        foreach (ContentTableRow row in rows) table.Children.Add(row);
        return table;
    }

    /// <summary>
    /// Reads the sheet's cell comments, each as its own section.
    /// </summary>
    /// <remarks>
    /// Their own sections rather than text spliced into the commented cell, because a comment
    /// is not part of what the cell says — merging them would corrupt a row's text — and
    /// because the author is worth keeping. This is what the ODS path does with
    /// <c>office:annotation</c>.
    /// </remarks>
    public IEnumerable<ContentSection> ReadComments(XlsxSheetEntry sheet)
    {
        XElement? root = _file.LoadComments(sheet);
        if (root is null) yield break;

        List<string> authors =
            [.. Xlsx.Children(Xlsx.Child(root, "authors"), "author").Select(a => a.Value)];

        int index = 0;
        foreach (XElement comment in Xlsx.Children(Xlsx.Child(root, "commentList"), "comment"))
        {
            string text = XlsxSharedStrings.ReadRichString(Xlsx.Child(comment, "text"));
            if (text.Length == 0) continue;

            int authorId = Xlsx.Integer(comment, "authorId") ?? -1;
            ContentSection section = new()
            {
                Kind = SectionKind.Comment,
                Index = index++,
                Name = authorId >= 0 && authorId < authors.Count ? authors[authorId] : null,
            };
            AddText(section, text);
            yield return section;
        }
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

    /// <summary>
    /// Reads one row's cells, filling addressing gaps and dropping trailing empties.
    /// </summary>
    /// <remarks>
    /// Interior gaps become empty cells so that a row's tab-separated text keeps its columns:
    /// a row holding only A5 and D5 must read "Grand total\t\t\t£85.50", not
    /// "Grand total\t£85.50". Trailing gaps are padding and are dropped.
    /// </remarks>
    private List<ContentTableCell> ReadCells(XElement rowElement, int rowIndex, MergeMap merges)
    {
        List<ContentTableCell> cells = [];
        int expectedColumn = 0;
        int lastWithContent = -1;

        foreach (XElement cellElement in Xlsx.Children(rowElement, "c"))
        {
            int column = expectedColumn;
            if (Xlsx.Attribute(cellElement, "r") is { } reference
                && Xlsx.TryParseCellReference(reference, out int parsedColumn, out _))
            {
                column = parsedColumn;
            }
            if (column < 0) column = expectedColumn;
            expectedColumn = column + 1;

            if (column >= MaxColumns)
            {
                ReportTruncation("columns");
                break;
            }

            // A cell covered by a merge holds nothing of its own; the anchor carries the span.
            if (merges.IsCovered(rowIndex, column)) continue;

            for (int gap = cells.Count == 0 ? 0 : cells[^1].Column + 1; gap < column; gap++)
            {
                // A covered cell is not invented: the anchor's span already accounts for it.
                if (merges.IsCovered(rowIndex, gap)) continue;
                cells.Add(new ContentTableCell { Row = rowIndex, Column = gap });
            }

            ContentTableCell cell = ReadCell(cellElement, rowIndex, column, merges);
            if (!IsEmpty(cell)) lastWithContent = cells.Count;
            cells.Add(cell);
        }

        if (lastWithContent + 1 < cells.Count)
            cells.RemoveRange(lastWithContent + 1, cells.Count - lastWithContent - 1);
        return cells;
    }

    private static bool IsEmpty(ContentTableCell cell)
        => cell.Value is null && cell.Formula is null && cell.Children.Count == 0;

    private ContentTableCell ReadCell(XElement element, int row, int column, MergeMap merges)
    {
        NumberFormatCode format = _file.Styles.FormatFor(Xlsx.Integer(element, "s"));
        (object? value, string display) = ReadValue(element, format);
        (int columnSpan, int rowSpan) = merges.SpanAt(row, column);

        ContentTableCell cell = new()
        {
            Row = row,
            Column = column,
            ColumnSpan = columnSpan,
            RowSpan = rowSpan,
            Value = value,
            Formula = ReadFormula(element, row, column),
        };

        AddText(cell, display);
        return cell;
    }

    /// <summary>
    /// Reads a cell's typed value and the text it displays.
    /// </summary>
    /// <remarks>
    /// The <c>t</c> attribute names six genuinely different things and they are not
    /// interchangeable: <c>s</c> is an index into the shared string table, <c>inlineStr</c> a
    /// string written in place, <c>str</c> a formula's cached string result, <c>b</c> a
    /// boolean stored as 1 or 0, <c>e</c> an error stored as its own display text, and the
    /// absent default a number. Treating any of them as the default reads a string as the
    /// number zero.
    /// </remarks>
    private (object? Value, string Display) ReadValue(XElement element, NumberFormatCode format)
    {
        string type = Xlsx.Attribute(element, "t") ?? "n";
        XElement? valueElement = Xlsx.Child(element, "v");

        switch (type)
        {
            case "s":
            {
                if (valueElement is null) return (null, string.Empty);
                if (!int.TryParse(valueElement.Value, NumberStyles.Integer, CultureInfo.InvariantCulture,
                                  out int index))
                    return (null, string.Empty);

                string? shared = _file.SharedStrings[index];
                if (shared is null)
                {
                    _diagnostics.Add(new Diagnostic(
                        DiagnosticSeverity.Warning, "PL2143",
                        $"A cell refers to shared string {index}, which the string table does "
                        + "not contain; the cell has been read as empty."));
                    return (null, string.Empty);
                }
                return (shared, NumberFormatter.Format(format, shared));
            }

            case "inlineStr":
            {
                string inline = XlsxSharedStrings.ReadRichString(Xlsx.Child(element, "is"));
                return inline.Length == 0
                    ? (null, string.Empty)
                    : (inline, NumberFormatter.Format(format, inline));
            }

            case "str":
            {
                string cached = valueElement?.Value ?? string.Empty;
                return cached.Length == 0
                    ? (null, string.Empty)
                    : (cached, NumberFormatter.Format(format, cached));
            }

            case "b":
            {
                if (valueElement is null) return (null, string.Empty);
                // Some producers write "true" rather than 1; LibreOffice tolerates both
                // (sheetdatacontext.cxx:210) and so must this.
                string text = valueElement.Value.Trim();
                bool flag = text is not ("0" or "" or "false" or "FALSE" or "False");

                // A boolean has no number of its own to format, so General means the words
                // rather than the 1 or 0 the file stores. A format such as
                // "TRUE";"TRUE";"FALSE" — which is how LibreOffice exports a boolean cell —
                // does have an opinion, and it wins.
                string display = format.IsGeneral
                    ? flag ? "TRUE" : "FALSE"
                    : NumberFormatter.Format(format, flag ? 1.0 : 0.0, _file.DateSystem);
                return (flag, display);
            }

            case "e":
            {
                string text = valueElement?.Value ?? string.Empty;
                return (ParseCellError(text), text);
            }

            case "d":
            {
                // ECMA-376 2nd edition added an ISO 8601 date cell. Rare, but it is the one
                // cell type whose value is not a number at all.
                string text = valueElement?.Value ?? string.Empty;
                if (!DateTime.TryParse(text, CultureInfo.InvariantCulture,
                                       DateTimeStyles.RoundtripKind, out DateTime parsed))
                    return (null, text);

                return (parsed, format.IsGeneral
                    ? text
                    : NumberFormatter.Format(format, ToSerial(parsed), _file.DateSystem));
            }

            default:
            {
                if (valueElement is null) return (null, string.Empty);
                if (Xlsx.Double(valueElement.Value) is not { } number)
                {
                    // A non-numeric value on a numeric cell is a producer bug; keeping the text
                    // loses nothing and reading it as zero would invent data.
                    string text = valueElement.Value;
                    return text.Length == 0 ? (null, string.Empty) : (text, text);
                }

                return (TypedNumber(number, format),
                        NumberFormatter.Format(format, number, _file.DateSystem));
            }
        }
    }

    /// <summary>
    /// Surfaces a number as the type its format says it is.
    /// </summary>
    /// <remarks>
    /// A spreadsheet has no date type — a date is a serial number and only the format says so —
    /// so the format is what decides here. A time-only format becomes a
    /// <see cref="TimeSpan"/> rather than a time-of-day <see cref="DateTime"/>, matching the
    /// ODS path: the file genuinely does not distinguish 14:30 from an elapsed fourteen and a
    /// half hours, and inventing a date would invent data.
    /// </remarks>
    private object TypedNumber(double number, NumberFormatCode format)
    {
        if (!format.IsDateTime) return number;

        if (format.IsTimeOnly)
        {
            bool elapsed = format.Sections[0].HasElapsed;
            return SpreadsheetDate.ToTimeOfDay(number, keepWholeDays: elapsed);
        }

        return SpreadsheetDate.FromSerial(number, _file.DateSystem) is { } moment
            ? moment
            : number;
    }

    private double ToSerial(DateTime moment)
    {
        DateTime epoch = _file.DateSystem == SpreadsheetDateSystem.Date1904
            ? new DateTime(1904, 1, 1)
            : new DateTime(1899, 12, 30);
        return (moment - epoch).TotalDays;
    }

    /// <summary>
    /// Reads a cell's formula, reconstructing it when the cell is a shared-formula follower.
    /// </summary>
    /// <remarks>
    /// The cached result is preferred over recomputation — that is the settled policy — so
    /// nothing here evaluates anything. An array formula's master keeps its expression and the
    /// cells it spills into keep none, which is what the file says.
    /// </remarks>
    private string? ReadFormula(XElement element, int row, int column)
    {
        XElement? formula = Xlsx.Child(element, "f");
        if (formula is null) return null;

        string text = formula.Value;
        string kind = Xlsx.Attribute(formula, "t") ?? "normal";

        if (kind == "shared" && Xlsx.Integer(formula, "si") is { } id)
        {
            if (text.Length > 0)
            {
                _sharedFormulas[id] = new SharedFormula(text, row, column);
                return text;
            }
            if (_sharedFormulas.TryGetValue(id, out SharedFormula master))
                return XlsxFormula.Shift(master.Text, row - master.Row, column - master.Column);

            // The master has not been seen. It is meant to come first, and a file where it does
            // not is one whose formula cannot be reconstructed at all.
            _diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Warning, "PL2144",
                $"A cell uses shared formula {id} before the formula itself has been read, so "
                + "the cell keeps its cached result but no expression."));
            return null;
        }

        return text.Length == 0 ? null : text;
    }

    /// <summary>
    /// Maps an error cell's stored text onto <see cref="CellError"/>.
    /// </summary>
    /// <remarks>
    /// SpreadsheetML stores an error as the literal string a reader shows, so the display text
    /// is the only statement of which error occurred. An unrecognised one becomes
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
        return CellError.Unknown;
    }

    /// <summary>
    /// Adds display text as paragraphs, one per line.
    /// </summary>
    /// <remarks>
    /// A cell's text may hold newlines — a hard line break inside a cell is stored as one — and
    /// the ODS path writes each line as its own paragraph, so this does too.
    /// </remarks>
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

    private readonly record struct SharedFormula(string Text, int Row, int Column);

    /// <summary>
    /// The sheet's merged ranges, as a lookup from cell to span.
    /// </summary>
    /// <remarks>
    /// A merge is declared once, in <c>mergeCells</c>, rather than on the cells — so a reader
    /// that only walks <c>sheetData</c> sees a merged block as one populated cell followed by
    /// several empty ones and reports no span at all.
    /// </remarks>
    private sealed class MergeMap
    {
        private readonly Dictionary<(int Row, int Column), (int Columns, int Rows)> _anchors = [];
        private readonly HashSet<(int Row, int Column)> _covered = [];

        public static MergeMap Read(XElement worksheet)
        {
            MergeMap map = new();
            foreach (XElement merge in Xlsx.Children(Xlsx.Child(worksheet, "mergeCells"), "mergeCell"))
            {
                if (!Xlsx.TryParseRange(Xlsx.Attribute(merge, "ref"),
                                        out int firstColumn, out int firstRow,
                                        out int lastColumn, out int lastRow))
                    continue;

                if (lastColumn < firstColumn || lastRow < firstRow) continue;

                int columns = lastColumn - firstColumn + 1;
                int rows = lastRow - firstRow + 1;
                if (columns == 1 && rows == 1) continue;

                // A merge covering a whole sheet would otherwise enumerate every cell in it.
                if ((long)columns * rows > 1_000_000) continue;

                map._anchors[(firstRow, firstColumn)] = (columns, rows);
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
}
