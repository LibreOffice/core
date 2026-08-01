using System.Buffers.Binary;
using Paperless.Core.Geometry;
using Paperless.Core.Units;

namespace Paperless.Presentations.Layout;

/// <summary>
/// What a slide needs to know about an embedded picture without decoding it: how large it is,
/// and where a crop puts it.
/// </summary>
/// <remarks>
/// <para>
/// <b>This reads headers, not images.</b> A picture's pixels are none of a reader's business —
/// <c>RasterImage.Encoded</c> hands the file's own bytes on and a backend decodes them when it
/// wants pixels — but two of the things a slide states are relative to the picture's
/// <em>natural size</em>, and a tile scaled to 471698 thousandths of a percent means nothing
/// until that size is known. Twenty bytes of PNG header answer it; a codec is not needed and
/// would drag the rasteriser into the extraction path, which the layering forbids.
/// </para>
/// <para>
/// The natural size is a physical length, not a pixel count. LibreOffice asks the graphic for
/// its <c>Size100thMM</c> and falls back to converting the pixel size at the screen's
/// resolution when the format states none (<c>GraphicHelper::getOriginalSize</c>,
/// <c>oox/source/helper/graphichelper.cxx:302</c>), so a PNG's <c>pHYs</c> chunk and a JPEG's
/// JFIF density are read where present and 96 dpi assumed where they are not. Measured on
/// <c>paint-fills-pptx.pptx</c>: LibreOffice's own export scaled an 8-pixel checkerboard by
/// 4.71698 to mean a one-centimetre tile, which is only true at 96 dpi.
/// </para>
/// </remarks>
internal static class SlideImages
{
    /// <summary>Dots per inch assumed when a picture's own header states no resolution.</summary>
    private const double ScreenDpi = 96;

    private const long EmuPerInch = 914400;

    /// <summary>
    /// The natural size of an encoded picture, or null when the bytes are not a format whose
    /// header this reads.
    /// </summary>
    /// <remarks>
    /// PNG, JPEG, GIF and BMP, which is every raster format the corpus and every real deck use
    /// for a tiled fill. A caller that gets null falls back to the shape's own size, which
    /// draws the fill once rather than not at all.
    /// </remarks>
    /// <param name="bytes">The picture exactly as the file stored it.</param>
    public static DocSize? NaturalSize(ReadOnlySpan<byte> bytes)
    {
        if (Png(bytes) is { } png) return png;
        if (Jpeg(bytes) is { } jpeg) return jpeg;
        if (Gif(bytes) is { } gif) return gif;
        if (Bmp(bytes) is { } bmp) return bmp;
        return null;
    }

    /// <summary>
    /// Where the <em>whole</em> picture goes, given where the visible part of it must land.
    /// </summary>
    /// <remarks>
    /// A crop has no representation in the drawing IR and needs none: cropping is drawing the
    /// picture larger and clipping it, and every backend already clips. If the source rectangle
    /// throws away a fraction <c>l</c> of the left edge, then the surviving <c>1 - l - r</c> of
    /// the image is what fills the destination, so the whole of it is that much wider — which
    /// is the same arithmetic <c>CropQuotientsFromSrcRect</c> does from the other end
    /// (<c>oox/source/drawingml/fillproperties.cxx:106</c>).
    /// </remarks>
    /// <param name="destination">Where the visible part of the picture goes.</param>
    /// <param name="left">Fraction cropped from the source's left edge.</param>
    /// <param name="top">Fraction cropped from its top edge.</param>
    /// <param name="right">Fraction cropped from its right edge.</param>
    /// <param name="bottom">Fraction cropped from its bottom edge.</param>
    /// <returns>
    /// The rectangle to draw the undisturbed picture into, or null when the crop keeps
    /// nothing — which a file can state and which would otherwise divide by zero.
    /// </returns>
    public static DocRect? Uncropped(
        DocRect destination, double left, double top, double right, double bottom)
    {
        double horizontal = 1 - left - right;
        double vertical = 1 - top - bottom;
        if (horizontal <= 0 || vertical <= 0) return null;

        double width = destination.Width.Emu / horizontal;
        double height = destination.Height.Emu / vertical;

        return new DocRect(
            Length.FromEmu(destination.Left.Emu - (long)Math.Round(left * width)),
            Length.FromEmu(destination.Top.Emu - (long)Math.Round(top * height)),
            Length.FromEmu((long)Math.Round(width)),
            Length.FromEmu((long)Math.Round(height)));
    }

    /// <summary>
    /// The rectangle a stretched fill draws its picture into, inset by <c>a:fillRect</c>.
    /// </summary>
    /// <remarks>
    /// The mirror image of <see cref="Uncropped"/> and the reason both exist: a positive
    /// <c>a:srcRect</c> edge throws away part of the picture, a positive <c>a:fillRect</c> edge
    /// leaves part of the <em>shape</em> empty. A negative one on either grows rather than
    /// shrinks, which is legal and is how a file states an overhanging fill.
    /// </remarks>
    /// <param name="area">The area being filled.</param>
    /// <param name="left">Fraction of the area's width to leave at the left.</param>
    /// <param name="top">Fraction of its height to leave at the top.</param>
    /// <param name="right">Fraction of its width to leave at the right.</param>
    /// <param name="bottom">Fraction of its height to leave at the bottom.</param>
    public static DocRect Inset(
        DocRect area, double left, double top, double right, double bottom)
    {
        double width = area.Width.Emu;
        double height = area.Height.Emu;

        return new DocRect(
            Length.FromEmu(area.Left.Emu + (long)Math.Round(left * width)),
            Length.FromEmu(area.Top.Emu + (long)Math.Round(top * height)),
            Length.FromEmu((long)Math.Round(width * (1 - left - right))),
            Length.FromEmu((long)Math.Round(height * (1 - top - bottom))));
    }

    /// <summary>
    /// Where the first tile of a repeating fill sits, given which point of the box the grid is
    /// anchored on.
    /// </summary>
    /// <remarks>
    /// <c>BitmapPaint.TileOffset</c> is a phase rather than a position, so any tile of the grid
    /// says the same thing; this returns the one touching the anchor. DrawingML spells the
    /// anchor <c>a:tile/@algn</c> and ODF spells it <c>draw:fill-image-ref-point</c>, with the
    /// same nine values under different names, and both default to a corner in the schema and
    /// to the centre in LibreOffice's own files.
    /// </remarks>
    /// <param name="box">The box being filled.</param>
    /// <param name="tile">One tile's size.</param>
    /// <param name="horizontal">-1 for the left edge, 0 for the centre, 1 for the right edge.</param>
    /// <param name="vertical">-1 for the top edge, 0 for the middle, 1 for the bottom edge.</param>
    public static DocPoint TileOrigin(DocRect box, DocSize tile, int horizontal, int vertical)
        => new(
            Length.FromEmu(horizontal switch
            {
                < 0 => box.Left.Emu,
                0 => box.Left.Emu + ((box.Width.Emu - tile.Width.Emu) / 2),
                _ => box.Right.Emu - tile.Width.Emu,
            }),
            Length.FromEmu(vertical switch
            {
                < 0 => box.Top.Emu,
                0 => box.Top.Emu + ((box.Height.Emu - tile.Height.Emu) / 2),
                _ => box.Bottom.Emu - tile.Height.Emu,
            }));

    // ---------------------------------------------------------------------------- the headers

    private static DocSize? Png(ReadOnlySpan<byte> bytes)
    {
        ReadOnlySpan<byte> signature = [0x89, (byte)'P', (byte)'N', (byte)'G', 0x0D, 0x0A, 0x1A, 0x0A];
        if (bytes.Length < 33 || !bytes[..8].SequenceEqual(signature)) return null;
        if (!bytes.Slice(12, 4).SequenceEqual("IHDR"u8)) return null;

        int width = BinaryPrimitives.ReadInt32BigEndian(bytes.Slice(16, 4));
        int height = BinaryPrimitives.ReadInt32BigEndian(bytes.Slice(20, 4));
        if (width <= 0 || height <= 0) return null;

        // pHYs is optional and may sit behind any number of other chunks, so the chunk list is
        // walked rather than guessed at. Unit 1 is metres; unit 0 means "aspect ratio only",
        // which states no resolution at all and falls back to the screen's.
        int at = 8;
        while (at + 8 <= bytes.Length)
        {
            int length = BinaryPrimitives.ReadInt32BigEndian(bytes.Slice(at, 4));
            if (length < 0 || at + 12 + length > bytes.Length) break;

            if (bytes.Slice(at + 4, 4).SequenceEqual("pHYs"u8) && length >= 9
                && bytes[at + 8 + 8] == 1)
            {
                int perMetreX = BinaryPrimitives.ReadInt32BigEndian(bytes.Slice(at + 8, 4));
                int perMetreY = BinaryPrimitives.ReadInt32BigEndian(bytes.Slice(at + 12, 4));

                if (perMetreX > 0 && perMetreY > 0)
                {
                    return new DocSize(
                        Length.FromEmu((long)Math.Round(width * 36000000.0 / perMetreX)),
                        Length.FromEmu((long)Math.Round(height * 36000000.0 / perMetreY)));
                }
            }

            if (bytes.Slice(at + 4, 4).SequenceEqual("IDAT"u8)) break;
            at += 12 + length;
        }

        return AtScreenResolution(width, height);
    }

    private static DocSize? Jpeg(ReadOnlySpan<byte> bytes)
    {
        if (bytes.Length < 4 || bytes[0] != 0xFF || bytes[1] != 0xD8) return null;

        double dpiX = ScreenDpi;
        double dpiY = ScreenDpi;
        int at = 2;

        while (at + 4 <= bytes.Length)
        {
            if (bytes[at] != 0xFF) return null;

            byte marker = bytes[at + 1];

            // Standalone markers carry no length, and the padding fill bytes before a marker are
            // 0xFF repeated; both would otherwise be read as a segment length.
            if (marker == 0xFF) { at++; continue; }
            if (marker is 0xD8 or 0x01 or >= 0xD0 and <= 0xD7) { at += 2; continue; }

            int length = BinaryPrimitives.ReadUInt16BigEndian(bytes.Slice(at + 2, 2));
            if (length < 2 || at + 2 + length > bytes.Length) return null;

            if (marker == 0xE0 && length >= 16 && bytes.Slice(at + 4, 5).SequenceEqual("JFIF\0"u8))
            {
                int units = bytes[at + 11];
                int densityX = BinaryPrimitives.ReadUInt16BigEndian(bytes.Slice(at + 12, 2));
                int densityY = BinaryPrimitives.ReadUInt16BigEndian(bytes.Slice(at + 14, 2));

                if (densityX > 0 && densityY > 0)
                {
                    // Unit 1 is dots per inch, unit 2 dots per centimetre; unit 0 is a bare
                    // aspect ratio and says nothing about physical size.
                    if (units == 1) (dpiX, dpiY) = (densityX, densityY);
                    else if (units == 2) (dpiX, dpiY) = (densityX * 2.54, densityY * 2.54);
                }
            }

            // The frame headers, of which SOF0 through SOF15 all carry the size in the same
            // place. DHT (0xC4), JPG (0xC8) and DAC (0xCC) share the range and do not.
            if (marker is >= 0xC0 and <= 0xCF && marker is not (0xC4 or 0xC8 or 0xCC))
            {
                if (length < 7) return null;

                int height = BinaryPrimitives.ReadUInt16BigEndian(bytes.Slice(at + 5, 2));
                int width = BinaryPrimitives.ReadUInt16BigEndian(bytes.Slice(at + 7, 2));
                return width <= 0 || height <= 0 ? null : Physical(width, height, dpiX, dpiY);
            }

            at += 2 + length;
        }

        return null;
    }

    private static DocSize? Gif(ReadOnlySpan<byte> bytes)
    {
        if (bytes.Length < 10
            || (!bytes[..6].SequenceEqual("GIF87a"u8) && !bytes[..6].SequenceEqual("GIF89a"u8)))
        {
            return null;
        }

        return AtScreenResolution(
            BinaryPrimitives.ReadUInt16LittleEndian(bytes.Slice(6, 2)),
            BinaryPrimitives.ReadUInt16LittleEndian(bytes.Slice(8, 2)));
    }

    private static DocSize? Bmp(ReadOnlySpan<byte> bytes)
    {
        if (bytes.Length < 30 || bytes[0] != 'B' || bytes[1] != 'M') return null;

        int width = BinaryPrimitives.ReadInt32LittleEndian(bytes.Slice(18, 4));
        int height = Math.Abs(BinaryPrimitives.ReadInt32LittleEndian(bytes.Slice(22, 4)));
        if (width <= 0 || height <= 0) return null;

        if (bytes.Length >= 46)
        {
            int perMetreX = BinaryPrimitives.ReadInt32LittleEndian(bytes.Slice(38, 4));
            int perMetreY = BinaryPrimitives.ReadInt32LittleEndian(bytes.Slice(42, 4));

            if (perMetreX > 0 && perMetreY > 0)
            {
                return new DocSize(
                    Length.FromEmu((long)Math.Round(width * 36000000.0 / perMetreX)),
                    Length.FromEmu((long)Math.Round(height * 36000000.0 / perMetreY)));
            }
        }

        return AtScreenResolution(width, height);
    }

    private static DocSize AtScreenResolution(int width, int height)
        => Physical(width, height, ScreenDpi, ScreenDpi);

    private static DocSize Physical(int width, int height, double dpiX, double dpiY)
        => new(
            Length.FromEmu((long)Math.Round(width * EmuPerInch / dpiX)),
            Length.FromEmu((long)Math.Round(height * EmuPerInch / dpiY)));
}
