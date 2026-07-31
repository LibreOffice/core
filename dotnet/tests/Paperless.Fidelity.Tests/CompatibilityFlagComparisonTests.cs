using Paperless.Core.Documents;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Paperless.WordProcessing;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Checks the one <c>w:compat</c> flag that measurably moves what this engine draws.
/// </summary>
/// <remarks>
/// <para>
/// The pair of corpus documents is the same text twice, differing only in whether
/// <c>settings.xml</c> carries <c>w:doNotExpandShiftReturn</c>. Both hold a justified paragraph
/// split by a <c>w:br</c> — so its first line is not the paragraph's last, and is therefore
/// stretched unless the flag says otherwise — followed by an ordinary justified paragraph that
/// must stay stretched either way, so that a reader which simply stopped justifying would fail.
/// </para>
/// <para>
/// Measured on LibreOffice 24.2: the broken line's last word sits at x = 538.75 pt without the
/// flag, hard against the right margin, and at x = 154.0 pt with it. Comparing against the
/// reference rather than against a stored number keeps this honest across a LibreOffice upgrade.
/// </para>
/// </remarks>
public sealed class CompatibilityFlagComparisonTests : IDisposable
{
    /// <summary>How far a drawn word may sit from LibreOffice's, in points.</summary>
    /// <remarks>
    /// A point, which is looser than the differential justification test's tenth of a point and
    /// has to be: this compares an absolute right edge rather than a difference, so it carries
    /// the whole of the width disagreement between HarfBuzz and Writer — measured at 0.62 pt at
    /// the end of a 480 pt justified line. What is being checked is a 385 pt difference, so the
    /// tolerance only has to exclude coincidence.
    /// </remarks>
    private const double TolerancePoints = 1.0;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-compat").FullName;

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

    /// <summary>
    /// The line the break ends is stretched to the margin without the flag and not with it, in
    /// both engines and by the same amount.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The stretched case is compared against LibreOffice absolutely — the line ends at the right
    /// margin and both engines agree to well under a point. The ragged case is compared as a
    /// <em>difference</em> instead, because LibreOffice 24.2 breaks a justified paragraph's lines
    /// differently from the same text left ragged: measured here, its unstretched line 2 ends 29 pt
    /// short of where this engine's does, because it re-broke the paragraph and this engine did not.
    /// That deviation is already recorded against <c>JustificationComparisonTests</c> and is not what
    /// this test is about; what it is about is that the line stops being stretched, which is a
    /// 385 pt effect in both.
    /// </para>
    /// <para>
    /// It is the <b>second</b> line that moves, which is the detail that cost the time. The
    /// paragraph's text wraps, so the line the <c>w:br</c> ends is not the paragraph's first — and a
    /// first-line test saw the two documents agree and concluded the flag did nothing.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheLineEndedByABreakIsStretchedExactlyWhenLibreOfficeStretchesIt()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string expandPath = Corpus.Require("compat-shift-expand.docx");
        string returnPath = Corpus.Require("compat-shift-return.docx");

        List<double> renderedExpand = Rendered(expandPath);
        Assert.SkipWhen(
            renderedExpand.Count == 0,
            "pdftotext is not available; install poppler-utils — see check-env.sh");

        List<double> renderedReturn = Rendered(returnPath);
        List<double> drawnExpand = LineEnds(
            Drawn(expandPath).Select(word => (word.Right, word.Baseline)));
        List<double> drawnReturn = LineEnds(
            Drawn(returnPath).Select(word => (word.Right, word.Baseline)));

        // Stretched: the line runs to the right margin, and the two engines agree there.
        Math.Abs(drawnExpand[1] - renderedExpand[1]).ShouldBeLessThanOrEqualTo(
            TolerancePoints,
            $"stretched, line 2 ends at {drawnExpand[1]:F2} pt against LibreOffice's "
            + $"{renderedExpand[1]:F2} pt");

        // Ragged: the same line stops being stretched, by the same amount in both.
        double byLibreOffice = renderedExpand[1] - renderedReturn[1];
        double byPaperless = drawnExpand[1] - drawnReturn[1];

        byLibreOffice.ShouldBeGreaterThan(300);
        byPaperless.ShouldBeGreaterThan(300);

        // Every other line is left exactly where it was: the flag is per line, not per paragraph.
        for (int i = 0; i < drawnExpand.Count; i++)
        {
            if (i == 1) continue;

            Math.Abs(drawnExpand[i] - drawnReturn[i]).ShouldBeLessThan(
                0.01, $"line {i + 1} moved as well as the one the break ends");
        }
    }

    private List<double> Rendered(string path)
        => LineEnds(
            PdfWords.Read(_libreOffice.ConvertToPdf(path, _workDirectory))
                    .Select(word => (word.Right, word.Top)));

    /// <summary>The right edge of the last word on each line, top to bottom.</summary>
    private static List<double> LineEnds(IEnumerable<(double Right, double Top)> words)
        => [.. words.GroupBy(word => Math.Round(word.Top, 1))
                    .OrderBy(line => line.Key)
                    .Select(line => line.Max(word => word.Right))];

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
