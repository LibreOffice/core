using Paperless.Core.Documents;
using Paperless.Rendering.Pdf;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Compares the table a slide draws against the table LibreOffice draws, in its own PDF.
/// </summary>
/// <remarks>
/// <para>
/// A table is the one thing on a slide that exercises all three readers at once, which is why it
/// has a comparison of its own: its grid is <see cref="PdfStrokes"/>, its cell backgrounds are
/// <see cref="PdfFills"/>, and its text is <see cref="PdfTextRuns"/>. A table drawn in the right
/// place with the wrong pen, or with the right pens in the wrong order, passes two of the three.
/// </para>
/// <para>
/// <strong>The stroke comparison is ordered, deliberately.</strong> LibreOffice consolidates a
/// table's borders into one stroke per grid line and emits them in a specific order — the cells
/// walked row-major, each contributing its top, its bottom if it is on the last row, its left,
/// and its right if it is on the last column. Sorting both sides before comparing would hide a
/// consolidation that produced the right lines by the wrong route, which is exactly the mistake
/// worth catching.
/// </para>
/// </remarks>
public sealed class SlideTableComparisonTests : IDisposable
{
    /// <summary>A tenth of a point, as everywhere else in this project.</summary>
    private const double TolerancePoints = 0.1;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-tables").FullName;

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
    [InlineData("slide-table-grid.pptx")]
    [InlineData("deck-features.pptx")]
    public void ADeckHasAPagePerSlideItShows(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        List<PdfPageSize> ours = PdfPageSizes.Read(Ours(path));
        List<PdfPageSize> theirs = PdfPageSizes.Read(_libreOffice.ConvertToPdf(path, _workDirectory));

        Assert.SkipWhen(theirs.Count == 0, "pdftotext is not available; install poppler-utils");

        // The cheapest assertion in the suite and the one no per-shape comparison makes:
        // deck-features.pptx has four slides and three pages, because its fourth is hidden and
        // LibreOffice's PDF export leaves a hidden slide out. Paperless laid all four out until
        // this was measured.
        ours.Count.ShouldBe(theirs.Count, $"{fileName}: page count");
    }

    [Fact]
    public void EveryGridLineIsTheStrokeLibreOfficeDraws()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require("slide-table-grid.pptx");
        List<PdfStroke> ours = PdfStrokes.Read(Ours(path));
        List<PdfStroke> theirs = PdfStrokes.Read(_libreOffice.ConvertToPdf(path, _workDirectory));

        Assert.SkipWhen(theirs.Count == 0, "pdftotext is not available; install poppler-utils");

        ours.Count.ShouldBe(theirs.Count, "number of grid lines");

        for (int i = 0; i < theirs.Count; i++)
        {
            PdfStroke mine = ours[i];
            PdfStroke reference = theirs[i];
            string where = $"grid line {i + 1} on slide {reference.PageIndex + 1}";

            mine.PageIndex.ShouldBe(reference.PageIndex, $"{where}: slide");
            mine.FromX.ShouldBe(reference.FromX, TolerancePoints, $"{where}: start across");
            mine.FromY.ShouldBe(reference.FromY, TolerancePoints, $"{where}: start down");
            mine.ToX.ShouldBe(reference.ToX, TolerancePoints, $"{where}: end across");
            mine.ToY.ShouldBe(reference.ToY, TolerancePoints, $"{where}: end down");

            // The pen, which is the half of a border that no placement check sees — and the one
            // that goes through the EMU to 1/100 mm to halved-BorderLine2 to twips conversion
            // that makes a stated one point come out as 0.85.
            mine.Width.ShouldBe(reference.Width, TolerancePoints, $"{where}: pen width");
            mine.Colour.ShouldBe(reference.Colour, $"{where}: pen colour");
        }
    }

    [Fact]
    public void EveryCellBackgroundIsWhereAndWhatLibreOfficeFillsIt()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require("slide-table-grid.pptx");
        List<PdfFill> ours = CellFills(PdfFills.Read(Ours(path)));
        List<PdfFill> theirs = CellFills(
            PdfFills.Read(_libreOffice.ConvertToPdf(path, _workDirectory)));

        Assert.SkipWhen(theirs.Count == 0, "pdftotext is not available; install poppler-utils");

        ours.Count.ShouldBe(theirs.Count, "number of filled cells");

        for (int i = 0; i < theirs.Count; i++)
        {
            PdfFill mine = ours[i];
            PdfFill reference = theirs[i];
            string where = $"cell fill {i + 1} on slide {reference.PageIndex + 1}";

            mine.PageIndex.ShouldBe(reference.PageIndex, $"{where}: slide");
            mine.Left.ShouldBe(reference.Left, TolerancePoints, $"{where}: left");
            mine.Top.ShouldBe(reference.Top, TolerancePoints, $"{where}: top");
            mine.Width.ShouldBe(reference.Width, TolerancePoints, $"{where}: width");
            mine.Height.ShouldBe(reference.Height, TolerancePoints, $"{where}: height");
            mine.Colour.ShouldBe(reference.Colour, $"{where}: colour");
        }
    }

    /// <summary>
    /// Every cell's text, against the reference's.
    /// </summary>
    /// <remarks>
    /// <c>slide-table-grid.pptx</c> only, and not <c>deck-features.pptx</c>, whose table slide
    /// this now draws correctly while its <em>outline</em> slide does not: its bullet markers sit
    /// 8.19 pt below where LibreOffice draws them, which is a recorded open item about label
    /// placement and would fail this comparison for a reason that has nothing to do with tables.
    /// </remarks>
    [Fact]
    public void EveryCellsTextIsDrawnWhereLibreOfficeDrawsIt()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        const string fileName = "slide-table-grid.pptx";
        string path = Corpus.Require(fileName);
        List<PdfTextRun> ours = PdfTextRuns.Read(Ours(path));
        List<PdfTextRun> theirs = PdfTextRuns.Read(
            _libreOffice.ConvertToPdf(path, _workDirectory));

        Assert.SkipWhen(theirs.Count == 0, "pdftotext is not available; install poppler-utils");

        ours.Count.ShouldBe(theirs.Count, $"{fileName}: number of drawn text runs");

        for (int i = 0; i < theirs.Count; i++)
        {
            PdfTextRun mine = ours[i];
            PdfTextRun reference = theirs[i];
            string where = $"{fileName}: run {i + 1} on slide {reference.PageIndex + 1}";

            mine.PageIndex.ShouldBe(reference.PageIndex, $"{where}: slide");
            mine.X.ShouldBe(reference.X, TolerancePoints, $"{where}: pen across");
            mine.Y.ShouldBe(reference.Y, TolerancePoints, $"{where}: baseline");
            // A hundredth of a point rather than a thousandth: a 40 pt title comes out of the
            // export at 39.997, which is the same 1/100 mm rounding every other number here
            // carries and not a size difference.
            mine.FontSize.ShouldBe(reference.FontSize, 0.01, $"{where}: font size");

            // Equal, or one short where a line wrapped. LibreOffice draws the space a line broke
            // at as part of that line's run; the shared layouter stops at the last visible
            // character, so a wrapped line is one glyph shorter here. It is a space at the end of
            // a line, so nothing is visibly missing — but it is a real difference and it is
            // recorded in the TODO rather than hidden by comparing sets.
            mine.GlyphCount.ShouldBeInRange(
                reference.GlyphCount - 1, reference.GlyphCount, $"{where}: glyph count");
        }
    }

    /// <summary>
    /// The fills that are cells rather than the sheet, on a 720 by 540 pt slide.
    /// </summary>
    /// <remarks>
    /// The same exclusion the shape comparison makes and for the same reason: the page rectangle
    /// is the one fill whose size is the export's own rounding rather than the document's.
    /// </remarks>
    private static List<PdfFill> CellFills(List<PdfFill> fills)
        => [.. fills.Where(fill => fill.Width < 700 || fill.Height < 520)];

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
