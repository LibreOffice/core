using Paperless.Core.Documents;
using Paperless.Rendering.Pdf;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Compares the table an ODP draws against LibreOffice's own PDF, and against the PPTX it came
/// from.
/// </summary>
/// <remarks>
/// <para>
/// <c>odp-table-grid.odp</c> is <c>slide-table-grid.pptx</c> put through
/// <c>soffice --convert-to odp</c>, which makes it the strongest check available here: the two
/// files describe the same table in vocabularies that share not one element name, so a run of
/// strokes that agrees on both is a run of strokes the grid model produced rather than one either
/// reader happened to arrive at. It is also the only way to measure the border-width conversion,
/// which halves the stated width on the OOXML side and does not on the ODF side precisely because
/// the exporter already wrote the halved number out.
/// </para>
/// <para>
/// The stroke order is compared as written, for the reason
/// <see cref="SlideTableComparisonTests"/> states: LibreOffice consolidates the grid in a specific
/// order and sorting would hide a consolidation that reached the right lines by the wrong route.
/// </para>
/// </remarks>
public sealed class OdpTableComparisonTests : IDisposable
{
    /// <summary>A tenth of a point, as everywhere else in this project.</summary>
    private const double TolerancePoints = 0.1;

    /// <summary>
    /// What the round trip through centimetres costs.
    /// </summary>
    /// <remarks>
    /// LibreOffice writes an ODF length to three decimal places of a centimetre, so a table
    /// LibreOffice put at 19.05 cm comes back as <c>19.049cm</c> — 0.0283 pt narrower, the same
    /// unit of 1/100 mm that its PDF export already rounds every number to. Comparing the two
    /// formats against each other therefore needs a twentieth of a point where comparing either
    /// against the reference needs a tenth.
    /// </remarks>
    private const double CentimetreTolerance = 0.05;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-odp-tables").FullName;

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
    public void EveryGridLineIsTheStrokeLibreOfficeDraws()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require("odp-table-grid.odp");
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

            // The pen, which is where the two front ends could most easily disagree: an ODF border
            // states the width LibreOffice's OOXML importer had already halved, so the reader that
            // halves again draws every rule at 0.42 pt where the reference draws 0.85.
            mine.Width.ShouldBe(reference.Width, TolerancePoints, $"{where}: pen width");
            mine.Colour.ShouldBe(reference.Colour, $"{where}: pen colour");
        }
    }

    [Fact]
    public void EveryCellBackgroundIsWhereAndWhatLibreOfficeFillsIt()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require("odp-table-grid.odp");
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

    [Fact]
    public void EveryCellsTextIsDrawnWhereLibreOfficeDrawsIt()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        const string fileName = "odp-table-grid.odp";
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
            mine.FontSize.ShouldBe(reference.FontSize, 0.01, $"{where}: font size");

            // One short where a line wrapped, for the reason the OOXML table comparison records.
            mine.GlyphCount.ShouldBeInRange(
                reference.GlyphCount - 1, reference.GlyphCount, $"{where}: glyph count");
        }
    }

    /// <summary>
    /// The same table through the two front ends produces the same display list.
    /// </summary>
    /// <remarks>
    /// Needs no LibreOffice at run time and is the check that would catch a divergence appearing
    /// in one reader only — the ODF equivalent of the equality test between
    /// <c>ppt-features.ppt</c> and <c>slides-features.odp</c> on the extraction side. A failure
    /// here names which reader is wrong, where a failure against the reference only says that one
    /// of them is.
    /// </remarks>
    [Fact]
    public void TheSameTableThroughEitherFormatDrawsTheSameStrokes()
    {
        List<PdfStroke> odf = PdfStrokes.Read(Ours(Corpus.Require("odp-table-grid.odp")));
        List<PdfStroke> ooxml = PdfStrokes.Read(Ours(Corpus.Require("slide-table-grid.pptx")));

        odf.Count.ShouldBe(ooxml.Count, "number of grid lines");

        for (int i = 0; i < ooxml.Count; i++)
        {
            string where = $"grid line {i + 1}";

            odf[i].PageIndex.ShouldBe(ooxml[i].PageIndex, $"{where}: slide");
            odf[i].FromX.ShouldBe(ooxml[i].FromX, CentimetreTolerance, $"{where}: start across");
            odf[i].FromY.ShouldBe(ooxml[i].FromY, CentimetreTolerance, $"{where}: start down");
            odf[i].ToX.ShouldBe(ooxml[i].ToX, CentimetreTolerance, $"{where}: end across");
            odf[i].ToY.ShouldBe(ooxml[i].ToY, CentimetreTolerance, $"{where}: end down");
            odf[i].Width.ShouldBe(ooxml[i].Width, 0.001, $"{where}: pen width");
            odf[i].Colour.ShouldBe(ooxml[i].Colour, $"{where}: pen colour");
        }
    }

    /// <summary>The fills that are cells rather than the sheet, on a 720 by 540 pt slide.</summary>
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
