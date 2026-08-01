using System.IO.Compression;
using System.Text;
using Paperless.Core.Diagnostics;
using Paperless.Core.Geometry;
using Paperless.Core.Units;
using ShimSkiaSharp;
using Svg;
using Svg.Model;
using Svg.Model.Services;
using Svg.Skia;

namespace Paperless.Vector.Svg;

/// <summary>
/// Decodes SVG and SVGZ into the Paperless display list.
/// </summary>
/// <remarks>
/// <para>
/// SVG is the one vector format Paperless does not parse itself. <c>Svg.SceneGraph</c> plus
/// <c>Svg.Model</c> already implement the CSS cascade, the path grammar, gradients, clip
/// paths, masks and filters, and — crucially — compile them to a device-independent canvas
/// command list rather than to pixels. Hand-rolling that would have bought no fidelity, since
/// the command list is what a hand-rolled parser would have had to produce anyway. See
/// <c>src/Paperless.Vector/TODO.md</c> for the full argument and
/// <c>Directory.Packages.props</c> for the licence position.
/// </para>
/// <para>
/// What Paperless keeps for itself is the three things a library cannot be trusted with in a
/// document parser: the security posture (<see cref="SvgAssetLoader"/>), the work bound
/// (<see cref="SvgExpansionGuard"/> and <see cref="VectorLimits"/>), and the size rule
/// (<see cref="SvgViewport"/>, which differs from the library's and follows LibreOffice's).
/// </para>
/// </remarks>
public sealed class SvgImageDecoder : IVectorImageDecoder
{
    /// <summary>The media types an SVG arrives under.</summary>
    /// <remarks>
    /// <c>image/svg+xml</c> is the registered one and the only one OOXML and ODF write.
    /// The <c>+xml-compressed</c> spelling is what a gzipped <c>.svgz</c> is sometimes
    /// labelled; the bytes are sniffed regardless, so the label is only ever a hint.
    /// </remarks>
    public IReadOnlyCollection<string> SupportedMediaTypes { get; } =
        ["image/svg+xml", "image/svg+xml-compressed"];

    /// <inheritdoc/>
    public bool CanDecode(ReadOnlySpan<byte> data)
    {
        if (data.Length < 4) return false;

        // Gzip: a .svgz, which every producer that writes one leaves compressed in the package.
        if (data[0] == 0x1F && data[1] == 0x8B) return true;

        // Otherwise look for the root element in the first stretch of the file, past whatever
        // byte-order mark, XML declaration, doctype and comments precede it. Sniffing rather
        // than trusting the declared media type is the tree's rule for whole documents and
        // holds just as well for the pictures inside them.
        int length = Math.Min(data.Length, 4096);
        string head = Decode(data[..length]);

        return head.Contains("<svg", StringComparison.OrdinalIgnoreCase)
            || head.Contains("http://www.w3.org/2000/svg", StringComparison.Ordinal);
    }

    /// <inheritdoc/>
    public VectorImage Decode(ReadOnlyMemory<byte> data, VectorLimits? limits = null)
    {
        VectorLimits caps = limits ?? VectorLimits.Default;
        List<Diagnostic> diagnostics = [];

        if (data.Length > caps.MaxBytes)
        {
            return Failed(diagnostics, "PL6002", $"An SVG of {data.Length} bytes exceeded the {caps.MaxBytes}-byte limit.");
        }

        string? xml = Source(data.Span, caps, diagnostics);
        if (xml is null) return VectorImage.Empty with { Diagnostics = diagnostics };

        switch (SvgExpansionGuard.Inspect(xml, caps))
        {
            case SvgExpansionGuard.Verdict.TooManyExpandedNodes:
                return Failed(
                    diagnostics,
                    "PL6001",
                    $"An SVG's <use> references expand to more than {caps.MaxExpandedNodes} nodes; it was not drawn.");

            case SvgExpansionGuard.Verdict.Cyclic:
                return Failed(diagnostics, "PL6001", "An SVG contains a cyclic <use> reference; it was not drawn.");

            case SvgExpansionGuard.Verdict.TooDeep:
                return Failed(diagnostics, "PL6001", "An SVG nests elements more deeply than Paperless will read.");

            default:
                break;
        }

        SvgDocument? document;
        try
        {
            document = SvgService.FromSvg(xml, Parameters, captureCompatibilityStyleState: false);
        }
        catch (Exception exception) when (exception is not OutOfMemoryException and not StackOverflowException)
        {
            return Failed(diagnostics, "PL6000", $"An SVG could not be parsed: {exception.Message}");
        }

        if (document is null) return Failed(diagnostics, "PL6000", "An SVG could not be parsed.");

        if (SvgViewport.Measure(document) is not { Width: > 0, Height: > 0 } viewport)
        {
            return Failed(diagnostics, "PL6000", "An SVG states no usable width, height or viewBox.");
        }

        SvgViewport.Impose(document, viewport);

        SvgTextEngine text = new();
        SvgAssetLoader assets = new(text, caps, diagnostics);
        DisplayList list = new();

        SKPicture? picture;
        try
        {
            picture = SvgSceneRuntime.CreateModel(document, assets, DrawAttributes.None);
        }
        catch (Exception exception) when (exception is not OutOfMemoryException and not StackOverflowException)
        {
            return Failed(diagnostics, "PL6000", $"An SVG could not be compiled: {exception.Message}");
        }

        SvgPictureTranslator translator = new(list, text, caps, diagnostics);
        translator.Translate(picture);

        // The view box comes from the compiled picture rather than from the computed viewport
        // so that it agrees exactly with the viewBox mapping the library baked into the
        // commands; the intrinsic size comes from the computed viewport, which is the
        // unrounded answer and the one a caller with no frame extent should be given.
        DocRect box = picture is null ? DocRect.Empty : ShimGeometry.Rect(picture.CullRect);
        if (box.IsEmpty) box = new DocRect(DocPoint.Origin, Size(viewport));

        return new VectorImage
        {
            Content = list,
            ViewBox = box,
            IntrinsicSize = Size(viewport),
            Diagnostics = diagnostics,
            IsTruncated = translator.IsTruncated,
        };
    }

    /// <summary>
    /// How the document is parsed: statically, with nothing outside it resolvable.
    /// </summary>
    /// <remarks>
    /// <c>SecureStatic</c> turns off scripting and animation — a static export shows the
    /// initial state, which is what these two settings produce together — and
    /// <c>SameDocumentAndDataOnly</c> is what stops an <c>href</c> reaching the filesystem or
    /// the network. That second one is load-bearing: without it, an
    /// <c>&lt;image href="file:///etc/passwd"&gt;</c> caused the file's bytes to be read and
    /// handed to the asset loader. Measured, on this machine, against version 5.1.1.
    /// </remarks>
    private static SvgParameters Parameters => new(
        null,
        null,
        null,
        new SvgDocumentLoadOptions
        {
            ProcessingMode = SvgProcessingMode.SecureStatic,
            ExternalResources = SvgExternalResourcePolicy.SameDocumentAndDataOnly,
        });

    private static DocSize Size(SKSize viewport)
        => new(ShimGeometry.Emu(viewport.Width), ShimGeometry.Emu(viewport.Height));

    private static VectorImage Failed(List<Diagnostic> diagnostics, string code, string message)
    {
        diagnostics.Add(new Diagnostic(DiagnosticSeverity.Warning, code, message));
        return VectorImage.Empty with { Diagnostics = diagnostics };
    }

    /// <summary>
    /// The document's text, decompressing an SVGZ on the way.
    /// </summary>
    /// <remarks>
    /// The expansion cap applies to the <em>decompressed</em> size, for the same reason the
    /// ZIP reader caps uncompressed bytes rather than compressed ones: a gzip stream is as
    /// capable of a thousandfold expansion as a ZIP entry is.
    /// </remarks>
    private static string? Source(ReadOnlySpan<byte> data, VectorLimits limits, List<Diagnostic> diagnostics)
    {
        if (data.Length < 2 || data[0] != 0x1F || data[1] != 0x8B) return Decode(data);

        try
        {
            using MemoryStream compressed = new(data.ToArray());
            using GZipStream gzip = new(compressed, CompressionMode.Decompress);
            using MemoryStream plain = new();

            byte[] chunk = new byte[64 * 1024];
            long total = 0;

            while (true)
            {
                int read = gzip.Read(chunk, 0, chunk.Length);
                if (read <= 0) break;

                total += read;
                if (total > limits.MaxBytes)
                {
                    diagnostics.Add(new Diagnostic(
                        DiagnosticSeverity.Warning,
                        "PL6002",
                        $"An SVGZ expanded past the {limits.MaxBytes}-byte limit; it was not drawn."));
                    return null;
                }

                plain.Write(chunk, 0, read);
            }

            return Decode(plain.GetBuffer().AsSpan(0, (int)plain.Length));
        }
        catch (InvalidDataException exception)
        {
            diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Warning, "PL6000", $"An SVGZ could not be decompressed: {exception.Message}"));
            return null;
        }
    }

    /// <summary>
    /// The bytes as text, honouring a byte-order mark.
    /// </summary>
    /// <remarks>
    /// UTF-8 without one, which is what the XML specification says to assume and what every
    /// office producer writes. A UTF-16 SVG is legal and does turn up, and its mark is the
    /// only thing that distinguishes it before the declaration has been read.
    /// </remarks>
    private static string Decode(ReadOnlySpan<byte> data)
    {
        if (data.Length >= 3 && data[0] == 0xEF && data[1] == 0xBB && data[2] == 0xBF)
        {
            return Encoding.UTF8.GetString(data[3..]);
        }

        if (data.Length >= 2 && data[0] == 0xFF && data[1] == 0xFE) return Encoding.Unicode.GetString(data[2..]);
        if (data.Length >= 2 && data[0] == 0xFE && data[1] == 0xFF) return Encoding.BigEndianUnicode.GetString(data[2..]);

        return Encoding.UTF8.GetString(data);
    }
}
