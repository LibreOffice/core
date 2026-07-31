using Paperless.Core.Documents;
using Paperless.Core.Graphics;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Paperless.WordProcessing;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Checks paragraphs whose formatting varies within them against LibreOffice, run by run.
/// </summary>
/// <remarks>
/// <para>
/// Against the PDF's own text operators rather than against <c>pdftotext</c>'s word boxes, because a run is
/// the unit both sides actually emit: LibreOffice writes one <c>BT … ET</c> block per text portion — per
/// line, split again at every formatting change — and that is exactly what a glyph run is. Comparing them
/// directly needs no grouping, and grouping is where a word-box comparison breaks down on a mixed line,
/// since poppler groups by vertical position and a 22 pt word on an 11 pt line is not at its neighbours'
/// height.
/// </para>
/// <para>
/// It also gives the vertical for free. A word box's top is above the baseline by an ascent the PDF never
/// states, so boxes can only be compared relatively; a <c>Td</c> is the pen, so the baseline can be
/// compared outright. That makes this the first test able to say that a line whose height came from a
/// larger run inside it sits where Writer put it.
/// </para>
/// <para>
/// What each assertion covers: the origin's x is the measurement of everything before the run on its line,
/// the origin's y is the line heights of everything above it, the size is the resolution of the run's own
/// formatting, and the glyph count is the shaping. A failure in one names its own cause.
/// </para>
/// </remarks>
public sealed class MixedRunComparisonTests : IDisposable
{
    /// <summary>How far a drawn position may differ from LibreOffice's, in points.</summary>
    /// <remarks>
    /// A tenth of a point — two twips, five times tighter than the word-box comparison can be, because
    /// both sides here are pen positions rather than ink. It cannot be tighter than two twips because of
    /// the ascent: VCL rounds a face's metrics through its reference device, and the result differs from
    /// scaling the design values by up to a twip. For 11 pt Carlito the design metrics give 209.47 twips
    /// and LibreOffice uses 210; at 9 pt they give 171.39 and it uses 171. A constant per-size offset like
    /// that shifts every baseline of one size equally, which this cannot see — but an error that
    /// <em>accumulates</em> down the page grows past two twips within a few lines, and every line on the
    /// page is compared, so that is what this catches.
    /// </remarks>
    private const double TolerancePoints = 0.1;

    /// <summary>
    /// The horizontal offset LibreOffice's PDF export adds to every pen position, in points.
    /// </summary>
    /// <remarks>
    /// Measured, not guessed: with a left margin of 1 cm, 2.5 cm and 5 cm, LibreOffice lays the body out
    /// at 567, 1417 and 2835 twips — its own RTF export states those — and its PDF places the first pen of
    /// each at 28.45, 70.95 and 141.85 pt, which is two twips more in every case. It is additive rather
    /// than a scale, since a scale would put the widest margin furthest out. Vertically there is no such
    /// offset, so it belongs to the export rather than to the layout.
    /// </remarks>
    private const double PdfPenOffsetPoints = 0.1;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-mixed").FullName;

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
    [InlineData("mixed-runs.fodt")]
    public void EveryRunIsDrawnWhereLibreOfficeDrawsIt(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        RecordingDrawingSink sink = Record(path);
        List<PdfTextRun> reference = PdfTextRuns.Read(_libreOffice.ConvertToPdf(path, _workDirectory));

        List<List<PdfTextRun>> expectedLines = IntoLines(reference, run => run.Y);
        List<List<DrawnGlyphRun>> actualLines =
            IntoLines(sink.Pages.SelectMany(page => page.Runs), run => run.Origin.Y.Points);

        // Lines first: a line too many or too few means the text broke somewhere else, and every position
        // after that would fail for the one reason.
        actualLines.Count.ShouldBe(
            expectedLines.Count,
            $"{fileName}: {actualLines.Count} lines drawn, "
            + $"{expectedLines.Count} in LibreOffice's own output");

        for (int line = 0; line < expectedLines.Count; line++)
        {
            List<PdfTextRun> expected = expectedLines[line];
            List<DrawnGlyphRun> actual = actualLines[line];

            // A line's trailing space is drawn by LibreOffice and not by Paperless — it has no ink and it
            // hangs past the margin — and LibreOffice puts it either at the end of the last run or, when
            // the space belongs to a different run from the text before it, in a run of its own. So the
            // reference may carry runs this does not, but only at the end of a line and only whitespace.
            actual.Count.ShouldBeLessThanOrEqualTo(
                expected.Count, $"{fileName}: line {line + 1} run count");

            foreach (PdfTextRun trailing in expected.Skip(actual.Count))
            {
                trailing.GlyphCount.ShouldBeLessThanOrEqualTo(
                    2,
                    $"{fileName}: line {line + 1} has an unmatched run of {trailing.GlyphCount} glyphs, "
                    + "which is too many to be the line's trailing space");
            }

            for (int i = 0; i < actual.Count; i++)
            {
                Compare(expected[i], actual[i], $"{fileName}: line {line + 1}, run {i + 1}", i == actual.Count - 1);
            }
        }
    }

    /// <summary>Compares one drawn run against the reference's.</summary>
    /// <param name="expected">The run as LibreOffice's PDF positions it.</param>
    /// <param name="actual">The run as Paperless drew it.</param>
    /// <param name="where">How to name this run in a failure.</param>
    /// <param name="endsLine">
    /// True for the last run on its line, where the reference may show one glyph more: the trailing space.
    /// </param>
    private static void Compare(
        PdfTextRun expected, DrawnGlyphRun actual, string where, bool endsLine)
    {
        string what = $"{where} (\"{Excerpt(actual.Text)}\")";

        // The size, which is the resolution of the run's own formatting — and the cheapest thing to get
        // wrong, since a percentage read as a number gives a plausible-looking result.
        actual.Run.FontSize.Points.ShouldBe(expected.FontSize, 0.01, $"{what}: em size");

        // Where the pen was. The x holds every advance before it on the line, so a wrong width anywhere
        // earlier lands here; the y holds every line height above it on the page.
        Close(actual.Origin.X.Points, expected.X - PdfPenOffsetPoints, $"{what}: x");
        Close(actual.Origin.Y.Points, expected.Y, $"{what}: y");

        // And how many glyphs it took to show it, which is the shaping: a ligature that formed on one side
        // and not the other is the same characters at a different width.
        int missing = expected.GlyphCount - actual.Run.Glyphs.Count;

        if (endsLine)
        {
            missing.ShouldBeInRange(0, 1, $"{what}: glyph count, allowing the line's trailing space");
        }
        else
        {
            missing.ShouldBe(0, $"{what}: glyph count");
        }
    }

    /// <summary>
    /// Groups a sequence of runs into lines by their baseline.
    /// </summary>
    /// <remarks>
    /// Each side is grouped against its own baselines rather than against the other's, because the two
    /// agree to within a twip and not exactly — so a shared threshold would put a run on one side's line
    /// and not the other's. Consecutive runs at the same baseline are one line, which holds because both
    /// sides emit a page's runs in reading order.
    /// </remarks>
    private static List<List<T>> IntoLines<T>(IEnumerable<T> runs, Func<T, double> baselineOf)
    {
        List<List<T>> lines = [];
        double baseline = double.NaN;

        foreach (T run in runs)
        {
            double at = baselineOf(run);

            if (lines.Count == 0 || Math.Abs(at - baseline) > 0.001)
            {
                lines.Add([]);
                baseline = at;
            }

            lines[^1].Add(run);
        }

        return lines;
    }

    [Theory]
    [InlineData("mixed-runs.fodt")]
    public void ALineTakesItsHeightFromItsTallestRun(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        List<PdfTextRun> reference = PdfTextRuns.Read(_libreOffice.ConvertToPdf(path, _workDirectory));

        // The document's point: a 22 pt run inside an 11 pt paragraph. The lines around it are further
        // apart than the paragraph's own pitch, and by how much is what the tallest-run rule decides.
        List<double> baselines = [.. reference.Select(run => run.Y).Distinct().Order()];
        baselines.Count.ShouldBeGreaterThan(15, $"{fileName}: not enough lines to compare");

        double large = reference.Single(run => Math.Abs(run.FontSize - 22) < 0.01).Y;
        int at = baselines.IndexOf(large);
        at.ShouldBeGreaterThan(0);

        double before = large - baselines[at - 1];
        double after = baselines[at + 1] - large;

        // Sanity on the reference itself, so that a document edited into uniformity does not leave this
        // test quietly asserting nothing: the tall line really is spaced differently from a plain one.
        before.ShouldBeGreaterThan(16.0, $"{fileName}: the tall line is not actually taller");

        // And Paperless puts the same two gaps in the same places, which the run-by-run comparison already
        // proves for every line — this states the specific consequence so a regression names itself.
        RecordingDrawingSink sink = Record(path);
        List<double> drawn = [.. sink.Pages
            .SelectMany(page => page.Runs)
            .Select(run => run.Origin.Y.Points)
            .Distinct()
            .Order()];

        int drawnAt = drawn.FindIndex(y => Math.Abs(y - large) <= TolerancePoints);
        drawnAt.ShouldBeGreaterThan(0, $"{fileName}: no drawn line at the tall line's baseline");

        Close(large - drawn[drawnAt - 1], before, $"{fileName}: the gap above the tall line");
        Close(drawn[drawnAt + 1] - large, after, $"{fileName}: the gap below the tall line");
    }

    [Theory]
    [InlineData("mixed-runs.fodt")]
    public void AMixedParagraphIsDrawnAsSeveralRuns(string fileName)
    {
        string path = Corpus.Require(fileName);
        DrawnPage page = Record(path).Pages[0];

        // A bold word inside a line makes three runs: the text before it, it, and the text after. One run
        // per line would mean the runs never reached the drawing path at all.
        page.Runs.Count(run => Math.Abs(run.Run.FontSize.Points - 22) < 0.01)
            .ShouldBe(1, "the 22 pt span draws as a run of its own");

        // The faces differ too, which a size comparison would miss: a bold run drawn in the regular face
        // has the right size and the wrong glyphs.
        page.Runs.Select(run => run.Run.Font.FaceKey).Distinct().Count()
            .ShouldBeGreaterThan(2, "regular, bold, italic and serif faces are all used");

        // And every run carries a paint, with the coloured span's colour among them.
        page.Runs.ShouldAllBe(run => run.Paint is SolidPaint);
        page.Runs
            .Select(run => ((SolidPaint)run.Paint).Colour)
            .ShouldContain(Colour.FromRgb(0xC9211E));
    }

    // ------------------------------------------------------------------------- the machinery

    private static void Close(double actual, double expected, string what)
        => Math.Abs(actual - expected).ShouldBeLessThanOrEqualTo(
            TolerancePoints, $"{what}: {actual:F3} pt drawn, {expected:F3} pt rendered");

    private static string Excerpt(string text)
        => text.Length <= 24 ? text : text[..24] + "…";

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
