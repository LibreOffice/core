using Paperless.Core.Documents;
using Paperless.Rendering.Pdf;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Compares the preset shapes, dashes and markers Paperless draws against LibreOffice's.
/// </summary>
/// <remarks>
/// <para>
/// The corpus deck is <c>slide-shape-features.pptx</c>, hand-written because the point of it is
/// to state features LibreOffice's own export resolves away: a preset by name rather than as a
/// custom path, a <c>a:prstDash</c> rather than a dash array, and an <c>a:buAutoNum</c> rather
/// than the number it produced. Every shape is at a whole inch and every size is one and a half
/// by one, so a disagreement is a bug rather than a rounding.
/// </para>
/// <para>
/// Slide 1 is six presets whose edges are all straight, so their vertices compare one for one.
/// Slide 2 is five built out of arcs plus a five-pointed star, which compare by the points their
/// curves pass through — neither writer chooses the same number of cubics per arc, so the control
/// points are not comparable and are not compared.
/// </para>
/// </remarks>
public sealed class SlideShapeGeometryComparisonTests : IDisposable
{
    private const string Deck = "slide-shape-features.pptx";

    /// <summary>A tenth of a point, as everywhere else in this project.</summary>
    private const double TolerancePoints = 0.1;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-presets").FullName;

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
    public void EveryPresetIsTheShapeLibreOfficeDraws()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(Deck);
        List<PdfPath> ours = Shapes(PdfPaths.Read(Ours(path)));
        List<PdfPath> theirs = Shapes(PdfPaths.Read(_libreOffice.ConvertToPdf(path, _workDirectory)));

        Assert.SkipWhen(theirs.Count == 0, "pdftotext is not available; install poppler-utils");

        // Twelve shapes over the first two slides, and the count is half the test: an evaluator
        // that drew a bounding rectangle for an unknown preset would still produce twelve.
        ours.Count.ShouldBe(theirs.Count, "number of filled shapes");

        for (int i = 0; i < theirs.Count; i++)
        {
            PdfPath mine = ours[i];
            PdfPath reference = theirs[i];
            string where = $"shape {i + 1} on slide {reference.PageIndex + 1}";

            mine.PageIndex.ShouldBe(reference.PageIndex, $"{where}: slide");
            mine.Colour.ShouldBe(reference.Colour, $"{where}: fill colour");

            (double left, double top, double right, double bottom) = mine.Bounds;
            (double theirLeft, double theirTop, double theirRight, double theirBottom) =
                reference.Bounds;

            left.ShouldBe(theirLeft, TolerancePoints, $"{where}: left");
            top.ShouldBe(theirTop, TolerancePoints, $"{where}: top");
            right.ShouldBe(theirRight, TolerancePoints, $"{where}: right");
            bottom.ShouldBe(theirBottom, TolerancePoints, $"{where}: bottom");

            if (reference.Curves > 0 || mine.Curves > 0) continue;

            // A polygon: every vertex, in order. This is where a wrong guide formula shows —
            // a pentagon whose two lower corners are at the wrong height has the same bounding
            // box as a correct one.
            mine.Points.Count.ShouldBe(reference.Points.Count, $"{where}: vertex count");

            for (int p = 0; p < reference.Points.Count; p++)
            {
                mine.Points[p].X.ShouldBe(
                    reference.Points[p].X, TolerancePoints, $"{where}: vertex {p + 1} across");
                mine.Points[p].Y.ShouldBe(
                    reference.Points[p].Y, TolerancePoints, $"{where}: vertex {p + 1} down");
            }
        }
    }

    [Fact]
    public void EveryDashedLineCarriesLibreOfficesOwnDashArray()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(Deck);
        List<PdfStroke> ours = PdfStrokes.Read(Ours(path));
        List<PdfStroke> theirs = PdfStrokes.Read(_libreOffice.ConvertToPdf(path, _workDirectory));

        Assert.SkipWhen(theirs.Count == 0, "pdftotext is not available; install poppler-utils");

        // The three straight connectors on slide 3. The rectangle below them is dashed too and
        // is not a two-point line, so neither reader sees it.
        ours.Count.ShouldBe(theirs.Count, "number of straight strokes");

        for (int i = 0; i < theirs.Count; i++)
        {
            PdfStroke mine = ours[i];
            PdfStroke reference = theirs[i];
            string where = $"stroke {i + 1} on slide {reference.PageIndex + 1}";

            // The shaft's ends, which is where an arrowhead shows: the first line is shortened at
            // both ends to make room for its markers and the other two are not.
            mine.FromX.ShouldBe(reference.FromX, TolerancePoints, $"{where}: start across");
            mine.ToX.ShouldBe(reference.ToX, TolerancePoints, $"{where}: end across");
            mine.FromY.ShouldBe(reference.FromY, TolerancePoints, $"{where}: start down");
            mine.Width.ShouldBe(reference.Width, TolerancePoints, $"{where}: pen width");
            mine.Colour.ShouldBe(reference.Colour, $"{where}: pen colour");

            mine.Dashes.Count.ShouldBe(reference.Dashes.Count, $"{where}: dash array length");

            for (int d = 0; d < reference.Dashes.Count; d++)
            {
                mine.Dashes[d].ShouldBe(
                    reference.Dashes[d], TolerancePoints, $"{where}: dash {d + 1}");
            }
        }
    }

    [Fact]
    public void EveryAutomaticNumberIsDrawnWhereLibreOfficeDrawsIt()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(Deck);
        List<PdfTextRun> ours = PdfTextRuns.Read(Ours(path));
        List<PdfTextRun> theirs = PdfTextRuns.Read(
            _libreOffice.ConvertToPdf(path, _workDirectory));

        Assert.SkipWhen(theirs.Count == 0, "pdftotext is not available; install poppler-utils");

        // Twelve runs: six list items, each a label and a body. A deck that produced no numbers
        // at all would have six.
        ours.Count.ShouldBe(theirs.Count, "number of drawn text runs");

        for (int i = 0; i < theirs.Count; i++)
        {
            PdfTextRun mine = ours[i];
            PdfTextRun reference = theirs[i];
            string where = $"run {i + 1} on slide {reference.PageIndex + 1}";

            mine.X.ShouldBe(reference.X, TolerancePoints, $"{where}: pen across");
            mine.Y.ShouldBe(reference.Y, TolerancePoints, $"{where}: baseline");
            mine.GlyphCount.ShouldBe(reference.GlyphCount, $"{where}: glyph count");
        }
    }

    /// <summary>
    /// The filled paths that are shapes rather than the sheet, on a 720 by 540 pt slide.
    /// </summary>
    private static List<PdfPath> Shapes(List<PdfPath> paths)
        => [.. paths.Where(path =>
        {
            (double left, double top, double right, double bottom) = path.Bounds;
            return right - left < 700 || bottom - top < 520;
        })];

    private string Ours(string documentPath)
    {
        string destination = Path.Combine(
            _workDirectory, $"{Path.GetFileNameWithoutExtension(documentPath)}-paperless.pdf");

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
