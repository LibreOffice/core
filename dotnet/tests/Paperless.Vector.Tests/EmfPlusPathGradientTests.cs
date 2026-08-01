using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Shouldly;

namespace Paperless.Vector.Tests;

/// <summary>
/// A GDI+ path gradient, which is the brush that made <c>Paperless.Core</c> grow a
/// <see cref="MeshPaint"/>.
/// </summary>
/// <remarks>
/// <para>
/// The brush states a centre colour and a colour at <em>every vertex of an arbitrary
/// boundary</em>, Gouraud-shaded between them. No number of <see cref="GradientStop"/>s says
/// that — a ramp has one colour at each end whatever sits between — so before the mesh existed
/// this was drawn as the ramp from the centre to the first surround colour, with
/// <c>PL6040</c> when they differed.
/// </para>
/// <para>
/// Every fixture uses the same reference device as <c>EmfPlusDrawingTests</c>: 8000 pixels to
/// 80 mm, so one world unit is exactly 1/100 mm and an assertion can name the millimetre.
/// </para>
/// </remarks>
public sealed class EmfPlusPathGradientTests
{
    private const uint Red = 0xFFFF0000;
    private const uint Green = 0xFF00FF00;
    private const uint Blue = 0xFF0000FF;
    private const uint White = 0xFFFFFFFF;

    [Fact]
    public void EachBoundaryVertexKeepsItsOwnColour()
    {
        MeshPaint mesh = Sweep([Red, Green, Blue]);

        // One centre plus one vertex per boundary point, and the boundary colours are the
        // brush's own rather than a ramp between two of them.
        mesh.Vertices.Count.ShouldBe(4);
        mesh.Vertices[0].Colour.ShouldBe(new Colour(255, 255, 255));
        mesh.Vertices[1].Colour.ShouldBe(new Colour(255, 0, 0));
        mesh.Vertices[2].Colour.ShouldBe(new Colour(0, 255, 0));
        mesh.Vertices[3].Colour.ShouldBe(new Colour(0, 0, 255));
    }

    [Fact]
    public void TheBoundaryIsPartitionedIntoAFanAboutTheCentre()
    {
        MeshPaint mesh = Sweep([Red, Green, Blue]);

        // (centre, V(i), V(i+1)) for every segment, wrapping — so a triangle count equal to the
        // vertex count rather than two fewer, which is what a triangulation that does not close
        // the boundary produces.
        mesh.Triangles.Count.ShouldBe(3);
        mesh.Triangles.ShouldAllBe(triangle => triangle.A == 0);
        mesh.Triangles[^1].C.ShouldBe(1);
    }

    [Fact]
    public void OneSurroundColourAppliesToEveryVertex()
    {
        MeshPaint mesh = Sweep([Blue]);

        // The common case, and the one that reads as a radial gradient in the first place.
        mesh.Vertices[1].Colour.ShouldBe(new Colour(0, 0, 255));
        mesh.Vertices[2].Colour.ShouldBe(new Colour(0, 0, 255));
        mesh.Vertices[3].Colour.ShouldBe(new Colour(0, 0, 255));
    }

    [Fact]
    public void TheVerticesAreMappedIntoTheDocumentsOwnCoordinates()
    {
        MeshPaint mesh = Sweep([Blue]);

        // One world unit is 1/100 mm under this fixture's reference device, so a boundary at
        // 1000 world units is at 10 mm. A mesh in world coordinates would be a hundred times
        // too large and would not show as a scaling bug — it would show as nothing drawn.
        mesh.Vertices[0].Position.X.Millimetres.ShouldBe(20, 0.01);
        mesh.Vertices[1].Position.X.Millimetres.ShouldBe(10, 0.01);
        mesh.Vertices[3].Position.Y.Millimetres.ShouldBe(30, 0.01);
    }

    [Fact]
    public void APathGradientNoLongerReportsThatItCannotStateItsColours()
    {
        VectorImage image = Build(new EmfPlusBuilder()
            .Header()
            .PathGradientBrush(1, (2000, 2000), White, Boundary, [Red, Green, Blue])
            .FillRectsWithBrush(1, (1000, 1000, 2000, 2000))
            .End());

        // PL6040 retired when the mesh arrived; it now means only a boundary too degenerate to
        // fan, which no well-formed brush states.
        image.Diagnostics.ShouldNotContain(diagnostic => diagnostic.Code == "PL6040");
    }

    [Fact]
    public void ABoundaryOfFewerThanThreePointsFallsBackToARampAndSaysSo()
    {
        VectorImage image = Build(new EmfPlusBuilder()
            .Header()
            .PathGradientBrush(1, (2000, 2000), White, [(1000, 1000), (3000, 3000)], [Red, Blue])
            .FillRectsWithBrush(1, (1000, 1000, 2000, 2000))
            .End());

        // There is no interior to partition and nowhere for a colour per vertex to sit, so the
        // bounding-ellipse ramp this reader drew for everything before the mesh stays as the
        // fallback rather than nothing being drawn.
        image.Diagnostics.ShouldContain(diagnostic => diagnostic.Code == "PL6040");
    }

    [Fact]
    public void AShapeReachingPastTheBoundaryIsUndercoatedInTheEdgeColour()
    {
        Recorder sink = Draw(new EmfPlusBuilder()
            .Header()
            .PathGradientBrush(1, (2000, 2000), White, Boundary, [Blue])
            .FillRectsWithBrush(1, (0, 0, 8000, 8000))
            .End());

        // GDI+ clamps the centre-to-edge parameter at 1 outside the sweep rather than leaving
        // it unpainted — LibreOffice's second rasterisation pass. Without the undercoat
        // tdf143031_BrushPathGrad, whose boundary sits at 77-148 mm and whose shape sits at
        // 1-11 mm, came out empty at ink_ratio 0.000.
        sink.Fills.Count.ShouldBe(2);
        sink.Fills[0].Paint.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(new Colour(0, 0, 255));
        sink.Fills[1].Paint.ShouldBeOfType<MeshPaint>();
    }

    [Fact]
    public void AShapeInsideTheBoundaryIsNotUndercoated()
    {
        Recorder sink = Draw(new EmfPlusBuilder()
            .Header()
            .PathGradientBrush(1, (2000, 2000), White, Boundary, [Blue])
            .FillRectsWithBrush(1, (1500, 1500, 500, 500))
            .End());

        sink.Fills.ShouldHaveSingleItem().Paint.ShouldBeOfType<MeshPaint>();
    }

    // ---------------------------------------------------------------- helpers

    /// <summary>A triangle from 10 mm to 30 mm, whose centre of gravity is the brush's centre.</summary>
    private static readonly (float X, float Y)[] Boundary =
        [(1000, 1000), (3000, 1000), (2000, 3000)];

    private static MeshPaint Sweep(uint[] surround)
    {
        Recorder sink = Draw(new EmfPlusBuilder()
            .Header()
            .PathGradientBrush(1, (2000, 2000), White, Boundary, surround)
            .FillRectsWithBrush(1, (1000, 1000, 2000, 2000))
            .End());

        return sink.Fills.ShouldHaveSingleItem().Paint.ShouldBeOfType<MeshPaint>();
    }

    private static VectorImage Build(EmfPlusBuilder plus) => new EmfBuilder().Plus(plus).Decode();

    private static Recorder Draw(EmfPlusBuilder plus)
    {
        VectorImage image = Build(plus);
        Recorder sink = new();
        image.Draw(sink, new DocRect(DocPoint.Origin, image.IntrinsicSize));
        return sink;
    }
}
