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
/// which LibreOffice itself uses and which HarfBuzzSharp exposes.
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
    public void Render(IPageSequence pages, Stream destination) => throw new NotImplementedException();
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
}
