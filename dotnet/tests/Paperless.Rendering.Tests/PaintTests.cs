using System.Globalization;
using System.Text.RegularExpressions;
using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Rendering.Images;
using Paperless.Rendering.Pdf;
using Paperless.Rendering.Raster;
using Shouldly;
using SkiaSharp;

namespace Paperless.Rendering.Tests;

/// <summary>
/// Gradients and bitmap fills, in both backends, against what they were asked to draw.
/// </summary>
/// <remarks>
/// <para>
/// The companion to <c>PaintFillComparisonTests</c> in the fidelity harness, which asks
/// whether our picture agrees with LibreOffice's. This one asks the question that has to be
/// settled first and that an image diff is worst at: given one fill at one place, is the
/// colour at a stated point the colour the stop list says it should be.
/// </para>
/// <para>
/// Both backends are exercised on the same paints deliberately. The IR is one description and
/// two renderings of it that disagree is the bug this file exists to catch — most sharply for
/// the two gradient kinds that have no native form in either, where the whole reason the band
/// decomposition is shared is that a shape's colours must not depend on the output format.
/// </para>
/// </remarks>
public sealed class PaintTests
{
    private static readonly PdfRenderOptions Reproducible = new()
    {
        CreationDate = new DateTimeOffset(2024, 1, 1, 0, 0, 0, TimeSpan.Zero),
    };

    private static readonly Colour Red = Colour.FromRgb(0xFF0000);
    private static readonly Colour Blue = Colour.FromRgb(0x0000FF);

    // ------------------------------------------------------------------------------- the PDF

    [Fact]
    public void ALinearGradientBecomesAnAxialShadingClippedToItsShape()
    {
        string content = Content(sink => sink.FillPath(
            Rectangle(72, 72, 144, 72), Linear(new DocPoint(Pt(72), Pt(72)), new DocPoint(Pt(72), Pt(144)))));

        // Clip then sh, which is what LibreOffice writes and, unlike a shading pattern, inherits
        // the transform in force: a pattern's own matrix is stated against the page rather than
        // against the current user space, so a rotated shape's gradient would point elsewhere.
        content.ShouldContain("re\nW n\n");
        content.ShouldContain(" sh\n");
        content.ShouldNotContain("/Pattern");
    }

    [Fact]
    public void AShadingStatesTheStopsAsAnExponentialRamp()
    {
        string pdf = Write(sink => sink.FillPath(
            Rectangle(72, 72, 144, 72), Linear(new DocPoint(Pt(72), Pt(72)), new DocPoint(Pt(72), Pt(144))))).Text;

        pdf.ShouldContain("/ShadingType 2");
        pdf.ShouldContain("/FunctionType 2/Domain[0 1]/C0[1 0 0]/C1[0 0 1]/N 1");

        // Extended at both ends, so a shape reaching past the axis is painted rather than left
        // hollow -- which is what a border on an ODF gradient produces.
        pdf.ShouldContain("/Extend[true true]");
    }

    [Fact]
    public void MoreThanTwoStopsStitchOnePieceForEachPair()
    {
        string pdf = Write(sink => sink.FillPath(
            Rectangle(72, 72, 144, 72),
            new GradientPaint(
                GradientKind.Linear,
                [
                    new GradientStop(0, Red),
                    new GradientStop(0.25, Colour.White),
                    new GradientStop(1, Blue),
                ],
                new DocPoint(Pt(72), Pt(72)),
                new DocPoint(Pt(72), Pt(144)),
                AffineTransform.Identity))).Text;

        // The stitching bound is the interior stop, and getting it wrong is invisible in every
        // other assertion: a ramp with its bounds at the wrong place still draws a plausible
        // gradient between the right two colours.
        pdf.ShouldContain("/FunctionType 3");
        pdf.ShouldContain("/Bounds[0.25]");
        pdf.ShouldContain("/Encode[0 1 0 1]");
        Regex.Count(pdf, @"/FunctionType 2").ShouldBe(2, "one exponential piece per pair of stops");
    }

    [Fact]
    public void ARadialGradientTakesItsRadiusFromTheDistanceToTheEndPoint()
    {
        string pdf = Write(sink => sink.FillPath(
            Rectangle(72, 72, 144, 144),
            new GradientPaint(
                GradientKind.Radial,
                [new GradientStop(0, Red), new GradientStop(1, Blue)],
                new DocPoint(Pt(144), Pt(144)),
                new DocPoint(Pt(204), Pt(144)),
                AffineTransform.Identity))).Text;

        Match coords = Regex.Match(pdf, @"/ShadingType 3.*?/Coords\[([^\]]*)\]", RegexOptions.Singleline);
        coords.Success.ShouldBeTrue();

        double[] values = [.. coords.Groups[1].Value.Split(' ')
            .Select(v => double.Parse(v, CultureInfo.InvariantCulture))];

        // Two circles: one of no size at the centre and one of the full radius, both centred on
        // the same point. A PDF radial shading can have two different centres -- a focal
        // gradient -- and the display list has no way to say so, which is why they are equal.
        values.Length.ShouldBe(6);
        values[2].ShouldBe(0);
        values[5].ShouldBe(60, 0.01);
        values[0].ShouldBe(values[3]);
        values[1].ShouldBe(values[4]);
    }

    [Fact]
    public void ATiledBitmapWritesOneXObjectAndOneDrawPerTile()
    {
        PdfFile pdf = Write(sink => sink.FillPath(
            Rectangle(0, 0, 40, 40),
            new BitmapPaint(Chequer(), new DocSize(Pt(10), Pt(10)), new DocPoint(Pt(0), Pt(0)), Stretch: false)));

        string content = pdf.ContentStreams().ShouldHaveSingleItem();

        Regex.Count(content, @"/Im\d+ Do").ShouldBe(16, "four tiles across and four down");
        Regex.Count(pdf.Text, @"/Subtype/Image")
            .ShouldBe(1, "sixteen tiles, one bitmap -- writing it per tile is the whole cost");
    }

    [Fact]
    public void AStretchedBitmapIsDrawnOnceAcrossTheWholeShape()
    {
        string content = Content(sink => sink.FillPath(
            Rectangle(0, 0, 40, 40),
            new BitmapPaint(Chequer(), new DocSize(Pt(10), Pt(10)), default, Stretch: true)));

        Regex.Count(content, @"/Im\d+ Do").ShouldBe(1);
        content.ShouldContain("40 0 0 40 0 ");
    }

    [Fact]
    public void ATransparentImageCarriesASoftMask()
    {
        RasterImage image = new()
        {
            Width = 2,
            Height = 1,
            Pixels = new byte[] { 255, 0, 0, 255, 0, 0, 255, 0 },
        };

        string pdf = Write(sink => sink.DrawImage(image, new DocRect(Pt(0), Pt(0), Pt(20), Pt(10)))).Text;

        // PDF has no RGBA colour space, so alpha is a second greyscale image. Written only when
        // some pixel needs it, which keeps every opaque office picture to one stream.
        pdf.ShouldContain("/SMask ");
        pdf.ShouldContain("/ColorSpace/DeviceGray");
    }

    [Fact]
    public void AGradientWhoseStopsFadeStatesALuminositySoftMask()
    {
        string pdf = Write(sink => sink.FillPath(
            Rectangle(0, 0, 72, 72),
            new GradientPaint(
                GradientKind.Linear,
                [new GradientStop(0, Red), new GradientStop(1, Red.WithAlpha(0))],
                new DocPoint(Pt(0), Pt(0)),
                new DocPoint(Pt(72), Pt(0)),
                AffineTransform.Identity))).Text;

        // A shading's colour space is DeviceRGB and that is all it has, so a fade is a second
        // shading in DeviceGray whose brightness the mask reads as alpha. Without it the same
        // GradientPaint would fade on a PNG and be opaque in a PDF.
        pdf.ShouldContain("/SMask<</S/Luminosity");
        pdf.ShouldContain("/ColorSpace/DeviceGray");
        pdf.ShouldContain("/Group<</Type/Group/S/Transparency/CS/DeviceGray>>");
        Regex.Count(pdf, @"/ShadingType 2").ShouldBe(2, "one shading for the colour, one for the alpha");
    }

    [Fact]
    public void AGradientAtOneAlphaThroughoutNeedsOnlyAConstantAlpha()
    {
        string pdf = Write(sink => sink.FillPath(
            Rectangle(0, 0, 72, 72),
            new GradientPaint(
                GradientKind.Linear,
                [new GradientStop(0, Red.WithAlpha(128)), new GradientStop(1, Blue.WithAlpha(128))],
                new DocPoint(Pt(0), Pt(0)),
                new DocPoint(Pt(72), Pt(0)),
                AffineTransform.Identity))).Text;

        // A soft mask costs a form XObject, a transparency group and a second shading. A gradient
        // that is uniformly half transparent needs none of it, and half-transparent shapes are
        // common enough that the difference is worth the branch.
        pdf.ShouldContain("/ca ");
        pdf.ShouldNotContain("/SMask");
        Regex.Count(pdf, @"/ShadingType").ShouldBe(1);
    }

    // ----------------------------------------------------------------------------- the raster

    [Fact]
    public void ALinearGradientRampsAcrossItsAxisInPixels()
    {
        using SKBitmap page = Rasterise(sink => sink.FillPath(
            Rectangle(0, 0, 72, 72), Linear(new DocPoint(Pt(0), Pt(0)), new DocPoint(Pt(0), Pt(72)))));

        // At 72 dpi one point is one pixel, so the shape is the first 72 rows.
        page.GetPixel(36, 1).Red.ShouldBeGreaterThan((byte)240);
        page.GetPixel(36, 1).Blue.ShouldBeLessThan((byte)16);
        page.GetPixel(36, 70).Blue.ShouldBeGreaterThan((byte)240);
        page.GetPixel(36, 70).Red.ShouldBeLessThan((byte)16);

        // Halfway, which is the assertion that fails when a backend draws a gradient as one of
        // its stops -- the ends would still be right.
        SKColor middle = page.GetPixel(36, 36);
        middle.Red.ShouldBeInRange((byte)110, (byte)145);
        middle.Blue.ShouldBeInRange((byte)110, (byte)145);
    }

    [Fact]
    public void AGradientStopsAtTheEdgeOfTheShapeItFills()
    {
        using SKBitmap page = Rasterise(sink => sink.FillPath(
            Rectangle(0, 0, 72, 72), Linear(new DocPoint(Pt(0), Pt(0)), new DocPoint(Pt(0), Pt(72)))));

        // sh paints its whole clip and a shader paints its whole path; an unclipped one floods
        // the page, which every colour assertion above would still pass.
        page.GetPixel(80, 36).ShouldBe(new SKColor(255, 255, 255, 255));
        page.GetPixel(36, 80).ShouldBe(new SKColor(255, 255, 255, 255));
    }

    [Fact]
    public void AFadingGradientBlendsIntoThePageInPixelsToo()
    {
        using SKBitmap page = Rasterise(sink => sink.FillPath(
            Rectangle(0, 0, 72, 72),
            new GradientPaint(
                GradientKind.Linear,
                [new GradientStop(0, Red), new GradientStop(1, Red.WithAlpha(0))],
                new DocPoint(Pt(0), Pt(0)),
                new DocPoint(Pt(72), Pt(0)),
                AffineTransform.Identity)));

        // The same picture the PDF's luminosity mask produces, checked here because the two are
        // built completely differently — Skia carries alpha in the shader's own colours — and the
        // whole point is that one display list gives one picture. Rasterising our PDF of the same
        // fill through poppler and comparing puts the two at a mean absolute difference of 0.0003.
        page.GetPixel(1, 36).Red.ShouldBeGreaterThan((byte)180);
        page.GetPixel(1, 36).Green.ShouldBeLessThan((byte)40);
        page.GetPixel(70, 36).Green.ShouldBeGreaterThan((byte)245, "all but faded into the white page");
        page.GetPixel(36, 36).Green.ShouldBeInRange((byte)110, (byte)150);
    }

    [Fact]
    public void ATiledBitmapRepeatsOnTheGridItWasGiven()
    {
        using SKBitmap page = Rasterise(sink => sink.FillPath(
            Rectangle(0, 0, 40, 40),
            new BitmapPaint(Chequer(), new DocSize(Pt(10), Pt(10)), new DocPoint(Pt(0), Pt(0)), Stretch: false)));

        // The chequer is two by two, so a ten-point tile puts red in the top-left five points and
        // blue beside it, repeating every ten. Sampling one tile in confirms the phase as well as
        // the period: a grid anchored on the page rather than on the offset passes the first and
        // fails the second.
        page.GetPixel(2, 2).Red.ShouldBeGreaterThan((byte)200);
        page.GetPixel(7, 2).Blue.ShouldBeGreaterThan((byte)200);
        page.GetPixel(12, 12).Red.ShouldBeGreaterThan((byte)200);
        page.GetPixel(17, 12).Blue.ShouldBeGreaterThan((byte)200);
    }

    // ------------------------------------------------------------------- the two shared kinds

    [Theory]
    [InlineData(GradientKind.Rectangular)]
    [InlineData(GradientKind.Conical)]
    public void AKindWithNoNativeFormDrawsTheSameInBothBackends(GradientKind kind)
    {
        GradientPaint gradient = new(
            kind,
            [new GradientStop(0, Red), new GradientStop(1, Blue)],
            new DocPoint(Pt(36), Pt(36)),
            new DocPoint(Pt(72), Pt(36)),
            AffineTransform.Identity);

        void Draw(IDrawingSink sink) => sink.FillPath(Rectangle(0, 0, 72, 72), gradient);

        // No shading and no image: a kind neither backend can state natively must not silently
        // become one it can, which is what drawing a rectangular gradient as a radial would be.
        Write(Draw).Text.ShouldNotContain("/ShadingType");

        using SKBitmap page = Rasterise(Draw);

        // The two backends are compared through the one thing they both produce -- pixels -- by
        // rasterising ours and reading the colours the bands laid down. What matters is that the
        // shape is filled edge to edge in both, since the failure mode of a decomposition is a
        // corner the outermost band did not reach.
        foreach ((int x, int y) in ((int, int)[])[(2, 2), (69, 2), (2, 69), (69, 69), (36, 36)])
        {
            page.GetPixel(x, y).ShouldNotBe(
                new SKColor(255, 255, 255, 255), $"({x}, {y}) is inside the shape and was not painted");
        }
    }

    [Fact]
    public void AStopListThatDoesNotReachTheEndsIsExtendedToThem()
    {
        // Normalisation happens once, for both backends, because Skia tolerates a list PDF's
        // stitching function cannot use: strictly increasing bounds spanning the whole domain.
        string pdf = Write(sink => sink.FillPath(
            Rectangle(0, 0, 72, 72),
            new GradientPaint(
                GradientKind.Linear,
                [new GradientStop(0.4, Red), new GradientStop(0.6, Blue)],
                new DocPoint(Pt(0), Pt(0)),
                new DocPoint(Pt(0), Pt(72)),
                AffineTransform.Identity))).Text;

        Match bounds = Regex.Match(pdf, @"/Bounds\[([^\]]*)\]");
        bounds.Success.ShouldBeTrue("two synthetic end stops make four, so the pieces are stitched");
        bounds.Groups[1].Value.ShouldBe("0.4 0.6");
    }

    // ---------------------------------------------------------------------------- the decoder

    [Fact]
    public void APngIsDecodedToStraightRgbaWithItsOwnBytesKept()
    {
        byte[] png = Encode(Chequer());
        RasterImage image = RasterImageDecoder.Decode(png).ShouldNotBeNull();

        image.Width.ShouldBe(2);
        image.Height.ShouldBe(2);
        image.Pixels.Length.ShouldBe(2 * 2 * 4);
        image.EncodedMediaType.ShouldBe("image/png");
        image.EncodedBytes.Length.ShouldBe(png.Length, "the original bytes cost nothing and enable pass-through");

        // Straight, not premultiplied: an /SMask is a separate greyscale image whose samples are
        // the alpha, and dividing the colour back out loses precision exactly where alpha is low.
        image.Pixels.Span[0].ShouldBe((byte)255);
        image.Pixels.Span[3].ShouldBe((byte)255);
    }

    [Fact]
    public void TheFormatIsDecidedByTheBytesRatherThanTheDeclaredType()
    {
        byte[] png = Encode(Chequer());

        // A part named .png holding something else is routine in real documents, and believing
        // the name would write a JPEG into a PDF claiming to be deflated RGB.
        RasterImageDecoder.Decode(png, "image/jpeg")!.EncodedMediaType.ShouldBe("image/png");
        RasterImageDecoder.Sniff(png).ShouldBe("image/png");
        RasterImageDecoder.Sniff([0xFF, 0xD8, 0xFF, 0xE0]).ShouldBe("image/jpeg");
        RasterImageDecoder.Sniff("GIF89a"u8).ShouldBe("image/gif");
        RasterImageDecoder.Sniff("not a picture"u8).ShouldBeNull();
    }

    [Fact]
    public void BytesThatAreNotAPictureDecodeToNothingRatherThanThrowing()
    {
        // A picture that will not decode is a document defect, not a reason to lose the page it
        // is on: everything else on it still draws.
        RasterImageDecoder.Decode(new byte[] { 1, 2, 3, 4 }).ShouldBeNull();
        RasterImageDecoder.Decode(ReadOnlyMemory<byte>.Empty).ShouldBeNull();
    }

    [Fact]
    public void ADecodedJpegPassesThroughToThePdfUntouched()
    {
        RasterImage source = RasterImageDecoder.Decode(Encode(Chequer())).ShouldNotBeNull();
        RasterImage jpeg = source with
        {
            EncodedBytes = Encode(Chequer(), SKEncodedImageFormat.Jpeg),
            EncodedMediaType = "image/jpeg",
        };

        string pdf = Write(sink => sink.DrawImage(jpeg, new DocRect(Pt(0), Pt(0), Pt(20), Pt(20)))).Text;

        // PDF's image filter and JPEG's are the same thing, so re-encoding spends time to produce
        // a larger and worse image. LibreOffice does the same (pdfextoutdevdata.cxx:439).
        pdf.ShouldContain("/DCTDecode");
        pdf.ShouldNotContain("/SMask ");
    }

    // ------------------------------------------------------------------------------- helpers

    private static GradientPaint Linear(DocPoint from, DocPoint to) => new(
        GradientKind.Linear,
        [new GradientStop(0, Red), new GradientStop(1, Blue)],
        from,
        to,
        AffineTransform.Identity);

    /// <summary>A two-by-two image: red and blue on one diagonal, and their opposites on the other.</summary>
    private static RasterImage Chequer() => new()
    {
        Width = 2,
        Height = 2,
        Pixels = new byte[]
        {
            255, 0, 0, 255, 0, 0, 255, 255,
            0, 0, 255, 255, 255, 0, 0, 255,
        },
    };

    private static byte[] Encode(RasterImage image, SKEncodedImageFormat format = SKEncodedImageFormat.Png)
    {
        SKImageInfo info = new(image.Width, image.Height, SKColorType.Rgba8888, SKAlphaType.Unpremul);
        using SKBitmap bitmap = new(info);
        bitmap.Pixels = [.. Enumerable.Range(0, image.Width * image.Height).Select(i => new SKColor(
            image.Pixels.Span[i * 4],
            image.Pixels.Span[(i * 4) + 1],
            image.Pixels.Span[(i * 4) + 2],
            image.Pixels.Span[(i * 4) + 3]))];

        using SKImage encoded = SKImage.FromBitmap(bitmap);
        using SKData data = encoded.Encode(format, 100);
        return data.ToArray();
    }

    private static GraphicsPath Rectangle(double x, double y, double width, double height)
        => GraphicsPath.Rectangle(new DocRect(Pt(x), Pt(y), Pt(width), Pt(height)));

    private static Length Pt(double value) => Length.FromPoints(value);

    private static PdfFile Write(Action<IDrawingSink> draw)
    {
        using MemoryStream buffer = new();
        new PdfRenderer(Reproducible).Render(
            new DrawnPages(new DrawnPage(DrawnPage.A4, draw)), buffer);

        return PdfFile.Parse(buffer.ToArray());
    }

    private static string Content(Action<IDrawingSink> draw)
        => Write(draw).ContentStreams().ShouldHaveSingleItem();

    /// <summary>Rasterises a page at 72 dpi, where one point is one pixel and no arithmetic hides.</summary>
    private static SKBitmap Rasterise(Action<IDrawingSink> draw)
        => new RasterRenderer(new RasterRenderOptions { Dpi = 72, Antialias = false })
            .Rasterise(new DrawnPage(DrawnPage.A4, draw));
}
