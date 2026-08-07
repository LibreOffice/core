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
/// What a blip's <c>a:lum</c> actually does to a picture, measured against LibreOffice.
/// </summary>
/// <remarks>
/// <para>
/// The pair a file states is not always the pair that is applied, so this cannot be checked by
/// reading the markup — it has to be checked against pixels. The three cases and their
/// citations are on <see cref="Paperless.Core.Graphics.LuminanceRecolour"/>; the shortest
/// statement of the surprise is that <c>bright="70000" contrast="-70000"</c> — PowerPoint's
/// "Washout", and every washed-out background picture in the corpus — is mapped by the
/// reference to its own watermark mode and applied as <em>+50 and −70</em>.
/// </para>
/// <para>
/// <c>slide-picture-washout.pptx</c> draws one generated PNG four times: untouched, at the
/// washout pair, at a brightness with no contrast, and at a brightness and contrast that are
/// neither zero nor the washout. Those are the control and all three arithmetics. The picture
/// is a full 0–255 ramp in grey and then in two colour sweeps, so every input level of every
/// channel is present and a transfer curve that is right in the mid-tones and wrong at the ends
/// cannot pass.
/// </para>
/// <para>
/// <strong>The bound is the control band's own error, not a constant.</strong> Both renderers
/// scale a 256-pixel-wide picture to 560 and interpolate differently, which costs about two
/// levels of 255 on a steep ramp whatever the recolouring does — so a fixed threshold would
/// either be loose enough to pass a broken curve or tight enough to fail on resampling. Each
/// recoloured band is required to be no worse than the untouched one plus one level. Measured
/// against LibreOffice 24.2.7.2 at 72 dpi, mean absolute error per channel:
/// </para>
/// <list type="table">
/// <item><description>control 2.20 before and after</description></item>
/// <item><description>washout <strong>120.02 → 0.24</strong></description></item>
/// <item><description>brightness alone <strong>85.87 → 1.52</strong></description></item>
/// <item><description>both stated <strong>47.94 → 2.28</strong></description></item>
/// </list>
/// <para>
/// Both sides are rasterised by the same <c>pdftoppm</c> rather than one by Skia and one by
/// poppler, because the quantity under test is a colour and a second rasteriser's idea of a
/// colour would be measured along with it.
/// </para>
/// </remarks>
public sealed class SlidePictureRecolourComparisonTests : IDisposable
{
    private const string Deck = "slide-picture-washout.pptx";

    /// <summary>72 dpi, so one pixel is one point and the band edges below are the file's.</summary>
    private const int Dpi = 72;

    /// <summary>How much worse than the untouched picture a recoloured one may be.</summary>
    private const double Margin = 1.0;

    /// <summary>
    /// The four pictures, as the rows of the page they occupy.
    /// </summary>
    /// <remarks>
    /// Each is 1371600 EMU tall at 228600, 1943100, 3657600 and 5372100 on a 540 pt page, which
    /// at 72 dpi is 108 rows at 18, 153, 288 and 423. Six rows are dropped at each edge so the
    /// comparison never straddles the boundary between the picture and the paper, where the two
    /// rasterisers disagree about a half-covered pixel for reasons that are not this test's.
    /// </remarks>
    private static readonly (string Name, int Top, int Bottom)[] Bands =
    [
        ("untouched", 18, 126),
        ("washout, bright 70 contrast -70", 153, 261),
        ("brightness alone, bright 40", 288, 396),
        ("both stated, bright 20 contrast 20", 423, 531),
    ];

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-recolour").FullName;

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
    public void EveryPictureRecolourIsTheOneLibreOfficeApplies()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(Deck);

        string? ours = Rasterised(Ours(path), "ours");
        string? theirs = Rasterised(_libreOffice.ConvertToPdf(path, _workDirectory), "ref");

        Assert.SkipWhen(
            ours is null || theirs is null, "pdftoppm is not available; install poppler-utils");

        using SKBitmap mine = SKBitmap.Decode(ours!);
        using SKBitmap reference = SKBitmap.Decode(theirs!);

        mine.Width.ShouldBe(reference.Width, "page width in pixels");
        mine.Height.ShouldBe(reference.Height, "page height in pixels");

        double control = 0;

        for (int band = 0; band < Bands.Length; band++)
        {
            (string name, int top, int bottom) = Bands[band];
            double error = MeanError(mine, reference, top + 6, bottom - 6, 80, 640);

            if (band == 0)
            {
                // Nothing is asked of the untouched picture beyond its being drawn: it is the
                // yardstick, and a wrong one would show as every other band failing at once.
                control = error;
                control.ShouldBeLessThan(8.0, "untouched: the picture is drawn at all");
                continue;
            }

            error.ShouldBeLessThanOrEqualTo(
                control + Margin,
                $"{name}: mean absolute error {error:F2} of 255 against the untouched "
                + $"picture's {control:F2}");
        }
    }

    /// <summary>The mean absolute per-channel difference over a rectangle of two images.</summary>
    private static double MeanError(
        SKBitmap ours, SKBitmap theirs, int top, int bottom, int left, int right)
    {
        long total = 0;
        long count = 0;

        for (int y = top; y < bottom; y++)
        {
            for (int x = left; x < right; x++)
            {
                SKColor a = ours.GetPixel(x, y);
                SKColor b = theirs.GetPixel(x, y);

                total += Math.Abs(a.Red - b.Red)
                       + Math.Abs(a.Green - b.Green)
                       + Math.Abs(a.Blue - b.Blue);
                count += 3;
            }
        }

        return count == 0 ? 0 : (double)total / count;
    }

    /// <summary>A PDF's first page as a PNG, or null when <c>pdftoppm</c> is not installed.</summary>
    private string? Rasterised(string pdf, string prefix)
    {
        string root = Path.Combine(_workDirectory, prefix);

        ProcessStartInfo start = new("pdftoppm") { RedirectStandardError = true };
        start.ArgumentList.Add("-r");
        start.ArgumentList.Add(Dpi.ToString(CultureInfo.InvariantCulture));
        start.ArgumentList.Add("-png");
        start.ArgumentList.Add(pdf);
        start.ArgumentList.Add(root);

        try
        {
            using Process? process = Process.Start(start);
            if (process is null) return null;

            process.WaitForExit((int)LibreOfficeRunner.Timeout.TotalMilliseconds);
            if (process.ExitCode != 0) return null;
        }
        catch (System.ComponentModel.Win32Exception)
        {
            return null;
        }

        string[] pages = Directory.GetFiles(_workDirectory, prefix + "-*.png");
        return pages.Length == 0 ? null : pages.Order(StringComparer.Ordinal).First();
    }

    private string Ours(string documentPath)
    {
        string destination = Path.Combine(_workDirectory, "recolour-paperless.pdf");

        using IDocument document = PaperlessDocument.Open(documentPath);
        IPageSequence pages = ((IPaginatedDocument)document).Layout();

        using FileStream output = File.Create(destination);
        new PdfRenderer(new PdfRenderOptions
        {
            CreationDate = new DateTimeOffset(2024, 1, 1, 0, 0, 0, TimeSpan.Zero),
        }).Render(pages, output);

        return destination;
    }
}
