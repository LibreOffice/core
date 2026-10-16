using System.Buffers.Binary;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.MsBinary.Escher;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// An Escher pattern fill is a two-colour stencil, and painting its foreground colour over the
/// whole shape is a solid block where the file states a hatch.
/// </summary>
/// <remarks>
/// <para>
/// <c>EscherInk.Read</c> resolved <c>mso_fillPattern</c>, <c>mso_fillTexture</c> and
/// <c>mso_fillPicture</c> to <c>fillColor</c>, under a comment that recorded the shortcut and put
/// its reach at <em>two shapes in one corpus <c>.xls</c></em>. That count was taken while every
/// grouped shape was being dropped; with round 103's group fix in it is <b>four shapes in one
/// document</b> — <c>apron-area.xls</c>, all four <c>mso_fillPattern</c> — and they are four
/// solid grey rectangles where 26.2.4.2 draws a hatch. <c>probes/sheet-draw-r107/fill-census.txt</c>
/// is the census over all 64 corpus <c>.xls</c>: 562 shape containers, 4 stating one.
/// </para>
/// <para>
/// <strong>The two DIBs below are that workbook's own blips 4 and 6</strong>, lifted byte for
/// byte out of its <c>MSODRAWINGGROUP</c>, and the colours are the two <c>MSO_CLR</c> values its
/// shapes state resolved through Excel's palette. The expected pixels are <b>26.2.4.2's</b>: its
/// <c>--convert-to fods</c> of the same workbook writes the recoloured tiles out as four
/// <c>draw:fill-image</c> PNGs, and those PNGs carry the grey exactly where these bits are clear
/// and white exactly where they are set.
/// </para>
/// </remarks>
public sealed class EscherPatternFillTests
{
    /// <summary><c>mso_fillPattern</c>.</summary>
    private const uint PatternFill = 1;

    /// <summary><c>mso_fillSolid</c>.</summary>
    private const uint SolidFill = 0;

    /// <summary><c>mso_fillBackground</c>, which paints nothing.</summary>
    private const uint BackgroundFill = 9;

    /// <summary><c>mso_sptRectangle</c>, which is filled by default.</summary>
    private const ushort Rectangle = 1;

    /// <summary>
    /// <c>apron-area.xls</c>' blip 4: an eight-by-eight one-bit DIB whose every row is
    /// <c>00110011</c>, with a two-entry palette of white then black.
    /// </summary>
    private const string VerticalStripes =
        "KAAAAAgAAAAIAAAAAQABAAAAAAAgAAAAiQsAAIkLAAACAAAAAgAAAP///wAAAAAA"
        + "MwAAADMAAAAzAAAAMwAAADMAAAAzAAAAMwAAADMAAAA=";

    /// <summary>Its blip 6, the same shape with a diagonal bit pattern.</summary>
    private const string Diagonal =
        "KAAAAAgAAAAIAAAAAQABAAAAAAAgAAAAiQsAAIkLAAACAAAAAgAAAP///wAAAAAA"
        + "fAAAAPgAAADxAAAA4wAAAMcAAACPAAAAHwAAAD4AAAA=";

    /// <summary>Palette index 0x17, which that workbook's <c>PALETTE</c> makes mid grey.</summary>
    private static readonly Colour Grey = new(0x80, 0x80, 0x80);

    [Fact]
    public void APatternsBlackPixelsTakeTheBackColourAndTheRestTheForeColour()
    {
        // 26.2.4.2's `Bitmap_20_4`, row for row: two greys, two whites, two greys, two whites —
        // the inverse of the `00110011` bits, because a *set* bit is the palette's black and
        // `ApplyFillAttributes` writes `fillBackColor` there (`msdffimp.cxx`:1432-1435 in this
        // tree). `fillBackColor` is unstated on all four shapes and defaults to white.
        RasterImage tile = EscherPatternFill.Tile(Bmp(VerticalStripes), Grey, Colour.White)!;

        tile.Width.ShouldBe(8);
        tile.Height.ShouldBe(8);

        Row(tile, 0).ShouldBe([Grey, Grey, Colour.White, Colour.White,
                               Grey, Grey, Colour.White, Colour.White]);
        Row(tile, 7).ShouldBe(Row(tile, 0));
    }

    [Fact]
    public void TheSameBlipUnderADifferentForeColourGivesADifferentTile()
    {
        // `apron-area.xls` states blip 6 twice, once with palette 0x37 and once with 0x17, and
        // 26.2.4.2 writes both out: `Bitmap_20_1` in #969696 and `Bitmap_20_3` in #808080, the
        // same bit pattern. So the recolouring is per shape and not per blip.
        Colour lighter = new(0x96, 0x96, 0x96);

        Row(EscherPatternFill.Tile(Bmp(Diagonal), Grey, Colour.White)!, 0)
            .ShouldBe([Grey, Grey, Colour.White, Colour.White,
                       Colour.White, Colour.White, Colour.White, Grey]);

        Row(EscherPatternFill.Tile(Bmp(Diagonal), lighter, Colour.White)!, 0)
            .ShouldBe([lighter, lighter, Colour.White, Colour.White,
                       Colour.White, Colour.White, Colour.White, lighter]);
    }

    [Fact]
    public void AnEightPixelTileIsSixPointsAcross()
    {
        // A bitmap's own size is its pixel count at the device's resolution, which headless
        // LibreOffice reports as 96 dpi. Measured at the reference rather than derived: its PDF
        // of `apron-area.xls` draws the striped rectangle as a 629-pixel image placed 151.909 pt
        // wide with 51 stripes between x = 6 and x = 628, which is 3.0044 pt per half-tile.
        DocSize extent = EscherPatternFill.Extent(
            EscherPatternFill.Tile(Bmp(VerticalStripes), Grey, Colour.White)!);

        extent.Width.Points.ShouldBe(6, 0.001);
        extent.Height.Points.ShouldBe(6, 0.001);
    }

    [Fact]
    public void ABitmapFillReportsItselfRatherThanResolvingToItsForegroundColour()
    {
        // The three bitmap fill types leave `Fill` null and set `BitmapFill`, so the caller goes
        // to the blip store; a solid fill is unaffected and `mso_fillBackground` still paints
        // nothing at all.
        EscherInk.Ink pattern = EscherInk.Read(
            Table((EscherPropertyIds.FillType, PatternFill),
                  (EscherPropertyIds.FillColour, 0x00808080)),
            Rectangle);

        pattern.BitmapFill.ShouldBeTrue();
        pattern.Fill.ShouldBeNull();
        pattern.HasInk.ShouldBeTrue();

        EscherInk.Ink solid = EscherInk.Read(
            Table((EscherPropertyIds.FillType, SolidFill),
                  (EscherPropertyIds.FillColour, 0x00808080)),
            Rectangle);

        solid.BitmapFill.ShouldBeFalse();
        solid.Fill.ShouldBe(Grey);

        EscherInk.Ink behind = EscherInk.Read(
            Table((EscherPropertyIds.FillType, BackgroundFill),
                  (EscherPropertyIds.FillColour, 0x00808080)),
            Rectangle);

        behind.BitmapFill.ShouldBeFalse();
        behind.Fill.ShouldBeNull();
    }

    [Fact]
    public void FillOpacityIsASixteenSixteenFractionAndDefaultsToOpaque()
    {
        // All four of `apron-area.xls`' pattern fills state one — 22938, 26214, 32768 and 39322
        // — and 26.2.4.2's `fods` writes them back as `draw:opacity` 35%, 40%, 50% and 60%.
        EscherPatternFill.Opacity(Table((EscherPropertyIds.FillOpacity, 39322)))
            .ShouldBe(0.6, 0.0001);

        EscherPatternFill.Opacity(Table((EscherPropertyIds.FillColour, 0))).ShouldBe(1);
    }

    /// <summary>One row of a decoded tile, as colours.</summary>
    private static Colour[] Row(RasterImage tile, int row)
    {
        ReadOnlySpan<byte> pixels = tile.Pixels.Span;
        Colour[] colours = new Colour[tile.Width];
        for (int x = 0; x < tile.Width; x++)
        {
            int at = ((row * tile.Width) + x) * 4;
            colours[x] = new Colour(pixels[at], pixels[at + 1], pixels[at + 2]);
        }

        return colours;
    }

    /// <summary>
    /// A DIB behind the <c>BITMAPFILEHEADER</c> <see cref="EscherBlips"/> hands one out with.
    /// </summary>
    private static byte[] Bmp(string dib)
    {
        byte[] bits = Convert.FromBase64String(dib);
        byte[] file = new byte[14 + bits.Length];

        file[0] = (byte)'B';
        file[1] = (byte)'M';
        BinaryPrimitives.WriteUInt32LittleEndian(file.AsSpan(2), (uint)file.Length);
        BinaryPrimitives.WriteUInt32LittleEndian(file.AsSpan(10), 14 + 40 + 8);
        bits.CopyTo(file, 14);

        return file;
    }

    /// <summary>An <c>msofbtOPT</c> table holding exactly these properties.</summary>
    private static EscherPropertyTable Table(params (ushort Id, uint Value)[] entries)
    {
        byte[] content = new byte[entries.Length * 6];
        for (int i = 0; i < entries.Length; i++)
        {
            BinaryPrimitives.WriteUInt16LittleEndian(content.AsSpan(i * 6), entries[i].Id);
            BinaryPrimitives.WriteUInt32LittleEndian(content.AsSpan((i * 6) + 2), entries[i].Value);
        }

        return EscherPropertyTable.Read(content, entries.Length);
    }
}
