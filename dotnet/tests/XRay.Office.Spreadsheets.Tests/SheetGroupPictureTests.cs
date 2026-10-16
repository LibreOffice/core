using XRay.Office.Core.Documents;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A picture inside an <c>xdr:grpSp</c> is drawn, at its own place in the group's rectangle and at
/// its own opacity.
/// </summary>
/// <remarks>
/// <para>
/// <strong>It was read for its bounds and never painted.</strong> <c>XlsxDrawings.ReadAnchor</c>
/// looks for an <c>xdr:pic</c> directly under the anchor; a group is not one, so the anchor came
/// back with no image, no vector, no chart and no text, and <see cref="SheetPageGraphics"/>
/// skipped it — while <see cref="SheetDrawingArea"/> still counted it, so the group widened the
/// printed block and put nothing in it. Calc makes no such distinction:
/// <c>GroupShapeContext::createShapeContext</c> takes <c>sp</c>, <c>cxnSp</c>, <c>grpSp</c>,
/// <c>graphicFrame</c> and <c>pic</c> alike (<c>sc/source/filter/oox/drawingfragment.cxx:198</c>).
/// </para>
/// <para>
/// Measured on <c>SIL_TDB648.xlsx</c>, whose eleven sheet drawings each hold one group of fourteen
/// turned, faded copies of the same <c>Honeywell</c> wordmark: <c>pdfimages -list</c> counts that
/// image on 86 of the reference's 88 pages and on <b>none</b> of our 90.
/// </para>
/// <para>
/// The fixture holds one group of two pictures — one faded, one turned — and a loose picture
/// outside it. The loose one is the control: a reader that painted the group's parts twice, or
/// that lost the ordinary path while gaining the group one, would fail on it rather than on them.
/// LibreOffice 26.2.4.2 draws all three.
/// </para>
/// </remarks>
public sealed class SheetGroupPictureTests
{
    private const string Fixture = "sheet-group-picture.xlsx";

    /// <summary>The group's two leaf pictures reach the model as parts that carry an image.</summary>
    [Fact]
    public void AGroupsLeafPicturesReachTheModel()
    {
        IReadOnlyList<SheetDrawing> drawings = Drawings();

        drawings.Count.ShouldBe(2, "one group anchor and one loose picture");
        drawings[0].Image.ShouldBeNull("a group carries no picture of its own");
        drawings[0].Parts.Count(part => part.HasPicture)
            .ShouldBe(2, "and both of its leaves do");
    }

    /// <summary>Every one of them is painted, the loose picture included.</summary>
    [Fact]
    public void AllThreePicturesArePainted() => Painted().Count.ShouldBe(3);

    /// <summary>
    /// A leaf's <c>a:alphaModFix</c> is its own, and its sibling's absence of one is too.
    /// </summary>
    [Fact]
    public void ALeafIsPaintedAtItsOwnOpacity()
    {
        List<DrawnPicture> painted = Painted();

        painted.Count(picture => Math.Abs(picture.Opacity - 0.20) < 0.0005)
            .ShouldBe(1, "one leaf states amt=\"20000\" and nothing else does");
        painted.Count(picture => picture.Opacity >= 1).ShouldBe(2);
    }

    /// <summary>
    /// A leaf lands in its own part of the group's rectangle, not across the whole of it.
    /// </summary>
    /// <remarks>
    /// The first leaf is a quarter of the group's width and a quarter of its height, at its top
    /// left; a reader that ignored <c>a:chOff</c>/<c>a:chExt</c> and painted each leaf over the
    /// whole anchor would give both of them the group's own box.
    /// </remarks>
    [Fact]
    public void ALeafKeepsItsPlaceInsideTheGroup()
    {
        DrawnPicture faded = Painted().Single(picture => picture.Opacity < 0.5);
        DrawnPicture loose = Painted().MaxBy(picture => picture.Destination.X.Emu);

        faded.Destination.Width.ShouldBeLessThan(loose.Destination.Width * 2,
            "the leaf is a quarter of the group's width, not the whole of it");
        faded.Destination.Height.ShouldBeGreaterThan(XRay.Office.Core.Units.Length.Zero);
    }

    /// <summary>The cells under the group are still drawn.</summary>
    [Fact]
    public void TheCellsUnderItAreStillDrawn()
    {
        RecordingDrawingSink sink = Render();

        string.Concat(sink.Pages[0].Runs.Select(run => run.Run.Text))
            .ShouldContain("under the group");
    }

    private static IReadOnlyList<SheetDrawing> Drawings()
    {
        using IPaginatedDocument document =
            (IPaginatedDocument)OfficeDocument.Open(Corpus.Require(Fixture));

        return ((SpreadsheetPages)document.Layout()).Sheets[0].Drawings.Items;
    }

    private static List<DrawnPicture> Painted() => Render().Pages[0].Pictures;

    private static RecordingDrawingSink Render()
    {
        using IPaginatedDocument document =
            (IPaginatedDocument)OfficeDocument.Open(Corpus.Require(Fixture));

        RecordingDrawingSink sink = new();
        foreach (SheetPage page in ((SpreadsheetPages)document.Layout()).Pages) page.Draw(sink);
        return sink;
    }
}
