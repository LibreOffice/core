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
    public static FormatIdentification Identify(string path) => throw new NotImplementedException();

    /// <inheritdoc cref="Identify(string)"/>
    public static FormatIdentification Identify(Stream stream, string? fileNameHint = null)
        => throw new NotImplementedException();

    /// <summary>
    /// Opens a document from disk.
    /// </summary>
    /// <exception cref="Core.UnsupportedFormatException">The format is not supported.</exception>
    /// <exception cref="Core.PasswordRequiredException">The document is encrypted.</exception>
    public static IDocument Open(string path) => throw new NotImplementedException();

    /// <inheritdoc cref="Open(string)"/>
    public static IDocument Open(DocumentSource source) => throw new NotImplementedException();

    /// <summary>
    /// Extracts a document's text, tables and structure in one call, without laying it
    /// out.
    /// </summary>
    /// <remarks>
    /// The shortest path for search indexing and text analysis. Skips fonts, layout and
    /// rasterisation entirely, so it costs a small fraction of what rendering does.
    /// </remarks>
    public static ContentDocument Extract(string path) => throw new NotImplementedException();

    /// <inheritdoc cref="Extract(string)"/>
    public static ContentDocument Extract(DocumentSource source) => throw new NotImplementedException();
}
