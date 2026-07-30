using Paperless.Core.Documents;

namespace Paperless.Rendering.Raster;

/// <summary>
/// Renders laid-out pages to raster images using SkiaSharp.
/// </summary>
/// <remarks>
/// Skia is the default backend because it is what LibreOffice's own headless path
/// effectively does (cairo, an equivalent library), because it is permissively
/// licensed, and because it handles the awkward parts — path clipping, gradients,
/// transparency groups — correctly without extra work.
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
    public void RenderToPng(IPage page, Stream destination) => throw new NotImplementedException();
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
}
