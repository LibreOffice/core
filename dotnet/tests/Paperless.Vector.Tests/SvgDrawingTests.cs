using System.Text;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Vector.Svg;
using Shouldly;

namespace Paperless.Vector.Tests;

/// <summary>
/// What an SVG turns into: paths, paints, transforms, clips, text and embedded rasters.
/// </summary>
/// <remarks>
/// Assertions are on final document coordinates rather than on the shape of the command
/// stream, because the command stream is the library's business and the coordinates are
/// ours. A test that pinned the exact sequence of <c>Save</c>/<c>Transform</c> calls would
/// break on a library upgrade that drew the same picture differently, which is the wrong
/// thing to be sensitive to.
/// </remarks>
public sealed class SvgDrawingTests
{
    private static readonly SvgImageDecoder Decoder = new();

    [Fact]
    public void ARectangleLandsWhereItsUserCoordinatesSay()
    {
        Recorder sink = Draw("""
            <svg xmlns="http://www.w3.org/2000/svg" width="200" height="100">
              <rect x="20" y="10" width="60" height="30" fill="#ff0000"/>
            </svg>
            """);

        (DocRect bounds, Paint paint, _) = sink.Fills.ShouldHaveSingleItem();

        // The picture is drawn into a rectangle equal to its own view box, so user units map
        // one for one onto CSS pixels and a pixel is 1/96 inch — 9525 EMU exactly.
        bounds.Left.Emu.ShouldBe(20 * 9525);
        bounds.Top.Emu.ShouldBe(10 * 9525);
        bounds.Width.Emu.ShouldBe(60 * 9525);
        bounds.Height.Emu.ShouldBe(30 * 9525);

        paint.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(Colour.FromRgb(0xFF0000));
    }

    [Fact]
    public void AViewBoxScalesTheContentOntoTheViewport()
    {
        // Twice the view box in each direction, so every coordinate should halve.
        Recorder sink = Draw("""
            <svg xmlns="http://www.w3.org/2000/svg" width="100" height="50" viewBox="0 0 200 100">
              <rect x="20" y="10" width="60" height="30" fill="#ff0000"/>
            </svg>
            """);

        (DocRect bounds, _, _) = sink.Fills.ShouldHaveSingleItem();

        bounds.Left.Emu.ShouldBe(10 * 9525);
        bounds.Width.Emu.ShouldBe(30 * 9525);
    }

    [Fact]
    public void NestedTransformsCompose()
    {
        Recorder sink = Draw("""
            <svg xmlns="http://www.w3.org/2000/svg" width="200" height="200">
              <g transform="translate(50,20)">
                <g transform="scale(2)">
                  <rect x="10" y="5" width="10" height="10" fill="#000000"/>
                </g>
              </g>
            </svg>
            """);

        (DocRect bounds, _, _) = sink.Fills.ShouldHaveSingleItem();

        // translate then scale: (10,5) -> (20,10) -> (70,30), and the 10x10 becomes 20x20.
        bounds.Left.Emu.ShouldBe(70 * 9525);
        bounds.Top.Emu.ShouldBe(30 * 9525);
        bounds.Width.Emu.ShouldBe(20 * 9525);
    }

    [Fact]
    public void AStrokeKeepsItsWidthCapJoinAndDashes()
    {
        Recorder sink = Draw("""
            <svg xmlns="http://www.w3.org/2000/svg" width="200" height="100">
              <path d="M10,10 L90,10" fill="none" stroke="#0000ff" stroke-width="4"
                    stroke-linecap="round" stroke-linejoin="bevel" stroke-dasharray="8 3"/>
            </svg>
            """);

        (_, Stroke stroke) = sink.Strokes.ShouldHaveSingleItem();

        stroke.Width.Emu.ShouldBe(4 * 9525);
        stroke.Cap.ShouldBe(LineCap.Round);
        stroke.Join.ShouldBe(LineJoin.Bevel);
        stroke.Paint.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(Colour.FromRgb(0x0000FF));
        stroke.DashPattern.ShouldNotBeNull().Count.ShouldBe(2);
        stroke.DashPattern[0].Emu.ShouldBe(8 * 9525);
        stroke.DashPattern[1].Emu.ShouldBe(3 * 9525);
    }

    [Fact]
    public void ALinearGradientKeepsItsStopsInOrder()
    {
        Recorder sink = Draw("""
            <svg xmlns="http://www.w3.org/2000/svg" width="200" height="100">
              <defs><linearGradient id="g" x1="0" y1="0" x2="1" y2="0">
                <stop offset="0" stop-color="#ff0000"/>
                <stop offset="0.5" stop-color="#00ff00"/>
                <stop offset="1" stop-color="#0000ff" stop-opacity="0.5"/>
              </linearGradient></defs>
              <rect x="0" y="0" width="100" height="50" fill="url(#g)"/>
            </svg>
            """);

        GradientPaint gradient = sink.Fills.ShouldHaveSingleItem().Paint.ShouldBeOfType<GradientPaint>();

        gradient.Kind.ShouldBe(GradientKind.Linear);
        gradient.Stops.Count.ShouldBe(3);
        gradient.Stops[0].Colour.ShouldBe(Colour.FromRgb(0xFF0000));
        gradient.Stops[1].Offset.ShouldBe(0.5, 1e-6);
        gradient.Stops[2].Colour.A.ShouldBeInRange((byte)126, (byte)129);
    }

    [Fact]
    public void ARadialGradientBecomesARadialPaint()
    {
        Recorder sink = Draw("""
            <svg xmlns="http://www.w3.org/2000/svg" width="200" height="100">
              <defs><radialGradient id="r" cx="0.5" cy="0.5" r="0.5">
                <stop offset="0" stop-color="#ffffff"/><stop offset="1" stop-color="#000000"/>
              </radialGradient></defs>
              <rect x="0" y="0" width="100" height="100" fill="url(#r)"/>
            </svg>
            """);

        sink.Fills.ShouldHaveSingleItem().Paint.ShouldBeOfType<GradientPaint>().Kind.ShouldBe(GradientKind.Radial);
    }

    [Fact]
    public void AClipPathBecomesAClipRatherThanBeingIgnored()
    {
        Recorder sink = Draw("""
            <svg xmlns="http://www.w3.org/2000/svg" width="200" height="100">
              <defs><clipPath id="c"><rect x="0" y="0" width="50" height="50"/></clipPath></defs>
              <g clip-path="url(#c)"><rect x="0" y="0" width="200" height="100" fill="#ff0000"/></g>
            </svg>
            """);

        // The outermost clip is the frame the image is drawn into; the SVG's own is the second.
        sink.Clips.Count.ShouldBeGreaterThanOrEqualTo(2);
        sink.Clips.ShouldContain(clip => clip.Width.Emu == 50 * 9525 && clip.Height.Emu == 50 * 9525);
    }

    [Fact]
    public void GroupOpacityBecomesATransparencyGroup()
    {
        Recorder sink = Draw("""
            <svg xmlns="http://www.w3.org/2000/svg" width="200" height="100">
              <g opacity="0.4"><rect x="0" y="0" width="50" height="50" fill="#ff0000"/></g>
            </svg>
            """);

        sink.Groups.ShouldHaveSingleItem().ShouldBe(0.4, 0.01);
    }

    [Fact]
    public void FillOpacityStaysOnTheColourRatherThanBecomingAGroup()
    {
        // A per-shape alpha and a group alpha are different pictures wherever shapes overlap,
        // so conflating them would be wrong even though one shape cannot show the difference.
        Recorder sink = Draw("""
            <svg xmlns="http://www.w3.org/2000/svg" width="200" height="100">
              <rect x="0" y="0" width="50" height="50" fill="#ff0000" fill-opacity="0.5"/>
            </svg>
            """);

        sink.Groups.ShouldBeEmpty();
        sink.Fills.ShouldHaveSingleItem().Paint.ShouldBeOfType<SolidPaint>().Colour.A
            .ShouldBeInRange((byte)126, (byte)129);
    }

    [Fact]
    public void ATextRunIsShapedByPaperlessAndKeepsItsCharacters()
    {
        Recorder sink = Draw("""
            <svg xmlns="http://www.w3.org/2000/svg" width="300" height="100">
              <text x="20" y="40" font-family="Liberation Sans" font-size="16" fill="#123456">Hello</text>
            </svg>
            """);

        (string text, DocPoint origin, Length size, string family, Paint paint) = sink.Runs.ShouldHaveSingleItem();

        text.ShouldBe("Hello");
        origin.X.Emu.ShouldBe(20 * 9525);
        origin.Y.Emu.ShouldBe(40 * 9525);
        size.Emu.ShouldBe(16 * 9525);
        family.ShouldBe("Liberation Sans");
        paint.ShouldBeOfType<SolidPaint>().Colour.ShouldBe(Colour.FromRgb(0x123456));
    }

    [Fact]
    public void ACentredTextRunIsCentredOnItsAnchor()
    {
        // The library resolves text-anchor itself, from the width our own shaper measured, so
        // this asserts the measurement as much as the anchoring: a shaper that answered
        // differently would put the run somewhere else entirely.
        Recorder sink = Draw("""
            <svg xmlns="http://www.w3.org/2000/svg" width="300" height="100">
              <text x="150" y="40" font-family="Liberation Sans" font-size="16"
                    text-anchor="middle">Centred</text>
              <text x="150" y="70" font-family="Liberation Sans" font-size="16">Centred</text>
            </svg>
            """);

        sink.Runs.Count.ShouldBe(2);

        Length centred = sink.Runs[0].Origin.X;
        Length startAnchored = sink.Runs[1].Origin.X;

        centred.ShouldBeLessThan(startAnchored);

        // Half the run's width to the left of the anchor, and the run is not empty.
        Length shift = startAnchored - centred;
        shift.Emu.ShouldBeGreaterThan(10 * 9525);
        shift.Emu.ShouldBeLessThan(40 * 9525);
    }

    [Fact]
    public void AnEmbeddedDataUriImageIsHandedOnStillEncoded()
    {
        Recorder sink = Draw($"""
            <svg xmlns="http://www.w3.org/2000/svg" width="200" height="100">
              <image x="10" y="10" width="80" height="80" href="data:image/png;base64,{Png}"/>
            </svg>
            """);

        (RasterImage image, DocRect destination, _) = sink.Images.ShouldHaveSingleItem();

        // Undecoded, so nothing in Paperless.Vector needs a codec.
        image.IsDecoded.ShouldBeFalse();
        image.EncodedBytes.Length.ShouldBeGreaterThan(0);

        // Square source in an 80x80 box, so it fills it.
        destination.Width.Emu.ShouldBe(80 * 9525);
        destination.Height.Emu.ShouldBe(80 * 9525);
    }

    [Fact]
    public void AnEllipseBecomesCurvesRatherThanBeingDropped()
    {
        Recorder sink = Draw("""
            <svg xmlns="http://www.w3.org/2000/svg" width="200" height="100">
              <ellipse cx="100" cy="50" rx="40" ry="20" fill="#008000"/>
            </svg>
            """);

        (DocRect bounds, _, _) = sink.Fills.ShouldHaveSingleItem();

        // Bezier control points sit outside the ellipse, so the recorded extent is a little
        // wider than the true one — within a fifth of a per cent for a quarter-turn cubic.
        bounds.Width.Emu.ShouldBeInRange(80 * 9525, (long)(80 * 9525 * 1.11));
        bounds.Height.Emu.ShouldBeInRange(40 * 9525, (long)(40 * 9525 * 1.11));
    }

    [Fact]
    public void AnEllipticalArcIsFlattenedIntoTheRightQuadrant()
    {
        // The arc flag pair decides which side of the chord the centre falls on. Getting it
        // wrong mirrors the arc about its chord, which looks like a coordinate-system bug.
        Recorder sink = Draw("""
            <svg xmlns="http://www.w3.org/2000/svg" width="200" height="200">
              <path d="M100,100 A50,50 0 0,1 150,150" fill="none" stroke="#000000" stroke-width="1"/>
            </svg>
            """);

        (DocRect bounds, _) = sink.Strokes.ShouldHaveSingleItem();

        // Sweeping clockwise from (100,100) to (150,150) bulges right and up, so the arc must
        // stay at or above y=100 and reach x=150.
        bounds.Left.Emu.ShouldBe(100 * 9525);
        bounds.Right.Emu.ShouldBeGreaterThanOrEqualTo(150 * 9525);
        bounds.Top.Emu.ShouldBeGreaterThanOrEqualTo((long)(99.5 * 9525));
    }

    [Fact]
    public void ADecodedImageCanBeReplayedMoreThanOnce()
    {
        // The reason decoding produces a display list rather than drawing straight through:
        // the same logo appears on every slide and should cost one decode.
        VectorImage image = Decoder.Decode(Bytes("""
            <svg xmlns="http://www.w3.org/2000/svg" width="100" height="100">
              <rect x="0" y="0" width="10" height="10" fill="#000000"/>
            </svg>
            """));

        Recorder first = new();
        Recorder second = new();

        image.Draw(first, new DocRect(default, image.IntrinsicSize));
        image.Draw(second, new DocRect(default, image.IntrinsicSize));

        first.Fills.Count.ShouldBe(1);
        second.Fills.Count.ShouldBe(1);
        second.Fills[0].Bounds.ShouldBe(first.Fills[0].Bounds);
    }

    // ------------------------------------------------------------------------------- helpers

    /// <summary>A four-by-four blue PNG, base64-encoded.</summary>
    private const string Png =
        "iVBORw0KGgoAAAANSUhEUgAAAAQAAAAECAIAAAAmkwkpAAAAD0lEQVR4nGNgaPiPQMRxAF7CF/FUdRSEAAAAAElFTkSuQmCC";

    private static ReadOnlyMemory<byte> Bytes(string svg) => Encoding.UTF8.GetBytes(svg);

    /// <summary>Decodes an SVG and draws it at its intrinsic size, from the origin.</summary>
    private static Recorder Draw(string svg)
    {
        VectorImage image = Decoder.Decode(Bytes(svg));
        Recorder sink = new();

        image.Draw(sink, new DocRect(DocPoint.Origin, image.IntrinsicSize));
        return sink;
    }
}
