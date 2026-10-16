using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;

namespace XRay.Office.Rendering.Fills;

/// <summary>
/// Where the tiles of a <see cref="BitmapPaint"/> go.
/// </summary>
/// <remarks>
/// Shared so both backends lay the same grid: the PDF writer emits one image draw per
/// rectangle this yields, and the raster backend hands the same origin and step to a Skia
/// shader as its local matrix, so the two agree by construction rather than by inspection.
/// </remarks>
internal static class Tiles
{
    /// <summary>
    /// The most tiles one fill will draw over a region, derived from the region rather than
    /// chosen.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>The flat cap of 8192 that stood here truncated a real document's background.</strong>
    /// <c>redac-fullComm-201705-EE-FRs-briefing.pptx</c> takes its slide background from the third
    /// entry of the theme's <c>a:bgFillStyleLst</c>, a 5x5 texture tiled at 3.23 pt. Covering a
    /// 720x540 pt slide needs <strong>37 550</strong> tiles; the cap stopped at 8192, so the wash
    /// covered the top 22% of the slide and the rest came out pure white. That is not "visible and
    /// therefore reportable" -- it reads as a background that was never painted at all.
    /// </para>
    /// <para>
    /// LibreOffice imposes no limit at all: <c>GeoTexSvxTiled::iterateTiles</c>
    /// (<c>drawinglayer/source/texture/texture.cxx</c>:1009-1019) guards only against a zero-sized
    /// tile and emits one transform per cell, and its PDF of that slide duly carries all 37 550 of
    /// them. A ceiling is still worth keeping so that a degenerate grid cannot hang a rendering,
    /// but it has to come from the geometry rather than from a number: a tile smaller than a point
    /// cannot be told from its neighbour at the resolution a PDF's own coordinates are written in,
    /// so the ceiling is one tile per square point of the region being filled. The theme texture
    /// above is 37 550 of a possible 388 800, and every legitimate fill is far below it.
    /// </para>
    /// </remarks>
    /// <param name="region">The region being filled.</param>
    public static long Maximum(DocRect region)
        => (long)Math.Max(1, Math.Ceiling(region.Width.Points))
           * (long)Math.Max(1, Math.Ceiling(region.Height.Points));

    /// <summary>
    /// The origin and step of the tile grid covering a region.
    /// </summary>
    /// <remarks>
    /// The grid is anchored on <see cref="BitmapPaint.TileOffset"/> and walked backwards to
    /// the first tile that still touches the region, so moving the offset by exactly one tile
    /// leaves the picture unchanged — which is what makes an offset a phase rather than a
    /// translation.
    /// </remarks>
    public static (DocPoint Origin, DocSize Step)? Grid(BitmapPaint bitmap, DocRect region)
    {
        ArgumentNullException.ThrowIfNull(bitmap);

        long stepX = bitmap.TileSize.Width.Emu;
        long stepY = bitmap.TileSize.Height.Emu;
        if (stepX <= 0 || stepY <= 0 || region.Width.Emu <= 0 || region.Height.Emu <= 0) return null;

        return (new DocPoint(
                Length.FromEmu(Anchor(bitmap.TileOffset.X.Emu, stepX, region.Left.Emu)),
                Length.FromEmu(Anchor(bitmap.TileOffset.Y.Emu, stepY, region.Top.Emu))),
            new DocSize(Length.FromEmu(stepX), Length.FromEmu(stepY)));

        static long Anchor(long offset, long step, long edge)
        {
            long phase = ((offset % step) + step) % step;
            long start = edge - (((edge % step) + step) % step) + phase;
            return start > edge ? start - step : start;
        }
    }

    /// <summary>
    /// Every tile rectangle needed to cover a region, in drawing order.
    /// </summary>
    /// <remarks>
    /// A stretched paint yields exactly one rectangle — the region itself — because
    /// <see cref="BitmapPaint.Stretch"/> means "once across the whole thing", and expressing
    /// that as a degenerate grid keeps both backends on one code path.
    /// </remarks>
    public static IEnumerable<DocRect> Cover(BitmapPaint bitmap, DocRect region)
    {
        ArgumentNullException.ThrowIfNull(bitmap);

        if (bitmap.Stretch)
        {
            if (!region.IsEmpty) yield return region;
            yield break;
        }

        if (Grid(bitmap, region) is not { } grid) yield break;

        long drawn = 0;
        long maximum = Maximum(region);
        for (Length y = grid.Origin.Y; y.Emu < region.Bottom.Emu; y += grid.Step.Height)
        {
            for (Length x = grid.Origin.X; x.Emu < region.Right.Emu; x += grid.Step.Width)
            {
                if (drawn++ >= maximum) yield break;

                yield return new DocRect(x, y, grid.Step.Width, grid.Step.Height);
            }
        }
    }
}
