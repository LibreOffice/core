using Paperless.Core.Diagnostics;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;

namespace Paperless.OpenDocument;

/// <summary>An ODF document that has been read.</summary>
/// <remarks>
/// One implementation serves all three families. Nothing about holding an ODF document's
/// extracted content differs between a text document, a spreadsheet and a presentation — only
/// the walk that produced it does, and that is <see cref="OdfReader"/>'s job.
/// </remarks>
public sealed class OdfDocument : IDocument
{
    private readonly OdfFile _file;

    internal OdfDocument(
        DocumentFormat format,
        DocumentFamily family,
        OdfFile file,
        ContentDocument content,
        IReadOnlyList<Diagnostic> diagnostics)
    {
        Format = format;
        Family = family;
        _file = file;
        Content = content;
        Diagnostics = diagnostics;
    }

    /// <inheritdoc/>
    public DocumentFormat Format { get; }

    /// <inheritdoc/>
    public DocumentFamily Family { get; }

    /// <inheritdoc/>
    public DocumentMetadata Metadata => Content.Metadata;

    /// <inheritdoc/>
    public ContentDocument Content { get; }

    /// <inheritdoc/>
    public IReadOnlyList<Diagnostic> Diagnostics { get; }

    /// <summary>
    /// The underlying file: its styles, master pages and remaining parts.
    /// </summary>
    /// <remarks>
    /// Kept reachable because rendering will need the styles the content tree deliberately
    /// discards, and because a caller wanting an embedded image's bytes has to get at the
    /// package. Valid until this document is disposed.
    /// </remarks>
    public OdfFile File => _file;

    /// <inheritdoc/>
    public void Dispose() => _file.Dispose();
}
