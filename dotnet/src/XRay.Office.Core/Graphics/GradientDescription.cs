using XRay.Office.Core.Geometry;

namespace XRay.Office.Core.Graphics;

/// <summary>
/// A gradient before it knows where it is drawn: everything a file states about the ramp, with
/// the box left out.
/// </summary>
/// <remarks>
/// <para>
/// A <see cref="GradientPaint"/> holds absolute points, so it cannot be built until the box is
/// known — and a reader routinely knows a shape's fill long before the layout engine has decided
/// where the shape lands. That gap is what this closes: a reader resolves the colours and the
/// direction once, the drawing code supplies the rectangle, and neither has to know the other's
/// vocabulary.
/// </para>
/// <para>
/// Everything here is either a colour or a fraction, so the same description draws the same
/// gradient in any box. <see cref="CentreX"/> and <see cref="CentreY"/> are fractions of the box
/// rather than lengths for exactly that reason, and <see cref="AngleDegrees"/> is measured the
/// way a screen is: clockwise from the positive x axis, with y pointing down the page.
/// </para>
/// </remarks>
/// <param name="Kind">Which geometry the ramp is laid out in.</param>
/// <param name="Stops">The stops, stop 0 first — which for a centred gradient is the centre.</param>
public sealed record GradientDescription(GradientKind Kind, IReadOnlyList<GradientStop> Stops)
{
    /// <summary>
    /// The ramp's direction for a linear gradient, clockwise from the positive x axis.
    /// </summary>
    /// <remarks>Ignored by every other kind, which take their direction from the centre.</remarks>
    public double AngleDegrees { get; init; }

    /// <summary>Where stop 0 sits horizontally, as a fraction of the box from its left edge.</summary>
    /// <remarks>Ignored by <see cref="GradientKind.Linear"/>, whose ramp runs through the centre.</remarks>
    public double CentreX { get; init; } = 0.5;

    /// <summary>Where stop 0 sits vertically, as a fraction of the box from its top edge.</summary>
    public double CentreY { get; init; } = 0.5;

    /// <summary>
    /// The paint this draws over a box, in that box's own coordinate space.
    /// </summary>
    /// <remarks>
    /// The transform is the identity except where the geometry itself needs one — an elliptical
    /// gradient carries its aspect ratio there. A caller drawing into a rotated space substitutes
    /// its own.
    /// </remarks>
    /// <param name="box">The box being filled.</param>
    public GradientPaint Paint(DocRect box)
    {
        if (Kind == GradientKind.Linear)
        {
            double radians = AngleDegrees * Math.PI / 180.0;
            return GradientGeometry.Linear(box, Math.Cos(radians), Math.Sin(radians), Stops);
        }

        DocPoint centre = new(
            box.Left + (box.Width * CentreX),
            box.Top + (box.Height * CentreY));

        return GradientGeometry.Centred(Kind, box, centre, Stops);
    }
}
