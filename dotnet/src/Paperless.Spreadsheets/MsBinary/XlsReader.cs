using Paperless.Containers;
using Paperless.Containers.Ole2;
using Paperless.Core;
using Paperless.Core.Diagnostics;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;
using Paperless.MsBinary.PropertySets;

namespace Paperless.Spreadsheets.MsBinary;

/// <summary>
/// Reads legacy binary Excel workbooks: <c>xls</c> and the <c>xlt</c> template, as written
/// by Excel 5.0 through Excel 2003 and by LibreOffice.
/// </summary>
/// <remarks>
/// <para>
/// An XLS file is an OLE2 compound file whose content is one stream of BIFF records. Excel
/// 97 and later name that stream <c>Workbook</c>; Excel 5.0/95 named it <c>Book</c>, and a
/// file may hold both when it has been saved by a version that wrote a dual format — in
/// which case the newer one is the current one and the older is a stale copy that parses
/// perfectly well and describes an older state of the document.
/// </para>
/// <para>
/// Metadata comes from the OLE property sets rather than from any BIFF record, so it is read
/// by the shared reader in <c>Paperless.MsBinary</c> — the same one the DOC path uses.
/// </para>
/// </remarks>
public static class XlsReader
{
    /// <summary>The stream Excel 97 and later put the workbook in.</summary>
    public const string WorkbookStreamName = "Workbook";

    /// <summary>The stream Excel 5.0/95 put the workbook in.</summary>
    public const string BookStreamName = "Book";

    /// <summary>Reads a workbook, leaving the source's stream for the caller to dispose.</summary>
    /// <param name="source">The document to read.</param>
    /// <param name="format">The identified format, which is recorded on the result.</param>
    /// <exception cref="MalformedDocumentException">
    /// The compound file has neither a <c>Workbook</c> nor a <c>Book</c> stream, so it is not
    /// an Excel workbook whatever its extension says.
    /// </exception>
    /// <exception cref="PasswordRequiredException">The workbook is encrypted.</exception>
    public static XlsDocument Read(DocumentSource source, DocumentFormat format)
    {
        ArgumentNullException.ThrowIfNull(source);

        CompoundFile file = CompoundFile.Open(source.Stream, leaveOpen: true);
        try
        {
            List<Diagnostic> diagnostics = [.. file.Diagnostics];

            // Newest first: a dual-format file holds both streams and only the newer one is
            // the document as it was last saved.
            byte[] workbook = ReadStream(file, WorkbookStreamName)
                              ?? ReadStream(file, BookStreamName)
                              ?? throw new MalformedDocumentException(
                                  "The compound file has neither a Workbook nor a Book stream, "
                                  + "so it is not an Excel workbook.");

            XlsWorkbookReader reader = new(workbook, diagnostics);
            List<ContentSection> sheets = reader.Read();

            if (reader.IsEncrypted)
            {
                // BIFF encryption is XOR obfuscation or RC4, neither of which is implemented.
                // Reporting it as a password requirement is honest: the content is there and
                // unreadable, which is not the same as the file being malformed.
                throw new PasswordRequiredException(
                    "The workbook is encrypted; BIFF decryption is not implemented.",
                    passwordWasSupplied: false);
            }

            DocumentMetadata metadata = OlePropertySetReader.Read(file);
            ContentDocument content = new()
            {
                Metadata = metadata with
                {
                    // BIFF records no sheet count anywhere in its property sets, so the count
                    // is what the sheet directory holds — hidden sheets included, which is
                    // what ODF's meta:table-count means too.
                    Statistics = (metadata.Statistics ?? new DocumentStatistics()) with
                    {
                        SheetCount = reader.SheetCount,
                    },
                },
            };

            foreach (ContentSection sheet in sheets) content.Children.Add(sheet);
            return new XlsDocument(format, file, content, diagnostics);
        }
        catch
        {
            file.Dispose();
            throw;
        }
    }

    /// <summary>The whole of one stream, or null when the compound file has no such stream.</summary>
    /// <remarks>
    /// Read whole rather than streamed because a workbook is not read front to back: each
    /// sheet is reached by an offset the directory states, and those offsets are not in sheet
    /// order in a file that has been edited.
    /// </remarks>
    private static byte[]? ReadStream(CompoundFile package, string name)
    {
        IPackagePart? part = package.GetPart(name);
        if (part is null) return null;

        using Stream stream = part.Open();
        using MemoryStream buffer = new(part.Length > 0 ? (int)Math.Min(part.Length, int.MaxValue) : 0);
        stream.CopyTo(buffer);
        return buffer.ToArray();
    }
}

/// <summary>A legacy binary Excel workbook that has been read.</summary>
public sealed class XlsDocument : IDocument
{
    private readonly CompoundFile _file;

    internal XlsDocument(
        DocumentFormat format,
        CompoundFile file,
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
    /// The underlying compound file, for callers needing a stream the content tree does not
    /// expose — embedded objects and pictures above all. Valid until this document is
    /// disposed.
    /// </summary>
    public CompoundFile File => _file;

    /// <inheritdoc/>
    public void Dispose() => _file.Dispose();
}
