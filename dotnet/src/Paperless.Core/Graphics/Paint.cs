using Paperless.Core.Geometry;
using Paperless.Core.Units;

namespace Paperless.Core.Graphics;

/// <summary>
/// How to fill a region: a colour, a gradient, a tiled bitmap, or a coloured triangle mesh.
/// </summary>
/// <remarks>
/// Deliberately narrower than what the source formats express. LibreOffice supports
/// hatch and pattern fills as first-class kinds (<c>XFillStyleItem</c>); Paperless
/// resolves those into a tiled <see cref="BitmapPaint"/> or an explicit set of
/// stroked lines at read time, so backends have four cases instead of seven.
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
/// <param name="Spread">
/// What happens outside the ramp's own extent. Defaults to
/// <see cref="SpreadMethod.Pad"/>, which is what every gradient meant before this
/// existed.
/// </param>
/// <param name="Focus">
/// For a radial or elliptical gradient, where stop 0 sits when it is not the centre
/// of the outer circle, and null when it is. SVG spells it <c>fx</c>/<c>fy</c> and
/// DrawingML <c>a:fillToRect</c>; both backends have a native two-circle form for it —
/// PDF's <c>/ShadingType 3</c> takes two circles with different centres and Skia has a
/// two-point conical shader — so it costs a coordinate here and nothing anywhere else.
/// Ignored for the kinds that have no centre.
/// </param>
public sealed record GradientPaint(
    GradientKind Kind,
    IReadOnlyList<GradientStop> Stops,
    DocPoint Start,
    DocPoint End,
    AffineTransform Transform,
    SpreadMethod Spread = SpreadMethod.Pad,
    DocPoint? Focus = null) : Paint;

/// <summary>
/// What a gradient does with the space outside the ramp it states.
/// </summary>
/// <remarks>
/// SVG names all three on <c>spreadMethod</c>, GDI+ names all three on a brush's wrap mode
/// (its four tiling modes differ only in which axes a mirror applies to, which a
/// one-dimensional ramp cannot tell apart), and both backends have them natively — Skia as
/// <c>SKShaderTileMode</c>, PDF by extending the shading's own axis over as many periods as
/// the shape needs. Nothing else in the office formats asks for more than these three.
/// </remarks>
public enum SpreadMethod
{
    /// <summary>The end colours continue outwards for ever. PDF's <c>/Extend</c>.</summary>
    Pad = 0,

    /// <summary>The ramp repeats, reversing direction each period.</summary>
    Reflect,

    /// <summary>The ramp repeats from its start each period.</summary>
    Repeat,
}

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
    bool Stretch) : Paint
{
    /// <summary>
    /// How opaque the fill is, from 0 to 1; 1 unless the source states otherwise.
    /// </summary>
    /// <remarks>
    /// A picture fill can be transparent without any pixel of the picture being so — DrawingML
    /// spells it <c>a:blip/a:alphaModFix</c> and Escher <c>DFF_Prop_fillOpacity</c>, and both
    /// are properties of the fill rather than of the image. It is separate from
    /// <see cref="RasterImage.Duotone"/> for that reason: a recolouring changes the picture and
    /// belongs to it, and this does not, so two shapes filled with one image at different
    /// transparencies still share one decoded buffer.
    /// </remarks>
    public double Opacity { get; init; } = 1;
}

/// <summary>
/// A fill stated as triangles with a colour at each corner, interpolated across each
/// triangle's interior.
/// </summary>
/// <remarks>
/// <para>
/// <b>Why this exists rather than another <see cref="GradientKind"/>.</b> A
/// <see cref="GradientPaint"/> has exactly one <see cref="GradientPaint.Start"/> colour
/// and one <see cref="GradientPaint.End"/> colour however many stops sit between them, so
/// it can only say "this colour here, that colour there, along a line or out from a
/// point". A GDI+ path-gradient brush states a colour at <em>every vertex of an arbitrary
/// boundary</em> — a star with three surround colours is three coloured points and no
/// radial ramp anywhere in it — and no number of stops expresses that. The smallest thing
/// that does is the triangle with a colour at each corner, which is also what GDI+ itself
/// draws.
/// </para>
/// <para>
/// <b>Why this shape and not a Skia- or PDF-shaped one.</b> Both backends have a native
/// form and the two agree on exactly this much: PDF's <c>/ShadingType 4</c> is a stream of
/// triangles carrying a coordinate and a colour per vertex, and Skia's <c>SkVertices</c> is
/// a position array, a colour array and an index array. Vertices plus index triples is
/// their intersection, so neither backend has to reconstruct anything and
/// <see cref="Paperless.Core"/> gains no dependency. A patch mesh (PDF type 6 or 7) would
/// have carried curved edges and Skia could not have drawn it; a bitmap would have needed a
/// rasteriser here.
/// </para>
/// <para>
/// <b>Colour is interpolated linearly in straight, non-premultiplied RGBA</b>, which is
/// what both backends do with it and what GDI+ does. A mesh whose triangles overlap paints
/// them in order; a mesh is not required to be convex, connected or wound consistently,
/// because a boundary fan built from a concave path is none of those.
/// </para>
/// </remarks>
/// <param name="Vertices">
/// The corners, in document coordinates, each with the colour at that corner.
/// </param>
/// <param name="Triangles">
/// Index triples into <paramref name="Vertices"/>. A triple naming an index outside the
/// vertex list is skipped rather than fatal, since a mesh is generated rather than authored.
/// </param>
public sealed record MeshPaint(
    IReadOnlyList<MeshVertex> Vertices,
    IReadOnlyList<MeshTriangle> Triangles) : Paint;

/// <summary>One corner of a <see cref="MeshPaint"/>.</summary>
/// <param name="Position">Where the corner is, in document coordinates.</param>
/// <param name="Colour">The colour there.</param>
public readonly record struct MeshVertex(DocPoint Position, Colour Colour);

/// <summary>One triangle of a <see cref="MeshPaint"/>, as indices into its vertex list.</summary>
/// <param name="A">The first corner.</param>
/// <param name="B">The second corner.</param>
/// <param name="C">The third corner.</param>
public readonly record struct MeshTriangle(int A, int B, int C);

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
