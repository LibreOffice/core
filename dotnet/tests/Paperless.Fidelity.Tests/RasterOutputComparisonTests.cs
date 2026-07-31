using System.Diagnostics;
using System.Globalization;
using Paperless.Core.Documents;
using Paperless.Rendering.Raster;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Shouldly;
using SkiaSharp;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Compares a page Paperless rasterises against the same page LibreOffice rasterises.
/// </summary>
/// <remarks>
/// <para>
/// The weaker of the two comparisons in this project and deliberately kept so. A pixel diff
/// answers "does this look the same" with a percentage and is at its worst at saying which
/// half of the pipeline is wrong — the operator-for-operator comparison in
/// <see cref="PdfOutputComparisonTests"/> is what holds this backend's geometry to a tenth
/// of a point. What an image adds is the one thing the content stream cannot show: that the
/// marks actually land on the paper, in the right quantity, once a rasteriser has had them.
/// </para>
/// <para>
/// <b>The reference goes through PDF, not through LibreOffice's PNG export.</b> The two are
/// not equivalent: <c>--convert-to 'png:writer_png_Export'</c> produces LibreOffice's own
/// idea of a page image at its own scale — measured, an A4 page comes out 795 x 1124 — while
/// <c>--convert-to pdf</c> rasterised by <c>pdftoppm -r 150</c> gives exactly 150 dpi. A
/// comparison needs a known scale, so it takes the PDF route.
/// </para>
/// <para>
/// The bounds below are set from measurement rather than from taste. On
/// <c>prose-odt.odt</c> at 150 dpi the mean absolute error against poppler's rasterisation of
/// LibreOffice's own PDF is 0.0019 and the ink ratio 0.985; at 300 dpi the error halves. What
/// remains is two rasterisers disagreeing about antialiasing — Skia's coverage against
/// FreeType's — plus the 0.1 pt horizontal pen offset LibreOffice's PDF export adds, which at
/// 150 dpi is a fifth of a pixel on every glyph.
/// </para>
/// </remarks>
public sealed class RasterOutputComparisonTests : IDisposable
{
    private const double Dpi = 150;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-raster").FullName;

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
    [InlineData("prose-odt.odt")]
    [InlineData("prose-docx.docx")]
    [InlineData("table-borders.fodt")]
    [InlineData("table-shading.odt")]
    public void EveryPageRasterisesToTheSameImageAsLibreOffices(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        List<string> reference = Reference(path);

        Assert.SkipWhen(reference.Count == 0, "pdftoppm is not available; install poppler-utils");

        using IDocument document = PaperlessDocument.Open(path);
        IPageSequence pages = ((IPaginatedDocument)document).Layout();
        RasterRenderer renderer = new(new RasterRenderOptions { Dpi = Dpi });

        pages.Count.ShouldBe(reference.Count, $"{fileName}: page count");

        for (int i = 0; i < pages.Count; i++)
        {
            using SKBitmap ours = renderer.Rasterise(pages[i]);
            using SKBitmap theirs = SKBitmap.Decode(reference[i]);
            string where = $"{fileName}: page {i + 1}";

            // Size first, and nothing else is worth reading if it fails: a page-size mismatch means
            // the geometry was read wrongly, which is a more fundamental bug than anything a
            // difference score could describe, and every other measure becomes meaningless.
            ours.Width.ShouldBe(theirs.Width, $"{where}: width in pixels");
            ours.Height.ShouldBe(theirs.Height, $"{where}: height in pixels");

            (double error, double inkOurs, double inkTheirs) = Compare(ours, theirs);

            // How much ink, which is the measure that catches something skipped outright — an
            // unimplemented shape, a fill that never happened.
            //
            // Weighed rather than counted, and that distinction is the whole usefulness of the
            // measure. Counting pixels darker than a threshold makes a border-heavy page look
            // wrong for a reason that is not: a 0.5 pt grid line is 1.04 px at 150 dpi, Skia
            // spreads it across two rows and poppler snaps it to one, so the pixel count of
            // table-borders.fodt comes out 30% high while the two pages are indistinguishable.
            // Summing darkness instead makes the same page 1.005, because the same quantity of
            // ink was laid down either way.
            (inkOurs / inkTheirs).ShouldBeInRange(
                0.95, 1.05,
                $"{where}: {inkOurs:F0} units of ink drawn, {inkTheirs:F0} rendered");

            error.ShouldBeLessThan(
                0.01, $"{where}: mean absolute difference {error:F4} over the whole page");
        }
    }

    // ------------------------------------------------------------------------- the machinery

    /// <summary>LibreOffice's PDF, rasterised at a known resolution; the paths of the page images.</summary>
    private List<string> Reference(string documentPath)
    {
        string pdf = _libreOffice.ConvertToPdf(documentPath, _workDirectory);
        string prefix = Path.Combine(_workDirectory, Path.GetFileNameWithoutExtension(pdf) + "-page");

        ProcessStartInfo start = new("pdftoppm") { RedirectStandardError = true };
        start.ArgumentList.Add("-r");
        start.ArgumentList.Add(Dpi.ToString(CultureInfo.InvariantCulture));
        start.ArgumentList.Add("-png");
        start.ArgumentList.Add(pdf);
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

        return [.. Directory.GetFiles(_workDirectory, Path.GetFileName(prefix) + "-*.png").Order(StringComparer.Ordinal)];
    }

    /// <summary>
    /// The mean absolute difference between two images, and how much ink each carries.
    /// </summary>
    /// <remarks>
    /// Both on the same pass, because the two answer different questions and the pass is the
    /// expensive part: the mean says how close the pages are overall and is diluted by the
    /// whitespace that is most of a page, while the ink is what moves when something was not
    /// drawn at all.
    /// </remarks>
    private static (double Error, double InkOurs, double InkTheirs) Compare(SKBitmap ours, SKBitmap theirs)
    {
        long total = 0;
        long inkOurs = 0;
        long inkTheirs = 0;

        for (int y = 0; y < ours.Height; y++)
        {
            for (int x = 0; x < ours.Width; x++)
            {
                SKColor a = ours.GetPixel(x, y);
                SKColor b = theirs.GetPixel(x, y);

                int mine = (a.Red + a.Green + a.Blue) / 3;
                int reference = (b.Red + b.Green + b.Blue) / 3;

                total += Math.Abs(mine - reference);
                inkOurs += 255 - mine;
                inkTheirs += 255 - reference;
            }
        }

        long pixels = (long)ours.Width * ours.Height;
        return (total / (double)pixels / 255.0, inkOurs, Math.Max(inkTheirs, 1));
    }
}
