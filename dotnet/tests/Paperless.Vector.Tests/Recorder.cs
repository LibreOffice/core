using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Vector.Tests;

/// <summary>
/// A sink that keeps every call, with the transform stack actually applied.
/// </summary>
/// <remarks>
/// <para>
/// Unlike <c>Paperless.TestKit.RecordingDrawingSink</c>, which records transforms as counts
/// because layout emits none that matter, this one composes them and reports geometry in
/// final document coordinates. That is the whole question for vector import: a picture
/// arrives as nested transforms and the thing worth asserting is where the ink lands after
/// all of them, not how many there were.
/// </para>
/// <para>
/// Clips are recorded but not intersected: a test asserting on a clipped-away shape would be
/// asserting on this class rather than on the translator.
/// </para>
/// </remarks>
internal sealed class Recorder : IDrawingSink
{
    private readonly Stack<AffineTransform> _stack = new();
    private AffineTransform _current = AffineTransform.Identity;

    /// <summary>Every filled path, with its bounds in final coordinates.</summary>
    public List<(DocRect Bounds, Paint Paint, FillRule Rule)> Fills { get; } = [];

    /// <summary>Every stroked path, with its bounds in final coordinates.</summary>
    public List<(DocRect Bounds, Stroke Stroke)> Strokes { get; } = [];

    /// <summary>Every clip, with its bounds in final coordinates.</summary>
    public List<DocRect> Clips { get; } = [];

    /// <summary>Every glyph run, with its origin in final coordinates.</summary>
    public List<(string Text, DocPoint Origin, Length Size, string Family, Paint Paint)> Runs { get; } = [];

    /// <summary>Every image, with its destination in final coordinates.</summary>
    public List<(RasterImage Image, DocRect Destination, double Opacity)> Images { get; } = [];

    /// <summary>The opacities of the transparency groups opened.</summary>
    public List<double> Groups { get; } = [];

    /// <summary>How deep the state stack went.</summary>
    public int MaxDepth { get; private set; }

    /// <summary>The union of everything drawn.</summary>
    public DocRect Ink
    {
        get
        {
            DocRect ink = DocRect.Empty;
            foreach ((DocRect bounds, _, _) in Fills) ink = ink.Union(bounds);
            foreach ((DocRect bounds, _) in Strokes) ink = ink.Union(bounds);
            foreach ((_, DocRect destination, _) in Images) ink = ink.Union(destination);
            return ink;
        }
    }

    /// <inheritdoc/>
    public void BeginPage(DocSize size) { }

    /// <inheritdoc/>
    public void EndPage() { }

    /// <inheritdoc/>
    public void Save()
    {
        _stack.Push(_current);
        MaxDepth = Math.Max(MaxDepth, _stack.Count);
    }

    /// <inheritdoc/>
    public void Restore()
    {
        if (_stack.Count > 0) _current = _stack.Pop();
    }

    /// <inheritdoc/>
    public void Transform(AffineTransform transform) => _current = AffineTransform.Concat(transform, _current);

    /// <inheritdoc/>
    public void ClipPath(GraphicsPath path, FillRule rule = FillRule.NonZero) => Clips.Add(Bounds(path));

    /// <inheritdoc/>
    public void FillPath(GraphicsPath path, Paint paint, FillRule rule = FillRule.NonZero)
        => Fills.Add((Bounds(path), paint, rule));

    /// <inheritdoc/>
    public void StrokePath(GraphicsPath path, Stroke stroke) => Strokes.Add((Bounds(path), stroke));

    /// <inheritdoc/>
    public void DrawGlyphRun(GlyphRun run, Paint paint)
        => Runs.Add((run.Text, Map(run.Origin), run.FontSize, run.Font.FamilyName, paint));

    /// <inheritdoc/>
    public void DrawImage(RasterImage image, DocRect destination, double opacity = 1.0)
        => Images.Add((image, Map(destination), opacity));

    /// <inheritdoc/>
    public void BeginTransparencyGroup(double opacity) => Groups.Add(opacity);

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
        DocRect bounds = DocRect.Empty;
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

        return any ? new DocRect(left, top, right - left, bottom - top) : bounds;
    }
}
