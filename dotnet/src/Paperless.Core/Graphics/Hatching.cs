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
    /// Each segment spans the box's whole extent along the lines so the caller can clip rather
    /// than intersect, and the family is placed exactly where <c>GeoTexSvxHatch</c> places it
    /// rather than merely at the right spacing — see <see cref="Family"/> for why the phase is
    /// worth reproducing and what it costs to get wrong.
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

    /// <summary>The most lines one family draws across the box it fills.</summary>
    /// <remarks>
    /// A hatch finer than this is a solid fill in the line's colour, and drawing it line by line
    /// would cost millions of segments to say so. The distance is widened to fit instead of the
    /// count being truncated, so the family still covers the whole box rather than a stripe
    /// through its middle. Nothing in the corpus comes near it: the finest preset in
    /// <c>hatchmap.hxx</c> is 0.25 mm, which is 500 lines across a slide. LibreOffice's own
    /// guard is the same shape and sits at 10000 (<c>texture.cxx:905</c>), on the branch where
    /// the output range differs from the definition range.
    /// </remarks>
    private const int MaxLinesEachWay = 1000;

    /// <summary>One family of parallel lines at an angle, covering a box.</summary>
    /// <remarks>
    /// <para>
    /// <b>The phase is <c>GeoTexSvxHatch</c>'s and not the obvious one.</b> Centring the family
    /// on the box gives the right count and the right spacing and puts every line up to half a
    /// step away from where LibreOffice puts it — which is invisible at full size and not
    /// invisible to a comparison that renders both at 512 pixels, where a hatch is near its
    /// sampling limit and a phase difference reads as a quarter of the page being redrawn.
    /// Measured on <c>BMFE-06-03 (Gerflor) Smoke Density and Toxicity.pptx</c> page 3, whose
    /// hatched column matches the reference line for line — 49 crossings to its 49-50 on every
    /// scanned row — and still scored 3.28 of unaccounted ink centred against 0.00 in phase.
    /// </para>
    /// <para>
    /// The construction is <c>drawinglayer/source/texture/texture.cxx:820-860</c>: the box is
    /// grown to the size its rotation needs, keeping its centre; the number of steps is
    /// <c>fround(H / distance + 0.5)</c>; and a line is emitted at <c>a × distance</c> from the
    /// grown box's leading edge for <c>a</c> from 1 to steps − 1. So the first line sits one
    /// whole distance in from the edge, the last one short of the far edge, and the family is
    /// symmetric only when the box happens to be a whole number of steps across.
    /// </para>
    /// </remarks>
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

        // The box's extent along each of the hatch's own axes: across the lines, which is what
        // the family has to span, and along them, which is how long a segment has to be. Both
        // are the box projected onto that axis, and the second bounds every chord because a
        // centred box projects entirely into it.
        double across = (Math.Abs(nx) * width) + (Math.Abs(ny) * height);
        double along = ((Math.Abs(dx) * width) + (Math.Abs(dy) * height)) / 2;

        double step = Math.Max(distance.Emu, across / (2 * MaxLinesEachWay));
        int steps = (int)Math.Floor((across / step) + 1);

        for (int a = 1; a < steps; a++)
        {
            double offset = (a * step) - (across / 2);
            double px = centreX + (nx * offset);
            double py = centreY + (ny * offset);

            yield return (
                new DocPoint(Length.FromEmu((long)(px - (dx * along))), Length.FromEmu((long)(py - (dy * along)))),
                new DocPoint(Length.FromEmu((long)(px + (dx * along))), Length.FromEmu((long)(py + (dy * along)))));
        }
    }
}
