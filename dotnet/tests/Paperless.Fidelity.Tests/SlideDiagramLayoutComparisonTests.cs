using Paperless.Core.Documents;
using Paperless.Rendering.Pdf;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Compares a diagram <em>evaluated</em> from its layout definition against LibreOffice's.
/// </summary>
/// <remarks>
/// <para>
/// This is the only honest check available for the evaluator. A diagram with a baked drawing can
/// be checked against the picture in its own package; one without has no picture in it at all,
/// so the only thing to compare against is another implementation running the same program over
/// the same data — and LibreOffice's is the one Paperless's is ported from
/// (<c>oox/source/drawingml/diagram/</c>).
/// </para>
/// <para>
/// <strong>The fixture is hand-written, and had to be.</strong> LibreOffice cannot author
/// SmartArt, so a diagram fixture cannot come out of a conversion; and a diagram from a real
/// authoring application always <em>has</em> a baked drawing, which is the path this is not
/// testing. <c>slide-diagram-evaluated.pptx</c> is therefore built on
/// <c>slide-diagram-baked.pptx</c>'s skeleton with the drawing part and its relationship
/// removed, and a layout definition that uses the four commonest algorithms at once: <c>lin</c>
/// divides the frame into thirds, <c>composite</c> places a box and a label inside each third
/// from constraints, <c>sp</c> draws the box and <c>tx</c> sets the label.
/// </para>
/// <para>
/// <strong>What the wider corpus says about this.</strong> Run over the 37 decks in
/// LibreOffice's <c>sd/qa/unit/data/pptx</c> that carry a diagram with no usable baked drawing,
/// the evaluator draws 20 and declines 17 for naming an algorithm it does not implement. On all
/// 20 every filled shape agrees with LibreOffice's own rendering to within 0.07 pt, which is the
/// quantisation of LibreOffice's internal hundredth of a millimetre and not a disagreement. The
/// text agrees exactly on the ten where LibreOffice does not shrink it to fit; see
/// <c>src/Paperless.Presentations/TODO.md</c> for the autofit divergence.
/// </para>
/// </remarks>
public sealed class SlideDiagramLayoutComparisonTests : IDisposable
{
    /// <summary>A tenth of a point, as everywhere else in this project.</summary>
    private const double TolerancePoints = 0.1;

    /// <summary>
    /// How far a centred label's baseline may differ, in points.
    /// </summary>
    /// <remarks>
    /// Wider than the tenth of a point the shapes use, and for a reason that is not the
    /// evaluator's: a slide's line height is not the font's, and the two renderers derive it
    /// slightly differently, so a vertically centred single line sits 0.77 pt apart on this
    /// fixture. The shapes it sits in agree to 0.03 pt.
    /// </remarks>
    private const double LabelTolerancePoints = 1.0;

    /// <summary>
    /// How far a centred label's pen may differ across, in points.
    /// </summary>
    /// <remarks>
    /// Centring <em>halves</em> a width difference and then adds it to a box edge, and
    /// LibreOffice measures a string in hundredths of a millimetre before it centres it, so
    /// "Gamma" starts 0.11 pt apart in the two renderings. The box it is centred in agrees to
    /// 0.03 pt, which is the number that would move if the evaluation were wrong.
    /// </remarks>
    private const double CentringTolerancePoints = 0.15;

    private const string Deck = "slide-diagram-evaluated.pptx";

    /// <summary>
    /// The three accents the colour transform cycles, which is how each box is found.
    /// </summary>
    /// <remarks>
    /// Matching by colour rather than by paint order for the same reason the baked comparison
    /// does: the two writers do not agree on how many paths a slide is. Cycling three distinct
    /// accents also makes the assertion say something the shapes alone cannot — that the colour
    /// list is indexed by the presentation point's <c>presStyleIdx</c> rather than repeated.
    /// </remarks>
    private static readonly uint[] NodeColours = [0x18A303, 0x0369A3, 0xA33E03];

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-diagram-layout").FullName;

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
    /// Every evaluated node lands where LibreOffice's evaluation lands it.
    /// </summary>
    /// <remarks>
    /// The presence of the shapes is half the assertion: before the evaluator a diagram with no
    /// baked drawing drew as nothing at all, and 37 of the 66 diagram-bearing decks in
    /// LibreOffice's own corpus are in that state. The position is the other half, and it is
    /// where the arithmetic shows: the boxes are at 72, 252 and 432 pt because the linear
    /// algorithm divides in integer EMUs, and 54 pt down from the frame because the composite
    /// centres what its constraints did not fill.
    /// </remarks>
    [Fact]
    public void EveryNodeIsFilledWhereLibreOfficeFillsIt()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(Deck);
        string ourPdf = Ours(path);
        string theirPdf = _libreOffice.ConvertToPdf(path, _workDirectory);

        Assert.SkipWhen(PdfFills.Read(theirPdf).Count == 0,
                        "pdftotext is not available; install poppler-utils");

        foreach (uint colour in NodeColours)
        {
            string where = $"node {colour:X6}";

            (double left, double top, double right, double bottom) mine =
                Painted(ourPdf, colour).ShouldNotBeNull($"{where}: not drawn at all");
            (double left, double top, double right, double bottom) reference =
                Painted(theirPdf, colour).ShouldNotBeNull($"{where}: not in the reference");

            mine.left.ShouldBe(reference.left, TolerancePoints, $"{where}: left");
            mine.top.ShouldBe(reference.top, TolerancePoints, $"{where}: top");
            mine.right.ShouldBe(reference.right, TolerancePoints, $"{where}: right");
            mine.bottom.ShouldBe(reference.bottom, TolerancePoints, $"{where}: bottom");
        }
    }

    /// <summary>
    /// Every label is drawn where LibreOffice draws it, at the size it draws it.
    /// </summary>
    /// <remarks>
    /// The size is worth asserting on its own: the <c>tx</c> algorithm can state a font size
    /// through a <c>primFontSz</c> constraint, and this fixture deliberately does not — its runs
    /// state 14 pt themselves, so both renderers use it and the comparison measures placement
    /// rather than the size negotiation. A fixture that let the constraint decide would instead
    /// measure LibreOffice's shrink-to-fit, which Paperless does not implement.
    /// </remarks>
    [Fact]
    public void EveryLabelIsDrawnWhereLibreOfficeDrawsIt()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(Deck);
        List<PdfTextRun> ours = PdfTextRuns.Read(Ours(path));
        List<PdfTextRun> theirs =
            PdfTextRuns.Read(_libreOffice.ConvertToPdf(path, _workDirectory));

        Assert.SkipWhen(theirs.Count == 0, "pdftotext is not available; install poppler-utils");

        ours.Count.ShouldBe(theirs.Count, "number of drawn labels");

        for (int i = 0; i < theirs.Count; i++)
        {
            PdfTextRun mine = ours[i];
            PdfTextRun reference = theirs[i];
            string where = $"label {i + 1}";

            mine.PageIndex.ShouldBe(reference.PageIndex, $"{where}: slide");
            mine.X.ShouldBe(reference.X, CentringTolerancePoints, $"{where}: pen across");
            mine.Y.ShouldBe(reference.Y, LabelTolerancePoints, $"{where}: baseline");
            mine.FontSize.ShouldBe(reference.FontSize, TolerancePoints, $"{where}: em size");
            mine.GlyphCount.ShouldBe(reference.GlyphCount, $"{where}: glyph count");
        }
    }

    /// <summary>
    /// The rectangle everything painted in one colour covers, or null when nothing was.
    /// </summary>
    /// <remarks>
    /// Both readers, unioned, because neither sees every spelling: <see cref="PdfPaths"/> reads
    /// paths that begin with an <c>m</c> and so misses PDF's own <c>re</c>, and
    /// <see cref="PdfFills"/> reads rectangles and so misses a rounded corner. A union over one
    /// colour is the same rectangle either way round, since each colour belongs to one shape.
    /// </remarks>
    private static (double Left, double Top, double Right, double Bottom)? Painted(
        string pdfPath, uint colour)
    {
        List<(double Left, double Top, double Right, double Bottom)> boxes =
        [
            .. PdfPaths.Read(pdfPath).Where(path => path.Colour == colour)
                .Select(path => path.Bounds),
            .. PdfFills.Read(pdfPath).Where(fill => fill.Colour == colour)
                .Select(fill => (fill.Left, fill.Top,
                                 fill.Left + fill.Width, fill.Top + fill.Height)),
        ];

        return boxes.Count == 0
            ? null
            : (boxes.Min(b => b.Left), boxes.Min(b => b.Top),
               boxes.Max(b => b.Right), boxes.Max(b => b.Bottom));
    }

    private string Ours(string documentPath)
    {
        string destination = Path.Combine(
            _workDirectory, $"{Path.GetFileNameWithoutExtension(documentPath)}-paperless.pdf");

        using IDocument document = PaperlessDocument.Open(documentPath);
        IPageSequence pages = ((IPaginatedDocument)document).Layout();

        using FileStream output = File.Create(destination);
        new PdfRenderer(new PdfRenderOptions
        {
            CreationDate = new DateTimeOffset(2024, 1, 1, 0, 0, 0, TimeSpan.Zero),
        }).Render(pages, output);

        return destination;
    }
}
