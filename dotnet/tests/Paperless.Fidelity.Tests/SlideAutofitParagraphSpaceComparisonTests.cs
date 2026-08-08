using Paperless.Core.Documents;
using Paperless.Rendering.Pdf;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Whether the shrink-to-fit search's line-spacing scale also reaches a paragraph's own space.
/// </summary>
/// <remarks>
/// <para>
/// EditEngine puts every <c>SvxULSpaceItem</c> through <c>scaleYSpacingValue</c>
/// (<c>ImpEditEngine::CalcHeight</c>, <c>editeng/source/editeng/impedit2.cxx</c>), and that helper
/// is a no-op only when <c>maStatus.DoStretch()</c> is clear or the scale is one. Under autofit
/// the flag is <em>set</em>: <c>SdrTextObj::ImpSetupDrawOutlinerForPaint</c> turns
/// <c>EEControlBits::STRETCHING</c> on whenever <c>IsFitToSize() || IsAutoFit()</c> and only then
/// calls <c>setupAutoFitText</c> (<c>svx/source/svdraw/svdotext.cxx</c>:1177-1183).
/// </para>
/// <para>
/// The corpus cannot settle it, because the two readings differ only where the search's answer
/// tips between a larger font at reduced spacing and a smaller one at full spacing, and a corpus
/// page carries a dozen other differences at once.
/// <c>slide-autofit-paragraph-space.pptx</c> is authored for it: three boxes of one text, all
/// four insets zero, four paragraphs of two lines each with the second line forced by a hard
/// break so the line count cannot move with the font size, and a 12 pt <c>a:spcBef</c> on every
/// paragraph. A throwaway shape goes first on the slide, because LibreOffice's shared draw
/// outliner formats the first text object on a page before <c>SetFixedCellHeight</c> takes hold.
/// </para>
/// <para>
/// The box heights are the discriminating ones out of a twelve-box sweep
/// (<c>research/probes/slides-r20/make-spacing-probe.py</c>):
/// </para>
/// <list type="table">
/// <item><description>150 pt — the reference draws 11.99 pt; scaling only the lines gives
/// 15.00 pt at four-fifths spacing.</description></item>
/// <item><description>200 pt — the reference draws 17.01 pt; scaling only the lines gives
/// 18.99 pt at nine-tenths.</description></item>
/// <item><description>210 pt — both readings give 18.00 pt. A control, so a blanket shift
/// downwards cannot pass.</description></item>
/// </list>
/// <para>
/// The differences under test are two and three whole points, so the 0.05 pt tolerance here is
/// two orders of magnitude below the effect and exists only to absorb the draw layer's own
/// 1/100 mm grid — a 12 pt run is 423 units, which is 11.9905 pt rather than 12.
/// </para>
/// </remarks>
public sealed class SlideAutofitParagraphSpaceComparisonTests : IDisposable
{
    private const string Deck = "slide-autofit-paragraph-space.pptx";

    /// <summary>A twentieth of a point: far below the whole points this separates.</summary>
    private const double TolerancePoints = 0.05;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-autofit-para-space").FullName;

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

    [Fact]
    public void TheFitsSpacingScaleReachesAParagraphsOwnSpace()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        List<PdfTextRun> ours = PdfTextRuns.Read(Ours());
        List<PdfTextRun> theirs = PdfTextRuns.Read(
            _libreOffice.ConvertToPdf(Corpus.Require(Deck), _workDirectory));

        Assert.SkipWhen(theirs.Count == 0, "pdftotext is not available; install poppler-utils");

        // Ours as literals first, so this is a test of Paperless rather than of LibreOffice: a
        // comparison against the reference alone passes whatever we happen to draw.
        SizeAt(ours, 10).ShouldBe(11.99, TolerancePoints, "150 pt box");
        SizeAt(ours, 240).ShouldBe(17.01, TolerancePoints, "200 pt box");
        SizeAt(ours, 470).ShouldBe(18.00, TolerancePoints, "210 pt box — the control");

        // And the reference against the same three.
        SizeAt(theirs, 10).ShouldBe(SizeAt(ours, 10), TolerancePoints, "150 pt box");
        SizeAt(theirs, 240).ShouldBe(SizeAt(ours, 240), TolerancePoints, "200 pt box");
        SizeAt(theirs, 470).ShouldBe(SizeAt(ours, 470), TolerancePoints, "210 pt box");
    }

    /// <summary>
    /// The em size drawn in the box whose left edge is at <paramref name="left"/> points.
    /// </summary>
    /// <remarks>
    /// Keyed on the pen rather than on the drawing order, because the two writers do not emit the
    /// shapes in the same sequence and the warm-up shape sits between them. Every run in one box
    /// shares a size, so the first is the box's answer; that it is the <em>only</em> size in the
    /// box is asserted rather than assumed.
    /// </remarks>
    private static double SizeAt(List<PdfTextRun> runs, double left)
    {
        List<PdfTextRun> inBox = [.. runs.Where(r => Math.Abs(r.X - left) < 1.0)];

        inBox.Count.ShouldBe(8, $"runs in the box at x = {left}");
        inBox.Select(r => Math.Round(r.FontSize, 1)).Distinct().Count()
             .ShouldBe(1, $"sizes in the box at x = {left}");

        return inBox[0].FontSize;
    }

    private string Ours()
    {
        string source = Corpus.Require(Deck);
        string destination = Path.Combine(_workDirectory, "autofit-para-space-paperless.pdf");

        using IDocument document = PaperlessDocument.Open(source);
        IPageSequence pages = ((IPaginatedDocument)document).Layout();

        using FileStream output = File.Create(destination);
        new PdfRenderer(new PdfRenderOptions
        {
            CreationDate = new DateTimeOffset(2024, 1, 1, 0, 0, 0, TimeSpan.Zero),
        }).Render(pages, output);

        return destination;
    }
}
