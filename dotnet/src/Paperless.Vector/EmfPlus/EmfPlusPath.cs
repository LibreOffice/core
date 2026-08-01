using Paperless.Core.Geometry;
using Paperless.Core.Graphics;

namespace Paperless.Vector.EmfPlus;

/// <summary>
/// An EMF+ path: an array of points, an array of point types, and the rules that turn the two
/// into figures.
/// </summary>
/// <remarks>
/// <para>
/// Ported from <c>drawinglayer/source/tools/emfppath.cxx</c>. The points are kept in the file's
/// own coordinates and mapped only when a figure is built, because the same path object is drawn
/// under whatever world transform is in force at each of the records that name it — a path
/// created once and drawn at three scales is ordinary in real clip art.
/// </para>
/// <para>
/// <b>Three point encodings, and the flags word decides which.</b> A float pair, a compressed
/// 16-bit pair, or a <em>relative</em> pair of variable-length integers. The last of these is the
/// trap: relative points are deltas from the previous point and have to be accumulated, and
/// nothing about a file that uses them looks wrong until the geometry does. LibreOffice reads
/// them without accumulating (<c>emfppath.cxx</c>, the <c>0x800</c> branch pushes the delta
/// itself), so a path that uses them draws as a small cluster near the origin there.
/// </para>
/// </remarks>
internal sealed class EmfPlusPath : EmfPlusObject
{
    private readonly List<double> _x;
    private readonly List<double> _y;
    private byte[]? _types;

    /// <summary>Creates an empty path with room for a stated point count.</summary>
    /// <param name="capacity">The stated number of points, used only as a hint.</param>
    public EmfPlusPath(int capacity)
    {
        int room = Math.Clamp(capacity, 0, 4096);
        _x = new List<double>(room);
        _y = new List<double>(room);
    }

    /// <summary>How many points the path holds.</summary>
    public int Count => _x.Count;

    /// <summary>True when the path carries a point-type array, which only a path object does.</summary>
    public bool HasPointTypes => _types is not null;

    /// <summary>
    /// Reads a point array, and the point-type array when the path has one.
    /// </summary>
    /// <param name="stream">The record's cursor, positioned at the first point.</param>
    /// <param name="count">How many points the record states.</param>
    /// <param name="flags">The record or path flags, which name the point encoding.</param>
    /// <param name="withTypes">True for a path object, false for an inline point list.</param>
    /// <returns>The path, for chaining.</returns>
    public static EmfPlusPath? Read(EmfPlusStream stream, int count, uint flags, bool withTypes)
    {
        ArgumentNullException.ThrowIfNull(stream);

        if (count <= 0) return null;

        // The stream that is left says how many points there can really be, whatever count the
        // file gave — the cheapest guard against a record claiming a million points inside two
        // hundred bytes (emfppath.cxx does the same arithmetic).
        int each = (flags & 0x800) != 0 ? 2 : (flags & 0x4000) != 0 ? 4 : 8;
        if (withTypes) each += 1;

        int possible = stream.Remaining / each;
        if (count > possible) count = possible;
        if (count <= 0) return null;

        EmfPlusPath path = new(count);

        double x = 0;
        double y = 0;

        for (int i = 0; i < count && !stream.Failed; i++)
        {
            if ((flags & 0x800) != 0)
            {
                // EmfPlusPointR: a delta from the previous point, the first being from (0, 0).
                x += Integer(stream);
                y += Integer(stream);
            }
            else if ((flags & 0x4000) != 0)
            {
                x = stream.I16();
                y = stream.I16();
            }
            else
            {
                x = stream.F32();
                y = stream.F32();
            }

            path._x.Add(x);
            path._y.Add(y);
        }

        if (withTypes)
        {
            path._types = new byte[path._x.Count];
            for (int i = 0; i < path._types.Length; i++) path._types[i] = stream.U8();
        }

        return path._x.Count > 0 ? path : null;
    }

    /// <summary>
    /// Reads an <c>EmfPlusInteger7</c> or <c>EmfPlusInteger15</c>.
    /// </summary>
    /// <remarks>
    /// One byte or two, and the top bit of the first says which. The sign lives in bit 6 of the
    /// first byte in both cases, so a seven-bit value is sign-extended from there rather than
    /// from bit 7 ([MS-EMFPLUS] 2.2.2.21 and 2.2.2.22).
    /// </remarks>
    private static int Integer(EmfPlusStream stream)
    {
        byte first = stream.U8();
        bool wide = (first & 0x80) != 0;
        bool negative = (first & 0x40) != 0;
        int value = first & 0x7F;

        if (negative) value |= 0x80;

        if (!wide) return (sbyte)value;

        return (short)((value << 8) | stream.U8());
    }

    /// <summary>The point at an index, in the file's own coordinates.</summary>
    /// <param name="index">Which point.</param>
    public (double X, double Y) Raw(int index) => (_x[index], _y[index]);

    /// <summary>
    /// The path's own bounding rectangle, unmapped, which a path-gradient brush needs.
    /// </summary>
    public (double Left, double Top, double Right, double Bottom) RawBounds()
    {
        double left = _x[0];
        double right = _x[0];
        double top = _y[0];
        double bottom = _y[0];

        for (int i = 1; i < _x.Count; i++)
        {
            left = Math.Min(left, _x[i]);
            right = Math.Max(right, _x[i]);
            top = Math.Min(top, _y[i]);
            bottom = Math.Max(bottom, _y[i]);
        }

        return (left, top, right, bottom);
    }

    /// <summary>
    /// Builds the figures the point types describe, mapping every point as it goes.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The rules are exactly <c>EMFPPath::GetPolygon</c>'s. A type of 0 starts a new figure; the
    /// low three bits being 3 marks a Bézier point, and <em>which</em> of the three it is comes
    /// from its distance past the last non-Bézier point rather than from the type itself; the top
    /// bit closes the figure. Getting the modular arithmetic wrong is the classic way a curve
    /// comes out as a polyline through its own control points.
    /// </para>
    /// <para>
    /// A path with no point-type array — an inline point list, which is what every
    /// <c>DrawLines</c> and <c>FillPolygon</c> carries — is a single figure of straight lines.
    /// </para>
    /// </remarks>
    /// <param name="map">Turns a point in the file's coordinates into a document point.</param>
    /// <param name="close">True to close the last figure whether or not a type said to.</param>
    /// <returns>One path holding every figure as a subpath.</returns>
    public GraphicsPath ToPath(Func<double, double, DocPoint> map, bool close = false)
    {
        ArgumentNullException.ThrowIfNull(map);

        GraphicsPath path = new();

        int lastNormal = 0;
        int placed = 0;
        DocPoint? control1 = null;
        DocPoint? control2 = null;
        bool anyOpen = false;

        for (int i = 0; i < _x.Count; i++)
        {
            if (placed > 0 && _types is not null && _types[i] == 0)
            {
                lastNormal = i;
                placed = 0;
                control1 = null;
                control2 = null;
            }

            DocPoint mapped = map(_x[i], _y[i]);

            if (_types is not null)
            {
                if ((_types[i] & 0x07) == 3)
                {
                    int position = (i - lastNormal) % 3;

                    if (position == 1 && placed > 0)
                    {
                        control1 = mapped;
                        continue;
                    }

                    if (position == 2 && placed > 0)
                    {
                        control2 = mapped;
                        continue;
                    }
                }
                else
                {
                    lastNormal = i;
                }
            }

            if (placed == 0)
            {
                path.MoveTo(mapped);
                anyOpen = true;
            }
            else if (control1 is { } first && control2 is { } second)
            {
                path.CubicTo(first, second, mapped);
            }
            else
            {
                path.LineTo(mapped);
            }

            control1 = null;
            control2 = null;
            placed++;

            if (_types is not null && (_types[i] & 0x80) != 0)
            {
                path.Close();
                anyOpen = false;
                placed = 0;
                lastNormal = i + 1;
            }
        }

        if (close && anyOpen) path.Close();

        return path;
    }

    /// <summary>
    /// The path as a cardinal spline through its points, which is what a curve record names.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A cardinal spline passes <em>through</em> every point rather than being pulled towards it,
    /// and the tension says how tightly it turns. The basis matrix and the eight-step evaluation
    /// are ported from <c>EMFPPath::GetCardinalSpline</c>; the endpoints are duplicated so that
    /// the first and last segments have the four control points the basis needs.
    /// </para>
    /// <para>
    /// It is flattened to a polyline rather than converted to Béziers because that is what
    /// LibreOffice draws, and a curve record is rare enough that the difference is not worth a
    /// second geometry path — eight steps a segment is under a tenth of a millimetre on a typical
    /// picture.
    /// </para>
    /// </remarks>
    /// <param name="map">Turns a point in the file's coordinates into a document point.</param>
    /// <param name="tension">The spline's tension, normally 0.5.</param>
    /// <param name="offset">Which segment to start at.</param>
    /// <param name="segments">How many segments to draw, or zero for all of them.</param>
    /// <param name="closed">True for a closed spline, which wraps round to the first point.</param>
    public GraphicsPath? Spline(
        Func<double, double, DocPoint> map,
        double tension,
        int offset,
        int segments,
        bool closed)
    {
        ArgumentNullException.ThrowIfNull(map);

        int count = _x.Count;
        if (closed ? count < 3 : count < 2) return null;

        List<double> xs = [.. _x];
        List<double> ys = [.. _y];

        if (closed)
        {
            xs.AddRange([_x[0], _x[1], _x[2]]);
            ys.AddRange([_y[0], _y[1], _y[2]]);
            offset = 0;
            segments = count;
        }
        else
        {
            xs.Insert(0, _x[0]);
            ys.Insert(0, _y[0]);
            xs.Add(_x[^1]);
            ys.Add(_y[^1]);

            if (segments <= 0 || segments >= count) segments = count - 1;
            if (offset >= segments) return null;
        }

        double[,] basis = CardinalMatrix(tension);
        GraphicsPath path = new();
        bool first = true;

        for (int i = 3 + offset; i < segments + 3; i++)
        {
            if (i >= xs.Count) break;

            for (int step = 0; step < Steps; step++)
            {
                double x = Coefficient(xs[i - 3], xs[i - 2], xs[i - 1], xs[i], step, basis);
                double y = Coefficient(ys[i - 3], ys[i - 2], ys[i - 1], ys[i], step, basis);

                DocPoint point = map(x, y);

                if (first)
                {
                    path.MoveTo(point);
                    first = false;
                }
                else
                {
                    path.LineTo(point);
                }
            }
        }

        if (first) return null;
        if (closed) path.Close();

        return path;
    }

    private const int Steps = 8;

    private static double[,] CardinalMatrix(double tension)
    {
        double[,] m = new double[4, 4];

        m[0, 1] = 2.0 - tension;
        m[0, 2] = tension - 2.0;
        m[1, 0] = 2.0 * tension;
        m[1, 1] = tension - 3.0;
        m[1, 2] = 3.0 - (2.0 * tension);
        m[3, 1] = 1.0;
        m[0, 3] = m[2, 2] = tension;
        m[0, 0] = m[1, 3] = m[2, 0] = -tension;

        return m;
    }

    private static double Coefficient(double p0, double p1, double p2, double p3, int step, double[,] m)
    {
        double a = (m[0, 0] * p0) + (m[0, 1] * p1) + (m[0, 2] * p2) + (m[0, 3] * p3);
        double b = (m[1, 0] * p0) + (m[1, 1] * p1) + (m[1, 2] * p2) + (m[1, 3] * p3);
        double c = (m[2, 0] * p0) + (m[2, 2] * p2);
        double t = (step + 1.0) / Steps;

        return p1 + (t * (c + (t * (b + (t * a)))));
    }
}
