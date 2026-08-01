using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Presentations.Layout;

/// <summary>
/// Turns a gradient's stated parameters into the geometry a <see cref="GradientPaint"/> carries.
/// </summary>
/// <remarks>
/// <para>
/// Shared by the PPTX and ODP readers because the arithmetic is LibreOffice's rather than either
/// format's: both importers converge on <c>basegfx::BGradient</c>, and everything that decides
/// where a gradient's ends land happens after that, in
/// <c>basegfx/source/tools/gradienttools.cxx</c>. Porting it once means a deck and its ODF
/// export draw the same picture, which is the only way the two can be compared at all.
/// </para>
/// <para>
/// <b>The convention this produces is the backends'</b>, and it is not either format's:
/// <see cref="GradientPaint.Start"/> holds stop 0, which for a centred gradient is the
/// <em>centre</em>. ODF says the opposite — its <c>draw:start-color</c> paints the outer edge —
/// so the ODF reader swaps its ends before calling in here. See the TODO.
/// </para>
/// </remarks>
internal static class SlideGradients
{
    /// <summary>
    /// A linear ramp across a box, along a direction.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The axis runs through the centre of the box and is long enough to span the box's
    /// <em>rotated</em> extent — <c>w·|dx| + h·|dy|</c>, which is what
    /// <c>init1DGradientInfo</c> computes as <c>fNewY</c>
    /// (<c>gradienttools.cxx:75-81</c>) before it scales the unit square by it. Using the
    /// box's own height instead leaves a diagonal gradient's corners flat.
    /// </para>
    /// <para>
    /// Direction is a vector rather than an angle deliberately. The two formats measure their
    /// angles from different axes and in opposite senses, and a shared helper taking "an angle"
    /// is how one of them silently gets the other's convention.
    /// </para>
    /// </remarks>
    /// <param name="box">The box being filled, in the coordinate space the paint will be used in.</param>
    /// <param name="directionX">The ramp's horizontal component; need not be normalised.</param>
    /// <param name="directionY">Its vertical component, positive pointing down the page.</param>
    /// <param name="stops">The stops, stop 0 first.</param>
    public static GradientPaint Linear(
        DocRect box, double directionX, double directionY, IReadOnlyList<GradientStop> stops)
    {
        double length = Math.Sqrt((directionX * directionX) + (directionY * directionY));
        (double dx, double dy) = length <= 0 ? (0.0, 1.0) : (directionX / length, directionY / length);

        double span = (box.Width.Emu * Math.Abs(dx)) + (box.Height.Emu * Math.Abs(dy));
        DocPoint centre = Centre(box);

        return new GradientPaint(
            GradientKind.Linear,
            stops,
            Shift(centre, -dx * span / 2, -dy * span / 2),
            Shift(centre, dx * span / 2, dy * span / 2),
            AffineTransform.Identity);
    }

    /// <summary>
    /// A gradient that spreads out from a point: radial, elliptical or rectangular.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <b>The outer radius is half the box's diagonal, not half its width.</b>
    /// <c>initEllipticalGradientInfo</c> replaces the box with a square of side
    /// <c>hypot(w, h)</c> for a circular gradient (<c>gradienttools.cxx:145-153</c>) and scales
    /// each axis by √2 for an elliptical one, and the gradient reaches its last stop at the
    /// edge of <em>that</em>. Measured: half the width instead moves a page's mean absolute
    /// error against LibreOffice's own rendering from 0.0016 to 0.0054.
    /// </para>
    /// <para>
    /// An elliptical gradient is a radial one with the ellipse's aspect ratio in
    /// <see cref="GradientPaint.Transform"/>, which is all either backend needs — a shading
    /// pattern's <c>/Matrix</c> and a Skia shader's local matrix both express it, and neither
    /// has a native ellipse.
    /// </para>
    /// </remarks>
    /// <param name="kind">Which geometry.</param>
    /// <param name="box">The box being filled.</param>
    /// <param name="centre">The centre, in the same space as <paramref name="box"/>.</param>
    /// <param name="stops">The stops, the centre's colour first.</param>
    public static GradientPaint Centred(
        GradientKind kind, DocRect box, DocPoint centre, IReadOnlyList<GradientStop> stops)
    {
        double width = box.Width.Emu;
        double height = box.Height.Emu;

        return kind switch
        {
            GradientKind.Radial => new GradientPaint(
                kind, stops, centre,
                Shift(centre, Math.Sqrt((width * width) + (height * height)) / 2, 0),
                AffineTransform.Identity),

            // Squashed about its own centre, so the transform has to move the centre back:
            // scaling in place would drag every ellipse towards the origin of the slide.
            GradientKind.Elliptical => new GradientPaint(
                kind, stops, centre, Shift(centre, Math.Sqrt(2) * width / 2, 0),
                Squash(centre, height <= 0 || width <= 0 ? 1 : height / width)),

            _ => new GradientPaint(kind, stops, centre, Shift(centre, width / 2, 0),
                                   AffineTransform.Identity),
        };
    }

    /// <summary>
    /// The stops of an axial gradient — one colour at both ends, another in the middle — laid
    /// out on the linear ramp that draws it.
    /// </summary>
    /// <remarks>
    /// ODF's <c>axial</c> style is a linear gradient measured from the centre outwards:
    /// <c>init1DGradientInfo</c> halves the unit square and shifts it to 0.5, and
    /// <c>getAxialGradientAlpha</c> takes the absolute value
    /// (<c>gradienttools.cxx:86-90, 613-631</c>). So the ramp is symmetric about the middle,
    /// and stating it as three stops on an ordinary linear gradient is exact rather than an
    /// approximation.
    /// </remarks>
    /// <param name="middle">The colour at the centre — ODF's <c>draw:start-color</c>.</param>
    /// <param name="ends">The colour at both ends — ODF's <c>draw:end-color</c>.</param>
    /// <param name="border">
    /// <c>draw:border</c>: the fraction of the ramp held at the outer colour, split between the
    /// two ends because an axial gradient has two of them.
    /// </param>
    public static IReadOnlyList<GradientStop> Axial(Colour middle, Colour ends, double border = 0)
    {
        if (!(border > 0) || border >= 1)
        {
            return [new GradientStop(0, ends), new GradientStop(0.5, middle), new GradientStop(1, ends)];
        }

        double reach = (1 - border) / 2;
        return
        [
            new GradientStop(0, ends),
            new GradientStop(0.5 - reach, ends),
            new GradientStop(0.5, middle),
            new GradientStop(0.5 + reach, ends),
            new GradientStop(1, ends),
        ];
    }

    /// <summary>
    /// The stops with a border applied: a fraction of the ramp held at the colour of one of its
    /// ends.
    /// </summary>
    /// <remarks>
    /// <c>draw:border</c> shortens the ramp rather than shifting it —
    /// <c>init1DGradientInfo</c> scales the unit square by <c>1 - border</c> and translates it
    /// by the border (<c>gradienttools.cxx:93-97</c>), and the clamp in
    /// <c>getLinearGradientAlpha</c> holds everything before it at the first stop. Remapping the
    /// offsets says the same thing without needing a backend to know what a border is.
    /// </remarks>
    /// <param name="stops">The stops, already in ramp order.</param>
    /// <param name="border">The fraction of the ramp to hold, from 0 to 1.</param>
    /// <param name="atEnd">
    /// Which end holds. False for a linear ODF gradient, whose border sits at the start; true for
    /// a centred one, where the ODF start colour has already been swapped to the far end of the
    /// ramp and the border goes with it.
    /// </param>
    public static IReadOnlyList<GradientStop> WithBorder(
        IReadOnlyList<GradientStop> stops, double border, bool atEnd = false)
    {
        ArgumentNullException.ThrowIfNull(stops);

        if (!(border > 0) || border >= 1 || stops.Count == 0) return stops;

        List<GradientStop> bordered = [];

        if (atEnd)
        {
            foreach (GradientStop stop in stops)
                bordered.Add(stop with { Offset = stop.Offset * (1 - border) });

            bordered.Add(new GradientStop(1, stops[^1].Colour));
            return bordered;
        }

        bordered.Add(new GradientStop(0, stops[0].Colour));
        foreach (GradientStop stop in stops)
            bordered.Add(stop with { Offset = border + (stop.Offset * (1 - border)) });

        return bordered;
    }

    /// <summary>The centre of a box.</summary>
    public static DocPoint Centre(DocRect box)
        => new(box.Left + (box.Width / 2), box.Top + (box.Height / 2));

    private static DocPoint Shift(DocPoint point, double dx, double dy)
        => new(Length.FromEmu(point.X.Emu + (long)Math.Round(dx)),
               Length.FromEmu(point.Y.Emu + (long)Math.Round(dy)));

    /// <summary>A scale of the y axis about a fixed point.</summary>
    private static AffineTransform Squash(DocPoint about, double factor)
        => new(1, 0, 0, factor, 0, about.Y.Emu * (1 - factor));
}
