using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Paperless.Vector;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// The vector pictures anchored on <c>vector-picture-sheet</c>, in both formats.
/// </summary>
/// <remarks>
/// <para>
/// A WMF, an EMF and an SVG anchored to three cells, read as ODS and as LibreOffice's own XLSX
/// export of the same source. The XLSX is what settles the sniffing argument in its sharpest form:
/// LibreOffice writes the EMF into <c>xl/media/image2.wmf</c>, so the part name says one format and
/// the bytes say another, and only the bytes are believed.
/// </para>
/// <para>
/// A spreadsheet is also where the deferred decode matters most. A workbook is opened for its cell
/// values far more often than for its pictures, and nothing here should decode a metafile until a
/// page is drawn — which is what <see cref="Lazy{T}"/> on the drawing states and what the first
/// test below relies on.
/// </para>
/// </remarks>
public class SheetVectorPictureTests
{
    public static TheoryData<string> BothFormats => ["vector-picture-sheet.ods", "vector-picture-sheet.xlsx"];

    [Theory]
    [MemberData(nameof(BothFormats))]
    public void AllThreeDrawingsAreVectorsAndNoneIsARaster(string name)
    {
        IReadOnlyList<SheetDrawing> drawings = Drawings(name);

        drawings.Count.ShouldBe(3, $"{name}: drawings on the sheet");

        foreach (SheetDrawing drawing in drawings)
        {
            drawing.Vector.ShouldNotBeNull($"{name}: {drawing.Name} decoded as a vector");
            drawing.Chart.ShouldBeNull();
        }
    }

    [Theory]
    [MemberData(nameof(BothFormats))]
    public void NothingIsDecodedUntilSomethingAsksForThePicture(string name)
    {
        // The property that keeps `paperless extract` cheap: reading the sheet resolves the part,
        // sniffs four bytes and stops. Measured on this tree, the first VectorImages.Decode in a
        // process costs 1044 ms for a WMF with one text run — nearly all of it resolving faces
        // through Paperless.Text — against 0.21 ms once warm. A caller after cell values must not
        // pay that, and IsValueCreated is the only thing that can say it did not.
        foreach (SheetDrawing drawing in Drawings(name))
        {
            drawing.Vector.ShouldNotBeNull().IsValueCreated.ShouldBeFalse();
        }
    }

    [Theory]
    [MemberData(nameof(BothFormats))]
    public void EveryPictureDecodesToSomethingWithInkInIt(string name)
    {
        foreach (SheetDrawing drawing in Drawings(name))
        {
            VectorImage image = drawing.Vector.ShouldNotBeNull().Value;

            image.IsEmpty.ShouldBeFalse($"{name}: {drawing.Name}");
            image.IsTruncated.ShouldBeFalse($"{name}: {drawing.Name}");
            image.ViewBox.IsEmpty.ShouldBeFalse($"{name}: {drawing.Name}");
        }
    }

    [Theory]
    [MemberData(nameof(BothFormats))]
    public void AMetafileIsStretchedByItsFrameRatherThanByItsInk(string name)
    {
        // The trap a reader hits first. The WMF's view box is 80 × 60 mm and its own drawing sits
        // inside that with margins, so drawn into a 40 × 30 mm anchor box the ink comes out
        // *smaller* than the box. Scaling the ink onto the box instead fills it edge to edge,
        // which looks plausible on any one picture and is wrong on every one of them.
        VectorImage wmf = Drawings(name)[0].Vector.ShouldNotBeNull().Value;

        wmf.ViewBox.Width.Millimetres.ShouldBe(80, 0.05);
        wmf.ViewBox.Height.Millimetres.ShouldBe(60, 0.05);

        PlacedDrawingSink life = new();
        wmf.Draw(life, new DocRect(Length.Zero, Length.Zero, wmf.ViewBox.Width, wmf.ViewBox.Height));

        PlacedDrawingSink half = new();
        wmf.Draw(half, new DocRect(
            Length.Zero, Length.Zero, Length.FromMillimetres(40), Length.FromMillimetres(30)));

        // The ink does not reach the edges of either box, which is what makes this measurable at
        // all: a picture whose ink filled its frame would look identical under both rules.
        life.Ink.X.Millimetres.ShouldBeGreaterThan(0);
        life.Ink.Width.Millimetres.ShouldBeLessThan(80);

        // And it halves exactly, because the destination is half the view box in both directions.
        half.Ink.X.Millimetres.ShouldBe(life.Ink.X.Millimetres / 2, 0.05);
        half.Ink.Width.Millimetres.ShouldBe(life.Ink.Width.Millimetres / 2, 0.05);
    }

    [Fact]
    public void TheXlsxPrefersTheSvgOverTheRasterFallbackBesideIt()
    {
        // `BlipReference.Choose` in its spreadsheet form. The third blip names a PNG on r:embed and
        // the SVG in the {96DAC541-…} extension; a reader that stopped at r:embed would draw the
        // fallback, which is fixed at one resolution and is the whole thing vector import avoids.
        SheetDrawing svg = Drawings("vector-picture-sheet.xlsx")[2];

        svg.Vector.ShouldNotBeNull().Value.IsEmpty.ShouldBeFalse();
        svg.Image.ShouldNotBeNull("the raster fallback is kept for an empty decode to fall back to");
    }

    [Fact]
    public void TheOdsHasNoFallbackToChooseBetween()
    {
        foreach (SheetDrawing drawing in Drawings("vector-picture-sheet.ods"))
        {
            drawing.Image.ShouldBeNull();
        }
    }

    private static IReadOnlyList<SheetDrawing> Drawings(string name)
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require(name));

        return ((SpreadsheetPages)document.Layout()).Sheets[0].Drawings.Items;
    }
}
