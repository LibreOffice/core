using System.Diagnostics;
using System.Globalization;
using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Rendering.Images;
using Paperless.Rendering.Pdf;
using Paperless.Rendering.Raster;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Shouldly;
using SkiaSharp;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Gradients, tiled bitmaps and pictures, against LibreOffice's rendering of the same slide.
/// </summary>
/// <remarks>
/// <para>
/// <b>Why the display list is built here rather than read out of the document.</b> Nothing in
/// the readers emits a <c>GradientPaint</c> or a <c>BitmapPaint</c> yet — the slide layouters
/// deliberately leave a gradient unpainted rather than guess a colour — so what is under test
/// is the two <em>backends</em>, and a page built by hand is the only way to reach them. The
/// page mirrors <c>tests/corpus/features/paint-fills.fodp</c> exactly: same slide size, same
/// four rectangles, same colours. When a reader does start emitting these paints, the same
/// comparison run against its output is what will show the mapping is right.
/// </para>
/// <para>
/// <b>The two sides do not state a gradient the same way, and cannot be made to.</b>
/// LibreOffice's drawing layer decomposes every shape gradient into flat bands before the PDF
/// writer sees it — <c>VclMetafileProcessor2D::processPolyPolygonGradientPrimitive2D</c>
/// (<c>drawinglayer/source/processor2d/vclmetafileprocessor2d.cxx</c>): <c>"tdf#150551 for PDF
/// export, use the decomposition for better gradient visualization"</c> — so its own shading
/// writer (<c>pdfwriter_impl.cxx:7965</c>) is unreachable from Impress and its PDF of this
/// slide holds no shading dictionary at all. Measured: 91602 bytes of page-one content stream
/// on its side against 2570 on ours, which states three shadings instead. So the comparison
/// that can be made is not operator for operator but picture for picture, and it is made
/// twice — once on what we rasterise ourselves and once on what a third-party rasteriser
/// makes of our PDF.
/// </para>
/// </remarks>
public sealed class PaintFillComparisonTests : IDisposable
{
    private const double Dpi = 150;

    /// <summary>The default 16:9 Impress slide, which is what the corpus document uses.</summary>
    private static readonly DocSize Slide = new(Cm(28), Cm(15.75));

    private static readonly DocRect LinearRect = new(Cm(2), Cm(2), Cm(8), Cm(5));
    private static readonly DocRect AxialRect = new(Cm(14), Cm(2), Cm(8), Cm(5));
    private static readonly DocRect RadialRect = new(Cm(2), Cm(9), Cm(8), Cm(5));
    private static readonly DocRect BitmapRect = new(Cm(14), Cm(9), Cm(8), Cm(5));
    private static readonly DocRect PictureRect = new(Cm(4), Cm(3), Cm(8), Cm(6));

    private static readonly Colour Red = Colour.FromRgb(0xFF0000);
    private static readonly Colour Blue = Colour.FromRgb(0x0000FF);
    private static readonly Colour Green = Colour.FromRgb(0x008000);
    private static readonly Colour Yellow = Colour.FromRgb(0xFFFF00);
    private static readonly Colour Cyan = Colour.FromRgb(0x00C0C0);
    private static readonly Colour Ink = Colour.FromRgb(0x101010);

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-paint").FullName;

    public void Dispose()
    {
        _libreOffice.Dispose();
        try
        {
            Directory.Delete(_workDirectory, recursive: true);
        }
        catch (IOException)
        {
            // A leftover temporary directory is not worth failing a test over.
        }
    }

    [Fact]
    public void EachGradientBecomesOneShadingDictionaryStatingItsOwnEnds()
    {
        string pdf = OurPdf();
        List<PdfShading> shadings = PdfPaints.ReadShadings(pdf);

        shadings.Count.ShouldBe(3, "one shading per gradient, and nothing else");

        // Linear: an axial shading whose axis is the rectangle's vertical centre line, top to
        // bottom, because ODF's angle of zero puts the start colour at the top.
        shadings[0].Type.ShouldBe(2);
        shadings[0].StartColour.ShouldBe(0xFF0000u);
        shadings[0].EndColour.ShouldBe(0x0000FFu);
        shadings[0].Coords.Count.ShouldBe(4);
        shadings[0].Coords[1].ShouldBe(Points(Slide.Height - LinearRect.Top), 0.01);
        shadings[0].Coords[3].ShouldBe(Points(Slide.Height - LinearRect.Bottom), 0.01);

        // Axial: three stops, so the function stitches two exponentials and its ends are the
        // outer colour twice. A two-colour shading could not say this at all, which is the
        // reason the stitched form is written rather than LibreOffice's sampled one.
        shadings[1].Type.ShouldBe(2);
        shadings[1].StartColour.ShouldBe(0xFFFF00u);
        shadings[1].EndColour.ShouldBe(0xFFFF00u);

        // Radial: type 3, a circle of zero radius at the centre growing to the outer one.
        shadings[2].Type.ShouldBe(3);
        shadings[2].Coords.Count.ShouldBe(6);
        shadings[2].Coords[2].ShouldBe(0);
        shadings[2].Coords[5].ShouldBe(Points(RadialDistance()), 0.01);
        shadings[2].StartColour.ShouldBe(0x101010u, "the centre takes the first stop");
        shadings[2].EndColour.ShouldBe(0x00C0C0u);
    }

    [Fact]
    public void EveryGradientIsClippedToTheShapeItFills()
    {
        List<PdfShading> shadings = PdfPaints.ReadShadings(OurPdf());

        // sh paints the whole clip, so the clip is the shape. Getting this wrong is not subtle —
        // an unclipped shading floods the page — but it is invisible to a comparison that only
        // reads the shading dictionary, which states an infinite axis and says nothing about
        // where it stops.
        foreach ((PdfShading shading, DocRect expected) in
                 shadings.Zip((DocRect[])[LinearRect, AxialRect, RadialRect]))
        {
            PdfBox clip = shading.Clip.ShouldNotBeNull();
            clip.Left.ShouldBe(Points(expected.Left), 0.01);
            clip.Top.ShouldBe(Points(expected.Top), 0.01);
            clip.Width.ShouldBe(Points(expected.Width), 0.01);
            clip.Height.ShouldBe(Points(expected.Height), 0.01);
        }
    }

    [Fact]
    public void ATiledFillWritesTheBitmapOnceAndPlacesItManyTimes()
    {
        List<PdfImageDraw> draws = PdfPaints.ReadImageDraws(OurPdf());

        List<PdfImageDraw> tiles = [.. draws.Where(draw => draw.PageIndex == 0)];
        tiles.Count.ShouldBeGreaterThan(30, "an 8 by 5 cm shape holds forty-odd one-centimetre tiles");
        tiles.Select(tile => tile.Resource).Distinct().Count()
            .ShouldBe(1, "every tile draws the same XObject; writing it once is the point");

        foreach (PdfImageDraw tile in tiles)
        {
            tile.PixelWidth.ShouldBe(8);
            tile.PixelHeight.ShouldBe(8);
            tile.Box.Width.ShouldBe(Points(Cm(1)), 0.01);
            tile.Box.Height.ShouldBe(Points(Cm(1)), 0.01);
        }

        // Every tile has to touch the shape, or the grid was anchored somewhere else entirely.
        tiles.ShouldAllBe(tile =>
            tile.Box.Left < Points(BitmapRect.Right) && tile.Box.Left + tile.Box.Width > Points(BitmapRect.Left));
    }

    [Fact]
    public void AnEmbeddedPictureBecomesOneImageXObjectAtTheFramesPlace()
    {
        PdfImageDraw picture = PdfPaints.ReadImageDraws(OurPdf())
            .Where(draw => draw.PageIndex == 1)
            .ShouldHaveSingleItem();

        picture.PixelWidth.ShouldBe(16);
        picture.PixelHeight.ShouldBe(12);
        picture.Filter.ShouldBe("FlateDecode", "a PNG is decoded and re-deflated; only a JPEG passes through");
        picture.HasSoftMask.ShouldBeFalse("the picture is opaque, so no /SMask is worth its stream");

        picture.Box.Left.ShouldBe(Points(PictureRect.Left), 0.01);
        picture.Box.Top.ShouldBe(Points(PictureRect.Top), 0.01);
        picture.Box.Width.ShouldBe(Points(PictureRect.Width), 0.01);
        picture.Box.Height.ShouldBe(Points(PictureRect.Height), 0.01);
    }

    [Fact]
    public void LibreOfficeWritesNoShadingAtAllForTheSameSlide()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string reference = _libreOffice.ConvertToPdf(Corpus.Require("paint-fills.fodp"), _workDirectory);

        // Pinned as a test rather than left as a remark, because it is the reason this file
        // compares pictures instead of operators, and because a LibreOffice release that
        // reverted tdf#150551 would make an operator comparison possible and should be noticed
        // rather than assumed away.
        PdfPaints.ReadShadings(reference).ShouldBeEmpty(
            "Impress decomposes a shape gradient into flat bands before the PDF writer sees it");

        // The picture and the checkerboard still arrive as image XObjects, which is why the
        // image half of this comparison can be made operator for operator and the gradient half
        // cannot.
        PdfPaints.ReadImageDraws(reference).ShouldNotBeEmpty();
    }

    [Fact]
    public void EveryPageRasterisesToTheSameImageAsLibreOffices()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        List<string> reference = Reference(Corpus.Require("paint-fills.fodp"));
        Assert.SkipWhen(reference.Count == 0, "pdftoppm is not available; install poppler-utils");

        RasterRenderer renderer = new(new RasterRenderOptions { Dpi = Dpi });
        List<IPage> pages = Pages();

        pages.Count.ShouldBe(reference.Count);

        for (int i = 0; i < pages.Count; i++)
        {
            using SKBitmap ours = renderer.Rasterise(pages[i]);
            using SKBitmap theirs = SKBitmap.Decode(reference[i]);

            (double error, double inkOurs, double inkTheirs) = Compare(ours, theirs, $"page {i + 1}");

            (inkOurs / inkTheirs).ShouldBeInRange(0.95, 1.05, $"page {i + 1}: ink");

            // Measured, not chosen. Page one — three gradients and a checkerboard — comes out at
            // 0.0016 against LibreOffice's banded rendering of the same slide, and page two at
            // 0.0018. The second is entirely the picture: LibreOffice's own renderer interpolates
            // a magnified bitmap exactly as Skia does, but poppler rasterises the PDF's image
            // XObject with no interpolation at all, so a 16 by 12 picture blown up to 8 by 6 cm
            // comes back as hard blocks on the reference side. See "Known deviations".
            error.ShouldBeLessThan(0.005, $"page {i + 1}: mean absolute difference {error:F4}");
        }
    }

    [Fact]
    public void OurPdfRasterisesToTheSameImageAsLibreOfficesPdf()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        List<string> reference = Reference(Corpus.Require("paint-fills.fodp"));
        Assert.SkipWhen(reference.Count == 0, "pdftoppm is not available; install poppler-utils");

        List<string> ours = Rasterise(OurPdf(), "ours");
        ours.Count.ShouldBe(reference.Count);

        for (int i = 0; i < ours.Count; i++)
        {
            using SKBitmap mine = SKBitmap.Decode(ours[i]);
            using SKBitmap theirs = SKBitmap.Decode(reference[i]);

            (double error, _, _) = Compare(mine, theirs, $"page {i + 1}");

            // The sharpest measurement in this file, and the reason it is worth making separately
            // from the one above: the same rasteriser reads both PDFs, so the antialiasing and
            // image-filtering differences cancel and what is left is the drawing. Measured 0.0007
            // on page one and 0.0000 — identical, pixel for pixel — on the picture page.
            error.ShouldBeLessThan(
                0.002, $"page {i + 1}: one rasteriser, two PDFs, mean absolute difference {error:F4}");
        }
    }

    // ------------------------------------------------------------------------- the display list

    /// <summary>
    /// The two pages of <c>paint-fills.fodp</c>, stated as a display list.
    /// </summary>
    /// <remarks>
    /// The three gradients' geometry is ODF's, resolved the way LibreOffice resolves it, since
    /// the point of the comparison is lost if the two sides are asked to draw different shapes:
    /// a linear gradient at angle zero runs top to bottom; an axial one is a linear ramp with
    /// the outer colour at both ends and the inner one halfway; and a radial one's outer circle
    /// has the radius <c>Gradient::GetBoundRect</c> gives it — half the <em>diagonal</em> of the
    /// shape, <c>hypot(w, h)</c> being the side of the square it builds
    /// (<c>vcl/source/gdi/gradient.cxx:246-251</c>). Choosing half the width instead moves the
    /// mean absolute error on that page from 0.0016 to 0.0054, which is what measuring rather
    /// than assuming this was worth.
    /// </remarks>
    private static List<IPage> Pages()
    {
        RasterImage tile = Decode("tile");
        RasterImage picture = Decode("picture");

        DrawnFills first = new(Slide, sink =>
        {
            sink.FillPath(GraphicsPath.Rectangle(LinearRect), new GradientPaint(
                GradientKind.Linear,
                [new GradientStop(0, Red), new GradientStop(1, Blue)],
                new DocPoint(Middle(LinearRect).X, LinearRect.Top),
                new DocPoint(Middle(LinearRect).X, LinearRect.Bottom),
                AffineTransform.Identity));

            sink.FillPath(GraphicsPath.Rectangle(AxialRect), new GradientPaint(
                GradientKind.Linear,
                [new GradientStop(0, Yellow), new GradientStop(0.5, Green), new GradientStop(1, Yellow)],
                new DocPoint(Middle(AxialRect).X, AxialRect.Top),
                new DocPoint(Middle(AxialRect).X, AxialRect.Bottom),
                AffineTransform.Identity));

            sink.FillPath(GraphicsPath.Rectangle(RadialRect), new GradientPaint(
                GradientKind.Radial,
                [new GradientStop(0, Ink), new GradientStop(1, Cyan)],
                Middle(RadialRect),
                new DocPoint(Middle(RadialRect).X + RadialDistance(), Middle(RadialRect).Y),
                AffineTransform.Identity));

            sink.FillPath(GraphicsPath.Rectangle(BitmapRect), new BitmapPaint(
                tile,
                new DocSize(Cm(1), Cm(1)),
                new DocPoint(Middle(BitmapRect).X - (Cm(1) / 2), Middle(BitmapRect).Y - (Cm(1) / 2)),
                Stretch: false));
        });

        DrawnFills second = new(Slide, sink => sink.DrawImage(picture, PictureRect)) { Index = 1 };

        return [first, second];
    }

    /// <summary>
    /// A one-centimetre checkerboard and a small picture, decoded from the corpus document.
    /// </summary>
    /// <remarks>
    /// Taken out of the flat ODF rather than kept as separate files, so the bytes this draws are
    /// provably the bytes LibreOffice drew — a second copy could drift from it silently.
    /// </remarks>
    private static RasterImage Decode(string which)
    {
        string document = File.ReadAllText(Corpus.Require("paint-fills.fodp"));
        string marker = which == "tile" ? "draw:fill-image" : "draw:image";

        int at = document.IndexOf(marker, StringComparison.Ordinal);
        int start = document.IndexOf("<office:binary-data>", at, StringComparison.Ordinal)
                    + "<office:binary-data>".Length;
        int end = document.IndexOf("</office:binary-data>", start, StringComparison.Ordinal);

        byte[] bytes = Convert.FromBase64String(document[start..end]);
        return RasterImageDecoder.Decode(bytes).ShouldNotBeNull();
    }

    private static DocPoint Middle(DocRect rect)
        => new(rect.Left + (rect.Width / 2), rect.Top + (rect.Height / 2));

    /// <summary>Half the diagonal of the radial gradient's shape, which is its outer radius.</summary>
    private static Length RadialDistance() => Length.FromEmu((long)(Math.Sqrt(
        ((double)RadialRect.Width.Emu * RadialRect.Width.Emu)
        + ((double)RadialRect.Height.Emu * RadialRect.Height.Emu)) / 2));

    private static Length Cm(double value) => Length.FromMillimetres(value * 10);

    private static double Points(Length value) => value.Points;

    // ------------------------------------------------------------------------------ the machinery

    private string OurPdf()
    {
        string path = Path.Combine(_workDirectory, "paint-fills-ours.pdf");
        if (File.Exists(path)) return path;

        using FileStream file = File.Create(path);
        new PdfRenderer(new PdfRenderOptions { CreationDate = DateTimeOffset.UnixEpoch })
            .Render(Pages(), file);

        return path;
    }

    private List<string> Reference(string documentPath)
        => Rasterise(_libreOffice.ConvertToPdf(documentPath, _workDirectory), "reference");

    private List<string> Rasterise(string pdfPath, string tag)
    {
        string prefix = Path.Combine(_workDirectory, tag + "-page");

        ProcessStartInfo start = new("pdftoppm") { RedirectStandardError = true };
        start.ArgumentList.Add("-r");
        start.ArgumentList.Add(Dpi.ToString(CultureInfo.InvariantCulture));
        start.ArgumentList.Add("-png");
        start.ArgumentList.Add(pdfPath);
        start.ArgumentList.Add(prefix);

        try
        {
            using Process? process = Process.Start(start);
            if (process is null) return [];

            process.WaitForExit((int)LibreOfficeRunner.Timeout.TotalMilliseconds);
            if (process.ExitCode != 0) return [];
        }
        catch (System.ComponentModel.Win32Exception)
        {
            return [];
        }

        return [.. Directory.GetFiles(_workDirectory, tag + "-page-*.png").Order(StringComparer.Ordinal)];
    }

    /// <summary>The mean absolute difference between two images, and how much ink each carries.</summary>
    private static (double Error, double InkOurs, double InkTheirs) Compare(
        SKBitmap ours, SKBitmap theirs, string where)
    {
        // Size first: a page-size disagreement makes every other number meaningless, and the
        // pixel count is the ceiling of the page size rather than the nearest whole pixel, which
        // poppler decides and one pixel of which loses the whole comparison.
        ours.Width.ShouldBe(theirs.Width, $"{where}: width in pixels");
        ours.Height.ShouldBe(theirs.Height, $"{where}: height in pixels");

        long total = 0, inkOurs = 0, inkTheirs = 0;

        for (int y = 0; y < ours.Height; y++)
        {
            for (int x = 0; x < ours.Width; x++)
            {
                SKColor a = ours.GetPixel(x, y);
                SKColor b = theirs.GetPixel(x, y);

                // Per channel rather than on a luminance, because a gradient can be wrong in hue
                // while its brightness is right — a red-to-blue ramp drawn blue-to-red has almost
                // the same luminance profile and a completely different picture.
                total += Math.Abs(a.Red - b.Red) + Math.Abs(a.Green - b.Green) + Math.Abs(a.Blue - b.Blue);
                inkOurs += 765 - a.Red - a.Green - a.Blue;
                inkTheirs += 765 - b.Red - b.Green - b.Blue;
            }
        }

        long samples = (long)ours.Width * ours.Height * 3;
        return (total / (double)samples / 255.0, inkOurs, Math.Max(inkTheirs, 1));
    }

    /// <summary>A page built by hand, since no reader emits these paints yet.</summary>
    private sealed class DrawnFills(DocSize size, Action<IDrawingSink> draw) : IPage
    {
        public int Index { get; init; }

        public DocSize Size => size;

        public string? Label => null;

        public void Draw(IDrawingSink sink)
        {
            sink.BeginPage(size);
            draw(sink);
            sink.EndPage();
        }
    }
}
