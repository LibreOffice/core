using Paperless.Core.Geometry;
using Paperless.Core.Graphics;

namespace Paperless.Vector.Metafiles;

/// <summary>
/// The path a metafile builds between <c>BeginPath</c> and <c>EndPath</c>, and then strokes,
/// fills or clips with.
/// </summary>
/// <remarks>
/// <para>
/// <b>This is the one piece of device state WMF never needed.</b> In EMF the same drawing
/// records — <c>LineTo</c>, <c>Polygon</c>, <c>PolyBezier</c>, <c>Arc</c> — either draw or
/// append to a path, decided by whether a <c>BeginPath</c> is open. So every drawing record has
/// to ask, and the ones that are cheapest to get wrong are the ones that both draw and move the
/// current position.
/// </para>
/// <para>
/// <b>Open and closed subpaths are not the same thing and the format relies on the
/// difference.</b> <c>MoveTo</c> starts a new subpath; a polygon record appends a closed one; a
/// polyline record extends whichever subpath is open, or starts one if none is. That is
/// <c>WinMtfPathObj</c>'s <c>bClosed</c> flag (<c>emfio/source/reader/mtftools.cxx:159-204</c>),
/// and it is what makes a figure drawn as "move, line, line, closefigure" fill as a triangle
/// rather than as three separate strokes.
/// </para>
/// <para>
/// Points arrive already mapped, like everything else the geometry helpers take: the path may
/// outlive a mapping record, and a path half in one coordinate space and half in another is
/// the sort of bug that only shows on the files that change the mapping mid-figure.
/// </para>
/// </remarks>
public sealed class MetafilePath
{
    private GraphicsPath _path = new();
    private bool _open;
    private bool _shared;
    private int _points;

    /// <summary>True while a <c>BeginPath</c> is in force, so drawing records append.</summary>
    public bool IsRecording { get; set; }

    /// <summary>The path built so far.</summary>
    public GraphicsPath Path => _path;

    /// <summary>How many points have been appended, for charging against the work limits.</summary>
    public int PointCount => _points;

    /// <summary>True when nothing has been appended.</summary>
    public bool IsEmpty => _path.Commands.Count == 0;

    /// <summary>Discards the path, as <c>BeginPath</c> and <c>AbortPath</c> do.</summary>
    public void Clear()
    {
        _path = new GraphicsPath();
        _shared = false;
        _open = false;
        _points = 0;
    }

    /// <summary>
    /// A copy for the save stack, sharing the path until one of the two is next written to.
    /// </summary>
    /// <remarks>
    /// <c>SaveDC</c> saves the path (<c>MtfTools::Push</c>, <c>mtftools.cxx:3075</c>), and a
    /// file may hold hundreds of thousands of them. Copying a path that may carry millions of
    /// points at each one is a denial of service on untrusted input, so the copy is deferred to
    /// the next mutation — which in practice never comes, because the only record that starts
    /// writing to a path is <c>BeginPath</c> and it clears the path first.
    /// </remarks>
    public MetafilePath Clone()
    {
        _shared = true;
        return new MetafilePath
        {
            IsRecording = IsRecording,
            _path = _path,
            _shared = true,
            _open = _open,
            _points = _points,
        };
    }

    private void Mutating()
    {
        if (!_shared) return;

        GraphicsPath copy = new();
        foreach (PathCommand command in _path.Commands) Append(copy, command);

        _path = copy;
        _shared = false;
    }

    private static void Append(GraphicsPath path, PathCommand command)
    {
        switch (command.Verb)
        {
            case PathVerb.MoveTo:
                path.MoveTo(command.Point);
                break;

            case PathVerb.LineTo:
                path.LineTo(command.Point);
                break;

            case PathVerb.CubicTo:
                path.CubicTo(command.Control1, command.Control2, command.Point);
                break;

            default:
                path.Close();
                break;
        }
    }

    /// <summary>Starts a new subpath.</summary>
    public void MoveTo(DocPoint point)
    {
        Mutating();
        _path.MoveTo(point);
        _open = true;
        _points++;
    }

    /// <summary>Extends the open subpath, starting one if none is open.</summary>
    public void LineTo(DocPoint point)
    {
        if (!_open)
        {
            MoveTo(point);
            return;
        }

        Mutating();
        _path.LineTo(point);
        _points++;
    }

    /// <summary>Extends the open subpath with a cubic, starting one at <paramref name="from"/> if none is open.</summary>
    /// <param name="from">Where the curve begins; used only when no subpath is open.</param>
    /// <param name="control1">The first control point.</param>
    /// <param name="control2">The second control point.</param>
    /// <param name="to">The end point.</param>
    public void CubicTo(DocPoint from, DocPoint control1, DocPoint control2, DocPoint to)
    {
        if (!_open) MoveTo(from);

        Mutating();
        _path.CubicTo(control1, control2, to);
        _points += 3;
    }

    /// <summary>Appends a closed subpath, as a polygon record does.</summary>
    public void AddPolygon(IReadOnlyList<DocPoint> points)
    {
        ArgumentNullException.ThrowIfNull(points);
        if (points.Count == 0) return;

        Mutating();
        MetafileGeometry.AddPolygon(_path, points);
        _points += points.Count;
        _open = false;
    }

    /// <summary>Appends every subpath of a ready-built path.</summary>
    public void Add(GraphicsPath path)
    {
        ArgumentNullException.ThrowIfNull(path);

        Mutating();

        foreach (PathCommand command in path.Commands)
        {
            Append(_path, command);
            _points++;
        }

        _open = false;
    }

    /// <summary>
    /// Closes the open subpath, as <c>CloseFigure</c> does.
    /// </summary>
    /// <remarks>
    /// A closed figure is closed by a segment back to its own start, not by a flag: GDI's
    /// <c>CloseFigure</c> is defined as drawing that segment, and a stroked path that omits it
    /// is visibly missing one edge. The subsequent record starts a new subpath, which is what
    /// <c>bClosed</c> records.
    /// </remarks>
    public void Close()
    {
        if (_open)
        {
            Mutating();
            _path.Close();
        }

        _open = false;
    }
}
