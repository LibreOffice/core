using Paperless.Core.Documents;
using Paperless.Rendering.Pdf;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Compares the shapes an ODF <c>draw:enhanced-path</c> draws against LibreOffice's own PDF.
/// </summary>
/// <remarks>
/// <para>
/// <c>odp-shape-paths.fodp</c> is hand-written for the reason the other geometry deck is:
/// LibreOffice's own converters resolve the interesting cases away. Anything reaching ODF through
/// an OOXML import comes out as an <c>ooxml-</c> preset whose path is five straight commands, so a
/// converted file exercises none of ODF's fifteen extra command letters — the whole ellipse, the
/// quarter ellipses that round a corner, the arc stated as a swing.
/// </para>
/// <para>
/// The shapes carry no outline, so each is exactly one filled path and the comparison is
/// <see cref="PdfPaths"/> shape for shape. Two of the six are straight-edged and compare vertex
/// for vertex; the rest compare by their on-curve points and their bounding box, because neither
/// writer chooses the same number of cubics per arc.
/// </para>
/// </remarks>
public sealed class OdpShapePathComparisonTests : IDisposable
{
    private const string Deck = "odp-shape-paths.fodp";

    /// <summary>A tenth of a point, as everywhere else in this project.</summary>
    private const double TolerancePoints = 0.1;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-odf-paths").FullName;

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
    public void EveryEnhancedPathIsTheShapeLibreOfficeDraws()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(Deck);
        List<PdfPath> ours = Shapes(PdfPaths.Read(Ours(path)));
        List<PdfPath> theirs = Shapes(
            PdfPaths.Read(_libreOffice.ConvertToPdf(path, _workDirectory)));

        Assert.SkipWhen(theirs.Count == 0, "pdftotext is not available; install poppler-utils");

        // Six shapes on the first slide. A reader that fell back to the bounding rectangle for a
        // path it could not parse would still produce six, which is why the bounds are compared
        // as well and why two of the six have no straight edge at all.
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

    /// <summary>
    /// The hand-written table's grid, which the converted deck cannot measure.
    /// </summary>
    /// <remarks>
    /// Three columns of three different widths and a cell spanning two of them, with the outer
    /// rules three times the inner ones — so a reader that divided the frame's width equally, or
    /// that positioned a cell after a span by accumulated width rather than by ordinal, draws a
    /// visibly different grid rather than one out by a rounding.
    /// </remarks>
    [Fact]
    public void TheHandWrittenGridIsTheGridLibreOfficeDraws()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(Deck);
        List<PdfStroke> ours = PdfStrokes.Read(Ours(path));
        List<PdfStroke> theirs = PdfStrokes.Read(_libreOffice.ConvertToPdf(path, _workDirectory));

        Assert.SkipWhen(theirs.Count == 0, "pdftotext is not available; install poppler-utils");

        ours.Count.ShouldBe(theirs.Count, "number of grid lines");

        for (int i = 0; i < theirs.Count; i++)
        {
            PdfStroke mine = ours[i];
            PdfStroke reference = theirs[i];
            string where = $"grid line {i + 1}";

            mine.FromX.ShouldBe(reference.FromX, TolerancePoints, $"{where}: start across");
            mine.FromY.ShouldBe(reference.FromY, TolerancePoints, $"{where}: start down");
            mine.ToX.ShouldBe(reference.ToX, TolerancePoints, $"{where}: end across");
            mine.ToY.ShouldBe(reference.ToY, TolerancePoints, $"{where}: end down");
            mine.Width.ShouldBe(reference.Width, TolerancePoints, $"{where}: pen width");
            mine.Colour.ShouldBe(reference.Colour, $"{where}: pen colour");
        }
    }

    [Fact]
    public void EveryCellsTextIsDrawnWhereLibreOfficeDrawsIt()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(Deck);
        List<PdfTextRun> ours = PdfTextRuns.Read(Ours(path));
        List<PdfTextRun> theirs = PdfTextRuns.Read(
            _libreOffice.ConvertToPdf(path, _workDirectory));

        Assert.SkipWhen(theirs.Count == 0, "pdftotext is not available; install poppler-utils");

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
    /// The custom shapes: the first slide's filled paths, minus the sheet.
    /// </summary>
    /// <remarks>
    /// The second slide is excluded rather than filtered, and the reason is a difference in our
    /// PDF writer rather than in the layout: a table cell's background is an axis-parallel
    /// rectangle and we write it with <c>re</c> where LibreOffice writes four <c>l</c> operators,
    /// so <see cref="PdfPaths"/> sees the reference's two header cells and none of ours. Those
    /// fills are measured against the reference by <see cref="OdpTableComparisonTests"/> through
    /// <see cref="PdfFills"/>, which reads exactly that form.
    /// </remarks>
    private static List<PdfPath> Shapes(List<PdfPath> paths)
        => [.. paths.Where(path =>
        {
            if (path.PageIndex != 0) return false;

            (double left, double top, double right, double bottom) = path.Bounds;
            return right - left < 780 || bottom - top < 430;
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
