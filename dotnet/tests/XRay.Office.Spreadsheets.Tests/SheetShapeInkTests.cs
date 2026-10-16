using XRay.Office.Core.Documents;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A worksheet shape's fill and outline, which were read in no format at all.
/// </summary>
/// <remarks>
/// <para>
/// Every <c>xdr:sp</c> on every worksheet was drawn as bare text over whatever was under it.
/// Censused whole-corpus (<c>probes/sheet-fill-r84/census.py</c>): <b>644 worksheet shapes in 49
/// documents</b>, of which 421 state an <c>a:solidFill</c> of their own, 205 an
/// <c>a:ln/a:solidFill</c> and 583 an <c>xdr:style</c> naming the theme's format matrix — and
/// <b>no gate column can see one</b>, because a fill adds no glyph and no page.
/// </para>
/// <para>
/// <c>features/sheet-shape-ink.xlsx</c> is built by
/// <c>probes/sheet-fill-r84/make-fixture.py</c> and holds five shapes, one per arm of the rule.
/// LibreOffice <b>26.2.4.2</b>'s own PDF of it, read with PyMuPDF's <c>get_drawings()</c>, is
/// <b>two pages</b> and these five marks on the first:
/// </para>
/// <code>
///   fs    53.80 109.22 197.77 217.19   fill #FF0000  stroke #0000FF  w 2.239   the star
///   fs   242.59 109.22 386.59 181.22   fill #4472C4  stroke #4472C4  w 2.013   the themed box
///   s     53.83 198.82 233.83 234.82                 stroke #008000  w 1.502   the turned elbow
///   f    242.59 198.82 314.59 270.82   fill #FFA500                            the group's left
///   f    314.59 198.82 386.59 270.82   fill #00A0A0                            its a:grpFill twin
/// </code>
/// <para>
/// Four things are established by those five lines and the shape that is <em>not</em> among them.
/// The star's own fill and its own wider line are honoured. The themed box states no fill and no
/// line at all and is painted in <c>accent1</c> at the theme's <em>second</em> line style width,
/// 25400 EMU — neither number appears in the sheet's markup. The bare box states
/// <c>a:noFill</c> and <c>a:ln/a:noFill</c> under the same style reference and is drawn nowhere,
/// so the shape's own statement beats the matrix. The elbow is turned a quarter and its anchor
/// describes the turned rectangle, so it comes out 180 pt wide where its <c>a:ext</c> is 36. And
/// the group's second child says <c>a:grpFill</c> and takes the group's own teal.
/// </para>
/// <para>
/// This tree reproduces all five to <b>0.06 pt</b> in x and <b>0.14 pt</b> in y, and both page
/// counts. What it does not reproduce is the reference's shadows, which the fixture states none
/// of; see the round's write-up for the corpus documents that do.
/// </para>
/// </remarks>
public sealed class SheetShapeInkTests
{
    private const string Fixture = "sheet-shape-ink.xlsx";

    private static SheetDrawings Drawings()
    {
        using IPaginatedDocument document =
            (IPaginatedDocument)OfficeDocument.Open(Corpus.Require(Fixture));

        return ((SpreadsheetPages)document.Layout()).Sheets[0].Drawings;
    }

    private static SheetDrawing Named(string name)
        => Drawings().Items.SingleOrDefault(drawing => drawing.Name == name)
           .ShouldNotBeNull($"a drawing named {name}");

    /// <summary>A shape's own <c>a:solidFill</c> and <c>a:ln</c> are read, with the line's width.</summary>
    /// <remarks>
    /// The width matters as much as the colour: <c>a:ln/@w</c> is in EMUs and a shape stating
    /// none takes the theme's, so a reader that defaults it to zero draws every outline as a
    /// hairline. 28575 EMU is 2.25 pt, which is what 26.2.4.2 puts in the PDF.
    /// </remarks>
    [Fact]
    public void AShapeStatesItsOwnFillAndItsOwnOutline()
    {
        SheetDrawing star = Named("Inked Star");

        star.Fill.ShouldBe(Colour.FromRgb(0xFF0000));
        star.Stroke.ShouldBe(Colour.FromRgb(0x0000FF));
        star.StrokeWidth.ShouldBe(Length.FromEmu(28575));
        star.Text.ShouldBeNull("the shape carries no text at all");
        star.HasInk.ShouldBeTrue();
    }

    /// <summary>The outline is the preset's, not the anchor's rectangle.</summary>
    /// <remarks>
    /// 438 of the corpus's 644 worksheet shapes name a preset that is not <c>rect</c>, so this is
    /// the common case rather than a corner. <c>rect</c> is deliberately answered as null,
    /// because it evaluates to the box the painter's fallback already draws.
    /// </remarks>
    [Fact]
    public void AShapeCarriesThePresetItsInkIsPaintedThrough()
    {
        Named("Inked Star").Preset.ShouldBe("star5");
        Named("Bare Box").Preset.ShouldBeNull("rect is the box the fallback already paints");
    }

    /// <summary>
    /// A shape stating nothing takes both its fill and its line from the theme's format matrix.
    /// </summary>
    /// <remarks>
    /// <c>a:fillRef idx="1"</c> over <c>accent1</c> resolves the theme's first fill style, whose
    /// only colour is the <c>phClr</c> placeholder the reference substitutes; <c>a:lnRef
    /// idx="2"</c> takes the second line style, which is where the 25400 EMU comes from. Neither
    /// value is in the drawing part. <b>86 of the corpus's 644 shapes really depend on this</b> —
    /// the other 497 styled shapes state a fill of their own, which wins.
    /// </remarks>
    [Fact]
    public void AStyleReferenceSuppliesBothTheFillAndTheLineWidth()
    {
        SheetDrawing themed = Named("Themed Box");

        themed.Fill.ShouldBe(Colour.FromRgb(0x4472C4));
        themed.Stroke.ShouldBe(Colour.FromRgb(0x4472C4));
        themed.StrokeWidth.ShouldBe(Length.FromEmu(25400));
    }

    /// <summary>
    /// <c>a:noFill</c> beats the matrix, and so does <c>a:ln/a:noFill</c>.
    /// </summary>
    /// <remarks>
    /// The control for the test above, and the one that decides whether the feature can be shipped
    /// at all: 29 corpus shapes state <c>a:noFill</c> and painting them would put a coloured box
    /// over the cells under each. 26.2.4.2 draws no mark for this shape.
    /// </remarks>
    [Fact]
    public void ShapeSuppressingBothUnderAStyleReferenceHasNeither()
    {
        SheetDrawing bare = Named("Bare Box");

        bare.Fill.ShouldBeNull();
        bare.Gradient.ShouldBeNull();
        bare.Stroke.ShouldBeNull();
        bare.HasInk.ShouldBeFalse();
    }

    /// <summary>
    /// A shape stating nothing at all still keeps its place in the printed block.
    /// </summary>
    /// <remarks>
    /// It is the reason the fixture is two pages rather than one: the bare box is the rightmost
    /// object on the sheet and <c>ScDrawLayer::GetPrintArea</c> counts every object on the draw
    /// page, whatever it holds (<c>sc/source/core/data/drwlayer.cxx</c>:1397-1414). The
    /// SpreadsheetML reader has always kept such a shape; the ODF one did not, which is the
    /// other half of this round.
    /// </remarks>
    [Fact]
    public void AnUninkedShapeIsStillOnTheSheet()
        => Drawings().Items.Count.ShouldBe(5, "four shapes and a group, none of them dropped");

    /// <summary>
    /// A leaf of a group carries its own fill, and <c>a:grpFill</c> takes the group's.
    /// </summary>
    /// <remarks>
    /// <b>174 of the 644 corpus worksheet shapes sit inside a group</b>, in 6 documents, and
    /// <b>98 state <c>a:grpFill</c></b>, in 2. The anchor carries one fill and a group has many,
    /// so the ink goes on <see cref="SheetDrawingPart"/> — which is also where a turned shape's
    /// ink has to go, a part being the only thing that carries an angle.
    /// </remarks>
    [Fact]
    public void AGroupsLeavesCarryTheirOwnFillAndInheritItsOwn()
    {
        SheetDrawing pair = Named("Pair");

        pair.Parts.Count.ShouldBe(2);
        pair.Parts[0].Fill.ShouldBe(Colour.FromRgb(0xFFA500));
        pair.Parts[1].Fill.ShouldBe(
            Colour.FromRgb(0x00A0A0), "a:grpFill is the enclosing group's fill");
        pair.Parts[1].Preset.ShouldBe("ellipse");
    }

    /// <summary>
    /// A shape turned through a quarter has an anchor describing the rectangle it ends up in.
    /// </summary>
    /// <remarks>
    /// Excel rewrites the anchor cells of a shape rotated into <c>[45°, 135°)</c> or
    /// <c>[225°, 315°)</c>, so Calc reflects the rectangle in <c>y = x</c> before drawing
    /// (<c>sc/source/filter/oox/drawingfragment.cxx</c>:299-330). Without it an elbow connector's
    /// short leg points away from the box it joins — visible on
    /// <c>016_Free_Organizational_Chart_Template</c>, where three of the fifteen connectors ran
    /// off the top of the page. <b>Reach: 31 shapes in 3 corpus documents.</b>
    /// </remarks>
    [Fact]
    public void AQuarterTurnedShapesAnchorIsReflected()
    {
        SheetDrawing elbow = Named("Turned Elbow");

        elbow.QuarterTurnedAnchor.ShouldBeTrue();
        elbow.Fill.ShouldBeNull("a connector has no fill");
        elbow.Parts.ShouldHaveSingleItem().Degrees.ShouldBe(90);
        elbow.Parts[0].Stroke.ShouldBe(Colour.FromRgb(0x008000));
    }
}
