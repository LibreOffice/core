using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Vector.Metafiles;

/// <summary>
/// A metafile's clipping region, expressed as a stack of intersections.
/// </summary>
/// <remarks>
/// <para>
/// <b>The representation is chosen for what the sink can do, not for what GDI can say.</b>
/// <c>IDrawingSink.ClipPath</c> intersects the current clip with a path and offers no other
/// operation, so a clip that is a <em>list</em> of paths to intersect in turn needs no path
/// arithmetic at all: replaying the list is the intersection. Everything GDI expresses by
/// intersecting — <c>IntersectClipRect</c>, selecting a region, an <c>ETO_CLIPPED</c> text
/// rectangle — lands exactly.
/// </para>
/// <para>
/// What it cannot express is subtraction: <c>ExcludeClipRect</c> and <c>ExtSelectClipRgn</c>
/// with <c>RGN_DIFF</c>. Those are reported through <see cref="HasUnsupportedOperation"/> and
/// the excluded area is left unclipped, which draws too much rather than too little. Between
/// the two, drawing too much loses no content, and a metafile's exclusions are usually
/// redundant with a later intersection anyway.
/// </para>
/// <para>
/// A metafile's clip is device state, not a scope: it changes when a record says so and stays
/// changed. That does not match the sink's <c>Save</c>/<c>Restore</c> nesting, so the clip is
/// re-emitted lazily by <see cref="MetafilePainter"/> rather than tracked as a stack —
/// exactly what <c>MtfTools::UpdateClipRegion</c> does with a push and a pop
/// (<c>emfio/source/reader/mtftools.cxx:1254-1289</c>).
/// </para>
/// </remarks>
public sealed class MetafileClip
{
    private readonly List<(GraphicsPath Path, FillRule Rule)> _shapes;

    /// <summary>An unclipped state.</summary>
    public MetafileClip() => _shapes = [];

    private MetafileClip(List<(GraphicsPath, FillRule)> shapes, bool unsupported)
    {
        _shapes = shapes;
        HasUnsupportedOperation = unsupported;
    }

    /// <summary>True when nothing is clipped.</summary>
    public bool IsEmpty => _shapes.Count == 0;

    /// <summary>How many paths the clip intersects.</summary>
    public int Count => _shapes.Count;

    /// <summary>
    /// True when the file asked for a clip operation that cannot be expressed, so what is drawn
    /// may extend past where the producer intended.
    /// </summary>
    public bool HasUnsupportedOperation { get; private set; }

    /// <summary>Intersects the clip with a path.</summary>
    public void Intersect(GraphicsPath path, FillRule rule = FillRule.NonZero)
    {
        ArgumentNullException.ThrowIfNull(path);
        _shapes.Add((path, rule));
    }

    /// <summary>Intersects the clip with a rectangle.</summary>
    public void Intersect(DocRect rect) => Intersect(GraphicsPath.Rectangle(rect));

    /// <summary>Replaces the clip outright, as <c>RGN_COPY</c> and selecting a region do.</summary>
    /// <param name="path">The new clip, or null to clip nothing.</param>
    /// <param name="rule">Which rule decides what is inside it.</param>
    public void Replace(GraphicsPath? path, FillRule rule = FillRule.NonZero)
    {
        _shapes.Clear();
        if (path is not null) _shapes.Add((path, rule));
    }

    /// <summary>Records that an operation could not be honoured.</summary>
    public void MarkUnsupported() => HasUnsupportedOperation = true;

    /// <summary>Clears the clip.</summary>
    public void Reset() => _shapes.Clear();

    /// <summary>A copy, for the save stack.</summary>
    public MetafileClip Clone() => new([.. _shapes], HasUnsupportedOperation);

    /// <summary>
    /// A copy moved by an offset, as <c>OffsetClipRgn</c> asks for.
    /// </summary>
    /// <remarks>
    /// Rebuilt rather than transformed, because a path carries no transform of its own and the
    /// sink's would apply to the drawing as well as to the clip. Offsetting a clip is rare enough
    /// that rebuilding it costs nothing measurable.
    /// </remarks>
    public MetafileClip Translate(Length dx, Length dy)
    {
        List<(GraphicsPath, FillRule)> moved = new(_shapes.Count);

        foreach ((GraphicsPath path, FillRule rule) in _shapes)
        {
            GraphicsPath copy = new();

            foreach (PathCommand command in path.Commands)
            {
                switch (command.Verb)
                {
                    case PathVerb.MoveTo:
                        copy.MoveTo(command.Point.Offset(dx, dy));
                        break;

                    case PathVerb.LineTo:
                        copy.LineTo(command.Point.Offset(dx, dy));
                        break;

                    case PathVerb.CubicTo:
                        copy.CubicTo(
                            command.Control1.Offset(dx, dy),
                            command.Control2.Offset(dx, dy),
                            command.Point.Offset(dx, dy));
                        break;

                    default:
                        copy.Close();
                        break;
                }
            }

            moved.Add((copy, rule));
        }

        return new MetafileClip(moved, HasUnsupportedOperation);
    }

    /// <summary>
    /// True when two clips would produce the same sink calls.
    /// </summary>
    /// <remarks>
    /// Compared by path identity rather than by geometry: the paths are built once and never
    /// mutated, so identity is exact here, and comparing thousands of points on every drawing
    /// record would cost more than the clipping does.
    /// </remarks>
    public bool SameAs(MetafileClip other)
    {
        ArgumentNullException.ThrowIfNull(other);

        if (ReferenceEquals(this, other)) return true;
        if (_shapes.Count != other._shapes.Count) return false;

        for (int i = 0; i < _shapes.Count; i++)
        {
            if (!ReferenceEquals(_shapes[i].Path, other._shapes[i].Path)) return false;
            if (_shapes[i].Rule != other._shapes[i].Rule) return false;
        }

        return true;
    }

    /// <summary>Issues the clip into a sink, which must already have saved its state.</summary>
    public void Apply(IDrawingSink sink)
    {
        ArgumentNullException.ThrowIfNull(sink);

        foreach ((GraphicsPath path, FillRule rule) in _shapes) sink.ClipPath(path, rule);
    }
}
