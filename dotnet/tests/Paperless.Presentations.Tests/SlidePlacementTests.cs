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
/// Checks where a deck's shapes end up, against numbers read out of LibreOffice's own PDF.
/// </summary>
/// <remarks>
/// <para>
/// The corpus deck is <c>shape-geometry.pptx</c>, hand-written so that every offset is a round
/// number of inches: a shape at <c>a:off x="914400"</c> is at exactly 72 pt and any disagreement
/// is a bug rather than a rounding. The expectations below are the coordinates LibreOffice's PDF
/// export puts each shape at, transcribed once and quoted in each test.
/// </para>
/// <para>
/// <strong>LibreOffice's PDF is one unit of 1/100 mm smaller and further up-left than the file
/// says, everywhere.</strong> Its page clip starts at <c>0 0.028</c> rather than <c>0 0</c>, its
/// page background is 719.971 pt wide on a 720 pt page, and a rectangle stated as 72 pt from the
/// left comes out at 71.972 — 0.0283 pt, which is exactly one hundredth of a millimetre, the
/// drawing layer's own unit. It applies to the sheet as well as to the shapes, so it is the
/// export's rounding rather than a placement difference, and the tests here assert against the
/// file's exact numbers with a tolerance wide enough to cover it.
/// </para>
/// <para>
/// These tests need no LibreOffice: the reference numbers are already known. The comparison that
/// re-derives them lives in <c>Paperless.Fidelity.Tests</c>.
/// </para>
/// </remarks>
public class SlidePlacementTests
{
    /// <summary>
    /// How far a placement may differ from LibreOffice's, in points.
    /// </summary>
    /// <remarks>
    /// A twentieth of a point, which is nearly twice the 0.0283 pt the export's own rounding
    /// costs and far below anything visible. A real placement error — a missing group scale, a
    /// flip applied after a rotation — is wrong by tens of points, not by hundredths.
    /// </remarks>
    private const double TolerancePoints = 0.05;

    private static SlidePages Layout(string name)
    {
        using IDocument document =
            new PresentationReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

        document.ShouldBeAssignableTo<IPaginatedDocument>();
        return (SlidePages)((IPaginatedDocument)document).Layout();
    }

    private static PlacedShape Named(LaidOutSlide slide, string name)
        => slide.Shapes.FirstOrDefault(shape => shape.Name == name)
           ?? throw new InvalidOperationException(
               $"Slide {slide.Index + 1} has no shape named '{name}'; it has "
               + string.Join(", ", slide.Shapes.Select(shape => shape.Name ?? "<unnamed>")) + ".");

    private static void ShouldBeAt(
        DocRect rectangle, double left, double top, double width, double height)
    {
        rectangle.X.Points.ShouldBe(left, TolerancePoints);
        rectangle.Y.Points.ShouldBe(top, TolerancePoints);
        rectangle.Width.Points.ShouldBe(width, TolerancePoints);
        rectangle.Height.Points.ShouldBe(height, TolerancePoints);
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

    [Fact]
    public void ADeckLaysOutOnePagePerSlideAtTheSizeItStates()
    {
        SlidePages pages = Layout("shape-geometry.pptx");

        pages.Count.ShouldBe(4);

        // p:sldSz cx="9144000" cy="6858000", which LibreOffice's PDF reports as a 720 x 540 pt
        // MediaBox — the one office measurement already in EMUs, so nothing converts it.
        foreach (int index in Enumerable.Range(0, pages.Count))
        {
            pages[index].Size.Width.Points.ShouldBe(720, TolerancePoints);
            pages[index].Size.Height.Points.ShouldBe(540, TolerancePoints);
            pages[index].Index.ShouldBe(index);
        }
    }

    [Fact]
    public void AShapeLandsWhereItsOffsetAndExtentSayItDoes()
    {
        LaidOutSlide slide = Layout("shape-geometry.pptx").Slides[0];

        // LibreOffice's PDF fills 71.972 to 215.943 across and 35.972 to 107.943 down.
        ShouldBeAt(Named(slide, "PlainRect").Bounds, 72, 36, 144, 72);
        ShouldBeAt(Bounds(Named(slide, "PlainRect").Outline), 72, 36, 144, 72);
    }

    [Fact]
    public void ALiteralFillIsTheColourTheFileStates()
    {
        LaidOutSlide slide = Layout("shape-geometry.pptx").Slides[0];

        // 0.7529411764 0 0 rg in the reference, which is 192/255.
        Named(slide, "PlainRect").Fill.ShouldBe(Paint.Solid(Colour.FromRgb(0xC00000)));
    }

    [Fact]
    public void ASchemeColourResolvesThroughTheMastersColourMap()
    {
        LaidOutSlide slide = Layout("shape-geometry.pptx").Slides[0];

        // accent1 is 18a303 in this deck's theme; the reference writes
        // 0.094117647 0.6392156862 0.0117647058 rg, which is (24, 163, 3).
        Named(slide, "ThemedRect").Fill.ShouldBe(Paint.Solid(Colour.FromRgb(0x18A303)));
    }

    [Fact]
    public void ASchemeColourCarriesItsTransformChain()
    {
        LaidOutSlide slide = Layout("shape-geometry.pptx").Slides[0];

        // accent2 (0369a3) under lumMod 60% and lumOff 40%. LibreOffice resolves it to 34b3fb —
        // seen both in its PDF, 0.2039215686 0.7019607843 0.9843137254 rg, and in the
        // draw:fill-color of the same deck exported to ODF.
        Named(slide, "TintedRect").Fill.ShouldBe(Paint.Solid(Colour.FromRgb(0x34B3FB)));
    }

    [Fact]
    public void AnOutlineIsStrokedAtTheWidthAndColourStated()
    {
        LaidOutSlide slide = Layout("shape-geometry.pptx").Slides[0];

        PlacedShape outlined = Named(slide, "OutlinedRect");
        outlined.Fill.ShouldBeNull();

        // a:ln w="19050", which is 1.5 pt; the reference writes 1.50233 w, having rounded the
        // width through hundredths of a millimetre on the way out.
        outlined.Line.ShouldNotBeNull();
        outlined.Line.Width.Points.ShouldBe(1.5, TolerancePoints);
        outlined.Line.Paint.ShouldBe(Paint.Solid(Colour.FromRgb(0x00B050)));
    }

    [Fact]
    public void AnUnstrokedShapeStatesNoLineAtAll()
    {
        LaidOutSlide slide = Layout("shape-geometry.pptx").Slides[0];

        // <a:ln w="0"><a:noFill/></a:ln> is what LibreOffice's own export writes for a shape with
        // no outline. Reading the width alone would make it a black hairline round every shape in
        // every deck it wrote.
        Named(slide, "PlainRect").Line.ShouldBeNull();
    }

    [Fact]
    public void ARotationTurnsClockwiseAboutTheShapesOwnCentre()
    {
        LaidOutSlide slide = Layout("shape-geometry.pptx").Slides[0];

        PlacedShape rotated = Named(slide, "RotatedRect");

        // rot="1800000" is 30 degrees. The unrotated rectangle is 72,324 sized 144x72, so its
        // centre is at (144, 360) and stays there; the corner at (-72, -36) from the centre goes
        // to (-44.35, -67.18). LibreOffice's PDF puts that corner at (99.609, 292.791) with the
        // page flipped, which is (99.61, 292.79) down the page.
        DocPoint corner = rotated.Outline.Commands[0].Point;

        ShouldBeAt(rotated.Bounds, 72, 324, 144, 72);
        corner.X.Points.ShouldBe(99.61, 0.1);
        corner.Y.Points.ShouldBe(292.79, 0.1);
    }

    [Fact]
    public void AGroupScalesItsChildrenByTheRatioOfItsExtentToItsChildExtent()
    {
        LaidOutSlide slide = Layout("shape-geometry.pptx").Slides[1];

        // The group is at 72,72 sized 288x144 with a child space of 144x72, so everything inside
        // it doubles. Child A is stated at 0,0 sized 72x36 and child B at 72,36 sized 72x36.
        // LibreOffice's PDF fills 71.972-215.943 x 71.972-143.943 and
        // 215.972-359.943 x 143.972-215.943.
        ShouldBeAt(Named(slide, "GroupChildA").Bounds, 72, 72, 144, 72);
        ShouldBeAt(Named(slide, "GroupChildB").Bounds, 216, 144, 144, 72);

        // The ungrouped shape beside them is unaffected, which is what says the scale was applied
        // to the group's children rather than to the slide.
        ShouldBeAt(Named(slide, "Outside").Bounds, 72, 288, 144, 72);
    }

    [Fact]
    public void APresetTriangleHasItsRightAngleWhereThePresetPutsIt()
    {
        LaidOutSlide slide = Layout("shape-geometry.pptx").Slides[3];

        // rtTriangle's path is moveTo(l, b), lnTo(l, t), lnTo(r, b)
        // (oox/source/drawingml/customshapes/presetShapeDefinitions.xml). The reference draws it
        // at (71.972, 143.972), (71.972, 71.972), (215.972, 143.972) down the page.
        IReadOnlyList<PathCommand> path = Named(slide, "Triangle").Outline.Commands;

        Vertices(path).ShouldBe(
            [(72, 144), (72, 72), (216, 144)], new PointComparer());
    }

    [Fact]
    public void AFlipMirrorsAboutTheShapesOwnCentre()
    {
        LaidOutSlide slide = Layout("shape-geometry.pptx").Slides[3];

        // flipH on a shape at 288,72 sized 144x72: the right angle moves to the bottom right.
        // The reference draws (431.943, 143.972), (431.943, 71.972), (287.943, 143.972).
        Vertices(Named(slide, "TriangleFlipH").Outline.Commands).ShouldBe(
            [(432, 144), (432, 72), (288, 144)], new PointComparer());

        // flipV on a shape at 72,216: the right angle moves to the top left.
        Vertices(Named(slide, "TriangleFlipV").Outline.Commands).ShouldBe(
            [(72, 216), (72, 288), (216, 216)], new PointComparer());
    }

    [Fact]
    public void AFlipIsAppliedBeforeTheRotationAndNotAfterIt()
    {
        LaidOutSlide slide = Layout("shape-geometry.pptx").Slides[3];

        // The measurement the whole transform order rests on. The shape is an rtTriangle at
        // 288,216 sized 144x72 with flipH="1" and rot="5400000" — 90 degrees clockwise.
        //
        // Flipping first and then rotating gives (324, 324), (396, 324), (324, 180), which is
        // exactly what LibreOffice's PDF draws: 323.943 216 m, 395.943 216 l, 323.943 360 l on a
        // 540 pt page. Rotating first and then flipping gives (396, 180), (324, 180), (396, 324)
        // — a different triangle in a different corner, and the reason a rectangle cannot tell
        // the two orders apart.
        Vertices(Named(slide, "TriangleFlipRot").Outline.Commands).ShouldBe(
            [(324, 324), (396, 324), (324, 180)], new PointComparer());
    }

    private static List<(double X, double Y)> Vertices(IReadOnlyList<PathCommand> path)
        => [.. path
            .Where(command => command.Verb != PathVerb.Close)
            .Select(command => (command.Point.X.Points, command.Point.Y.Points))];

    private sealed class PointComparer : IEqualityComparer<(double X, double Y)>
    {
        public bool Equals((double X, double Y) a, (double X, double Y) b)
            => Math.Abs(a.X - b.X) < TolerancePoints && Math.Abs(a.Y - b.Y) < TolerancePoints;

        public int GetHashCode((double X, double Y) value) => 0;
    }
}
