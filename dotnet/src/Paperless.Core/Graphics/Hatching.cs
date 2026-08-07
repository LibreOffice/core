using Paperless.Core.Geometry;
using Paperless.Core.Units;

namespace Paperless.Core.Graphics;

/// <summary>
/// Expands a <see cref="HatchPaint"/> into the line segments that draw it.
/// </summary>
/// <remarks>
/// <para>
/// A hatch is the one <see cref="Paint"/> kind no backend implements, so somebody has to turn
/// it into ordinary drawing commands. This is that somebody, and it lives here rather than
/// inside whichever layout draws a shape because a second copy is exactly how the two
/// existing hatch expansions in this project came to disagree: <c>MetafilePainter</c> steps a
/// 45° diagonal by <c>spacing × √2</c> and this steps it by <c>spacing</c>, because GDI states
/// the spacing along the axis a diagonal crosses and LibreOffice states it perpendicular to
/// the line.
/// </para>
/// <para>
/// The caller clips to the shape and strokes each segment as a hairline, which is what
/// <c>FillHatchPrimitive2D</c> does — every line becomes a
/// <c>PolygonHairlinePrimitive2D</c>, never a widened polygon.
/// </para>
/// </remarks>
public static class Hatching
{
    /// <summary>
    /// The line segments of a hatch over a box, in document coordinates.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Each segment spans the box's whole diagonal extent so the caller can clip rather than
    /// intersect, and the families are centred on the box: LibreOffice grows the texture range
    /// to the rotated box's bounding size and offsets it by half the growth
    /// (<c>drawinglayer/source/texture/texture.cxx:826-839</c>), which centres it the same way.
    /// </para>
    /// <para>
    /// Returns nothing for an empty box or a non-positive distance, rather than looping for
    /// ever. A hatch whose distance rounds to zero is a solid fill in the line's colour and is
    /// not worth the millions of segments it would take to say so.
    /// </para>
    /// </remarks>
    /// <param name="bounds">The box to cover.</param>
    /// <param name="hatch">The hatch.</param>
    public static IEnumerable<(DocPoint From, DocPoint To)> Lines(DocRect bounds, HatchPaint hatch)
    {
        ArgumentNullException.ThrowIfNull(hatch);

        if (bounds.IsEmpty || hatch.Distance <= Length.Zero) yield break;

        // Same order as the primitive's fall-through: the extra families first, the stated
        // angle last, so a caller drawing them in sequence lays the same lines on top.
        double[] angles = hatch.Kind switch
        {
            HatchKind.ThreeWay => [hatch.Angle - (Math.PI / 4), hatch.Angle - (Math.PI / 2), hatch.Angle],
            HatchKind.TwoWay => [hatch.Angle - (Math.PI / 2), hatch.Angle],
            _ => [hatch.Angle],
        };

        foreach (double angle in angles)
        {
            foreach ((DocPoint from, DocPoint to) in Family(bounds, hatch.Distance, angle))
            {
                yield return (from, to);
            }
        }
    }

    /// <summary>
    /// The most lines one family draws to either side of the centre.
    /// </summary>
    /// <remarks>
    /// A hatch finer than this over the box it fills is a solid fill in the line's colour, and
    /// drawing it line by line would cost millions of segments to say so. The distance is
    /// widened to fit instead of the count being truncated, so the family still covers the
    /// whole box rather than a stripe through its middle. Nothing in the corpus comes near it:
    /// the finest preset in <c>hatchmap.hxx</c> is 0.25 mm, which is 500 lines across a slide.
    /// </remarks>
    private const int MaxLinesEachWay = 1000;

    /// <summary>One family of parallel lines at an angle, covering a box.</summary>
    private static IEnumerable<(DocPoint From, DocPoint To)> Family(
        DocRect bounds, Length distance, double angle)
    {
        // The document's y runs downwards and the angle is stated anticlockwise, so a line's
        // direction is (cos, -sin) and the normal it steps along is (sin, cos).
        double dx = Math.Cos(angle);
        double dy = -Math.Sin(angle);
        double nx = -dy;
        double ny = dx;

        double width = bounds.Width.Emu;
        double height = bounds.Height.Emu;
        double centreX = bounds.Left.Emu + (width / 2);
        double centreY = bounds.Top.Emu + (height / 2);

        // How far the box reaches along the normal. The segment length is the box's diagonal,
        // which bounds every chord from every offset — the caller clips, so an overlong
        // segment costs nothing and a short one leaves a corner unhatched.
        double reach = ((Math.Abs(nx) * width) + (Math.Abs(ny) * height)) / 2;
        double half = Math.Sqrt((width * width) + (height * height)) / 2;

        double step = Math.Max(distance.Emu, reach / MaxLinesEachWay);
        int count = (int)Math.Floor(reach / step);

        for (int i = -count; i <= count; i++)
        {
            double px = centreX + (nx * step * i);
            double py = centreY + (ny * step * i);

            yield return (
                new DocPoint(Length.FromEmu((long)(px - (dx * half))), Length.FromEmu((long)(py - (dy * half)))),
                new DocPoint(Length.FromEmu((long)(px + (dx * half))), Length.FromEmu((long)(py + (dy * half)))));
        }
    }
}
