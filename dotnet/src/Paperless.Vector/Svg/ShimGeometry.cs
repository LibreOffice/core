using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using ShimSkiaSharp;
using ShimPathCommand = ShimSkiaSharp.PathCommand;

namespace Paperless.Vector.Svg;

/// <summary>
/// Converts <c>ShimSkiaSharp</c>'s geometry into Paperless's.
/// </summary>
/// <remarks>
/// <para>
/// The shim states everything in <c>float</c> SVG user units; Paperless states everything in
/// integer EMUs. The conversion happens here and nowhere else, which is the point: a scale
/// factor applied in two places is a scale factor applied twice, and that is the single most
/// common way vector import comes out at the wrong size.
/// </para>
/// <para>
/// The shim's path also carries whole primitives — rectangles, ovals, rounded rectangles,
/// SVG elliptical arcs — that <see cref="GraphicsPath"/> deliberately does not, because a
/// display list of lines and cubics is what every backend ultimately needs. Flattening them
/// here rather than in each backend is the same argument LibreOffice makes for
/// <c>basegfx::B2DPolygon</c>, which also carries only lines and cubics.
/// </para>
/// </remarks>
internal static class ShimGeometry
{
    /// <summary>
    /// The circle-to-Bezier constant: how far along the tangents the control points sit for a
    /// quarter turn. 4/3·(√2 − 1), the classic value, whose worst-case radial error is about
    /// 2.7 parts in 10 000 of the radius — a fifth of a micrometre on a 25 mm circle.
    /// </summary>
    private const double Kappa = 0.5522847498307933;

    /// <summary>EMUs per SVG user unit: the CSS pixel is 1/96 inch.</summary>
    /// <remarks>
    /// Not a guess and not configurable. LibreOffice fixes the same value —
    /// <c>o3tl::Length::px</c> is defined as "15 twip (96 ppi)"
    /// (<c>include/o3tl/unit_conversion.hxx:44</c>) and <c>svgio</c> converts every other unit
    /// into px through it (<c>svgio/source/svgreader/SvgNumber.cxx:41-54</c>) before scaling
    /// the finished picture by px→1/100 mm (<c>svgsvgnode.cxx:676</c>). 914400/96 is exact, so
    /// no rounding enters here at all.
    /// </remarks>
    public const double EmuPerPixel = Length.EmuPerInch / 96.0;

    /// <summary>A shim point as a document point.</summary>
    public static DocPoint Point(float x, float y)
        => new(Emu(x), Emu(y));

    /// <summary>A shim point as a document point.</summary>
    public static DocPoint Point(SKPoint point) => Point(point.X, point.Y);

    /// <summary>A shim length as a document length.</summary>
    public static Length Emu(double units)
        => Length.FromEmu((long)Math.Round(Math.Clamp(units * EmuPerPixel, -1e17, 1e17)));

    /// <summary>A shim rectangle as a document rectangle, normalised.</summary>
    public static DocRect Rect(SKRect rect) => DocRect.FromCorners(
        Point(Math.Min(rect.Left, rect.Right), Math.Min(rect.Top, rect.Bottom)),
        Point(Math.Max(rect.Left, rect.Right), Math.Max(rect.Top, rect.Bottom)));

    /// <summary>A shim colour as a Paperless colour.</summary>
    public static Colour Colour(SKColor colour) => new(colour.Red, colour.Green, colour.Blue, colour.Alpha);

    /// <summary>A shim floating-point colour as a Paperless colour.</summary>
    /// <remarks>
    /// The gradient shaders state their stops as <see cref="SKColorF"/> rather than
    /// <see cref="SKColor"/>, so the channels arrive as 0-to-1 floats and have to be rounded
    /// back onto the eight-bit grid the display list uses.
    /// </remarks>
    public static Colour Colour(SKColorF colour) => new(
        Channel(colour.Red), Channel(colour.Green), Channel(colour.Blue), Channel(colour.Alpha));

    private static byte Channel(float value)
        => (byte)Math.Clamp(Math.Round(value * 255.0), 0, 255);

    /// <summary>
    /// A shim matrix as a Paperless transform.
    /// </summary>
    /// <remarks>
    /// The translation is converted to EMUs and the linear part is not, matching
    /// <see cref="AffineTransform"/>'s contract — scaling a scale factor would square it.
    /// The perspective row is dropped: SVG cannot express one and the shim only carries it to
    /// mirror Skia's layout.
    /// </remarks>
    public static AffineTransform Transform(SKMatrix matrix) => new(
        matrix.ScaleX,
        matrix.SkewY,
        matrix.SkewX,
        matrix.ScaleY,
        matrix.TransX * EmuPerPixel,
        matrix.TransY * EmuPerPixel);

    /// <summary>A shim fill type as a fill rule.</summary>
    public static FillRule Rule(SKPathFillType type)
        => type == SKPathFillType.EvenOdd ? FillRule.EvenOdd : FillRule.NonZero;

    /// <summary>
    /// Converts a shim path to a document path, optionally pre-transformed.
    /// </summary>
    /// <param name="path">The shim path.</param>
    /// <param name="transform">
    /// An extra transform in shim space, applied to every point before conversion. Used for
    /// clip paths, whose transform cannot be expressed as a sink call: a clip survives the
    /// <c>Restore</c> that would have undone the transform.
    /// </param>
    /// <param name="segments">Incremented by the number of segments produced.</param>
    public static GraphicsPath ToPath(SKPath path, SKMatrix? transform, ref int segments)
    {
        ArgumentNullException.ThrowIfNull(path);

        GraphicsPath result = new();
        SKPoint current = default;
        SKPoint start = default;

        foreach (ShimPathCommand command in path.Commands ?? [])
        {
            switch (command)
            {
                case MoveToPathCommand move:
                    current = start = new SKPoint(move.X, move.Y);
                    result.MoveTo(Map(current, transform));
                    break;

                case LineToPathCommand line:
                    current = new SKPoint(line.X, line.Y);
                    result.LineTo(Map(current, transform));
                    break;

                case QuadToPathCommand quad:
                    // A quadratic upconverts to a cubic exactly: the control points sit two
                    // thirds of the way from each end towards the quadratic's own control.
                    {
                        SKPoint control = new(quad.X0, quad.Y0);
                        SKPoint end = new(quad.X1, quad.Y1);
                        result.CubicTo(
                            Map(Lerp(current, control, 2.0 / 3.0), transform),
                            Map(Lerp(end, control, 2.0 / 3.0), transform),
                            Map(end, transform));
                        current = end;
                    }
                    break;

                case CubicToPathCommand cubic:
                    result.CubicTo(
                        Map(new SKPoint(cubic.X0, cubic.Y0), transform),
                        Map(new SKPoint(cubic.X1, cubic.Y1), transform),
                        Map(new SKPoint(cubic.X2, cubic.Y2), transform));
                    current = new SKPoint(cubic.X2, cubic.Y2);
                    break;

                case ArcToPathCommand arc:
                    {
                        SKPoint end = new(arc.X, arc.Y);
                        AppendArc(result, current, arc, end, transform);
                        current = end;
                    }
                    break;

                case ClosePathCommand:
                    result.Close();
                    current = start;
                    break;

                case AddRectPathCommand rect:
                    AppendPolygon(result, Corners(rect.Rect), transform);
                    break;

                case AddRoundRectPathCommand round:
                    AppendRoundRect(result, round, transform);
                    break;

                case AddOvalPathCommand oval:
                    AppendOval(result, oval.Rect, transform);
                    break;

                case AddCirclePathCommand circle:
                    AppendOval(
                        result,
                        new SKRect(
                            circle.X - circle.Radius,
                            circle.Y - circle.Radius,
                            circle.X + circle.Radius,
                            circle.Y + circle.Radius),
                        transform);
                    break;

                case AddPolyPathCommand poly:
                    AppendPolygon(result, [.. poly.Points ?? []], transform, poly.Close);
                    break;

                default:
                    break;
            }
        }

        segments += result.Commands.Count;
        return result;
    }

    private static SKPoint[] Corners(SKRect rect) =>
    [
        new(rect.Left, rect.Top),
        new(rect.Right, rect.Top),
        new(rect.Right, rect.Bottom),
        new(rect.Left, rect.Bottom),
    ];

    private static void AppendPolygon(GraphicsPath path, SKPoint[] points, SKMatrix? transform, bool close = true)
    {
        if (points.Length == 0) return;

        path.MoveTo(Map(points[0], transform));
        for (int i = 1; i < points.Length; i++) path.LineTo(Map(points[i], transform));
        if (close) path.Close();
    }

    private static void AppendRoundRect(GraphicsPath path, AddRoundRectPathCommand round, SKMatrix? transform)
    {
        SKRect rect = round.Rect;
        float width = Math.Abs(rect.Right - rect.Left);
        float height = Math.Abs(rect.Bottom - rect.Top);
        float rx = Math.Clamp(Math.Abs(round.Rx), 0, width / 2);
        float ry = Math.Clamp(Math.Abs(round.Ry), 0, height / 2);

        if (rx <= 0 || ry <= 0)
        {
            AppendPolygon(path, Corners(rect), transform);
            return;
        }

        float left = Math.Min(rect.Left, rect.Right);
        float right = Math.Max(rect.Left, rect.Right);
        float top = Math.Min(rect.Top, rect.Bottom);
        float bottom = Math.Max(rect.Top, rect.Bottom);
        double cx = rx * Kappa;
        double cy = ry * Kappa;

        path.MoveTo(Map(new SKPoint(left + rx, top), transform));
        path.LineTo(Map(new SKPoint(right - rx, top), transform));
        Corner(path, transform, right - rx + cx, top, right, top + ry - cy, right, top + ry);
        path.LineTo(Map(new SKPoint(right, bottom - ry), transform));
        Corner(path, transform, right, bottom - ry + cy, right - rx + cx, bottom, right - rx, bottom);
        path.LineTo(Map(new SKPoint(left + rx, bottom), transform));
        Corner(path, transform, left + rx - cx, bottom, left, bottom - ry + cy, left, bottom - ry);
        path.LineTo(Map(new SKPoint(left, top + ry), transform));
        Corner(path, transform, left, top + ry - cy, left + rx - cx, top, left + rx, top);
        path.Close();
    }

    private static void Corner(
        GraphicsPath path,
        SKMatrix? transform,
        double c1x,
        double c1y,
        double c2x,
        double c2y,
        double x,
        double y)
        => path.CubicTo(
            Map(new SKPoint((float)c1x, (float)c1y), transform),
            Map(new SKPoint((float)c2x, (float)c2y), transform),
            Map(new SKPoint((float)x, (float)y), transform));

    private static void AppendOval(GraphicsPath path, SKRect rect, SKMatrix? transform)
    {
        double left = Math.Min(rect.Left, rect.Right);
        double right = Math.Max(rect.Left, rect.Right);
        double top = Math.Min(rect.Top, rect.Bottom);
        double bottom = Math.Max(rect.Top, rect.Bottom);
        double rx = (right - left) / 2;
        double ry = (bottom - top) / 2;
        double cx = left + rx;
        double cy = top + ry;
        double kx = rx * Kappa;
        double ky = ry * Kappa;

        path.MoveTo(Map(new SKPoint((float)(cx + rx), (float)cy), transform));
        Corner(path, transform, cx + rx, cy + ky, cx + kx, cy + ry, cx, cy + ry);
        Corner(path, transform, cx - kx, cy + ry, cx - rx, cy + ky, cx - rx, cy);
        Corner(path, transform, cx - rx, cy - ky, cx - kx, cy - ry, cx, cy - ry);
        Corner(path, transform, cx + kx, cy - ry, cx + rx, cy - ky, cx + rx, cy);
        path.Close();
    }

    /// <summary>
    /// Appends an SVG elliptical arc as cubics.
    /// </summary>
    /// <remarks>
    /// The endpoint-to-centre conversion of SVG 1.1 appendix F.6.5, then one cubic per
    /// quarter turn. Splitting at 90° keeps the same accuracy as <see cref="Kappa"/> gives a
    /// circle; a single cubic across a half turn would be visibly wrong.
    /// </remarks>
    private static void AppendArc(
        GraphicsPath path,
        SKPoint from,
        ArcToPathCommand arc,
        SKPoint to,
        SKMatrix? transform)
    {
        double rx = Math.Abs(arc.Rx);
        double ry = Math.Abs(arc.Ry);

        // A zero radius is defined to mean a straight line, and so is a zero-length arc.
        if (rx <= 0 || ry <= 0 || (from.X == to.X && from.Y == to.Y))
        {
            path.LineTo(Map(to, transform));
            return;
        }

        double phi = arc.XAxisRotate * Math.PI / 180.0;
        double cosPhi = Math.Cos(phi);
        double sinPhi = Math.Sin(phi);

        double dx2 = (from.X - to.X) / 2.0;
        double dy2 = (from.Y - to.Y) / 2.0;
        double x1 = (cosPhi * dx2) + (sinPhi * dy2);
        double y1 = (-sinPhi * dx2) + (cosPhi * dy2);

        // F.6.6: radii too small to span the chord are scaled up until they just reach.
        double lambda = ((x1 * x1) / (rx * rx)) + ((y1 * y1) / (ry * ry));
        if (lambda > 1)
        {
            double root = Math.Sqrt(lambda);
            rx *= root;
            ry *= root;
        }

        // F.6.5.2: the centre sits on one side of the chord or the other, and which one is
        // decided by the two flags disagreeing. Getting this backwards mirrors every arc in
        // the file about its own chord, which looks like a coordinate-system bug and is not.
        bool large = arc.LargeArc == SKPathArcSize.Large;
        bool sweepPositive = arc.Sweep == SKPathDirection.Clockwise;
        double sign = large != sweepPositive ? 1 : -1;
        double numerator = (rx * rx * ry * ry) - (rx * rx * y1 * y1) - (ry * ry * x1 * x1);
        double denominator = (rx * rx * y1 * y1) + (ry * ry * x1 * x1);
        double factor = denominator <= 0 ? 0 : sign * Math.Sqrt(Math.Max(0, numerator / denominator));

        double cx1 = factor * rx * y1 / ry;
        double cy1 = -factor * ry * x1 / rx;
        double cx = (cosPhi * cx1) - (sinPhi * cy1) + ((from.X + to.X) / 2.0);
        double cy = (sinPhi * cx1) + (cosPhi * cy1) + ((from.Y + to.Y) / 2.0);

        double startAngle = Angle(1, 0, (x1 - cx1) / rx, (y1 - cy1) / ry);
        double sweep = Angle((x1 - cx1) / rx, (y1 - cy1) / ry, (-x1 - cx1) / rx, (-y1 - cy1) / ry);

        if (!sweepPositive && sweep > 0) sweep -= 2 * Math.PI;
        else if (sweepPositive && sweep < 0) sweep += 2 * Math.PI;

        int steps = Math.Max(1, (int)Math.Ceiling(Math.Abs(sweep) / (Math.PI / 2)));
        double step = sweep / steps;
        double alpha = 4.0 / 3.0 * Math.Tan(step / 4.0);

        double angle = startAngle;
        for (int i = 0; i < steps; i++)
        {
            double next = angle + step;
            (double sx, double sy) = OnEllipse(cx, cy, rx, ry, cosPhi, sinPhi, angle);
            (double ex, double ey) = OnEllipse(cx, cy, rx, ry, cosPhi, sinPhi, next);
            (double sdx, double sdy) = Tangent(rx, ry, cosPhi, sinPhi, angle);
            (double edx, double edy) = Tangent(rx, ry, cosPhi, sinPhi, next);

            Corner(
                path,
                transform,
                sx + (alpha * sdx),
                sy + (alpha * sdy),
                ex - (alpha * edx),
                ey - (alpha * edy),
                ex,
                ey);

            angle = next;
        }
    }

    private static (double X, double Y) OnEllipse(
        double cx, double cy, double rx, double ry, double cosPhi, double sinPhi, double angle)
    {
        double x = rx * Math.Cos(angle);
        double y = ry * Math.Sin(angle);
        return (cx + (cosPhi * x) - (sinPhi * y), cy + (sinPhi * x) + (cosPhi * y));
    }

    private static (double X, double Y) Tangent(
        double rx, double ry, double cosPhi, double sinPhi, double angle)
    {
        double x = -rx * Math.Sin(angle);
        double y = ry * Math.Cos(angle);
        return ((cosPhi * x) - (sinPhi * y), (sinPhi * x) + (cosPhi * y));
    }

    private static double Angle(double ux, double uy, double vx, double vy)
    {
        double dot = (ux * vx) + (uy * vy);
        double lengths = Math.Sqrt(((ux * ux) + (uy * uy)) * ((vx * vx) + (vy * vy)));
        if (lengths <= 0) return 0;

        double angle = Math.Acos(Math.Clamp(dot / lengths, -1, 1));
        return (ux * vy) - (uy * vx) < 0 ? -angle : angle;
    }

    private static SKPoint Lerp(SKPoint a, SKPoint b, double t)
        => new((float)(a.X + ((b.X - a.X) * t)), (float)(a.Y + ((b.Y - a.Y) * t)));

    private static DocPoint Map(SKPoint point, SKMatrix? transform)
        => transform is { } matrix ? Point(matrix.MapPoint(point)) : Point(point);
}
