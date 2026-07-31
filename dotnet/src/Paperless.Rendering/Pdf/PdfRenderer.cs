using System.Globalization;
using System.Text;
using Paperless.Core.Documents;

namespace Paperless.Rendering.Pdf;

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
/// The fidelity harness in <c>tests/Paperless.TestKit/LibreOffice/</c> was written to read
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
    public void Render(IPageSequence pages, Stream destination)
    {
        ArgumentNullException.ThrowIfNull(pages);

        List<IPage> all = new(pages.Count);
        for (int i = 0; i < pages.Count; i++) all.Add(pages[i]);

        Render(all, destination);
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
    {
        ArgumentNullException.ThrowIfNull(pages);
        ArgumentNullException.ThrowIfNull(destination);

        PdfDocumentWriter writer = new();
        PdfFontCatalogue fonts = new(Options.FontProvider ?? FileFontProvider.Instance, Options.EmbedFonts);

        // Reserved before anything is drawn: a page names its resources, and so does a form XObject
        // written in the middle of a page, so the number has to exist before the content does.
        int resources = writer.Reserve();
        int tree = writer.Reserve();

        PdfContentSink sink = new(writer, fonts, Options) { ResourcesPlaceholder = resources };

        foreach (IPage page in pages) page.Draw(sink);
        if (sink.HasUnclosedPage) sink.EndPage();

        List<int> pageIds = [];
        StringBuilder kids = new();

        foreach (PdfPageContent content in sink.Pages)
        {
            int stream = writer.Reserve();
            writer.SetStream(stream, string.Empty, Encoding.Latin1.GetBytes(content.Content), compress: true);

            int id = writer.Add(
                $"<</Type/Page/Parent {tree} 0 R/Resources {resources} 0 R"
                + $"/MediaBox[0 0 {PdfSyntax.Number(content.Size.Width.Points)} "
                + $"{PdfSyntax.Number(content.Size.Height.Points)}]/Contents {stream} 0 R>>");

            pageIds.Add(id);
            kids.Append(CultureInfo.InvariantCulture, $"{id} 0 R ");
        }

        StringBuilder dictionary = new("<</ProcSet[/PDF/Text/ImageC/ImageB]");
        fonts.Write(writer, dictionary);
        sink.WriteResources(dictionary);
        writer.Set(resources, dictionary.Append(">>").ToString());

        writer.Set(tree,
            $"<</Type/Pages/Resources {resources} 0 R/Kids[{kids.ToString().TrimEnd()}]/Count {pageIds.Count}>>");

        int catalogue = writer.Add($"<</Type/Catalog/Pages {tree} 0 R>>");
        int info = writer.Add(
            $"<</Producer{PdfSyntax.LiteralString(Options.Producer)}"
            + $"/CreationDate{PdfSyntax.Date(Options.CreationDate ?? DateTimeOffset.Now)}>>");

        writer.Write(destination, catalogue, info);
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
    public string Producer { get; init; } = "Paperless";
}
