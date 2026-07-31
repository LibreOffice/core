using Paperless.Core.Documents;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Paperless.WordProcessing;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Checks that a document changing its page setup part way through changes it where LibreOffice does.
/// </summary>
/// <remarks>
/// <para>
/// A section boundary changes everything at once — the paper size, the margins, the width lines break at,
/// and the headers — so it is the one place where being wrong is wrong in several ways simultaneously. The
/// corpus document turns a portrait A4 document sideways for one stretch and back again, with different
/// margins in the landscape part, which separates the two things that can fail: the page's own size, and
/// the text area inside it.
/// </para>
/// <para>
/// The page <em>size</em> is compared directly, since the PDF states it outright and a reader that carried
/// the first section's geometry throughout gets three portrait pages where LibreOffice gives two portrait
/// and one landscape. The text is then compared per page, which is what catches a paragraph laid out at the
/// wrong width: its lines break in different places, so the words on each page differ even when the page
/// count happens to agree.
/// </para>
/// </remarks>
public sealed class SectionComparisonTests : IDisposable
{
    /// <summary>How far a drawn word may differ from LibreOffice's, in points.</summary>
    private const double TolerancePoints = 0.1;

    /// <summary>The horizontal offset LibreOffice's PDF export adds to every pen position, in points.</summary>
    private const double PdfPenOffsetPoints = 0.1;

    /// <summary>How far a page dimension may differ, in points.</summary>
    /// <remarks>
    /// A twentieth of a point. A page size is stated in the document and written into the PDF unchanged, so
    /// the only difference is the conversion — and a landscape page mistaken for a portrait one is out by a
    /// third of the sheet rather than by a fraction of a point.
    /// </remarks>
    private const double SizeTolerancePoints = 0.05;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-sections").FullName;

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
    [InlineData("sections.fodt")]
    [InlineData("sections.odt")]
    [InlineData("sections.docx")]
    [InlineData("sections.rtf")]
    [InlineData("sections.doc")]
    public void EveryPageTakesItsOwnSectionsGeometry(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        WordProcessingPages pages = Layout(path);
        List<PdfPageSize> rendered = PdfPageSizes.Read(_libreOffice.ConvertToPdf(path, _workDirectory));

        Assert.SkipWhen(rendered.Count == 0, "pdftotext is not available; see check-env.sh");

        pages.Pages.Count.ShouldBe(rendered.Count, $"{fileName}: page count");

        // At least one page has to differ from its neighbour, or the document proves nothing about
        // sections — a corpus file that lost its landscape stretch would otherwise pass.
        rendered.Select(size => Math.Round(size.Width, 1)).Distinct().Count()
            .ShouldBeGreaterThan(1, $"{fileName}: every rendered page is the same size");

        for (int page = 0; page < rendered.Count; page++)
        {
            LaidOutPage laid = pages.Pages[page];

            Math.Abs(laid.Size.Width.Points - rendered[page].Width).ShouldBeLessThanOrEqualTo(
                SizeTolerancePoints,
                $"{fileName}: page {page + 1} is {laid.Size.Width.Points:F2} pt wide laid out, "
                + $"{rendered[page].Width:F2} pt rendered");

            Math.Abs(laid.Size.Height.Points - rendered[page].Height).ShouldBeLessThanOrEqualTo(
                SizeTolerancePoints,
                $"{fileName}: page {page + 1} is {laid.Size.Height.Points:F2} pt tall laid out, "
                + $"{rendered[page].Height:F2} pt rendered");
        }
    }

    [Theory]
    [InlineData("sections.fodt")]
    [InlineData("sections.odt")]
    [InlineData("sections.docx")]
    [InlineData("sections.rtf")]
    [InlineData("sections.doc")]
    public void EveryPageHoldsTheWordsLibreOfficePutsOnIt(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        List<List<DrawnWord>> drawn = Drawn(path);
        List<PdfWord> everything = PdfWords.Read(_libreOffice.ConvertToPdf(path, _workDirectory));

        Assert.SkipWhen(everything.Count == 0, "pdftotext is not available; see check-env.sh");

        int pages = everything.Select(word => word.PageIndex).Distinct().Count();
        drawn.Count.ShouldBe(pages, $"{fileName}: page count");

        int compared = 0;
        for (int page = 0; page < pages; page++)
        {
            List<PdfWord> reference = InReadingOrder(
                [.. everything.Where(word => word.PageIndex == page)]);
            List<DrawnWord> mine = InDrawnOrder(drawn[page]);

            string.Join(' ', mine.Select(word => word.Text))
                .ShouldBe(
                    string.Join(' ', reference.Select(word => word.Text)),
                    $"{fileName}: page {page + 1}'s drawn text differs from the rendered text");

            // The first word of each line, as elsewhere: it starts where the geometry says, while a word
            // further along carries the reference's own per-glyph rounding. Here the left margin is the
            // thing under test — the landscape section has a wider one — so it is also the interesting
            // position.
            for (int i = 0; i < reference.Count; i++)
            {
                if (i > 0 && Math.Abs(reference[i].Top - reference[i - 1].Top) < 0.5) continue;

                Math.Abs(mine[i].Left - (reference[i].Left - PdfPenOffsetPoints))
                    .ShouldBeLessThanOrEqualTo(
                        TolerancePoints,
                        $"{fileName}: page {page + 1}, word {i + 1} (\"{reference[i].Text}\") starts at "
                        + $"{mine[i].Left:F3} pt drawn, "
                        + $"{reference[i].Left - PdfPenOffsetPoints:F3} pt rendered");

                compared++;
            }
        }

        compared.ShouldBeGreaterThan(30, $"{fileName}: too few lines compared to prove anything");
    }

    // ------------------------------------------------------------------------- the machinery

    private static List<PdfWord> InReadingOrder(List<PdfWord> words)
        => [.. words.OrderBy(word => Math.Round(word.Top, 1)).ThenBy(word => word.Left)];

    private static List<DrawnWord> InDrawnOrder(List<DrawnWord> words)
        => [.. words.OrderBy(word => Math.Round(word.Baseline, 1)).ThenBy(word => word.Left)];

    private static WordProcessingPages Layout(string path)
    {
        using FileStream stream = File.OpenRead(path);
        using DocumentSource source = DocumentSource.FromStream(stream, Path.GetFileName(path));
        using IDocument document = new WordProcessingReader().Read(source);

        return (WordProcessingPages)((IPaginatedDocument)document).Layout();
    }

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
