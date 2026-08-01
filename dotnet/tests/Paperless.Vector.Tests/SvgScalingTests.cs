using System.Text;
using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.Vector.Svg;
using Shouldly;

namespace Paperless.Vector.Tests;

/// <summary>
/// The size rules: intrinsic size, view box, and which of them the frame overrides.
/// </summary>
/// <remarks>
/// <para>
/// The metafile TODO calls this "the most common source of wrongly-scaled or mirrored
/// output", and SVG has the same three quantities the metafile formats will: an intrinsic
/// physical size, a coordinate space, and whatever extent the document states for the frame.
/// Every expected value below was checked against LibreOffice 24.2 rather than derived from
/// the specification — see the remarks on <c>SvgViewport</c> for the case where the two
/// disagree.
/// </para>
/// </remarks>
public sealed class SvgScalingTests
{
    private static readonly SvgImageDecoder Decoder = new();

    [Fact]
    public void PhysicalUnitsResolveAgainstTheCssInch()
    {
        // 96 CSS pixels to the inch, which is what LibreOffice's o3tl::Length::px fixes
        // (include/o3tl/unit_conversion.hxx:44) and what svgio converts every other unit
        // through (SvgNumber.cxx:41-54).
        VectorImage image = Decode("""<svg xmlns="http://www.w3.org/2000/svg" width="1in" height="0.5in"/>""");

        image.IntrinsicSize.Width.Emu.ShouldBe(Length.EmuPerInch);
        image.IntrinsicSize.Height.Emu.ShouldBe(Length.EmuPerInch / 2);
    }

    [Fact]
    public void MillimetresBecomeTheSamePhysicalSizeLibreOfficeGives()
    {
        // Measured: soffice --convert-to pdf on a width="120mm" height="80mm" SVG produced a
        // 340.157 x 226.772 pt page, which is 120.00 x 80.00 mm.
        VectorImage image = Decode("""<svg xmlns="http://www.w3.org/2000/svg" width="120mm" height="80mm" viewBox="0 0 300 200"/>""");

        image.IntrinsicSize.Width.Millimetres.ShouldBe(120, 0.01);
        image.IntrinsicSize.Height.Millimetres.ShouldBe(80, 0.01);
    }

    [Fact]
    public void OneStatedDimensionTakesTheOtherFromTheViewBoxRatio()
    {
        // The case where the vetted library and LibreOffice disagree, and LibreOffice wins.
        // svgsvgnode.cxx:504-516 derives the missing height as fW / fViewBoxRatio; the
        // library instead keeps the view box's own height, which would letterbox the drawing
        // into half the frame. Measured on LibreOffice 24.2: a width="200" viewBox="0 0 400
        // 100" SVG rendered with a content area of 624 x 156 px at 300 dpi — aspect 4.000,
        // not 2.000.
        VectorImage image = Decode("""<svg xmlns="http://www.w3.org/2000/svg" width="200" viewBox="0 0 400 100"><rect width="1" height="1"/></svg>""");

        double aspect = (double)image.IntrinsicSize.Width.Emu / image.IntrinsicSize.Height.Emu;
        aspect.ShouldBe(4.0, 0.001);
    }

    [Fact]
    public void NoStatedDimensionsFallBackToTheViewBox()
    {
        VectorImage image = Decode("""<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 400 100"><rect width="1" height="1"/></svg>""");

        image.IntrinsicSize.Width.Emu.ShouldBe(400 * 9525);
        image.IntrinsicSize.Height.Emu.ShouldBe(100 * 9525);
    }

    [Fact]
    public void PercentageDimensionsAreTreatedAsAbsentAtTheRoot()
    {
        // There is no parent viewport to resolve them against, so LibreOffice treats
        // "absolute" and "usable" as the same question (svgsvgnode.cxx:504-505).
        VectorImage image = Decode("""<svg xmlns="http://www.w3.org/2000/svg" width="100%" height="100%" viewBox="0 0 400 100"><rect width="1" height="1"/></svg>""");

        image.IntrinsicSize.Width.Emu.ShouldBe(400 * 9525);
        image.IntrinsicSize.Height.Emu.ShouldBe(100 * 9525);
    }

    [Fact]
    public void TheFramesExtentOverridesTheImagesOwnAspectRatio()
    {
        // Measured on LibreOffice 24.2: an ODT frame of svg:width="12cm" svg:height="4cm"
        // holding a 120x80 mm SVG rendered the picture's own background filling
        // 119.2 x 39.3 mm — squashed to the frame, not letterboxed inside it. So the
        // document's extent wins and the mapping is a plain stretch.
        VectorImage image = Decode("""
            <svg xmlns="http://www.w3.org/2000/svg" width="120mm" height="80mm" viewBox="0 0 300 200">
              <rect x="0" y="0" width="300" height="200" fill="#ff0000"/>
            </svg>
            """);

        DocRect frame = new(
            Length.Zero, Length.Zero, Length.FromMillimetres(120), Length.FromMillimetres(40));

        Recorder sink = new();
        image.Draw(sink, frame);

        (DocRect bounds, _, _) = sink.Fills.ShouldHaveSingleItem();

        bounds.Width.Millimetres.ShouldBe(120, 0.05);
        bounds.Height.Millimetres.ShouldBe(40, 0.05);
    }

    [Fact]
    public void DrawingClipsToTheFrame()
    {
        VectorImage image = Decode("""<svg xmlns="http://www.w3.org/2000/svg" width="100" height="100"><rect width="10" height="10"/></svg>""");

        DocRect frame = new(Length.FromPoints(10), Length.FromPoints(20), Length.FromPoints(30), Length.FromPoints(40));

        Recorder sink = new();
        image.Draw(sink, frame);

        sink.Clips.ShouldNotBeEmpty();
        sink.Clips[0].ShouldBe(frame);
    }

    [Fact]
    public void OneStatedDimensionOnAFrameTakesTheOtherFromTheImage()
    {
        VectorImage image = Decode("""<svg xmlns="http://www.w3.org/2000/svg" width="200" height="50"/>""");

        DocSize size = image.SizeFor(Length.FromMillimetres(100), null);

        size.Width.Millimetres.ShouldBe(100, 0.001);
        size.Height.Millimetres.ShouldBe(25, 0.01);
    }

    [Fact]
    public void AZeroWidthImageRendersNothing()
    {
        // "Svg defines that a negative value is an error and that 0.0 disables rendering"
        // — svgsvgnode.cxx:487-489.
        VectorImage image = Decode("""<svg xmlns="http://www.w3.org/2000/svg" width="0" height="100"><rect width="10" height="10"/></svg>""");

        image.IsEmpty.ShouldBeTrue();
    }

    private static VectorImage Decode(string svg) => Decoder.Decode(Encoding.UTF8.GetBytes(svg));
}
