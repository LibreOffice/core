using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Vector.Metafiles;

/// <summary>
/// The ternary raster operations a metafile blits with, and the one of them that has to be
/// resolved rather than reported.
/// </summary>
/// <remarks>
/// <para>
/// A ternary raster operation combines three inputs — the source bitmap, the destination
/// already on the page, and the current brush — under one of 256 boolean functions, named by
/// the middle byte of the operation code. A display list can read none of them back, so most
/// of the 256 are genuinely out of reach and are reported (<c>PL6033</c>).
/// </para>
/// <para>
/// <b>One of them is not, and it is the one that matters.</b> "Transparent bitmap" has no
/// record in either format: a producer says it by blitting a monochrome mask with
/// <c>SRCAND</c> and then the colour image with <c>SRCPAINT</c> to the same rectangle. The two
/// together are a bitmap with an alpha channel, and merging them needs nothing from the page —
/// only the two sources. That is why this is worth resolving and the other 254 are not, and it
/// is what <c>MtfTools::ResolveBitmapActions</c> exists for
/// (<c>emfio/source/reader/mtftools.cxx:2612-2657</c>).
/// </para>
/// <para>
/// The merge needs pixels, and <b>pixels here need no codec</b>: an uncompressed DIB is a
/// stride and a channel order, so <see cref="DeviceIndependentBitmap.ReadPixels"/> answers
/// straight RGBA by arithmetic. The ordinary opaque blit still goes through undecoded as
/// <c>RasterImage.Encoded</c>, so the common path costs nothing and the dependency
/// <c>Paperless.Vector</c> was arranged to avoid stays avoided.
/// </para>
/// </remarks>
public static class RasterOperations
{
    /// <summary>Copy the source over the destination.</summary>
    public const uint SourceCopy = 0x00CC0020;

    /// <summary>OR the source into the destination: the second half of the transparent idiom.</summary>
    public const uint SourcePaint = 0x00EE0086;

    /// <summary>AND the source into the destination: the mask half of the transparent idiom.</summary>
    public const uint SourceAnd = 0x008800C6;

    /// <summary>XOR the source into the destination.</summary>
    public const uint SourceInvert = 0x00660046;

    /// <summary>Fill with black.</summary>
    public const uint Blackness = 0x00000042;

    /// <summary>Fill with white.</summary>
    public const uint Whiteness = 0x00FF0062;

    /// <summary>Leave the destination alone.</summary>
    public const uint DestinationCopy = 0x00AA0029;

    /// <summary>
    /// True when the two operations, in this order and at one rectangle, are the transparent
    /// bitmap idiom.
    /// </summary>
    /// <param name="first">The raster operation of the first blit.</param>
    /// <param name="second">The raster operation of the second.</param>
    /// <param name="invertMask">
    /// Set when the first bitmap is the colour image rather than the mask, so its luminance has
    /// to be inverted before it can be read as transparency. Both orders occur; i20085 records
    /// the second.
    /// </param>
    public static bool IsTransparentPair(uint first, uint second, out bool invertMask)
    {
        invertMask = first == SourcePaint;

        return (first == SourceAnd && second is SourcePaint or SourceInvert)
            || (first == SourcePaint && second == SourceAnd);
    }

    /// <summary>
    /// Combines a colour bitmap with a monochrome mask into one bitmap with an alpha channel.
    /// </summary>
    /// <remarks>
    /// The mask is read as <em>transparency</em>, not as coverage: a <c>SRCAND</c> mask is white
    /// where the destination is meant to show through, because ANDing with white leaves the
    /// destination alone. Getting that sense backwards produces a picture that is a silhouette
    /// of itself, which looks like a mask bug and reads like a colour bug.
    /// </remarks>
    /// <param name="colour">The colour image.</param>
    /// <param name="mask">The mask, at any size; it is sampled nearest-neighbour if it differs.</param>
    /// <param name="invertMask">True to read black rather than white as transparent.</param>
    public static RasterImage Merge(
        DeviceIndependentBitmap.Pixels colour,
        DeviceIndependentBitmap.Pixels mask,
        bool invertMask)
    {
        byte[] rgba = (byte[])colour.Rgba.Clone();

        for (int y = 0; y < colour.Height; y++)
        {
            int maskY = mask.Height == colour.Height ? y : y * mask.Height / colour.Height;

            for (int x = 0; x < colour.Width; x++)
            {
                int maskX = mask.Width == colour.Width ? x : x * mask.Width / colour.Width;
                int at = ((maskY * mask.Width) + maskX) * 4;

                // Luminance rather than one channel, because a mask need not be strictly
                // monochrome: producers write anti-aliased ones and grey then means partial
                // transparency, which is exactly what an alpha channel wants.
                int transparency = ((mask.Rgba[at] * 77) + (mask.Rgba[at + 1] * 151) + (mask.Rgba[at + 2] * 28)) >> 8;
                if (invertMask) transparency = 255 - transparency;

                rgba[(((y * colour.Width) + x) * 4) + 3] = (byte)(255 - transparency);
            }
        }

        return new RasterImage { Width = colour.Width, Height = colour.Height, Pixels = rgba };
    }

    /// <summary>
    /// Makes every pixel of one colour fully transparent, as <c>TransparentBlt</c> asks.
    /// </summary>
    /// <param name="pixels">The image.</param>
    /// <param name="colour">The colour to knock out; alpha is ignored.</param>
    public static RasterImage KnockOut(DeviceIndependentBitmap.Pixels pixels, Colour colour)
    {
        byte[] rgba = (byte[])pixels.Rgba.Clone();

        for (int i = 0; i < rgba.Length; i += 4)
        {
            if (rgba[i] == colour.R && rgba[i + 1] == colour.G && rgba[i + 2] == colour.B) rgba[i + 3] = 0;
        }

        return new RasterImage { Width = pixels.Width, Height = pixels.Height, Pixels = rgba };
    }
}

/// <summary>
/// A blit held back for one record, so that a mask and its image can be recognised as a pair.
/// </summary>
/// <remarks>
/// The idiom is two consecutive records at one destination rectangle, so exactly one blit has
/// to be deferred to see it: if the next record is its partner the two are merged, and
/// otherwise the deferred one is drawn as it stands. LibreOffice defers the whole file's worth
/// and groups by rectangle (<c>mtftools.cxx:2557</c>), but only ever merges a pair found at the
/// start of a group, so one slot is the same answer for less state — and it bounds the memory
/// an adversarial file can make a decoder hold.
/// </remarks>
/// <param name="Data">The DIB's bytes, kept so the merge can decode them only if it happens.</param>
/// <param name="Bitmap">The DIB already measured and re-wrapped, for the unmerged case.</param>
/// <param name="Destination">Where it goes, already mapped.</param>
/// <param name="Operation">The ternary raster operation.</param>
/// <param name="Source">The source rectangle in pixels, or an empty one for the whole bitmap.</param>
public sealed record PendingBlit(
    ReadOnlyMemory<byte> Data,
    DeviceIndependentBitmap.Result Bitmap,
    DocRect Destination,
    uint Operation,
    (int X, int Y, int Width, int Height) Source)
{
    /// <summary>True when this and a following blit are the transparent bitmap idiom.</summary>
    /// <param name="next">The blit that follows.</param>
    /// <param name="invertMask">Set when the mask's sense is inverted; see <see cref="RasterOperations"/>.</param>
    public bool PairsWith(PendingBlit next, out bool invertMask)
    {
        ArgumentNullException.ThrowIfNull(next);

        invertMask = false;

        return Destination == next.Destination
            && Bitmap.Width == next.Bitmap.Width
            && Bitmap.Height == next.Bitmap.Height
            && RasterOperations.IsTransparentPair(Operation, next.Operation, out invertMask);
    }
}

/// <summary>
/// The size a source rectangle turns a destination into when the whole bitmap is placed
/// instead of a crop of it.
/// </summary>
/// <remarks>
/// <b>Cropping without a codec.</b> The image is normally still encoded, so a source rectangle
/// cannot be cut out of it. Placing the whole image scaled so that the wanted part lands
/// exactly on the destination, and clipping to the destination, is the same picture and needs
/// no decode.
/// </remarks>
public static class SourceRectangle
{
    /// <summary>Whether a stated source rectangle actually crops anything.</summary>
    /// <param name="source">The source rectangle in pixels.</param>
    /// <param name="width">The bitmap's width in pixels.</param>
    /// <param name="height">The bitmap's height in pixels.</param>
    public static bool Crops((int X, int Y, int Width, int Height) source, int width, int height)
        => source.Width > 0
            && source.Height > 0
            && source.X >= 0
            && source.Y >= 0
            && (source.X != 0 || source.Y != 0 || source.Width != width || source.Height != height)
            && source.X + source.Width <= width
            && source.Y + source.Height <= height;

    /// <summary>Where the whole bitmap goes so that the source rectangle lands on the destination.</summary>
    /// <param name="destination">Where the cropped part goes.</param>
    /// <param name="source">The source rectangle in pixels.</param>
    /// <param name="width">The bitmap's width in pixels.</param>
    /// <param name="height">The bitmap's height in pixels.</param>
    public static DocRect Whole(
        DocRect destination,
        (int X, int Y, int Width, int Height) source,
        int width,
        int height)
    {
        double scaleX = (double)destination.Width.Emu / source.Width;
        double scaleY = (double)destination.Height.Emu / source.Height;

        return new DocRect(
            destination.X - Length.FromEmu((long)Math.Round(source.X * scaleX)),
            destination.Y - Length.FromEmu((long)Math.Round(source.Y * scaleY)),
            Length.FromEmu((long)Math.Round(width * scaleX)),
            Length.FromEmu((long)Math.Round(height * scaleY)));
    }
}
