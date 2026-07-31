using Paperless.Core.Documents;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Paperless.WordProcessing;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Checks text flowing round a floating frame against LibreOffice's own rendering of the same document.
/// </summary>
/// <remarks>
/// <para>
/// The measurement that matters is the <em>start</em> of each line, because that is the only thing a wrap
/// changes: a line beside a frame at the start margin begins where the frame ends, and the line above it
/// begins at the margin. So the assertion is one number per line — where its leftmost pen went — and a
/// failure names itself, since a frame read at the wrong width moves every wrapped line by the same
/// amount while a frame read at the wrong height moves the boundary between the wrapped lines and the
/// rest.
/// </para>
/// <para>
/// Against the PDF's own text operators rather than <c>pdftotext</c>'s word boxes, for the reason
/// <see cref="MixedRunComparisonTests"/> gives: a <c>Td</c> is the pen, and a pen is what a line's start
/// is. It also lets the frame's own text be told apart from the body's — poppler would merge the two
/// wherever they share a baseline, which beside a frame is most of them.
/// </para>
/// <para>
/// The corpus document is deliberately the smallest thing that shows the behaviour: one frame, one wrap
/// mode, four centimetres wide and three tall at the start margin of the second paragraph, on an A4 page
/// with 2 cm margins. A cascade in a document with several frames is far harder to attribute.
/// </para>
/// </remarks>
public sealed class FrameComparisonTests : IDisposable
{
    /// <summary>How far a drawn pen may differ from LibreOffice's, in points.</summary>
    private const double TolerancePoints = 0.1;

    /// <summary>The horizontal offset LibreOffice's PDF export adds to every pen position, in points.</summary>
    private const double PdfPenOffsetPoints = 0.1;

    /// <summary>The em size the corpus document's body is set at.</summary>
    private const double BodyPoints = 11;

    /// <summary>
    /// The em size the frame's own text is set at.
    /// </summary>
    /// <remarks>
    /// Deliberately different from the body's, so that the two can be told apart by something other than
    /// where they are — the position being what is under test. Nine point rather than any other because
    /// it is also a size whose ascent LibreOffice rounds <em>down</em> through its reference device
    /// (171 twips against the design metrics' 171.39), so a frame line placed at the wrong ascent shows
    /// up rather than cancelling out.
    /// </remarks>
    private const double FramePoints = 9;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-frames").FullName;

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
    [InlineData("frame-wrap.fodt")]
    [InlineData("frame-wrap.odt")]
    public void EveryLineStartsWhereLibreOfficeStartsIt(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        List<Line> expected = Reference(path);
        List<Line> actual = Drawn(path);

        // The line count first: a wrap that reserved the wrong height gives the page a different number of
        // lines, and every position after the difference would then fail for that one reason.
        actual.Count.ShouldBe(
            expected.Count,
            $"{fileName}: {actual.Count} lines drawn against {expected.Count} in LibreOffice's own output");

        for (int i = 0; i < expected.Count; i++)
        {
            actual[i].Page.ShouldBe(expected[i].Page, $"{fileName}: line {i + 1} page");

            Close(
                actual[i].Left,
                expected[i].Left - PdfPenOffsetPoints,
                $"{fileName}: line {i + 1} starts at");

            Close(actual[i].Baseline, expected[i].Baseline, $"{fileName}: line {i + 1} baseline");
        }
    }

    /// <summary>
    /// States the wrap outright: some lines start at the frame's far edge and the rest at the margin.
    /// </summary>
    /// <remarks>
    /// A guard on the comparison above rather than a second measurement of the same thing. That test
    /// compares Paperless against LibreOffice and would pass just as happily if a future change made both
    /// sides draw an unwrapped document — if, say, the frame stopped being read at all and the reference
    /// were regenerated from a document that had lost it. This says what the document is <em>for</em>: the
    /// page holds lines at two distinct starts, 4 cm apart, and the indented ones are a contiguous run.
    /// </remarks>
    [Theory]
    [InlineData("frame-wrap.fodt")]
    [InlineData("frame-wrap.odt")]
    public void TheWrappedLinesAreIndentedByTheFramesWidth(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        List<Line> lines = Drawn(Corpus.Require(fileName));
        lines.Count.ShouldBeGreaterThan(10, $"{fileName}: too few lines to say anything");

        double margin = lines.Min(line => line.Left);
        List<int> wrapped = [.. lines
            .Index()
            .Where(pair => pair.Item.Left > margin + 1)
            .Select(pair => pair.Index)];

        wrapped.Count.ShouldBeInRange(6, 9, $"{fileName}: how many lines the frame narrowed");

        // Contiguous, because a frame occupies one band of the page: a gap would mean a line inside the
        // band was left at the margin, which is a hole in the wrap rather than a different wrap.
        wrapped[^1].ShouldBe(
            wrapped[0] + wrapped.Count - 1, $"{fileName}: the narrowed lines are not consecutive");

        // 4 cm — the frame's own width — since it sits at the start margin with no spacing round it.
        double indent = lines[wrapped[0]].Left - margin;
        indent.ShouldBe(113.39, 0.5, $"{fileName}: the wrapped lines' indent");
    }

    /// <summary>The frame's own text is drawn, inside the rectangle the frame was given.</summary>
    /// <remarks>
    /// The half of a frame that the wrap cannot check: a reader that got the geometry right and never read
    /// the <c>draw:text-box</c> would narrow every line correctly and leave the frame empty. Compared
    /// against LibreOffice's own placement of it, which is what makes this an assertion about position
    /// rather than about presence.
    /// </remarks>
    [Theory]
    [InlineData("frame-wrap.fodt")]
    [InlineData("frame-wrap.odt")]
    public void TheFramesOwnTextIsDrawnInsideIt(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        RecordingDrawingSink sink = Record(path);

        LaidOutPage page = Pages(path).Pages[0];
        page.Frames.Count.ShouldBe(1, $"{fileName}: frames on the first page");

        PlacedFrame frame = page.Frames[0];
        frame.Content.ShouldNotBeNull($"{fileName}: the frame's own content");
        frame.Content!.Lines.Count.ShouldBe(2, $"{fileName}: lines inside the frame");

        // Inside the rectangle, which is the check that the content was laid out at the frame's width
        // rather than the page's — a frame whose text broke at the body width would run out of it.
        foreach (var line in frame.Content.Lines)
        {
            line.Box.Left.ShouldBeGreaterThanOrEqualTo(
                Core.Units.Length.Zero, $"{fileName}: a frame line starts left of the frame");
        }

        // And LibreOffice draws it at the same two baselines. The frame's text is the only 12 pt text in
        // the document, which is what separates it from the body without knowing where the frame is.
        List<double> reference = [.. PdfTextRuns
            .Read(_libreOffice.ConvertToPdf(path, _workDirectory))
            .Where(run => Math.Abs(run.FontSize - FramePoints) < 0.01)
            .Select(run => run.Y)
            .Distinct()
            .Order()];

        List<double> drawn = [.. sink.Pages
            .SelectMany(drawnPage => drawnPage.Runs)
            .Where(run => Math.Abs(run.Run.FontSize.Points - FramePoints) < 0.01)
            .Select(run => run.Origin.Y.Points)
            .Distinct()
            .Order()];

        drawn.Count.ShouldBe(reference.Count, $"{fileName}: baselines inside the frame");

        for (int i = 0; i < reference.Count; i++)
        {
            Close(drawn[i], reference[i], $"{fileName}: frame line {i + 1} baseline");
        }
    }

    /// <summary>One drawn line: which page it is on, where it starts, and where its baseline sits.</summary>
    private readonly record struct Line(int Page, double Left, double Baseline);

    /// <summary>
    /// The body's lines as LibreOffice drew them.
    /// </summary>
    /// <remarks>
    /// The frame's own text is excluded by its size, which the corpus document arranges deliberately: its
    /// body is 11 pt and the frame's own text 9 pt. Excluding it by position instead would
    /// mean the test knowing where the frame is, which is the thing under test.
    /// </remarks>
    private List<Line> Reference(string path)
        => Group(
            PdfTextRuns
                .Read(_libreOffice.ConvertToPdf(path, _workDirectory))
                .Where(run => Math.Abs(run.FontSize - BodyPoints) < 0.01)
                .Select(run => new Line(run.PageIndex, run.X, run.Y)));

    /// <summary>The body's lines as Paperless drew them.</summary>
    private static List<Line> Drawn(string path)
    {
        RecordingDrawingSink sink = Record(path);
        List<Line> lines = [];

        for (int page = 0; page < sink.Pages.Count; page++)
        {
            foreach (DrawnGlyphRun run in sink.Pages[page].Runs)
            {
                if (Math.Abs(run.Run.FontSize.Points - BodyPoints) > 0.01) continue;

                lines.Add(new Line(page, run.Origin.X.Points, run.Origin.Y.Points));
            }
        }

        return Group(lines);
    }

    /// <summary>
    /// One entry per line: its leftmost pen.
    /// </summary>
    /// <remarks>
    /// Grouped on the page and the baseline together, since two pages have lines at the same height, and
    /// ordered by both so that the two sides' lists correspond. The <em>leftmost</em> pen because a line
    /// can be several runs and only its first says where the line begins.
    /// </remarks>
    private static List<Line> Group(IEnumerable<Line> runs)
        => [.. runs
            .GroupBy(run => (run.Page, Baseline: Math.Round(run.Baseline, 2)))
            .Select(group => new Line(
                group.Key.Page, group.Min(run => run.Left), group.Key.Baseline))
            .OrderBy(line => line.Page)
            .ThenBy(line => line.Baseline)];

    private static WordProcessingPages Pages(string path)
    {
        using IDocument document = Open(path);
        return (WordProcessingPages)((IPaginatedDocument)document).Layout();
    }

    private static RecordingDrawingSink Record(string path)
    {
        using IDocument document = Open(path);
        IPageSequence pages = ((IPaginatedDocument)document).Layout();

        RecordingDrawingSink sink = new();
        for (int i = 0; i < pages.Count; i++) pages[i].Draw(sink);

        return sink;
    }

    private static IDocument Open(string path)
    {
        using FileStream stream = File.OpenRead(path);
        using DocumentSource source = DocumentSource.FromStream(stream, Path.GetFileName(path));
        return new WordProcessingReader().Read(source);
    }

    private static void Close(double actual, double expected, string what)
        => actual.ShouldBe(
            expected,
            TolerancePoints,
            $"{what}: {actual:0.00} pt against LibreOffice's {expected:0.00} pt");
}
