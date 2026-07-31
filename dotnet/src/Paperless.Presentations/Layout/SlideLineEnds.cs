using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Presentations.Layout;

/// <summary>One end of a line: what marker it carries and how big.</summary>
/// <param name="Type">
/// <c>a:headEnd/@type</c> or <c>a:tailEnd/@type</c>: <c>triangle</c>, <c>arrow</c>,
/// <c>stealth</c>, <c>diamond</c>, <c>oval</c> or <c>none</c>.
/// </param>
/// <param name="Width">The <c>w</c> attribute: <c>sm</c>, <c>med</c> or <c>lg</c>.</param>
/// <param name="Length">The <c>len</c> attribute, the same three values.</param>
public readonly record struct SlideLineEnd(string? Type, string? Width, string? Length);

/// <summary>
/// Draws the arrowheads at the ends of a line, and shortens the line to make room for them.
/// </summary>
/// <remarks>
/// <para>
/// An arrowhead is a <em>filled polygon</em>, not a property of a stroke, which is why nothing in
/// the display list has to know about it: what comes out of here is one more
/// <see cref="PlacedShape"/> per end, filled with the line's own colour, plus a shortened shaft.
/// LibreOffice does the same thing at the same layer — <c>PolygonStrokeArrowPrimitive2D</c>
/// decomposes into a stroke and up to two filled polygons
/// (<c>drawinglayer/source/primitive2d/polygonprimitive2d.cxx:704-760</c>).
/// </para>
/// <para>
/// <strong>The size is a multiple of the line width, floored at 0.7 mm.</strong>
/// <c>lclPushMarkerProperties</c> (<c>oox/source/drawingml/lineproperties.cxx:239-330</c>) takes
/// <c>max(lineWidth, 70)</c> in hundredths of a millimetre and multiplies it by 2, 3 or 5 for a
/// small, medium or large marker — 2.5, 3.5 or 5.5 for the open <c>arrow</c>, which is drawn
/// thinner. So a medium triangle on a three-point line is 9.01 pt across and 9.01 pt long, and on
/// a hairline it is 2.1 pt rather than nothing.
/// </para>
/// <para>
/// <strong>The shaft is shortened by the marker's length less a fifteenth of its width.</strong>
/// That fifteenth is a deliberate overlap — "a compromise between straight and peaked markers",
/// in the comment at <c>polygonprimitive2d.cxx:730</c> — and without it a peaked marker shows a
/// gap between its notch and the line. Measured on <c>slide-shape-features.pptx</c>: a 15.02 pt
/// stealth tail on a line ending at 504 pt leaves the shaft ending at 489.969, which is
/// 15.024 − 15.024/15 to within six thousandths of a point.
/// </para>
/// </remarks>
public static class SlideLineEnds
{
    /// <summary>
    /// The smallest marker base, in hundredths of a millimetre: 0.7 mm.
    /// </summary>
    /// <remarks>
    /// <c>nBaseLineWidth = max(nLineWidth, 70)</c>. Without the floor a hairline — which is what
    /// LibreOffice's own export writes for a line whose width the file omits — would carry an
    /// arrowhead of no size at all.
    /// </remarks>
    private const long MinimumBaseMm100 = 70;

    /// <summary>
    /// Draws the markers a line's two ends carry and returns the shaft to draw instead of it.
    /// </summary>
    /// <param name="outline">The line, in slide coordinates. Must be an open polyline.</param>
    /// <param name="stroke">The pen, whose paint the markers are filled with.</param>
    /// <param name="head">The marker at the start, if any.</param>
    /// <param name="tail">The marker at the end, if any.</param>
    /// <param name="name">The shape's name, carried onto each marker.</param>
    public static (GraphicsPath Shaft, List<PlacedShape> Markers) Apply(
        GraphicsPath outline, Stroke stroke, SlideLineEnd head, SlideLineEnd tail, string? name)
    {
        ArgumentNullException.ThrowIfNull(outline);
        ArgumentNullException.ThrowIfNull(stroke);

        List<PlacedShape> markers = [];
        if (Polyline(outline) is not { Count: >= 2 } points) return (outline, markers);

        long baseWidth = Math.Max(
            stroke.Width.Emu, Length.FromMm100(MinimumBaseMm100).Emu);

        double trimStart = End(head, points[0], points[1], baseWidth, stroke, name, markers);
        double trimEnd = End(
            tail, points[^1], points[^2], baseWidth, stroke, name, markers);

        return (Trimmed(points, trimStart, trimEnd), markers);
    }

    /// <summary>
    /// Places one end's marker and returns how much of the shaft it consumes.
    /// </summary>
    /// <param name="end">The marker.</param>
    /// <param name="tip">The line's own end point, where the marker's point goes.</param>
    /// <param name="towards">The next point along the line, which gives the direction.</param>
    /// <param name="baseWidth">The size the marker is a multiple of, in EMUs.</param>
    /// <param name="stroke">The pen, for the fill colour and for the open arrow's own width.</param>
    /// <param name="name">The shape's name.</param>
    /// <param name="markers">Receives the marker, when there is one.</param>
    private static double End(
        SlideLineEnd end,
        DocPoint tip,
        DocPoint towards,
        long baseWidth,
        Stroke stroke,
        string? name,
        List<PlacedShape> markers)
    {
        if (Outline(end.Type) is not { } shape) return 0;

        bool open = end.Type == "arrow";
        double width = Size(end.Width, open) * baseWidth;
        double length = Size(end.Length, open) * baseWidth;

        // The open arrow's own stroke thickness, as a percentage of the marker's width — the
        // marker is a stroked outline drawn as a filled polygon, so it has to know how thick it
        // is (lineproperties.cxx:313).
        double halfWidth = Math.Max(100.0 * 0.5 * stroke.Width.Emu / Math.Max(1, width), 1.0);

        double dx = towards.X.Emu - tip.X.Emu;
        double dy = towards.Y.Emu - tip.Y.Emu;
        double span = Math.Sqrt((dx * dx) + (dy * dy));
        if (span <= 0) return 0;

        // The marker's own axes: y runs from the tip back along the line, x across it.
        double ax = dx / span;
        double ay = dy / span;

        bool centred = end.Type is "diamond" or "oval";
        double docking = centred ? 0.5 : 0.0;

        GraphicsPath path = new();
        bool first = true;

        foreach ((double px, double py) in shape(halfWidth))
        {
            // Local (0..100, 0..100), scaled so the marker is `width` across and `length` along,
            // then rotated onto the line and moved to the tip. A centred marker slides back by
            // half its length so that its middle, not its point, sits on the end.
            double across = ((px / 100.0) - 0.5) * width;
            double along = ((py / 100.0) - docking) * length;

            // The across axis is the direction turned anticlockwise in this y-down space, which is
            // what puts the polygon's vertices out in the order LibreOffice writes them: its head
            // triangle runs tip, upper-right, lower-right, and turning the other way reverses the
            // winding — invisible under a non-zero fill and not under an even-odd one.
            DocPoint point = new(
                Length.FromEmu((long)Math.Round(tip.X.Emu + (along * ax) + (across * ay))),
                Length.FromEmu((long)Math.Round(tip.Y.Emu + (along * ay) - (across * ax))));

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

        path.Close();

        markers.Add(new PlacedShape
        {
            Name = name,
            Outline = path,
            Bounds = DocRect.Empty,
            Fill = stroke.Paint,
        });

        // What the shaft gives up: the marker's length beyond the docking point, less the
        // fifteenth of its width that makes the two overlap.
        return Math.Max(0, (length * (1.0 - docking)) - (width / 15.0));
    }

    /// <summary>How many times the base width a marker is, by <c>sm</c>/<c>med</c>/<c>lg</c>.</summary>
    /// <remarks>
    /// The open <c>arrow</c> is half a step larger at every size, because it is an outline rather
    /// than a solid and would otherwise read as smaller (<c>lineproperties.cxx:287-298</c>).
    /// </remarks>
    private static double Size(string? size, bool open) => size switch
    {
        "sm" => open ? 2.5 : 2.0,
        "lg" => open ? 5.5 : 5.0,
        _ => open ? 3.5 : 3.0,
    };

    /// <summary>
    /// The marker outlines, in a hundred-by-hundred space with the point at the top.
    /// </summary>
    /// <remarks>
    /// Transcribed from <c>lclPushMarkerProperties</c>, which builds exactly these five polygons
    /// (<c>oox/source/drawingml/lineproperties.cxx:318-368</c>). The <c>oval</c> is a polygon
    /// there too — twelve points round the ellipse — rather than a curve, so this is the shape
    /// LibreOffice actually fills and not an approximation of it.
    /// </remarks>
    private static Func<double, IEnumerable<(double X, double Y)>>? Outline(string? type)
        => type switch
        {
            "triangle" => _ => [(50, 0), (100, 100), (0, 100)],
            "stealth" => _ => [(50, 0), (100, 100), (50, 60), (0, 100)],
            "diamond" => _ => [(50, 0), (100, 50), (50, 100), (0, 50)],
            "oval" => _ =>
            [
                (50, 0), (75, 7), (93, 25), (100, 50), (93, 75), (75, 93),
                (50, 100), (25, 93), (7, 75), (0, 50), (7, 25), (25, 7),
            ],
            "arrow" => half =>
            [
                (50, 0),
                (100, 100 - (half * 1.5)),
                (100 - (half * 1.5), 100),
                (50 + half, 5.5 * half),
                (50 + half, 100),
                (50 - half, 100),
                (50 - half, 5.5 * half),
                (half * 1.5, 100),
                (0, 100 - (half * 1.5)),
            ],
            _ => null,
        };

    /// <summary>
    /// The polyline a path is, or null when it is not one.
    /// </summary>
    /// <remarks>
    /// Straight segments only, and open: an arrowhead belongs on a line, and a closed or curved
    /// path is left alone rather than being given one at an arbitrary point. Every connector
    /// preset — <c>line</c>, <c>straightConnector1</c>, the bent ones — is a polyline.
    /// </remarks>
    private static List<DocPoint>? Polyline(GraphicsPath path)
    {
        List<DocPoint> points = [];

        foreach (PathCommand command in path.Commands)
        {
            switch (command.Verb)
            {
                case PathVerb.MoveTo when points.Count == 0:
                case PathVerb.LineTo when points.Count > 0:
                    points.Add(command.Point);
                    break;
                default:
                    return null;
            }
        }

        return points.Count >= 2 ? points : null;
    }

    /// <summary>The polyline with a length taken off each end.</summary>
    /// <remarks>
    /// Within the first and last segments, which is where a marker always fits: a marker longer
    /// than the segment it sits on would leave nothing to draw, so the trim is clamped rather
    /// than allowed to walk into the next segment and reverse the line.
    /// </remarks>
    private static GraphicsPath Trimmed(List<DocPoint> points, double fromStart, double fromEnd)
    {
        List<DocPoint> trimmed = [.. points];

        if (fromStart > 0) trimmed[0] = Along(trimmed[0], trimmed[1], fromStart);
        if (fromEnd > 0) trimmed[^1] = Along(trimmed[^1], trimmed[^2], fromEnd);

        GraphicsPath path = new();
        path.MoveTo(trimmed[0]);
        for (int i = 1; i < trimmed.Count; i++) path.LineTo(trimmed[i]);
        return path;
    }

    private static DocPoint Along(DocPoint from, DocPoint towards, double distance)
    {
        double dx = towards.X.Emu - from.X.Emu;
        double dy = towards.Y.Emu - from.Y.Emu;
        double span = Math.Sqrt((dx * dx) + (dy * dy));
        if (span <= 0) return from;

        double fraction = Math.Min(distance / span, 0.5);

        return new DocPoint(
            Length.FromEmu((long)Math.Round(from.X.Emu + (dx * fraction))),
            Length.FromEmu((long)Math.Round(from.Y.Emu + (dy * fraction))));
    }
}
