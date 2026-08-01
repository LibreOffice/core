using Paperless.Core;
using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Presentations.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// Checks where a binary PowerPoint deck's shapes end up, against numbers read out of
/// LibreOffice's own PDF.
/// </summary>
/// <remarks>
/// <para>
/// The corpus deck is <c>shape-geometry-ppt.ppt</c>, which is <c>shape-geometry.pptx</c> converted
/// — the same four slides through the binary vocabulary, so a disagreement between this file and
/// <see cref="SlidePlacementTests"/> names which reader is wrong rather than merely that something
/// is.
/// </para>
/// <para>
/// <strong>A PPT's coordinates convert exactly, and a PPTX's do not.</strong> Everything a binary
/// shape states is in master units of a 576th of an inch, and 576 divides an inch into 2540
/// hundredths of a millimetre without remainder — so LibreOffice's own PDF export of this deck
/// puts a shape stated at 72 pt at exactly 72.000, where the same deck as PPTX comes out at
/// 71.972. The expectations below are therefore round numbers rather than round numbers minus
/// 0.028, which is itself worth knowing before chasing a difference in either direction.
/// </para>
/// <para>
/// Shapes are identified by their position in the slide's shape list rather than by name, because
/// <strong>LibreOffice's PPT export writes no shape names at all</strong>: the property table it
/// emits carries no <c>wzName</c>, so every shape in a converted deck is anonymous. The list order
/// is the file's, which is z-order.
/// </para>
/// <para>
/// These tests need no LibreOffice: the reference numbers are already known. The comparison that
/// re-derives them lives in <c>Paperless.Fidelity.Tests</c>.
/// </para>
/// </remarks>
public class PptSlidePlacementTests
{
    /// <summary>How far a placement may differ from LibreOffice's, in points.</summary>
    /// <remarks>
    /// A twentieth of a point. The binary path has no export rounding to absorb, so most of these
    /// agree exactly; what is left is LibreOffice's habit of giving a rectangle one extra unit of
    /// width, which is a hundredth of a millimetre.
    /// </remarks>
    private const double TolerancePoints = 0.05;

    private static SlidePages Layout(string name)
    {
        using IDocument document =
            new PresentationReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

        document.ShouldBeAssignableTo<IPaginatedDocument>();
        return (SlidePages)((IPaginatedDocument)document).Layout();
    }

    private static void ShouldBeAt(
        DocRect rectangle, double left, double top, double width, double height)
    {
        rectangle.X.Points.ShouldBe(left, TolerancePoints);
        rectangle.Y.Points.ShouldBe(top, TolerancePoints);
        rectangle.Width.Points.ShouldBe(width, TolerancePoints);
        rectangle.Height.Points.ShouldBe(height, TolerancePoints);
    }

    [Fact]
    public void ADeckLaysOutOnePagePerSlideAtTheSizeTheDocumentAtomStates()
    {
        SlidePages pages = Layout("shape-geometry-ppt.ppt");

        pages.Count.ShouldBe(4);

        // The DocumentAtom says 5760 x 4320 master units, which is 10 x 7.5 inches. LibreOffice's
        // PDF reports a 720 x 540 pt MediaBox for it, exactly.
        foreach (int index in Enumerable.Range(0, pages.Count))
        {
            pages[index].Size.Width.Points.ShouldBe(720, TolerancePoints);
            pages[index].Size.Height.Points.ShouldBe(540, TolerancePoints);
            pages[index].Index.ShouldBe(index);
        }
    }

    [Fact]
    public void AShapeLandsWhereItsClientAnchorSaysItDoes()
    {
        LaidOutSlide slide = Layout("shape-geometry-ppt.ppt").Slides[0];

        // The anchor is eight bytes of signed words in the order top, left, right, bottom — the
        // order really is that — so this shape states 288, 576, 1728, 864 and belongs at 72 pt
        // from the left and 36 pt down. LibreOffice's PDF fills 72 to 216 across and 36 to 108
        // down, to the digit.
        ShouldBeAt(slide.Shapes[0].Bounds, 72, 36, 144, 72);
    }

    [Fact]
    public void AFillIsTheColourTheShapesPropertyTableStates()
    {
        LaidOutSlide slide = Layout("shape-geometry-ppt.ppt").Slides[0];

        // fillColor is packed blue-green-red, so 0x00C000 is red rather than green. The reference
        // writes 0.7529411764 0 0 rg, which is 192/255.
        slide.Shapes[0].Fill.ShouldBe(Paint.Solid(Colour.FromRgb(0xC00000)));

        // The theme colours the PPTX states survive the conversion as literals, which is what
        // makes them checkable here at all: LibreOffice resolves accent1 and the lumMod/lumOff
        // pair on the way out, and its PDF then writes the same two numbers for both files.
        slide.Shapes[1].Fill.ShouldBe(Paint.Solid(Colour.FromRgb(0x18A303)));
        slide.Shapes[2].Fill.ShouldBe(Paint.Solid(Colour.FromRgb(0x34B3FB)));
    }

    [Fact]
    public void AnOutlineIsStrokedAtTheWidthAndColourStated()
    {
        LaidOutSlide slide = Layout("shape-geometry-ppt.ppt").Slides[0];

        // lineWidth is in EMUs: 38100 is 3 pt, and the reference writes 3.00467 w having rounded
        // it through hundredths of a millimetre on the way out.
        Stroke themed = slide.Shapes[1].Line.ShouldNotBeNull();
        themed.Width.Points.ShouldBe(3.0, TolerancePoints);

        // A shape with a line and no line colour is black, not "no line": lineColor defaults to
        // zero and the reference writes 0 0 0 RG.
        themed.Paint.ShouldBe(Paint.Solid(Colour.Black));

        PlacedShape outlined = slide.Shapes[3];
        outlined.Fill.ShouldBeNull();
        Stroke line = outlined.Line.ShouldNotBeNull();
        line.Width.Points.ShouldBe(1.5, TolerancePoints);
        line.Paint.ShouldBe(Paint.Solid(Colour.FromRgb(0x00B050)));
    }

    [Fact]
    public void AnUnfilledShapeStatesNoFillAtAll()
    {
        LaidOutSlide slide = Layout("shape-geometry-ppt.ppt").Slides[0];

        // fFilled is a boolean packed into property 447, and reading the group's whole word would
        // make every shape that mentions any fill property filled.
        slide.Shapes[3].Fill.ShouldBeNull();
    }

    [Fact]
    public void ARotationTurnsClockwiseAboutTheShapesOwnCentre()
    {
        LaidOutSlide slide = Layout("shape-geometry-ppt.ppt").Slides[0];

        // Rotation is 16.16 fixed-point degrees: 1966080 is 30. The unrotated rectangle is at
        // 72,324 sized 144x72, so its centre is at (144, 360) and stays there. LibreOffice's PDF
        // draws the corners at (63.609, 355.181), (99.609, 292.819), (224.306, 175.181 up) and
        // (188.306, 112.819 up), which down the page is (99.61, 292.82) for the top-left corner.
        PlacedShape rotated = slide.Shapes[4];

        ShouldBeAt(rotated.Bounds, 72, 324, 144, 72);
        rotated.Outline.Commands[0].Point.X.Points.ShouldBe(99.61, 0.1);
        rotated.Outline.Commands[0].Point.Y.Points.ShouldBe(292.82, 0.1);
    }

    [Fact]
    public void AQuarterTurnedShapeIsMeasuredInTheBoxItOccupiesAfterTurning()
    {
        LaidOutSlide slide = Layout("shape-geometry-ppt.ppt").Slides[3];

        // The trap this format keeps for a reader who has done DrawingML first. The fourth shape
        // states an anchor 72 wide and 144 tall with a rotation of 270 degrees; the drawing layer
        // takes that as the box the shape occupies *after* turning, so the width and height are
        // swapped about the centre before the rotation is applied (msdffimp.cxx:4356-4368). The
        // PPTX this deck was converted from states the same shape as 144 x 72 at 288,216 with a
        // 90 degree rotation — the same picture — and both must come out as a 72 x 144 box at
        // 324,180 on the page.
        DocRect drawn = Bounds(slide.Shapes[3].Outline);

        drawn.X.Points.ShouldBe(324, TolerancePoints);
        drawn.Y.Points.ShouldBe(180, TolerancePoints);
        drawn.Width.Points.ShouldBe(72, TolerancePoints);
        drawn.Height.Points.ShouldBe(144, TolerancePoints);
    }

    [Fact]
    public void AGroupMapsItsChildrenOntoItsOwnAnchor()
    {
        LaidOutSlide slide = Layout("shape-geometry-ppt.ppt").Slides[1];

        // The group's client anchor is 576,576 to 2879,1727 and its two children state child
        // anchors of 576,576-1727,1151 and 1728,1152-2879,1727 in the same units, so the map is
        // one to one and the children land at 72,72 and 216,144. LibreOffice's PDF fills
        // 72-215.887 x 72-143.887 and 216-359.887 x 144-215.887; the eighth of a point it is
        // short by is its own rectangle arithmetic, which counts a width as right minus left
        // plus one unit of a hundredth of a millimetre.
        ShouldBeAt(slide.Shapes[0].Bounds, 72, 72, 143.875, 71.875);
        ShouldBeAt(slide.Shapes[1].Bounds, 216, 144, 143.875, 71.875);

        // The shape beside the group is unaffected, which is what says the map reached the
        // group's children rather than the slide.
        ShouldBeAt(slide.Shapes[2].Bounds, 72, 288, 144, 72);
    }

    [Fact]
    public void AGroupsChildrenAreStillLaidOutInSlideCoordinates()
    {
        LaidOutSlide slide = Layout("shape-geometry-ppt.ppt").Slides[1];

        // A group's map is a scale as well as a translation, so a reader testing for the identity
        // treats every group child as rotated and hides its text behind a matrix. Both children
        // here hold no text, but the same rule decides ppt-features.ppt's group, whose two text
        // frames LibreOffice draws at pens of 89.94 and 340.21.
        foreach (PlacedShape shape in slide.Shapes)
        {
            if (shape.Text is { } text) text.IsUpright.ShouldBeTrue();
        }
    }

    [Fact]
    public void TheSlideBackgroundIsTheBackgroundShapesFill()
    {
        SlidePages pages = Layout("shape-geometry-ppt.ppt");

        // The background is a shape rather than a property — an SpContainer flagged Background,
        // sitting beside the patriarch group rather than inside it — and it must not appear in
        // the shape list as well, or every slide gains a full-sheet rectangle in front of nothing.
        foreach (LaidOutSlide slide in pages.Slides)
        {
            slide.Background.ShouldBe(Paint.Solid(Colour.White));
        }

        pages.Slides[0].Shapes.Count.ShouldBe(5);
    }

    [Fact]
    public void AHiddenSlideIsLaidOutAndFlaggedRatherThanDropped()
    {
        SlidePages pages = Layout("ppt-features.ppt");

        pages.Count.ShouldBe(3);
        pages.Slides.Select(slide => slide.IsHidden).ShouldBe([false, false, true]);
    }

    [Fact]
    public void APresetShapeTypeExpandsToItsGeometryRatherThanItsBox()
    {
        // ppt-features.ppt came through ODF rather than through DrawingML, and its exporter does
        // name preset types where the PPTX one writes vertex arrays: its second slide holds an
        // ellipse (type 3) and a rounded rectangle (type 2). An ellipse is the check worth making
        // because its path is four cubics rather than four lines, so a reader falling back to the
        // bounding box produces a five-command path instead of a nine-command one.
        LaidOutSlide slide = Layout("ppt-features.ppt").Slides[1];

        PlacedShape ellipse = slide.Shapes[1];
        ellipse.Outline.Commands.Count(command => command.Verb == PathVerb.CubicTo).ShouldBe(4);

        // And its text sits in the box inscribed at 45 degrees rather than in the bounding box,
        // which is what puts a caption inside a circle clear of its edge: the shape is at 72 pt
        // and 226.5 wide, and LibreOffice draws its first line at a pen of 89.943.
        PlacedText text = ellipse.Text.ShouldNotBeNull();
        text.Runs[0].Run.Origin.X.Points.ShouldBe(89.94, 0.1);
    }

    private static DocRect Bounds(GraphicsPath path)
    {
        Length left = Length.FromEmu(long.MaxValue);
        Length top = Length.FromEmu(long.MaxValue);
        Length right = Length.FromEmu(long.MinValue);
        Length bottom = Length.FromEmu(long.MinValue);

        foreach (PathCommand command in path.Commands)
        {
            if (command.Verb == PathVerb.Close) continue;

            left = Length.Min(left, command.Point.X);
            top = Length.Min(top, command.Point.Y);
            right = Length.Max(right, command.Point.X);
            bottom = Length.Max(bottom, command.Point.Y);
        }

        return new DocRect(left, top, right - left, bottom - top);
    }
}
