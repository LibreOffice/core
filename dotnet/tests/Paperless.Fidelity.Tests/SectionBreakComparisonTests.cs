using Paperless.Core.Documents;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Paperless.WordProcessing;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Checks that a section break costs a page exactly when LibreOffice makes it cost one.
/// </summary>
/// <remarks>
/// <para>
/// The kinds that obviously cost a page are covered by <see cref="SectionComparisonTests"/>. This is for the
/// one that does not obviously anything: a <em>column</em> break, RTF's <c>\sbkcol</c> and OOXML's
/// <c>nextColumn</c>, which starts the section where the next column would be. Word 2013 and later turn that
/// into a page break whenever the columns do not line up — no previous section, fewer than two columns, or a
/// different count than before — which is most of the time, and LibreOffice's importer says so in as many
/// words. A reader that treated the kind as continuous puts three sections on one page where LibreOffice
/// gives three pages.
/// </para>
/// <para>
/// The corpus document also covers a bug that has nothing to do with the kind and everything to do with
/// columns: its first section has two of them and is only part full, so ending the page means ending
/// <em>both</em> columns. A page break that stopped at the column boundary would put section two beside
/// section one rather than after it.
/// </para>
/// </remarks>
public sealed class SectionBreakComparisonTests : IDisposable
{
    /// <summary>How far a drawn word may differ from LibreOffice's, in points.</summary>
    private const double TolerancePoints = 0.1;

    /// <summary>The horizontal offset LibreOffice's PDF export adds to every pen position, in points.</summary>
    private const double PdfPenOffsetPoints = 0.1;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-section-breaks").FullName;

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
    [InlineData("section-columns.rtf")]
    [InlineData("section-columns.docx")]
    [InlineData("section-columns.doc")]
    public void AColumnBreakCostsAPageWhenTheColumnsDoNotLineUp(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        List<List<DrawnWord>> drawn = Drawn(path);
        List<PdfWord> everything = PdfWords.Read(_libreOffice.ConvertToPdf(path, _workDirectory));

        Assert.SkipWhen(everything.Count == 0, "pdftotext is not available; see check-env.sh");

        int pages = everything.Select(word => word.PageIndex).Distinct().Count();

        // One page per section, which is the whole assertion: a reader reading the kind as continuous gets
        // one page, and one that ended the column rather than the page gets two.
        pages.ShouldBe(3, $"{fileName}: the reference should put each section on its own page");
        drawn.Count.ShouldBe(pages, $"{fileName}: page count");

        for (int page = 0; page < pages; page++)
        {
            List<PdfWord> reference = ReadingOrder.Of(
                [.. everything.Where(word => word.PageIndex == page)]);
            List<DrawnWord> mine = ReadingOrder.Of(drawn[page]);

            string.Join(' ', mine.Select(word => word.Text))
                .ShouldBe(
                    string.Join(' ', reference.Select(word => word.Text)),
                    $"{fileName}: page {page + 1}'s drawn text differs from the rendered text");

            mine.Count.ShouldBeGreaterThan(0, $"{fileName}: page {page + 1} drew nothing");

            Math.Abs(mine[0].Left - (reference[0].Left - PdfPenOffsetPoints))
                .ShouldBeLessThanOrEqualTo(
                    TolerancePoints,
                    $"{fileName}: page {page + 1} starts at {mine[0].Left:F3} pt drawn, "
                    + $"{reference[0].Left - PdfPenOffsetPoints:F3} pt rendered");
        }
    }

    // ------------------------------------------------------------------------- the machinery

    private static List<List<DrawnWord>> Drawn(string path)
    {
        RecordingDrawingSink sink = new();

        using (FileStream stream = File.OpenRead(path))
        {
            using DocumentSource source = DocumentSource.FromStream(stream, Path.GetFileName(path));
            using IDocument document = new WordProcessingReader().Read(source);

            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            for (int i = 0; i < pages.Count; i++) pages[i].Draw(sink);
        }

        return [.. sink.Pages.Select(page => DrawnWords.On(page))];
    }
}
