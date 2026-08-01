using Paperless.Core.Documents;
using Paperless.Rendering.Pdf;
using Paperless.TestKit;
using Paperless.TestKit.LibreOffice;
using Shouldly;

namespace Paperless.Fidelity.Tests;

/// <summary>
/// Compares where a picture anchored to a cell lands with where LibreOffice puts it.
/// </summary>
/// <remarks>
/// <para>
/// The assertion worth having is the <em>rectangle</em>, because everything the anchor does shows
/// up in it. A two-cell anchor states two cells and two offsets and the picture spans whatever
/// lies between them, so the width is the sum of the columns it crosses less its own start offset
/// — a reader that took the frame's stated size instead would be right on a file it wrote itself
/// and wrong on one whose grid has changed. The corpus document is exactly that case: its frame
/// states 1.28 in and ends at C3, and LibreOffice draws 1.3201 in.
/// </para>
/// <para>
/// <strong>The picture is compared and its pixels are not.</strong> Both renderers embed the
/// file's own PNG as an image XObject, so the placement is a fair comparison and the samples are
/// the same bytes on both sides by construction. What a codec makes of them is
/// <c>Paperless.Rendering</c>'s business and is tested there.
/// </para>
/// </remarks>
public sealed class SheetDrawingComparisonTests : IDisposable
{
    /// <summary>A tenth of a point, two twips, as everywhere else in this project.</summary>
    private const double TolerancePoints = 0.1;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("paperless-sheet-drawing").FullName;

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
    [InlineData("sheet-rich-text.fods")]
    [InlineData("sheet-rich-text.xlsx")]
    public void APictureIsDrawnWhereLibreOfficeDrawsIt(string name)
    {
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, "LibreOffice is not installed");

        string path = Corpus.Require(name);
        List<PdfImageDraw> ours = PdfPaints.ReadImageDraws(Ours(path));
        List<PdfImageDraw> theirs = PdfPaints.ReadImageDraws(
            _libreOffice.ConvertToPdf(path, _workDirectory));

        Assert.SkipWhen(theirs.Count == 0, "no image placements could be read from the reference");

        ours.Count.ShouldBe(theirs.Count, $"{name}: number of pictures drawn");

        for (int i = 0; i < theirs.Count; i++)
        {
            PdfImageDraw mine = ours[i];
            PdfImageDraw reference = theirs[i];
            string where = $"{name}: picture {i + 1} on page {reference.PageIndex + 1}";

            mine.PageIndex.ShouldBe(reference.PageIndex, $"{where}: page");
            mine.Box.Left.ShouldBe(reference.Box.Left, TolerancePoints, $"{where}: left");
            mine.Box.Top.ShouldBe(reference.Box.Top, TolerancePoints, $"{where}: top");

            // The width is the two-cell span and the height the row's, so between them they cover
            // both axes of the anchor arithmetic.
            mine.Box.Width.ShouldBe(reference.Box.Width, TolerancePoints, $"{where}: width");
            mine.Box.Height.ShouldBe(reference.Box.Height, TolerancePoints, $"{where}: height");
        }
    }

    [Theory]
    [InlineData("sheet-rich-text.fods")]
    [InlineData("sheet-rich-text.xlsx")]
    public void APictureIsDrawnOnItsOwnSheetAndOnNoOther(string name)
    {
        string path = Corpus.Require(name);
        List<PdfImageDraw> ours = PdfPaints.ReadImageDraws(Ours(path));

        // Needs no LibreOffice. The corpus document's picture is on its third sheet, so a reader
        // that anchored it to the wrong sheet, or a page that drew every sheet's drawings, shows
        // here and nowhere else — the two text sheets carry no picture at all.
        ours.Count.ShouldBe(1, $"{name}: exactly one picture is drawn");
        ours[0].PageIndex.ShouldBe(2, $"{name}: on the sheet that holds it");
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
