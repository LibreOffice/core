using System.Diagnostics;
using System.Globalization;
using Paperless.Core.Documents;
using Paperless.Rendering.Pdf;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Shouldly;
using SkiaSharp;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// The gradients, tiled fills and pictures a slide <em>reader</em> produces, measured against
/// LibreOffice's rendering of the same document.
/// </summary>
/// <remarks>
/// <para>
/// The counterpart of <see cref="PaintFillComparisonTests"/> and deliberately separate from it.
/// That file builds its display list by hand, because when it was written nothing emitted a
/// <c>GradientPaint</c> or a <c>BitmapPaint</c> and the two backends were what was under test.
/// This one reads the document, so what is under test is the mapping from the file's own numbers
/// to that display list — which is where every format-specific trap lives, and where a hand-built
/// list can prove nothing at all.
/// </para>
/// <para>
/// <b>The same slide is read twice, once from each format.</b> <c>paint-fills-pptx.pptx</c> is
/// LibreOffice's own export of <c>paint-fills.fodp</c>, and the two files state the same three
/// gradients in opposite conventions: ODF's <c>draw:start-color</c> paints the outer edge of a
/// radial gradient and DrawingML's first <c>a:gs</c> paints the centre. Comparing both against
/// one reference is what makes a reader that has a convention backwards fail here rather than
/// look plausible.
/// </para>
/// <para>
/// <b>The comparison is picture for picture, not operator for operator, and it has to be.</b>
/// Impress decomposes every shape gradient into flat bands before its PDF writer sees one —
/// <c>VclMetafileProcessor2D::processPolyPolygonGradientPrimitive2D</c>, "tdf#150551 for PDF
/// export, use the decomposition for better gradient visualization" — so LibreOffice's PDF of
/// this slide holds no shading dictionary at all, against the three ours states. Measured: 91602
/// bytes of page-one content stream on its side against 2570 on ours.
/// </para>
/// </remarks>
public sealed class SlidePaintComparisonTests : IDisposable
{
    private const double Dpi = 150;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-slide-paint").FullName;

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

    [Theory]
    [InlineData("paint-fills.fodp")]
    [InlineData("paint-fills-pptx.pptx")]
    public void EveryPageRasterisesToTheSameImageAsLibreOffices(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        List<string> theirs = Rasterise(_libreOffice.ConvertToPdf(path, _workDirectory), "reference");
        Assert.SkipWhen(theirs.Count == 0, "pdftoppm is not available; install poppler-utils");

        List<string> ours = Rasterise(Ours(path), "ours");
        ours.Count.ShouldBe(theirs.Count, $"{fileName}: page count");

        for (int i = 0; i < ours.Count; i++)
        {
            using SKBitmap mine = SKBitmap.Decode(ours[i]);
            using SKBitmap reference = SKBitmap.Decode(theirs[i]);

            (double error, double inkOurs, double inkTheirs) =
                Compare(mine, reference, $"{fileName} page {i + 1}");

            (inkOurs / inkTheirs).ShouldBeInRange(0.95, 1.05, $"{fileName}: page {i + 1} ink");

            // Measured, not chosen, and the same rasteriser reads both files so what is left is
            // the drawing rather than the antialiasing. Page one — three gradients and a
            // checkerboard — comes out at 0.0007 from the flat ODF and 0.0009 from the PPTX; page
            // two, one picture, at 0.0001 from both. The bound is loose enough to survive a
            // LibreOffice point release and far tighter than any mapping error: a radial whose
            // ends are swapped measures 0.05 and a radius taken from the width rather than the
            // diagonal measures 0.0054.
            error.ShouldBeLessThan(
                0.002, $"{fileName}: page {i + 1}, mean absolute difference {error:F4}");
        }
    }

    [Theory]
    [InlineData("paint-fills.fodp")]
    [InlineData("paint-fills-pptx.pptx")]
    public void EachGradientBecomesOneShadingAndThePictureOneImage(string fileName)
    {
        string pdf = Ours(Corpus.Require(fileName));

        // Three gradients, one shading each. Worth asserting beside the picture comparison
        // because a reader that emitted no gradient at all and left the shape unpainted would
        // still be within the error bound for a small enough shape, and because a gradient
        // decomposed into bands would pass the picture test and lose every advantage of the
        // display list having a gradient in it.
        PdfPaints.ReadShadings(pdf).Count.ShouldBe(3, $"{fileName}: one shading per gradient");

        List<PdfImageDraw> draws = PdfPaints.ReadImageDraws(pdf);

        List<PdfImageDraw> tiles = [.. draws.Where(draw => draw.PageIndex == 0)];
        tiles.Count.ShouldBeGreaterThan(30, "an 8 by 5 cm shape holds forty-odd one-centimetre tiles");
        tiles.Select(tile => tile.Resource).Distinct().Count()
            .ShouldBe(1, "every tile draws the same XObject; writing it once is the point");
        tiles.ShouldAllBe(tile => tile.PixelWidth == 8 && tile.PixelHeight == 8);

        PdfImageDraw picture = draws.Where(draw => draw.PageIndex == 1).ShouldHaveSingleItem();
        picture.PixelWidth.ShouldBe(16);
        picture.PixelHeight.ShouldBe(12);

        // 4 cm by 3 cm, 8 cm by 6 cm, which is the frame the document states — so the picture is
        // neither cropped nor stretched to the shape's aspect by accident.
        picture.Box.Left.ShouldBe(Millimetres(40), 0.05);
        picture.Box.Width.ShouldBe(Millimetres(80), 0.05);
        picture.Box.Height.ShouldBe(Millimetres(60), 0.05);
    }

    [Fact]
    public void TheTwoFormatsOfOneSlideDrawTheSamePicture()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        List<string> odf = Rasterise(Ours(Corpus.Require("paint-fills.fodp")), "odf");
        Assert.SkipWhen(odf.Count == 0, "pdftoppm is not available; install poppler-utils");

        List<string> ooxml = Rasterise(Ours(Corpus.Require("paint-fills-pptx.pptx")), "ooxml");
        ooxml.Count.ShouldBe(odf.Count);

        // The sharpest question this corpus can be asked, and the one no single-format comparison
        // can. The two files are the same slide written twice by LibreOffice, so any difference
        // here is one of the two readers having a convention backwards — and unlike a comparison
        // against the reference, there is no antialiasing, no band decomposition and no image
        // filtering between the two sides to hide behind.
        for (int i = 0; i < odf.Count; i++)
        {
            using SKBitmap first = SKBitmap.Decode(odf[i]);
            using SKBitmap second = SKBitmap.Decode(ooxml[i]);

            (double error, _, _) = Compare(first, second, $"page {i + 1}");
            error.ShouldBeLessThan(0.001, $"page {i + 1}: ODF against OOXML, {error:F4}");
        }
    }

    private static double Millimetres(double value)
        => Core.Units.Length.FromMillimetres(value).Points;

    private string Ours(string documentPath)
    {
        string destination = Path.Combine(
            _workDirectory, $"{Path.GetFileNameWithoutExtension(documentPath)}-paperless.pdf");

        if (File.Exists(destination)) return destination;

        using IDocument document = PaperlessDocument.Open(documentPath);
        IPageSequence pages = ((IPaginatedDocument)document).Layout();

        using FileStream output = File.Create(destination);
        new PdfRenderer(new PdfRenderOptions { CreationDate = DateTimeOffset.UnixEpoch })
            .Render(pages, output);

        return destination;
    }

    /// <summary>
    /// Rasterises a PDF with poppler, so that both sides of a comparison go through one
    /// rasteriser and the antialiasing cancels.
    /// </summary>
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
                // while its brightness is right: a red-to-blue ramp drawn blue-to-red has almost
                // the same luminance profile and a completely different picture.
                total += Math.Abs(a.Red - b.Red) + Math.Abs(a.Green - b.Green) + Math.Abs(a.Blue - b.Blue);
                inkOurs += 765 - a.Red - a.Green - a.Blue;
                inkTheirs += 765 - b.Red - b.Green - b.Blue;
            }
        }

        long samples = (long)ours.Width * ours.Height * 3;
        return (total / (double)samples / 255.0, inkOurs, Math.Max(inkTheirs, 1));
    }
}
