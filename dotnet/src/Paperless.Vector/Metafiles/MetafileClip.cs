using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Vector.Metafiles;

/// <summary>
/// A metafile's clipping region: a set of disjoint rectangles, intersected with a list of
/// arbitrary paths.
/// </summary>
/// <remarks>
/// <para>
/// <b>The representation is chosen for what the sink can do, not for what GDI can say.</b>
/// <c>IDrawingSink.ClipPath</c> intersects the current clip with a path and offers no other
/// operation, so a clip that is a <em>list</em> of paths to intersect in turn needs no path
/// arithmetic at all: replaying the list is the intersection. Everything GDI expresses by
/// intersecting — <c>IntersectClipRect</c>, selecting a region, an <c>ETO_CLIPPED</c> text
/// rectangle, <c>SelectClipPath</c> with <c>RGN_AND</c> — lands exactly.
/// </para>
/// <para>
/// <b>Subtraction is the operation that list cannot express, and it is expressible anyway
/// because a GDI region is rectangles.</b> <c>ExcludeClipRect</c> and <c>ExtSelectClipRgn</c>
/// with <c>RGN_DIFF</c> both subtract a set of rectangles, and rectangle sets are closed under
/// subtraction — no general path arithmetic, just integer comparisons. Better, it distributes:
/// subtracting <em>R</em> from (rectangles ∩ paths) is (rectangles − <em>R</em>) ∩ paths, so an
/// exclusion stays exact even when a non-rectangular clip path is also in force. That is why the
/// rectangular part is kept apart from the arbitrary part rather than everything being one list,
/// and it is what closes the gap WMF left open.
/// </para>
/// <para>
/// <b>Union, symmetric difference and complement follow from the same observation, once the
/// rectangular part is a real region rather than a running intersection.</b> A GDI region
/// <em>is</em> a scan list of rectangles, and rectangle sets are closed under every one of the
/// five combine modes, so <see cref="RectangleRegion"/> answers all of them exactly with a band
/// sweep over integer coordinates — no flattening, no crossover solver, no tolerance. What
/// remains genuinely out of reach is an operation whose operand or whose existing clip is an
/// arbitrary <em>path</em>: <c>SelectClipPath</c> with <c>RGN_XOR</c>, EMF+ <c>SetClipPath</c>
/// with a union or a complement. Those need a general polygon boolean, are reported through
/// <see cref="HasUnsupportedOperation"/>, and leave the clip as it was.
/// </para>
/// <para>
/// A metafile's clip is device state, not a scope: it changes when a record says so and stays
/// changed. That does not match the sink's <c>Save</c>/<c>Restore</c> nesting, so the clip is
/// re-emitted lazily by <see cref="MetafilePainter"/> rather than tracked as a stack — exactly
/// what <c>MtfTools::UpdateClipRegion</c> does with a push and a pop
/// (<c>emfio/source/reader/mtftools.cxx:1254-1289</c>).
/// </para>
/// </remarks>
public sealed class MetafileClip
{
    /// <summary>
    /// How many rectangles a combine may produce before the result is abandoned.
    /// </summary>
    /// <remarks>
    /// A band sweep can still be made to grow: <em>n</em> horizontal bars combined with
    /// <em>n</em> vertical ones is a grid of <em>n</em>² pieces, so an adversarial file of a few
    /// hundred clip records would otherwise allocate without bound. Past the cap the operation
    /// stops being applied and the clip is reported as approximate, which draws too much — the
    /// same direction every other unexpressible clip operation errs in.
    /// </remarks>
    public const int MaxRectangles = 1024;

    private readonly List<(GraphicsPath Path, FillRule Rule)> _shapes;
    private DocRect[]? _rectangles;

    /// <summary>An unclipped state.</summary>
    public MetafileClip() => _shapes = [];

    private MetafileClip(List<(GraphicsPath, FillRule)> shapes, DocRect[]? rectangles, bool unsupported)
    {
        _shapes = shapes;
        _rectangles = rectangles;
        HasUnsupportedOperation = unsupported;
    }

    /// <summary>
    /// The rectangle everything is drawn inside, which an exclusion subtracts from.
    /// </summary>
    /// <remarks>
    /// Subtracting from an unbounded plane has no finite answer, so the first exclusion needs
    /// something to subtract from. The picture's own frame is the honest choice — nothing
    /// outside it is drawn anyway — and it keeps the coordinates a backend sees the same order
    /// of magnitude as the picture rather than at the far end of the number line.
    /// </remarks>
    public DocRect Bounds { get; set; } = new(
        Length.FromEmu(-1L << 40),
        Length.FromEmu(-1L << 40),
        Length.FromEmu(1L << 41),
        Length.FromEmu(1L << 41));

    /// <summary>True when nothing is clipped.</summary>
    public bool IsEmpty => _shapes.Count == 0 && _rectangles is null;

    /// <summary>
    /// True when the clip is a rectangle set, so a union, a symmetric difference or a complement
    /// against another rectangle set is exact.
    /// </summary>
    /// <remarks>
    /// The distinction the whole region question turns on. Intersection and exclusion never need
    /// it — they distribute over the shape list — but the three operations that read the clip's
    /// own area do, because the area of "this path intersected with that one" is not something a
    /// rectangle sweep can produce.
    /// </remarks>
    public bool IsRectangular => _shapes.Count == 0;

    /// <summary>How many paths the clip intersects.</summary>
    public int Count => _shapes.Count + (_rectangles is null ? 0 : 1);

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

    /// <summary>Intersects the clip with a rectangle, exactly.</summary>
    public void Intersect(DocRect rect) => Intersect([rect]);

    /// <summary>Intersects the clip with a set of rectangles, exactly.</summary>
    /// <param name="rectangles">The region's scan list.</param>
    public void Intersect(IReadOnlyList<DocRect> rectangles)
    {
        ArgumentNullException.ThrowIfNull(rectangles);

        Store(_rectangles is null
            ? RectangleRegion.Normalise(rectangles)
            : RectangleRegion.Combine(_rectangles, rectangles, RegionOp.Intersect));
    }

    /// <summary>
    /// Subtracts a rectangle, as <c>ExcludeClipRect</c> and <c>RGN_DIFF</c> ask.
    /// </summary>
    /// <remarks>
    /// Exact whatever else is in the clip, because subtraction distributes over intersection:
    /// removing an area from the intersection of several shapes is the same as removing it from
    /// any one of them.
    /// </remarks>
    public void Exclude(DocRect rect) => Exclude([rect]);

    /// <summary>Subtracts a set of rectangles, which <c>RGN_DIFF</c> against a region asks for.</summary>
    /// <param name="rectangles">The region's scan list.</param>
    public void Exclude(IReadOnlyList<DocRect> rectangles)
    {
        ArgumentNullException.ThrowIfNull(rectangles);
        Store(RectangleRegion.Combine(_rectangles ?? [Bounds], rectangles, RegionOp.Difference));
    }

    /// <summary>
    /// Widens the clip to include a set of rectangles, as <c>RGN_OR</c> asks.
    /// </summary>
    /// <remarks>
    /// Exact only when <see cref="IsRectangular"/>, because a union reads the clip's own area and
    /// the area of an intersection list is not a rectangle set. An unbounded clip already
    /// contains everything, so the union with one is the identity rather than a special case.
    /// </remarks>
    public void Union(IReadOnlyList<DocRect> rectangles)
    {
        ArgumentNullException.ThrowIfNull(rectangles);

        if (!IsRectangular) { MarkUnsupported(); return; }
        if (_rectangles is null) return;

        Store(RectangleRegion.Combine(_rectangles, rectangles, RegionOp.Union));
    }

    /// <summary>
    /// Keeps what is in the clip or in a set of rectangles but not in both, as <c>RGN_XOR</c> asks.
    /// </summary>
    /// <remarks>Exact only when <see cref="IsRectangular"/>, for the same reason a union is.</remarks>
    public void SymmetricDifference(IReadOnlyList<DocRect> rectangles)
    {
        ArgumentNullException.ThrowIfNull(rectangles);

        if (!IsRectangular) { MarkUnsupported(); return; }

        Store(RectangleRegion.Combine(_rectangles ?? [Bounds], rectangles, RegionOp.SymmetricDifference));
    }

    /// <summary>
    /// Replaces the clip with the part of a set of rectangles that is <em>not</em> in it, which is
    /// GDI+'s <c>CombineModeComplement</c>.
    /// </summary>
    /// <remarks>
    /// The one combine mode whose operands are the other way round —
    /// <c>emfphelperdata.cxx:1553-1558</c> spells it <c>solvePolygonOperationDiff(right, left)</c>
    /// — so it is a separate member rather than an argument to <see cref="Exclude(DocRect)"/>,
    /// where the order would be easy to get silently backwards.
    /// </remarks>
    /// <param name="rectangles">The new region.</param>
    public void Complement(IReadOnlyList<DocRect> rectangles)
    {
        ArgumentNullException.ThrowIfNull(rectangles);

        if (!IsRectangular) { MarkUnsupported(); return; }

        Store(RectangleRegion.Combine(rectangles, _rectangles ?? [Bounds], RegionOp.Difference));
    }

    private void Store(DocRect[] result)
    {
        // Past the cap the operation is abandoned rather than approximated, which draws too much
        // — the same direction every other unexpressible clip operation errs in. The band sweep
        // makes this far harder to reach than the rectangle-at-a-time subtraction it replaced,
        // because it never emits a rectangle an operand did not imply.
        if (result.Length > MaxRectangles) { MarkUnsupported(); return; }

        _rectangles = result;
    }

    /// <summary>Replaces the clip outright, as <c>RGN_COPY</c> and selecting a region do.</summary>
    /// <param name="path">The new clip, or null to clip nothing.</param>
    /// <param name="rule">Which rule decides what is inside it.</param>
    public void Replace(GraphicsPath? path, FillRule rule = FillRule.NonZero)
    {
        _shapes.Clear();
        _rectangles = null;
        if (path is not null) _shapes.Add((path, rule));
    }

    /// <summary>Replaces the clip with a region's scan list, which stays exact under subtraction.</summary>
    /// <param name="rectangles">The region's rectangles; they need not be disjoint.</param>
    public void Replace(IReadOnlyList<DocRect> rectangles)
    {
        ArgumentNullException.ThrowIfNull(rectangles);

        _shapes.Clear();
        _rectangles = RectangleRegion.Normalise(rectangles);
    }

    /// <summary>Records that an operation could not be honoured.</summary>
    public void MarkUnsupported() => HasUnsupportedOperation = true;

    /// <summary>Clears the clip.</summary>
    public void Reset()
    {
        _shapes.Clear();
        _rectangles = null;
    }

    /// <summary>A copy, for the save stack.</summary>
    /// <remarks>
    /// The rectangle set is never mutated in place — every operation replaces it — so the copy
    /// shares it, which is also what makes <see cref="SameAs"/> a reference comparison.
    /// </remarks>
    public MetafileClip Clone() => new([.. _shapes], _rectangles, HasUnsupportedOperation) { Bounds = Bounds };

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

        DocRect[]? rectangles = null;

        if (_rectangles is not null)
        {
            rectangles = new DocRect[_rectangles.Length];
            for (int i = 0; i < _rectangles.Length; i++)
            {
                rectangles[i] = new DocRect(
                    _rectangles[i].X + dx, _rectangles[i].Y + dy, _rectangles[i].Width, _rectangles[i].Height);
            }
        }

        return new MetafileClip(moved, rectangles, HasUnsupportedOperation) { Bounds = Bounds };
    }

    /// <summary>
    /// True when two clips would produce the same sink calls.
    /// </summary>
    /// <remarks>
    /// Compared by identity rather than by geometry: the paths and the rectangle set are built
    /// once and never mutated, so identity is exact here, and comparing thousands of points on
    /// every drawing record would cost more than the clipping does.
    /// </remarks>
    public bool SameAs(MetafileClip other)
    {
        ArgumentNullException.ThrowIfNull(other);

        if (ReferenceEquals(this, other)) return true;
        if (!ReferenceEquals(_rectangles, other._rectangles)) return false;
        if (_shapes.Count != other._shapes.Count) return false;

        for (int i = 0; i < _shapes.Count; i++)
        {
            if (!ReferenceEquals(_shapes[i].Path, other._shapes[i].Path)) return false;
            if (_shapes[i].Rule != other._shapes[i].Rule) return false;
        }

        return true;
    }

    /// <summary>Issues the clip into a sink, which must already have saved its state.</summary>
    /// <remarks>
    /// The rectangle set goes in as one path of disjoint subpaths, which is their union under
    /// either fill rule — the same fact that lets a region be a clip at all. An empty set is a
    /// clip that admits nothing, and is emitted as such rather than skipped: a region can
    /// genuinely be empty, and skipping it would draw the whole picture where the file asked for
    /// none of it.
    /// </remarks>
    public void Apply(IDrawingSink sink)
    {
        ArgumentNullException.ThrowIfNull(sink);

        if (_rectangles is { } rectangles)
        {
            GraphicsPath path = new();

            if (rectangles.Length == 0) path.MoveTo(DocPoint.Origin).LineTo(DocPoint.Origin).Close();
            else foreach (DocRect rect in rectangles) MetafileGeometry.AddRectangle(path, rect);

            sink.ClipPath(path);
        }

        foreach ((GraphicsPath path, FillRule rule) in _shapes) sink.ClipPath(path, rule);
    }
}
