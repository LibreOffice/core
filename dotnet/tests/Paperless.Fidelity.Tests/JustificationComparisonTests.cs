using Paperless.Core.Documents;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Paperless.WordProcessing;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Checks that an aligned line puts its words where LibreOffice puts them.
/// </summary>
/// <remarks>
/// <para>
/// Differentially, and on single-line paragraphs, for two separate reasons.
/// </para>
/// <para>
/// Differentially because a PDF's word boxes are not accurate enough to compare directly: LibreOffice
/// writes each glyph's advance into the embedded subset rounded to a thousandth of an em and corrects the
/// kerning with <c>TJ</c> adjustments at the same granularity, so <c>pdftotext</c>'s idea of where a word
/// ends drifts from the layout's by a fraction of a point per glyph — over a full line, by more than a
/// point. So the corpus document holds every paragraph twice, once aligned and once start-aligned, and
/// what is compared is the <em>difference</em> between the two copies. Both embed one subset and shape the
/// same glyphs in the same order, so every quantisation error appears in both and subtracts out.
/// </para>
/// <para>
/// Single-line because LibreOffice 24.2 breaks a justified paragraph's lines differently from the same
/// text left ragged — measured: a line that holds twenty-one words ragged holds nineteen justified, though
/// the ragged line ends well inside the margin. Whatever the rule is, it is not the greedy break Paperless
/// and Writer agree on everywhere else, and a multi-line paragraph would compare words on different lines.
/// One line per paragraph removes the question and leaves exactly what this test is for: how far the
/// alignment moved each word. The deviation is recorded in the library's TODO.
/// </para>
/// </remarks>
public sealed class JustificationComparisonTests : IDisposable
{
    /// <summary>How far a drawn alignment may differ from LibreOffice's, in points.</summary>
    /// <remarks>
    /// <para>
    /// A tenth of a point, two twips, per blank. The residual it has to admit is about 0.7 twips: Writer
    /// measures a line roughly 0.15% wider than HarfBuzz does — twelve twips on a 395 pt line, measured on
    /// this document — and justification divides that difference over the line's blanks along with
    /// everything else. Every other comparison is blind to it, because <c>pdftotext</c>'s own quantisation
    /// is larger; a differential test is the only thing that can see it at all.
    /// </para>
    /// <para>
    /// Still tight enough for what this checks. Sharing the slack over the characters instead of the
    /// blanks, or over the paragraph's blanks instead of the line's, changes the stretch several-fold, and
    /// rounding it to whole twips instead of hundredths misses by up to half a twip per blank. The corpus
    /// document's lines hold five, seventeen and twenty-one blanks so that no wrong divisor passes by
    /// coincidence.
    /// </para>
    /// </remarks>
    private const double TolerancePoints = 0.1;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-justified").FullName;

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
    [InlineData("justified.fodt")]
    [InlineData("justified.docx")]
    [InlineData("justified.doc")]
    [InlineData("justified.rtf")]
    public void EachWordIsAlignedAsLibreOfficeAlignsIt(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        List<DrawnWord> drawn = Drawn(path);
        List<PdfWord> rendered = InReadingOrder(
            PdfWords.Read(_libreOffice.ConvertToPdf(path, _workDirectory)));

        Assert.SkipWhen(
            rendered.Count == 0,
            "pdftotext is not available; install poppler-utils — see check-env.sh");

        // The same words in the same order first: a mismatch means the lines broke somewhere else, and
        // every position after that would fail for the one reason.
        string.Join(' ', drawn.Select(word => word.Text))
            .ShouldBe(
                string.Join(' ', rendered.Select(word => word.Text)),
                $"{fileName}: the drawn text differs from the rendered text");

        // The document is two halves: the aligned paragraphs, then the same text start-aligned.
        int half = rendered.Count / 2;
        (half * 2).ShouldBe(
            rendered.Count, $"{fileName}: the corpus document's two halves are not the same length");

        // Per blank rather than cumulatively. The stretch of one blank is what the code computes; the
        // position of the twentieth word on a line is that number times twenty, and it also carries
        // twenty times whatever the two engines disagree about in the line's own width — Writer measures
        // a 395 pt line about 0.15% wider than HarfBuzz does, which is 0.7 twips of stretch per blank and
        // a whole point by the end of the line. Comparing the increments keeps that at its per-blank size.
        int compared = 0;
        for (int i = 1; i < half; i++)
        {
            // Only within a line: the increment across a line break is not one blank's stretch.
            if (Math.Abs(rendered[i].Top - rendered[i - 1].Top) > 0.5) continue;

            rendered[i].Text.ShouldBe(
                rendered[i + half].Text,
                $"{fileName}: word {i + 1} — the document's two halves are misaligned");

            double expected = (rendered[i].Left - rendered[i + half].Left)
                              - (rendered[i - 1].Left - rendered[i - 1 + half].Left);
            double actual = (drawn[i].Left - drawn[i + half].Left)
                            - (drawn[i - 1].Left - drawn[i - 1 + half].Left);

            Math.Abs(actual - expected).ShouldBeLessThanOrEqualTo(
                TolerancePoints,
                $"{fileName}: the blank before word {i + 1} (\"{rendered[i].Text}\") was stretched "
                + $"{actual:F3} pt, LibreOffice {expected:F3} pt");

            compared++;
        }

        // The lines really are stretched, so a document edited into raggedness does not leave this
        // quietly asserting that zero equals zero.
        compared.ShouldBeGreaterThan(30, $"{fileName}: only {compared} blanks were compared");

        drawn.Where((word, i) => i < half && Math.Abs(word.Left - drawn[i + half].Left) > 1.0)
            .Count()
            .ShouldBeGreaterThan(half / 3, $"{fileName}: too few words were moved at all");
    }

    [Theory]
    [InlineData("justified.fodt")]
    [InlineData("justified.docx")]
    [InlineData("justified.doc")]
    [InlineData("justified.rtf")]
    public void AJustifiedLineReachesTheMarginAndItsLastLineDoesNot(string fileName)
    {
        string path = Corpus.Require(fileName);
        List<DrawnWord> drawn = Drawn(path);

        // Every stretched line ends at the same place, whatever it holds — that is what justifying is —
        // and the four lines hold six, eighteen, twenty-two and five words, so a wrong divisor could not
        // put all four in one place by coincidence.
        List<double> ends = [.. drawn
            .Where(word => word.Baseline < drawn.Max(w => w.Baseline) / 2)
            .GroupBy(word => Math.Round(word.Baseline, 1))
            .Select(line => line.Max(word => word.Right))];

        ends.Count.ShouldBeGreaterThan(6, $"{fileName}: too few lines to prove anything");

        // The stretched lines, which are the ones that reach furthest.
        List<double> flush = [.. ends.OrderDescending().Take(4)];
        foreach (double right in flush)
        {
            Math.Abs(right - flush[0]).ShouldBeLessThanOrEqualTo(
                TolerancePoints,
                $"{fileName}: a stretched line ends at {right:F3} pt where another ends at "
                + $"{flush[0]:F3} pt");
        }

        // And "End", which is each paragraph's own last line, is not stretched at all: it is one word, so
        // it sits at the margin it started from.
        foreach (DrawnWord end in drawn.Where(word => word.Text == "End"))
        {
            end.Left.ShouldBeLessThan(
                flush[0] / 2, $"{fileName}: a paragraph's last line was stretched");
        }
    }

    // ------------------------------------------------------------------------- the machinery

    /// <summary>
    /// Puts a page's words into reading order: down the page, then across each line.
    /// </summary>
    /// <remarks>
    /// Both sides are sorted, because neither arrives that way. <c>pdftotext</c>'s order is its own
    /// business and it is not reading order for a document of short lines — it reported the second line of
    /// a two-line paragraph between the first line's first and second words. Sorting each side by its own
    /// vertical coordinate and then horizontally makes the two sequences comparable without either having
    /// to trust the other's idea of a line.
    /// </remarks>
    private static List<PdfWord> InReadingOrder(List<PdfWord> words)
        => [.. words
            .OrderBy(word => word.PageIndex)
            .ThenBy(word => Math.Round(word.Top, 1))
            .ThenBy(word => word.Left)];

    private static List<DrawnWord> Drawn(string path)
    {
        RecordingDrawingSink sink = new();

        using (FileStream stream = File.OpenRead(path))
        {
            using DocumentSource source = DocumentSource.FromStream(stream, Path.GetFileName(path));
            using IDocument document = new WordProcessingReader().Read(source);

            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            for (int i = 0; i < pages.Count; i++) pages[i].Draw(sink);
        }

        return
        [
            .. sink.Pages.SelectMany(page => DrawnWords.On(page)
                .OrderBy(word => Math.Round(word.Baseline, 1))
                .ThenBy(word => word.Left)),
        ];
    }
}
