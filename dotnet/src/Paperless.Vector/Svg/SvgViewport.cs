using ShimSkiaSharp;
using Svg;

namespace Paperless.Vector.Svg;

/// <summary>
/// The size an outermost <c>&lt;svg&gt;</c> asks to be drawn at, computed LibreOffice's way.
/// </summary>
/// <remarks>
/// <para>
/// <b>This is where scaling goes wrong, and the disagreement is real rather than theoretical.</b>
/// An SVG carries three statements of size — <c>width</c>, <c>height</c> and <c>viewBox</c> —
/// and the interesting case is when only one of the first two is given. For
/// <c>&lt;svg width="100" viewBox="0 0 200 50"&gt;</c>, LibreOffice derives the missing
/// height from the view box's aspect ratio and gets a 100×25 viewport
/// (<c>svgio/source/svgreader/svgsvgnode.cxx:504-516</c>: <c>fH = fW / fViewBoxRatio</c>).
/// <c>Svg.SceneGraph</c> 5.1.1 instead keeps the view box's own height and gets 100×50 —
/// measured, not inferred.
/// </para>
/// <para>
/// The difference is not academic. At 100×50 the <c>xMidYMid meet</c> fit letterboxes the
/// drawing into the middle half of the frame; at 100×25 it fills it. Same file, same frame,
/// picture half the size and floating.
/// </para>
/// <para>
/// Rather than re-implementing the view-box mapping, the computed viewport is written back
/// onto the document before the scene is compiled. The library then produces exactly the
/// mapping LibreOffice would, and its own rule never runs. <see cref="Impose"/> explains why
/// what is written back is not simply the physical size in pixels.
/// </para>
/// </remarks>
internal static class SvgViewport
{
    /// <summary>CSS pixels per inch, which is what an SVG user unit is defined against.</summary>
    private const double PixelsPerInch = 96.0;

    /// <summary>
    /// The default font size a root-level <c>em</c> or <c>ex</c> resolves against.
    /// </summary>
    /// <remarks>
    /// CSS's <c>medium</c>. Only reachable when the root element states its own width in font
    /// units, which is vanishingly rare and still better answered than treated as zero.
    /// </remarks>
    private const double RootFontSizePixels = 16.0;

    /// <summary>The viewport in SVG user units, or null when the document renders nothing.</summary>
    /// <param name="fragment">The parsed root element.</param>
    public static SKSize? Measure(SvgFragment fragment)
    {
        ArgumentNullException.ThrowIfNull(fragment);

        // "Svg defines that a negative value is an error and that 0.0 disables rendering"
        // — svgsvgnode.cxx:487-489.
        if (IsSet(fragment.Width) && fragment.Width.Value <= 0) return null;
        if (IsSet(fragment.Height) && fragment.Height.Value <= 0) return null;

        double? width = Absolute(fragment.Width);
        double? height = Absolute(fragment.Height);
        SvgViewBox box = fragment.ViewBox;
        bool hasBox = box.Width > 0 && box.Height > 0;

        if (!hasBox)
        {
            // No view box means no mapping, so the viewport is whatever the attributes say.
            // With neither stated there is no parent to resolve a percentage against and
            // LibreOffice falls back to the content's own bounds; the library does the same,
            // so leaving it alone is the right answer rather than an omission.
            return width is { } w && height is { } h ? new SKSize((float)w, (float)h) : null;
        }

        double ratio = box.Width / box.Height;

        return (width, height) switch
        {
            ({ } w, { } h) => new SKSize((float)w, (float)h),
            ({ } w, null) => new SKSize((float)w, (float)(w / ratio)),
            (null, { } h) => new SKSize((float)(h * ratio), (float)h),

            // Neither stated: the view box is the viewport, so the mapping is the identity.
            _ => new SKSize(box.Width, box.Height),
        };
    }

    /// <summary>
    /// Rewrites a document's <c>width</c> and <c>height</c> so the library computes the
    /// viewport LibreOffice would.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Not the physical size in pixels, and the reason is a trap that cost real time.
    /// <c>Svg.SceneGraph</c> rounds the viewport it computes to whole pixels, and it rounds
    /// each dimension independently: 120 × 80 mm is 453.543 × 302.362 px, which rounds to
    /// 454 × 302 — an aspect ratio of 1.5033 where the <c>viewBox</c>'s is 1.5000. The
    /// <c>xMidYMid meet</c> fit then dutifully pillarboxes the drawing by half a pixel on
    /// each side, and the picture comes out 119.74 mm wide in a 120 mm frame. Nothing about
    /// that looks like a rounding bug; it looks like a scaling bug, and the SVG's own file is
    /// exact.
    /// </para>
    /// <para>
    /// So when there is a view box the viewport is expressed in the view box's own units
    /// instead — <c>(viewBoxHeight × aspect, viewBoxHeight)</c> — which is the view box
    /// exactly whenever the two ratios agree, and therefore rounds to itself. The absolute
    /// size is not lost by this: it lives in
    /// <see cref="VectorImage.IntrinsicSize"/>, and the display list is stretched onto the
    /// frame regardless, so only the <em>ratio</em> has to survive this step.
    /// </para>
    /// </remarks>
    /// <param name="fragment">The parsed root element, modified in place.</param>
    /// <param name="viewport">The physical viewport from <see cref="Measure"/>.</param>
    public static void Impose(SvgFragment fragment, SKSize viewport)
    {
        ArgumentNullException.ThrowIfNull(fragment);

        SvgViewBox box = fragment.ViewBox;
        SKSize imposed = viewport;

        if (box.Width > 0 && box.Height > 0 && viewport.Height > 0)
        {
            imposed = new SKSize(box.Height * viewport.Width / viewport.Height, box.Height);
        }

        fragment.Width = new SvgUnit(SvgUnitType.Pixel, imposed.Width);
        fragment.Height = new SvgUnit(SvgUnitType.Pixel, imposed.Height);
    }

    private static bool IsSet(SvgUnit unit) => !unit.IsEmpty && !unit.IsNone;

    /// <summary>
    /// A length in SVG user units, or null when it is a percentage or absent.
    /// </summary>
    /// <remarks>
    /// A percentage at the root has nothing to resolve against — there is no parent viewport —
    /// which is why LibreOffice treats "absolute" and "usable" as the same question here
    /// (<c>svgsvgnode.cxx:504-505</c>).
    /// </remarks>
    private static double? Absolute(SvgUnit unit)
    {
        if (!IsSet(unit)) return null;

        return unit.Type switch
        {
            SvgUnitType.Percentage => null,
            SvgUnitType.Point => unit.Value * PixelsPerInch / 72.0,
            SvgUnitType.Pica => unit.Value * PixelsPerInch / 6.0,
            SvgUnitType.Centimeter => unit.Value * PixelsPerInch / 2.54,
            SvgUnitType.Millimeter => unit.Value * PixelsPerInch / 25.4,
            SvgUnitType.Inch => unit.Value * PixelsPerInch,
            SvgUnitType.Em => unit.Value * RootFontSizePixels,
            SvgUnitType.Ex => unit.Value * RootFontSizePixels / 2.0,
            _ => unit.Value,
        };
    }
}
