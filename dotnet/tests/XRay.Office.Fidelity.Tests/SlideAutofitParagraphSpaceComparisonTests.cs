using XRay.Office.Core.Documents;
using XRay.Office.Rendering.Pdf;
using XRay.Office.TestKit;
using XRay.Office.TestKit.LibreOffice;
using Shouldly;

namespace XRay.Office.Fidelity.Tests;

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
/// <item><description>150 pt — scaling only the lines lands a level away.</description></item>
/// <item><description>200 pt — likewise.</description></item>
/// <item><description>210 pt — both readings agree. A control, so a blanket shift downwards
/// cannot pass.</description></item>
/// </list>
/// <para>
/// <strong>The sizes those boxes resolve to are a property of the reference's version, and this
/// file's literals were a release behind.</strong> The autofit ladder moved between 24.2.7.2 and
/// 26.2.4.2 — <c>SlideAutofit.FitLevels</c> ports 26.2's <c>constScaleLevels</c>, where 24.2
/// bisected — so the same deck resolves differently. Measured on this fixture, nothing changed
/// but the binary:
/// </para>
/// <list type="table">
/// <item><description>150 pt: <b>11.991</b> under 24.2.7.2, <b>14.003</b> under 26.2.4.2</description></item>
/// <item><description>200 pt: <b>17.008</b> and <b>18.992</b></description></item>
/// <item><description>210 pt: <b>18.000</b> and <b>20.013</b></description></item>
/// </list>
/// <para>
/// Ours is 26.2.4.2's to the hundredth on all three. The literals now say so, and name the
/// version they came from — a bare number here is a claim about an environment that the number
/// itself cannot carry.
/// </para>
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
        Directory.CreateTempSubdirectory("xray-office-autofit-para-space").FullName;

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
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, LibreOfficeRunner.UnavailableReason);

        List<PdfTextRun> ours = PdfTextRuns.Read(Ours());
        List<PdfTextRun> theirs = PdfTextRuns.Read(
            _libreOffice.ConvertToPdf(Corpus.Require(Deck), _workDirectory));

        Assert.SkipWhen(theirs.Count == 0, "pdftotext is not available; install poppler-utils");

        // Ours as literals first, so this is a test of X-Ray.Office rather than of LibreOffice: a
        // comparison against the reference alone passes whatever we happen to draw.
        //
        // These are 26.2.4.2's answers, which is the reference this tree is developed against,
        // and they were 24.2.7.2's until this was measured. The autofit ladder moved between the
        // two releases -- `SlideAutofit.FitLevels` is a port of 26.2's `constScaleLevels`, where
        // 24.2 bisected -- so the same deck resolves differently, and the literals are only
        // meaningful with the version they came from named:
        //
        //     box      24.2.7.2   26.2.4.2   ours
        //     150 pt     11.991     14.003   14.0031
        //     200 pt     17.008     18.992   18.9921
        //     210 pt     18.000     20.013   20.0126
        //
        // Ours is 26.2.4.2's to the hundredth on all three, so what this file set out to check --
        // that the fit's spacing scale reaches a paragraph's own space -- is answered: the deck
        // was authored so that failing to scale it lands on a different level, and we land on the
        // reference's.
        SizeAt(ours, 10).ShouldBe(14.00, TolerancePoints, "150 pt box");
        SizeAt(ours, 240).ShouldBe(18.99, TolerancePoints, "200 pt box");
        SizeAt(ours, 470).ShouldBe(20.01, TolerancePoints, "210 pt box — the control");

        // And the reference against the same three. This half is version-sensitive by
        // construction and will fail on a 24.2.7.2 machine, which is the correct outcome: the
        // divergence is real and belongs in the open, not smoothed over by a tolerance wide
        // enough to swallow two whole points.
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
        string destination = Path.Combine(_workDirectory, "autofit-para-space-xray-office.pdf");

        using IDocument document = OfficeDocument.Open(source);
        IPageSequence pages = ((IPaginatedDocument)document).Layout();

        using FileStream output = File.Create(destination);
        new PdfRenderer(new PdfRenderOptions
        {
            CreationDate = new DateTimeOffset(2024, 1, 1, 0, 0, 0, TimeSpan.Zero),
        }).Render(pages, output);

        return destination;
    }
}
