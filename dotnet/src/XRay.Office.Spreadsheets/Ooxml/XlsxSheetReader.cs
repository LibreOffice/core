using System.Globalization;
using System.Xml.Linq;
using XRay.Office.Core.Diagnostics;
using XRay.Office.Core.Extraction;
using XRay.Office.Core.Numbers;
using XRay.Office.Spreadsheets.Layout;

namespace XRay.Office.Spreadsheets.Ooxml;

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
    /// <summary>The 2009 conditional-formatting extension namespace.</summary>
    private const string X14Namespace
        = "http://schemas.microsoft.com/office/spreadsheetml/2009/9/main";

    /// <summary>The namespace an extension states its ranges and formulas in.</summary>
    private const string XmNamespace = "http://schemas.microsoft.com/office/excel/2006/main";

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
    private XlsxPivotLabels _pivotLabels = XlsxPivotLabels.None;
    private bool _reportedTruncation;

    /// <summary>Reads a sheet's cells.</summary>
    /// <param name="worksheet">The worksheet part's root.</param>
    /// <param name="sheet">
    /// Which sheet it is, when the caller knows. Needed only to reach the parts that hang off the
    /// worksheet by relationship rather than living inside it — today the pivot tables, whose
    /// output cells Calc lays out itself. Null reads the cells exactly as stated.
    /// </param>
    public ContentTable ReadSheet(XElement worksheet, XlsxSheetEntry? sheet = null)
    {
        ArgumentNullException.ThrowIfNull(worksheet);

        _sharedFormulas.Clear();
        MergeMap merges = MergeMap.Read(worksheet);
        _pivotLabels = XlsxPivotLabels.Read(_file, sheet, worksheet);

        List<ContentTableRow> rows = [];
        ColumnStyles columnStyles = ColumnStyles.Read(worksheet);
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

            // A row's own format reaches the cells that state none of their own, and only when
            // the row says customFormat: measured on both binaries, a `<row s="3">` without it
            // leaves its cells on the sheet default. See ColumnStyles.
            int? rowStyle = Xlsx.Flag(rowElement, "customFormat")
                ? Xlsx.Integer(rowElement, "s")
                : null;

            List<ContentTableCell> cells =
                ReadCells(rowElement, rowIndex, merges, rowStyle, columnStyles);
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
    /// The merged blocks a worksheet part states.
    /// </summary>
    /// <remarks>
    /// Read separately from the cells because layout needs the merges the cells cannot carry: an
    /// empty merged block's anchor is an empty cell, and an empty cell past the last filled one in
    /// its row is padding the content tree drops. It costs one walk of <c>mergeCells</c>, which is
    /// a single small element, against re-deriving something the file already states outright.
    /// </remarks>
    /// <param name="worksheet">The worksheet part's root element, or null when it did not load.</param>
    public static IReadOnlyList<SheetRange> ReadMerges(XElement? worksheet)
        => worksheet is null ? [] : MergeMap.Read(worksheet).Ranges;

    /// <summary>
    /// The blocks a worksheet part's <c>hyperlinks</c> element covers.
    /// </summary>
    /// <remarks>
    /// A link with neither a relationship nor a location resolves to an empty URL and is skipped
    /// before it reaches a cell — <c>getHyperlinkUrl</c> builds the string from exactly those two
    /// and <c>finalizeHyperlinkRanges</c> tests it for emptiness
    /// (<c>sc/source/filter/oox/worksheethelper.cxx:1011-1032</c>). What the URL <em>is</em> does
    /// not matter here; that a cell holds one does, because it turns the cell into a field.
    /// </remarks>
    /// <param name="worksheet">The worksheet part's root element, or null when it did not load.</param>
    public static IReadOnlyList<SheetRange> ReadHyperlinks(XElement? worksheet)
    {
        if (worksheet is null) return [];

        List<SheetRange> links = [];
        foreach (XElement link in Xlsx.Children(Xlsx.Child(worksheet, "hyperlinks"), "hyperlink"))
        {
            bool stated =
                Xlsx.Attribute(link, "location") is { Length: > 0 }
                || link.Attributes().Any(
                    a => a.Name.LocalName == "id" && a.Value.Length > 0);
            if (!stated) continue;

            if (!Xlsx.TryParseRange(Xlsx.Attribute(link, "ref"),
                                    out int firstColumn, out int firstRow,
                                    out int lastColumn, out int lastRow))
                continue;

            if (lastColumn < firstColumn || lastRow < firstRow) continue;

            links.Add(new SheetRange(firstColumn, firstRow, lastColumn, lastRow));
        }

        return links;
    }

    /// <summary>
    /// The blocks a worksheet part's <c>conditionalFormatting</c> elements cover.
    /// </summary>
    /// <remarks>
    /// Every one of them, whatever the rule inside says, because what the row height turns on is
    /// the presence of <c>ATTR_CONDITIONAL</c> on the cell's pattern rather than any condition
    /// being true — see <see cref="SheetLayout.ConditionalRanges"/>.
    /// <para>
    /// <strong>And the <c>x14</c> extension list is one of them.</strong> This used to skip it, on
    /// the stated grounds that "the plain element is what every workbook the corpus holds writes"
    /// — which is false: <strong>six corpus documents state an <c>x14:cfRule</c> over a range no
    /// main-namespace <c>conditionalFormatting</c> covers</strong>, carrying it as an
    /// <c>xm:sqref</c> beside the rule, and <c>ExtLstLocalContext</c> builds a whole
    /// <c>ScConditionalFormat</c> for such an entry
    /// (<c>sc/source/filter/oox/extlstcontext.cxx</c>:165-194). Its cells carry
    /// <c>ATTR_CONDITIONAL</c> like any other and are therefore measured through
    /// <c>GetNeededSize</c> rather than by <c>lcl_GetAttribHeight</c>'s arithmetic. The difference
    /// is 22 twips a row at 11 pt, and an authored fixture shows it directly: 26.2.4.2's baseline
    /// pitch on <c>tests/corpus/features/sheet-cf-icon-set-formula.xlsx</c>, whose only rule is an
    /// <c>x14</c>-only one, is <strong>14.88 pt where this reader gave 13.78</strong>.
    /// </para>
    /// <para>
    /// An extension a main-namespace rule <em>does</em> claim needs no entry of its own, since the
    /// rule it extends already covers the range; adding one anyway costs nothing, so the two are
    /// not told apart here.
    /// </para>
    /// </remarks>
    /// <param name="worksheet">The worksheet part's root element, or null when it did not load.</param>
    public static IReadOnlyList<SheetRange> ReadConditionalRanges(XElement? worksheet)
    {
        if (worksheet is null) return [];

        List<SheetRange> ranges = [];

        foreach (XElement format in Xlsx.Children(worksheet, "conditionalFormatting"))
        {
            Add(ranges, Xlsx.Attribute(format, "sqref"));
        }

        foreach (XElement format in worksheet.Descendants(
                     XName.Get("conditionalFormatting", X14Namespace)))
        {
            Add(ranges, format.Element(XName.Get("sqref", XmNamespace))?.Value);
        }

        return ranges;
    }

    /// <summary>Adds every range one <c>sqref</c> names, however the element spelled it.</summary>
    private static void Add(List<SheetRange> ranges, string? sqref)
    {
        if (string.IsNullOrEmpty(sqref)) return;

        foreach (string part in sqref.Split(
                     ' ', StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries))
        {
            if (!Xlsx.TryParseRange(part, out int firstColumn, out int firstRow,
                                    out int lastColumn, out int lastRow))
                continue;

            if (lastColumn < firstColumn || lastRow < firstRow) continue;

            ranges.Add(new SheetRange(firstColumn, firstRow, lastColumn, lastRow));
        }
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

    /// <summary>
    /// Reads a sheet's comments for the pages they may be listed on.
    /// </summary>
    /// <remarks>
    /// The same part <see cref="ReadComments"/> reads, and read again rather than shared, because
    /// the two want different things: extraction wants the author and the text as a section, and
    /// layout wants the cell and the text as one string. Splicing the address into the extraction
    /// path to serve both would change what a caller indexing a workbook sees.
    /// </remarks>
    /// <param name="sheet">The sheet.</param>
    public SheetNotes ReadNotes(XlsxSheetEntry sheet)
    {
        XElement? root = _file.LoadComments(sheet);
        if (root is null) return SheetNotes.Empty;

        List<SheetNote> notes = [];
        foreach (XElement comment in Xlsx.Children(Xlsx.Child(root, "commentList"), "comment"))
        {
            string text = XlsxSharedStrings.ReadRichString(Xlsx.Child(comment, "text"));
            if (text.Length == 0) continue;

            if (!SheetAddress.TryParseCell(Xlsx.Attribute(comment, "ref"), out int column, out int row))
                continue;
            if (column < 0 || row < 0) continue;

            notes.Add(new SheetNote(column, row, text));
        }

        return notes.Count == 0 ? SheetNotes.Empty : new SheetNotes { Items = notes };
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
    private List<ContentTableCell> ReadCells(
        XElement rowElement, int rowIndex, MergeMap merges,
        int? rowStyle, ColumnStyles columnStyles)
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

            ContentTableCell cell = ReadCell(
                cellElement, rowIndex, column, merges,
                rowStyle ?? columnStyles.At(column));
            if (!IsEmpty(cell)) lastWithContent = cells.Count;
            cells.Add(cell);
        }

        if (lastWithContent + 1 < cells.Count)
            cells.RemoveRange(lastWithContent + 1, cells.Count - lastWithContent - 1);
        return cells;
    }

    private static bool IsEmpty(ContentTableCell cell)
        => cell.Value is null && cell.Formula is null && cell.Children.Count == 0;

    private ContentTableCell ReadCell(
        XElement element, int row, int column, MergeMap merges, int? inherited)
    {
        // The cell's own s wins, then the row's, then the column's, then the Default cell
        // style — measured on both installed binaries over a probe workbook giving all four
        // different formats (dotnet/probes/numfmt-r68/make-default.py).
        NumberFormatCode format =
            _file.Styles.FormatFor(Xlsx.Integer(element, "s") ?? inherited);
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

        // **A cell an icon set or a data bar hides is hidden at PAINT time and not here.**
        // `ScOutputData::DrawStrings` clears `bDoCell` after the row's height is settled
        // (`sc/source/ui/view/output2.cxx`:1691-1698 in this tree), so the string still measures
        // the row — and a cell whose pattern carries a conditional format is measured through
        // the EditEngine branch, which is a line taller than the arithmetic one. Dropping the
        // text here made the row shrink with it: on
        // `tests/corpus/features/sheet-cf-icon-set-x14-custom.xlsx` the pitch was 13.78 pt
        // against 26.2.4.2's own `style:row-height="0.2071in"`, 14.91. `SheetDecoration.HidesValue`
        // is the drawing-time answer and `SpreadsheetPages.DrawCell` asks it.
        if (!_pivotLabels.Blanks(row, column)) AddText(cell, display);
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
            $"A sheet addresses more {what} than X-Ray.Office materialises; the excess has been "
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
        private readonly List<SheetRange> _ranges = [];

        /// <summary>Every merged block, as the sheet states it.</summary>
        public IReadOnlyList<SheetRange> Ranges => _ranges;

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

    /// <summary>
    /// The cell format each column's <c>&lt;col style=…&gt;</c> gives the cells that state none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A column's default pattern reaches a cell with no <c>s</c> of its own, and a row's reaches
    /// it in preference to the column's. Measured on both installed binaries over a probe
    /// workbook giving the column, the row, the cell and the Default cell style four
    /// distinguishable formats (<c>dotnet/probes/numfmt-r68/make-default.py</c>): a cell in a
    /// styled column shows the column's, the same cell inside a <c>customFormat</c> row shows the
    /// row's, and a cell with its own <c>s</c> shows its own in both.
    /// </para>
    /// <para>
    /// Ranges rather than an array, because <c>&lt;col&gt;</c> legitimately spans to the sheet's
    /// last column and a per-column array would be sixteen thousand entries for one statement.
    /// </para>
    /// </remarks>
    private sealed class ColumnStyles
    {
        private static readonly ColumnStyles Empty = new();
        private readonly List<(int First, int Last, int Style)> _ranges = [];

        public static ColumnStyles Read(XElement worksheet)
        {
            ColumnStyles styles = new();
            foreach (XElement column in Xlsx.Children(Xlsx.Child(worksheet, "cols"), "col"))
            {
                if (Xlsx.Integer(column, "style") is not { } style || style < 0) continue;

                int first = (Xlsx.Integer(column, "min") - 1) ?? 0;
                int last = (Xlsx.Integer(column, "max") - 1) ?? first;
                if (first < 0) first = 0;
                if (last < first) continue;

                styles._ranges.Add((first, Math.Min(last, MaxColumns - 1), style));
            }

            return styles._ranges.Count == 0 ? Empty : styles;
        }

        /// <summary>The style index a column names, or null when it names none.</summary>
        public int? At(int column)
        {
            // Last statement wins: <cols> may overlap, and a later, narrower run is the one a
            // producer writing a whole-sheet default followed by exceptions means.
            for (int at = _ranges.Count - 1; at >= 0; at--)
            {
                (int first, int last, int style) = _ranges[at];
                if (column >= first && column <= last) return style;
            }
            return null;
        }
    }
}
