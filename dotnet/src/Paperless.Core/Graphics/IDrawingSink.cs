using Paperless.Core.Geometry;
using Paperless.Core.Units;

namespace Paperless.Core.Graphics;

/// <summary>
/// The device-independent drawing interface every rendering backend implements and
/// every laid-out page draws into.
/// </summary>
/// <remarks>
/// <para>
/// This is the seam that keeps format parsing separate from rasterisation. Layout
/// code emits drawing commands; backends in <c>Paperless.Rendering</c> turn them into
/// pixels, PDF operators or SVG elements. Neither side knows about the other.
/// </para>
/// <para>
/// It is closely modelled on LibreOffice's <c>GDIMetaFile</c> / <c>MetaAction</c>
/// display list (<c>include/vcl/metaact.hxx</c>) and its <c>drawinglayer</c>
/// primitives, for a specific reason: those are the two places LibreOffice itself
/// funnels all output through, so anything expressible in a Writer, Calc or Impress
/// document is expressible here. Where the two differ, we follow
/// <c>drawinglayer</c>'s retained-primitive style rather than the older imperative
/// metafile style, since it composes better.
/// </para>
/// <para>
/// Design constraints worth preserving:
/// </para>
/// <list type="bullet">
///   <item><description>
///     <b>Resolution independence.</b> Every coordinate is a <see cref="Length"/> in
///     EMUs. Backends decide device resolution; layout never sees pixels. This is what
///     lets one layout drive both a 96 DPI thumbnail and a print-resolution PDF.
///   </description></item>
///   <item><description>
///     <b>Text stays text.</b> <see cref="DrawGlyphRun"/> carries positioned glyph
///     IDs, not outlines, so a PDF backend can embed real searchable text and a
///     raster backend can hint properly. Converting text to paths early would make
///     both impossible.
///   </description></item>
///   <item><description>
///     <b>Explicit state stack.</b> <see cref="Save"/>/<see cref="Restore"/> scope the
///     clip and transform. There is no ambient style state: every draw call carries
///     its own paint, which keeps command streams reorderable and cacheable.
///   </description></item>
/// </list>
/// </remarks>
public interface IDrawingSink
{
    /// <summary>
    /// Begins a page of the given size. Called once before that page's content.
    /// </summary>
    void BeginPage(DocSize size);

    /// <summary>Ends the current page.</summary>
    void EndPage();

    /// <summary>Pushes the current clip and transform onto the state stack.</summary>
    void Save();

    /// <summary>Pops the state stack, restoring the previous clip and transform.</summary>
    void Restore();

    /// <summary>
    /// Concatenates a transform onto the current one. Affects everything drawn until
    /// the next <see cref="Restore"/>.
    /// </summary>
    void Transform(AffineTransform transform);

    /// <summary>
    /// Intersects the current clip with a path. Office formats need arbitrary-path
    /// clipping, not just rectangles — cropped and rotated pictures are routine.
    /// </summary>
    void ClipPath(GraphicsPath path, FillRule rule = FillRule.NonZero);

    /// <summary>Fills a path.</summary>
    void FillPath(GraphicsPath path, Paint paint, FillRule rule = FillRule.NonZero);

    /// <summary>Strokes a path.</summary>
    void StrokePath(GraphicsPath path, Stroke stroke);

    /// <summary>
    /// Draws a run of positioned glyphs from a single font.
    /// </summary>
    /// <remarks>
    /// Shaping has already happened by this point: the run holds resolved glyph IDs
    /// and their positions. Backends must not re-shape, or they would disagree with
    /// the layout that produced the positions.
    /// </remarks>
    void DrawGlyphRun(GlyphRun run, Paint paint);

    /// <summary>
    /// Draws a raster image into the given destination rectangle.
    /// </summary>
    /// <param name="image">The image to draw.</param>
    /// <param name="destination">Where to place it; the image is scaled to fit.</param>
    /// <param name="opacity">A uniform opacity multiplier from 0 to 1.</param>
    void DrawImage(RasterImage image, DocRect destination, double opacity = 1.0);

    /// <summary>
    /// Begins a transparency group: subsequent drawing is composited offscreen and
    /// then blended in one step at the given opacity.
    /// </summary>
    /// <remarks>
    /// Required for correctness, not just efficiency. A group drawn at 50% opacity
    /// looks different from each of its members drawn at 50%, and office formats
    /// apply group-level transparency to shape groups.
    /// </remarks>
    void BeginTransparencyGroup(double opacity);

    /// <summary>Ends the innermost transparency group and composites it.</summary>
    void EndTransparencyGroup();
}

/// <summary>How to decide which parts of a self-intersecting path are inside it.</summary>
public enum FillRule
{
    /// <summary>The non-zero winding rule. The default in every office format.</summary>
    NonZero = 0,

    /// <summary>The even-odd rule.</summary>
    EvenOdd,
}

/// <summary>
/// A geometric path: a sequence of subpaths made of lines and cubic Beziers.
/// </summary>
/// <remarks>
/// Cubic Beziers alone are enough — quadratics upconvert exactly, and arcs and
/// rounded rectangles are approximated on construction, which is what both
/// LibreOffice's <c>basegfx::B2DPolygon</c> and every output backend do anyway.
/// </remarks>
public sealed class GraphicsPath
{
    private readonly List<PathCommand> _commands = [];

    /// <summary>The commands making up the path, in order.</summary>
    public IReadOnlyList<PathCommand> Commands => _commands;

    /// <summary>Starts a new subpath at the given point.</summary>
    public GraphicsPath MoveTo(DocPoint point)
    {
        _commands.Add(new PathCommand(PathVerb.MoveTo, point, default, default));
        return this;
    }

    /// <summary>Adds a straight segment.</summary>
    public GraphicsPath LineTo(DocPoint point)
    {
        _commands.Add(new PathCommand(PathVerb.LineTo, point, default, default));
        return this;
    }

    /// <summary>Adds a cubic Bezier segment.</summary>
    public GraphicsPath CubicTo(DocPoint control1, DocPoint control2, DocPoint end)
    {
        _commands.Add(new PathCommand(PathVerb.CubicTo, end, control1, control2));
        return this;
    }

    /// <summary>Closes the current subpath.</summary>
    public GraphicsPath Close()
    {
        _commands.Add(new PathCommand(PathVerb.Close, default, default, default));
        return this;
    }

    /// <summary>Creates a closed rectangular path.</summary>
    public static GraphicsPath Rectangle(DocRect rect) => new GraphicsPath()
        .MoveTo(new DocPoint(rect.Left, rect.Top))
        .LineTo(new DocPoint(rect.Right, rect.Top))
        .LineTo(new DocPoint(rect.Right, rect.Bottom))
        .LineTo(new DocPoint(rect.Left, rect.Bottom))
        .Close();
}

/// <summary>The kinds of <see cref="PathCommand"/>.</summary>
public enum PathVerb
{
    /// <summary>Start a new subpath.</summary>
    MoveTo,

    /// <summary>Straight segment to the point.</summary>
    LineTo,

    /// <summary>Cubic Bezier segment.</summary>
    CubicTo,

    /// <summary>Close the current subpath.</summary>
    Close,
}

/// <summary>One command in a <see cref="GraphicsPath"/>.</summary>
/// <param name="Verb">Which command this is.</param>
/// <param name="Point">The end point; unused for <see cref="PathVerb.Close"/>.</param>
/// <param name="Control1">First Bezier control point; only for <see cref="PathVerb.CubicTo"/>.</param>
/// <param name="Control2">Second Bezier control point; only for <see cref="PathVerb.CubicTo"/>.</param>
public readonly record struct PathCommand(
    PathVerb Verb,
    DocPoint Point,
    DocPoint Control1,
    DocPoint Control2);
