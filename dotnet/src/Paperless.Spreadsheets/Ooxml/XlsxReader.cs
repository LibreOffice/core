using System.Xml.Linq;
using Paperless.Containers.Ooxml;
using Paperless.Core.Diagnostics;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;
using Paperless.Ooxml;
using Paperless.Ooxml.DrawingML;
using Paperless.Spreadsheets.Layout;

namespace Paperless.Spreadsheets.Ooxml;

/// <summary>
/// Reads OOXML spreadsheets: <c>xlsx</c>, <c>xlsm</c>, and the <c>xltx</c> and <c>xltm</c>
/// templates.
/// </summary>
/// <remarks>
/// <para>
/// The macro-enabled variants are read as data like any other. A macro's presence is reported
/// through <c>FormatInfo.CanCarryMacros</c>; nothing is ever executed.
/// </para>
/// <para>
/// Each sheet becomes a <see cref="SectionKind.Sheet"/> section holding one
/// <see cref="ContentTable"/>, and cell comments follow the sheet they belong to as
/// <see cref="SectionKind.Comment"/> sections — the same shape the ODS path produces, so a
/// caller indexing a mixed corpus never branches on which of the two it opened.
/// </para>
/// <para>
/// Hidden sheets are extracted and flagged rather than skipped. That is a deliberate difference
/// from LibreOffice's own output: neither its CSV filter nor its PDF export includes a hidden
/// sheet at all, and a caller indexing content wants it.
/// </para>
/// </remarks>
public static class XlsxReader
{
    /// <summary>Reads a workbook, leaving the source's stream for the caller to dispose.</summary>
    /// <param name="source">The document to read.</param>
    /// <param name="format">The identified format, recorded on the result.</param>
    public static OoxmlSpreadsheetDocument Read(DocumentSource source, DocumentFormat format)
    {
        ArgumentNullException.ThrowIfNull(source);

        XlsxFile file = XlsxFile.Open(source.Stream, leaveOpen: true);
        try
        {
            // The file keeps accumulating diagnostics while its sheets are read — a part that
            // will not parse is only discovered then — so the two lists are joined at the end
            // rather than copied up front, where everything found during the walk is lost.
            List<Diagnostic> diagnostics = [];
            ContentDocument content = new()
            {
                Metadata = OoxmlMetadata.Read(
                    (OpcPackage)file.Package, DocumentFamily.Spreadsheet),
            };

            XlsxSheetReader reader = new(file, diagnostics);

            // The print names are workbook-level and scoped to a sheet by position, so they are
            // read once up front rather than looked up per sheet.
            Dictionary<int, XlsxSheetPrintNames> names = XlsxPrintNames.Read(file.Workbook);
            List<SheetLayout> layouts = [];

            // The workbook's cell formats, read once. Only layout looks at them, and only the
            // fonts-and-alignment half of styles.xml is read here — the number formats extraction
            // needs are already resolved on file.Styles.
            XlsxCellFormatTable cellFormats = XlsxCellFormats.Read(file.StyleSheet, file.Styles);

            // The workbook's theme, read once. A chart part that states an a:schemeClr needs it
            // and every other drawing does not, which is why it was missing: nothing in a
            // spreadsheet's drawing path asked for a theme until a chart did.
            DrawingTheme? theme = DrawingTheme.Read(file.ThemeRoot);

            // The same theme part, read for its other half. A text box's runs name their face
            // indirectly far more often than directly, and the six names that indirection uses
            // live here rather than in the colour scheme.
            DrawingFontScheme? themeFonts = DrawingFontScheme.Read(
                Drawing.Child(Drawing.Child(file.ThemeRoot, "themeElements"), "fontScheme"));

            foreach (XlsxSheetEntry entry in file.Sheets)
            {
                ContentSection section = new()
                {
                    Kind = SectionKind.Sheet,
                    Index = entry.Index,
                    Name = entry.Name,
                    IsHidden = entry.IsHidden,
                };

                XElement? worksheet = file.LoadSheet(entry);
                ContentTable table = worksheet is null
                    ? new ContentTable()
                    : reader.ReadSheet(worksheet);

                section.Children.Add(table);
                content.Children.Add(section);

                // The print setup is read from the part that is already open rather than in a
                // second pass, because parsing the worksheet is the expensive half of reading a
                // workbook and doing it twice is also a second chance to disagree with itself.
                XlsxSheetPrintNames print = names.GetValueOrDefault(entry.Index, XlsxSheetPrintNames.None);
                (SheetPrintSetup setup, SheetGrid grid) = XlsxPrintSetup.Read(
                    worksheet, print.PrintAreas, print.RepeatColumns, print.RepeatRows,
                    cellFormats.DefaultColumnFont);

                (SheetCellFormats formats, SheetRichText rich) =
                    XlsxSheetFormats.Read(worksheet, cellFormats, file);

                layouts.Add(new SheetLayout
                {
                    Name = entry.Name,
                    Index = entry.Index,
                    IsHidden = entry.IsHidden,
                    Setup = setup,
                    Grid = grid,
                    Cells = table,
                    StatedMerges = XlsxSheetReader.ReadMerges(worksheet),
                    HyperlinkRanges = XlsxSheetReader.ReadHyperlinks(worksheet),
                    Formatting = XlsxCellDecoration.Read(file.StyleSheet, file.ThemeRoot, worksheet),
                    Formats = formats,
                    RichText = rich,
                    Drawings = XlsxDrawings.Read(
                        file.Package, entry.PartName, theme, themeFonts),
                    Notes = setup.PrintsNotes ? reader.ReadNotes(entry) : SheetNotes.Empty,
                    FileName = source.FileName ?? string.Empty,
                });

                // Comments belong to the sheet that holds them, so they land immediately after
                // it rather than at the end of the workbook.
                foreach (ContentSection comment in reader.ReadComments(entry))
                    content.Children.Add(comment);

                // A chart follows its sheet for the same reason, and cannot go inside it: the
                // sheet section holds exactly one table, and a chart is another one. The ODS
                // path puts it in the same place.
                foreach (ContentSection chart in XlsxCharts.Read(file.Package, entry.PartName))
                    content.Children.Add(chart);
            }

            if (file.Sheets.Count == 0)
            {
                diagnostics.Add(new Diagnostic(
                    DiagnosticSeverity.Error, "PL2146",
                    "The workbook lists no sheets, so it has no content to extract."));
            }

            return new OoxmlSpreadsheetDocument(
                format, file, content, [.. file.Diagnostics, .. diagnostics], layouts);
        }
        catch
        {
            file.Dispose();
            throw;
        }
    }
}

/// <summary>An OOXML spreadsheet that has been read.</summary>
public sealed class OoxmlSpreadsheetDocument : IPaginatedDocument
{
    private readonly XlsxFile _file;

    internal OoxmlSpreadsheetDocument(
        DocumentFormat format,
        XlsxFile file,
        ContentDocument content,
        IReadOnlyList<Diagnostic> diagnostics,
        IReadOnlyList<SheetLayout> sheets)
    {
        Format = format;
        _file = file;
        Content = content;
        Diagnostics = diagnostics;
        Sheets = sheets;
    }

    /// <summary>The sheets' print setups and geometry, in workbook order.</summary>
    public IReadOnlyList<SheetLayout> Sheets { get; }

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
    /// The underlying package: its sheets, styles, shared strings and remaining parts.
    /// </summary>
    /// <remarks>
    /// Kept reachable because rendering will need the formatting the content tree deliberately
    /// discards — print setup above all, since a spreadsheet's page geometry exists nowhere
    /// else. Valid until this document is disposed.
    /// </remarks>
    public XlsxFile File => _file;

    /// <inheritdoc/>
    /// <remarks>
    /// A workbook's pages come from its print setup and from nothing else, so this is where the
    /// difference between the families shows: laying out a text document means measuring text,
    /// and laying out a workbook means dividing a grid whose sizes the file already states.
    /// </remarks>
    public IPageSequence Layout(LayoutOptions? options = null)
        => new SpreadsheetPages(Sheets, options);

    /// <inheritdoc/>
    public void Dispose() => _file.Dispose();
}
