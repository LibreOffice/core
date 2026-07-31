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

/// <summary>
/// The same thing for a document that can be laid out.
/// </summary>
/// <remarks>
/// A separate type because <see cref="IPaginatedDocument"/> is discovered by a type test:
/// a caller asks <c>document is IPaginatedDocument</c> and lays it out if it is. A wrapper
/// implementing only <see cref="IDocument"/> answers no on behalf of a document that
/// answers yes, so every word-processing file opened by path reports that it cannot be
/// rendered — which is exactly what happened the first time <c>paperless render</c> was
/// pointed at one.
/// </remarks>
internal sealed class OwnedPaginatedDocument(IPaginatedDocument inner, DocumentSource source)
    : IPaginatedDocument
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
    public IPageSequence Layout(LayoutOptions? options = null) => inner.Layout(options);

    /// <inheritdoc/>
    public void Dispose()
    {
        inner.Dispose();
        source.Dispose();
    }
}
