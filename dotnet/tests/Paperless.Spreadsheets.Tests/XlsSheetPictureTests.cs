using Paperless.Core.Documents;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// The pictures a legacy workbook anchors on its sheets.
/// </summary>
/// <remarks>
/// <para>
/// <c>vector-picture-sheet.xls</c> is LibreOffice 24.2.7.2's own <c>MS Excel 97</c> export of the
/// ODS the sibling tests read, so the three pictures are the same three and only the container has
/// changed: a WMF, an EMF and — the SVG having no BIFF representation — a raster. They arrive as
/// three <c>msofbtBSE</c> entries in the workbook's one <c>MSODRAWINGGROUP</c>, which is the whole
/// point. Excel keeps its blip store inline in the globals where Word keeps it in the table stream
/// and PowerPoint in a <c>PPDrawingGroup</c>, and the store is workbook-wide while the shapes that
/// index it are per sheet.
/// </para>
/// <para>
/// The defect this pins is not a wrong picture but no picture at all: <c>XlsDrawingCollector</c>
/// emitted a drawing only for a shape carrying <c>TXO</c> text, so a picture shape was walked, its
/// <c>OBJ</c> consumed, and then dropped. Reintroducing that — restoring the
/// <c>if (entry.Text is not { Length: &gt; 0 }) continue;</c> guard ahead of the picture lookup —
/// makes every assertion below fail, because the sheet then has no drawings on it whatsoever.
/// </para>
/// </remarks>
public class XlsSheetPictureTests
{
    [Fact]
    public void AWorkbookSPicturesReachTheSheetTheyAreAnchoredOn()
    {
        IReadOnlyList<SheetDrawing> drawings = Drawings();

        drawings.Count.ShouldBe(3, "the pictures anchored on the first sheet");

        foreach (SheetDrawing drawing in drawings)
        {
            (drawing.Image is not null || drawing.Vector is not null)
                .ShouldBeTrue($"{drawing.Name}: carries a picture");
        }
    }

    [Fact]
    public void TheTwoMetafilesStayMetafilesAndTheThirdStaysARaster()
    {
        // The blip record's own type is not what decides this — `VectorImages.For` is, from the
        // bytes, exactly as the package path decides it. An Escher store is where the two are
        // easiest to confuse, because a metafile blip carries a thirty-four byte
        // `OfficeArtMetafileHeader` and a raster blip a single tag byte, and reading one as the
        // other puts header bytes in front of a signature.
        IReadOnlyList<SheetDrawing> drawings = Drawings();

        drawings.Count(d => d.Vector is not null).ShouldBe(2, "the WMF and the EMF");
        drawings.Count(d => d.Image is not null && d.Vector is null).ShouldBe(1, "the raster");
    }

    [Fact]
    public void NothingIsDecodedUntilSomethingAsksForThePicture()
    {
        // The same property the package path holds, and it matters more here: a `.xls` is read for
        // its cell values far more often than for its pictures, and inflating a metafile costs the
        // font stack's start-up on a caller that never draws a page.
        //
        // Counted before it is walked, deliberately. A `foreach` over the drawings asserts nothing
        // when there are none, which is exactly the state the defect this file pins produces — so
        // without the count this test passes with every picture dropped.
        IReadOnlyList<SheetDrawing> drawings = Drawings();
        drawings.Count(d => d.Vector is not null).ShouldBe(2);

        foreach (SheetDrawing drawing in drawings)
        {
            if (drawing.Vector is { } vector) vector.IsValueCreated.ShouldBeFalse();
        }
    }

    [Fact]
    public void EveryMetafileDecodesToSomethingWithInkInIt()
    {
        IReadOnlyList<SheetDrawing> drawings = Drawings();
        drawings.Count(d => d.Vector is not null).ShouldBe(2);

        foreach (SheetDrawing drawing in drawings)
        {
            if (drawing.Vector is not { } vector) continue;

            vector.Value.IsEmpty.ShouldBeFalse($"{drawing.Name}");
            vector.Value.ViewBox.IsEmpty.ShouldBeFalse($"{drawing.Name}");
        }
    }

    [Fact]
    public void APictureIsAnchoredWhereTheOdsPutsItRatherThanAtTheSheetSOrigin()
    {
        // A picture shape reaches the page through the same eighteen-byte client anchor a text box
        // does, so this asserts the two halves are wired to each other rather than a picture being
        // produced with nothing to place it by. The first drawing sits below and right of the
        // sheet's first cell in both formats.
        SheetDrawing first = Drawings()[0];

        first.Anchor.ShouldBe(SheetAnchorKind.TwoCell);
        (first.To.Column > first.From.Column || first.To.Row > first.From.Row)
            .ShouldBeTrue("the anchor spans cells rather than collapsing to a point");
    }

    private static IReadOnlyList<SheetDrawing> Drawings()
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require("vector-picture-sheet.xls"));

        return ((SpreadsheetPages)document.Layout()).Sheets[0].Drawings.Items;
    }
}
