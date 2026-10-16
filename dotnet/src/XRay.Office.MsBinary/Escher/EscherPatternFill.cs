using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Vector.Metafiles;

namespace XRay.Office.MsBinary.Escher;

/// <summary>
/// The tile an Escher <c>mso_fillPattern</c>, <c>mso_fillTexture</c> or <c>mso_fillPicture</c>
/// fill is drawn with.
/// </summary>
/// <remarks>
/// <para>
/// <strong>A pattern fill is a two-colour stencil, not a picture.</strong> The blip
/// <c>fillBlip</c> names is an eight-by-eight monochrome DIB whose <em>black</em> pixels stand
/// for the shape's <c>fillBackColor</c> and whose other pixels stand for its <c>fillColor</c>;
/// the bitmap itself carries neither colour. Drawing it as authored gives a black-and-white
/// checker where the file states grey-on-white, and painting the shape in <c>fillColor</c> —
/// which is what this library did until this class existed — gives a solid block where the file
/// states a hatch.
/// </para>
/// <para>
/// <c>DffPropertyReader::ApplyFillAttributes</c> is the rule
/// (<c>filter/source/msfilter/msdffimp.cxx</c>:1401-1443 <strong>in this tree</strong>, which
/// declares <c>27.2.0.0.alpha0+</c> and is not the 26.2.4.2 binary's source). It guards the
/// recolouring on the bitmap being exactly eight by eight, tests each pixel against
/// <c>Color(0)</c> — pure black — and writes <c>fillBackColor</c> there and <c>fillColor</c>
/// everywhere else, defaulting both to white. A bitmap of any other size is drawn as it was
/// stored, which is what a texture and a picture fill are.
/// </para>
/// <para>
/// <strong>Confirmed against 26.2.4.2's own resolved view rather than against this source.</strong>
/// <c>apron-area.xls</c> converted to <c>fods</c> at that binary writes the four recoloured tiles
/// out as <c>draw:fill-image</c> PNGs: the workbook's blips 4 and 6 are an eight-row
/// <c>00110011</c> and a 45° diagonal, and the reference's own PNGs carry
/// <c>#969696</c>/<c>#808080</c> exactly where those bits are clear and <c>#ffffff</c> exactly
/// where they are set — the stated <c>fillColor</c> and the defaulted <c>fillBackColor</c>, in
/// that order.
/// </para>
/// </remarks>
public static class EscherPatternFill
{
    /// <summary>The side a bitmap must have for the two fill colours to be written into it.</summary>
    /// <remarks>
    /// <c>aBmp.GetSizePixel().Width() == 8 &amp;&amp; aBmp.GetSizePixel().Height() == 8</c>. The
    /// reference also requires the bitmap to be eight bits per pixel, which is a property of the
    /// <c>CreateColorBitmap</c> conversion it has just done rather than of the file: every
    /// paletted DIB reaches that test as <c>N8_BPP</c>.
    /// </remarks>
    private const int PatternSide = 8;

    /// <summary>The size of the file header a DIB blip is handed out behind.</summary>
    /// <remarks>
    /// <see cref="EscherBlips"/> puts a <c>BITMAPFILEHEADER</c> in front of a DIB blip's bytes so
    /// that a decoder will take them, exactly as <c>SvxMSDffManager::GetBLIPDirect</c> does. The
    /// pixels are read from the DIB behind it.
    /// </remarks>
    private const int FileHeaderSize = DeviceIndependentBitmap.FileHeaderSize;

    /// <summary>The first of the two bytes a <c>BITMAPFILEHEADER</c> starts with.</summary>
    private const byte FileHeaderMagicLow = (byte)'B';

    /// <summary>The second.</summary>
    private const byte FileHeaderMagicHigh = (byte)'M';

    /// <summary>
    /// How many points one pixel of a fill bitmap covers when nothing states a size.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A pattern states no tile size, so the tile is the bitmap's own — and a bitmap's own size is
    /// its pixel count at the device's resolution, which headless LibreOffice reports as
    /// <b>96 dpi</b> (<c>SvpSalGraphics::GetResolution</c>, <c>vcl/headless/svpgdi.cxx</c>:44 in
    /// this tree). One pixel is therefore three quarters of a point.
    /// </para>
    /// <para>
    /// <strong>Measured, not derived.</strong> 26.2.4.2's own PDF of <c>apron-area.xls</c> draws
    /// the vertical-stripe pattern as a 629-pixel-wide image placed 151.909 pt wide, with 51
    /// stripes between x = 6 and x = 628: a period of 12.44 image pixels, or 3.0044 pt, and the
    /// eight-pixel tile holds two periods — <b>6.0087 pt against the 6.0 this constant gives</b>.
    /// </para>
    /// </remarks>
    private static readonly Length PixelSize = Length.FromPoints(0.75);

    /// <summary>
    /// The tile a pattern, texture or picture fill draws, or null when the blip cannot be
    /// turned into one.
    /// </summary>
    /// <param name="bytes">
    /// The blip's bytes as <see cref="EscherBlips"/> hands them out — a DIB behind a synthesised
    /// file header, or an encoded raster.
    /// </param>
    /// <param name="foreground">
    /// The shape's <c>fillColor</c>, already resolved through the host's palette.
    /// </param>
    /// <param name="background">
    /// Its <c>fillBackColor</c>, resolved the same way; white where the shape states none, which
    /// is the reference's <c>Color aCol2( COL_WHITE )</c>.
    /// </param>
    /// <returns>
    /// The recoloured stencil where the blip is an eight-by-eight bitmap, and the blip's own
    /// image where it is anything else.
    /// </returns>
    public static RasterImage? Tile(
        ReadOnlySpan<byte> bytes, Colour foreground, Colour background)
    {
        if (bytes.IsEmpty) return null;

        ReadOnlySpan<byte> dib = Dib(bytes);
        if (dib.IsEmpty) return RasterImage.Encoded(bytes.ToArray());

        if (DeviceIndependentBitmap.ReadPixels(dib) is not { } pixels)
        {
            return RasterImage.Encoded(bytes.ToArray());
        }

        if (pixels.Width != PatternSide || pixels.Height != PatternSide) return pixels.Image;

        byte[] recoloured = new byte[pixels.Rgba.Length];
        for (int at = 0; at + 4 <= pixels.Rgba.Length; at += 4)
        {
            bool black = pixels.Rgba[at] == 0 && pixels.Rgba[at + 1] == 0 && pixels.Rgba[at + 2] == 0;
            Colour colour = black ? background : foreground;

            recoloured[at] = colour.R;
            recoloured[at + 1] = colour.G;
            recoloured[at + 2] = colour.B;
            recoloured[at + 3] = pixels.Rgba[at + 3];
        }

        return new RasterImage
        {
            Width = pixels.Width, Height = pixels.Height, Pixels = recoloured,
        };
    }

    /// <summary>
    /// One tile's size at the resolution the reference draws it, before any print zoom.
    /// </summary>
    /// <param name="image">The tile.</param>
    public static DocSize Extent(RasterImage image)
    {
        ArgumentNullException.ThrowIfNull(image);

        return new DocSize(
            Length.FromEmu(image.Width * PixelSize.Emu),
            Length.FromEmu(image.Height * PixelSize.Emu));
    }

    /// <summary>
    /// How opaque a fill is, from <c>fillOpacity</c>.
    /// </summary>
    /// <remarks>
    /// A 16.16 fixed-point fraction, and <c>ApplyFillAttributes</c> turns it into an
    /// <c>XFillTransparenceItem</c> for every fill style but a gradient
    /// (<c>msdffimp.cxx</c>:1367-1376 in this tree). All four of <c>apron-area.xls</c>'s pattern
    /// fills state one — 0.35, 0.4, 0.5 and 0.6 — and 26.2.4.2's <c>fods</c> writes the same four
    /// out as <c>draw:opacity</c>.
    /// </remarks>
    /// <param name="properties">The shape's property table.</param>
    public static double Opacity(EscherPropertyTable properties)
    {
        ArgumentNullException.ThrowIfNull(properties);

        if (!properties.Has(EscherPropertyIds.FillOpacity)) return 1;

        double stated = properties.Value(EscherPropertyIds.FillOpacity) / 65536.0;
        return Math.Clamp(stated, 0, 1);
    }

    /// <summary>The DIB inside a blip's bytes, or null when the bytes are not one.</summary>
    private static ReadOnlySpan<byte> Dib(ReadOnlySpan<byte> bytes)
    {
        if (bytes.Length <= FileHeaderSize) return default;
        if (bytes[0] != FileHeaderMagicLow || bytes[1] != FileHeaderMagicHigh) return default;

        return bytes[FileHeaderSize..];
    }
}
