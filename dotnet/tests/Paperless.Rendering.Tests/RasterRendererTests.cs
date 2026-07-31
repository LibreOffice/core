using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Rendering.Raster;
using Shouldly;
using SkiaSharp;

namespace Paperless.Rendering.Tests;

/// <summary>
/// What the rasteriser puts on the page, in pixels.
/// </summary>
/// <remarks>
/// <para>
/// An image comparison against LibreOffice answers "does this look the same" and is at its
/// worst at saying <em>why</em> when it does not. These tests ask the smaller question the
/// image diff cannot: given a page with one known thing on it, are those pixels the ones
/// that changed. A backend that passes these and still differs from the reference has a
/// layout problem; one that fails them has a drawing problem, and the two are chased in
/// different libraries.
/// </para>
/// <para>
/// The images are decoded with Skia itself, which is not the circularity it looks like: the
/// encoder and the decoder are separate code, and what is being checked is the geometry of
/// what was drawn rather than the fidelity of the PNG round trip.
/// </para>
/// </remarks>
public sealed class RasterRendererTests
{
    [Theory]
    [InlineData(96)]
    [InlineData(150)]
    [InlineData(300)]
    public void APageIsRasterisedAtTheChosenResolution(double dpi)
    {
        using SKBitmap bitmap = new RasterRenderer(new RasterRenderOptions { Dpi = dpi })
            .Rasterise(new DrawnPage(DrawnPage.A4, sink => { }));

        // Rounded up, which is measured rather than chosen: poppler takes the ceiling, so an A4
        // page 595.28 pt wide is 1240.16 pixels at 150 dpi and pdftoppm produces 1241. The
        // comparison script stops at DIMENSION MISMATCH before measuring anything, so a
        // one-pixel disagreement here costs the whole comparison.
        bitmap.Width.ShouldBe((int)Math.Ceiling(DrawnPage.A4.Width.Inches * dpi - 1e-6));
        bitmap.Height.ShouldBe((int)Math.Ceiling(DrawnPage.A4.Height.Inches * dpi - 1e-6));
    }

    [Fact]
    public void APageStartsAsOpaqueWhite()
    {
        using SKBitmap bitmap = new RasterRenderer().Rasterise(new DrawnPage(DrawnPage.A4, sink => { }));

        // LibreOffice's own PNG and PDF export both produce a white page. A transparent one
        // compares as a solid difference against either, whatever is drawn on it.
        bitmap.GetPixel(0, 0).ShouldBe(new SKColor(255, 255, 255, 255));
        bitmap.GetPixel(bitmap.Width - 1, bitmap.Height - 1).ShouldBe(new SKColor(255, 255, 255, 255));
    }

    [Fact]
    public void AFilledRectangleCoversTheRightPixels()
    {
        const double dpi = 150;
        using SKBitmap bitmap = new RasterRenderer(new RasterRenderOptions { Dpi = dpi })
            .Rasterise(new DrawnPage(
                DrawnPage.A4,
                sink => sink.FillPath(
                    GraphicsPath.Rectangle(new DocRect(
                        Length.FromInches(1), Length.FromInches(2),
                        Length.FromInches(3), Length.FromInches(1))),
                    Paint.Solid(Colour.Black))));

        // One inch in from the left and two down, three wide and one tall, at 150 dpi: pixels 150
        // to 599 across and 300 to 449 down. The corners are checked from inside and outside,
        // which is what catches a rectangle drawn a pixel out or scaled from the wrong origin.
        bitmap.GetPixel(151, 301).ShouldBe(new SKColor(0, 0, 0, 255));
        bitmap.GetPixel(598, 448).ShouldBe(new SKColor(0, 0, 0, 255));
        bitmap.GetPixel(148, 301).ShouldBe(new SKColor(255, 255, 255, 255));
        bitmap.GetPixel(151, 298).ShouldBe(new SKColor(255, 255, 255, 255));
        bitmap.GetPixel(601, 448).ShouldBe(new SKColor(255, 255, 255, 255));
        bitmap.GetPixel(598, 451).ShouldBe(new SKColor(255, 255, 255, 255));
    }

    [Fact]
    public void AStrokeIsCentredOnItsOwnPath()
    {
        const double dpi = 300;
        using SKBitmap bitmap = new RasterRenderer(new RasterRenderOptions { Dpi = dpi, Antialias = false })
            .Rasterise(new DrawnPage(
                DrawnPage.A4,
                sink => sink.StrokePath(
                    new GraphicsPath()
                        .MoveTo(new DocPoint(Length.FromInches(1), Length.FromInches(1)))
                        .LineTo(new DocPoint(Length.FromInches(2), Length.FromInches(1))),
                    new Stroke(Paint.Solid(Colour.Black), Length.FromPoints(6)))));

        // Six points is a twelfth of an inch, so at 300 dpi the line is 25 pixels thick and
        // straddles row 300 — twelve or thirteen either side. Centred rather than drawn to one
        // side of the path is the whole reason a table's borders can be consolidated into one
        // stroke per grid line: half of the border belongs to the cell on each side.
        bitmap.GetPixel(450, 300).ShouldBe(new SKColor(0, 0, 0, 255));
        bitmap.GetPixel(450, 290).ShouldBe(new SKColor(0, 0, 0, 255));
        bitmap.GetPixel(450, 310).ShouldBe(new SKColor(0, 0, 0, 255));
        bitmap.GetPixel(450, 285).ShouldBe(new SKColor(255, 255, 255, 255));
        bitmap.GetPixel(450, 315).ShouldBe(new SKColor(255, 255, 255, 255));
    }

    [Fact]
    public void AGlyphRunPutsItsInkAboveItsBaseline()
    {
        Assert.SkipUnless(TestFace.IsAvailable, "no usable font face on this machine");

        const double dpi = 150;
        DocPoint origin = new(Length.FromInches(1), Length.FromInches(2));

        using SKBitmap bitmap = new RasterRenderer(new RasterRenderOptions { Dpi = dpi })
            .Rasterise(new DrawnPage(
                DrawnPage.A4,
                sink => sink.DrawGlyphRun(
                    TestFace.Run("Hnnn", origin, Length.FromPoints(48)), Paint.Solid(Colour.Black))));

        // A run's origin is the start of the baseline, not the top of a box. So the ink sits
        // above row 300 and starts at column 150; a backend that took the origin for a box corner
        // would put the whole line a font ascent too low, which is the single most common way to
        // get text drawing wrong and invisible in a thumbnail.
        int top = 0, bottom = 0, left = 0;
        for (int y = 0; y < bitmap.Height; y++)
        {
            for (int x = 0; x < bitmap.Width; x++)
            {
                if (bitmap.GetPixel(x, y).Red >= 128) continue;

                if (top == 0) top = y;
                bottom = y;
                if (left == 0 || x < left) left = x;
            }
        }

        // A 48 pt em is 100 px at 150 dpi and a capital reaches about two thirds of it, so the
        // top of an "H" lands around row 234 and never below the baseline at 300.
        top.ShouldBeInRange(215, 299, "the ink should start above the baseline");
        bottom.ShouldBeInRange(295, 302, "and a run of capitals and x-height letters should end on it");
        left.ShouldBeInRange(148, 156, "and start at the pen");
    }

    [Fact]
    public void TheSamePageRasterisedTwiceIsTheSameFile()
    {
        static IPage Page() => new DrawnPage(
            DrawnPage.A4,
            sink => sink.FillPath(
                GraphicsPath.Rectangle(new DocRect(
                    Length.FromInches(1), Length.FromInches(1),
                    Length.FromInches(2), Length.FromInches(1))),
                Paint.Solid(new Colour(40, 80, 160))));

        // The reason PNG is the format that matters: lossless and byte-deterministic, so a
        // checksum means something and a golden image can be committed.
        Encode(Page(), RasterFormat.Png).ShouldBe(Encode(Page(), RasterFormat.Png));
    }

    [Fact]
    public void PngIsLosslessAndJpegIsNot()
    {
        IPage page = new DrawnPage(
            DrawnPage.A4,
            sink => sink.FillPath(
                GraphicsPath.Rectangle(new DocRect(
                    Length.FromInches(1), Length.FromInches(1),
                    Length.FromInches(2), Length.FromInches(1))),
                Paint.Solid(new Colour(40, 80, 160))));

        byte[] png = Encode(page, RasterFormat.Png);
        byte[] jpeg = Encode(page, RasterFormat.Jpeg);

        png.Take(8).ShouldBe(new byte[] { 0x89, (byte)'P', (byte)'N', (byte)'G', 0x0D, 0x0A, 0x1A, 0x0A });
        jpeg.Take(2).ShouldBe(new byte[] { 0xFF, 0xD8 });

        // And the reason JPEG must not be compared against anything: it does not give back what
        // it was given. The fill is a flat colour and comes back a different one.
        using SKBitmap fromPng = SKBitmap.Decode(png);
        using SKBitmap fromJpeg = SKBitmap.Decode(jpeg);

        fromPng.GetPixel(300, 250).ShouldBe(new SKColor(40, 80, 160, 255));
        fromJpeg.GetPixel(300, 250).ShouldNotBe(new SKColor(40, 80, 160, 255));
    }

    [Fact]
    public void ATransparencyGroupIsCompositedAsAWhole()
    {
        const double dpi = 150;
        using SKBitmap bitmap = new RasterRenderer(new RasterRenderOptions { Dpi = dpi })
            .Rasterise(new DrawnPage(
                DrawnPage.A4,
                sink =>
                {
                    sink.BeginTransparencyGroup(0.5);
                    sink.FillPath(Square(1, 1, 2), Paint.Solid(Colour.Black));
                    sink.FillPath(Square(2, 1, 2), Paint.Solid(Colour.Black));
                    sink.EndTransparencyGroup();
                }));

        // The two squares overlap between two and three inches across. A group at half opacity
        // shows one flat half-tone there; two half-opacity fills would show a darker patch where
        // they meet, and that difference is the whole reason the drawing IR has groups at all.
        byte single = bitmap.GetPixel(225, 225).Red;
        byte overlapping = bitmap.GetPixel(375, 225).Red;

        single.ShouldBeInRange((byte)120, (byte)136);
        overlapping.ShouldBe(single);
    }

    // ------------------------------------------------------------------------- the machinery

    private static GraphicsPath Square(double leftInches, double topInches, double sizeInches)
        => GraphicsPath.Rectangle(new DocRect(
            Length.FromInches(leftInches), Length.FromInches(topInches),
            Length.FromInches(sizeInches), Length.FromInches(sizeInches)));

    private static byte[] Encode(IPage page, RasterFormat format)
    {
        using MemoryStream buffer = new();
        new RasterRenderer(new RasterRenderOptions { Dpi = 150 }).Render(page, buffer, format);
        return buffer.ToArray();
    }
}
