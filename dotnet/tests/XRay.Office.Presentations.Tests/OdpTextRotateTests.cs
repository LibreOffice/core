using XRay.Office.Core;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Presentations.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// A custom shape that turns its own text, which is how LibreOffice writes a wide text box that
/// arrived as a rotated shape.
/// </summary>
/// <remarks>
/// <para>
/// <c>draw:text-rotate-angle</c> on the <c>draw:enhanced-geometry</c> is the text's rotation
/// <em>inside</em> the shape, and it composes with the shape's own. It becomes the geometry
/// item's <c>TextRotateAngle</c> (<c>xmloff/source/draw/ximpcustomshape.cxx</c>:917), which
/// <c>SdrObjCustomShape::GetExtraTextRotation</c> (<c>svx/source/svdraw/svdoashp.cxx</c>:486-514)
/// answers and <c>ViewContactOfSdrObjCustomShape</c> applies to the text box before the shape's
/// matrix (<c>svx/source/sdr/contact/viewcontactofsdrobjcustomshape.cxx</c>:171-191). Read
/// nowhere, a quarter-turned shape's whole body is drawn on its side.
/// </para>
/// <para>
/// <strong>The padding does the swapping, and the layout box does not.</strong> That is the half
/// of this that a measurement had to settle: LibreOffice writes a turned shape's insets in the
/// text's own orientation, so a 2 cm wide shape carries a <em>negative</em> left and right
/// padding widening its text area past the shape and a tall positive top and bottom padding
/// narrowing it. <c>SdrTextObj::AdjustRectToTextDistance</c>
/// (<c>svx/source/svdraw/svdotext.cxx</c>:577-618) adds them to the anchor rectangle and
/// <c>TakeTextRect</c> takes that rectangle's width as the wrapping limit. Swapping the box's
/// two dimensions as well wraps the text at twice the room it has.
/// </para>
/// <para>
/// Reach: 38 <c>draw:text-rotate-angle</c> statements in 7 of the converted corpus's documents —
/// 90 in three decks and ±180 in four more, every one of them written by 26.2.4.2's own
/// exporter. On <c>schematicplaymar21.odp</c> and its twin it is the whole of a −348 character
/// page: 11 122 of the reference's 11 470 before, 11 472 after.
/// </para>
/// </remarks>
public class OdpTextRotateTests
{
    private static SlidePages Layout()
    {
        using IDocument document = new PresentationReader().Read(
            DocumentSource.FromFile(Corpus.Require("odp-text-rotate.fodp")));

        document.ShouldBeAssignableTo<IPaginatedDocument>();
        return (SlidePages)((IPaginatedDocument)document).Layout();
    }

    private static PlacedText TextOf(LaidOutSlide slide)
        => slide.Shapes.Select(shape => shape.Text).First(text => text is not null)!;

    /// <summary>
    /// The turned slide and the upright one draw the same words in the same places.
    /// </summary>
    /// <remarks>
    /// 26.2.4.2 draws them identically — the first line of each spans x 92.10…491.02 and
    /// 92.13…491.04 at y 97.74 — so the pair is its own control and needs no stored geometry.
    /// </remarks>
    [Fact]
    public void ATurnedShapeDrawsItsTextExactlyWhereAnUprightOneDoes()
    {
        SlidePages pages = Layout();
        pages.Count.ShouldBe(2);

        PlacedText turned = TextOf(pages.Slides[0]);
        PlacedText upright = TextOf(pages.Slides[1]);

        turned.Runs.Count.ShouldBe(upright.Runs.Count);

        for (int index = 0; index < upright.Runs.Count; index++)
        {
            DocPoint here = Placed(turned, index);
            DocPoint there = Placed(upright, index);

            turned.Runs[index].Run.Text.ShouldBe(upright.Runs[index].Run.Text);
            here.X.Points.ShouldBe(there.X.Points, 0.05);
            here.Y.Points.ShouldBe(there.Y.Points, 0.05);
        }
    }

    /// <summary>
    /// The two rotations cancel, so the turned shape's text is drawn upright after all.
    /// </summary>
    /// <remarks>
    /// A quarter turn clockwise on the shape and a quarter turn anticlockwise on its text leave
    /// the linear part of the composed matrix the identity. Getting either sign wrong leaves the
    /// body on its side, which is what the two <c>schematicplay</c> decks showed.
    /// </remarks>
    [Fact]
    public void TheShapesTurnAndTheTextsTurnCancel()
    {
        AffineTransform transform = TextOf(Layout().Slides[0]).Transform;

        transform.A.ShouldBe(1.0, 1e-9);
        transform.B.ShouldBe(0.0, 1e-9);
        transform.C.ShouldBe(0.0, 1e-9);
        transform.D.ShouldBe(1.0, 1e-9);
    }

    /// <summary>
    /// The text wraps at the room the padding leaves, not at the shape's stated width.
    /// </summary>
    /// <remarks>
    /// The shape is 2 cm wide and its text area 15.5 cm; wrapping at 2 cm puts one word on a
    /// line, and swapping the box's dimensions on top of the padding wraps at 31 cm and gives
    /// one line for the whole paragraph. Two lines is the reference's answer and the upright
    /// control's.
    /// </remarks>
    [Fact]
    public void TheTurnedTextWrapsAtTheRoomThePaddingLeaves()
    {
        PlacedText turned = TextOf(Layout().Slides[0]);

        turned.Runs.Select(run => run.Run.Origin.Y).Distinct().Count().ShouldBe(2);
    }

    private static DocPoint Placed(PlacedText text, int index)
        => ShapeTransform.Apply(text.Transform, text.Runs[index].Run.Origin);
}
