using Paperless.Core.Geometry;
using Paperless.Core.Units;

namespace Paperless.Core.Graphics;

/// <summary>
/// How to fill a region: a colour, a gradient, or a tiled bitmap.
/// </summary>
/// <remarks>
/// Deliberately narrower than what the source formats express. LibreOffice supports
/// hatch and pattern fills as first-class kinds (<c>XFillStyleItem</c>); Paperless
/// resolves those into a tiled <see cref="BitmapPaint"/> or an explicit set of
/// stroked lines at read time, so backends have three cases instead of six.
/// </remarks>
public abstract record Paint
{
    /// <summary>A uniform colour fill.</summary>
    public static Paint Solid(Colour colour) => new SolidPaint(colour);
}

/// <summary>A uniform colour fill.</summary>
/// <param name="Colour">The colour.</param>
public sealed record SolidPaint(Colour Colour) : Paint;

/// <summary>
/// A gradient fill.
/// </summary>
/// <param name="Kind">The gradient's geometry.</param>
/// <param name="Stops">
/// The colour stops, sorted by offset. At least two are required.
/// </param>
/// <param name="Start">
/// For a linear gradient, the start point. For radial and elliptical gradients, the
/// centre.
/// </param>
/// <param name="End">
/// For a linear gradient, the end point. For radial and elliptical gradients, a
/// point on the outer edge.
/// </param>
/// <param name="Transform">
/// An extra transform applied to gradient space, used to express rotated and
/// squashed gradients without distorting the geometry being filled.
/// </param>
public sealed record GradientPaint(
    GradientKind Kind,
    IReadOnlyList<GradientStop> Stops,
    DocPoint Start,
    DocPoint End,
    AffineTransform Transform) : Paint;

/// <summary>The geometries a <see cref="GradientPaint"/> can take.</summary>
public enum GradientKind
{
    /// <summary>Colour varies along a line.</summary>
    Linear,

    /// <summary>Colour varies with distance from a centre point.</summary>
    Radial,

    /// <summary>Colour varies with distance from a centre, scaled per-axis.</summary>
    Elliptical,

    /// <summary>Colour varies with angle about a centre point.</summary>
    Conical,

    /// <summary>
    /// Nested rectangles, LibreOffice's "square" and OOXML's rectangular
    /// <c>path</c> gradient.
    /// </summary>
    Rectangular,
}

/// <summary>One colour stop in a gradient.</summary>
/// <param name="Offset">Position from 0 at the start to 1 at the end.</param>
/// <param name="Colour">The colour at that position.</param>
public readonly record struct GradientStop(double Offset, Colour Colour);

/// <summary>
/// A bitmap fill, tiled or stretched. Also the resolved form of hatch and pattern
/// fills.
/// </summary>
/// <param name="Image">The image to fill with.</param>
/// <param name="TileSize">The size of one tile.</param>
/// <param name="TileOffset">The offset of the first tile from the fill origin.</param>
/// <param name="Stretch">
/// When true the image is stretched once across the whole region and tiling is
/// ignored.
/// </param>
public sealed record BitmapPaint(
    RasterImage Image,
    DocSize TileSize,
    DocPoint TileOffset,
    bool Stretch) : Paint;

/// <summary>How to stroke a path.</summary>
/// <param name="Paint">What to stroke with.</param>
/// <param name="Width">
/// Line width. Zero means a hairline — the thinnest line the device can draw, which
/// is a distinct concept the office formats use and not the same as "invisible".
/// </param>
/// <param name="Cap">How line ends are drawn.</param>
/// <param name="Join">How corners are drawn.</param>
/// <param name="MiterLimit">The miter limit, when <paramref name="Join"/> is a miter.</param>
/// <param name="DashPattern">
/// Alternating dash and gap lengths, or null for a solid line.
/// </param>
/// <param name="DashOffset">How far into the dash pattern to start.</param>
public sealed record Stroke(
    Paint Paint,
    Length Width,
    LineCap Cap = LineCap.Butt,
    LineJoin Join = LineJoin.Miter,
    double MiterLimit = 10.0,
    IReadOnlyList<Length>? DashPattern = null,
    Length DashOffset = default);

/// <summary>How the ends of a stroked line are drawn.</summary>
public enum LineCap
{
    /// <summary>Squared off exactly at the endpoint.</summary>
    Butt = 0,

    /// <summary>Rounded, extending half the line width past the endpoint.</summary>
    Round,

    /// <summary>Squared, extending half the line width past the endpoint.</summary>
    Square,
}

/// <summary>How the corners of a stroked path are drawn.</summary>
public enum LineJoin
{
    /// <summary>Extended to a point, subject to the miter limit.</summary>
    Miter = 0,

    /// <summary>Rounded.</summary>
    Round,

    /// <summary>Cut off flat.</summary>
    Bevel,
}
