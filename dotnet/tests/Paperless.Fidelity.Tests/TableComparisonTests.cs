using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
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
    [InlineData("table-grid.rtf")]
    [InlineData("table-grid.doc")]
    [InlineData("table-pages.fodt")]
    [InlineData("table-pages.odt")]
    [InlineData("table-pages.docx")]
    [InlineData("table-pages.doc")]
    [InlineData("table-pages.rtf")]
    // A row whose height is *exact* rather than a floor, and whose content is taller than it — so the row does
    // not grow and the rows below it move up by the difference. Measured: 22.70 pt where the same row grows to
    // 32.60 when the height is read as a minimum, which moves every row after it.
    [InlineData("table-exact-row.fodt")]
    [InlineData("table-exact-row.odt")]
    [InlineData("table-exact-row.docx")]
    [InlineData("table-exact-row.doc")]
    [InlineData("table-exact-row.rtf")]
    // `table-borders` is deliberately absent, and only in the shading test: a border *takes space*, half its
    // width on each side of the grid line, so a table with 0.05 pt borders is 0.1 pt taller per row boundary
    // than the same table without them. Border widths are not read yet, so its text sits exactly that much
    // high — right at the tolerance, and a real difference rather than noise.
    // A nested table in every format, which is worth stating because no two of them express it the same
    // way: ODF and DOCX nest the markup, while DOC and RTF nest the *paragraph* — sprmPItap and \itap give
    // a depth, and an inner table's cells end at paragraph marks rather than at U+0007.
    [InlineData("table-nested.fodt")]
    [InlineData("table-nested.odt")]
    [InlineData("table-nested.docx")]
    [InlineData("table-nested.rtf")]
    [InlineData("table-nested.doc")]
    public void EveryCellHoldsItsTextWhereLibreOfficeDoes(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        List<List<DrawnWord>> drawnPages = Drawn(path);
        List<PdfWord> everything = PdfWords.Read(_libreOffice.ConvertToPdf(path, _workDirectory));

        Assert.SkipWhen(
            everything.Count == 0,
            "pdftotext is not available; install poppler-utils — see check-env.sh");

        int pages = everything.Select(word => word.PageIndex).Distinct().Count();
        drawnPages.Count.ShouldBe(pages, $"{fileName}: page count");

        int compared = 0;
        for (int page = 0; page < pages; page++)
        {
            compared += ComparePage(
                fileName,
                page,
                InDrawnOrder(drawnPages[page]),
                InReadingOrder([.. everything.Where(word => word.PageIndex == page)]));
        }

        compared.ShouldBeGreaterThan(10, $"{fileName}: too few cells compared to prove anything");
    }

    /// <summary>
    /// Compares one page's words, returning how many line starts were checked.
    /// </summary>
    /// <remarks>
    /// Page by page because a table that crosses a break is the case worth testing hardest: its
    /// continuation starts at whichever row did not fit, and its heading rows are placed again above that.
    /// Comparing the document's words as one stream would let a row placed on the wrong page pass, since
    /// the order would still be right.
    /// </remarks>
    private static int ComparePage(
        string fileName, int page, List<DrawnWord> drawn, List<PdfWord> rendered)
    {
        string where = $"{fileName}: page {page + 1}";

        // Text first: a table whose cells were laid out in the wrong order, or whose covered cell was
        // taken for a real one, fails here rather than on a position that would take longer to read.
        string.Join(' ', drawn.Select(word => word.Text))
            .ShouldBe(
                string.Join(' ', rendered.Select(word => word.Text)),
                $"{where}: the drawn text differs from the rendered text");

        int lines = 0;
        for (int i = 0; i < rendered.Count; i++)
        {
            // The first word of each line only, as in the furniture comparison: a word further along
            // carries the reference's own per-glyph rounding, while the first word of a line starts where
            // the geometry says. Inside a table that is also the interesting position — it is the one the
            // column grid and the cell padding decide.
            if (i > 0 && !StartsALine(rendered, i)) continue;

            string word = $"{where}, word {i + 1} (\"{rendered[i].Text}\")";

            Math.Abs(drawn[i].Left - (rendered[i].Left - PdfPenOffsetPoints))
                .ShouldBeLessThanOrEqualTo(
                    TolerancePoints,
                    $"{word}: starts at {drawn[i].Left:F3} pt drawn, "
                    + $"{rendered[i].Left - PdfPenOffsetPoints:F3} pt rendered");

            lines++;
        }

        // Vertically as differences from the first word, which cancels the ascent that separates a box top
        // from a baseline. This is what the row heights are checked by: every word below the first row is
        // where it is because of how tall the rows above it turned out.
        for (int i = 1; i < rendered.Count; i++)
        {
            double drawnGap = drawn[i].Baseline - drawn[0].Baseline;
            double renderedGap = rendered[i].Top - rendered[0].Top;

            Math.Abs(drawnGap - renderedGap).ShouldBeLessThanOrEqualTo(
                TolerancePoints,
                $"{where}, word {i + 1} (\"{rendered[i].Text}\") sits {drawnGap:F3} pt below the "
                + $"first word drawn, {renderedGap:F3} pt rendered");
        }

        return lines;
    }

    [Theory]
    [InlineData("table-shading.fodt")]
    [InlineData("table-shading.odt")]
    [InlineData("table-shading.docx")]
    [InlineData("table-shading.rtf")]
    // DOC is absent from *this* test only: its cell shading is a per-band `WW8_SHD` array from
    // `sprmTDefTableShd`, indexed by cell, with a newer three-sprm form carrying full RGB — a bigger read than
    // the other three and still open. Its text is compared above like every other format's.
    public void AShadedCellIsFilledWhereLibreOfficeFillsIt(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        // Distinct rectangles, because a reference can paint one twice: LibreOffice's own DOCX render fills
        // each shaded cell in this document exactly twice, at identical coordinates, where its ODF render fills
        // it once. Counting fills would make the same document pass in one format and fail in the other for a
        // reason that has nothing to do with where anything went.
        List<PdfFill> reference =
            [.. PdfFills.Read(_libreOffice.ConvertToPdf(path, _workDirectory))
                .Where(fill => fill.PageIndex == 0 && fill.Height > 1 && fill.Width < 500)
                .DistinctBy(fill => (Math.Round(fill.Left, 1), Math.Round(fill.Top, 1)))
                .OrderBy(fill => fill.Top).ThenBy(fill => fill.Left)];

        Assert.SkipWhen(reference.Count == 0, "the reference PDF filled no cell-sized paths");

        List<DocRect> mine =
            [.. Rendered(path)[0].FilledPaths
                .Select(fill => fill.Bounds)
                .DistinctBy(bounds => (Math.Round(bounds.X.Points, 1), Math.Round(bounds.Y.Points, 1)))
                .OrderBy(bounds => bounds.Y.Points).ThenBy(bounds => bounds.X.Points)];

        mine.Count.ShouldBe(
            reference.Count, $"{fileName}: page 1 filled a different number of distinct cell shades");

        for (int i = 0; i < reference.Count; i++)
        {
            DocRect drawn = mine[i];

            Math.Abs(drawn.X.Points - reference[i].Left).ShouldBeLessThanOrEqualTo(
                TolerancePoints,
                $"{fileName}: shade {i + 1} starts at {drawn.X.Points:F3} pt drawn, "
                + $"{reference[i].Left:F3} pt rendered");

            Math.Abs(drawn.Width.Points - reference[i].Width).ShouldBeLessThanOrEqualTo(
                TolerancePoints,
                $"{fileName}: shade {i + 1} is {drawn.Width.Points:F3} pt wide drawn, "
                + $"{reference[i].Width:F3} pt rendered");

            // Vertically as a difference from the first shade, which cancels the one thing this cannot know:
            // where the table's top edge sits relative to the border LibreOffice draws over it.
            double drawnGap = drawn.Y.Points - mine[0].Y.Points;
            double renderedGap = reference[i].Top - reference[0].Top;

            Math.Abs(drawnGap - renderedGap).ShouldBeLessThanOrEqualTo(
                TolerancePoints,
                $"{fileName}: shade {i + 1} sits {drawnGap:F3} pt below the first drawn, "
                + $"{renderedGap:F3} pt rendered");
        }
    }

    [Theory]
    [InlineData("table-borders.fodt")]
    public void ABorderIsStrokedAsOneLinePerGridLine(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        List<PdfStroke> reference =
            [.. PdfStrokes.Read(_libreOffice.ConvertToPdf(path, _workDirectory))
                .Where(stroke => stroke.PageIndex == 0)];

        Assert.SkipWhen(reference.Count == 0, "the reference PDF stroked nothing");

        List<DrawnStroke> lines = [.. Rendered(path)[0].StrokedPaths];
        List<Stroke> mine = [.. Rendered(path)[0].Strokes];

        // The count is the assertion that matters most, and it is why this test exists: LibreOffice writes one
        // stroke per grid line across the whole table — five horizontals for a four-row table and one vertical
        // per column boundary — and a reader that drew four borders round each cell would produce three times
        // as many strokes in the right places. Being right on the page is not the same as agreeing.
        mine.Count.ShouldBe(
            reference.Count,
            $"{fileName}: page 1 stroked {mine.Count} lines, LibreOffice {reference.Count}");

        // Then where each ran. Compared numerically rather than as formatted text, because several of these
        // land within a rounding of a tenth of a point — a border's position is a grid line plus half a pen
        // width, and 70.45 rounds two ways. Paired by sorting both sides the same way, which is sound because
        // the counts already agree.
        List<Line> drawnLines = [.. lines.Select(Line.Of).OrderBy(line => line.Key)];
        List<Line> renderedLines = [.. reference.Select(Line.Of).OrderBy(line => line.Key)];

        for (int i = 0; i < renderedLines.Count; i++)
        {
            Line drawn = drawnLines[i];
            Line rendered = renderedLines[i];

            drawn.IsHorizontal.ShouldBe(
                rendered.IsHorizontal, $"{fileName}: stroke {i + 1} runs along the other axis");

            Math.Abs(drawn.At - rendered.At).ShouldBeLessThanOrEqualTo(
                TolerancePoints,
                $"{fileName}: stroke {i + 1} sits at {drawn.At:F3} pt drawn, {rendered.At:F3} pt rendered");

            Math.Abs(drawn.From - rendered.From).ShouldBeLessThanOrEqualTo(
                TolerancePoints,
                $"{fileName}: stroke {i + 1} starts at {drawn.From:F3} pt drawn, "
                + $"{rendered.From:F3} pt rendered");

            Math.Abs(drawn.To - rendered.To).ShouldBeLessThanOrEqualTo(
                TolerancePoints,
                $"{fileName}: stroke {i + 1} ends at {drawn.To:F3} pt drawn, {rendered.To:F3} pt rendered");

            Math.Abs(drawn.Width - rendered.Width).ShouldBeLessThanOrEqualTo(
                TolerancePoints,
                $"{fileName}: stroke {i + 1} is {drawn.Width:F3} pt thick drawn, "
                + $"{rendered.Width:F3} pt rendered");
        }
    }

    /// <summary>One stroked grid line, from either side, as the four numbers worth comparing.</summary>
    /// <param name="IsHorizontal">True when it runs across the page.</param>
    /// <param name="At">Where it sits on the other axis.</param>
    /// <param name="From">Where it starts along its own axis.</param>
    /// <param name="To">Where it ends.</param>
    /// <param name="Width">The pen width.</param>
    private readonly record struct Line(
        bool IsHorizontal, double At, double From, double To, double Width)
    {
        /// <summary>A sort key that pairs the two sides' strokes: axis first, then position, then start.</summary>
        public (bool, double, double) Key => (IsHorizontal, Math.Round(At, 1), Math.Round(From, 1));

        public static Line Of(DrawnStroke stroke)
        {
            DocRect bounds = stroke.Bounds;

            return bounds.Height.Points < 0.05
                ? new Line(true, bounds.Y.Points, bounds.X.Points, bounds.Right.Points,
                    stroke.Stroke.Width.Points)
                : new Line(false, bounds.X.Points, bounds.Y.Points, bounds.Bottom.Points,
                    stroke.Stroke.Width.Points);
        }

        public static Line Of(PdfStroke stroke)
            => stroke.IsHorizontal
                ? new Line(
                    true, stroke.FromY, Math.Min(stroke.FromX, stroke.ToX),
                    Math.Max(stroke.FromX, stroke.ToX), stroke.Width)
                : new Line(
                    false, stroke.FromX, Math.Min(stroke.FromY, stroke.ToY),
                    Math.Max(stroke.FromY, stroke.ToY), stroke.Width);
    }

    // ------------------------------------------------------------------------- the machinery

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
