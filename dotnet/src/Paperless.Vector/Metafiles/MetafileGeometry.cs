using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Vector.Metafiles;

/// <summary>
/// Builds the shapes a metafile's drawing records name, in already-mapped document
/// coordinates.
/// </summary>
/// <remarks>
/// <para>
/// Every method takes points that have already been through <see cref="MetafileMapping"/>.
/// That ordering is deliberate and is LibreOffice's: an arc's sweep direction and a rounded
/// rectangle's corner radii both depend on the mapping, so building the shape in logical
/// coordinates and mapping the result would get a y-flipped picture's arcs backwards.
/// </para>
/// <para>
/// Curves come out as cubic Beziers rather than as the polygon approximations
/// <c>tools::Polygon</c> uses, because <c>GraphicsPath</c> carries cubics natively and a
/// backend can then flatten to its own device resolution instead of to the 16-to-256 points
/// <c>ImplPolygon</c> chose for a screen.
/// </para>
/// </remarks>
public static class MetafileGeometry
{
    /// <summary>
    /// The circle-to-Bezier constant for a quarter turn, 4/3·(√2 − 1).
    /// </summary>
    /// <remarks>
    /// Worst-case radial error about 2.7 parts in 10 000 — a fifth of a micrometre on a 25 mm
    /// circle, which is below the resolution of anything that will draw it.
    /// </remarks>
    public const double Kappa = 0.5522847498307933;

    /// <summary>A closed rectangle.</summary>
    public static GraphicsPath Rectangle(DocRect rect) => GraphicsPath.Rectangle(rect);

    /// <summary>
    /// A rounded rectangle, with the corner radii clamped to half the side they run along.
    /// </summary>
    /// <remarks>
    /// GDI states the corner <em>ellipse</em>'s width and height, so the radii are half of each;
    /// the caller has already halved them. Clamping matters because a metafile may state a
    /// corner larger than the rectangle, which GDI draws as an ellipse rather than as a shape
    /// that folds through itself.
    /// </remarks>
    public static GraphicsPath RoundRectangle(DocRect rect, Length radiusX, Length radiusY)
    {
        Length rx = Length.Min(Length.Max(radiusX, Length.Zero), rect.Width / 2.0);
        Length ry = Length.Min(Length.Max(radiusY, Length.Zero), rect.Height / 2.0);

        if (rx <= Length.Zero || ry <= Length.Zero) return Rectangle(rect);

        Length cx = rx * Kappa;
        Length cy = ry * Kappa;
        Length l = rect.Left;
        Length t = rect.Top;
        Length r = rect.Right;
        Length b = rect.Bottom;

        return new GraphicsPath()
            .MoveTo(new DocPoint(l + rx, t))
            .LineTo(new DocPoint(r - rx, t))
            .CubicTo(new DocPoint(r - rx + cx, t), new DocPoint(r, t + ry - cy), new DocPoint(r, t + ry))
            .LineTo(new DocPoint(r, b - ry))
            .CubicTo(new DocPoint(r, b - ry + cy), new DocPoint(r - rx + cx, b), new DocPoint(r - rx, b))
            .LineTo(new DocPoint(l + rx, b))
            .CubicTo(new DocPoint(l + rx - cx, b), new DocPoint(l, b - ry + cy), new DocPoint(l, b - ry))
            .LineTo(new DocPoint(l, t + ry))
            .CubicTo(new DocPoint(l, t + ry - cy), new DocPoint(l + rx - cx, t), new DocPoint(l + rx, t))
            .Close();
    }

    /// <summary>An ellipse inscribed in a rectangle.</summary>
    public static GraphicsPath Ellipse(DocRect rect)
    {
        Length rx = rect.Width / 2.0;
        Length ry = rect.Height / 2.0;
        Length cx = rect.X + rx;
        Length cy = rect.Y + ry;
        Length kx = rx * Kappa;
        Length ky = ry * Kappa;

        return new GraphicsPath()
            .MoveTo(new DocPoint(cx + rx, cy))
            .CubicTo(new DocPoint(cx + rx, cy + ky), new DocPoint(cx + kx, cy + ry), new DocPoint(cx, cy + ry))
            .CubicTo(new DocPoint(cx - kx, cy + ry), new DocPoint(cx - rx, cy + ky), new DocPoint(cx - rx, cy))
            .CubicTo(new DocPoint(cx - rx, cy - ky), new DocPoint(cx - kx, cy - ry), new DocPoint(cx, cy - ry))
            .CubicTo(new DocPoint(cx + kx, cy - ry), new DocPoint(cx + rx, cy - ky), new DocPoint(cx + rx, cy))
            .Close();
    }

    /// <summary>What an arc record closes its shape with.</summary>
    public enum ArcClosure
    {
        /// <summary>Nothing: an open curve, which is what <c>Arc</c> draws.</summary>
        Open,

        /// <summary>A straight line between the ends, which is what <c>Chord</c> draws.</summary>
        Chord,

        /// <summary>Two lines through the centre, which is what <c>Pie</c> draws.</summary>
        Pie,
    }

    /// <summary>
    /// An elliptical arc between two radial points, as <c>Arc</c>, <c>Chord</c> and <c>Pie</c>
    /// state it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <b>GDI names the ends by points, not by angles</b>, and the point need not lie on the
    /// ellipse: it names the ray from the centre that the end sits on. For a non-circular
    /// ellipse the angle of that ray is not the parametric angle of the point where it crosses,
    /// so the conversion has to go through the ellipse's own parameter —
    /// <c>atan2(rx·sin θ, ry·cos θ)</c>, which is <c>ImplGetParameter</c> in
    /// <c>tools/source/generic/poly.cxx:60-67</c>. Using the ray's angle directly puts the ends
    /// of every non-circular arc in the wrong place, by up to the ellipse's eccentricity.
    /// </para>
    /// <para>
    /// The parameter is measured with y <em>upwards</em>, which is the maths convention and not
    /// the document's, so the sweep runs anticlockwise on the page for GDI's default arc
    /// direction. That sign is the whole of the "arcs come out mirrored" bug.
    /// </para>
    /// </remarks>
    /// <param name="rect">The bounding rectangle of the ellipse, already mapped.</param>
    /// <param name="start">A point on the ray where the arc starts, already mapped.</param>
    /// <param name="end">A point on the ray where the arc ends, already mapped.</param>
    /// <param name="closure">How to close the shape.</param>
    /// <param name="clockwise">True when the arc sweeps clockwise in the mapped space.</param>
    public static GraphicsPath Arc(
        DocRect rect,
        DocPoint start,
        DocPoint end,
        ArcClosure closure,
        bool clockwise = false)
    {
        double rx = rect.Width.Emu / 2.0;
        double ry = rect.Height.Emu / 2.0;
        double cx = rect.X.Emu + rx;
        double cy = rect.Y.Emu + ry;

        if (rx <= 0 || ry <= 0) return new GraphicsPath();

        double from = Parameter(cx, cy, start, rx, ry);
        double to = Parameter(cx, cy, end, rx, ry);
        double sweep = to - from;

        if (!clockwise)
        {
            // A start equal to the end is a full ellipse, not an empty arc: [MS-WMF] says so and
            // i53768 records a file that depends on it.
            if (sweep <= 0) sweep += 2 * Math.PI;
        }
        else
        {
            sweep -= 2 * Math.PI;
            if (sweep < -2 * Math.PI) sweep += 2 * Math.PI;
        }

        GraphicsPath path = new();
        DocPoint first = OnEllipse(cx, cy, rx, ry, from);

        if (closure == ArcClosure.Pie)
        {
            path.MoveTo(new DocPoint(Length.FromEmu((long)Math.Round(cx)), Length.FromEmu((long)Math.Round(cy))));
            path.LineTo(first);
        }
        else
        {
            path.MoveTo(first);
        }

        // Split into quarter turns or less: a single cubic cannot follow more than about a
        // quarter of an ellipse to within the error Kappa promises.
        int segments = Math.Max(1, (int)Math.Ceiling(Math.Abs(sweep) / (Math.PI / 2)));
        double step = sweep / segments;
        double alpha = 4.0 / 3.0 * Math.Tan(step / 4.0);

        double angle = from;
        for (int i = 0; i < segments; i++)
        {
            double next = angle + step;

            // The tangent at parameter t is (-rx·sin t, -ry·cos t) in document coordinates: the
            // second component's sign is flipped from the maths convention because y points down.
            double sinA = Math.Sin(angle);
            double cosA = Math.Cos(angle);
            double sinB = Math.Sin(next);
            double cosB = Math.Cos(next);

            DocPoint p1 = Emu(
                cx + (rx * cosA) - (alpha * rx * sinA),
                cy - (ry * sinA) - (alpha * ry * cosA));
            DocPoint p2 = Emu(
                cx + (rx * cosB) + (alpha * rx * sinB),
                cy - (ry * sinB) + (alpha * ry * cosB));
            DocPoint p3 = Emu(cx + (rx * cosB), cy - (ry * sinB));

            path.CubicTo(p1, p2, p3);
            angle = next;
        }

        if (closure != ArcClosure.Open) path.Close();

        return path;
    }

    /// <summary>A polygon: a closed path through the points.</summary>
    public static GraphicsPath Polygon(IReadOnlyList<DocPoint> points)
    {
        ArgumentNullException.ThrowIfNull(points);

        GraphicsPath path = new();
        if (points.Count == 0) return path;

        path.MoveTo(points[0]);
        for (int i = 1; i < points.Count; i++) path.LineTo(points[i]);
        path.Close();
        return path;
    }

    /// <summary>A polyline: an open path through the points.</summary>
    public static GraphicsPath Polyline(IReadOnlyList<DocPoint> points)
    {
        ArgumentNullException.ThrowIfNull(points);

        GraphicsPath path = new();
        if (points.Count == 0) return path;

        path.MoveTo(points[0]);
        for (int i = 1; i < points.Count; i++) path.LineTo(points[i]);
        return path;
    }

    /// <summary>Appends a closed subpath to an existing path, as a poly-polygon needs.</summary>
    public static void AddPolygon(GraphicsPath path, IReadOnlyList<DocPoint> points)
    {
        ArgumentNullException.ThrowIfNull(path);
        ArgumentNullException.ThrowIfNull(points);

        if (points.Count == 0) return;

        path.MoveTo(points[0]);
        for (int i = 1; i < points.Count; i++) path.LineTo(points[i]);
        path.Close();
    }

    /// <summary>Appends a closed rectangular subpath, as a region's scan list needs.</summary>
    public static void AddRectangle(GraphicsPath path, DocRect rect)
    {
        ArgumentNullException.ThrowIfNull(path);

        path.MoveTo(new DocPoint(rect.Left, rect.Top))
            .LineTo(new DocPoint(rect.Right, rect.Top))
            .LineTo(new DocPoint(rect.Right, rect.Bottom))
            .LineTo(new DocPoint(rect.Left, rect.Bottom))
            .Close();
    }

    /// <summary>How many segments a path holds, for charging against the work limits.</summary>
    public static int SegmentCount(GraphicsPath path)
    {
        ArgumentNullException.ThrowIfNull(path);
        return path.Commands.Count;
    }

    private static double Parameter(double cx, double cy, DocPoint point, double rx, double ry)
    {
        double dx = point.X.Emu - cx;

        // Upwards-positive, because the parameterisation below is: the point (cx+rx·cos t,
        // cy-ry·sin t) is at parameter t.
        double dy = cy - point.Y.Emu;

        double angle = Math.Atan2(dy, dx);
        return Math.Atan2(rx * Math.Sin(angle), ry * Math.Cos(angle));
    }

    private static DocPoint OnEllipse(double cx, double cy, double rx, double ry, double t)
        => Emu(cx + (rx * Math.Cos(t)), cy - (ry * Math.Sin(t)));

    private static DocPoint Emu(double x, double y) => new(
        Length.FromEmu((long)Math.Round(Math.Clamp(x, -1e17, 1e17))),
        Length.FromEmu((long)Math.Round(Math.Clamp(y, -1e17, 1e17))));
}
