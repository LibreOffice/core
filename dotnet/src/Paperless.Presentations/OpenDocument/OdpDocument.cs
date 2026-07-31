using Paperless.Core.Diagnostics;
using Paperless.Core.Documents;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;
using Paperless.Core.Geometry;
using Paperless.OpenDocument;
using Paperless.OpenDocument.Styles;
using Paperless.Presentations.Layout;

namespace Paperless.Presentations.OpenDocument;

/// <summary>
/// An ODF presentation that has been read, and can be laid out.
/// </summary>
/// <remarks>
/// A wrapper round <see cref="OdfDocument"/> rather than a change to it. One
/// <see cref="OdfDocument"/> serves all three ODF families, and only one of them paginates into
/// slides — making the shared type implement <see cref="IPaginatedDocument"/> would claim a
/// spreadsheet and a text document lay out this way too. The wrapper is discovered exactly as
/// the word-processing ones are, by a type test on <see cref="IPaginatedDocument"/>.
/// </remarks>
public sealed class OdpDocument : IPaginatedDocument
{
    private readonly OdfDocument _inner;

    internal OdpDocument(OdfDocument inner) => _inner = inner;

    /// <inheritdoc/>
    public DocumentFormat Format => _inner.Format;

    /// <inheritdoc/>
    public DocumentFamily Family => _inner.Family;

    /// <inheritdoc/>
    public DocumentMetadata Metadata => _inner.Metadata;

    /// <inheritdoc/>
    public ContentDocument Content => _inner.Content;

    /// <inheritdoc/>
    public IReadOnlyList<Diagnostic> Diagnostics => _inner.Diagnostics;

    /// <summary>The underlying file: its styles, master pages and remaining parts.</summary>
    public OdfFile File => _inner.File;

    /// <summary>
    /// The slide size, from the first master page the deck's slides name.
    /// </summary>
    /// <remarks>
    /// Reported alongside the PPTX reader's for the same reason: deciding whether a deck is 4:3
    /// or 16:9 needs no fonts and no rasteriser, so it should not cost a layout. Zero when the
    /// deck names no master page, which no real file does.
    /// </remarks>
    public DocSize SlideSize
    {
        get
        {
            IPageSequence pages = Layout(new LayoutOptions { MaxPages = 1 });
            return pages.Count > 0 ? pages[0].Size : default;
        }
    }

    /// <inheritdoc/>
    /// <remarks>
    /// One page per <c>draw:page</c>, in document order, each sized by the master page it names —
    /// which is not the first <c>style:page-layout</c> in the file: a deck carries at least two,
    /// and the notes one is A4 portrait in everything LibreOffice writes.
    /// </remarks>
    public IPageSequence Layout(LayoutOptions? options = null)
    {
        List<LaidOutSlide> slides = new OdpSlideLayout(_inner.File, new SlideFonts()).Layout();

        int limit = options?.MaxPages ?? 0;
        if (limit > 0 && slides.Count > limit) slides = [.. slides.Take(limit)];

        return new SlidePages(slides);
    }

    /// <inheritdoc/>
    public void Dispose() => _inner.Dispose();
}
