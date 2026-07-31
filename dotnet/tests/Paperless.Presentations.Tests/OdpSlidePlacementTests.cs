using Paperless.Core;
using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Presentations.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// Checks the ODF path puts the same deck's shapes in the same places as the OOXML one.
/// </summary>
/// <remarks>
/// <para>
/// <c>shape-geometry.odp</c> is <c>shape-geometry.pptx</c> converted by LibreOffice, so the two
/// describe the same slides through two vocabularies and each can be measured against its own
/// reference PDF. The expected numbers are therefore the same numbers, to within the hundredth of
/// a millimetre LibreOffice rounds every dimension to on the way out — a 144 pt rectangle becomes
/// 5.079 cm, which is 143.97 pt.
/// </para>
/// <para>
/// What the ODF file exercises that the OOXML one cannot: positions in centimetres rather than
/// EMUs, a <c>draw:transform</c> whose rotation runs the opposite way, a <c>draw:g</c> whose
/// children carry absolute coordinates instead of a child coordinate space, and fills reached
/// through a style rather than stated on the shape.
/// </para>
/// </remarks>
public class OdpSlidePlacementTests
{
    /// <summary>
    /// How far a placement may differ, in points.
    /// </summary>
    /// <remarks>
    /// A twentieth of a point, as for the OOXML deck. The conversion's own quantisation is
    /// already inside the expected numbers rather than inside the tolerance: the file says
    /// 5.079 cm and the test expects 143.97 pt.
    /// </remarks>
    private const double TolerancePoints = 0.05;

    private static SlidePages Layout()
    {
        using IDocument document = new PresentationReader().Read(
            DocumentSource.FromFile(Corpus.Require("shape-geometry.odp")));

        document.ShouldBeAssignableTo<IPaginatedDocument>();
        return (SlidePages)((IPaginatedDocument)document).Layout();
    }

    private static PlacedShape Named(LaidOutSlide slide, string name)
        => slide.Shapes.First(shape => shape.Name == name);

    private static void ShouldBeAt(
        DocRect rectangle, double left, double top, double width, double height)
    {
        rectangle.X.Points.ShouldBe(left, TolerancePoints);
        rectangle.Y.Points.ShouldBe(top, TolerancePoints);
        rectangle.Width.Points.ShouldBe(width, TolerancePoints);
        rectangle.Height.Points.ShouldBe(height, TolerancePoints);
    }

    [Fact]
    public void TheSlideSizeComesFromTheMastersPageLayoutAndNotTheFirstOne()
    {
        SlidePages pages = Layout();

        pages.Count.ShouldBe(4);

        // 25.4 x 19.05 cm, which is the 720 x 540 pt the OOXML original states. The first
        // style:page-layout in the file is the notes one and is A4 portrait, so a reader taking
        // that instead reports a 595 x 842 pt slide.
        pages[0].Size.Width.Points.ShouldBe(720, TolerancePoints);
        pages[0].Size.Height.Points.ShouldBe(540, TolerancePoints);
    }

    [Fact]
    public void AShapeLandsWhereItsSvgAttributesSayItDoes()
    {
        LaidOutSlide slide = Layout().Slides[0];

        // svg:x="2.54cm" svg:y="1.27cm" svg:width="5.079cm" svg:height="2.539cm".
        ShouldBeAt(Named(slide, "PlainRect").Bounds, 72, 36, 143.97, 71.97);
        Named(slide, "PlainRect").Fill.ShouldBe(Paint.Solid(Colour.FromRgb(0xC00000)));
    }

    [Fact]
    public void AThemedFillArrivesAlreadyResolvedInTheStyle()
    {
        LaidOutSlide slide = Layout().Slides[0];

        // The ODF exporter writes the resolved colour rather than the scheme reference, so this is
        // the same value the OOXML path computes from accent1 and from accent2 under lumMod and
        // lumOff — which is what makes the pair a cross-check rather than two separate tests.
        Named(slide, "ThemedRect").Fill.ShouldBe(Paint.Solid(Colour.FromRgb(0x18A303)));
        Named(slide, "TintedRect").Fill.ShouldBe(Paint.Solid(Colour.FromRgb(0x34B3FB)));
    }

    [Fact]
    public void AnOutlineKeepsItsWidthThroughTheStyleChain()
    {
        LaidOutSlide slide = Layout().Slides[0];

        PlacedShape outlined = Named(slide, "OutlinedRect");

        outlined.Fill.ShouldBeNull();
        outlined.Line.ShouldNotBeNull();

        // svg:stroke-width="0.053cm", which is the 1.5 pt the OOXML file states as w="19050",
        // rounded to a hundredth of a millimetre on the way through.
        outlined.Line.Width.Points.ShouldBe(1.5, 0.02);
        outlined.Line.Paint.ShouldBe(Paint.Solid(Colour.FromRgb(0x00B050)));
    }

    [Fact]
    public void AnOdfRotationTurnsTheOppositeWayFromAnOoxmlOne()
    {
        LaidOutSlide slide = Layout().Slides[0];

        // draw:transform="rotate (-0.523598775598299) translate (3.515cm 10.33cm)" is the same
        // 30 degrees clockwise the OOXML file writes as rot="1800000" — with the sign the other
        // way round, because ODF's angle is counter-clockwise. Reading it as written puts the
        // shape's centre at 5.0795 cm, 12.6995 cm; reading the sign the OOXML way puts it
        // somewhere else entirely.
        PlacedShape rotated = Named(slide, "RotatedRect");
        DocRect bounds = rotated.Bounds;

        ((bounds.X + bounds.Width / 2).Points).ShouldBe(144, 0.1);
        ((bounds.Y + bounds.Height / 2).Points).ShouldBe(360, 0.1);

        // The first vertex, which the OOXML deck's reference PDF puts at (99.61, 292.79).
        DocPoint corner = rotated.Outline.Commands[0].Point;
        corner.X.Points.ShouldBe(99.61, 0.2);
        corner.Y.Points.ShouldBe(292.79, 0.2);
    }

    [Fact]
    public void AGroupsChildrenCarryAbsoluteCoordinates()
    {
        LaidOutSlide slide = Layout().Slides[1];

        // The OOXML original states these inside a group whose child space is half its extent, so
        // the reader has to double them. LibreOffice's ODF export has already done that: the
        // children read 2.54, 2.54 and 7.62, 5.08 cm outright, and a reader that scaled them
        // again would double them twice.
        ShouldBeAt(Named(slide, "GroupChildA").Bounds, 72, 72, 143.97, 71.97);
        ShouldBeAt(Named(slide, "GroupChildB").Bounds, 216, 144, 143.97, 71.97);
        ShouldBeAt(Named(slide, "Outside").Bounds, 72, 288, 143.97, 71.97);
    }

    [Fact]
    public void AMirroredPresetIsMirroredInItsOwnBox()
    {
        LaidOutSlide slide = Layout().Slides[3];

        // ODF puts the flip on the geometry as draw:mirror-horizontal rather than on the
        // transform, so it acts in the shape's own box — the same place a:xfrm's flipH acts.
        List<(double X, double Y)> vertices =
        [
            .. Named(slide, "TriangleFlipH").Outline.Commands
                .Where(command => command.Verb != PathVerb.Close)
                .Select(command => (command.Point.X.Points, command.Point.Y.Points)),
        ];

        vertices.Count.ShouldBe(3);
        vertices.ShouldContain(vertex => Math.Abs(vertex.X - 431.97) < 0.1
                                         && Math.Abs(vertex.Y - 143.97) < 0.1);
        vertices.ShouldContain(vertex => Math.Abs(vertex.X - 288) < 0.1
                                         && Math.Abs(vertex.Y - 143.97) < 0.1);
    }

    [Fact]
    public void TextIsPlacedWhereTheOoxmlPathPlacesIt()
    {
        LaidOutSlide slide = Layout().Slides[2];

        PlacedShape box = Named(slide, "TopLeftText");
        box.Text.ShouldNotBeNull();

        GlyphRun run = box.Text.Runs.Single().Run;

        // The same 72, 90 the OOXML deck's reference PDF draws, reached through a completely
        // different set of properties: fo:padding-* instead of a:bodyPr insets, and
        // style:font-independent-line-spacing instead of the importer turning it on.
        run.Text.ShouldBe("Anchored top left");
        run.Origin.X.Points.ShouldBe(72, 0.1);
        run.Origin.Y.Points.ShouldBe(90, 0.1);
    }
}
