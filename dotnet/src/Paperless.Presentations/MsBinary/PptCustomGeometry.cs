using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.MsBinary.Escher;
using Paperless.MsBinary.Records;

namespace Paperless.Presentations.MsBinary;

/// <summary>
/// Builds a shape's outline from the vertex and segment arrays it states outright.
/// </summary>
/// <remarks>
/// <para>
/// The binary drawing layer's answer to DrawingML's <c>a:custGeom</c>: <c>pVertices</c> (325) is
/// an array of points in the shape's own geometry space and <c>pSegmentInfo</c> (326) is a
/// program saying how to join them — move, line, cubic, close, end.
/// </para>
/// <para>
/// <strong>This is not a corner case, it is the common case.</strong> LibreOffice's own PPT
/// export writes a vertex array on nearly every shape it emits rather than naming a preset, so a
/// deck round-tripped through Impress states <em>no</em> preset geometry at all: a right-angled
/// triangle comes out as shape type 4095 — <c>mso_sptNil</c> — with three points and four
/// segment words. Without this, every such shape draws as its bounding rectangle, which on the
/// converted corpus deck is four triangles drawn as four boxes.
/// </para>
/// <para>
/// The geometry space is <c>geoLeft</c>/<c>geoTop</c>/<c>geoRight</c>/<c>geoBottom</c> (320-323),
/// which default to a 21600-unit box; the points are mapped from it onto the shape's extent.
/// Where the box is degenerate — which a writer that states the properties as zero produces, and
/// LibreOffice's does — the vertices are already in the 21600 box, so that is what is substituted
/// rather than a division by zero.
/// </para>
/// </remarks>
internal static class PptCustomGeometry
{
    /// <summary>The vertex array.</summary>
    private const ushort Vertices = 325;

    /// <summary>The segment program.</summary>
    private const ushort Segments = 326;

    /// <summary>The four properties bounding the geometry space.</summary>
    private const ushort GeoLeft = 320;
    private const ushort GeoTop = 321;
    private const ushort GeoRight = 322;
    private const ushort GeoBottom = 323;

    /// <summary>The view box a shape with no geometry properties is measured in.</summary>
    private const int DefaultViewBox = 21600;

    /// <summary>
    /// Whether a stated coordinate is a reference to a guide rather than a number.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The top bit means "the low bits index <c>pFormulas</c>", so the point is the result of an
    /// expression rather than a position — which is the preset evaluator's job and not this one's.
    /// A shape whose path is built that way is handed back to the caller unresolved so that it
    /// draws its bounding rectangle, which is where it is and the wrong shape, rather than a
    /// path built from coordinates near ±2³¹.
    /// </para>
    /// <para>
    /// <strong>And LibreOffice cannot draw them either, in the file that matters most.</strong>
    /// Its PPTX-to-PPT export writes a guide-referencing vertex array on every preset shape
    /// <em>and</em> writes <c>geoLeft</c> through <c>geoBottom</c> as zero, so every guide
    /// evaluates in a zero-sized view box: <c>shape-geometry</c>'s four triangles come out of its
    /// own PDF export as four one-point paths — <c>72 468 m 72 468 l 72 468 l h f*</c>. Drawing
    /// their bounding rectangles is strictly more than the reference manages.
    /// </para>
    /// </remarks>
    private static bool IsGuide(uint coordinate) => (coordinate & 0x80000000) != 0;

    /// <summary>Whether the shape states a path of its own.</summary>
    public static bool Has(EscherPropertyTable properties)
    {
        ArgumentNullException.ThrowIfNull(properties);
        return properties.Has(Vertices);
    }

    /// <summary>
    /// The outline the shape's own arrays describe, in its local coordinates, or null when they
    /// cannot be read.
    /// </summary>
    /// <param name="properties">The shape's property table.</param>
    /// <param name="size">The shape's extent, which the geometry space maps onto.</param>
    public static GraphicsPath? Outline(EscherPropertyTable properties, DocSize size)
    {
        ArgumentNullException.ThrowIfNull(properties);

        ReadOnlySpan<byte> points = properties.Array(Vertices, out int count, out int pointSize);
        if (count < 2 || pointSize < 4) return null;

        int left = properties.SignedValue(GeoLeft);
        int top = properties.SignedValue(GeoTop);
        int right = properties.SignedValue(GeoRight);
        int bottom = properties.SignedValue(GeoBottom);

        if (right <= left) { left = 0; right = DefaultViewBox; }
        if (bottom <= top) { top = 0; bottom = DefaultViewBox; }

        double scaleX = size.Width.Emu / (double)(right - left);
        double scaleY = size.Height.Emu / (double)(bottom - top);

        // Mapped into slide-shaped coordinates up front rather than on demand: the array lives in
        // a span, and a span cannot be captured by the closure a lazy reader would need.
        DocPoint[] mapped = new DocPoint[count];
        for (int i = 0; i < count; i++)
        {
            ReadOnlySpan<byte> element = points[(i * pointSize)..];

            if (pointSize >= 8
                && (IsGuide(DffRecordBuffer.ReadUInt32(element))
                    || IsGuide(DffRecordBuffer.ReadUInt32(element[4..]))))
            {
                return null;
            }

            (int x, int y) = pointSize >= 8
                ? (unchecked((int)DffRecordBuffer.ReadUInt32(element)),
                   unchecked((int)DffRecordBuffer.ReadUInt32(element[4..])))
                : (unchecked((short)DffRecordBuffer.ReadUInt16(element)),
                   unchecked((short)DffRecordBuffer.ReadUInt16(element[2..])));

            mapped[i] = new DocPoint(
                Length.FromEmu((long)Math.Round((x - left) * scaleX)),
                Length.FromEmu((long)Math.Round((y - top) * scaleY)));
        }

        ReadOnlySpan<byte> program = properties.Array(Segments, out int steps, out int stepSize);

        return steps > 0 && stepSize == 2 ? Run(program, steps, mapped) : Polygon(mapped);
    }

    /// <summary>
    /// Runs the segment program.
    /// </summary>
    /// <remarks>
    /// The opcodes that matter, from <c>msdffimp.cxx:2278-2400</c>: <c>0x4000</c> starts a
    /// sub-path at the next point, <c>0x6000</c> closes it, <c>0x8000</c> ends the program, and
    /// anything else is a run whose low twelve bits are the count — <c>0x2000</c> and above are
    /// cubic segments of three points each, everything below is a line of one. A word the reader
    /// does not know consumes nothing, which leaves the path short rather than reading the vertex
    /// array off its end.
    /// </remarks>
    private static GraphicsPath Run(
        ReadOnlySpan<byte> program, int steps, ReadOnlySpan<DocPoint> at)
    {
        int count = at.Length;
        GraphicsPath path = new();
        int point = 0;
        bool started = false;

        for (int step = 0; step < steps; step++)
        {
            ushort word = DffRecordBuffer.ReadUInt16(program[(step * 2)..]);

            if (word == 0x8000) break;

            if (word == 0x4000)
            {
                if (point >= count) break;
                if (started) path.Close();
                path.MoveTo(at[point++]);
                started = true;
                continue;
            }

            if (word == 0x6000)
            {
                if (started) path.Close();
                started = false;
                continue;
            }

            int repeats = Math.Max(word & 0x0FFF, 1);
            bool cubic = (word & 0xF000) == 0x2000;

            for (int i = 0; i < repeats; i++)
            {
                if (!started)
                {
                    if (point >= count) break;
                    path.MoveTo(at[point++]);
                    started = true;
                    continue;
                }

                if (cubic)
                {
                    if (point + 3 > count) break;
                    path.CubicTo(at[point], at[point + 1], at[point + 2]);
                    point += 3;
                }
                else
                {
                    if (point >= count) break;
                    path.LineTo(at[point++]);
                }
            }
        }

        if (started) path.Close();
        return path;
    }

    /// <summary>A shape stating vertices and no program is a closed polygon through them.</summary>
    private static GraphicsPath Polygon(ReadOnlySpan<DocPoint> at)
    {
        GraphicsPath path = new();
        path.MoveTo(at[0]);
        for (int i = 1; i < at.Length; i++) path.LineTo(at[i]);
        path.Close();
        return path;
    }
}
