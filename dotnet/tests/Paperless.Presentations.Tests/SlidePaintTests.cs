using Paperless.Core;
using Paperless.Core.Documents;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Presentations.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// The gradient, tiled-bitmap and picture fills of <c>paint-fills.fodp</c> and its PPTX export.
/// </summary>
/// <remarks>
/// <para>
/// One corpus document in two formats, which is the whole point of testing them together: the
/// PPTX is LibreOffice's own export of the flat ODF, so any place where the two readers disagree
/// is a place where one of them has the format's convention backwards. Those conventions are the
/// substance of this feature and every one of them is invisible until a colour is compared —
/// a red-to-blue ramp drawn blue-to-red is the right shape, the right size and the wrong picture.
/// </para>
/// <para>
/// The measurement against LibreOffice's rendering lives in <c>Paperless.Fidelity.Tests</c>;
/// this file pins the numbers that comparison depends on so that a regression says <em>which</em>
/// mapping broke rather than only that the page changed.
/// </para>
/// </remarks>
public class SlidePaintTests
{
    private static readonly Colour Red = Colour.FromRgb(0xFF0000);
    private static readonly Colour Blue = Colour.FromRgb(0x0000FF);
    private static readonly Colour Green = Colour.FromRgb(0x008000);
    private static readonly Colour Yellow = Colour.FromRgb(0xFFFF00);
    private static readonly Colour Cyan = Colour.FromRgb(0x00C0C0);
    private static readonly Colour Ink = Colour.FromRgb(0x101010);

    public static TheoryData<string> BothFormats => ["paint-fills.fodp", "paint-fills-pptx.pptx"];

    [Theory]
    [MemberData(nameof(BothFormats))]
    public void ALinearGradientRunsDownTheShapeFromItsFirstColour(string document)
    {
        GradientPaint gradient = Gradient(document, shape: 0);

        gradient.Kind.ShouldBe(GradientKind.Linear);
        gradient.Stops[0].Colour.ShouldBe(Red);
        gradient.Stops[^1].Colour.ShouldBe(Blue);

        // Vertical, top to bottom. ODF spells that draw:angle="0deg" and DrawingML spells it
        // a:lin ang="5400000", and the two only agree because the conversions are inverses:
        // LibreOffice's importer writes (8100 - ang/6000) % 3600 tenths of a degree
        // (fillproperties.cxx:563), which takes 90 degrees clockwise from the x axis back to
        // zero on ODF's own anticlockwise-from-the-top scale.
        gradient.Start.X.ShouldBe(gradient.End.X);
        gradient.Start.Y.Emu.ShouldBeLessThan(gradient.End.Y.Emu);

        // The axis spans the shape's height exactly, because the ramp is axis-aligned; a
        // diagonal one would span w·|dx| + h·|dy| instead.
        (gradient.End.Y - gradient.Start.Y).Millimetres.ShouldBe(50, 0.01);
    }

    [Theory]
    [MemberData(nameof(BothFormats))]
    public void AnAxialGradientPutsItsOuterColourAtBothEnds(string document)
    {
        GradientPaint gradient = Gradient(document, shape: 1);

        gradient.Kind.ShouldBe(GradientKind.Linear);
        gradient.Stops.Count.ShouldBe(3, "an axial ramp is symmetric about its middle");
        gradient.Stops[0].Colour.ShouldBe(Yellow);
        gradient.Stops[1].Colour.ShouldBe(Green);
        gradient.Stops[2].Colour.ShouldBe(Yellow);
        gradient.Stops[1].Offset.ShouldBe(0.5, 1e-9);
    }

    [Theory]
    [MemberData(nameof(BothFormats))]
    public void ARadialGradientPutsItsFirstColourAtTheCentre(string document)
    {
        GradientPaint gradient = Gradient(document, shape: 2);

        gradient.Kind.ShouldBe(GradientKind.Radial);

        // The trap, and it is invisible in every measurement that is not a colour. ODF's
        // draw:start-color paints the OUTER edge of a radial gradient — getRadialGradientAlpha
        // returns 1 - hypot (basegfx/source/tools/gradienttools.cxx:641), so the centre takes
        // the end of the ramp — while DrawingML's first a:gs is the centre, because LibreOffice
        // reverses the stop list for a path gradient (fillproperties.cxx:544) into a model that
        // then reads it outside-in. The two files state the same picture in opposite orders and
        // both must arrive here the same way round.
        gradient.Stops[0].Colour.ShouldBe(Ink, "the centre takes the first stop");
        gradient.Stops[^1].Colour.ShouldBe(Cyan);

        // Half the diagonal, not half the width: Gradient::GetBoundRect builds a square of side
        // hypot(w, h) for a circular gradient (vcl/source/gdi/gradient.cxx:246-251). Half the
        // width instead moves the page's mean absolute error from 0.0016 to 0.0054.
        double radius = (gradient.End.X - gradient.Start.X).Millimetres;
        radius.ShouldBe(Math.Sqrt((80 * 80) + (50 * 50)) / 2, 0.01);
    }

    [Theory]
    [MemberData(nameof(BothFormats))]
    public void ATiledBitmapFillStatesOneCentimetreTiles(string document)
    {
        BitmapPaint bitmap = Shape(document, slide: 0, index: 3).Fill.ShouldBeOfType<BitmapPaint>();

        bitmap.Stretch.ShouldBeFalse();
        bitmap.Image.IsDecoded.ShouldBeFalse("a reader hands the bytes on; a backend decodes them");
        bitmap.Image.EncodedBytes.Length.ShouldBeGreaterThan(0);

        // One centimetre, stated two different ways. ODF says draw:fill-image-width="1cm"
        // outright; the PPTX says a:tile sx="471698" over an eight-pixel checkerboard, which is
        // one centimetre only if those eight pixels are 8/96 of an inch — so the reader has to
        // know the picture's natural size without decoding it. The tolerance covers that: 8 px
        // at 96 dpi is 2.1167 mm and 4.71698 of them is 9.984 mm, not 10.
        bitmap.TileSize.Width.Millimetres.ShouldBe(10, 0.05);
        bitmap.TileSize.Height.Millimetres.ShouldBe(10, 0.05);

        // Anchored on the middle of the shape rather than its corner: ODF's
        // draw:fill-image-ref-point and DrawingML's a:tile/@algn both default to a corner in
        // their schemas and are written centred by LibreOffice's own export.
        (bitmap.TileOffset.X + (bitmap.TileSize.Width / 2)).Millimetres.ShouldBe(180, 0.05);
        (bitmap.TileOffset.Y + (bitmap.TileSize.Height / 2)).Millimetres.ShouldBe(115, 0.05);
    }

    [Theory]
    [MemberData(nameof(BothFormats))]
    public void APictureIsPlacedWhereItsFrameIs(string document)
    {
        PlacedPicture picture = Shape(document, slide: 1, index: 0).Picture.ShouldNotBeNull();

        picture.Opacity.ShouldBe(1.0);

        RasterImage raster = picture.Image.ShouldNotBeNull();
        raster.IsDecoded.ShouldBeFalse();
        raster.EncodedBytes.Length.ShouldBeGreaterThan(0);

        picture.Destination.Left.Millimetres.ShouldBe(40, 0.05);
        picture.Destination.Top.Millimetres.ShouldBe(30, 0.05);
        picture.Destination.Width.Millimetres.ShouldBe(80, 0.05);
        picture.Destination.Height.Millimetres.ShouldBe(60, 0.05);
    }

    [Theory]
    [MemberData(nameof(BothFormats))]
    public void EveryFillOnTheFirstSlideIsPainted(string document)
    {
        LaidOutSlide slide = Slides(document)[0];

        slide.Shapes.Count.ShouldBe(4);
        slide.Shapes.ShouldAllBe(shape => shape.Fill != null);
    }

    private static GradientPaint Gradient(string document, int shape)
        => Shape(document, slide: 0, shape).Fill.ShouldBeOfType<GradientPaint>();

    private static PlacedShape Shape(string document, int slide, int index)
    {
        IReadOnlyList<LaidOutSlide> slides = Slides(document);
        slides.Count.ShouldBeGreaterThan(slide);
        slides[slide].Shapes.Count.ShouldBeGreaterThan(index);
        return slides[slide].Shapes[index];
    }

    private static IReadOnlyList<LaidOutSlide> Slides(string document)
    {
        using IDocument read =
            new PresentationReader().Read(DocumentSource.FromFile(Corpus.Require(document)));

        read.ShouldBeAssignableTo<IPaginatedDocument>();
        return ((SlidePages)((IPaginatedDocument)read).Layout()).Slides;
    }
}
