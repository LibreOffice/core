using System.Globalization;
using System.Text;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Extraction;
using XRay.Office.Core.Formats;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Threading;

namespace XRay.Office.Rendering.Pdf;

/// <summary>
/// Writes laid-out pages to a PDF file.
/// </summary>
/// <remarks>
/// <para>
/// Hand-rolled rather than delegated to an existing library. The requirement is to
/// emit real text with embedded subset fonts, correct transparency groups and
/// pass-through of already-compressed images, and no C# PDF library covers that
/// combination — see <c>dotnet/research/06-rendering.md</c> section G. LibreOffice
/// reaches the same conclusion and writes its own
/// (<c>vcl/source/gdi/pdfwriter_impl.cxx</c>).
/// </para>
/// <para>
/// Font subsetting is the one part with a ready answer: HarfBuzz's <c>hb-subset</c>,
/// which LibreOffice itself uses and which HarfBuzzSharp's native library exposes.
/// </para>
/// <para>
/// <b>What makes this backend verifiable, and why the file is shaped the way it is.</b>
/// The fidelity harness in <c>tests/XRay.Office.TestKit/LibreOffice/</c> was written to read
/// LibreOffice's PDFs — <c>PdfTextRuns</c> for pen positions and sizes, <c>PdfFills</c> for
/// filled rectangles, <c>PdfStrokes</c> for stroked lines. It is pointed at ours as well,
/// so the two can be compared operator for operator rather than pixel for pixel. That
/// dictates several decisions that would otherwise be free: content streams are deflated
/// and font programs are not, text is shown from simple one-byte-coded TrueType fonts,
/// rectangles are written as <c>re</c>, and a stroke states its pen width immediately
/// before its own path.
/// </para>
/// </remarks>
public sealed class PdfRenderer
{
    /// <summary>Creates a renderer with the given options.</summary>
    public PdfRenderer(PdfRenderOptions? options = null)
        => Options = options ?? PdfRenderOptions.Default;

    /// <summary>The options in effect.</summary>
    public PdfRenderOptions Options { get; }

    /// <summary>Writes every page of a sequence to a PDF stream.</summary>
    public void Render(IPageSequence pages, Stream destination) => Render(pages, destination, CancellationToken.None);

    /// <summary>
    /// Writes every page of a sequence to a PDF stream, checking <paramref name="cancellationToken"/> as it goes.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The file is written to <paramref name="destination"/> as it is produced — each page's content as
    /// that page is drawn, the fonts and the page tree at the end — so memory is bounded by one page and
    /// not by the document, and the stream need not be seekable. A temp-file stream is the natural
    /// destination for a large document.
    /// </para>
    /// <para>
    /// A cancelled render throws <see cref="OperationCanceledException"/> and leaves what it had written
    /// in the stream, which is not a valid PDF; the caller discards it.
    /// </para>
    /// </remarks>
    public void Render(IPageSequence pages, Stream destination, CancellationToken cancellationToken)
    {
        ArgumentNullException.ThrowIfNull(pages);

        Render(Enumerate(pages), destination, cancellationToken);
    }

    /// <summary>
    /// Writes every page of a sequence to a PDF stream, with the document's title, author, subject,
    /// keywords and language in the file's information dictionary and XMP packet.
    /// </summary>
    /// <param name="document">The document the pages were laid out from; its metadata is written.</param>
    /// <param name="pages">The laid-out pages.</param>
    /// <param name="destination">Where to write the file.</param>
    /// <param name="cancellationToken">Cancels the render.</param>
    public void Render(
        IDocument document, IPageSequence pages, Stream destination, CancellationToken cancellationToken = default)
    {
        ArgumentNullException.ThrowIfNull(document);
        ArgumentNullException.ThrowIfNull(pages);

        PdfRenderer renderer = new(Options with { Metadata = Options.Metadata ?? document.Metadata });
        IReadOnlyList<string>? titles = Options.WriteOutline && document.Family == DocumentFamily.Presentation
            ? SlideTitles(document.Content)
            : null;

        OfficeOperation.Run(() => renderer.RenderCore(Enumerate(pages), destination, titles), cancellationToken);
    }

    /// <summary>
    /// What each shown slide is called in the outline: its name when the deck gives one, otherwise the first
    /// line of its text, otherwise "Slide N".
    /// </summary>
    /// <remarks>
    /// A slide is a page and has no headings of its own the layout could report, so its entry is read from
    /// the extracted content, which is in the deck's own order. A hidden slide is not a page, so it is
    /// skipped here exactly as the page sequence skips it.
    /// </remarks>
    private static List<string> SlideTitles(ContentNode content)
    {
        List<string> titles = [];

        foreach (ContentNode child in content.Children)
        {
            if (child is not ContentSection { Kind: SectionKind.Slide } slide || slide.IsHidden) continue;

            string? title = PdfMetadata.Clean(slide.Name) is { Length: > 0 } named ? named : null;
            if (title is null)
            {
                foreach (string line in slide.GetText().Split('\n'))
                {
                    if (PdfMetadata.Clean(line) is not { Length: > 0 } cleaned) continue;
                    title = cleaned.Length > 120 ? cleaned[..120] : cleaned;
                    break;
                }
            }

            titles.Add(title ?? string.Create(CultureInfo.InvariantCulture, $"Slide {titles.Count + 1}"));
        }

        return titles;
    }

    /// <summary>
    /// Lays a document out and writes it to a PDF stream, in one call.
    /// </summary>
    /// <param name="document">The document to render.</param>
    /// <param name="destination">Where to write the file.</param>
    /// <param name="layout">Layout options, including <see cref="LayoutOptions.Timeout"/>; null for the defaults.</param>
    /// <param name="cancellationToken">Cancels the layout and the render.</param>
    public void RenderDocument(
        IPaginatedDocument document,
        Stream destination,
        LayoutOptions? layout = null,
        CancellationToken cancellationToken = default)
    {
        ArgumentNullException.ThrowIfNull(document);

        IPageSequence pages = document.Layout(layout, cancellationToken);
        Render(document, pages, destination, cancellationToken);
    }

    /// <summary>
    /// Writes a chosen set of pages to a PDF stream.
    /// </summary>
    /// <remarks>
    /// Taking the pages rather than a range so that a caller's page selection — the CLI's
    /// <c>--pages</c>, say — is applied once, where it can be reported, instead of being
    /// re-interpreted by every backend.
    /// </remarks>
    /// <param name="pages">The pages to write, in order.</param>
    /// <param name="destination">Where to write the file.</param>
    public void Render(IEnumerable<IPage> pages, Stream destination)
        => Render(pages, destination, CancellationToken.None);

    /// <summary>
    /// Writes a chosen set of pages to a PDF stream, checking <paramref name="cancellationToken"/> as it goes.
    /// </summary>
    /// <param name="pages">The pages to write, in order.</param>
    /// <param name="destination">Where to write the file.</param>
    /// <param name="cancellationToken">Cancels the render.</param>
    public void Render(IEnumerable<IPage> pages, Stream destination, CancellationToken cancellationToken)
    {
        ArgumentNullException.ThrowIfNull(pages);
        ArgumentNullException.ThrowIfNull(destination);

        OfficeOperation.Run(() => RenderCore(pages, destination, null), cancellationToken);
    }

    private static List<IPage> Enumerate(IPageSequence pages)
    {
        List<IPage> all = new(pages.Count);
        for (int i = 0; i < pages.Count; i++) all.Add(pages[i]);
        return all;
    }

    private bool RenderCore(IEnumerable<IPage> pages, Stream destination, IReadOnlyList<string>? pageTitles)
    {
        OfficeOperation.ThrowIfCancelled();

        PdfDocumentWriter writer = new(destination);
        PdfFontCatalogue fonts = new(
            Options.FontProvider ?? FileFontProvider.Instance, Options.EmbedFonts, Options);

        // Reserved before anything is drawn: a page names its resources, and so does a form XObject
        // written in the middle of a page, so the number has to exist before the content does.
        int resources = writer.Reserve();
        int tree = writer.Reserve();

        PdfContentSink sink = new(writer, fonts, Options) { ResourcesPlaceholder = resources };

        // What is kept of a page once it has been drawn: where its content went, which object it will
        // be, and what is clickable on it. The content itself is already in the file.
        List<int> pageIds = [];
        List<DocSize> sizes = [];
        List<int> contentIds = [];
        List<IReadOnlyList<PdfLink>> links = [];
        List<string?> labels = [];
        string? currentLabel = null;

        sink.PageEnded = content =>
        {
            int stream = writer.Reserve();
            writer.SetStream(stream, string.Empty, Encoding.Latin1.GetBytes(content.Content), compress: true);

            contentIds.Add(stream);
            pageIds.Add(writer.Reserve());
            sizes.Add(content.Size);
            links.Add(content.Links);
            labels.Add(currentLabel);
        };

        foreach (IPage page in pages)
        {
            OfficeOperation.ThrowIfCancelled();

            if (page is IPageRenderLimits limits)
            {
                sink.MaxImagePixels = limits.MaxImagePixels;
                sink.OversizedImages = limits.OversizedImages;
            }
            else
            {
                sink.MaxImagePixels = 0;
                sink.OversizedImages = OversizedImagePolicy.Skip;
            }

            currentLabel = page.Label;
            int before = sink.PagesEnded;
            page.Draw(sink);
            if (sink.HasUnclosedPage) sink.EndPage();

            // A deck's pages have no headings to draw an outline from, so each shown slide is entered under
            // the title taken from the content. See `SlideTitles`.
            if (pageTitles is not null && sink.PagesEnded == before + 1 && before < pageTitles.Count)
            {
                sink.Outline.Add(new PdfOutlineEntry(1, pageTitles[before], new PdfDestination(before, 0, sizes[before].Height.Points)));
            }
        }

        OfficeOperation.ThrowIfCancelled();

        DateTimeOffset created = Options.CreationDate ?? DateTimeOffset.Now;
        PdfMetadata.Facts facts = PdfMetadata.From(Options, created);

        string[] annotations = Options.WriteLinks
            ? PdfNavigation.WriteAnnotations(writer, pageIds, links, sink.Destinations)
            : new string[pageIds.Count];

        StringBuilder kids = new();
        for (int i = 0; i < pageIds.Count; i++)
        {
            DocSize size = sizes[i];
            writer.Set(
                pageIds[i],
                $"<</Type/Page/Parent {tree} 0 R/Resources {resources} 0 R"
                + $"/MediaBox[0 0 {PdfSyntax.Number(size.Width.Points)} {PdfSyntax.Number(size.Height.Points)}]"
                + $"/Contents {contentIds[i]} 0 R{annotations[i]}>>");

            kids.Append(CultureInfo.InvariantCulture, $"{pageIds[i]} 0 R ");
        }

        StringBuilder dictionary = new("<</ProcSet[/PDF/Text/ImageC/ImageB]");
        fonts.Write(writer, dictionary);
        sink.WriteResources(dictionary);
        writer.Set(resources, dictionary.Append(">>").ToString());

        writer.Set(tree,
            $"<</Type/Pages/Resources {resources} 0 R/Kids[{kids.ToString().TrimEnd()}]/Count {pageIds.Count}>>");

        StringBuilder catalogue = new($"<</Type/Catalog/Pages {tree} 0 R");

        if (Options.WriteXmp)
        {
            int metadata = writer.Reserve();
            writer.SetStream(metadata, "/Type/Metadata/Subtype/XML", PdfMetadata.Xmp(facts), compress: false);
            catalogue.Append(CultureInfo.InvariantCulture, $"/Metadata {metadata} 0 R");
        }

        if (facts.Language is not null) catalogue.Append("/Lang").Append(PdfSyntax.LiteralString(facts.Language));
        if (facts.Title is not null) catalogue.Append("/ViewerPreferences<</DisplayDocTitle true>>");

        if (Options.WriteOutline
            && PdfNavigation.WriteOutline(writer, sink.Outline, pageIds) is var outline and > 0)
        {
            catalogue.Append(CultureInfo.InvariantCulture, $"/Outlines {outline} 0 R/PageMode/UseOutlines");
        }

        if (Options.WriteLinks && PdfNavigation.WriteNames(writer, sink.Destinations, pageIds) is var names and > 0)
        {
            catalogue.Append(CultureInfo.InvariantCulture, $"/Names {names} 0 R");
        }

        if (Options.WritePageLabels) catalogue.Append(PdfNavigation.PageLabels(labels));

        int root = writer.Add(catalogue.Append(">>").ToString());
        int info = writer.Add(PdfMetadata.InfoDictionary(facts));

        writer.Finish(root, info, PdfMetadata.FileId(facts, writer.ObjectCount));
        return true;
    }
}

/// <summary>Options controlling PDF output.</summary>
public sealed record PdfRenderOptions
{
    /// <summary>The defaults.</summary>
    public static readonly PdfRenderOptions Default = new();

    /// <summary>Whether to embed subsetted copies of the fonts used.</summary>
    public bool EmbedFonts { get; init; } = true;

    /// <summary>
    /// Whether to pass JPEG images through without re-encoding. Faster and lossless,
    /// but produces a file that is not byte-comparable with a re-encoding writer.
    /// </summary>
    public bool PassThroughJpeg { get; init; } = true;

    /// <summary>
    /// A fixed creation timestamp, for reproducible output. When null, the current
    /// time is used. Set this in tests: it is the difference between a PDF that
    /// hashes the same on every run and one that does not.
    /// </summary>
    public DateTimeOffset? CreationDate { get; init; }

    /// <summary>
    /// Where the bytes of a face come from.
    /// </summary>
    /// <remarks>
    /// Defaults to <see cref="FileFontProvider"/>, which reads
    /// <c>FontReference.FaceKey</c> as a path — which is what the system font resolver
    /// produces. A caller with fonts somewhere else, or with a document's own embedded
    /// fonts to hand, supplies its own.
    /// </remarks>
    public IPdfFontProvider? FontProvider { get; init; }

    /// <summary>What the file names as its producer.</summary>
    public string Producer { get; init; } = "X-Ray.Office";

    /// <summary>
    /// The document's own metadata, written to the information dictionary and the XMP packet.
    /// </summary>
    /// <remarks>
    /// Set by <see cref="PdfRenderer.Render(IDocument, IPageSequence, Stream, CancellationToken)"/> and
    /// <see cref="PdfRenderer.RenderDocument"/>, which take it from the document; set it here when
    /// rendering a page sequence on its own. Null writes the producer and the date only.
    /// </remarks>
    public DocumentMetadata? Metadata { get; init; }

    /// <summary>
    /// Whether <see cref="Metadata"/> reaches the file's information dictionary: title, author, subject,
    /// keywords and the application that made the source document.
    /// </summary>
    public bool WriteDocumentInfo { get; init; } = true;

    /// <summary>
    /// Whether to write an XMP metadata stream beside the information dictionary. It carries the same
    /// facts, and is what a modern reader (and a PDF/A validator) looks at first.
    /// </summary>
    public bool WriteXmp { get; init; } = true;

    /// <summary>
    /// The document language for the catalogue's <c>/Lang</c>, as a BCP 47 tag. Null takes it from
    /// <see cref="Metadata"/>, and writes none when the document names none.
    /// </summary>
    public string? Language { get; init; }

    /// <summary>
    /// Whether links the pages declare are written: hyperlinks as <c>/Link</c> annotations with a
    /// <c>/URI</c> action, and links to bookmarks, cross-references and table-of-contents entries as
    /// annotations with a <c>/Dest</c>, along with a name tree of the destinations.
    /// </summary>
    public bool WriteLinks { get; init; } = true;

    /// <summary>
    /// Whether to write the document outline (the bookmarks panel): headings in a word-processing
    /// document, sheets in a workbook.
    /// </summary>
    public bool WriteOutline { get; init; } = true;

    /// <summary>
    /// Whether to write <c>/PageLabels</c> when the pages' printed numbers differ from their positions,
    /// as after a title page or a restarted section.
    /// </summary>
    public bool WritePageLabels { get; init; } = true;

    /// <summary>
    /// The largest picture, in pixels, that is decoded into the file; zero for no limit. The limit a
    /// page carries from <see cref="LayoutOptions.MaxImagePixels"/> applies as well, and the smaller wins.
    /// </summary>
    public long MaxImagePixels { get; init; }

    /// <summary>What happens to a picture over <see cref="MaxImagePixels"/>.</summary>
    public OversizedImagePolicy OversizedImages { get; init; }
}
