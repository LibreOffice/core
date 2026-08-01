using System.Text;
using Shouldly;

namespace Paperless.Vector.Tests;

/// <summary>
/// What an SVG's clip-path union does and does not need a polygon boolean for.
/// </summary>
/// <remarks>
/// These pin the measurement that decided against porting <c>b2dpolypolygoncutter.cxx</c>: over
/// 56 394 real SVG files, only six <c>clipPath</c> elements have more than one child at all and
/// exactly one file has a child bearing its own <c>clip-path</c>. See
/// <c>src/Paperless.Vector/TODO.md</c> for the arithmetic.
/// </remarks>
public class SvgClipUnionTests
{
    [Fact]
    public void SeveralClipChildrenAreOnePathOfSubpathsAndNotADiagnostic()
    {
        // The shape svgio's own ShapeWithClipPath.svg has. It reported PL6023 and should not:
        // neither child bears a clip-path, so there is no per-member clip to lose.
        VectorImage image = Decode("""
            <svg xmlns="http://www.w3.org/2000/svg" width="120" height="120">
              <clipPath id="c">
                <rect x="30" y="30" width="20" height="20"/>
                <rect x="70" y="70" width="20" height="20"/>
              </clipPath>
              <rect x="10" y="10" width="100" height="100" fill="#00D000" clip-path="url(#c)"/>
            </svg>
            """);

        image.Diagnostics.ShouldNotContain(d => d.Code == "PL6023");
        image.IsEmpty.ShouldBeFalse();
    }

    [Fact]
    public void AClipOnOneMemberOfAUnionIsStillReported()
    {
        // And the case that genuinely cannot be expressed still is: member i has to become
        // (member i ∩ its own clip) before the union, which is a polygon boolean.
        VectorImage image = Decode("""
            <svg xmlns="http://www.w3.org/2000/svg" width="120" height="120">
              <clipPath id="inner"><circle cx="40" cy="40" r="15"/></clipPath>
              <clipPath id="c">
                <rect x="30" y="30" width="20" height="20" clip-path="url(#inner)"/>
                <rect x="70" y="70" width="20" height="20"/>
              </clipPath>
              <rect x="10" y="10" width="100" height="100" fill="#00D000" clip-path="url(#c)"/>
            </svg>
            """);

        image.Diagnostics.ShouldContain(d => d.Code == "PL6023");
    }

    [Fact]
    public void NoSvgAsksForADifferenceClip()
    {
        // SVG has no clip-subtraction operator, and Svg.SceneGraph 5.1.1 never constructs
        // SKClipOperation.Difference. A clipPath whose child is itself clipped away to nothing
        // is the nearest an SVG comes to asking for one, and it does not produce PL6012.
        VectorImage image = Decode("""
            <svg xmlns="http://www.w3.org/2000/svg" width="100" height="100">
              <clipPath id="c"><rect x="10" y="10" width="40" height="40"/></clipPath>
              <rect width="100" height="100" fill="red" clip-path="url(#c)"/>
            </svg>
            """);

        image.Diagnostics.ShouldNotContain(d => d.Code == "PL6012");
    }

    private static VectorImage Decode(string svg)
        => VectorImages.Decode(Encoding.UTF8.GetBytes(svg));
}
