using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;

namespace XRay.Office.Core.Charts;

/// <summary>
/// A three-dimensional pie: the extruded solid 26.2.4.2 draws for a <c>c:pie3DChart</c>.
/// </summary>
/// <remarks>
/// <para>
/// <strong>The reference rasterises this chart, so every law here was fitted to a bitmap.</strong>
/// A 3-D chart reaches the page as one <c>Do</c> of an RGB image with a soft mask — the labels
/// and the chart's frame are still operators, the solid is not — so there is no vector geometry
/// to compare against and no prospect of a byte-equal rendering. What can be measured exactly is
/// the silhouette, and <c>probes/pie3d-r187</c> does: 36 one-attribute rewrites of a corpus
/// document's own chart part, nine elevations by four depths, each rendered through 26.2.4.2 and
/// each solid's outline fitted.
/// </para>
/// <para>
/// <strong>The projection is parallel.</strong> Fitting the silhouette's upper boundary as an
/// ellipse over every column of the bitmap leaves a mean residual of <strong>0.28 to 0.45
/// pixels</strong> at all nine elevations, on images 1370 pixels wide. A perspective projection
/// would not be an ellipse and that residual is how one would know. It says so although
/// <c>View3DConverter::convertFromModel</c> asks for <c>ProjectionMode_PERSPECTIVE</c> — a
/// <c>c:perspective</c> of 30 halves to 15, and 15 is not zero
/// (<c>oox/source/drawingml/chart/plotareaconverter.cxx</c>:299-308) — because the camera is far
/// enough away that the conic is an ellipse to within the raster.
/// </para>
/// <para>
/// <strong>The squash is the sine of the elevation.</strong> Measured
/// <c>B/A</c> against <c>sin(rotX)</c> over the nine: 0.164/0.174, 0.340/0.342, 0.499/0.500,
/// 0.647/0.643, 0.763/0.766, 0.866/0.866, 0.939/0.940, 0.966/0.985, 0.997/1.000 — within
/// <strong>0.7% at seven of the nine</strong>, and the two that miss (10° and 80°) are where the
/// silhouette is hardest to fit, one because the ellipse is nearly a line and the other because
/// it is nearly a circle.
/// </para>
/// <para>
/// <strong><c>c:depthPercent</c> is not read, and that is measured rather than assumed.</strong>
/// The same document at 20, 50, 100 and 200 gives geometry identical to the hundredth of a point
/// at every one of the nine elevations. The pie branch of <c>View3DConverter</c> never passes it
/// on; the depth comes from the diagram's preferred aspect ratio, which
/// <c>PieChart::getPreferredDiagramAspectRatio</c> states as <c>(1, 1, 0.10)</c>
/// (<c>chart2/source/view/charttypes/PieChart.cxx</c>:252-256).
/// </para>
/// <para>
/// <strong>The solid is fitted to the plot area isotropically.</strong> A unit disc of diameter 1
/// and thickness <see cref="Thickness"/> projects to a box 1 wide and
/// <c>sin θ + 0.10·cos θ</c> tall, and the scale is whichever of the two limits binds. Predicted
/// against measured width over the nine elevations: 478/478, 478/480, 477/469, 388/386, 337/341,
/// 308/308, 288/290, 284/290, 280/280 — <strong>within 2% at every one</strong>, and the
/// changeover from width-limited to height-limited between 20° and 30° is reproduced. The
/// depth is the weaker half: 45/50, 41/43, 30/30, 22/20, 16/13, 10/8 over 20° to 70°, so it is
/// right to about a fifth at the ends and to a twentieth of the figure's height throughout.
/// </para>
/// <para>
/// <strong>The shading is two flat multipliers of the series' own colour.</strong> Counted over
/// every opaque pixel of the corpus document's bitmap: the top faces are the declared colour
/// times <strong>0.957</strong> (0.959 on accent 6, 0.955 on accent 5) and the extruded wall runs
/// from about 0.83 at the sides to 0.91 at the front, which is one light turning with the
/// surface. A single <see cref="WallShade"/> is used for the wall rather than a lighting model,
/// because the band is narrow and what the page shows is the shape.
/// </para>
/// </remarks>
public static partial class ChartLayout
{
    /// <summary>The solid's thickness, as a fraction of its diameter, before projection.</summary>
    /// <remarks>
    /// The <c>z</c> of <c>PieChart::getPreferredDiagramAspectRatio</c>'s
    /// <c>Direction3D(1, 1, 0.10)</c>.
    /// </remarks>
    private const double Thickness = 0.10;

    /// <summary>What the top face's colour is multiplied by.</summary>
    private const double TopShade = 0.957;

    /// <summary>What the extruded wall's is.</summary>
    /// <remarks>
    /// The midpoint of the 0.83–0.91 the reference's own bitmap carries across the wall, which
    /// turns with the surface. Flat here: see the remarks on the type.
    /// </remarks>
    private const double WallShade = 0.87;

    /// <summary>The projected geometry of a 3-D pie fitted to a rectangle.</summary>
    /// <param name="Centre">The top face's ellipse centre.</param>
    /// <param name="A">Its horizontal semi-axis.</param>
    /// <param name="B">Its vertical semi-axis.</param>
    /// <param name="Depth">How far below it the extruded wall reaches.</param>
    internal readonly record struct Pie3DGeometry(DocPoint Centre, Length A, Length B, Length Depth);

    /// <summary>Fits the solid into <paramref name="area"/> at the elevation given.</summary>
    /// <param name="area">The rectangle the whole solid, wall included, is centred in.</param>
    /// <param name="elevation">Degrees, 0 edge on and 90 straight down.</param>
    internal static Pie3DGeometry Pie3DFit(DocRect area, double elevation)
    {
        double radians = Math.Clamp(elevation, 0.0, 90.0) * Math.PI / 180.0;
        double sin = Math.Sin(radians);
        double cos = Math.Cos(radians);

        // The unit solid is 1 across and this tall, so the binding limit is whichever of the two
        // the rectangle runs out of first.
        double tall = sin + (Thickness * cos);
        Length scale = tall > 0.0
            ? Length.Min(area.Width, area.Height / tall)
            : area.Width;

        Length a = scale / 2;
        Length depth = scale * (Thickness * cos);

        // Centre the whole silhouette — the ellipse plus the wall below it — rather than the
        // ellipse, which is what puts the solid in the middle of the plot area.
        return new Pie3DGeometry(
            new DocPoint(
                area.X + (area.Width / 2),
                area.Y + (area.Height / 2) - (depth / 2)),
            a,
            a * sin,
            depth);
    }

    /// <summary>One face of the solid, with the order it is painted in.</summary>
    /// <param name="Path">Its outline.</param>
    /// <param name="Fill">Its colour, already shaded.</param>
    /// <param name="Order">Larger is nearer the viewer and painted later.</param>
    private readonly record struct Pie3DFace(GraphicsPath Path, Colour Fill, double Order);

    /// <summary>
    /// Draws a series as an extruded pie, walls first and then the top faces.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The solid is convex and seen from above, so the painter's order is exact rather than a
    /// heuristic: the nearest point of the silhouette is the bottom of the ellipse, every wall is
    /// ordered by how far its own angle is from there, and the top faces are coplanar, never
    /// overlap one another and are all above every wall on the page.
    /// </para>
    /// <para>
    /// A face is culled by its outward normal against the view vector
    /// <c>(0, −sin θ, cos θ)</c>, which is the direction the projection above implies: the top
    /// face gives <c>cos θ</c> and is always drawn; the outer wall at model angle φ gives
    /// <c>−sin φ · sin θ</c>, so the front half of the rim is the visible half; and a radial cut
    /// reduces to a test on <c>cos</c> of its own angle, one sign per side of the sector.
    /// </para>
    /// </remarks>
    private static void AddWedges3D(
        ChartPlot plot,
        ChartSeries series,
        Pie3DGeometry pie,
        List<ChartShape> shapes)
    {
        double total = series.Total();
        if (!(total > 0.0) || pie.A <= Length.Zero) return;

        List<Pie3DFace> walls = [];
        List<Pie3DFace> tops = [];

        double start = Math.PI / 2;

        for (int at = 0; at < series.Values.Count; at++)
        {
            if (series.Values[at] is not { } value || !double.IsFinite(value)) continue;

            double sweep = Math.Abs(value) / total * (2 * Math.PI);
            if (sweep <= 0.0) continue;

            double from = start;
            double to = start - sweep;
            Colour fill = series.FillAt(at) ?? Colour.Black;

            tops.Add(new Pie3DFace(
                EllipticalWedge(pie, from, -sweep), Shade(fill, TopShade), 0.0));

            if (pie.Depth > Length.Zero)
            {
                foreach (Pie3DFace wall in WallsOf(pie, from, to, Shade(fill, WallShade)))
                    walls.Add(wall);
            }

            start = to;
        }

        walls.Sort((left, right) => left.Order.CompareTo(right.Order));

        foreach (Pie3DFace face in walls)
            shapes.Add(new ChartShape(face.Path, face.Fill, series.Line, series.LineWidth));

        foreach (Pie3DFace face in tops)
            shapes.Add(new ChartShape(face.Path, face.Fill, series.Line, series.LineWidth));
    }

    /// <summary>The visible walls of one sector: its front rim and whichever cut faces us.</summary>
    /// <param name="pie">The fitted geometry the faces are built on.</param>
    /// <param name="from">The sector's start angle, counter-clockwise from three o'clock.</param>
    /// <param name="to">Its end angle, which is smaller because a pie sweeps clockwise.</param>
    /// <param name="fill">The wall's colour, already shaded.</param>
    private static IEnumerable<Pie3DFace> WallsOf(
        Pie3DGeometry pie, double from, double to, Colour fill)
    {
        // The rim is visible where the model's y is negative, which is the near half on the page.
        // In [to, from] that is the intersection with the half-turn beginning at −π; the sector
        // can straddle it, so the overlap is taken on the circle rather than on the line.
        foreach ((double a, double b) in FrontArcs(to, from))
            yield return new Pie3DFace(RimWall(pie, a, b), fill, Nearness((a + b) / 2));

        // A cut at angle φ has its outward normal ninety degrees round from the radius, on the
        // side away from the sector. Against the view vector (0, −sin θ, cos θ) the start cut is
        // visible for cos(from) < 0 and the end cut for cos(to) > 0.
        if (Math.Cos(from) < 0.0)
            yield return new Pie3DFace(CutWall(pie, from), fill, Nearness(from));

        if (Math.Cos(to) > 0.0)
            yield return new Pie3DFace(CutWall(pie, to), fill, Nearness(to));
    }

    /// <summary>How near the viewer an angle is: 1 at the bottom of the ellipse, 0 at the top.</summary>
    private static double Nearness(double angle) => (1.0 - Math.Sin(angle)) / 2.0;

    /// <summary>The parts of <c>[a, b]</c> whose rim faces the viewer, as angle pairs.</summary>
    /// <remarks>
    /// The near half is where <c>sin</c> is negative. Working modulo a full turn keeps a sector
    /// that straddles three o'clock — which every pie's first sector can, once the start angle
    /// has been wound past it — from being split into the wrong two pieces.
    /// </remarks>
    private static IEnumerable<(double From, double To)> FrontArcs(double a, double b)
    {
        const double Turn = 2 * Math.PI;

        // Walk the sector in small enough steps that a run of near-facing angles is found whole,
        // and emit each run. A sector can contribute at most two, and stepping is what keeps the
        // straddling cases honest without a case analysis that has to be right four ways.
        int steps = Math.Max(8, (int)Math.Ceiling(Math.Abs(b - a) / (Math.PI / 90)));
        double step = (b - a) / steps;

        double? open = null;
        for (int at = 0; at <= steps; at++)
        {
            double angle = a + (step * at);
            bool near = Math.Sin(((angle % Turn) + Turn) % Turn) < 0.0;

            if (near && open is null) open = angle;
            if (!near && open is { } began)
            {
                yield return (began, angle);
                open = null;
            }
        }

        if (open is { } last) yield return (last, b);
    }

    /// <summary>The curved wall under an arc: the arc, down, back, and up.</summary>
    private static GraphicsPath RimWall(Pie3DGeometry pie, double from, double to)
    {
        GraphicsPath path = new();
        path.MoveTo(Rim(pie, from));
        EllipticalArc(path, pie, from, to - from);
        path.LineTo(Dropped(Rim(pie, to), pie.Depth));
        EllipticalArc(path, pie, to, from - to, pie.Depth);
        path.Close();
        return path;
    }

    /// <summary>The flat wall where a sector is cut: the radius, down, back, and up.</summary>
    private static GraphicsPath CutWall(Pie3DGeometry pie, double angle)
    {
        DocPoint rim = Rim(pie, angle);
        GraphicsPath path = new();
        path.MoveTo(pie.Centre);
        path.LineTo(rim);
        path.LineTo(Dropped(rim, pie.Depth));
        path.LineTo(Dropped(pie.Centre, pie.Depth));
        path.Close();
        return path;
    }

    /// <summary>One sector of the top face.</summary>
    private static GraphicsPath EllipticalWedge(Pie3DGeometry pie, double start, double sweep)
    {
        GraphicsPath path = new();
        path.MoveTo(pie.Centre);
        path.LineTo(Rim(pie, start));
        EllipticalArc(path, pie, start, sweep);
        path.Close();
        return path;
    }

    /// <summary>A point on the top face's rim.</summary>
    private static DocPoint Rim(Pie3DGeometry pie, double angle)
        => new(pie.Centre.X + (pie.A * Math.Cos(angle)),
               pie.Centre.Y - (pie.B * Math.Sin(angle)));

    private static DocPoint Dropped(DocPoint point, Length by)
        => new(point.X, point.Y + by);

    /// <summary>
    /// Appends an elliptical arc, quarter turn by quarter turn, optionally dropped by the depth.
    /// </summary>
    /// <remarks>
    /// The same cubic approximation the flat pie uses — handles <c>4/3 × tan(θ/4)</c> of the
    /// radius along the tangent — with the two semi-axes kept apart, because an ellipse's tangent
    /// is <c>(−A sin θ, −B cos θ)</c> in a y-down space and not a scalar multiple of the circle's.
    /// A quarter turn is accurate to a thousandth of the radius; a half turn in one cubic is
    /// visibly flat at the sides.
    /// </remarks>
    private static void EllipticalArc(
        GraphicsPath path, Pie3DGeometry pie, double start, double sweep, Length? drop = null)
    {
        Length down = drop ?? Length.Zero;

        int segments = Math.Max(1, (int)Math.Ceiling(Math.Abs(sweep) / (Math.PI / 2)));
        double step = sweep / segments;
        double handle = 4.0 / 3.0 * Math.Tan(step / 4.0);

        double angle = start;

        for (int at = 0; at < segments; at++)
        {
            DocPoint a = Dropped(Rim(pie, angle), down);
            DocPoint b = Dropped(Rim(pie, angle + step), down);

            path.CubicTo(
                new DocPoint(
                    a.X - (pie.A * (handle * Math.Sin(angle))),
                    a.Y - (pie.B * (handle * Math.Cos(angle)))),
                new DocPoint(
                    b.X + (pie.A * (handle * Math.Sin(angle + step))),
                    b.Y + (pie.B * (handle * Math.Cos(angle + step)))),
                b);

            angle += step;
        }
    }

    /// <summary>A colour multiplied by a flat shade, channel by channel.</summary>
    private static Colour Shade(Colour colour, double by)
        => new(
            (byte)Math.Clamp(Math.Round(colour.R * by), 0, 255),
            (byte)Math.Clamp(Math.Round(colour.G * by), 0, 255),
            (byte)Math.Clamp(Math.Round(colour.B * by), 0, 255),
            colour.A);
}
