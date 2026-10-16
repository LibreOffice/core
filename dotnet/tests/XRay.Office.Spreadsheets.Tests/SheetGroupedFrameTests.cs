using XRay.Office.Core.Documents;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A picture on an ODF sheet may be wrapped, and it may be turned; neither is visible to a walk
/// of a cell's own <c>draw:frame</c> children.
/// </summary>
/// <remarks>
/// <para>
/// Three shapes stand between a <c>table:table-cell</c> and its pictures. A <c>draw:g</c> groups
/// them, a <c>draw:a</c> hyperlinks one — ODF states a shape's link as a wrapper element rather
/// than as an attribute — and a turned frame carries a <c>draw:transform</c> in place of
/// <c>svg:x</c>/<c>svg:y</c> altogether. Neither wrapper has a rectangle of its own, so a child's
/// coordinates are measured from the anchor cell whether or not it sits in one.
/// </para>
/// <para>
/// Censused over the 307 converted <c>.ods</c>: <strong>89 frames sit inside a <c>draw:g</c></strong>
/// — 72 of them in <c>SIL_TDB648.ods</c>, which 26.2.4.2 renders as 88 pages against the 60 this
/// tree drew without them, because the pictures widen the print area
/// (<c>ScDrawLayer::GetPrintArea</c>, <c>sc/source/core/data/drwlayer.cxx</c>:1400-1424) and keep
/// pages that hold nothing else (<c>ScDocument::HasAnyDraw</c>, <c>documen9.cxx</c>:382-404) —
/// and <strong>33 inside a <c>draw:a</c></strong>, across thirteen documents.
/// </para>
/// <para>
/// <strong>The group is not read as a drawing.</strong> ODF gives it no rectangle; the
/// <c>table:end-cell-address</c> LibreOffice writes on one is its cached bounding anchor for the
/// union of its children, which <see cref="SheetDrawingArea"/> already takes from the children
/// themselves.
/// </para>
/// <para>
/// Every figure below is LibreOffice 26.2.4.2's own, read out of its PDF of
/// <c>sheet-grouped-frames.fods</c> with PyMuPDF: the three pictures are drawn at
/// (74.69, 74.67)–(146.69, 110.67), (74.69, 182.67)–(146.69, 218.67) and
/// (128.69, 272.67)–(289.39, 407.03) points from the page's top-left.
/// </para>
/// </remarks>
public sealed class SheetGroupedFrameTests
{
    private static SheetDrawings Read(string name)
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require(name));

        document.Layout();
        return ((SpreadsheetPages)document.Layout()).Sheets[0].Drawings;
    }

    /// <summary>All three pictures are found, each anchored to its own row.</summary>
    /// <remarks>
    /// The grouped and the linked frames state a plain <c>svg:x</c>, so this is the wrapper alone:
    /// before it they were not in the model at all, and nothing about the page said so.
    /// </remarks>
    [Fact]
    public void AWrappedFrameIsFoundAndKeepsItsAnchor()
    {
        IReadOnlyList<SheetDrawing> drawings = Read("sheet-grouped-frames.fods").Items;

        drawings.Count.ShouldBe(3);
        drawings.Select(d => d.Name).ShouldBe(["InGroup", "InLink", "Turned"]);

        SheetDrawing grouped = drawings.Single(d => d.Name == "InGroup");
        SheetDrawing linked = drawings.Single(d => d.Name == "InLink");

        grouped.From.Row.ShouldBe(0);
        linked.From.Row.ShouldBe(1);
        grouped.From.ColumnOffset.Inches.ShouldBe(0.25, 0.001);
        linked.From.RowOffset.Inches.ShouldBe(0.25, 0.001);
        grouped.Extent.Width.Inches.ShouldBe(1.0, 0.001);
    }

    /// <summary>
    /// A turned frame's rectangle is its bounding box, and the picture inside keeps its angle.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The fixture's third frame is 2 in × 1 in turned through <c>rotate (0.5235987755982988)</c>,
    /// which is 30°, and translated by an inch each way. The bounding box of that is
    /// <c>2·cos30 + 1·sin30 = 2.2321 in</c> across and <c>2·sin30 + 1·cos30 = 1.8660 in</c> down,
    /// and 26.2.4.2 draws it 160.70 × 134.36 pt — the same two numbers.
    /// </para>
    /// <para>
    /// The box rather than the shape, because the box is what Calc's two page-deciding questions
    /// ask for; the shape's own size and angle ride inside it as a
    /// <see cref="SheetDrawingPart"/>, which is the same shape the SpreadsheetML reader produces
    /// for a grouped, turned watermark.
    /// </para>
    /// <para>
    /// <strong>The reference does not turn the picture at draw time — it turns the pixels.</strong>
    /// 26.2.4.2's content stream places this one with <c>160.696 0 0 134.362 128.693 434.862 cm</c>,
    /// an axis-aligned matrix over a bitmap it has already rotated into the bounding box, where we
    /// emit a rotation and then the shape's own 143.99 × 71.99 box inside it. The ink is the same
    /// and the box is the same; only the representation differs, which matters if a later round
    /// compares content streams rather than pages.
    /// </para>
    /// </remarks>
    [Fact]
    public void ATurnedFrameIsPlacedByItsTransform()
    {
        SheetDrawing turned = Read("sheet-grouped-frames.fods").Items.Single(d => d.Name == "Turned");

        turned.From.Row.ShouldBe(2);
        turned.Extent.Width.Inches.ShouldBe(2.2321, 0.002);
        turned.Extent.Height.Inches.ShouldBe(1.8660, 0.002);

        // The translate is where the shape's own top-left corner lands, and the box grows from it
        // in whichever directions the turn takes the other three: rightwards and *upwards* here,
        // so the box's left edge is the translate exactly and its top edge is an inch above it.
        turned.From.ColumnOffset.Inches.ShouldBe(1.0, 0.002);
        turned.From.RowOffset.Inches.ShouldBe(0.0, 0.002);

        // Negative because SheetDrawingPart counts clockwise and ODF's angle is the other way —
        // the same sense the ODP reader takes, and the reason OdfTransform.Rotation is not a
        // plain rotation matrix. Confirmed against the reference by rendering an asymmetric
        // picture: its black quadrant is in the same corner of the parallelogram on both sides.
        SheetDrawingPart part = turned.Parts.ShouldHaveSingleItem();
        part.Degrees.ShouldBe(-30.0, 0.01);
        part.HasPicture.ShouldBeTrue();
        (part.Width * turned.Extent.Width.Inches).ShouldBe(2.0, 0.002);
        (part.Height * turned.Extent.Height.Inches).ShouldBe(1.0, 0.002);
    }

    /// <summary>
    /// The control: an untransformed frame carries no part, so nothing about the ordinary path
    /// moved.
    /// </summary>
    /// <remarks>
    /// A part exists only to carry an angle. Giving one to every frame would send every sheet
    /// picture through <see cref="SheetDrawingBounds"/>' union and
    /// <c>SheetPageGraphics.DrawParts</c> instead of the direct paint, which is a change to every
    /// workbook in the corpus rather than to the five that state a transform.
    /// </remarks>
    [Fact]
    public void AnUnturnedFrameCarriesNoPart()
        => Read("sheet-grouped-frames.fods").Items
            .Where(d => d.Name != "Turned")
            .ShouldAllBe(d => d.Parts.Count == 0 && d.Image != null);
}
