using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;

namespace XRay.Office.Core.Tests;

/// <summary>
/// Reads a series mark's rectangle back out of the path it is now drawn as.
/// </summary>
/// <remarks>
/// <para>
/// Bars, candles and up-down bars used to be emitted as <c>ChartBox</c> and are now
/// <c>ChartShape</c>, because a chart's Z order requires it: every consumer paints
/// <c>Boxes</c> before <c>Lines</c>, so a bar left in <c>Boxes</c> is drawn *under* the
/// gridlines and each major tick rules a light-grey line across it — the "bars filled with
/// horizontal stripes" this repository's parity sweep found on three unrelated workbooks.
/// </para>
/// <para>
/// The tests that assert where a bar sits therefore have to measure a path rather than read a
/// <c>Bounds</c>, and they should not each re-derive how. A rectangular path's extent is just
/// the extent of its points; nothing here is drawn with a curve, so the control points of a
/// <c>CubicTo</c> are deliberately included too — for a shape that had one, ignoring them
/// would understate the extent rather than overstate it, and an understated bar is the
/// direction that makes a "the label sits above the bar" assertion pass when it should not.
/// </para>
/// </remarks>
internal static class ChartSeriesMarks
{
    /// <summary>The rectangle a shape's path spans.</summary>
    internal static DocRect Bounds(this ChartShape shape)
    {
        bool any = false;
        Length left = default, top = default, right = default, bottom = default;

        foreach (PathCommand command in shape.Path.Commands)
        {
            foreach (DocPoint point in Points(command))
            {
                if (!any)
                {
                    left = right = point.X;
                    top = bottom = point.Y;
                    any = true;
                    continue;
                }

                left = Length.Min(left, point.X);
                right = Length.Max(right, point.X);
                top = Length.Min(top, point.Y);
                bottom = Length.Max(bottom, point.Y);
            }
        }

        return new DocRect(left, top, right - left, bottom - top);
    }

    /// <summary>The filled series marks, in paint order.</summary>
    internal static List<ChartShape> Filled(this ChartDrawing drawing) =>
        [.. drawing.Shapes.Where(shape => shape.Fill is not null)];

    private static IEnumerable<DocPoint> Points(PathCommand command)
    {
        // `Close` carries no coordinate -- `GraphicsPath.Close` stores `default` for all three
        // points -- so counting it would drag every rectangle's extent back to the origin. That
        // is not hypothetical: it is what the first cut of this helper did, and it turned "the
        // bar starts inside the plot area" into "the bar starts at 0pt".
        if (command.Verb == PathVerb.Close) yield break;

        yield return command.Point;

        if (command.Verb != PathVerb.CubicTo) yield break;

        yield return command.Control1;
        yield return command.Control2;
    }
}
