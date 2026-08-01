using System.Text;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Vector.Svg;
using Shouldly;

namespace Paperless.Vector.Tests;

/// <summary>
/// The two gradient features SVG shares with EMF+, and which used to be reported rather than
/// drawn: <c>spreadMethod</c> and the <c>fx</c>/<c>fy</c> focus.
/// </summary>
/// <remarks>
/// Both were recorded twice — as <c>PL6021</c> and <c>PL6018</c> here, and as <c>PL6041</c>
/// and the path-gradient note in EMF+ — which is what made them worth closing in
/// <c>Paperless.Core</c> rather than working round per format. Nothing in either backend
/// approximates them: Skia has a tile mode and a two-point conical shader, and PDF has a
/// lengthened shading axis and a two-circle <c>/ShadingType 3</c>.
/// </remarks>
public sealed class SvgGradientTests
{
    private static readonly SvgImageDecoder Decoder = new();

    [Theory]
    [InlineData("pad", SpreadMethod.Pad)]
    [InlineData("reflect", SpreadMethod.Reflect)]
    [InlineData("repeat", SpreadMethod.Repeat)]
    public void ASpreadMethodReachesThePaint(string spelling, SpreadMethod expected)
    {
        Recorder sink = Draw($"""
            <svg xmlns="http://www.w3.org/2000/svg" width="200" height="100">
              <defs>
                <linearGradient id="g" x1="0" y1="0" x2="0.2" y2="0" spreadMethod="{spelling}">
                  <stop offset="0" stop-color="#ff0000"/>
                  <stop offset="1" stop-color="#0000ff"/>
                </linearGradient>
              </defs>
              <rect x="0" y="0" width="200" height="100" fill="url(#g)"/>
            </svg>
            """);

        GradientPaint gradient = sink.Fills.ShouldHaveSingleItem().Paint.ShouldBeOfType<GradientPaint>();
        gradient.Spread.ShouldBe(expected);
    }

    [Fact]
    public void ARepeatingGradientNoLongerReportsThatItWasClamped()
    {
        VectorImage image = Decoder.Decode(Bytes("""
            <svg xmlns="http://www.w3.org/2000/svg" width="200" height="100">
              <defs>
                <linearGradient id="g" x1="0" y1="0" x2="0.2" y2="0" spreadMethod="repeat">
                  <stop offset="0" stop-color="#ff0000"/>
                  <stop offset="1" stop-color="#0000ff"/>
                </linearGradient>
              </defs>
              <rect x="0" y="0" width="200" height="100" fill="url(#g)"/>
            </svg>
            """));

        image.Diagnostics.ShouldNotContain(diagnostic => diagnostic.Code == "PL6021");
    }

    [Fact]
    public void AFocalRadialCarriesItsFocusRatherThanBeingCentred()
    {
        Recorder sink = Draw("""
            <svg xmlns="http://www.w3.org/2000/svg" width="100" height="100">
              <defs>
                <radialGradient id="g" gradientUnits="userSpaceOnUse"
                                cx="50" cy="50" r="40" fx="30" fy="30">
                  <stop offset="0" stop-color="#ff0000"/>
                  <stop offset="1" stop-color="#0000ff"/>
                </radialGradient>
              </defs>
              <rect x="0" y="0" width="100" height="100" fill="url(#g)"/>
            </svg>
            """);

        GradientPaint gradient = sink.Fills.ShouldHaveSingleItem().Paint.ShouldBeOfType<GradientPaint>();

        // A user unit is 1/96 inch, so 30 units is 7.94 mm and 50 is 13.23 mm. The focus is a
        // separate point from the centre, which is the whole visible consequence of fx/fy.
        gradient.Focus.ShouldNotBeNull();
        gradient.Focus!.Value.X.Millimetres.ShouldBe(7.94, 0.02);
        gradient.Focus!.Value.Y.Millimetres.ShouldBe(7.94, 0.02);
        gradient.Start.X.Millimetres.ShouldBe(13.23, 0.02);
    }

    [Fact]
    public void AFocalRadialNoLongerReportsThatItWasCentred()
    {
        VectorImage image = Decoder.Decode(Bytes("""
            <svg xmlns="http://www.w3.org/2000/svg" width="100" height="100">
              <defs>
                <radialGradient id="g" gradientUnits="userSpaceOnUse"
                                cx="50" cy="50" r="40" fx="30" fy="30">
                  <stop offset="0" stop-color="#ff0000"/>
                  <stop offset="1" stop-color="#0000ff"/>
                </radialGradient>
              </defs>
              <rect x="0" y="0" width="100" height="100" fill="url(#g)"/>
            </svg>
            """));

        image.Diagnostics.ShouldNotContain(diagnostic => diagnostic.Code == "PL6018");
    }

    [Fact]
    public void ARadialWithNoFocusStatesNone()
    {
        Recorder sink = Draw("""
            <svg xmlns="http://www.w3.org/2000/svg" width="100" height="100">
              <defs>
                <radialGradient id="g" gradientUnits="userSpaceOnUse" cx="50" cy="50" r="40">
                  <stop offset="0" stop-color="#ff0000"/>
                  <stop offset="1" stop-color="#0000ff"/>
                </radialGradient>
              </defs>
              <rect x="0" y="0" width="100" height="100" fill="url(#g)"/>
            </svg>
            """);

        // A focus equal to the centre is spelled as no focus at all, so a plain radial keeps
        // taking the one-circle path in both backends rather than a degenerate two-circle one.
        sink.Fills.ShouldHaveSingleItem().Paint.ShouldBeOfType<GradientPaint>().Focus.ShouldBeNull();
    }

    private static ReadOnlyMemory<byte> Bytes(string svg) => Encoding.UTF8.GetBytes(svg);

    private static Recorder Draw(string svg)
    {
        VectorImage image = Decoder.Decode(Bytes(svg));
        Recorder sink = new();

        image.Draw(sink, new DocRect(DocPoint.Origin, image.IntrinsicSize));
        return sink;
    }
}
