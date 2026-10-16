using XRay.Office.Core.Formats;
using XRay.Office.Core.Geometry;

namespace XRay.Office.Core.Documents;

/// <summary>
/// A loaded office document.
/// </summary>
/// <remarks>
/// <para>
/// X-Ray.Office separates <em>reading</em> a document from <em>using</em> it. An
/// <see cref="IDocumentReader"/> turns bytes into an <see cref="IDocument"/>; from
/// there, callers either extract content (<see cref="Extraction"/>) or lay it out
/// and render it (<see cref="IPaginatedDocument"/>).
/// </para>
/// <para>
/// The split matters for performance: extraction is the common case and never
/// needs fonts, layout or a rasteriser, so it must not pay for them.
/// </para>
/// </remarks>
public interface IDocument : IDisposable
{
    /// <summary>The format this document was read from.</summary>
    DocumentFormat Format { get; }

    /// <summary>Which application family the document belongs to.</summary>
    DocumentFamily Family { get; }

    /// <summary>Document-level metadata: title, author, timestamps, custom properties.</summary>
    DocumentMetadata Metadata { get; }

    /// <summary>
    /// The document's text, tables and structure as a format-independent tree.
    /// </summary>
    /// <remarks>
    /// Populated when the document is read rather than on demand: parsing the file is the
    /// work, and a reader that has parsed it has already produced this. The expensive,
    /// deferred step is <see cref="IPaginatedDocument.Layout(LayoutOptions?)"/>, which is why that is a
    /// method and this is a property.
    /// </remarks>
    Extraction.ContentDocument Content { get; }

    /// <summary>
    /// Non-fatal problems encountered while reading. X-Ray.Office is deliberately
    /// lenient — real-world files violate their own specifications constantly —
    /// so a successful read can still report diagnostics.
    /// </summary>
    IReadOnlyList<Diagnostics.Diagnostic> Diagnostics { get; }
}

/// <summary>
/// A document that can be laid out into pages and rendered.
/// </summary>
/// <remarks>
/// "Page" means whatever the family's natural unit is: a paper page for word
/// processing, a printed sheet page for spreadsheets, and a slide for
/// presentations.
/// </remarks>
public interface IPaginatedDocument : IDocument
{
    /// <summary>
    /// Lays the document out and returns the resulting pages.
    /// </summary>
    /// <remarks>
    /// Layout is deferred rather than done at load time because it is by far the
    /// expensive half of the work and callers that only want text never need it.
    /// </remarks>
    /// <param name="options">Layout options; pass <c>null</c> for the defaults.</param>
    IPageSequence Layout(LayoutOptions? options = null);

    /// <summary>
    /// Lays the document out, checking <paramref name="cancellationToken"/> as it goes.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Throws <see cref="OperationCanceledException"/> when the token is cancelled, and
    /// <see cref="Threading.LayoutTimeoutException"/> (also an <see cref="OperationCanceledException"/>)
    /// when <see cref="LayoutOptions.Timeout"/> runs out first. The checks sit at the loops that bound
    /// latency: each paragraph, line and table row of a word-processing document, each row strip and
    /// formula pass of a spreadsheet, each slide and shape of a deck.
    /// </para>
    /// <para>
    /// A default interface method, so an implementation written against the earlier interface keeps
    /// compiling; ours override nothing and pick the scope up from here.
    /// </para>
    /// </remarks>
    /// <param name="options">Layout options; pass <c>null</c> for the defaults.</param>
    /// <param name="cancellationToken">Cancels the layout.</param>
    IPageSequence Layout(LayoutOptions? options, CancellationToken cancellationToken)
        => Threading.OfficeOperation.Run(() => Layout(options), cancellationToken);
}

/// <summary>The pages produced by laying out a document.</summary>
public interface IPageSequence
{
    /// <summary>How many pages the layout produced.</summary>
    int Count { get; }

    /// <summary>Gets a page by zero-based index.</summary>
    IPage this[int index] { get; }
}

/// <summary>A single laid-out page, ready to be drawn.</summary>
public interface IPage
{
    /// <summary>The zero-based page index.</summary>
    int Index { get; }

    /// <summary>The page's physical size.</summary>
    DocSize Size { get; }

    /// <summary>
    /// An optional label: a slide name, a sheet name, or a printed page number
    /// that differs from <see cref="Index"/>.
    /// </summary>
    string? Label { get; }

    /// <summary>
    /// Emits the page's content as a sequence of drawing commands.
    /// </summary>
    /// <param name="sink">Receives the drawing commands.</param>
    void Draw(Graphics.IDrawingSink sink);
}

/// <summary>Options controlling how a document is laid out.</summary>
public sealed record LayoutOptions
{
    /// <summary>The defaults.</summary>
    public static readonly LayoutOptions Default = new();

    /// <summary>
    /// Upper bound on the number of pages to produce, as a guard against
    /// pathological or hostile documents. Zero means unlimited.
    /// </summary>
    public int MaxPages { get; init; }

    /// <summary>
    /// Whether to honour the document's own "update fields on load" behaviour.
    /// When false, field results stored in the file are used verbatim — which is
    /// what matching a reference renderer requires.
    /// </summary>
    public bool UpdateFields { get; init; }

    /// <summary>
    /// Whether to recalculate formulas rather than trusting cached results.
    /// Spreadsheets only.
    /// </summary>
    public bool RecalculateFormulas { get; init; }

    /// <summary>
    /// Whether to render tracked changes as accepted. When false, insertions and
    /// deletions are shown with change marks, as LibreOffice does by default.
    /// </summary>
    public bool AcceptTrackedChanges { get; init; }

    /// <summary>
    /// A wall-clock limit for <see cref="IPaginatedDocument.Layout(LayoutOptions?)"/>, as a guard
    /// against documents that lay out for minutes. Null, or a non-positive value, means no limit.
    /// </summary>
    /// <remarks>
    /// Exceeding it throws <see cref="Threading.LayoutTimeoutException"/>, which derives from
    /// <see cref="OperationCanceledException"/> but is distinguishable from the caller cancelling a
    /// token. The limit covers layout only; rendering the pages is bounded by its own token.
    /// </remarks>
    public TimeSpan? Timeout { get; init; }

    /// <summary>
    /// The largest embedded picture, in pixels (width times height), that is decoded when the pages
    /// are rendered. Zero means unlimited.
    /// </summary>
    /// <remarks>
    /// A header is read to learn the size before any pixel is, so a poster-sized scan costs a
    /// header read and not a gigabyte. What happens to a picture over the limit is
    /// <see cref="OversizedImages"/>. The limit travels with the laid-out pages, so the PDF and
    /// raster renderers apply it without being told; a renderer's own option, if set, is applied as
    /// well and the stricter of the two wins.
    /// </remarks>
    public long MaxImagePixels { get; init; }

    /// <summary>What happens to a picture larger than <see cref="MaxImagePixels"/>.</summary>
    public OversizedImagePolicy OversizedImages { get; init; }
}

/// <summary>What a renderer does with a picture over the pixel limit.</summary>
public enum OversizedImagePolicy
{
    /// <summary>The picture is left out; the rest of the page is drawn as usual.</summary>
    Skip = 0,

    /// <summary>
    /// The picture is decoded at a reduced size when its codec can do that cheaply (JPEG decodes
    /// at 1/2, 1/4 or 1/8 scale), and left out when it cannot.
    /// </summary>
    Downscale = 1,
}

/// <summary>
/// Implemented by a page that carries rendering limits chosen when the document was laid out.
/// </summary>
/// <remarks>
/// A renderer asks each page, so that <see cref="LayoutOptions.MaxImagePixels"/> reaches the decoder
/// without the caller having to repeat it as a renderer option.
/// </remarks>
public interface IPageRenderLimits
{
    /// <summary>See <see cref="LayoutOptions.MaxImagePixels"/>.</summary>
    long MaxImagePixels { get; }

    /// <summary>See <see cref="LayoutOptions.OversizedImages"/>.</summary>
    OversizedImagePolicy OversizedImages { get; }
}
