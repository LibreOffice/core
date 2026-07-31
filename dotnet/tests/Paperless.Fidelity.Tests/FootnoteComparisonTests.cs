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
    [InlineData("footnotes.doc")]
    [InlineData("footnote-pages.doc")]
    // RTF is deliberately absent, and the reason is in LibreOffice rather than here: its RTF import drops the
    // character and paragraph formatting stated inside a `{\*\footnote …}` group and falls back to the
    // document's defaults. A note the file sets in Carlito at 10 pt with no indent renders in Liberation Serif
    // with a 340-twip hanging indent, which moves every word of every note line. Confirmed on a hand-written
    // three-line RTF as well as on the corpus files, so it is not an artefact of how these were exported.
    // Paperless reads what the file says; `FootnoteReadingTests` checks it structurally instead. DOC has no
    // such problem — LibreOffice's WW8 import keeps the note's formatting, so both files are compared here.
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

    [Theory]
    [InlineData("footnotes.fodt")]
    [InlineData("footnotes.odt")]
    [InlineData("footnotes.docx")]
    [InlineData("footnotes.doc")]
    public void EveryCitationIsSetAndRaisedAsLibreOfficeSetsIt(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        List<PdfTextRun> reference = PdfTextRuns.Read(
            _libreOffice.ConvertToPdf(path, _workDirectory));

        Assert.SkipWhen(reference.Count == 0, "the reference PDF held no positioned text");

        List<Pen> mine = Pens(
            [.. Rendered(path)[0].Runs.Select(
                run => (run.Origin.X.Points, run.Origin.Y.Points, run.Run.FontSize.Points))]);

        List<Pen> theirs = Pens(
            [.. reference.Where(run => run.PageIndex == 0)
                .Select(run => (run.X - PdfPenOffsetPoints, run.Y, run.FontSize))]);

        // Collapsed rather than compared run for run, because the two sides split runs differently and
        // legitimately: once a citation is drawn at the same size and on the same baseline as the text after
        // it, nothing distinguishes the two — but LibreOffice still writes them as separate text portions,
        // because it knew they came from different runs of the document.
        string drawn = string.Join(" | ", mine.Select(pen => pen.ToString()));
        string rendered = string.Join(" | ", theirs.Select(pen => pen.ToString()));

        drawn.ShouldBe(
            rendered,
            $"{fileName}: page 1's pens differ from LibreOffice's, as "
            + "'<line start> <size>@<rise above the line's baseline>'");
    }

    // ------------------------------------------------------------------------- the machinery

    /// <summary>
    /// One drawn portion, as the pair of numbers a word-box comparison cannot see.
    /// </summary>
    /// <remarks>
    /// The size and the rise, and the rise is the point: the PDF's <c>Td</c> is the pen, so a superscript's
    /// shift is in it directly with no font ascent in the way. A box's top carries that ascent, which is why
    /// the other test here can only ever compare boxes to boxes of the same size.
    /// </remarks>
    /// <param name="LineStart">Where the line this portion belongs to begins, in points.</param>
    /// <param name="Size">Its em size, in points.</param>
    /// <param name="Rise">How far above its line's baseline it sits, in points.</param>
    private readonly record struct Pen(double LineStart, double Size, double Rise)
    {
        public override string ToString()
            => $"{LineStart:F1} {Size:F2}@{Rise:F2}";
    }

    /// <summary>
    /// Turns positioned portions into pens, one per formatting change per line.
    /// </summary>
    /// <remarks>
    /// Grouped into lines by the lowest baseline within <see cref="SameLine"/>, because a raised portion's
    /// own pen is not its line's baseline and comparing the two across documents would compare two rises
    /// against nothing. Adjacent portions that agree on both numbers collapse into one, so that a producer
    /// splitting a uniform stretch in two does not read as a difference.
    /// </remarks>
    private static List<Pen> Pens(List<(double X, double Y, double Size)> portions)
    {
        List<Pen> pens = [];

        foreach ((double x, double y, double size) in portions)
        {
            double baseline = portions
                .Where(other => Math.Abs(other.Y - y) <= SameLine)
                .Max(other => other.Y);

            double start = portions
                .Where(other => Math.Abs(other.Y - baseline) <= SameLine)
                .Min(other => other.X);

            Pen pen = new(start, Math.Round(size, 2), Math.Round(baseline - y, 2));
            if (pens.Count > 0 && pens[^1] == pen) continue;

            pens.Add(pen);
        }

        return pens;
    }

    /// <summary>The first word drawn at a given size, which anchors that size's vertical comparison.</summary>
    private static int FirstOfSize(List<DrawnWord> words, double size)
    {
        for (int i = 0; i < words.Count; i++)
        {
            if (Math.Abs(words[i].Size - size) < 0.01) return i;
        }

        return 0;
    }

    /// <summary>How far two portions' verticals may differ and still be one line, in points.</summary>
    /// <remarks><see cref="ReadingOrder.SameLinePoints"/>, for the reason given there.</remarks>
    private const double SameLine = ReadingOrder.SameLinePoints;

    private static List<PdfWord> InReadingOrder(List<PdfWord> words) => ReadingOrder.Of(words);

    private static List<DrawnWord> InDrawnOrder(List<DrawnWord> words) => ReadingOrder.Of(words);

    private static List<List<DrawnWord>> Drawn(string path)
        => [.. Rendered(path).Select(DrawnWords.On)];

    /// <summary>Lays a document out and records what it drew, page by page.</summary>
    private static IReadOnlyList<DrawnPage> Rendered(string path)
    {
        RecordingDrawingSink sink = new();

        using (FileStream stream = File.OpenRead(path))
        {
            using DocumentSource source = DocumentSource.FromStream(stream, Path.GetFileName(path));
            using IDocument document = new WordProcessingReader().Read(source);

            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            for (int i = 0; i < pages.Count; i++) pages[i].Draw(sink);
        }

        return sink.Pages;
    }
}
