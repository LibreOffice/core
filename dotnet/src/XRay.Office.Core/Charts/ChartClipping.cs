using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;

namespace XRay.Office.Core.Charts;

/// <summary>
/// Clips a series' own geometry at the plot rectangle, the way every chart2 plotter does before
/// it makes a shape of it.
/// </summary>
/// <remarks>
/// <para>
/// <strong>A series mark contributes only the part of itself that falls inside the plot, and the
/// reference clips the <em>geometry</em> rather than painting the whole shape under a clip
/// path.</strong> Every plotter runs its polygon through
/// <c>Clipping::clipPolygonAtRectangle</c> against
/// <c>PlottingPositionHelper::getScaledLogicClipDoubleRect</c> before transforming it to the
/// scene — <c>chart2/source/view/charttypes/AreaChart.cxx</c>:318, 336, 343, 359 and 445,
/// <c>NetChart.cxx</c>:138, 144 and 213, <c>BarChart.cxx</c>:533, and
/// <c>VSeriesPlotter.cxx</c>:1423 for a regression curve. That rectangle is the axis' own
/// minimum and maximum after the scaling function
/// (<c>chart2/source/view/main/PlottingPositionHelper.cxx</c>:295-311), which the transform maps
/// exactly onto the plot rectangle, so clipping in page space against the plot rectangle is the
/// same cut.
/// </para>
/// <para>
/// <strong>Measured on <c>171128IPAP.pptx</c> slide 38</strong>, whose line chart states a date
/// axis running 40179…43831 over points beginning in 2006: 26.2.4.2 draws all three series
/// between x = 119.54 and x = 615.49, which is its plot rectangle to the hundredth of a point,
/// and this tree drew them from x = −866.50 to x = 758.03 on a 720 pt page. Three of the deck's
/// forty pages carried vector ink outside the page and none of the reference's did.
/// </para>
/// <para>
/// The algorithm is Liang–Barsky per segment (<c>Clipping.cxx</c>:47-128) with chart2's own
/// piece-joining on top (<c>:340-421</c>): consecutive visible segments are collected into one
/// run, and a run that had to skip a segment starts a new one — <c>clipPolygonAtRectangle</c>'s
/// <c>bSplitPiecesToDifferentPolygons</c>, which defaults to <c>true</c>
/// (<c>chart2/source/view/inc/Clipping.hxx</c>:51) and is passed <c>false</c> only by the two
/// callers that clip a <em>filled</em> polygon. A line that leaves the plot and comes back is
/// therefore two strokes and not one, with no chord drawn across the gap.
/// </para>
/// <para>
/// The bounding-box short circuit is the reference's own (<c>:350-365</c>) and is what keeps this
/// free: a series that fits its plot is returned unchanged and byte for byte, so only a chart
/// that already drew outside its plot can move.
/// </para>
/// </remarks>
public static class ChartClipping
{
    /// <summary>
    /// Whether a point falls inside the plot rectangle, edges included.
    /// </summary>
    /// <remarks>
    /// <c>PlottingPositionHelper::isLogicVisible</c>
    /// (<c>chart2/source/view/inc/PlottingPositionHelper.hxx</c>:333-339) asked of a point's
    /// logic values, which for our linear mapping is this test on the page. <c>AreaChart</c>
    /// consults it before it makes any mark at all — <c>if (!bIsVisible) continue;</c>,
    /// <c>AreaChart.cxx</c>:760-761, above the symbol, the error bars <em>and</em> the data label
    /// — so a point outside the axis range keeps its place in the polyline and gets nothing else.
    /// </remarks>
    /// <param name="rect">The plot rectangle.</param>
    /// <param name="point">The point.</param>
    public static bool Contains(DocRect rect, DocPoint point)
        => point.X >= rect.Left && point.X <= rect.Right
        && point.Y >= rect.Top && point.Y <= rect.Bottom;

    /// <summary>
    /// The parts of a polyline that fall inside a rectangle, in order.
    /// </summary>
    /// <param name="points">The polyline's vertices, in order.</param>
    /// <param name="rect">The rectangle to clip at — the plot rectangle.</param>
    /// <returns>
    /// One list per surviving run. Empty when nothing of the polyline is inside; the input itself
    /// when all of it is.
    /// </returns>
    public static IReadOnlyList<IReadOnlyList<DocPoint>> ClipPolyline(
        IReadOnlyList<DocPoint> points, DocRect rect)
    {
        ArgumentNullException.ThrowIfNull(points);

        if (points.Count == 0) return [];

        // "need clipping?" — Clipping.cxx:350-365. Wholly inside is returned untouched, which is
        // what makes this a no-op for every chart whose series fit their plot; wholly outside
        // is rejected without walking a segment.
        Length minX = points[0].X, maxX = points[0].X;
        Length minY = points[0].Y, maxY = points[0].Y;

        for (int at = 1; at < points.Count; at++)
        {
            minX = Length.Min(minX, points[at].X);
            maxX = Length.Max(maxX, points[at].X);
            minY = Length.Min(minY, points[at].Y);
            maxY = Length.Max(maxY, points[at].Y);
        }

        if (minX >= rect.Left && maxX <= rect.Right && minY >= rect.Top && maxY <= rect.Bottom)
            return [points];

        if (maxX < rect.Left || minX > rect.Right || maxY < rect.Top || minY > rect.Bottom)
            return [];

        List<IReadOnlyList<DocPoint>> pieces = [];
        List<DocPoint>? run = null;

        // chart2 seeds "the previous segment's end" with a point that cannot be on the polyline,
        // so the first visible segment always opens a run (Clipping.cxx:379-381).
        DocPoint? last = null;

        for (int at = 1; at < points.Count; at++)
        {
            if (!ClipSegment(points[at - 1], points[at], rect, out DocPoint from, out DocPoint to))
                continue;

            if (last is { } end && end == from)
            {
                // The run continues through this segment; only its far end is new.
                if (to != from) run!.Add(to);
            }
            else
            {
                // A break — either the first surviving segment or one that follows a rejected
                // stretch. `nOldPoint != 1` in the reference: the very first segment cannot
                // start a *second* run, and neither can a break before anything was kept.
                if (at != 1 && run is { Count: > 0 }) run = null;

                if (run is null)
                {
                    run = [];
                    pieces.Add(run);
                }

                run.Add(from);
                if (to != from) run.Add(to);
            }

            last = to;
        }

        // A single point is not a stroke. chart2 reaches the same answer through
        // ShapeFactory::hasPolygonAnyLines, which refuses a polygon of fewer than two points.
        pieces.RemoveAll(piece => piece.Count < 2);

        return pieces;
    }

    /// <summary>
    /// Clips one segment at the rectangle, Liang–Barsky.
    /// </summary>
    /// <remarks>
    /// <c>lcl_clip2d</c> (<c>chart2/source/view/main/Clipping.cxx</c>:86-128) with its
    /// <c>lcl_CLIPt</c> helper (<c>:47-71</c>), described there as "Liang-Biarsky parametric
    /// line-clipping … Foley et al., section 3.12.4". A zero-length segment is visible exactly
    /// when its point is inside, which is the degenerate case the reference tests first.
    /// </remarks>
    private static bool ClipSegment(
        DocPoint a, DocPoint b, DocRect rect, out DocPoint from, out DocPoint to)
    {
        from = a;
        to = b;

        double dx = (double)b.X.Emu - a.X.Emu;
        double dy = (double)b.Y.Emu - a.Y.Emu;

        if (dx == 0.0 && dy == 0.0) return Contains(rect, a);

        double enter = 0.0;
        double leave = 1.0;

        if (!Cut(dx, (double)rect.Left.Emu - a.X.Emu, ref enter, ref leave)) return false;
        if (!Cut(-dx, (double)a.X.Emu - rect.Right.Emu, ref enter, ref leave)) return false;
        if (!Cut(dy, (double)rect.Top.Emu - a.Y.Emu, ref enter, ref leave)) return false;
        if (!Cut(-dy, (double)a.Y.Emu - rect.Bottom.Emu, ref enter, ref leave)) return false;

        if (leave < 1.0)
        {
            to = new DocPoint(
                Length.FromEmu(a.X.Emu + (long)Math.Round(leave * dx)),
                Length.FromEmu(a.Y.Emu + (long)Math.Round(leave * dy)));
        }

        if (enter > 0.0)
        {
            from = new DocPoint(
                Length.FromEmu(a.X.Emu + (long)Math.Round(enter * dx)),
                Length.FromEmu(a.Y.Emu + (long)Math.Round(enter * dy)));
        }

        return true;
    }

    /// <summary>
    /// Narrows the segment's visible parameter range against one of the rectangle's four edges.
    /// </summary>
    /// <remarks><c>lcl_CLIPt</c>, <c>Clipping.cxx</c>:47-71.</remarks>
    private static bool Cut(double denominator, double numerator, ref double enter, ref double leave)
    {
        if (denominator > 0.0)
        {
            double t = numerator / denominator;
            if (t > leave) return false;
            if (t > enter) enter = t;
        }
        else if (denominator < 0.0)
        {
            double t = numerator / denominator;
            if (t < enter) return false;
            if (t < leave) leave = t;
        }
        else if (numerator > 0.0)
        {
            // The segment runs along the edge's normal's zero and lies outside it.
            return false;
        }

        return true;
    }
}
