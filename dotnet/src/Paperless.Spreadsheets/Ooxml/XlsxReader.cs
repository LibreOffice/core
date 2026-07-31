using System.Xml.Linq;
using Paperless.Containers.Ooxml;
using Paperless.Core.Diagnostics;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;
using Paperless.Ooxml;

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
            List<Diagnostic> diagnostics = [.. file.Diagnostics];
            ContentDocument content = new()
            {
                Metadata = OoxmlMetadata.Read(
                    (OpcPackage)file.Package, DocumentFamily.Spreadsheet),
            };

            XlsxSheetReader reader = new(file, diagnostics);
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
                section.Children.Add(worksheet is null
                    ? new ContentTable()
                    : reader.ReadSheet(worksheet));
                content.Children.Add(section);

                // Comments belong to the sheet that holds them, so they land immediately after
                // it rather than at the end of the workbook.
                foreach (ContentSection comment in reader.ReadComments(entry))
                    content.Children.Add(comment);
            }

            if (file.Sheets.Count == 0)
            {
                diagnostics.Add(new Diagnostic(
                    DiagnosticSeverity.Error, "PL2146",
                    "The workbook lists no sheets, so it has no content to extract."));
            }

            return new OoxmlSpreadsheetDocument(format, file, content, diagnostics);
        }
        catch
        {
            file.Dispose();
            throw;
        }
    }
}

/// <summary>An OOXML spreadsheet that has been read.</summary>
public sealed class OoxmlSpreadsheetDocument : IDocument
{
    private readonly XlsxFile _file;

    internal OoxmlSpreadsheetDocument(
        DocumentFormat format,
        XlsxFile file,
        ContentDocument content,
        IReadOnlyList<Diagnostic> diagnostics)
    {
        Format = format;
        _file = file;
        Content = content;
        Diagnostics = diagnostics;
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
    /// The underlying package: its sheets, styles, shared strings and remaining parts.
    /// </summary>
    /// <remarks>
    /// Kept reachable because rendering will need the formatting the content tree deliberately
    /// discards — print setup above all, since a spreadsheet's page geometry exists nowhere
    /// else. Valid until this document is disposed.
    /// </remarks>
    public XlsxFile File => _file;

    /// <inheritdoc/>
    public void Dispose() => _file.Dispose();
}
