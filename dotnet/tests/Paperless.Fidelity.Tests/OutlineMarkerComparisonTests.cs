using Paperless.Core.Documents;
using Paperless.Rendering.Pdf;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Compares a bulleted outline — markers and text — against LibreOffice's own PDF, in both
/// families.
/// </summary>
/// <remarks>
/// <para>
/// One rule and two formats, which is the point of testing them together. A bullet is not drawn on
/// its paragraph's baseline: <c>Outliner::ImpCalcBulletArea</c> centres its box against the line's
/// <em>text</em> and <c>Outliner::StripBullet</c> then draws it from that box's bottom less the
/// bullet font's descent. The offset from the text baseline is therefore
/// <c>lineHeight − textHeight/2 + (markerAscent − markerDescent)/2 − lineAscent</c>, which the two
/// decks reach through completely different formatting and which was 8.19 pt out on one and 6.72
/// on the other.
/// </para>
/// <para>
/// The two decks also disagree about the line height they are measured against —
/// <c>deck-features.pptx</c> gets the font-independent rule from its importer and
/// <c>slides-features.odp</c> gets the face's own metrics — so a rule that happened to fit one
/// would visibly miss the other. That is why both are here rather than only the one that
/// regressed.
/// </para>
/// </remarks>
public sealed class OutlineMarkerComparisonTests : IDisposable
{
    /// <summary>
    /// An eighth of a point.
    /// </summary>
    /// <remarks>
    /// Looser than the tenth used elsewhere by exactly the recorded per-line residual: an 18 pt
    /// Liberation Sans line is one unit of 1/100 mm shorter here than in the reference, and the
    /// ODF deck's third outline paragraph is the fourth line down, so it carries 0.109 pt of
    /// accumulated difference before anything about the marker is measured.
    /// </remarks>
    private const double TolerancePoints = 0.125;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-markers").FullName;

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
    [InlineData("deck-features.pptx")]
    [InlineData("slides-features.odp")]
    public void EveryOutlineMarkerAndItsTextAreWhereLibreOfficeDrawsThem(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        List<PdfTextRun> ours = Outline(PdfTextRuns.Read(Ours(path)));
        List<PdfTextRun> theirs = Outline(
            PdfTextRuns.Read(_libreOffice.ConvertToPdf(path, _workDirectory)));

        Assert.SkipWhen(theirs.Count == 0, "pdftotext is not available; install poppler-utils");

        // Seven runs on each deck's first slide: a title, then three bullets each followed by its
        // own text. A reader that drew no markers would have four.
        ours.Count.ShouldBe(theirs.Count, $"{fileName}: runs on the outline slide");

        for (int i = 0; i < theirs.Count; i++)
        {
            PdfTextRun mine = ours[i];
            PdfTextRun reference = theirs[i];
            string where = $"{fileName}: run {i + 1}";

            mine.X.ShouldBe(reference.X, TolerancePoints, $"{where}: pen across");
            mine.Y.ShouldBe(reference.Y, TolerancePoints, $"{where}: baseline");
            mine.FontSize.ShouldBe(reference.FontSize, 0.02, $"{where}: font size");
        }
    }

    /// <summary>
    /// A hidden slide is not a page, in either format.
    /// </summary>
    /// <remarks>
    /// The ODF flag is <c>presentation:visibility</c> on the page's <em>drawing-page style</em>
    /// rather than an attribute on the page, so a reader looking for one on the page finds nothing
    /// and lays out a slide LibreOffice's PDF export leaves out — which is the whole of the
    /// difference between three pages and two on <c>slides-features.odp</c>.
    /// </remarks>
    [Theory]
    [InlineData("deck-features.pptx")]
    [InlineData("slides-features.odp")]
    public void ADeckHasAPagePerSlideItShows(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        List<PdfPageSize> ours = PdfPageSizes.Read(Ours(path));
        List<PdfPageSize> theirs = PdfPageSizes.Read(_libreOffice.ConvertToPdf(path, _workDirectory));

        Assert.SkipWhen(theirs.Count == 0, "pdftotext is not available; install poppler-utils");

        ours.Count.ShouldBe(theirs.Count, $"{fileName}: page count");
    }

    /// <summary>The first slide's runs, which is the bulleted outline in both decks.</summary>
    private static List<PdfTextRun> Outline(List<PdfTextRun> runs)
        => [.. runs.Where(run => run.PageIndex == 0)];

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
