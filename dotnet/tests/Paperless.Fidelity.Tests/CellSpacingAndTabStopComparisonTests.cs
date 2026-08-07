using Paperless.Core.Documents;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Paperless.WordProcessing;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Two rules a Word document's tables and running heads depend on, measured against LibreOffice.
/// </summary>
/// <remarks>
/// <para>
/// <c>cell-line-spacing.docx</c> holds the same six-row table twice, at 115% line spacing and at 100%,
/// so the fixture answers with a <em>difference</em>. That is deliberate: the two engines still disagree
/// about the absolute row pitch by half a point, because the borders this table's style declares are not
/// read yet, and a test on the absolute figure would be measuring that instead. The difference between
/// the two tables is exactly the allowance <c>AddParaLineSpacingToTableCells</c> adds and nothing else.
/// </para>
/// <para>
/// <c>style-tab-stops.docx</c> holds a running-head paragraph whose <c>w:tabs</c> clears one of its
/// style's stops and adds two of its own — so it can only be set correctly by an engine that merges the
/// two sets — and a contents entry whose style puts a dotted right stop at the text area's own width on
/// a paragraph that also carries a right indent, so the stop is past the line's edge and has to be
/// honoured there rather than breaking the entry into four lines.
/// </para>
/// </remarks>
public sealed class CellSpacingAndTabStopComparisonTests : IDisposable
{
    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-cell-tab").FullName;

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
    /// A 115%-spaced row is taller than a single-spaced one by what LibreOffice makes it taller by.
    /// </summary>
    /// <remarks>
    /// 41 twips for a twelve-point paragraph — <c>SwBorderAttrs::CalcLineSpacing_</c>,
    /// <c>sw/source/core/layout/frmtool.cxx</c>:2681 — which is 2.05 pt, and the whole of the gap this
    /// engine used to leave.
    /// </remarks>
    [Fact]
    public void ASpacedTableRowIsTallerThanASingleSpacedOneByLibreOfficesAllowance()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require("cell-line-spacing.docx");

        List<PdfWord> reference = PdfWords.Read(_libreOffice.ConvertToPdf(path, _workDirectory));
        Assert.SkipWhen(
            reference.Count == 0,
            "pdftotext is not available; install poppler-utils — see check-env.sh");

        double referenceGain = Pitch(reference, "Spaced") - Pitch(reference, "Single");
        double drawnGain = Pitch(Drawn(path), "Spaced") - Pitch(Drawn(path), "Single");

        referenceGain.ShouldBeInRange(1.9, 2.2, "the fixture must exercise the allowance at all");

        Math.Abs(drawnGain - referenceGain).ShouldBeLessThan(
            0.1,
            $"rows gain {drawnGain:F2} pt from 115% spacing against LibreOffice's {referenceGain:F2}");
    }

    /// <summary>
    /// A running head's third tab reaches the stop its <em>style</em> and its own <c>w:tabs</c> agree on.
    /// </summary>
    /// <remarks>
    /// The paragraph's set replaces the style's in an engine that does not merge them, and its last tab
    /// then has no stop left to reach: the trailing text starts past the margin and the line wraps. So the
    /// assertion is both that the text is where LibreOffice puts it and that it is on the first line.
    /// </remarks>
    [Fact]
    public void ARunningHeadsTabsReachTheStopsItsStyleContributed()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require("style-tab-stops.docx");

        List<PdfWord> reference = PdfWords.Read(_libreOffice.ConvertToPdf(path, _workDirectory));
        Assert.SkipWhen(
            reference.Count == 0,
            "pdftotext is not available; install poppler-utils — see check-env.sh");

        double referenceTop = reference.First(word => word.Text == "Autumn").Top;
        double referenceEnds = reference.First(word => word.Text == "Ends").Left;

        reference.First(word => word.Text == "Ends").Top.ShouldBe(referenceTop, 0.01);

        List<DrawnWord> drawn = Drawn(path);
        double drawnBaseline = drawn.First(word => word.Text == "Autumn").Baseline;
        DrawnWord ends = drawn.First(word => word.Text == "Ends");

        ends.Baseline.ShouldBe(
            drawnBaseline, 0.01, "the trailing text belongs on the running head's own line");

        Math.Abs(ends.Left - referenceEnds).ShouldBeLessThan(
            1.0, $"the tabbed text starts at {ends.Left:F2} pt against LibreOffice's {referenceEnds:F2}");
    }

    /// <summary>A contents entry whose right stop is past its line's edge stays on one line.</summary>
    /// <remarks>
    /// <para>
    /// LibreOffice keeps the number, the title, the leader and the page on one line; an engine that
    /// honours the stop where it was declared needs a line 720 twips wider than the paragraph has and
    /// breaks the entry four ways. Only the line count and the page number's line are asserted, not the
    /// leader's right edge: LibreOffice's <c>TabOverSpacing</c> lets the run reach the <em>frame's</em>
    /// edge rather than the paragraph's, so its dots run half a centimetre further than ours. That
    /// remains a real difference and is recorded rather than tested.
    /// </para>
    /// </remarks>
    [Fact]
    public void AContentsEntryWithAStopPastItsLineEdgeStaysOnOneLine()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require("style-tab-stops.docx");

        List<PdfWord> reference = PdfWords.Read(_libreOffice.ConvertToPdf(path, _workDirectory));
        Assert.SkipWhen(
            reference.Count == 0,
            "pdftotext is not available; install poppler-utils — see check-env.sh");

        // The page number, found by suffix: poppler reads the leader dots and the number as one token
        // where this engine draws them as two, since a tab leaves a gap and a dot leader does not.
        reference.First(word => word.Text.EndsWith("2-1", StringComparison.Ordinal)).Top
            .ShouldBe(reference.First(word => word.Text == "2.1.1.1").Top, 0.01);

        List<DrawnWord> drawn = Drawn(path);

        drawn.First(word => word.Text.EndsWith("2-1", StringComparison.Ordinal)).Baseline.ShouldBe(
            drawn.First(word => word.Text == "2.1.1.1").Baseline,
            0.01,
            "the page number belongs on the entry's own line");
    }

    /// <summary>The pitch between the first two rows whose text starts with a label.</summary>
    private static double Pitch(IEnumerable<PdfWord> words, string label)
    {
        List<double> tops = [.. words.Where(word => word.Text == label)
                                     .Select(word => word.Top).Order()];

        return tops[1] - tops[0];
    }

    /// <summary>The same, for what this engine drew.</summary>
    private static double Pitch(IEnumerable<DrawnWord> words, string label)
    {
        List<double> baselines = [.. words.Where(word => word.Text == label)
                                          .Select(word => word.Baseline).Order()];

        return baselines[1] - baselines[0];
    }

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
