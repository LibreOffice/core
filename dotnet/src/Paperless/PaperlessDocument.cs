using Paperless.Containers;
using Paperless.Core;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;

namespace Paperless;

/// <summary>
/// The one-line entry point to Paperless: hand it a file, get back a document.
/// </summary>
/// <remarks>
/// <para>
/// Sniffs the content to decide what the file really is, then dispatches to the reader
/// for that family. Callers who already know the format, or who want to keep their
/// dependency footprint down, can skip this and use
/// <see cref="WordProcessing.WordProcessingReader"/>,
/// <see cref="Spreadsheets.SpreadsheetReader"/> or
/// <see cref="Presentations.PresentationReader"/> directly.
/// </para>
/// <para>
/// Identification is always by content, never by extension. Mislabelled files are
/// common, and several distinctions cannot be made from a name at all.
/// </para>
/// </remarks>
public static class PaperlessDocument
{
    /// <summary>
    /// Identifies a file's format without reading it.
    /// </summary>
    /// <remarks>
    /// Cheap — it reads only the container header and, for packages, the few entries
    /// needed to be sure. Use it to filter a corpus before deciding what to open.
    /// </remarks>
    public static FormatIdentification Identify(string path)
    {
        ArgumentException.ThrowIfNullOrEmpty(path);
        using FileStream stream = File.OpenRead(path);
        return Identify(stream, Path.GetFileName(path));
    }

    /// <inheritdoc cref="Identify(string)"/>
    public static FormatIdentification Identify(Stream stream, string? fileNameHint = null)
        => FormatIdentifier.Instance.Identify(stream, fileNameHint);

    /// <summary>
    /// Opens a document from disk.
    /// </summary>
    /// <exception cref="Core.UnsupportedFormatException">The format is not supported.</exception>
    /// <exception cref="Core.PasswordRequiredException">The document is encrypted.</exception>
    public static IDocument Open(string path)
    {
        ArgumentException.ThrowIfNullOrEmpty(path);
        DocumentSource source = DocumentSource.FromFile(path);
        try
        {
            // A reader never takes ownership of a source it was handed, so a document opened
            // from a path has to carry the file handle's lifetime itself. Without this the
            // handle would stay open until the finaliser ran.
            return new OwnedSourceDocument(Open(source), source);
        }
        catch
        {
            source.Dispose();
            throw;
        }
    }

    /// <inheritdoc cref="Open(string)"/>
    public static IDocument Open(DocumentSource source)
    {
        ArgumentNullException.ThrowIfNull(source);

        DocumentFormat format = SourceIdentification.Resolve(source);
        FormatInfo? info = FormatCatalogue.Instance.GetInfo(format);

        // Dispatch on family rather than on a per-format list, so a format added to the
        // catalogue reaches its reader without this method needing to change.
        return (info?.Family ?? DocumentFamily.Unknown) switch
        {
            DocumentFamily.WordProcessing => new WordProcessing.WordProcessingReader().Read(source),
            DocumentFamily.Spreadsheet => new Spreadsheets.SpreadsheetReader().Read(source),
            DocumentFamily.Presentation => new Presentations.PresentationReader().Read(source),
            _ => throw new UnsupportedFormatException(
                format,
                format == DocumentFormat.Unknown
                    ? "The file is not a recognised office document."
                    : $"{format} has no reader."),
        };
    }

    /// <summary>
    /// Extracts a document's text, tables and structure in one call, without laying it
    /// out.
    /// </summary>
    /// <remarks>
    /// The shortest path for search indexing and text analysis. Skips fonts, layout and
    /// rasterisation entirely, so it costs a small fraction of what rendering does.
    /// </remarks>
    public static ContentDocument Extract(string path)
    {
        ArgumentException.ThrowIfNullOrEmpty(path);
        using DocumentSource source = DocumentSource.FromFile(path);
        using IDocument document = Open(source);
        return document.Content;
    }

    /// <inheritdoc cref="Extract(string)"/>
    public static ContentDocument Extract(DocumentSource source)
    {
        using IDocument document = Open(source);
        return document.Content;
    }
}
