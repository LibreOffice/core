using System.Globalization;
using System.Text.RegularExpressions;
using Paperless.Core.Documents;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Paperless.WordProcessing;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Checks that the glyph runs a page draws land where LibreOffice draws its text.
/// </summary>
/// <remarks>
/// <para>
/// The last link: pagination decides which line goes where, and this is what a backend is handed. The
/// comparison is against a display list rather than against pixels, which is the point of having one — an
/// image comparison answers "does it look the same", which is both a weaker question and much harder to
/// attribute, since a baseline one point out and a wrong glyph look equally different.
/// </para>
/// <para>
/// LibreOffice's PDF gives the ground truth twice over: <c>pdftotext -bbox</c> reports each word's box,
/// whose left edge is where the text starts and whose top is the line box's top. Comparing the run's
/// origin against the first and the run's width against the last word's right edge checks the placement
/// and the measurement separately, which is what makes a failure diagnosable.
/// </para>
/// </remarks>
public sealed class PageDrawingComparisonTests : IDisposable
{
    /// <summary>
    /// How far a drawn position may differ from LibreOffice's, in points.
    /// </summary>
    /// <remarks>
    /// Half a point. A PDF word box's left edge is where the first glyph's ink starts rather than where
    /// the pen was, and its top is the line box's — both differ from a baseline origin by a fraction of a
    /// point that depends on the glyph. Anything larger is a real difference in placement.
    /// </remarks>
    private const double TolerancePoints = 0.5;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-drawing").FullName;

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
    [InlineData("paginated.fodt")]
    [InlineData("paginated.docx")]
    [InlineData("paginated.doc")]
    [InlineData("paginated.rtf")]
    public void EveryLineIsDrawnWhereLibreOfficeDrawsIt(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);

        RecordingDrawingSink sink = new();
        using (IDocument document = Open(path))
        {
            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            for (int i = 0; i < pages.Count; i++) pages[i].Draw(sink);
        }

        sink.UnclosedPages.ShouldBe(0, "a page left open nests the next one inside it");

        List<List<RenderedWord>> reference = RenderedWords(path);
        sink.Pages.Count.ShouldBe(reference.Count, $"{fileName}: page count");

        for (int page = 0; page < reference.Count; page++)
        {
            List<RenderedLine> lines = GroupIntoLines(reference[page]);
            List<DrawnGlyphRun> runs = sink.Pages[page].Runs;

            runs.Count.ShouldBe(lines.Count, $"{fileName}: page {page + 1} line count");

            for (int i = 0; i < lines.Count; i++)
            {
                RenderedLine line = lines[i];
                DrawnGlyphRun run = runs[i];
                string where = $"{fileName}: page {page + 1}, line {i + 1} (\"{line.Text}\")";

                // Where the line starts. The x is the pen position against the first glyph's ink, so it
                // is the one number a whole-page offset would show up in.
                Close(run.Origin.X, line.Left, $"{where}: left");

                // And how wide it is, which is the measurement rather than the placement: a run drawn in
                // the right place with the wrong advances has the right origin and the wrong extent.
                Close(run.Origin.X + run.Width, line.Right, $"{where}: right");
            }
        }
    }

    [Theory]
    [InlineData("paginated.fodt")]
    [InlineData("paginated.docx")]
    public void BaselinesFallOnLibreOfficesLinePitch(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);

        RecordingDrawingSink sink = new();
        using (IDocument document = Open(path))
        {
            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            pages[0].Draw(sink);
        }

        List<RenderedLine> lines = GroupIntoLines(RenderedWords(path)[0]);
        List<DrawnGlyphRun> runs = sink.Pages[0].Runs;
        runs.Count.ShouldBe(lines.Count);

        // The distance between consecutive baselines, which is the number pagination adds up. Compared as
        // a pitch rather than as an absolute position because a PDF word box's top is the line box's top
        // and not a baseline: the two differ by the font's ascent, which cancels between two lines only
        // when both are set in the same font. So the comparison skips the pair that straddles a size
        // change — the corpus document's 16 pt headings — where the ascent does not cancel and the
        // difference would be about poppler's idea of an ascent rather than about the layout.
        int compared = 0;
        for (int i = 1; i < runs.Count; i++)
        {
            if (runs[i].Run.FontSize != runs[i - 1].Run.FontSize) continue;

            double drawn = (runs[i].Origin.Y - runs[i - 1].Origin.Y).Points;
            double rendered = lines[i].Top - lines[i - 1].Top;
            compared++;

            Math.Abs(drawn - rendered).ShouldBeLessThanOrEqualTo(
                TolerancePoints,
                $"{fileName}: pitch between lines {i} and {i + 1} is {rendered:F3} pt rendered "
                + $"and {drawn:F3} pt drawn");
        }

        // Including the pitch across a paragraph boundary, which is the one the paragraph spacing decides
        // and the one that would be wrong if space-after and space-before were combined differently.
        compared.ShouldBeGreaterThan(30, $"{fileName}: too few comparable pitches to prove anything");
    }

    [Fact]
    public void ARunCarriesWhatABackendNeedsToDrawIt()
    {
        string path = Corpus.Require("paginated.fodt");

        RecordingDrawingSink sink = new();
        using (IDocument document = Open(path))
        {
            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            pages[0].Draw(sink);
        }

        DrawnGlyphRun run = sink.Pages[0].Runs[0];

        // The font it was measured with, so a PDF backend can embed the right face and a raster one can
        // ask the right face for outlines. Naming the requested family instead would embed a font the
        // document was not laid out against.
        run.Run.Font.FamilyName.ShouldBe("Carlito");
        run.Run.FontSize.ShouldBe(Length.FromPoints(11));

        // Glyph ids, not characters: shaping has already happened and a backend must not redo it, or its
        // output would disagree with the line breaks around it.
        run.Run.Glyphs.ShouldNotBeEmpty();
        run.Run.Glyphs.ShouldAllBe(g => g.GlyphId > 0);
        run.Run.Glyphs.ShouldAllBe(g => g.Advance > Length.Zero);

        // The text and the cluster map, which are what make extracted text from a rendered page work and
        // a PDF's ToUnicode correct.
        run.Text.ShouldStartWith("Section 1.");
        run.Run.ClusterMap.Count.ShouldBe(run.Run.Glyphs.Count);
        run.Run.ClusterMap.ShouldBeInOrder();

        // And a colour, since a paint is what the sink is given rather than something it infers.
        run.Paint.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(Colour.Black);
    }

    [Fact]
    public void APageIsDrawnBetweenABeginAndAnEnd()
    {
        string path = Corpus.Require("paginated.fodt");

        RecordingDrawingSink sink = new();
        using (IDocument document = Open(path))
        {
            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            for (int i = 0; i < pages.Count; i++) pages[i].Draw(sink);
        }

        // One recorded page per drawn page, each with the sheet's own size — a backend allocates its
        // surface from this, so a page that reported the text area instead would clip its own margins.
        sink.Pages.Count.ShouldBe(5);
        sink.Pages.ShouldAllBe(p => Math.Abs(p.Size.Width.Millimetres - 210.0) < 0.1);
        sink.Pages.ShouldAllBe(p => Math.Abs(p.Size.Height.Millimetres - 297.0) < 0.1);
        sink.Pages.ShouldAllBe(p => p.Runs.Count > 0);
    }

    // ------------------------------------------------------------------------- the machinery

    private static IDocument Open(string path)
    {
        using FileStream stream = File.OpenRead(path);
        using DocumentSource source = DocumentSource.FromStream(stream, Path.GetFileName(path));
        return new WordProcessingReader().Read(source);
    }

    private static void Close(Length actual, double expectedPoints, string what)
        => Math.Abs(actual.Points - expectedPoints).ShouldBeLessThanOrEqualTo(
            TolerancePoints,
            $"{what}: {actual.Points:F3} pt drawn, {expectedPoints:F3} pt rendered");

    /// <summary>One word as LibreOffice's PDF reports it.</summary>
    private readonly record struct RenderedWord(double Left, double Right, double Top, string Text);

    /// <summary>One line: the extent of its words, and what they say.</summary>
    private readonly record struct RenderedLine(double Left, double Right, double Top, string Text);

    /// <summary>
    /// The words on each page, grouped later rather than here.
    /// </summary>
    private List<List<RenderedWord>> RenderedWords(string documentPath)
    {
        string pdf = _libreOffice.ConvertToPdf(documentPath, _workDirectory);

        System.Diagnostics.ProcessStartInfo start = new("pdftotext")
        {
            RedirectStandardOutput = true,
            RedirectStandardError = true,
        };
        start.ArgumentList.Add("-bbox");
        start.ArgumentList.Add(pdf);
        start.ArgumentList.Add("-");

        using System.Diagnostics.Process process =
            System.Diagnostics.Process.Start(start)
            ?? throw new InvalidOperationException("pdftotext did not start");

        string output = process.StandardOutput.ReadToEnd();
        process.WaitForExit((int)LibreOfficeRunner.Timeout.TotalMilliseconds);

        Assert.SkipWhen(
            process.ExitCode != 0,
            "pdftotext is not available; install poppler-utils — see check-env.sh");

        List<List<RenderedWord>> pages = [];
        foreach (Match page in Regex.Matches(
                     output, "<page[^>]*>(.*?)</page>", RegexOptions.Singleline))
        {
            List<RenderedWord> words = [];
            foreach (Match word in Regex.Matches(
                         page.Groups[1].Value,
                         "<word xMin=\"([0-9.]+)\" yMin=\"([0-9.]+)\" xMax=\"([0-9.]+)\""
                         + "[^>]*>([^<]*)</word>"))
            {
                words.Add(new RenderedWord(
                    Number(word.Groups[1].Value),
                    Number(word.Groups[3].Value),
                    Number(word.Groups[2].Value),
                    System.Net.WebUtility.HtmlDecode(word.Groups[4].Value)));
            }

            pages.Add(words);
        }

        return pages;
    }

    /// <summary>
    /// Groups a page's words into lines by their vertical position.
    /// </summary>
    /// <remarks>
    /// A PDF has no lines, only positioned text, so words whose tops are within a point of each other are
    /// taken to be on one. Safe here because the corpus document's line pitch is over fourteen points.
    /// </remarks>
    private static List<RenderedLine> GroupIntoLines(List<RenderedWord> words)
    {
        List<RenderedLine> lines = [];
        if (words.Count == 0) return lines;

        double top = words[0].Top;
        double left = words[0].Left;
        double right = words[0].Right;
        List<string> text = [words[0].Text];

        for (int i = 1; i < words.Count; i++)
        {
            RenderedWord word = words[i];
            if (Math.Abs(word.Top - top) <= 1.0)
            {
                right = Math.Max(right, word.Right);
                text.Add(word.Text);
                continue;
            }

            lines.Add(new RenderedLine(left, right, top, string.Join(' ', text)));
            top = word.Top;
            left = word.Left;
            right = word.Right;
            text = [word.Text];
        }

        lines.Add(new RenderedLine(left, right, top, string.Join(' ', text)));
        return lines;
    }

    private static double Number(string value)
        => double.Parse(value, CultureInfo.InvariantCulture);
}
