using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Rendering.Fills;

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
    /// The most tiles one fill will draw.
    /// </summary>
    /// <remarks>
    /// A one-point tile over an A4 page is half a million draws, which is a document doing
    /// something pathological rather than a fill worth reproducing exactly. At the cap the
    /// grid is drawn as far as it goes and the remainder is left unpainted, which is visible
    /// and therefore reportable — unlike stretching the tile, which would look deliberate.
    /// </remarks>
    public const int Maximum = 8192;

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

        int drawn = 0;
        for (Length y = grid.Origin.Y; y.Emu < region.Bottom.Emu; y += grid.Step.Height)
        {
            for (Length x = grid.Origin.X; x.Emu < region.Right.Emu; x += grid.Step.Width)
            {
                if (drawn++ >= Maximum) yield break;

                yield return new DocRect(x, y, grid.Step.Width, grid.Step.Height);
            }
        }
    }
}
