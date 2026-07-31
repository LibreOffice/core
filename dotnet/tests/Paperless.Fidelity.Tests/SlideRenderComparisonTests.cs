using Paperless.Core.Documents;
using Paperless.Rendering.Pdf;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Compares the slides Paperless draws against the slides LibreOffice draws, in its own PDF.
/// </summary>
/// <remarks>
/// <para>
/// The presentation counterpart of <see cref="PdfOutputComparisonTests"/>, and it asks the same
/// question of a family where almost nothing flows: is each shape the right size, in the right
/// place, in the right colour, with its text at the right pen. Both sides go through the same
/// readers, so what is compared is two content streams rather than two pictures.
/// </para>
/// <para>
/// <strong>What the readers can and cannot see, and why that is the right subset.</strong>
/// <see cref="PdfFills"/> reads a filled path only when it is an axis-parallel rectangle, so the
/// rotated rectangle and the four triangles on <c>shape-geometry.pptx</c> are invisible to it —
/// on <em>both</em> sides, which keeps the comparison honest rather than lenient. Their geometry
/// is asserted vertex by vertex against the same PDF's numbers in
/// <c>Paperless.Presentations.Tests</c>, where the expectations are transcribed once and need no
/// LibreOffice to check. <see cref="PdfStrokes"/> reads only two-point lines and a rectangle's
/// outline is a five-point closed path, so outlines are compared there too rather than here.
/// </para>
/// <para>
/// <strong>LibreOffice's export is one hundredth of a millimetre small and one up-and-left of
/// what the file says.</strong> Its page clip starts at <c>0 0.028</c>, its page background is
/// 719.971 pt on a 720 pt page, and a rectangle stated at 72 pt comes out at 71.972. It applies
/// to the sheet as well as to the shapes, so it is the export's own rounding into the drawing
/// layer's unit rather than a placement difference — 0.0283 pt, comfortably inside the tenth of a
/// point this project holds itself to everywhere.
/// </para>
/// </remarks>
public sealed class SlideRenderComparisonTests : IDisposable
{
    /// <summary>A tenth of a point, two twips, as everywhere else in this project.</summary>
    private const double TolerancePoints = 0.1;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-slides").FullName;

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
    [InlineData("shape-geometry.pptx")]
    [InlineData("shape-geometry.odp")]
    public void EverySlideIsTheSizeTheDeckStates(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        List<PdfPageSize> ours = PdfPageSizes.Read(Ours(path));
        List<PdfPageSize> theirs = PdfPageSizes.Read(_libreOffice.ConvertToPdf(path, _workDirectory));

        Assert.SkipWhen(theirs.Count == 0, "pdftotext is not available; install poppler-utils");

        // A page per slide, which is the whole of what pagination means here — and the cheapest
        // check that the deck's own p:sldSz reached the output rather than a default A4.
        ours.Count.ShouldBe(theirs.Count, $"{fileName}: slide count");

        for (int i = 0; i < theirs.Count; i++)
        {
            ours[i].Width.ShouldBe(theirs[i].Width, TolerancePoints, $"{fileName}: slide {i + 1} width");
            ours[i].Height.ShouldBe(theirs[i].Height, TolerancePoints, $"{fileName}: slide {i + 1} height");
        }
    }

    [Theory]
    [InlineData("shape-geometry.pptx")]
    [InlineData("shape-geometry.odp")]
    public void EveryRectangularFillIsWhereAndWhatLibreOfficeFillsIt(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        List<PdfFill> ours = ShapeFills(PdfFills.Read(Ours(path)));
        List<PdfFill> theirs = ShapeFills(PdfFills.Read(_libreOffice.ConvertToPdf(path, _workDirectory)));

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

            // The colour as well as the geometry, which is what makes this a test of theme
            // resolution and not only of placement: two of these fills are a scheme colour, one of
            // them under a lumMod/lumOff pair.
            mine.Colour.ShouldBe(reference.Colour, $"{where}: colour");
        }
    }

    [Theory]
    [InlineData("shape-geometry.pptx")]
    [InlineData("shape-geometry.odp")]
    public void EveryLineOfShapeTextIsDrawnWhereLibreOfficeDrawsIt(string fileName)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(fileName);
        List<PdfTextRun> ours = PdfTextRuns.Read(Ours(path));
        List<PdfTextRun> theirs = PdfTextRuns.Read(
            _libreOffice.ConvertToPdf(path, _workDirectory));

        ours.Count.ShouldBe(theirs.Count, $"{fileName}: number of drawn text runs");

        for (int i = 0; i < theirs.Count; i++)
        {
            PdfTextRun mine = ours[i];
            PdfTextRun reference = theirs[i];
            string where = $"{fileName}: run {i + 1} on slide {reference.PageIndex + 1}";

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
    /// The fills that are shapes rather than the sheet.
    /// </summary>
    /// <remarks>
    /// Both writers paint the slide background as a full-page rectangle, and both are excluded
    /// here — not to hide a difference but because the page rectangle is the one fill whose size
    /// is the export's rounding rather than the document's, and comparing it would assert on
    /// LibreOffice's clip rather than on ours. A fill covering more than 95% of the sheet in both
    /// directions is the sheet.
    /// </remarks>
    private static List<PdfFill> ShapeFills(List<PdfFill> fills)
        => [.. fills.Where(fill => fill.Width < 600 || fill.Height < 450)];

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
