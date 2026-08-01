using Paperless.Core.Diagnostics;
using Svg.Model;
using ShimSkiaSharp;

namespace Paperless.Vector.Svg;

/// <summary>
/// The only door between the SVG library and the outside world, deliberately nailed shut.
/// </summary>
/// <remarks>
/// <para>
/// <b>A document parser that fetches URLs is an exfiltration channel.</b> This is not
/// theoretical here: with the library's stock settings, an <c>&lt;image
/// href="file:///etc/passwd"&gt;</c> in an SVG caused <c>LoadImage</c> to be called with
/// 1 238 bytes of that file's contents — measured against <c>Svg.SceneGraph</c> 5.1.1 on
/// this machine. A picture in a spreadsheet e-mailed to a victim would have read a local
/// file and then had it drawn onto the page.
/// </para>
/// <para>
/// Two independent gates therefore stand between an <c>href</c> and any I/O.
/// <c>SvgProcessingMode.SecureStatic</c> with
/// <c>SvgExternalResourcePolicy.SameDocumentAndDataOnly</c> stops the library resolving
/// anything but a <c>data:</c> URI or a same-document fragment — verified: with it set, the
/// same file produced no <c>LoadImage</c> call at all. And this loader refuses any URI that
/// is not a <c>data:</c> URI on its own account, so a future change of the library's default
/// cannot quietly reopen the hole.
/// </para>
/// <para>
/// Images are handed on <em>undecoded</em>. Only the dimensions are read, from the header
/// (<see cref="EncodedImageSize"/>), because placement needs the aspect ratio and nothing
/// else here needs pixels. That keeps <c>Paperless.Vector</c> free of a codec, which is the
/// same reason <c>RasterImage.Encoded</c> exists.
/// </para>
/// </remarks>
internal sealed class SvgAssetLoader : ISvgAssetLoader, ISvgImageAssetLoader, ISvgTextRunTypefaceResolver
{
    private readonly SvgTextEngine _text;
    private readonly VectorLimits _limits;
    private readonly List<Diagnostic> _diagnostics;

    /// <summary>Creates a loader over a text engine.</summary>
    /// <param name="text">Resolves, measures and shapes text.</param>
    /// <param name="limits">Caps the size of an embedded raster.</param>
    /// <param name="diagnostics">Collects what was refused or could not be read.</param>
    public SvgAssetLoader(SvgTextEngine text, VectorLimits limits, List<Diagnostic> diagnostics)
    {
        _text = text;
        _limits = limits;
        _diagnostics = diagnostics;
    }

    /// <summary>
    /// SVG fonts are off: a <c>&lt;font&gt;</c> element defines glyph outlines inside the
    /// document, which would bypass the resolver and shaper everything else goes through.
    /// </summary>
    public bool EnableSvgFonts => false;

    /// <inheritdoc/>
    public SKImage LoadImage(Stream stream)
    {
        ArgumentNullException.ThrowIfNull(stream);

        byte[]? bytes = Read(stream);
        if (bytes is null) return null!;

        if (EncodedImageSize.Read(bytes) is not { } size)
        {
            _diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Warning,
                "PL6003",
                "An image embedded in an SVG is in a format whose dimensions could not be read; it was skipped."));
            return null!;
        }

        // Width and Height are floats on the shim, but they are pixel counts: the library
        // divides by them to fit the picture into the element's box.
        return new SKImage { Data = bytes, Width = size.Width, Height = size.Height };
    }

    /// <inheritdoc/>
    public SKImage LoadImage(Stream stream, SvgImageLoadContext context)
    {
        if (context.ResourceUri is { IsAbsoluteUri: true } uri
            && !string.Equals(uri.Scheme, "data", StringComparison.OrdinalIgnoreCase))
        {
            _diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Warning,
                "PL6004",
                $"An SVG referenced an external image ('{uri.Scheme}:'); Paperless resolves no reference outside the document."));
            return null!;
        }

        return LoadImage(stream);
    }

    /// <inheritdoc/>
    public List<TypefaceSpan> FindTypefaces(string? text, SKPaint? paintPreferredTypeface)
    {
        // One span for the whole run: per-character fallback would need the library to hand
        // back several faces, and Paperless resolves fallback inside its own shaper instead.
        if (string.IsNullOrEmpty(text)) return [];

        return [new TypefaceSpan(text, _text.Measure(text, paintPreferredTypeface), paintPreferredTypeface?.Typeface)];
    }

    /// <inheritdoc/>
    public SKTypeface? FindRunTypeface(string? text, SKPaint? paintPreferredTypeface)
        => paintPreferredTypeface?.Typeface;

    /// <inheritdoc/>
    public SKFontMetrics GetFontMetrics(SKPaint? paint) => _text.Metrics(paint);

    /// <inheritdoc/>
    public float MeasureText(string? text, SKPaint? paint, ref SKRect bounds)
    {
        float width = _text.Measure(text, paint);
        SKFontMetrics metrics = _text.Metrics(paint);

        bounds = new SKRect(0, metrics.Ascent, width, metrics.Descent);
        return width;
    }

    /// <summary>
    /// Text as an outline, which Paperless does not produce.
    /// </summary>
    /// <remarks>
    /// The library asks for this only when text is used as a clip path or a mask. Returning an
    /// empty path clips everything away rather than clipping nothing, which is the safer of
    /// the two wrong answers: a missing shape is visible as missing, whereas an unclipped one
    /// silently paints over its surroundings. Producing real outlines needs a glyph
    /// rasteriser, which lives in <c>Paperless.Rendering</c> and must not be depended on here.
    /// </remarks>
    public SKPath GetTextPath(string? text, SKPaint? paint, float x, float y)
    {
        if (!string.IsNullOrEmpty(text))
        {
            _diagnostics.Add(new Diagnostic(
                DiagnosticSeverity.Warning,
                "PL6005",
                "An SVG used text as a clip path or mask; Paperless does not convert glyphs to outlines."));
        }

        return new SKPath();
    }

    private byte[]? Read(Stream stream)
    {
        using MemoryStream buffer = new();
        byte[] chunk = new byte[64 * 1024];
        long total = 0;

        while (true)
        {
            int read = stream.Read(chunk, 0, chunk.Length);
            if (read <= 0) break;

            total += read;
            if (total > _limits.MaxBytes)
            {
                _diagnostics.Add(new Diagnostic(
                    DiagnosticSeverity.Warning,
                    "PL6002",
                    $"An image embedded in an SVG exceeded the {_limits.MaxBytes}-byte limit and was skipped."));
                return null;
            }

            buffer.Write(chunk, 0, read);
        }

        return total == 0 ? null : buffer.ToArray();
    }
}
