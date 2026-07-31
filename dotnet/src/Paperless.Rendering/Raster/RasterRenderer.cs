using Paperless.Core.Documents;
using Paperless.Core.Units;
using SkiaSharp;

namespace Paperless.Rendering.Raster;

/// <summary>
/// Renders laid-out pages to raster images using SkiaSharp.
/// </summary>
/// <remarks>
/// <para>
/// Skia is the default backend because it is what LibreOffice's own headless path
/// effectively does (cairo, an equivalent library), because it is permissively
/// licensed, and because it handles the awkward parts — path clipping, gradients,
/// transparency groups — correctly without extra work.
/// </para>
/// <para>
/// <b>PNG is the format that matters and JPEG is not.</b> PNG is lossless and Skia's
/// encoder is deterministic, so a checksum of a page image means something and a golden
/// image can be committed. JPEG exists for callers who want a thumbnail; it must not be
/// used for any comparison, because its quantisation moves every pixel a little and would
/// turn a rendering difference into a question about the encoder.
/// </para>
/// </remarks>
public sealed class RasterRenderer
{
    /// <summary>Creates a renderer with the given options.</summary>
    public RasterRenderer(RasterRenderOptions? options = null)
        => Options = options ?? RasterRenderOptions.Default;

    /// <summary>The options in effect.</summary>
    public RasterRenderOptions Options { get; }

    /// <summary>
    /// Renders one page and writes it to a stream as PNG.
    /// </summary>
    public void RenderToPng(IPage page, Stream destination) => Render(page, destination, RasterFormat.Png);

    /// <summary>
    /// Renders one page and writes it to a stream in the given format.
    /// </summary>
    /// <param name="page">The page to draw.</param>
    /// <param name="destination">Where to write the encoded image.</param>
    /// <param name="format">Which encoding to use.</param>
    public void Render(IPage page, Stream destination, RasterFormat format)
    {
        ArgumentNullException.ThrowIfNull(page);
        ArgumentNullException.ThrowIfNull(destination);

        using SKBitmap bitmap = Rasterise(page);
        using SKData encoded = bitmap.Encode(
            format == RasterFormat.Jpeg ? SKEncodedImageFormat.Jpeg : SKEncodedImageFormat.Png,
            format == RasterFormat.Jpeg ? Options.JpegQuality : 100)
            ?? throw new InvalidOperationException("Skia could not encode the page image.");

        encoded.SaveTo(destination);
    }

    /// <summary>
    /// Draws a page into a bitmap at the configured resolution.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Opaque white, not transparent: LibreOffice's PNG and PDF export both produce a white
    /// page, and a transparent one compares as a solid difference against either.
    /// </para>
    /// </remarks>
    public SKBitmap Rasterise(IPage page)
    {
        ArgumentNullException.ThrowIfNull(page);

        (int width, int height) = PixelSize(page.Size);
        SKBitmap bitmap = new(new SKImageInfo(width, height, SKColorType.Rgba8888, SKAlphaType.Premul));
        using SKCanvas canvas = new(bitmap);

        canvas.Clear(new SKColor(
            Options.Background.R, Options.Background.G, Options.Background.B, Options.Background.A));

        using SkiaDrawingSink sink = new(canvas, Options);
        page.Draw(sink);
        canvas.Flush();

        return bitmap;
    }

    /// <summary>
    /// How many pixels wide and tall a page comes out at the current resolution.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <b>Rounded up, not to nearest</b>, which is measured rather than chosen. The
    /// comparison this has to survive is against <c>pdftoppm -r 150</c>, and poppler takes
    /// the ceiling: an A4 page 595.304 pt wide is 1240.22 pixels at 150 dpi and it produces
    /// 1241, where rounding to nearest gives 1240. The image comparison in the
    /// <c>render-comparison</c> skill stops at <c>DIMENSION MISMATCH</c> before measuring
    /// anything, so one pixel of disagreement here costs the whole comparison. Confirmed on
    /// both axes of the same page: 841.89 pt is 1753.94 pixels and both rules give 1754.
    /// </para>
    /// <para>
    /// The epsilon is against a page whose size is a whole number of pixels: 8.5 inches at
    /// 72 dpi is 612 exactly in arithmetic and 612.0000000001 in binary floating point, and
    /// a bare ceiling would make it 613.
    /// </para>
    /// <para>
    /// Exposed because the first thing a comparison needs to agree on is the size of the two
    /// images, and a caller that wants to check that before rendering should not have to
    /// rasterise to find out.
    /// </para>
    /// </remarks>
    public (int Width, int Height) PixelSize(Core.Geometry.DocSize size) => (
        Pixels(size.Width.Inches * Options.Dpi),
        Pixels(size.Height.Inches * Options.Dpi));

    private static int Pixels(double exact) => Math.Max(1, (int)Math.Ceiling(exact - 1e-6));
}

/// <summary>The raster encodings the renderer can write.</summary>
public enum RasterFormat
{
    /// <summary>Lossless and deterministic. The only one a comparison should use.</summary>
    Png = 0,

    /// <summary>Lossy, for thumbnails. Never for comparison.</summary>
    Jpeg,
}

/// <summary>Options controlling raster output.</summary>
public sealed record RasterRenderOptions
{
    /// <summary>The defaults: 150 DPI, antialiased, on a white background.</summary>
    public static readonly RasterRenderOptions Default = new();

    /// <summary>
    /// Output resolution in dots per inch. 150 is a good default for comparison work:
    /// high enough that text differences are visible, low enough to stay fast.
    /// </summary>
    public double Dpi { get; init; } = 150;

    /// <summary>Whether to antialias geometry and text.</summary>
    public bool Antialias { get; init; } = true;

    /// <summary>
    /// The colour to clear each page to before drawing. Opaque white matches what
    /// LibreOffice's own PDF and PNG export produce, so leave it alone when comparing.
    /// </summary>
    public Core.Graphics.Colour Background { get; init; } = Core.Graphics.Colour.White;

    /// <summary>
    /// JPEG quality from 1 to 100. Ignored for PNG, which is always lossless.
    /// </summary>
    public int JpegQuality { get; init; } = 85;

    /// <summary>
    /// Whether to draw glyphs from their outlines rather than from Skia's glyph cache.
    /// </summary>
    /// <remarks>
    /// On by default, and the reason is a measurement rather than a preference: the glyph
    /// cache places a glyph mask at a whole pixel vertically, which moved a page's ink down
    /// by 0.56 px — a quarter of a point — at 150 dpi and made the comparison script report
    /// a reflow cascade on a page whose layout was exact. Turn it off for speed when the
    /// output is a thumbnail rather than something to be compared.
    /// </remarks>
    public bool GlyphOutlines { get; init; } = true;
}
