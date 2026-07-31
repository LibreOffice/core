using Paperless.Core.Documents;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Paperless.WordProcessing;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Checks that text in a multi-column section flows down one column and on into the next.
/// </summary>
/// <remarks>
/// <para>
/// Two things have to be right and they fail differently. The column's <em>width</em> decides where every
/// line breaks — it is the text width less the gaps, divided by the count, and getting it wrong changes the
/// words on every line. The column's <em>position</em> decides where the second column's text starts, and
/// getting that wrong leaves the right words in the wrong place. A reader that ignored columns altogether
/// would get both wrong at once and also put half as much on each page.
/// </para>
/// <para>
/// The vertical check is what proves the flow rather than the geometry: a column's second line sits one
/// line height below its first, and the second <em>column</em>'s first line sits back at the top of the
/// page. So the top of the page is reached twice, which is the whole difference between columns and a
/// narrower text area.
/// </para>
/// </remarks>
public sealed class ColumnComparisonTests : IDisposable
{
    /// <summary>How far a drawn word may differ from LibreOffice's, in points.</summary>
    private const double TolerancePoints = 0.1;

    /// <summary>The horizontal offset LibreOffice's PDF export adds to every pen position, in points.</summary>
    private const double PdfPenOffsetPoints = 0.1;

    /// <summary>
    /// The gap between two words, in points, beyond which they are in different columns.
    /// </summary>
    /// <remarks>
    /// The column gap in the corpus document is a centimetre — over twenty-eight points — and a word space
    /// at eleven point is under three. Six separates them without the test needing to know the geometry it
    /// is testing.
    /// </remarks>
    private const double GapBetweenColumns = 6;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-columns").FullName;

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
    [InlineData("columns.fodt")]
    [InlineData("columns.odt")]
    [InlineData("columns.docx")]
    [InlineData("columns.doc")]
    [InlineData("columns.rtf")]
    public void TextFillsOneColumnBeforeTheNext(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        List<List<DrawnWord>> drawn = Drawn(path);
        List<PdfWord> everything = PdfWords.Read(_libreOffice.ConvertToPdf(path, _workDirectory));

        Assert.SkipWhen(everything.Count == 0, "pdftotext is not available; see check-env.sh");

        int pages = everything.Select(word => word.PageIndex).Distinct().Count();
        drawn.Count.ShouldBe(pages, $"{fileName}: page count");

        int compared = 0;
        int columnStarts = 0;

        for (int page = 0; page < pages; page++)
        {
            List<PdfWord> reference = InReadingOrder(
                [.. everything.Where(word => word.PageIndex == page)]);
            List<DrawnWord> mine = InDrawnOrder(drawn[page]);

            string.Join(' ', mine.Select(word => word.Text))
                .ShouldBe(
                    string.Join(' ', reference.Select(word => word.Text)),
                    $"{fileName}: page {page + 1}'s drawn text differs from the rendered text");

            for (int i = 0; i < reference.Count; i++)
            {
                if (i > 0 && !StartsALine(reference, i)) continue;

                Math.Abs(mine[i].Left - (reference[i].Left - PdfPenOffsetPoints))
                    .ShouldBeLessThanOrEqualTo(
                        TolerancePoints,
                        $"{fileName}: page {page + 1}, word {i + 1} (\"{reference[i].Text}\") starts at "
                        + $"{mine[i].Left:F3} pt drawn, "
                        + $"{reference[i].Left - PdfPenOffsetPoints:F3} pt rendered");

                // Vertically as a difference from the page's first word, which cancels the ascent.
                double drawnGap = mine[i].Baseline - mine[0].Baseline;
                double renderedGap = reference[i].Top - reference[0].Top;

                Math.Abs(drawnGap - renderedGap).ShouldBeLessThanOrEqualTo(
                    TolerancePoints,
                    $"{fileName}: page {page + 1}, word {i + 1} (\"{reference[i].Text}\") sits "
                    + $"{drawnGap:F3} pt below the page's first word drawn, {renderedGap:F3} pt rendered");

                compared++;

                // A line that starts near the top of the page but well to the right of the left margin is a
                // second column beginning. Counting them is what distinguishes real columns from a document
                // that merely came out narrow.
                if (renderedGap < 1 && reference[i].Left - reference[0].Left > GapBetweenColumns)
                {
                    columnStarts++;
                }
            }
        }

        compared.ShouldBeGreaterThan(40, $"{fileName}: too few lines compared to prove anything");
        columnStarts.ShouldBe(pages, $"{fileName}: expected one second column per page");
    }

    // ------------------------------------------------------------------------- the machinery

    /// <summary>
    /// Whether a word begins a line, where a change of column begins one too.
    /// </summary>
    /// <remarks>
    /// Not "the first word at this vertical position": two columns' lines share their tops, so every line
    /// but the first column's would go unchecked. A large horizontal gap from the previous word is what a
    /// column boundary looks like, since words within a line are a space apart.
    /// </remarks>
    private static bool StartsALine(List<PdfWord> words, int index)
        => Math.Abs(words[index].Top - words[index - 1].Top) >= 0.5
           || words[index].Left - words[index - 1].Right > GapBetweenColumns;

    private static List<PdfWord> InReadingOrder(List<PdfWord> words)
        => [.. words.OrderBy(word => Math.Round(word.Top, 1)).ThenBy(word => word.Left)];

    private static List<DrawnWord> InDrawnOrder(List<DrawnWord> words)
        => [.. words.OrderBy(word => Math.Round(word.Baseline, 1)).ThenBy(word => word.Left)];

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
