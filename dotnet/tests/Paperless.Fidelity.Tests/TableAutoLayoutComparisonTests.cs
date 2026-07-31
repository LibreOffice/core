using Paperless.Core.Documents;
using Paperless.Core.Graphics;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Paperless.WordProcessing;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Checks a table whose columns state no widths against LibreOffice's own rendering of it.
/// </summary>
/// <remarks>
/// <para>
/// The interesting half of this test is what it turned out <em>not</em> to be. A width-less table looks
/// like a job for content-based automatic layout — measure each column's longest word and its unwrapped
/// text, then distribute — because that is what CSS does and what the numbers superficially suggest: the
/// corpus table comes out 160.6, 107.1 and 214.1 pt, which is neither equal nor the ratio of anything the
/// file says. It is not that. Move the one long paragraph from the third column to the second, or delete
/// it, and the columns do not shift by a twip. Writer owns exactly one content-measuring table layout,
/// <c>SwHTMLTableLayout</c>, and only the HTML filter ever installs it.
/// </para>
/// <para>
/// What decides the widths is arithmetic on the declared grid, and the two families do it differently
/// enough that the same table is a different shape in each — which is why the corpus carries the ODF and
/// the Word spelling of one document and expects two different answers:
/// </para>
/// <list type="bullet">
///   <item>
///     <b>ODF</b>, when the table itself states a width and an alignment, distributes each width-less
///     column as <c>MINLAY × remaining / totalRelative</c> while decrementing <c>remaining</c> and not
///     <c>totalRelative</c>, so three equal columns come out 3:2:4. When it states no alignment the width
///     is discarded and the columns come out equal.
///   </item>
///   <item>
///     <b>Word</b> states the grid as relative separators against a table that starts equal, and a
///     separator of zero is dropped rather than applied — so an unsized column's divider stays where the
///     equal division put it.
///   </item>
/// </list>
/// <para>
/// The comparison is against the PDF's own text operators rather than <c>pdftotext</c>'s word boxes,
/// because a pen position is directly comparable and a box carries the font's ascent. A wrong column width
/// moves a pen, so this measures the thing under test at a tenth of a point.
/// </para>
/// </remarks>
public sealed class TableAutoLayoutComparisonTests : IDisposable
{
    /// <summary>How far a drawn pen may differ from LibreOffice's, in points.</summary>
    private const double TolerancePoints = 0.1;

    /// <summary>The horizontal offset LibreOffice's PDF export adds to every pen position, in points.</summary>
    private const double PdfPenOffsetPoints = 0.1;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-autofit").FullName;

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
    // ODF, oriented: table:align="left" and style:width="17cm", every column width-less. The 3:2:4 case,
    // and the one the whole item was written around — 160.6, 107.1 and 214.1 pt.
    [InlineData("table-autofit.fodt")]
    // The same table with table:align removed, which makes it HoriOrientation::FULL — the stated width is
    // then ignored outright and the columns come out equal. Two documents differing by one attribute and
    // laying out 54 pt apart at the third column, which is why the attribute is read at all.
    [InlineData("table-autofit-full.fodt")]
    // Two columns stating a width and one not, which is the branch that restates the absolute columns
    // against the narrowest of them before distributing.
    [InlineData("table-autofit-mixed.fodt")]
    // The Word family's spelling of the same table: a grid of zeroes and w:tblW auto. It has no spanning
    // row where the ODF documents do, and the reason is a real divergence rather than tidiness — see
    // TheColumnsComeOutAtTheWidthsLibreOfficeGivesThem, and the TODO entry it points at.
    [InlineData("table-autofit.docx")]
    // RTF says it with \cellx0, which reaches Writer through the same dmapper the DOCX does — and does
    // keep its spanning row, because RTF states a short row as a row with fewer \cellx edges and the
    // fallback counts cells, which lands the last row's second cell in the same column Writer puts it in.
    [InlineData("table-autofit.rtf")]
    public void EveryCellStartsWhereLibreOfficeStartsIt(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        RecordingDrawingSink sink = Record(path);
        List<PdfTextRun> reference = PdfTextRuns.Read(_libreOffice.ConvertToPdf(path, _workDirectory));

        reference.Count.ShouldBeGreaterThan(0, $"{fileName}: LibreOffice drew nothing");

        List<List<PdfTextRun>> expectedLines = IntoLines(reference, run => run.Y, run => run.X);
        List<List<DrawnGlyphRun>> actualLines = IntoLines(
            sink.Pages.SelectMany(page => page.Runs),
            run => run.Origin.Y.Points,
            run => run.Origin.X.Points);

        // The line count first: a column too narrow wraps a cell that should not wrap, which adds a line,
        // and every position after it would then fail for the one reason.
        actualLines.Count.ShouldBe(
            expectedLines.Count,
            $"{fileName}: {actualLines.Count} lines drawn, "
            + $"{expectedLines.Count} in LibreOffice's own output");

        for (int line = 0; line < expectedLines.Count; line++)
        {
            List<PdfTextRun> expected = expectedLines[line];
            List<DrawnGlyphRun> actual = actualLines[line];

            actual.Count.ShouldBe(
                expected.Count, $"{fileName}: line {line + 1} run count");

            for (int i = 0; i < actual.Count; i++)
            {
                actual[i].Origin.X.Points.ShouldBe(
                    expected[i].X - PdfPenOffsetPoints,
                    TolerancePoints,
                    $"{fileName}: line {line + 1}, run {i + 1} pen x");
            }
        }
    }

    /// <summary>
    /// The measurements themselves, written down.
    /// </summary>
    /// <remarks>
    /// The run comparison above would pass on a table whose columns were all wrong in the same way as the
    /// reference's, which cannot happen — but it says nothing about <em>what</em> the widths are, and the
    /// widths are the finding. These are LibreOffice's own, read off its rendering of each file, so a
    /// change to the distribution has to argue with a number rather than with a diff.
    /// </remarks>
    [Theory]
    // 3:2:4 out of the 17 cm the table declares. Not a ratio the file states anywhere and not equal
    // thirds: the importer divides by the total relative width while decrementing the remainder.
    [InlineData("table-autofit.fodt", 160.6, 107.1, 214.1, TolerancePoints)]
    // The same file without table:align, where the declared width is discarded and the text area's
    // 481.9 pt is divided equally.
    [InlineData("table-autofit-full.fodt", 160.6, 160.6, 160.6, TolerancePoints)]
    // 3 cm and 5 cm stated, the middle column taking what is left of the 17 cm.
    [InlineData("table-autofit-mixed.fodt", 85.05, 255.15, 141.75, TolerancePoints)]
    [InlineData("table-autofit.docx", 160.6, 160.6, 160.6, TolerancePoints)]
    [InlineData("table-autofit.rtf", 160.6, 160.6, 160.6, TolerancePoints)]
    // The separator rule, and the one case that needs a looser bound. The first divider does not move,
    // because its separator is zero, so the first column keeps the equal division's 160.6 and the second
    // is squeezed into what is left before the second divider — which does move. The third comes out three
    // twips wide of LibreOffice's: Writer moves a divider through the table's stored "wish" width
    // (SwTable::NewSetTabCols's lcl_MulDiv64) rather than by scaling the separator against the table's
    // width directly, and the two round apart. Three twips, on a column squeezed to eleven points.
    [InlineData("table-autofit-partial.docx", 160.6, 11.5, 309.75, 0.2)]
    public void TheColumnsComeOutAtTheWidthsLibreOfficeGivesThem(
        string fileName, double first, double second, double third, double tolerance)
    {
        // The columns as laid out, taken from the first row's cells rather than from the table's own grid:
        // a cell's rectangle is what the text actually breaks in, and the grid is only how it got there.
        List<double> widths = ColumnWidths(Corpus.Require(fileName));

        widths.Count.ShouldBe(3, $"{fileName}: column count");
        widths[0].ShouldBe(first, tolerance, $"{fileName}: first column");
        widths[1].ShouldBe(second, tolerance, $"{fileName}: second column");
        widths[2].ShouldBe(third, tolerance, $"{fileName}: third column");
    }

    /// <summary>The first row's cell widths, in points.</summary>
    private static List<double> ColumnWidths(string path)
    {
        using FileStream stream = File.OpenRead(path);
        using DocumentSource source = DocumentSource.FromStream(stream, Path.GetFileName(path));
        using IDocument document = new WordProcessingReader().Read(source);

        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        PlacedTable table = pages.Pages
            .SelectMany(page => page.Tables)
            .First();

        return
        [
            .. table.Cells
                .Where(cell => cell.Row == 0)
                .OrderBy(cell => cell.Area.X.Points)
                .Select(cell => cell.Area.Width.Points),
        ];
    }

    /// <summary>
    /// Groups runs into lines by their baseline, left to right within each.
    /// </summary>
    /// <remarks>
    /// Sorted rather than taken in drawn order, which the run comparisons outside tables can afford to do
    /// and this cannot: both sides draw a table <em>cell by cell</em>, so the three cells of one row are
    /// three separate visits to the same baseline with a wrapped cell's second line in between. Sorting by
    /// baseline and then by pen puts a row back together on both sides, whatever order the cells were
    /// walked in.
    /// </remarks>
    private static List<List<T>> IntoLines<T>(
        IEnumerable<T> runs, Func<T, double> baselineOf, Func<T, double> penOf)
    {
        List<List<T>> lines = [];
        double baseline = double.NaN;

        foreach (T run in runs.OrderBy(baselineOf).ThenBy(penOf))
        {
            double at = baselineOf(run);

            // A third of a point: closer together than any two lines of this corpus and wider than the
            // difference between the two sides' arithmetic for one line.
            if (lines.Count == 0 || at - baseline > 0.3)
            {
                lines.Add([]);
                baseline = at;
            }

            lines[^1].Add(run);
        }

        return lines;
    }

    private static RecordingDrawingSink Record(string path)
    {
        RecordingDrawingSink sink = new();

        using FileStream stream = File.OpenRead(path);
        using DocumentSource source = DocumentSource.FromStream(stream, Path.GetFileName(path));
        using IDocument document = new WordProcessingReader().Read(source);

        IPageSequence pages = ((IPaginatedDocument)document).Layout();
        for (int i = 0; i < pages.Count; i++) pages[i].Draw(sink);

        return sink;
    }
}
