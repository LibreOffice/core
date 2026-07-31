using Paperless.Core.Documents;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Paperless.WordProcessing;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Checks that endnotes collect where their document says, which is not always after the last page.
/// </summary>
/// <remarks>
/// <para>
/// A footnote and an endnote read identically and place completely differently, which is the whole point of
/// this file existing next to <see cref="FootnoteComparisonTests"/>. A footnote takes its room out of the
/// page that cites it; an endnote takes none at all, and instead adds pages at the end. So a reader that
/// treated one as the other fails here twice over: page one comes out short of body text, and the notes are
/// at the bottom of it instead of on a page of their own.
/// </para>
/// <para>
/// The numbering differs too, and not in a way any specification announces: LibreOffice cites footnotes
/// 1, 2, 3 and endnotes <em>i, ii, iii</em>. The corpus document states its citations as 2 and 5, so a reader
/// that believed the file, or that used the footnote sequence, produces the wrong text on both pages.
/// </para>
/// </remarks>
public sealed class EndnoteComparisonTests : IDisposable
{
    /// <summary>How far a drawn word may differ from LibreOffice's, in points.</summary>
    private const double TolerancePoints = 0.1;

    /// <summary>The horizontal offset LibreOffice's PDF export adds to every pen position, in points.</summary>
    private const double PdfPenOffsetPoints = 0.1;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-endnotes").FullName;

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
    [InlineData("endnotes.fodt")]
    [InlineData("endnotes.odt")]
    [InlineData("endnotes.docx")]
    // The DOC is the same document with its endnotes somewhere else, and it belongs in the same test for
    // exactly that reason: LibreOffice's WW8 export writes the DOP's `epc` as 0, "collect at the end of the
    // *section*", and its import turns that into `SwFormatEndAtTextEnd` (`ww8par6.cxx`, `if (0 == epc)`). So
    // these notes render in the page-bottom note area instead — one page rather than two — and a reader that
    // placed notes by class rather than by position lays the same document out two ways depending on which
    // format it was saved in. RTF is absent for the separate reason given in `FootnoteComparisonTests`.
    [InlineData("endnotes.doc")]
    public void EndnotesCollectWhereTheirDocumentPutsThem(string fileName)
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
            List<PdfWord> reference = ReadingOrder.Of(
                [.. everything.Where(word => word.PageIndex == page)]);
            List<DrawnWord> mine = ReadingOrder.Of(drawn[page]);

            string.Join(' ', mine.Select(word => word.Text))
                .ShouldBe(
                    string.Join(' ', reference.Select(word => word.Text)),
                    $"{fileName}: page {page + 1}'s drawn text differs from the rendered text");

            for (int i = 0; i < reference.Count; i++)
            {
                if (i > 0 && Math.Abs(reference[i].Top - reference[i - 1].Top) < 0.5) continue;

                Math.Abs(mine[i].Left - (reference[i].Left - PdfPenOffsetPoints))
                    .ShouldBeLessThanOrEqualTo(
                        TolerancePoints,
                        $"{fileName}: page {page + 1}, word {i + 1} (\"{reference[i].Text}\") starts at "
                        + $"{mine[i].Left:F3} pt drawn, "
                        + $"{reference[i].Left - PdfPenOffsetPoints:F3} pt rendered");

                // Vertically as a difference from the first word *of the same size*, which is what cancels the
                // ascent — see ReadingOrder.FirstOfSize. On a page holding both body text and notes the two
                // sizes differ, so anchoring at the page's first word leaves 0.95 pt of ascent difference
                // behind and reads as a placement error.
                int anchor = ReadingOrder.FirstOfSize(mine, mine[i].Size);

                double drawnGap = mine[i].Baseline - mine[anchor].Baseline;
                double renderedGap = reference[i].Top - reference[anchor].Top;

                Math.Abs(drawnGap - renderedGap).ShouldBeLessThanOrEqualTo(
                    TolerancePoints,
                    $"{fileName}: page {page + 1}, word {i + 1} (\"{reference[i].Text}\") sits "
                    + $"{drawnGap:F3} pt below the first word of its size drawn, "
                    + $"{renderedGap:F3} pt rendered");

                compared++;
            }
        }

        compared.ShouldBeGreaterThan(20, $"{fileName}: too few lines compared to prove anything");
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
