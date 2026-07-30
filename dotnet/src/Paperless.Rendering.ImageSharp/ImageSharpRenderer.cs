using Paperless.Core.Documents;

namespace Paperless.Rendering.ImageSharp;

/// <summary>
/// An alternative fully-managed raster backend built on ImageSharp.
/// </summary>
/// <remarks>
/// <para>
/// Exists for deployments that cannot ship SkiaSharp's native binaries — for instance
/// a fully-managed or trimmed single-file build, or a platform Skia has no build for.
/// </para>
/// <para>
/// Two things to know before choosing it. Its output is <em>not</em> pixel-identical to
/// the Skia backend, because the two rasterise antialiasing differently, so golden
/// images are per-backend. And ImageSharp v2 and later are under the Six Labors Split
/// Licence rather than Apache-2.0, which is why this lives in a separate opt-in
/// package that no other Paperless library references.
/// </para>
/// </remarks>
public sealed class ImageSharpRenderer
{
    /// <summary>Renders one page and writes it to a stream as PNG.</summary>
    public void RenderToPng(IPage page, Stream destination) => throw new NotImplementedException();
}
