using System.Buffers.Binary;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;

namespace XRay.Office.Text.Fonts;

/// <summary>
/// A glyph's filled outline, read from the face's own <c>glyf</c> table.
/// </summary>
/// <remarks>
/// <para>
/// <strong>This exists for one feature and should stay that way.</strong> Text reaches a backend as
/// a <see cref="GlyphRun"/> — glyph ids and positions — precisely so that a PDF can hold real,
/// searchable text and a rasteriser can hint. Turning a glyph into a path throws both away, so the
/// only caller entitled to it is one drawing something that <em>is not text in the reference's
/// output either</em>: WordArt warped along a <c>a:prstTxWarp</c>, which LibreOffice converts to
/// <c>tools::PolyPolygon</c> outlines in
/// <c>svx/source/customshapes/EnhancedCustomShapeFontWork.cxx</c> and draws as filled curves
/// carrying no glyph and no <c>ToUnicode</c>.
/// </para>
/// <para>
/// It reads <c>glyf</c> and nothing else. A CFF face — an <c>OTTO</c> file, whose outlines are Type 2
/// charstrings — answers null, and the caller falls back to whatever it does for a face it cannot
/// outline. That is a deliberate floor rather than an oversight: every face the corpus resolves
/// WordArt to is TrueType (Liberation, Carlito, Caladea, DejaVu and the Microsoft core fonts all
/// carry <c>glyf</c>), and a Type 2 interpreter is a second body of work with no measured reach.
/// </para>
/// <para>
/// The result is in document space, not font space: y grows downward and the origin is the pen
/// position on the baseline, so a path can be placed by translating it to where the pen is.
/// </para>
/// <para>
/// <strong>Nothing that draws text reaches this class</strong>, and it is worth saying because a
/// round has already looked here for a defect that was not here. A glyph run reaches the raster sink
/// as ids and positions and is drawn through Skia's own <c>SKFont.GetGlyphPath</c>, and reaches the
/// PDF sink as a font program to embed; neither asks this reader for anything. So a face this reader
/// cannot outline is not thereby a blank on a page — see <see cref="ColourBitmaps"/> for the one
/// flavour of face that <em>was</em>, and <see cref="GlyphPainting"/> for what decides it.
/// </para>
/// </remarks>
public static class GlyphOutlines
{
    /// <summary>How deep a composite glyph may nest before the reader gives up.</summary>
    /// <remarks>
    /// A malformed font can make a component refer to its own parent. Five levels is far past
    /// anything real — accented Latin is two, a stacked Vietnamese glyph three.
    /// </remarks>
    private const int MaximumComponentDepth = 5;

    /// <summary>Whether this face has outlines this reader can produce.</summary>
    public static bool CanOutline(OpenTypeFace? face)
        => face is not null && face.File.Has("glyf") && face.File.Has("loca");

    /// <summary>
    /// The outline of one glyph at the given em size, or null when the face has no <c>glyf</c>.
    /// </summary>
    /// <param name="face">The face to read.</param>
    /// <param name="glyphId">The glyph index within that face.</param>
    /// <param name="emSize">The em size to scale the design units to.</param>
    /// <returns>
    /// A path whose origin is the pen position on the baseline and whose y grows downward, or null
    /// when the face carries no <c>glyf</c> outlines. A blank glyph — a space — answers an empty
    /// path rather than null, because "nothing to draw" and "cannot be drawn" are different answers.
    /// </returns>
    public static GraphicsPath? Of(OpenTypeFace? face, ushort glyphId, Length emSize)
    {
        if (!CanOutline(face) || face is null) return null;

        List<Contour> contours = [];
        Read(face, glyphId, new Placement(1, 0, 0, 1, 0, 0), contours, 0);

        double scale = emSize.Emu / (double)face.UnitsPerEm;
        GraphicsPath path = new();

        foreach (Contour contour in contours) Emit(contour, scale, path);

        return path;
    }

    /// <summary>Turns one closed quadratic contour into cubic path commands.</summary>
    /// <remarks>
    /// TrueType stores a quadratic B-spline: consecutive off-curve points imply an on-curve point
    /// halfway between them, and a contour may begin off-curve. Both are handled by normalising the
    /// point list first, so the emitter below only ever sees on/off pairs. A quadratic maps to a
    /// cubic exactly — the two control points sit a third of the way from each end toward the
    /// quadratic's own — so nothing is approximated here.
    /// </remarks>
    private static void Emit(Contour contour, double scale, GraphicsPath path)
    {
        IReadOnlyList<OutlinePoint> points = contour.Points;
        if (points.Count == 0) return;

        // Find a point to start from. A contour may open on an off-curve point, in which case the
        // start is the implied midpoint before it.
        int start = -1;
        for (int i = 0; i < points.Count; i++)
        {
            if (points[i].OnCurve) { start = i; break; }
        }

        double startX;
        double startY;
        if (start < 0)
        {
            // Every point is off-curve, which a font is allowed to do for a circle: the whole
            // contour is implied midpoints.
            start = 0;
            startX = (points[0].X + points[^1].X) / 2;
            startY = (points[0].Y + points[^1].Y) / 2;
        }
        else
        {
            startX = points[start].X;
            startY = points[start].Y;
        }

        path.MoveTo(At(startX, startY, scale));

        double currentX = startX;
        double currentY = startY;
        double? controlX = null;
        double? controlY = null;

        for (int step = 1; step <= points.Count; step++)
        {
            OutlinePoint point = points[(start + step) % points.Count];

            if (!point.OnCurve)
            {
                if (controlX is { } heldX && controlY is { } heldY)
                {
                    // Two off-curve points in a row imply an on-curve point between them.
                    double midX = (heldX + point.X) / 2;
                    double midY = (heldY + point.Y) / 2;
                    Quadratic(path, currentX, currentY, heldX, heldY, midX, midY, scale);
                    currentX = midX;
                    currentY = midY;
                }

                controlX = point.X;
                controlY = point.Y;
                continue;
            }

            if (controlX is { } cx && controlY is { } cy)
            {
                Quadratic(path, currentX, currentY, cx, cy, point.X, point.Y, scale);
                controlX = null;
                controlY = null;
            }
            else
            {
                path.LineTo(At(point.X, point.Y, scale));
            }

            currentX = point.X;
            currentY = point.Y;
        }

        // The loop above ends on the start point, so a control still held belongs to the closing
        // segment back to it.
        if (controlX is { } lastX && controlY is { } lastY)
        {
            Quadratic(path, currentX, currentY, lastX, lastY, startX, startY, scale);
        }

        path.Close();
    }

    /// <summary>Appends one quadratic segment, as the cubic that traces the same curve.</summary>
    private static void Quadratic(
        GraphicsPath path,
        double fromX, double fromY,
        double controlX, double controlY,
        double toX, double toY,
        double scale)
    {
        double c1x = fromX + (2.0 / 3.0 * (controlX - fromX));
        double c1y = fromY + (2.0 / 3.0 * (controlY - fromY));
        double c2x = toX + (2.0 / 3.0 * (controlX - toX));
        double c2y = toY + (2.0 / 3.0 * (controlY - toY));

        path.CubicTo(At(c1x, c1y, scale), At(c2x, c2y, scale), At(toX, toY, scale));
    }

    /// <summary>A design-unit point in document space: scaled, and with y pointing down.</summary>
    private static DocPoint At(double x, double y, double scale)
        => new(Length.FromEmu((long)Math.Round(x * scale)), Length.FromEmu((long)Math.Round(-y * scale)));

    /// <summary>Reads one glyph's contours, following components when it is a composite.</summary>
    private static void Read(
        OpenTypeFace face, ushort glyphId, Placement placement, List<Contour> into, int depth)
    {
        if (depth > MaximumComponentDepth) return;

        ReadOnlySpan<byte> glyf = face.File.Table("glyf");
        if (!Locate(face, glyphId, out int offset, out int length)) return;
        if (length < 10 || offset + length > glyf.Length) return;

        ReadOnlySpan<byte> glyph = glyf.Slice(offset, length);
        int contourCount = BinaryPrimitives.ReadInt16BigEndian(glyph);

        if (contourCount >= 0) ReadSimple(glyph, contourCount, placement, into);
        else ReadComposite(face, glyph, placement, into, depth);
    }

    /// <summary>Where a glyph's record sits in <c>glyf</c>, from <c>loca</c>.</summary>
    private static bool Locate(OpenTypeFace face, ushort glyphId, out int offset, out int length)
    {
        offset = 0;
        length = 0;

        ReadOnlySpan<byte> loca = face.File.Table("loca");
        bool isLong = face.Head.IndexToLocFormat != 0;
        int entry = isLong ? 4 : 2;

        int start = glyphId * entry;
        if (start + (2 * entry) > loca.Length) return false;

        long from = isLong
            ? BinaryPrimitives.ReadUInt32BigEndian(loca[start..])
            : BinaryPrimitives.ReadUInt16BigEndian(loca[start..]) * 2L;
        long to = isLong
            ? BinaryPrimitives.ReadUInt32BigEndian(loca[(start + entry)..])
            : BinaryPrimitives.ReadUInt16BigEndian(loca[(start + entry)..]) * 2L;

        if (to <= from || from < 0 || to > int.MaxValue) return false;

        offset = (int)from;
        length = (int)(to - from);
        return true;
    }

    /// <summary>Reads a simple glyph: end points, flags, then the two delta-coded coordinate runs.</summary>
    private static void ReadSimple(
        ReadOnlySpan<byte> glyph, int contourCount, Placement placement, List<Contour> into)
    {
        int cursor = 10;
        if (cursor + (contourCount * 2) + 2 > glyph.Length) return;

        int[] ends = new int[contourCount];
        for (int i = 0; i < contourCount; i++)
        {
            ends[i] = BinaryPrimitives.ReadUInt16BigEndian(glyph[cursor..]);
            cursor += 2;
        }

        int pointCount = contourCount == 0 ? 0 : ends[^1] + 1;
        if (pointCount <= 0) return;

        int instructions = BinaryPrimitives.ReadUInt16BigEndian(glyph[cursor..]);
        cursor += 2 + instructions;
        if (cursor > glyph.Length) return;

        byte[] flags = new byte[pointCount];
        for (int i = 0; i < pointCount;)
        {
            if (cursor >= glyph.Length) return;

            byte flag = glyph[cursor++];
            flags[i++] = flag;

            if ((flag & 0x08) == 0) continue;
            if (cursor >= glyph.Length) return;

            int repeat = glyph[cursor++];
            for (int r = 0; r < repeat && i < pointCount; r++) flags[i++] = flag;
        }

        int[] xs = new int[pointCount];
        int value = 0;
        for (int i = 0; i < pointCount; i++)
        {
            byte flag = flags[i];
            if ((flag & 0x02) != 0)
            {
                if (cursor >= glyph.Length) return;
                int delta = glyph[cursor++];
                value += (flag & 0x10) != 0 ? delta : -delta;
            }
            else if ((flag & 0x10) == 0)
            {
                if (cursor + 2 > glyph.Length) return;
                value += BinaryPrimitives.ReadInt16BigEndian(glyph[cursor..]);
                cursor += 2;
            }

            xs[i] = value;
        }

        int[] ys = new int[pointCount];
        value = 0;
        for (int i = 0; i < pointCount; i++)
        {
            byte flag = flags[i];
            if ((flag & 0x04) != 0)
            {
                if (cursor >= glyph.Length) return;
                int delta = glyph[cursor++];
                value += (flag & 0x20) != 0 ? delta : -delta;
            }
            else if ((flag & 0x20) == 0)
            {
                if (cursor + 2 > glyph.Length) return;
                value += BinaryPrimitives.ReadInt16BigEndian(glyph[cursor..]);
                cursor += 2;
            }

            ys[i] = value;
        }

        int first = 0;
        foreach (int end in ends)
        {
            if (end < first || end >= pointCount) break;

            List<OutlinePoint> points = new(end - first + 1);
            for (int i = first; i <= end; i++)
            {
                (double x, double y) = placement.Apply(xs[i], ys[i]);
                points.Add(new OutlinePoint(x, y, (flags[i] & 0x01) != 0));
            }

            into.Add(new Contour(points));
            first = end + 1;
        }
    }

    /// <summary>Reads a composite glyph, placing each component through its own matrix.</summary>
    /// <remarks>
    /// Only the <c>ARGS_ARE_XY_VALUES</c> form of the offset is honoured. The other form matches a
    /// point of the component against a point of the composite, which needs the parent's points to
    /// resolve and which no shipped Latin face uses; a component stating it is placed at its own
    /// origin rather than dropped.
    /// </remarks>
    private static void ReadComposite(
        OpenTypeFace face, ReadOnlySpan<byte> glyph, Placement placement, List<Contour> into, int depth)
    {
        int cursor = 10;

        while (cursor + 4 <= glyph.Length)
        {
            int flags = BinaryPrimitives.ReadUInt16BigEndian(glyph[cursor..]);
            ushort component = BinaryPrimitives.ReadUInt16BigEndian(glyph[(cursor + 2)..]);
            cursor += 4;

            bool wordArguments = (flags & 0x0001) != 0;
            bool xyValues = (flags & 0x0002) != 0;

            double dx = 0;
            double dy = 0;
            if (wordArguments)
            {
                if (cursor + 4 > glyph.Length) return;
                if (xyValues)
                {
                    dx = BinaryPrimitives.ReadInt16BigEndian(glyph[cursor..]);
                    dy = BinaryPrimitives.ReadInt16BigEndian(glyph[(cursor + 2)..]);
                }

                cursor += 4;
            }
            else
            {
                if (cursor + 2 > glyph.Length) return;
                if (xyValues)
                {
                    dx = (sbyte)glyph[cursor];
                    dy = (sbyte)glyph[cursor + 1];
                }

                cursor += 2;
            }

            double a = 1;
            double b = 0;
            double c = 0;
            double d = 1;

            if ((flags & 0x0008) != 0)
            {
                if (cursor + 2 > glyph.Length) return;
                a = d = F2Dot14(glyph[cursor..]);
                cursor += 2;
            }
            else if ((flags & 0x0040) != 0)
            {
                if (cursor + 4 > glyph.Length) return;
                a = F2Dot14(glyph[cursor..]);
                d = F2Dot14(glyph[(cursor + 2)..]);
                cursor += 4;
            }
            else if ((flags & 0x0080) != 0)
            {
                if (cursor + 8 > glyph.Length) return;
                a = F2Dot14(glyph[cursor..]);
                b = F2Dot14(glyph[(cursor + 2)..]);
                c = F2Dot14(glyph[(cursor + 4)..]);
                d = F2Dot14(glyph[(cursor + 6)..]);
                cursor += 8;
            }

            Placement inner = placement.Concat(new Placement(a, b, c, d, dx, dy));
            Read(face, component, inner, into, depth + 1);

            if ((flags & 0x0020) == 0) return;
        }
    }

    /// <summary>The 2.14 fixed-point number a component's matrix entries are stored as.</summary>
    private static double F2Dot14(ReadOnlySpan<byte> bytes)
        => BinaryPrimitives.ReadInt16BigEndian(bytes) / 16384.0;

    /// <summary>One point of a quadratic contour, in design units with y pointing up.</summary>
    private readonly record struct OutlinePoint(double X, double Y, bool OnCurve);

    /// <summary>One closed contour.</summary>
    private sealed record Contour(IReadOnlyList<OutlinePoint> Points);

    /// <summary>A component's placement inside its composite: a 2×2 matrix and an offset.</summary>
    private readonly record struct Placement(double A, double B, double C, double D, double X, double Y)
    {
        /// <summary>Maps a design-unit point through this placement.</summary>
        public (double X, double Y) Apply(double x, double y)
            => ((A * x) + (C * y) + X, (B * x) + (D * y) + Y);

        /// <summary>This placement with another applied inside it.</summary>
        public Placement Concat(Placement inner) => new(
            (inner.A * A) + (inner.B * C),
            (inner.A * B) + (inner.B * D),
            (inner.C * A) + (inner.D * C),
            (inner.C * B) + (inner.D * D),
            (inner.X * A) + (inner.Y * C) + X,
            (inner.X * B) + (inner.Y * D) + Y);
    }
}
