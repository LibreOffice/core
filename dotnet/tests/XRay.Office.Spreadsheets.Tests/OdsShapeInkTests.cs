using XRay.Office.Core.Documents;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// The ODF half of a worksheet shape's fill and outline, and the page it was costing.
/// </summary>
/// <remarks>
/// <para>
/// <c>features/sheet-shape-ink.ods</c> is LibreOffice 26.2.4.2's own conversion of
/// <c>sheet-shape-ink.xlsx</c>, so both readers are answering for the same five shapes and every
/// divergence between them is ours. The ODF file states each shape's ink on its
/// <c>draw:style-name</c> graphic style — <c>draw:fill</c>, <c>draw:fill-color</c>,
/// <c>draw:stroke</c>, <c>svg:stroke-color</c>, <c>svg:stroke-width</c> — and its geometry as a
/// <c>draw:enhanced-geometry</c> whose <c>draw:type</c> keeps the DrawingML preset's name under
/// an <c>ooxml-</c> prefix.
/// </para>
/// <para>
/// <strong>The page count is the half no colour census could have found.</strong> 26.2.4.2 prints
/// this file on <b>two</b> pages, the second of them empty, because the rightmost object on the
/// sheet is the bare box — a rectangle stating <c>draw:fill="none"</c> and
/// <c>draw:stroke="none"</c> — and <c>ScDrawLayer::GetPrintArea</c> widens the printed block to
/// cover every object on the draw page, its only exclusion being the hidden-comment layer
/// (<c>sc/source/core/data/drwlayer.cxx</c>:1397-1414). The reader answered null for a shape with
/// neither text nor a picture, so that object was not in the model at all and this file printed on
/// one page. The <c>.xlsx</c> twin was already two of two, because an <c>xdr:sp</c> has always
/// produced a drawing whatever it holds.
/// </para>
/// <para>
/// Against 26.2.4.2's own marks this tree draws all five to <b>0.07 pt</b>, and closer in x than
/// the <c>.xlsx</c> path manages — 242.62 against 242.59 — because ODF states the rectangle
/// outright where SpreadsheetML states cells and offsets.
/// </para>
/// </remarks>
public sealed class OdsShapeInkTests
{
    private const string Fixture = "sheet-shape-ink.ods";

    private static SpreadsheetPages Pages()
    {
        using IPaginatedDocument document =
            (IPaginatedDocument)OfficeDocument.Open(Corpus.Require(Fixture));

        return (SpreadsheetPages)document.Layout();
    }

    private static SheetDrawings Drawings() => Pages().Sheets[0].Drawings;

    private static SheetDrawing Named(string name)
        => Drawings().Items.SingleOrDefault(drawing => drawing.Name == name)
           .ShouldNotBeNull($"a drawing named {name}");

    /// <summary>The graphic style's fill, stroke colour and stroke width are all read.</summary>
    /// <remarks>
    /// <c>svg:stroke-width</c> is a length rather than DrawingML's EMU count, and 2.25 pt is what
    /// the converter wrote for the <c>a:ln w="28575"</c> of the shape's <c>.xlsx</c> original.
    /// Censused over the 307 converted <c>.ods</c>: <b>589 shapes state
    /// <c>draw:fill="solid"</c></b> and 490 <c>draw:stroke="solid"</c>.
    /// </remarks>
    [Fact]
    public void AShapesGraphicStyleCarriesItsFillAndItsOutline()
    {
        SheetDrawing star = Named("Inked Star");

        star.Fill.ShouldBe(Colour.FromRgb(0xFF0000));
        star.Stroke.ShouldBe(Colour.FromRgb(0x0000FF));
        star.StrokeWidth.Points.ShouldBe(2.25, 0.02);
        star.Preset.ShouldBe("star5", "the ooxml- prefix is stripped off draw:type");
    }

    /// <summary>
    /// A style stating <c>none</c> for both is not ink, and the shape is still on the sheet.
    /// </summary>
    /// <remarks>
    /// An absent <c>draw:fill</c> is not a fill either — the drawing layer's pool default is a
    /// solid blue that no ODF file means, and LibreOffice's exporter states the property on every
    /// shape it writes.
    /// </remarks>
    [Fact]
    public void AShapeStatingNeitherIsUninkedAndStillDrawn()
    {
        SheetDrawing bare = Named("Bare Box");

        bare.Fill.ShouldBeNull();
        bare.Stroke.ShouldBeNull();
        bare.HasInk.ShouldBeFalse();
        bare.Text.ShouldBeNull();
    }

    /// <summary>
    /// Every <c>draw:</c> shape reaches the model, ink or none, because every object widens the
    /// printed block.
    /// </summary>
    /// <remarks>
    /// The converter flattens the group into its two ellipses, so the six are the four top-level
    /// shapes and those two: ODF gives a group no rectangle of its own and each child carries
    /// absolute coordinates in the anchor's space.
    /// </remarks>
    [Fact]
    public void EveryShapeIsInTheModelWhateverItHolds()
        => Drawings().Items.Count.ShouldBe(6);

    /// <summary>The bare box is the rightmost object, and it keeps the second page alive.</summary>
    /// <remarks>
    /// The measurement this whole half rests on: 26.2.4.2 prints two pages and printed two before
    /// this round as well, while this tree printed one. Nothing about the text moves with it —
    /// the second page is empty — which is exactly why no gate column could see it.
    /// </remarks>
    [Fact]
    public void AnUninkedShapePastTheLastCellKeepsItsPage()
        => Pages().Pages.Count.ShouldBe(2);

    /// <summary>A converted group's leaves keep the fills they were given, resolved.</summary>
    /// <remarks>
    /// LibreOffice resolves <c>a:grpFill</c> on the way out, so the ODF file states the group's
    /// teal on the second ellipse outright and this reader never sees the reference. The two
    /// colours are the check that the flattening did not lose one.
    /// </remarks>
    [Fact]
    public void AConvertedGroupsLeavesKeepTheirOwnFills()
    {
        List<Colour?> fills = [.. Drawings().Items
            .Where(drawing => drawing.Preset == "ellipse")
            .Select(drawing => drawing.Fill)];

        fills.Count.ShouldBe(2);
        fills.ShouldContain(Colour.FromRgb(0xFFA500));
        fills.ShouldContain(Colour.FromRgb(0x00A0A0));
    }
}
