using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.TestKit;

/// <summary>
/// A sink that keeps every call with the transform stack actually applied, so geometry can be
/// asserted in final document coordinates.
/// </summary>
/// <remarks>
/// <para>
/// <see cref="RecordingDrawingSink"/> counts transforms rather than composing them, which is right
/// for layout — a page's text and rules arrive already in page coordinates and a count is enough to
/// show the state stack balanced. It is not enough for a picture. A vector image arrives as a view
/// box mapped onto a destination, and the only thing worth asserting about it is <em>where the ink
/// lands after that mapping</em>: a picture drawn at a tenth of its size and a picture drawn
/// correctly emit exactly the same commands and differ only in the matrix.
/// </para>
/// <para>
/// Clips are recorded and not intersected. A test asserting on a shape that a clip removes would be
/// asserting on this class rather than on the thing under test.
/// </para>
/// </remarks>
public sealed class PlacedDrawingSink : IDrawingSink
{
    private readonly Stack<AffineTransform> _stack = new();
    private AffineTransform _current = AffineTransform.Identity;

    /// <summary>Every filled path, with its bounds in final coordinates.</summary>
    public List<(DocRect Bounds, Paint Paint)> Fills { get; } = [];

    /// <summary>Every stroked path, with its bounds in final coordinates.</summary>
    public List<(DocRect Bounds, Stroke Stroke)> Strokes { get; } = [];

    /// <summary>Every clip, with its bounds in final coordinates.</summary>
    public List<DocRect> Clips { get; } = [];

    /// <summary>Every glyph run, with its origin in final coordinates.</summary>
    public List<(GlyphRun Run, DocPoint Origin)> Runs { get; } = [];

    /// <summary>Every image, with its destination in final coordinates.</summary>
    public List<(RasterImage Image, DocRect Destination)> Images { get; } = [];

    /// <summary>The union of everything drawn, in final coordinates.</summary>
    public DocRect Ink
    {
        get
        {
            DocRect ink = DocRect.Empty;
            foreach ((DocRect bounds, _) in Fills) ink = ink.Union(bounds);
            foreach ((DocRect bounds, _) in Strokes) ink = ink.Union(bounds);
            foreach ((_, DocRect destination) in Images) ink = ink.Union(destination);
            return ink;
        }
    }

    /// <inheritdoc/>
    public void BeginPage(DocSize size) { }

    /// <inheritdoc/>
    public void EndPage() { }

    /// <inheritdoc/>
    public void Save() => _stack.Push(_current);

    /// <inheritdoc/>
    public void Restore()
    {
        if (_stack.Count > 0) _current = _stack.Pop();
    }

    /// <inheritdoc/>
    public void Transform(AffineTransform transform)
        => _current = AffineTransform.Concat(transform, _current);

    /// <inheritdoc/>
    public void ClipPath(GraphicsPath path, FillRule rule = FillRule.NonZero) => Clips.Add(Bounds(path));

    /// <inheritdoc/>
    public void FillPath(GraphicsPath path, Paint paint, FillRule rule = FillRule.NonZero)
        => Fills.Add((Bounds(path), paint));

    /// <inheritdoc/>
    public void StrokePath(GraphicsPath path, Stroke stroke) => Strokes.Add((Bounds(path), stroke));

    /// <inheritdoc/>
    public void DrawGlyphRun(GlyphRun run, Paint paint)
    {
        ArgumentNullException.ThrowIfNull(run);
        Runs.Add((run, Map(run.Origin)));
    }

    /// <inheritdoc/>
    public void DrawImage(RasterImage image, DocRect destination, double opacity = 1.0)
        => Images.Add((image, Map(destination)));

    /// <inheritdoc/>
    public void BeginTransparencyGroup(double opacity) { }

    /// <inheritdoc/>
    public void EndTransparencyGroup() { }

    private DocRect Map(DocRect rect) => DocRect.FromCorners(
        Map(new DocPoint(rect.Left, rect.Top)),
        Map(new DocPoint(rect.Right, rect.Bottom)));

    private DocPoint Map(DocPoint point) => new(
        Length.FromEmu((long)Math.Round((point.X.Emu * _current.A) + (point.Y.Emu * _current.C) + _current.E)),
        Length.FromEmu((long)Math.Round((point.X.Emu * _current.B) + (point.Y.Emu * _current.D) + _current.F)));

    /// <summary>A path's extent in final coordinates, control points included.</summary>
    private DocRect Bounds(GraphicsPath path)
    {
        bool any = false;
        Length left = default, top = default, right = default, bottom = default;

        foreach (PathCommand command in path.Commands)
        {
            if (command.Verb == PathVerb.Close) continue;

            List<DocPoint> points = [command.Point];
            if (command.Verb == PathVerb.CubicTo) points.AddRange([command.Control1, command.Control2]);

            foreach (DocPoint raw in points)
            {
                DocPoint point = Map(raw);
                if (!any)
                {
                    left = right = point.X;
                    top = bottom = point.Y;
                    any = true;
                    continue;
                }

                left = Length.Min(left, point.X);
                top = Length.Min(top, point.Y);
                right = Length.Max(right, point.X);
                bottom = Length.Max(bottom, point.Y);
            }
        }

        return any ? new DocRect(left, top, right - left, bottom - top) : DocRect.Empty;
    }
}
