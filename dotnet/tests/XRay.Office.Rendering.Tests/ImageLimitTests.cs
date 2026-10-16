using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Rendering.Images;
using XRay.Office.Rendering.Pdf;
using XRay.Office.Rendering.Raster;
using Shouldly;
using SkiaSharp;

namespace XRay.Office.Rendering.Tests;

/// <summary>
/// The limit on how large a picture is decoded: skipped, or decoded smaller, and never both.
/// </summary>
public sealed class ImageLimitTests
{
    private static byte[] Encode(int width, int height, SKEncodedImageFormat format)
    {
        using SKBitmap bitmap = new(width, height);
        bitmap.Erase(new SKColor(200, 30, 30));
        using SKData data = bitmap.Encode(format, 90);
        return data.ToArray();
    }

    private static RasterImage Picture(byte[] bytes, string mediaType)
        => new() { EncodedBytes = bytes, EncodedMediaType = mediaType };

    private static DrawnPage PageWith(RasterImage image)
        => new(
            new DocSize(Length.FromPoints(200), Length.FromPoints(200)),
            sink => sink.DrawImage(image, new DocRect(Length.FromPoints(10), Length.FromPoints(10), Length.FromPoints(100), Length.FromPoints(100))));

    [Fact]
    public void AnImageWithinTheLimitIsDecodedAsBefore()
    {
        RasterImage? image = RasterImageDecoder.Ensure(Picture(Encode(40, 30, SKEncodedImageFormat.Png), "image/png"), 2_000, OversizedImagePolicy.Skip);

        image.ShouldNotBeNull();
        (image.Width, image.Height).ShouldBe((40, 30));
    }

    [Fact]
    public void AnImageOverTheLimitIsSkippedWithoutBeingDecoded()
    {
        RasterImage picture = Picture(Encode(400, 300, SKEncodedImageFormat.Png), "image/png");

        RasterImageDecoder.Ensure(picture, 1_000, OversizedImagePolicy.Skip).ShouldBeNull();

        // A limit of zero is no limit, and wins nothing over a real one in `Stricter`.
        RasterImageDecoder.Ensure(picture, 0, OversizedImagePolicy.Skip).ShouldNotBeNull();
        RasterImageDecoder.Stricter(0, 5).ShouldBe(5);
        RasterImageDecoder.Stricter(7, 0).ShouldBe(7);
        RasterImageDecoder.Stricter(7, 5).ShouldBe(5);
        RasterImageDecoder.Stricter(0, 0).ShouldBe(0);
    }

    [Fact]
    public void AJpegOverTheLimitCanBeDecodedAtAReducedScaleInstead()
    {
        RasterImage picture = Picture(Encode(800, 600, SKEncodedImageFormat.Jpeg), "image/jpeg");

        RasterImage? reduced = RasterImageDecoder.Ensure(picture, 150_000, OversizedImagePolicy.Downscale);

        reduced.ShouldNotBeNull();
        ((long)reduced.Width * reduced.Height).ShouldBeLessThanOrEqualTo(150_000);
        reduced.Width.ShouldBe(400, "the largest of the codec's own scales that fits is half");
        reduced.EncodedBytes.IsEmpty.ShouldBeTrue("the full-size bytes must not travel on to a PDF");

        // A codec that cannot decode small gives nothing rather than a full decode and a shrink.
        RasterImageDecoder.Ensure(Picture(Encode(800, 600, SKEncodedImageFormat.Png), "image/png"), 150_000, OversizedImagePolicy.Downscale)
            .ShouldBeNull();

        // A limit below what the smallest scale reaches is a skip too.
        RasterImageDecoder.Ensure(picture, 100, OversizedImagePolicy.Downscale).ShouldBeNull();
    }

    [Fact]
    public void ThePdfRendererLeavesAnOversizedImageOutOfTheFile()
    {
        RasterImage big = Picture(Encode(300, 200, SKEncodedImageFormat.Png), "image/png");

        string with = Render(PageWith(big), new PdfRenderOptions());
        with.ShouldContain("/Subtype/Image");

        string without = Render(PageWith(big), new PdfRenderOptions { MaxImagePixels = 10_000 });
        without.ShouldNotContain("/Subtype/Image");
    }

    [Fact]
    public void ThePdfRendererWritesTheReducedJpegAtItsOwnSize()
    {
        RasterImage big = Picture(Encode(800, 600, SKEncodedImageFormat.Jpeg), "image/jpeg");

        string text = Render(
            PageWith(big),
            new PdfRenderOptions { MaxImagePixels = 150_000, OversizedImages = OversizedImagePolicy.Downscale });

        text.ShouldContain("/Width 400/Height 300");
        text.ShouldNotContain("/DCTDecode", Case.Sensitive, "the original JPEG bytes were not passed through");
    }

    [Fact]
    public void ThePageCarriesTheLimitChosenAtLayoutAndTheRendererAppliesIt()
    {
        RasterImage big = Picture(Encode(300, 200, SKEncodedImageFormat.Png), "image/png");
        LimitedPage page = new(PageWith(big), maxImagePixels: 10_000);

        Render(page, new PdfRenderOptions()).ShouldNotContain("/Subtype/Image");

        using SKBitmap bitmap = new RasterRenderer(new RasterRenderOptions { Dpi = 72 }).Rasterise(page);
        bitmap.GetPixel(50, 50).ShouldBe(new SKColor(255, 255, 255, 255), "the picture was not drawn");

        // And without the limit the same page draws it.
        using SKBitmap drawn = new RasterRenderer(new RasterRenderOptions { Dpi = 72 }).Rasterise(PageWith(big));
        ((int)drawn.GetPixel(50, 50).Red).ShouldBeGreaterThan(150);
        ((int)drawn.GetPixel(50, 50).Green).ShouldBeLessThan(100);
    }

    [Fact]
    public void TheRasterRendererHasALimitOfItsOwn()
    {
        RasterImage big = Picture(Encode(300, 200, SKEncodedImageFormat.Png), "image/png");

        using SKBitmap bitmap = new RasterRenderer(new RasterRenderOptions { Dpi = 72, MaxImagePixels = 10_000 })
            .Rasterise(PageWith(big));
        bitmap.GetPixel(50, 50).ShouldBe(new SKColor(255, 255, 255, 255));
    }

    [Fact]
    public void LayoutOptionsReachThePagesOfARealDocument()
    {
        // A document with a picture: the layout carries the limit onto its pages, so a renderer that is
        // told nothing still applies it.
        using IDocument document = OfficeDocument.Open(XRay.Office.TestKit.Corpus.Require("picture-crop.pptx"));
        IPageSequence limited = ((IPaginatedDocument)document).Layout(new LayoutOptions { MaxImagePixels = 4 });
        IPageSequence open = ((IPaginatedDocument)document).Layout();

        ((IPageRenderLimits)limited[0]).MaxImagePixels.ShouldBe(4);
        ((IPageRenderLimits)open[0]).MaxImagePixels.ShouldBe(0);

        Render(open[0], new PdfRenderOptions()).ShouldContain("/Subtype/Image");
        Render(limited[0], new PdfRenderOptions()).ShouldNotContain("/Subtype/Image");
    }

    // ------------------------------------------------------------------------------------------------

    private static string Render(IPage page, PdfRenderOptions options)
    {
        using MemoryStream output = new();
        new PdfRenderer(options).Render(new DrawnPages(page), output);
        return PdfFile.Parse(output.ToArray()).Text;
    }

    private sealed class LimitedPage(IPage inner, long maxImagePixels) : IPage, IPageRenderLimits
    {
        public int Index => inner.Index;
        public DocSize Size => inner.Size;
        public string? Label => inner.Label;
        public long MaxImagePixels => maxImagePixels;
        public OversizedImagePolicy OversizedImages => OversizedImagePolicy.Skip;
        public void Draw(IDrawingSink sink) => inner.Draw(sink);
    }
}
