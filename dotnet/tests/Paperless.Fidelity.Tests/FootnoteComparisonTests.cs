using Paperless.Core.Documents;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Paperless.WordProcessing;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Checks that a page's footnotes sit where LibreOffice sits them, and take their room from the body.
/// </summary>
/// <remarks>
/// <para>
/// Footnotes are the one layout feature that changes <em>pagination</em> rather than only appearance: the
/// note area takes its room out of the body's, so a page with notes holds less text — and adding a note can
/// push the line that cites it onto the next page, which removes the note again. So the comparison is not
/// only about where the notes are drawn but about where the body text stops, and a reader that placed the
/// notes correctly while forgetting to charge the body for them would fail on the body rather than the
/// notes.
/// </para>
/// <para>
/// The citation is compared too, and it is worth saying why that is not a formality. LibreOffice draws the
/// number twice — as a superscript at the anchor in the sentence, and again at the start of the note itself
/// — and it <em>renumbers</em>, ignoring what <c>text:note-citation</c> says and counting in document order.
/// The corpus document states citations of 2 and 5 deliberately, so that a reader taking the file at its
/// word produces "2" where LibreOffice produces "1".
/// </para>
/// </remarks>
public sealed class FootnoteComparisonTests : IDisposable
{
    /// <summary>How far a drawn word may differ from LibreOffice's, in points.</summary>
    private const double TolerancePoints = 0.1;

    /// <summary>The horizontal offset LibreOffice's PDF export adds to every pen position, in points.</summary>
    private const double PdfPenOffsetPoints = 0.1;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-footnotes").FullName;

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
    [InlineData("footnotes.fodt")]
    // The second document is the one that tests the *reservation* rather than the placement: its notes are
    // long and sit near a page end, so page one holds twelve paragraphs where without them it would hold
    // thirteen. A reader that placed the notes and forgot to charge the body for them fails on the page
    // count and on every word of page two.
    [InlineData("footnote-pages.fodt")]
    [InlineData("footnotes.odt")]
    [InlineData("footnotes.docx")]
    [InlineData("footnote-pages.odt")]
    [InlineData("footnote-pages.docx")]
    public void EveryNoteSitsAtTheFootOfItsOwnPage(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        List<List<DrawnWord>> drawn = Drawn(path);
        List<PdfWord> everything = PdfWords.Read(_libreOffice.ConvertToPdf(path, _workDirectory));

        Assert.SkipWhen(everything.Count == 0, "pdftotext is not available; see check-env.sh");

        int pages = everything.Select(word => word.PageIndex).Distinct().Count();
        drawn.Count.ShouldBe(pages, $"{fileName}: page count");

        int compared = 0;
        int noteLines = 0;

        for (int page = 0; page < pages; page++)
        {
            List<PdfWord> reference = InReadingOrder(
                [.. everything.Where(word => word.PageIndex == page)]);
            List<DrawnWord> mine = InDrawnOrder(drawn[page]);

            // The citations are in here: a superscript number at each anchor, and the same number fused to
            // the first word of the note it cites. A reader that dropped either, or numbered from the file
            // rather than counting, fails on this line.
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

                // Vertically as a difference from the first word *of the same size*. A box top sits above
                // its baseline by the font's ascent, which the PDF never states — so the comparison has to
                // be of differences, and it only cancels between words the same size. A footnote is set
                // smaller than the body, so measuring a note line against a body line leaves the ascent
                // difference behind: on this document that is 0.95 pt, which reads as a placement error and
                // is not one.
                int anchor = FirstOfSize(mine, mine[i].Size);

                double drawnGap = mine[i].Baseline - mine[anchor].Baseline;
                double renderedGap = reference[i].Top - reference[anchor].Top;

                Math.Abs(drawnGap - renderedGap).ShouldBeLessThanOrEqualTo(
                    TolerancePoints,
                    $"{fileName}: page {page + 1}, word {i + 1} (\"{reference[i].Text}\") sits "
                    + $"{drawnGap:F3} pt below the first word of its size drawn, "
                    + $"{renderedGap:F3} pt rendered");

                compared++;

                // A note line is one set in the note's smaller size. Counting them is what stops a corpus
                // document whose notes were silently dropped from passing on the body alone.
                if (mine[i].Size < 10.5) noteLines++;
            }
        }

        compared.ShouldBeGreaterThan(30, $"{fileName}: too few lines compared to prove anything");
        noteLines.ShouldBeGreaterThanOrEqualTo(
            2, $"{fileName}: expected at least one line per footnote");
    }

    // ------------------------------------------------------------------------- the machinery

    /// <summary>The first word drawn at a given size, which anchors that size's vertical comparison.</summary>
    private static int FirstOfSize(List<DrawnWord> words, double size)
    {
        for (int i = 0; i < words.Count; i++)
        {
            if (Math.Abs(words[i].Size - size) < 0.01) return i;
        }

        return 0;
    }

    /// <summary>
    /// How far two words' verticals may differ and still be one line, in points.
    /// </summary>
    /// <remarks>
    /// Six points, which is more than a superscript's rise and less than a line height — and that gap is the
    /// whole reason this exists. A citation is raised about two points above its line, so sorting on the
    /// exact vertical puts it <em>before</em> every word of the line it belongs to, and the comparison then
    /// fails on word order for a document laid out perfectly. Both sides are grouped into lines first
    /// instead, and only then ordered across the page.
    /// </remarks>
    private const double SameLine = 6;

    private static List<PdfWord> InReadingOrder(List<PdfWord> words)
        => InLines(words, word => word.Top, word => word.Left);

    private static List<DrawnWord> InDrawnOrder(List<DrawnWord> words)
        => InLines(words, word => word.Baseline, word => word.Left);

    /// <summary>Groups words into lines by their vertical, then orders the lines down the page.</summary>
    private static List<T> InLines<T>(
        List<T> words, Func<T, double> vertical, Func<T, double> horizontal)
    {
        List<T> sorted = [.. words.OrderBy(vertical)];
        List<List<T>> lines = [];

        foreach (T word in sorted)
        {
            if (lines.Count > 0 && vertical(word) - vertical(lines[^1][0]) <= SameLine)
            {
                lines[^1].Add(word);
                continue;
            }

            lines.Add([word]);
        }

        return [.. lines.SelectMany(line => line.OrderBy(horizontal))];
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
