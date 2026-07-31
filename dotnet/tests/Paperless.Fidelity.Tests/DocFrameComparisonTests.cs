using Paperless.Core.Documents;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Paperless.WordProcessing;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// The same measurements <see cref="FrameComparisonTests"/> makes, on the one format whose frames
/// arrive through the Escher drawing layer rather than through markup.
/// </summary>
/// <remarks>
/// <para>
/// A file of its own rather than four more lines in the theory next door, because the DOC case needs
/// two things the other three do not and both are properties of the file rather than of the test.
/// It is the only format whose corpus document has a <em>border</em> — LibreOffice's DOC export gives
/// the text box a 0.75 pt outline that the ODF source asked it not to have — and that border is what
/// the wrap has to allow for, since Word states a shape's rectangle as the path its outline runs
/// along while Writer keeps text clear of the bounding rectangle the stroke straddles.
/// </para>
/// <para>
/// The numbers, all measured from LibreOffice's own PDF of <c>frame-wrap.doc</c> and worth keeping
/// because they are what make the difference legible: the shape's right edge is drawn at 170.05 pt,
/// which is exactly the 3401 twips its <c>FSPA</c> states; the body text resumes at 179.55 pt, which
/// is 3591. The gap of 190 twips is the 181 the drawing layer's default wrap distance supplies, plus
/// half of the 15-twip line, plus the 2 twips the same document's ODF form also shows.
/// </para>
/// </remarks>
public sealed class DocFrameComparisonTests : IDisposable
{
    /// <summary>How far a drawn pen may differ from LibreOffice's, in points.</summary>
    private const double TolerancePoints = 0.1;

    /// <summary>The horizontal offset LibreOffice's PDF export adds to every pen position, in points.</summary>
    private const double PdfPenOffsetPoints = 0.1;

    /// <summary>The em size the corpus document's body is set at.</summary>
    private const double BodyPoints = 11;

    /// <summary>The em size the frame's own text is set at, which is what separates the two.</summary>
    private const double FramePoints = 9;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-doc-frames").FullName;

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

    /// <summary>Every body line starts and sits where LibreOffice's own rendering puts it.</summary>
    [Fact]
    public void EveryLineStartsWhereLibreOfficeStartsIt()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require("frame-wrap.doc");
        List<Line> expected = Reference(path);
        List<Line> actual = Drawn(path);

        actual.Count.ShouldBe(
            expected.Count,
            $"{actual.Count} lines drawn against {expected.Count} in LibreOffice's own output");

        for (int i = 0; i < expected.Count; i++)
        {
            actual[i].Page.ShouldBe(expected[i].Page, $"line {i + 1} page");
            Close(actual[i].Left, expected[i].Left - PdfPenOffsetPoints, $"line {i + 1} starts at");
            Close(actual[i].Baseline, expected[i].Baseline, $"line {i + 1} baseline");
        }
    }

    /// <summary>
    /// The wrap is really there: a contiguous run of lines indented by the frame's width.
    /// </summary>
    /// <remarks>
    /// The guard the comparison above needs, for the reason its sibling states — that test would pass
    /// just as happily if a change made both sides draw an unwrapped document. The DOC's indent is
    /// larger than the ODF's by exactly the wrap distance and the half-line, which is why the range
    /// is stated separately rather than shared with <see cref="FrameComparisonTests"/>.
    /// </remarks>
    [Fact]
    public void TheWrappedLinesAreIndentedByTheFramesWidthAndItsWrapDistance()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        List<Line> lines = Drawn(Corpus.Require("frame-wrap.doc"));
        lines.Count.ShouldBeGreaterThan(10, "too few lines to say anything");

        double margin = lines.Min(line => line.Left);
        List<int> wrapped = [.. lines
            .Index()
            .Where(pair => pair.Item.Left > margin + 1)
            .Select(pair => pair.Index)];

        wrapped.Count.ShouldBeInRange(6, 9, "how many lines the frame narrowed");
        wrapped[^1].ShouldBe(wrapped[0] + wrapped.Count - 1, "the narrowed lines are not consecutive");

        // 4 cm of frame — 113.35 pt — plus 190 twips of wrap distance and half-line, which is 9.5 pt.
        (lines[wrapped[0]].Left - margin).ShouldBeInRange(122.0, 123.5, "the wrapped lines' indent");
    }

    /// <summary>
    /// The shape is read as a text box with a border and no fill, and its own text is laid out inside
    /// it where LibreOffice draws it.
    /// </summary>
    /// <remarks>
    /// The half of a DOC frame that the wrap cannot check, and the half that needs the most of the
    /// record streams: the rectangle comes from the <c>FSPA</c>, the border colour and width from the
    /// shape's <c>OPT</c> property table, and the text from the story its <c>lTxid</c> indexes — three
    /// structures in two streams for one frame. The fill is asserted absent because it is the one that
    /// is stated as a <em>boolean</em>: <c>fFilled</c> is bit 4 of property 447, and a reader that asks
    /// for property 443 finds nothing and fills the frame white over the text behind it.
    /// </remarks>
    [Fact]
    public void TheFramesOwnTextIsDrawnInsideIt()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require("frame-wrap.doc");
        RecordingDrawingSink sink = Record(path);

        LaidOutPage page = Pages(path).Pages[0];
        page.Frames.Count.ShouldBe(1, "frames on the first page");

        PlacedFrame frame = page.Frames[0];
        frame.Frame.Wrap.ShouldBe(TextWrap.Right, "the wrap FSPA's nwr and nwrk ask for");
        frame.Frame.Fill.ShouldBeNull("the shape states fFilled false");
        frame.Frame.BorderColour.ShouldBe(
            new Core.Graphics.Colour(0x34, 0x65, 0xA4),
            "the line colour, whose MSO_CLR channel order is the reverse of every XML format's");

        frame.Content.ShouldNotBeNull("the frame's own content");
        frame.Content!.Lines.Count.ShouldBe(2, "lines inside the frame");

        // The frame's is the only 9 pt text in the document, which separates it from the body without
        // the test needing to know where the frame is — the thing under test. Both coordinates,
        // because between them they pin the whole rectangle: the shape's own text is drawn at its
        // corner plus the four dxText insets, which this document states as zero.
        List<(double X, double Y)> reference = [.. PdfTextRuns
            .Read(_libreOffice.ConvertToPdf(path, _workDirectory))
            .Where(run => Math.Abs(run.FontSize - FramePoints) < 0.01)
            .Select(run => (run.X, run.Y))
            .Distinct()
            .OrderBy(run => run.Y)];

        List<(double X, double Y)> drawn = [.. sink.Pages
            .SelectMany(drawnPage => drawnPage.Runs)
            .Where(run => Math.Abs(run.Run.FontSize.Points - FramePoints) < 0.01)
            .Select(run => (run.Origin.X.Points, run.Origin.Y.Points))
            .Distinct()
            .OrderBy(run => run.Item2)];

        drawn.Count.ShouldBe(reference.Count, "lines inside the frame");
        for (int i = 0; i < reference.Count; i++)
        {
            Close(drawn[i].X, reference[i].X, $"frame line {i + 1} starts at");
            Close(drawn[i].Y, reference[i].Y, $"frame line {i + 1} baseline");
        }
    }

    /// <summary>One drawn line: which page it is on, where it starts, and where its baseline sits.</summary>
    private readonly record struct Line(int Page, double Left, double Baseline);

    private List<Line> Reference(string path)
        => Group(
            PdfTextRuns
                .Read(_libreOffice.ConvertToPdf(path, _workDirectory))
                .Where(run => Math.Abs(run.FontSize - BodyPoints) < 0.01)
                .Select(run => new Line(run.PageIndex, run.X, run.Y)));

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

    private static List<Line> Group(IEnumerable<Line> runs)
        => [.. runs
            .GroupBy(run => (run.Page, Baseline: Math.Round(run.Baseline, 2)))
            .Select(group => new Line(group.Key.Page, group.Min(run => run.Left), group.Key.Baseline))
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
