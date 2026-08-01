using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Rendering.Pdf;
using Paperless.Rendering.Raster;
using Shouldly;
using SkiaSharp;

namespace Paperless.Rendering.Tests;

/// <summary>
/// The three things a gradient could not say before: a colour per vertex, a spread, a focus.
/// </summary>
/// <remarks>
/// <para>
/// Each is exercised in <em>both</em> backends on the same paint, for the reason
/// <c>PaintTests</c> states: the IR is one description, and two renderings of it that disagree
/// is the bug worth catching. The three are together in one file because they are one change
/// — the paint additions that closed <c>PL6018</c>, <c>PL6021</c>, <c>PL6040</c> and
/// <c>PL6041</c> at once — and because each has a PDF form and a Skia form that have to be
/// checked against each other rather than against a rasteriser's opinion.
/// </para>
/// <para>
/// The raster assertions are made at 72 dpi with antialiasing off, so one point is one pixel
/// and no arithmetic hides in a filter.
/// </para>
/// </remarks>
public sealed class MeshAndSpreadTests
{
    private static readonly PdfRenderOptions Reproducible = new()
    {
        CreationDate = new DateTimeOffset(2024, 1, 1, 0, 0, 0, TimeSpan.Zero),
    };

    private static readonly Colour Red = Colour.FromRgb(0xFF0000);
    private static readonly Colour Green = Colour.FromRgb(0x00FF00);
    private static readonly Colour Blue = Colour.FromRgb(0x0000FF);

    // ------------------------------------------------------------------------------ the mesh

    [Fact]
    public void AMeshBecomesAFreeFormGouraudTriangleShading()
    {
        PdfFile pdf = Write(sink => sink.FillPath(Rectangle(72, 72, 144, 144), Fan()));

        // Type 4 rather than 5: a fan built from a boundary is not a lattice, and type 5's
        // /VerticesPerRow would have to be invented.
        pdf.Text.ShouldContain("/ShadingType 4");
        pdf.Text.ShouldContain("/BitsPerCoordinate 32/BitsPerComponent 8/BitsPerFlag 8");

        // Painted inside the path as a clip, exactly as a gradient is, because a path
        // gradient's boundary and the shape it fills need not be the same polygon.
        pdf.ContentStreams().ShouldHaveSingleItem().ShouldContain(" sh\n");
    }

    [Fact]
    public void EveryVertexIsTwelveBytesSoNoneStraddlesAByte()
    {
        // 1 flag + 4 + 4 coordinate + 3 colour. The alternative — 16-bit coordinates —
        // quantises a page-wide mesh to a fifteenth of a millimetre, which shows as a ragged
        // boundary on the very shape this paint exists to draw.
        byte[] stream = Write(sink => sink.FillPath(Rectangle(72, 72, 144, 144), Fan()))
            .Streams()
            .First(entry => entry.Dictionary.Contains("/ShadingType 4", StringComparison.Ordinal))
            .Data;

        // Four triangles, three vertices each, written as flag-0 triples.
        stream.Length.ShouldBe(4 * 3 * 12);
        for (int i = 0; i < stream.Length; i += 12) stream[i].ShouldBe((byte)0);
    }

    [Fact]
    public void AMeshInterpolatesItsCornerColoursAcrossEachTriangle()
    {
        using SKBitmap page = Rasterise(sink => sink.FillPath(Rectangle(72, 72, 144, 144), Fan()));

        // The centre vertex is white and each corner is a primary, so a point close to a corner
        // is nearly that primary and the middle is nearly white.
        page.GetPixel(144, 144).Red.ShouldBeGreaterThan((byte)230);
        page.GetPixel(144, 144).Green.ShouldBeGreaterThan((byte)230);

        page.GetPixel(78, 78).Red.ShouldBeGreaterThan((byte)180);
        page.GetPixel(78, 78).Blue.ShouldBeLessThan((byte)80);
    }

    [Fact]
    public void AMeshPaintsNothingOutsideItsOwnTriangles()
    {
        // The fan covers the square; a point outside the clip is untouched. Stated because the
        // Skia backend draws the vertices inside a clip rather than as a shader on the path,
        // and a missing clip is invisible on a mesh that happens to fit.
        using SKBitmap page = Rasterise(sink => sink.FillPath(Rectangle(72, 72, 144, 144), Fan()));

        page.GetPixel(300, 300).ShouldBe(new SKColor(255, 255, 255, 255));
    }

    [Fact]
    public void ATranslucentVertexTakesTheSameSoftMaskAFadingGradientDoes()
    {
        PdfFile pdf = Write(sink => sink.FillPath(
            Rectangle(72, 72, 144, 144),
            Fan(Colour.FromArgb(0x40FF0000))));

        // A shading's colour space is DeviceRGB and carries no alpha, so the alpha is a second
        // shading in DeviceGray read as a luminosity mask — the machinery a fading gradient
        // already had, shared so that a faded mesh and a faded gradient cannot disagree.
        pdf.Text.ShouldContain("/SMask<</S/Luminosity");
        pdf.Text.ShouldContain("/ShadingType 4/ColorSpace/DeviceGray");
    }

    [Fact]
    public void AMeshTriangleNamingAMissingVertexIsSkippedRatherThanFatal()
    {
        MeshPaint broken = new(
            [
                new MeshVertex(new DocPoint(Pt(72), Pt(72)), Red),
                new MeshVertex(new DocPoint(Pt(216), Pt(72)), Green),
                new MeshVertex(new DocPoint(Pt(216), Pt(216)), Blue),
            ],
            [new MeshTriangle(0, 1, 2), new MeshTriangle(0, 1, 9), new MeshTriangle(0, 0, 1)]);

        byte[] stream = Write(sink => sink.FillPath(Rectangle(72, 72, 144, 144), broken))
            .Streams()
            .First(entry => entry.Dictionary.Contains("/ShadingType 4", StringComparison.Ordinal))
            .Data;

        // A mesh is generated from a document's own geometry rather than authored, so a
        // boundary with a doubled point produces a degenerate triple and it is dropped.
        stream.Length.ShouldBe(3 * 12);
    }

    // ---------------------------------------------------------------------------- the spread

    [Theory]
    [InlineData(SpreadMethod.Repeat)]
    [InlineData(SpreadMethod.Reflect)]
    public void ARepeatingGradientLengthensTheShadingsAxisRatherThanExtendingIt(SpreadMethod spread)
    {
        // PDF's /Extend clamps the parameter and cannot repeat it, so the only way to spell a
        // repeat is an axis lengthened over as many periods as the shape spans, with a domain
        // and a stitching function to match. Skia states the same thing as a tile mode.
        PdfFile pdf = Write(sink => sink.FillPath(
            Rectangle(72, 72, 288, 72),
            Ramp(new DocPoint(Pt(72), Pt(72)), new DocPoint(Pt(108), Pt(72)), spread)));

        pdf.Text.ShouldContain("/ShadingType 2");
        pdf.Text.ShouldContain("/Domain[0 8]");
        pdf.Text.ShouldContain("/Bounds[1 2 3 4 5 6 7]");
    }

    [Fact]
    public void AReflectingGradientReversesEveryOtherPeriodsEncode()
    {
        PdfFile pdf = Write(sink => sink.FillPath(
            Rectangle(72, 72, 288, 72),
            Ramp(new DocPoint(Pt(72), Pt(72)), new DocPoint(Pt(108), Pt(72)), SpreadMethod.Reflect)));

        // A reversed /Encode is how PDF spells a mirrored copy without a second function.
        pdf.Text.ShouldContain("/Encode[0 1 1 0 0 1 1 0 0 1 1 0 0 1 1 0]");
    }

    [Fact]
    public void APaddedGradientStillWritesTheSimpleDomain()
    {
        PdfFile pdf = Write(sink => sink.FillPath(
            Rectangle(72, 72, 288, 72),
            Ramp(new DocPoint(Pt(72), Pt(72)), new DocPoint(Pt(108), Pt(72)), SpreadMethod.Pad)));

        // Nothing about a padded gradient changed, which is the point of the default.
        pdf.Text.ShouldNotContain("/Domain[0 8]");
        pdf.Text.ShouldContain("/Extend[true true]");
    }

    [Theory]
    [InlineData(SpreadMethod.Repeat, 255, 0)]
    [InlineData(SpreadMethod.Reflect, 0, 255)]
    public void ARepeatRestartsTheRampWhereAReflectTurnsItRound(
        SpreadMethod spread, int expectedRed, int expectedBlue)
    {
        // One 36 pt period from red to blue, sampled 6 pt into the second period. A repeat is
        // near red again there; a reflect is still near blue, because the second period runs
        // backwards.
        using SKBitmap page = Rasterise(sink => sink.FillPath(
            Rectangle(72, 72, 288, 72),
            Ramp(new DocPoint(Pt(72), Pt(72)), new DocPoint(Pt(108), Pt(72)), spread)));

        SKColor sample = page.GetPixel(114, 108);

        if (expectedRed > 128) sample.Red.ShouldBeGreaterThan((byte)150);
        else sample.Red.ShouldBeLessThan((byte)105);

        if (expectedBlue > 128) sample.Blue.ShouldBeGreaterThan((byte)150);
        else sample.Blue.ShouldBeLessThan((byte)105);
    }

    // ----------------------------------------------------------------------------- the focus

    [Fact]
    public void AFocalRadialWritesTwoCirclesWithDifferentCentres()
    {
        PdfFile pdf = Write(sink => sink.FillPath(
            Rectangle(72, 72, 144, 144),
            new GradientPaint(
                GradientKind.Radial,
                [new GradientStop(0, Red), new GradientStop(1, Blue)],
                new DocPoint(Pt(144), Pt(144)),
                new DocPoint(Pt(216), Pt(144)),
                AffineTransform.Identity,
                SpreadMethod.Pad,
                new DocPoint(Pt(108), Pt(108)))));

        // /ShadingType 3 already took two circles; a focus is the inner one collapsed to a
        // point somewhere other than the centre, which is exact rather than an approximation —
        // so nothing here is a second code path.
        pdf.Text.ShouldContain("/ShadingType 3");
        pdf.Text.ShouldContain("/Coords[108 733.8898 0 144 697.8898 72]");
    }

    [Fact]
    public void AFocalRadialPutsItsFirstStopAtTheFocusRatherThanTheCentre()
    {
        using SKBitmap page = Rasterise(sink => sink.FillPath(
            Rectangle(72, 72, 144, 144),
            new GradientPaint(
                GradientKind.Radial,
                [new GradientStop(0, Red), new GradientStop(1, Blue)],
                new DocPoint(Pt(144), Pt(144)),
                new DocPoint(Pt(216), Pt(144)),
                AffineTransform.Identity,
                SpreadMethod.Pad,
                new DocPoint(Pt(108), Pt(108)))));

        // Red at the focus, not at the geometric centre — which is the whole visible
        // consequence of the focus and the thing PL6018 used to report as lost.
        page.GetPixel(108, 108).Red.ShouldBeGreaterThan((byte)230);
        page.GetPixel(144, 144).Red.ShouldBeLessThan((byte)200);
    }

    // ---------------------------------------------------------------------------- helpers

    /// <summary>
    /// Four triangles fanned about a white centre, with a primary at each corner of a square:
    /// the shape a path-gradient brush makes and the smallest one no stop list can state.
    /// </summary>
    private static MeshPaint Fan(Colour? first = null) => new(
        [
            new MeshVertex(new DocPoint(Pt(144), Pt(144)), Colour.FromRgb(0xFFFFFF)),
            new MeshVertex(new DocPoint(Pt(72), Pt(72)), first ?? Red),
            new MeshVertex(new DocPoint(Pt(216), Pt(72)), Green),
            new MeshVertex(new DocPoint(Pt(216), Pt(216)), Blue),
            new MeshVertex(new DocPoint(Pt(72), Pt(216)), Green),
        ],
        [
            new MeshTriangle(0, 1, 2),
            new MeshTriangle(0, 2, 3),
            new MeshTriangle(0, 3, 4),
            new MeshTriangle(0, 4, 1),
        ]);

    private static GradientPaint Ramp(DocPoint from, DocPoint to, SpreadMethod spread) => new(
        GradientKind.Linear,
        [new GradientStop(0, Red), new GradientStop(1, Blue)],
        from,
        to,
        AffineTransform.Identity,
        spread);

    private static GraphicsPath Rectangle(double x, double y, double width, double height)
        => GraphicsPath.Rectangle(new DocRect(Pt(x), Pt(y), Pt(width), Pt(height)));

    private static Length Pt(double value) => Length.FromPoints(value);

    private static PdfFile Write(Action<IDrawingSink> draw)
    {
        using MemoryStream buffer = new();
        new PdfRenderer(Reproducible).Render(
            new DrawnPages(new DrawnPage(DrawnPage.A4, draw)), buffer);

        return PdfFile.Parse(buffer.ToArray());
    }

    private static SKBitmap Rasterise(Action<IDrawingSink> draw)
        => new RasterRenderer(new RasterRenderOptions { Dpi = 72, Antialias = false })
            .Rasterise(new DrawnPage(DrawnPage.A4, draw));
}
