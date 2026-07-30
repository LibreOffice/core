using Paperless.Core.Diagnostics;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;

namespace Paperless;

/// <summary>
/// A document that also owns the source it was read from, so that disposing it closes the
/// underlying file.
/// </summary>
/// <remarks>
/// Readers deliberately do not take ownership of a <see cref="DocumentSource"/> they are
/// handed — the caller may want to reuse the stream, and a library that closes streams it did
/// not open is a nuisance. But <see cref="PaperlessDocument.Open(string)"/> creates the source
/// itself, so something has to close it; this is that something.
/// </remarks>
internal sealed class OwnedSourceDocument(IDocument inner, DocumentSource source) : IDocument
{
    /// <inheritdoc/>
    public DocumentFormat Format => inner.Format;

    /// <inheritdoc/>
    public DocumentFamily Family => inner.Family;

    /// <inheritdoc/>
    public DocumentMetadata Metadata => inner.Metadata;

    /// <inheritdoc/>
    public ContentDocument Content => inner.Content;

    /// <inheritdoc/>
    public IReadOnlyList<Diagnostic> Diagnostics => inner.Diagnostics;

    /// <inheritdoc/>
    public void Dispose()
    {
        // The document first: it reads from the source, so it must let go before the stream
        // underneath it closes.
        inner.Dispose();
        source.Dispose();
    }
}
