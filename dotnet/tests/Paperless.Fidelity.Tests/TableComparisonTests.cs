using Paperless.Core.Documents;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Paperless.WordProcessing;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Checks that a table's cells hold their text where LibreOffice holds it.
/// </summary>
/// <remarks>
/// <para>
/// A table is where several separate measurements have to agree at once, which is what makes it worth its
/// own comparison. The column grid decides where a cell starts; the cell's padding decides where its text
/// starts inside that; the cell's width less its padding decides where the text breaks; and the tallest
/// cell of each row decides where the next row begins. Any one of those being wrong moves words, and each
/// moves them in its own direction — a bad column width moves them sideways, a bad row height downwards.
/// </para>
/// <para>
/// So the comparison is absolute horizontally and differential vertically, the same split the furniture
/// comparison uses and for the same reason: a cell's left edge is stated by the document's own arithmetic,
/// while the distance from a word box's top to its baseline is the font's ascent, which the PDF never
/// states.
/// </para>
/// <para>
/// The corpus document was written to make each of those separable. Its three columns have three different
/// widths, so a reader that divided the table's width equally — which is what LibreOffice itself does when
/// it cannot resolve the column styles — is caught at once. One cell has a much larger left padding than
/// the rest, so padding cannot be confused with the column edge. One cell holds enough text to wrap twice,
/// which is what proves the breaking width and makes its row taller than its neighbours. And the last row
/// spans two columns, which is the one case where a cell's width is not a column's.
/// </para>
/// </remarks>
public sealed class TableComparisonTests : IDisposable
{
    /// <summary>How far a drawn word may differ from LibreOffice's, in points.</summary>
    /// <remarks>
    /// A tenth of a point, two twips, as everywhere else. Worth noting what this bound proves here: a
    /// column width read in the wrong unit, or a cell padding defaulted to zero rather than to Writer's
    /// 0.097 cm, is out by several points rather than a fraction of one.
    /// </remarks>
    private const double TolerancePoints = 0.1;

    /// <summary>The horizontal offset LibreOffice's PDF export adds to every pen position, in points.</summary>
    private const double PdfPenOffsetPoints = 0.1;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-tables").FullName;

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
    [InlineData("table-grid.fodt")]
    [InlineData("table-grid.odt")]
    [InlineData("table-grid.docx")]
    public void EveryCellHoldsItsTextWhereLibreOfficeDoes(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        List<DrawnWord> drawn = InDrawnOrder(Drawn(path));
        List<PdfWord> rendered = InReadingOrder(
            PdfWords.Read(_libreOffice.ConvertToPdf(path, _workDirectory)));

        Assert.SkipWhen(
            rendered.Count == 0,
            "pdftotext is not available; install poppler-utils — see check-env.sh");

        // Text first: a table whose cells were laid out in the wrong order, or whose covered cell was
        // taken for a real one, fails here rather than on a position that would take longer to read.
        string.Join(' ', drawn.Select(word => word.Text))
            .ShouldBe(
                string.Join(' ', rendered.Select(word => word.Text)),
                $"{fileName}: the drawn text differs from the rendered text");

        int lines = 0;
        for (int i = 0; i < rendered.Count; i++)
        {
            // The first word of each line only, as in the furniture comparison: a word further along
            // carries the reference's own per-glyph rounding, while the first word of a line starts where
            // the geometry says. Inside a table that is also the interesting position — it is the one the
            // column grid and the cell padding decide.
            if (i > 0 && !StartsALine(rendered, i)) continue;

            string where = $"{fileName}: word {i + 1} (\"{rendered[i].Text}\")";

            Math.Abs(drawn[i].Left - (rendered[i].Left - PdfPenOffsetPoints))
                .ShouldBeLessThanOrEqualTo(
                    TolerancePoints,
                    $"{where}: starts at {drawn[i].Left:F3} pt drawn, "
                    + $"{rendered[i].Left - PdfPenOffsetPoints:F3} pt rendered");

            lines++;
        }

        lines.ShouldBeGreaterThan(10, $"{fileName}: too few cells compared to prove anything");

        // Vertically as differences from the first word, which cancels the ascent that separates a box top
        // from a baseline. This is what the row heights are checked by: every word below the first row is
        // where it is because of how tall the rows above it turned out.
        for (int i = 1; i < rendered.Count; i++)
        {
            double drawnGap = drawn[i].Baseline - drawn[0].Baseline;
            double renderedGap = rendered[i].Top - rendered[0].Top;

            Math.Abs(drawnGap - renderedGap).ShouldBeLessThanOrEqualTo(
                TolerancePoints,
                $"{fileName}: word {i + 1} (\"{rendered[i].Text}\") sits {drawnGap:F3} pt below the "
                + $"first word drawn, {renderedGap:F3} pt rendered");
        }
    }

    // ------------------------------------------------------------------------- the machinery

    /// <summary>
    /// The gap between two words, in points, beyond which they belong to different cells.
    /// </summary>
    /// <remarks>
    /// A word space at the sizes the corpus uses is under three points, and the narrowest gap between two
    /// cells in the document is the wide column's padding either side of its border — several times that.
    /// Six points separates the two comfortably without needing to know the table's geometry, which is the
    /// thing under test and so cannot be an input to the test's own bookkeeping.
    /// </remarks>
    private const double GapBetweenCells = 6;

    /// <summary>
    /// Whether a word is the first on its line, where a "line" is one cell's worth.
    /// </summary>
    /// <remarks>
    /// Not simply "the first word at this vertical position": several cells of one row share a baseline, so
    /// each row would otherwise contribute a single comparison and the columns after the first would go
    /// unchecked. A large horizontal gap from the previous word starts a line too — which is exactly what a
    /// cell boundary is, since the words within a cell are a space apart.
    /// </remarks>
    private static bool StartsALine(List<PdfWord> words, int index)
        => Math.Abs(words[index].Top - words[index - 1].Top) >= 0.5
           || words[index].Left - words[index - 1].Right > GapBetweenCells;

    /// <summary>
    /// The rendered words in reading order.
    /// </summary>
    /// <remarks>
    /// Sorted rather than taken as poppler gives them, because poppler reads a table column by column: it
    /// emits every word of the first column, then every word of the second. Paperless draws cell by cell in
    /// row order. Neither is wrong and neither is the other, so both are put in the same order — down the
    /// page, then across it — before they are compared.
    /// </remarks>
    private static List<PdfWord> InReadingOrder(List<PdfWord> words)
        => [.. words
            .OrderBy(word => word.PageIndex)
            .ThenBy(word => Math.Round(word.Top, 1))
            .ThenBy(word => word.Left)];

    private static List<DrawnWord> InDrawnOrder(List<DrawnWord> words)
        => [.. words
            .OrderBy(word => Math.Round(word.Baseline, 1))
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

        return [.. sink.Pages.SelectMany(DrawnWords.On)];
    }
}
