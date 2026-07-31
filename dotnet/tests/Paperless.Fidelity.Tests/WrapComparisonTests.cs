using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Paperless.WordProcessing;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Checks that text goes round a floating frame where LibreOffice puts it.
/// </summary>
/// <remarks>
/// <para>
/// The first feature in this library where a line's available width is not a property of its paragraph:
/// a frame occupies a rectangle, and whether a line runs into it depends on where the line is. So the
/// interesting assertion is not that the wrapped lines are narrower — it is <em>which</em> lines are
/// wrapped, since that is the part a reader could get right by accident and wrong by one line.
/// </para>
/// <para>
/// The corpus document is built so that both edges of the frame's reach are inside it: three lines above
/// the frame at the full width, seven beside it, and one below it back at the full width. A reader
/// applying the wrap to the whole paragraph would narrow the first three; one applying it to the anchoring
/// paragraph alone would leave the last four wide.
/// </para>
/// </remarks>
public sealed class WrapComparisonTests : IDisposable
{
    /// <inheritdoc cref="TableComparisonTests.TolerancePoints"/>
    private const double TolerancePoints = 0.25;

    /// <summary>What LibreOffice's PDF export adds to every horizontal pen position.</summary>
    private const double PdfPenOffsetPoints = 0.1;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-wrap").FullName;

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
    [InlineData("wrap-frame.fodt")]
    public void TextGoesRoundAFrameWhereLibreOfficePutsIt(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        List<PdfWord> words = PdfWords.Read(_libreOffice.ConvertToPdf(path, _workDirectory));

        Assert.SkipWhen(
            words.Count == 0,
            "pdftotext is not available; install poppler-utils — see check-env.sh");

        List<(double At, double Left, string First)> rendered = LinesOf(
            ReadingOrder.Of(words), word => word.Top, word => word.Left, word => word.Text);
        List<(double At, double Left, string First)> drawn = LinesOf(
            ReadingOrder.Of(Drawn(path)), word => word.Baseline, word => word.Left, word => word.Text);

        drawn.Count.ShouldBe(
            rendered.Count, $"{fileName}: laid out {drawn.Count} lines, LibreOffice {rendered.Count}");

        for (int i = 0; i < rendered.Count; i++)
        {
            double mine = drawn[i].Left;
            double theirs = rendered[i].Left - PdfPenOffsetPoints;

            Math.Abs(mine - theirs).ShouldBeLessThanOrEqualTo(
                TolerancePoints,
                $"{fileName}: line {i + 1} (\"{rendered[i].First}\") starts at {mine:F2} pt drawn, "
                + $"{theirs:F2} pt rendered");
        }
    }

    [Fact]
    public void TheFrameItselfLandsOnThePageItsAnchorDid()
    {
        // Structural, and it needs no LibreOffice: the frame is 5 cm by 3 cm, one centimetre below the top of
        // its anchoring paragraph, which is the second on the page. If it were resolved against the page
        // rather than against the anchor it would sit a line and a half higher.
        LaidOutPage page = Pages("wrap-frame.fodt")[0];

        PlacedFrame frame = page.Frames.ShouldHaveSingleItem();

        frame.Bounds.Width.Millimetres.ShouldBe(50.0, 0.1);
        frame.Bounds.Height.Millimetres.ShouldBe(30.0, 0.1);

        // Its region is the frame grown by the wrap margins the style states: 0.2 cm each side, none
        // vertically. So the region is wider than the frame and exactly as tall.
        frame.Region.Width.Millimetres.ShouldBe(54.0, 0.1);
        frame.Region.Height.ShouldBe(frame.Bounds.Height);

        // A centimetre below the anchoring paragraph, whose own top is one line below the text area's.
        (frame.Bounds.Y - page.BodyArea.Y).Millimetres.ShouldBeGreaterThan(10.0);
    }

    /// <summary>
    /// One entry per line of a page's words: where it sits, where it starts, and its first word.
    /// </summary>
    /// <remarks>
    /// The words arrive already in reading order, so a new line begins wherever the vertical jumps by more
    /// than a line's worth. That is all this needs to know — the assertion is about each line's left edge.
    /// </remarks>
    private static List<(double At, double Left, string First)> LinesOf<T>(
        List<T> words, Func<T, double> vertical, Func<T, double> horizontal, Func<T, string> text)
    {
        List<(double At, double Left, string First)> lines = [];

        foreach (T word in words)
        {
            if (lines.Count > 0
                && Math.Abs(vertical(word) - lines[^1].At) <= ReadingOrder.SameLinePoints)
            {
                if (horizontal(word) < lines[^1].Left)
                {
                    lines[^1] = lines[^1] with { Left = horizontal(word), First = text(word) };
                }

                continue;
            }

            lines.Add((vertical(word), horizontal(word), text(word)));
        }

        return lines;
    }

    private static IReadOnlyList<LaidOutPage> Pages(string fileName)
    {
        using FileStream stream = File.OpenRead(Corpus.Require(fileName));
        using DocumentSource source = DocumentSource.FromStream(stream, Path.GetFileName(fileName));
        using IDocument document = new WordProcessingReader().Read(source);

        return ((WordProcessingPages)((IPaginatedDocument)document).Layout()).Pages;
    }

    private static List<DrawnWord> Drawn(string path)
    {
        RecordingDrawingSink sink = new();

        using (FileStream stream = File.OpenRead(path))
        {
            using DocumentSource source = DocumentSource.FromStream(stream, Path.GetFileName(path));
            using IDocument document = new WordProcessingReader().Read(source);

            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            pages[0].Draw(sink);
        }

        return [.. DrawnWords.On(sink.Pages[0])];
    }
}
