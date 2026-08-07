using Paperless.Core.Documents;
using Paperless.Rendering.Pdf;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Where LibreOffice puts a text box whose rectangle is not a whole number of hundredths of a
/// millimetre.
/// </summary>
/// <remarks>
/// <para>
/// oox builds a shape's matrix in EMUs and scales it into hundredths of a millimetre at the end
/// (<c>oox/source/drawingml/shape.cxx</c>:1226-1230, fetched at tag
/// <c>libreoffice-24.2.7.2</c> — the release that made this reference); <c>SvxShape</c> then hands
/// the result to <c>SdrObject::SetSnapRect</c>, whose <c>tools::Rectangle</c> holds four
/// <c>sal_Int32</c> of the model's map unit. So the rectangle a slide's text is broken and
/// anchored against has integer edges, and ours carried the file's full EMU precision.
/// </para>
/// <para>
/// <c>slide-text-area-grid.pptx</c> is authored to separate the two readings, because no corpus
/// deck can: the difference is at most half a unit, 0.0142 pt, and every ordinary tolerance in
/// this suite is 0.1 pt. Its three boxes state <c>lIns</c>, <c>tIns</c>, <c>rIns</c> and
/// <c>bIns</c> of zero and one 18 pt run each, so nothing between the rectangle and the pen can
/// absorb the difference:
/// </para>
/// <list type="table">
/// <item><description>914579 EMU across — 2540.497 units, rounding <em>down</em> to
/// 72.000 pt against the 72.0141 the EMU value gives.</description></item>
/// <item><description>1828981 EMU across — 5080.503 units, rounding <em>up</em> to
/// 144.028 pt against 144.0143.</description></item>
/// <item><description>3200579 EMU down — 8890.497 units, rounding down, so its baseline sits
/// 216.000 pt below the first box's rather than 216.0141.</description></item>
/// </list>
/// <para>
/// Two directions and two axes, so a constant shift cannot pass it. Measured against
/// LibreOffice 24.2.7.2, which draws 72, 144.028 and a 216.000 pt separation.
/// </para>
/// <para>
/// <strong>The two baselines are compared as a difference rather than absolutely</strong>,
/// because LibreOffice's PDF export puts its whole page one unit — 0.028 pt — above ours: its
/// background rectangle starts at <c>y = 0.028</c> on a 540 pt page and every baseline on the
/// slide carries the same offset. Subtracting one baseline from another cancels it and leaves
/// exactly the quantity under test. The horizontal pens carry no such offset and are compared
/// outright.
/// </para>
/// </remarks>
public sealed class SlideTextAreaGridComparisonTests : IDisposable
{
    private const string Deck = "slide-text-area-grid.pptx";

    /// <summary>
    /// Five thousandths of a point.
    /// </summary>
    /// <remarks>
    /// A third of the 0.0142 pt the defect is worth, and far tighter than the 0.1 pt this suite
    /// uses elsewhere — deliberately, because 0.1 pt is seven times the whole effect and a test
    /// carrying it here could not fail. Both writers state these pens to three decimal places,
    /// so the precision is available.
    /// </remarks>
    private const double TolerancePoints = 0.005;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-text-area-grid").FullName;

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
    public void ATextRectangleIsRoundedToTheDrawLayersOwnGrid()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        List<PdfTextRun> ours = PdfTextRuns.Read(Ours());
        List<PdfTextRun> theirs = PdfTextRuns.Read(
            _libreOffice.ConvertToPdf(Corpus.Require(Deck), _workDirectory));

        Assert.SkipWhen(theirs.Count == 0, "pdftotext is not available; install poppler-utils");

        ours.Count.ShouldBe(3, "boxes drawn");
        theirs.Count.ShouldBe(3, "boxes the reference drew");

        // Ours as literals first, so this is a test of Paperless rather than of LibreOffice:
        // a comparison against the reference alone passes whatever we draw.
        ours[0].X.ShouldBe(72.000, TolerancePoints, "box one: rounds down");
        ours[1].X.ShouldBe(144.028, TolerancePoints, "box two: rounds up");
        (ours[2].Y - ours[0].Y).ShouldBe(216.000, TolerancePoints, "box three: rounds down");

        // And the reference against the same literals.
        theirs[0].X.ShouldBe(ours[0].X, TolerancePoints, "box one: pen across");
        theirs[1].X.ShouldBe(ours[1].X, TolerancePoints, "box two: pen across");
        (theirs[2].Y - theirs[0].Y).ShouldBe(
            ours[2].Y - ours[0].Y, TolerancePoints, "box three: baseline separation");
    }

    private string Ours()
    {
        string source = Corpus.Require(Deck);
        string destination = Path.Combine(_workDirectory, "text-area-grid-paperless.pdf");

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
