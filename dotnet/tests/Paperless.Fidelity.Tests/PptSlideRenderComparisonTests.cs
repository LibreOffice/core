using Paperless.Core.Documents;
using Paperless.Rendering.Pdf;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Compares the slides Paperless draws from a <em>binary</em> PowerPoint deck against the slides
/// LibreOffice draws from the same file.
/// </summary>
/// <remarks>
/// <para>
/// The binary counterpart of <see cref="SlideRenderComparisonTests"/>, and the sharper of the two:
/// PPT states its coordinates in master units of a 576th of an inch, which divide an inch into
/// hundredths of a millimetre without remainder, so LibreOffice's export introduces none of the
/// 0.028 pt rounding it does on a PPTX. A shape stated at 72 pt comes out of the reference at
/// exactly 72.000, and a difference here is a difference rather than a rounding.
/// </para>
/// <para>
/// <strong>Slide 4 of <c>shape-geometry-ppt.ppt</c> is excluded from the fill comparison, and the
/// reason is the reference rather than us.</strong> LibreOffice's own PPTX-to-PPT export writes
/// every preset shape as a vertex array whose coordinates are references into a formula table,
/// <em>and</em> writes the geometry box those formulas are evaluated in as zero — so its own
/// importer resolves all four of that slide's triangles to a single point and its PDF draws
/// <c>72 468 m 72 468 l 72 468 l h f*</c> four times. Paperless draws their bounding rectangles,
/// which is in the right place in the right colour with the wrong outline, and is strictly more
/// than the reference manages. Comparing the two would assert that drawing nothing is correct.
/// </para>
/// </remarks>
public sealed class PptSlideRenderComparisonTests : IDisposable
{
    /// <summary>A tenth of a point, two twips, as everywhere else in this project.</summary>
    private const double TolerancePoints = 0.1;

    /// <summary>How many slides of the geometry deck carry comparable fills.</summary>
    /// <remarks>Three of four; see the note on the class for the fourth.</remarks>
    private const int ComparableFillSlides = 3;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-ppt-slides").FullName;

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
    [InlineData("shape-geometry-ppt.ppt")]
    [InlineData("slides-ppt.ppt")]
    public void EverySlideIsTheSizeTheDocumentAtomStates(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        List<PdfPageSize> ours = PdfPageSizes.Read(Ours(path));
        List<PdfPageSize> theirs = PdfPageSizes.Read(_libreOffice.ConvertToPdf(path, _workDirectory));

        Assert.SkipWhen(theirs.Count == 0, "pdftotext is not available; install poppler-utils");

        ours.Count.ShouldBe(theirs.Count, $"{fileName}: slide count");

        for (int i = 0; i < theirs.Count; i++)
        {
            ours[i].Width.ShouldBe(theirs[i].Width, TolerancePoints, $"{fileName}: slide {i + 1} width");
            ours[i].Height.ShouldBe(theirs[i].Height, TolerancePoints, $"{fileName}: slide {i + 1} height");
        }
    }

    [Theory]
    [InlineData("shape-geometry-ppt.ppt")]
    [InlineData("slides-ppt.ppt")]
    public void EveryRectangularFillIsWhereAndWhatLibreOfficeFillsIt(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        string mineOnDisk = Ours(path);
        string theirsOnDisk = _libreOffice.ConvertToPdf(path, _workDirectory);

        List<PdfFill> ours = ShapeFills(PdfFills.Read(mineOnDisk), PdfPageSizes.Read(mineOnDisk));
        List<PdfFill> theirs = ShapeFills(
            PdfFills.Read(theirsOnDisk), PdfPageSizes.Read(theirsOnDisk));

        ours.Count.ShouldBe(theirs.Count, $"{fileName}: number of rectangular fills");

        for (int i = 0; i < theirs.Count; i++)
        {
            PdfFill mine = ours[i];
            PdfFill reference = theirs[i];
            string where = $"{fileName}: fill {i + 1} on slide {reference.PageIndex + 1}";

            mine.PageIndex.ShouldBe(reference.PageIndex, $"{where}: slide");
            mine.Left.ShouldBe(reference.Left, TolerancePoints, $"{where}: left");
            mine.Top.ShouldBe(reference.Top, TolerancePoints, $"{where}: top");
            mine.Width.ShouldBe(reference.Width, TolerancePoints, $"{where}: width");
            mine.Height.ShouldBe(reference.Height, TolerancePoints, $"{where}: height");

            // The colour as well, which makes this a test of the colour-word decoding and not
            // only of placement: a shape's fill is a packed blue-green-red literal or an index
            // into the page's colour scheme, and the two are told apart by the top byte alone.
            mine.Colour.ShouldBe(reference.Colour, $"{where}: colour");
        }
    }

    [Fact]
    public void EveryLineOfShapeTextIsDrawnWhereLibreOfficeDrawsIt()
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        // shape-geometry-ppt.ppt alone. slides-ppt.ppt's master carries a footer and a
        // slide-number placeholder, which LibreOffice renders onto every slide and Paperless does
        // not draw at all — a master's own shapes are not yet placed on the slides under it, on
        // any of the three paths — so its run counts differ for a reason this comparison is not
        // about. The four text boxes here cover what it is about: the two inset spellings, a
        // centred paragraph and a middle-anchored one.
        string path = Corpus.Require("shape-geometry-ppt.ppt");
        List<PdfTextRun> ours = PdfTextRuns.Read(Ours(path));
        List<PdfTextRun> theirs = PdfTextRuns.Read(_libreOffice.ConvertToPdf(path, _workDirectory));

        ours.Count.ShouldBe(theirs.Count, "number of drawn text runs");

        for (int i = 0; i < theirs.Count; i++)
        {
            PdfTextRun mine = ours[i];
            PdfTextRun reference = theirs[i];
            string where = $"run {i + 1} on slide {reference.PageIndex + 1}";

            mine.PageIndex.ShouldBe(reference.PageIndex, $"{where}: slide");
            mine.X.ShouldBe(reference.X, TolerancePoints, $"{where}: pen across");

            // The vertical, which is the number that says the font-independent line height took
            // effect: the face's own ascent would put every one of these 1.7 pt high.
            mine.Y.ShouldBe(reference.Y, TolerancePoints, $"{where}: baseline");
            mine.FontSize.ShouldBe(reference.FontSize, 0.001, $"{where}: font size");
            mine.GlyphCount.ShouldBe(reference.GlyphCount, $"{where}: glyph count");
        }
    }

    /// <summary>
    /// The fills that are shapes rather than the sheet, on the slides worth comparing.
    /// </summary>
    /// <remarks>
    /// Both writers paint the slide background as a full-page rectangle, and both are excluded —
    /// not to hide a difference but because the page rectangle is the one fill whose size is the
    /// export's clip rather than the document's. A fill covering more than 95% of the sheet in
    /// both directions is the sheet.
    /// <para>
    /// Measured against the page rather than against a constant, because this deck is not the
    /// 720 × 540 one: <c>slides-ppt.ppt</c> is 793.7 × 446.5, and a fixed threshold of 450 points
    /// of height lets its sheet through as though it were a shape — which shows up as a fill
    /// count that is exactly one per page too high on each side and looks like a missing shape.
    /// </para>
    /// </remarks>
    private static List<PdfFill> ShapeFills(List<PdfFill> fills, List<PdfPageSize> pages)
        => [.. fills.Where(fill => fill.PageIndex < ComparableFillSlides && !IsSheet(fill, pages))];

    private static bool IsSheet(PdfFill fill, List<PdfPageSize> pages)
    {
        if (fill.PageIndex < 0 || fill.PageIndex >= pages.Count) return false;

        PdfPageSize page = pages[fill.PageIndex];
        return fill.Width > page.Width * 0.95 && fill.Height > page.Height * 0.95;
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
