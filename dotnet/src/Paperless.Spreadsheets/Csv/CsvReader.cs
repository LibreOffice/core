using System.Globalization;
using Paperless.Core.Diagnostics;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;

namespace Paperless.Spreadsheets.Csv;

/// <summary>
/// Reads separated-value text: <c>csv</c>, and the tab- and semicolon-separated files that
/// travel under the same name.
/// </summary>
/// <remarks>
/// <para>
/// A CSV file states nothing about itself — not its encoding, not its separator, not whether
/// it quotes — so reading one is detection followed by parsing. The detection lives in
/// <see cref="CsvDialect"/> and every decision it makes is recorded as a
/// <see cref="Diagnostic"/>, because a disagreement with another reader is usually a
/// different reading of an ambiguous file rather than a defect, and that distinction can
/// only be made if the reading is visible.
/// </para>
/// <para>
/// The one thing this reader deliberately does not do is interpret. A field reading
/// <c>=B2*C2</c> is extracted as those six characters; Calc's import would compile it into a
/// formula and show the result instead. A field reading <c>4.50</c> keeps its trailing zero
/// rather than becoming 4.5. Extraction reports what the file says, and a file of text is
/// text until somebody decides otherwise.
/// </para>
/// </remarks>
public static class CsvReader
{
    /// <summary>The most rows one file materialises.</summary>
    public const int MaxRows = 1_048_576;

    /// <summary>The most fields one row materialises.</summary>
    public const int MaxColumns = 16384;

    /// <summary>Reads a file, leaving the source's stream for the caller to dispose.</summary>
    public static CsvDocument Read(DocumentSource source, DocumentFormat format)
    {
        ArgumentNullException.ThrowIfNull(source);

        using MemoryStream buffer = new();
        source.Stream.Position = 0;
        source.Stream.CopyTo(buffer);
        byte[] bytes = buffer.ToArray();

        List<Diagnostic> diagnostics = [];
        CsvDialect dialect = CsvDialect.Detect(bytes, source.FallbackEncoding, diagnostics);
        string text = dialect.Decode(bytes);

        ContentDocument content = new()
        {
            Metadata = new DocumentMetadata
            {
                // Nothing in a CSV file records metadata. One sheet is the only fact there is.
                Statistics = new DocumentStatistics { SheetCount = 1 },
            },
        };

        ContentSection sheet = new()
        {
            Kind = SectionKind.Sheet,
            Index = 0,

            // Calc names the sheet after the file, which is the only name available.
            Name = SheetNameOf(source.FileName),
        };

        sheet.Children.Add(Parse(text, dialect, diagnostics));
        content.Children.Add(sheet);

        return new CsvDocument(format, content, diagnostics, dialect);
    }

    private static string? SheetNameOf(string? fileName)
        => fileName is null ? null : Path.GetFileNameWithoutExtension(fileName);

    /// <summary>
    /// Splits the text into rows and fields.
    /// </summary>
    /// <remarks>
    /// The quoting rules are the ones RFC 4180 wrote down after the fact: a field may be
    /// wrapped in quotes, inside which separators and line breaks are ordinary characters and
    /// a doubled quote means one quote. Text after a closing quote but before the next
    /// separator is kept rather than discarded — malformed, but discarding it loses content
    /// that is plainly there.
    /// </remarks>
    private static ContentTable Parse(string text, CsvDialect dialect, List<Diagnostic> diagnostics)
    {
        ContentTableRow row = new() { Index = 0 };
        List<ContentTableRow> rows = [row];
        System.Text.StringBuilder field = new();
        bool quoted = false;
        bool anyContent = false;
        int column = 0;
        bool truncated = false;

        for (int i = 0; i < text.Length; i++)
        {
            char c = text[i];

            if (quoted)
            {
                if (c != dialect.Quote)
                {
                    field.Append(c);
                    continue;
                }

                // A doubled quote inside a quoted field is one quote.
                if (i + 1 < text.Length && text[i + 1] == dialect.Quote)
                {
                    field.Append(c);
                    i++;
                    continue;
                }

                quoted = false;
                continue;
            }

            if (c == dialect.Quote && field.Length == 0)
            {
                quoted = true;
                continue;
            }

            if (c == dialect.Separator)
            {
                anyContent |= AddField(row, field, column) && field.Length > 0;
                column++;
                continue;
            }

            if (c is '\r' or '\n')
            {
                // A CRLF is one break, and a lone CR is the break a classic Mac file uses.
                if (c == '\r' && i + 1 < text.Length && text[i + 1] == '\n') i++;

                anyContent |= AddField(row, field, column) && field.Length > 0;
                column = 0;

                if (rows.Count >= MaxRows)
                {
                    truncated = true;
                    break;
                }

                row = new ContentTableRow { Index = rows.Count };
                rows.Add(row);
                continue;
            }

            if (column < MaxColumns) field.Append(c);
        }

        AddField(row, field, column);

        // A trailing line break makes an empty last row that is not in the file.
        if (rows.Count > 1 && rows[^1].Children.Count <= 1 && rows[^1].GetText().Trim().Length == 0)
            rows.RemoveAt(rows.Count - 1);

        if (truncated)
        {
            diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Warning, "PL2343",
                $"The file has more than {MaxRows} rows; the rest have been dropped."));
        }

        _ = anyContent;

        ContentTable table = new()
        {
            ColumnCount = rows.Count == 0 ? 0 : rows.Max(r => r.Children.Count),
        };

        foreach (ContentTableRow parsed in rows) table.Children.Add(parsed);
        return table;
    }

    private static bool AddField(ContentTableRow row, System.Text.StringBuilder field, int column)
    {
        if (column >= MaxColumns)
        {
            field.Clear();
            return false;
        }

        string text = field.ToString();
        field.Clear();

        ContentTableCell cell = new()
        {
            Row = row.Index,
            Column = column,
            Value = ValueOf(text),
        };

        if (text.Length > 0)
        {
            ContentParagraph paragraph = new();
            paragraph.Children.Add(new ContentRun { Text = text });
            cell.Children.Add(paragraph);
        }

        row.Children.Add(cell);
        return true;
    }

    /// <summary>
    /// A field's value: a number when the text is one exactly, and the text otherwise.
    /// </summary>
    /// <remarks>
    /// "Exactly" means the number formats back to the identical characters. That is what
    /// keeps <c>007</c> and <c>4.50</c> as text — a spreadsheet application would convert
    /// both and show 7 and 4.5, losing what the file said — while <c>12</c> and
    /// <c>-3.25</c> become numbers a caller can compute with. The displayed text is the
    /// field either way, so the choice never changes what the extraction reads as.
    /// </remarks>
    private static object? ValueOf(string text)
    {
        if (text.Length == 0) return null;

        return double.TryParse(text, NumberStyles.Float, CultureInfo.InvariantCulture, out double number)
               && number.ToString("R", CultureInfo.InvariantCulture) == text
            ? number
            : text;
    }
}

/// <summary>A separated-value file that has been read.</summary>
public sealed class CsvDocument : IDocument
{
    internal CsvDocument(
        DocumentFormat format,
        ContentDocument content,
        IReadOnlyList<Diagnostic> diagnostics,
        CsvDialect dialect)
    {
        Format = format;
        Content = content;
        Diagnostics = diagnostics;
        Dialect = dialect;
    }

    /// <inheritdoc/>
    public DocumentFormat Format { get; }

    /// <inheritdoc/>
    public DocumentFamily Family => DocumentFamily.Spreadsheet;

    /// <inheritdoc/>
    public DocumentMetadata Metadata => Content.Metadata;

    /// <inheritdoc/>
    public ContentDocument Content { get; }

    /// <inheritdoc/>
    public IReadOnlyList<Diagnostic> Diagnostics { get; }

    /// <summary>
    /// How the file was read: which separator, which quote character and which encoding.
    /// </summary>
    /// <remarks>
    /// Exposed because it is a guess. A caller that knows better — because it has a hundred
    /// files from the same source and the first one came out wrong — needs to be able to see
    /// what was assumed.
    /// </remarks>
    public CsvDialect Dialect { get; }

    /// <inheritdoc/>
    public void Dispose()
    {
        // Nothing to release: the file was read into memory and the source still owns its
        // stream.
    }
}
