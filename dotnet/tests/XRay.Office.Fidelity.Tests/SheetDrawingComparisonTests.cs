using XRay.Office.Core.Documents;
using XRay.Office.Rendering.Pdf;
using XRay.Office.TestKit;
using XRay.Office.TestKit.LibreOffice;
using Shouldly;

namespace XRay.Office.Fidelity.Tests;

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
/// <c>XRay.Office.Rendering</c>'s business and is tested there.
/// </para>
/// </remarks>
// [26.2.4.2 clamps a full-cell anchor offset, classified as LibreOffice's, not closed]
// APictureIsDrawnWhereLibreOfficeDrawsIt fails on `sheet-rich-text.xlsx`, picture 1 on page 3, on
// width and height. The previous note here blamed the derived grid — "the span is a sum of grid
// extents and the grid's own units are character widths and font-derived row heights" — and that
// is **refuted**: the picture's height is `rowOff` 640080 less `rowOff` 45720 *within one row*, so
// it is 46.800 pt of pure EMU arithmetic with no font in it, and it moves by the same amount as
// the width. Both binaries also compute the same grid; their flat-ODF exports of this workbook
// agree column for column and row for row.
//
// What 26.2.4.2 does, measured over 34 probe renderings in `probes/sheets-anchor-clamp`: it clamps
// an in-cell anchor offset to `cellSize - 5` (1/100 mm), on both axes, and 24.2.7.2 clamps nothing
// at all — its drawn offset is `round(EMU / 360) - 1` even at 1.4x the cell's own width. The
// clamp is `ShapeAnchor::calcCellAnchorEmu` (`sc/source/filter/oox/drawingbase.cxx`), whose stated
// intent is "reduce cell's right edge by a full twip"; a full twip is 635 EMU and would give
// `cellSize - 3`, so the magnitude the binary applies is not the one its own source describes.
//
// [2026-09-06] And it is not the advance divergence either — `CLAUDE.md` listed this method under
// rule 3's reach and that was stale when it was written; rule 3 is now withdrawn outright. See
// `probes/advance-ppem/results.md`.
//
// Classified as LibreOffice's rather than ours, and deliberately not reproduced. It fires on a
// valid anchor — `colOff` equal to the cell extent is how a picture snapped to a column edge is
// written — and shrinks the picture below the size its own `a:ext cx="1207080" cy="594360"`
// states; 26.2.4.2 draws the *same picture* at the full 95.046 x 46.800 pt from the FODS spelling,
// 24.2.7.2 draws the XLSX at 95.017 x 46.772, and the tree's own C++ would give a third answer
// again. We draw 95.074 x 46.800, which is the anchor arithmetic the file states.
//
// Reach of the gap: 0.113 pt on a far edge, on an XLSX drawing whose `to` offset reaches the last
// 5/100 mm of its end cell. It moves no gate column and clears every other comparison's tolerance;
// this one asserts at 0.1 pt and misses by 0.070 pt across and 0.042 pt down.
public sealed class SheetDrawingComparisonTests : IDisposable
{
    /// <summary>A tenth of a point, two twips, as everywhere else in this project.</summary>
    private const double TolerancePoints = 0.1;

    private readonly LibreOfficeRunner _libreOffice = new();
    private readonly string _workDirectory =
        Directory.CreateTempSubdirectory("xray-office-sheet-drawing").FullName;

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
        Assert.SkipUnless(LibreOfficeRunner.IsAvailable, LibreOfficeRunner.UnavailableReason);

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
            _workDirectory, $"{Path.GetFileNameWithoutExtension(documentPath)}-xray-office.pdf");

        using IDocument document = OfficeDocument.Open(documentPath);
        IPageSequence pages = ((IPaginatedDocument)document).Layout();

        using FileStream output = File.Create(destination);
        new PdfRenderer(new PdfRenderOptions
        {
            CreationDate = new DateTimeOffset(2024, 1, 1, 0, 0, 0, TimeSpan.Zero),
        }).Render(pages, output);

        return destination;
    }
}
