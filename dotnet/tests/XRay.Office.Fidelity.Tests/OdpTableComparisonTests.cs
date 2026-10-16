using XRay.Office.Core.Documents;
using XRay.Office.Rendering.Pdf;
using XRay.Office.TestKit;
using XRay.Office.TestKit.LibreOffice;
using Shouldly;

namespace XRay.Office.Fidelity.Tests;

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
        Directory.CreateTempSubdirectory("xray-office-odp-tables").FullName;

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
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, LibreOfficeRunner.UnavailableReason);

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
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, LibreOfficeRunner.UnavailableReason);

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
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, LibreOfficeRunner.UnavailableReason);

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
    /// Where the last stated row ends, in points, on the second slide's table.
    /// </summary>
    /// <remarks>
    /// 72 pt of table origin plus two stated rows of one inch. Every coordinate above this is the
    /// file's own arithmetic and must agree between the two front ends to a rounding; every
    /// coordinate below it is inside the row that states <c>h="0"</c> and grows to its text, which
    /// is the one quantity the two formats no longer compute the same way. See
    /// <see cref="GrownRowDrop"/>.
    /// </remarks>
    private const double GrownRowTop = 216.0;

    /// <summary>
    /// How much shorter the ODF rendering of the grown row is, in points.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Two lines of it, at 21.600 pt of pitch through OOXML and 20.154 pt through ODF: 2.892 pt.
    /// <b>Measured on LibreOffice 26.2.4.2's own two renderings, not on ours</b> — it puts that
    /// table's bottom rule at 266.428 pt from <c>slide-table-grid.pptx</c> and at 263.537 pt from
    /// <c>odp-table-grid.odp</c>, a difference of 2.891, and its first cell baseline at 93.600
    /// against 91.928. Both of ours reproduce their own side against the reference, in
    /// <see cref="EveryGridLineIsTheStrokeLibreOfficeDraws"/> here and in
    /// <c>SlideTableComparisonTests</c>.
    /// </para>
    /// <para>
    /// <b>Why this is a restatement and not a relaxation.</b> This test's claim was that a table
    /// described in two vocabularies that share no element name comes out of the grid model the
    /// same way, and that claim is still worth making — it is what would catch a divergence in one
    /// reader only. What expired is the assumption that <em>every</em> number in it is
    /// format-independent. It is not: <c>a47776a938c</c> (2025-03-27, tdf#165521) moved PowerPoint
    /// table cells onto Microsoft's flat 1.2 × font height and left ODF's cells on the face's own
    /// metrics, so the height of a row the file does not state is now a property of the format.
    /// Asserting the divergence as a stated number keeps the guard: an error in either reader
    /// still fails here, and so would this difference drifting away from the reference's.
    /// </para>
    /// </remarks>
    private const double GrownRowDrop = 2.892;

    /// <summary>
    /// The same table through the two front ends produces the same display list, bar one row.
    /// </summary>
    /// <remarks>
    /// Needs no LibreOffice at run time and is the check that would catch a divergence appearing
    /// in one reader only — the ODF equivalent of the equality test between
    /// <c>ppt-features.ppt</c> and <c>slides-features.odp</c> on the extraction side. A failure
    /// here names which reader is wrong, where a failure against the reference only says that one
    /// of them is. The single stated exception is <see cref="GrownRowDrop"/>, which the reference
    /// makes too.
    /// </remarks>
    [Fact]
    public void TheSameTableThroughEitherFormatDrawsTheSameStrokes()
    {
        List<PdfStroke> odf = PdfStrokes.Read(Ours(Corpus.Require("odp-table-grid.odp")));
        List<PdfStroke> ooxml = PdfStrokes.Read(Ours(Corpus.Require("slide-table-grid.pptx")));

        odf.Count.ShouldBe(ooxml.Count, "number of grid lines");

        int inside = 0;

        for (int i = 0; i < ooxml.Count; i++)
        {
            string where = $"grid line {i + 1}";

            odf[i].PageIndex.ShouldBe(ooxml[i].PageIndex, $"{where}: slide");
            odf[i].FromX.ShouldBe(ooxml[i].FromX, CentimetreTolerance, $"{where}: start across");
            odf[i].ToX.ShouldBe(ooxml[i].ToX, CentimetreTolerance, $"{where}: end across");
            odf[i].Width.ShouldBe(ooxml[i].Width, 0.001, $"{where}: pen width");
            odf[i].Colour.ShouldBe(ooxml[i].Colour, $"{where}: pen colour");

            bool grown = ooxml[i].PageIndex == GrownRowSlide;

            odf[i].FromY.ShouldBe(
                Expected(ooxml[i].FromY, grown, ref inside), CentimetreTolerance,
                $"{where}: start down");
            odf[i].ToY.ShouldBe(
                Expected(ooxml[i].ToY, grown, ref inside), CentimetreTolerance,
                $"{where}: end down");
        }

        // The exception is not a blanket one: it reaches eight of this deck's forty-two stroke
        // coordinates, and a change that made it reach none — or the whole table, or the first
        // slide, whose three rows are all stated — would be a real one.
        inside.ShouldBe(8, "coordinates inside the row that grows to its text");
    }

    /// <summary>
    /// The slide holding the row with no stated height; the first slide states all three of its.
    /// </summary>
    private const int GrownRowSlide = 1;

    /// <summary>
    /// What the ODF rendering should put a coordinate at, given the OOXML one.
    /// </summary>
    /// <remarks>
    /// The same number everywhere the file states the geometry, and <see cref="GrownRowDrop"/>
    /// less inside the row that does not — counting the coordinates it applies to on the way, so
    /// that the exception cannot silently widen. The first slide is excluded outright rather than
    /// by its coordinates: it runs to 288 pt, past <see cref="GrownRowTop"/>, and every one of its
    /// rows is stated.
    /// </remarks>
    private static double Expected(double ooxmlY, bool grownRowSlide, ref int inside)
    {
        if (!grownRowSlide || ooxmlY <= GrownRowTop + CentimetreTolerance) return ooxmlY;

        inside++;
        return ooxmlY - GrownRowDrop;
    }

    /// <summary>The fills that are cells rather than the sheet, on a 720 by 540 pt slide.</summary>
    private static List<PdfFill> CellFills(List<PdfFill> fills)
        => [.. fills.Where(fill => fill.Width < 700 || fill.Height < 520)];

    private string Ours(string documentPath)
    {
        string destination = Path.Combine(
            _workDirectory, $"{Path.GetFileNameWithoutExtension(documentPath)}-xray-office.pdf");

        using IDocument document = OfficeDocument.Open(documentPath);
        IPageSequence pages = ((IPaginatedDocument)document).Layout();

        using FileStream output = File.Create(destination);
        new PdfRenderer(new PdfRenderOptions
        {
            CreationDate = new DateTimeOffset(2024, 1, 1, 0, 0, 0, TimeSpan.Zero),
        }).Render(pages, output);

        return destination;
    }
}
