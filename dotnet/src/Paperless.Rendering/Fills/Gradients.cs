using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Rendering.Fills;

/// <summary>
/// The arithmetic both backends share when they draw a <see cref="GradientPaint"/>.
/// </summary>
/// <remarks>
/// <para>
/// Two of the five geometries have a native form in both PDF and Skia — a linear gradient is
/// an axial shading and a linear shader, a radial one is a radial shading and a radial
/// shader — and those go straight through. The other two, <see cref="GradientKind.Conical"/>
/// and <see cref="GradientKind.Rectangular"/>, have a native form in neither, so both
/// backends decompose them into a stack of flat-coloured bands through
/// <see cref="DrawBands"/>. The decomposition lives here rather than in either backend
/// precisely so the two draw the same picture: a shape's colours must not depend on whether
/// the page was written as a PDF or rasterised.
/// </para>
/// <para>
/// That is also what LibreOffice does, and the band arithmetic is ported from it.
/// <c>Gradient::AddGradientActions</c> (<c>vcl/source/gdi/gradient.cxx:304</c>) turns a
/// gradient into a metafile of flat polygons, and
/// <c>Gradient::GetMetafileSteps</c> (<c>:336</c>) is where the number of them comes from.
/// </para>
/// <para>
/// <b>Stop 0 is at <see cref="GradientPaint.Start"/>.</b> For a radial, elliptical,
/// rectangular or conical gradient that is the <em>centre</em>, which is worth stating
/// because the office formats disagree with it: ODF's <c>draw:start-color</c> on a
/// <c>radial</c> gradient paints the outer edge and its <c>draw:end-color</c> the middle —
/// measured on a reference rendering of a <c>#00c0c0</c>-to-<c>#101010</c> radial, whose
/// centre comes out black. Putting the swap in the readers rather than in the backends keeps
/// one convention here and one place per format where the format's own is stated.
/// </para>
/// </remarks>
internal static class Gradients
{
    /// <summary>
    /// The most bands a decomposition will draw, whatever the geometry asks for.
    /// </summary>
    /// <remarks>
    /// An eight-bit channel cannot show more than 256 distinct values, so beyond this the
    /// bands are drawing each other's colours. LibreOffice reaches the same ceiling from the
    /// other direction, clamping its step count to the largest channel difference
    /// (<c>gradient.cxx:456-458</c>).
    /// </remarks>
    private const int MaximumSteps = 256;

    /// <summary>Whether a kind has a form both backends can state directly.</summary>
    /// <remarks>
    /// Elliptical counts: it is a radial gradient whose <see cref="GradientPaint.Transform"/>
    /// squashes one axis, which both a shading pattern's <c>/Matrix</c> and a Skia shader's
    /// local matrix express without any further work.
    /// </remarks>
    public static bool HasNativeForm(GradientKind kind)
        => kind is GradientKind.Linear or GradientKind.Radial or GradientKind.Elliptical;

    /// <summary>
    /// The stops, sorted, made strictly increasing, and spanning the whole of 0 to 1.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Every consumer needs the same three guarantees and none of them can be assumed of a
    /// list that came out of a document: PDF's stitching function needs strictly increasing
    /// bounds, Skia needs its offsets non-decreasing, and both need the ramp to reach the ends
    /// of the range or the fill stops short of the shape.
    /// </para>
    /// <para>
    /// A stop that repeats its predecessor's offset is nudged by one part in a million rather
    /// than dropped, because a repeated offset is how both formats spell a hard colour
    /// change and dropping one would turn a two-tone flag into a ramp.
    /// </para>
    /// </remarks>
    public static IReadOnlyList<GradientStop> Normalise(IReadOnlyList<GradientStop> stops)
    {
        ArgumentNullException.ThrowIfNull(stops);

        List<GradientStop> sorted = [.. stops.Select(
            stop => stop with { Offset = double.IsFinite(stop.Offset) ? Math.Clamp(stop.Offset, 0, 1) : 0 })];

        sorted.Sort((a, b) => a.Offset.CompareTo(b.Offset));

        if (sorted.Count == 0) return [new GradientStop(0, Colour.Transparent), new GradientStop(1, Colour.Transparent)];
        if (sorted.Count == 1) return [sorted[0] with { Offset = 0 }, sorted[0] with { Offset = 1 }];

        if (sorted[0].Offset > 0) sorted.Insert(0, sorted[0] with { Offset = 0 });
        if (sorted[^1].Offset < 1) sorted.Add(sorted[^1] with { Offset = 1 });

        const double Nudge = 1e-6;
        for (int i = 1; i < sorted.Count; i++)
        {
            if (sorted[i].Offset > sorted[i - 1].Offset) continue;

            sorted[i] = sorted[i] with { Offset = Math.Min(1, sorted[i - 1].Offset + Nudge) };
        }

        // The nudge can only have pushed offsets up, so the last one may now sit past the end.
        if (sorted[^1].Offset < 1) sorted[^1] = sorted[^1] with { Offset = 1 };

        return sorted;
    }

    /// <summary>The colour a normalised stop list takes at a position from 0 to 1.</summary>
    public static Colour Sample(IReadOnlyList<GradientStop> stops, double t)
    {
        ArgumentNullException.ThrowIfNull(stops);
        if (stops.Count == 0) return Colour.Transparent;

        if (t <= stops[0].Offset) return stops[0].Colour;
        if (t >= stops[^1].Offset) return stops[^1].Colour;

        for (int i = 1; i < stops.Count; i++)
        {
            if (t > stops[i].Offset) continue;

            double span = stops[i].Offset - stops[i - 1].Offset;
            double f = span <= 0 ? 0 : (t - stops[i - 1].Offset) / span;
            return Mix(stops[i - 1].Colour, stops[i].Colour, f);
        }

        return stops[^1].Colour;
    }

    /// <summary>
    /// How many flat bands a decomposition takes.
    /// </summary>
    /// <remarks>
    /// LibreOffice's rule, from <c>Gradient::GetMetafileSteps</c>
    /// (<c>vcl/source/gdi/gradient.cxx:336-348</c>): the shorter side of the rectangle in the
    /// device's own logical units — 1/100 mm for a drawing — clamped by the largest difference
    /// any colour channel actually spans (<c>:450-458</c>), because a ramp from one grey to
    /// the next grey cannot use more than one band however large the shape is. At least two,
    /// or there is no ramp at all.
    /// </remarks>
    public static int Steps(DocRect bounds, IReadOnlyList<GradientStop> stops)
    {
        ArgumentNullException.ThrowIfNull(stops);

        double hundredthsOfMillimetre = Math.Min(bounds.Width.Emu, bounds.Height.Emu) / 360.0;
        int geometric = (int)Math.Min(hundredthsOfMillimetre, MaximumSteps);

        int channels = 0;
        for (int i = 1; i < stops.Count; i++)
        {
            channels = Math.Max(channels, Math.Abs(stops[i].Colour.R - stops[i - 1].Colour.R));
            channels = Math.Max(channels, Math.Abs(stops[i].Colour.G - stops[i - 1].Colour.G));
            channels = Math.Max(channels, Math.Abs(stops[i].Colour.B - stops[i - 1].Colour.B));
        }

        return Math.Max(2, Math.Min(geometric, channels));
    }

    /// <summary>
    /// Draws a gradient with no native form as a stack of flat-coloured bands.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Outermost first and inwards, which is the order the shapes have to be painted in for
    /// the later ones to cover the earlier: the bands are nested rather than adjacent, so each
    /// is drawn whole and then partly hidden. That is how LibreOffice's own decomposition
    /// works too, and it is why a decomposed gradient's content stream is large — 91602 bytes
    /// for the four rectangles of <c>tests/corpus/features/paint-fills.fodp</c> in
    /// LibreOffice's own PDF, against 3062 for ours.
    /// </para>
    /// <para>
    /// Everything is drawn inside the caller's path as a clip, so a band that overhangs the
    /// shape — which the outermost one always does, since it has to cover the corners — does
    /// not paint outside it.
    /// </para>
    /// </remarks>
    public static void DrawBands(IDrawingSink sink, GraphicsPath path, GradientPaint gradient, FillRule rule)
    {
        ArgumentNullException.ThrowIfNull(sink);
        ArgumentNullException.ThrowIfNull(path);
        ArgumentNullException.ThrowIfNull(gradient);

        if (Bounds(path) is not { } bounds) return;

        IReadOnlyList<GradientStop> stops = Normalise(gradient.Stops);
        DocRect extent = Untransformed(bounds, gradient.Transform);
        int steps = Steps(extent, stops);

        sink.Save();
        sink.ClipPath(path, rule);
        if (gradient.Transform != AffineTransform.Identity) sink.Transform(gradient.Transform);

        if (gradient.Kind == GradientKind.Conical) DrawWedges(sink, gradient, extent, stops, steps);
        else DrawRings(sink, gradient, extent, stops, steps);

        sink.Restore();
    }

    /// <summary>
    /// Nested rectangles shrinking towards the centre: OOXML's <c>path="rect"</c> and ODF's
    /// <c>square</c> and <c>rectangular</c>.
    /// </summary>
    private static void DrawRings(
        IDrawingSink sink,
        GradientPaint gradient,
        DocRect extent,
        IReadOnlyList<GradientStop> stops,
        int steps)
    {
        DocPoint centre = gradient.Start;

        for (int i = steps; i >= 0; i--)
        {
            double f = (double)i / steps;
            Colour colour = Sample(stops, f);

            // Each edge closes on the centre at its own rate, so an off-centre gradient stays
            // off-centre all the way in rather than drifting back to the middle of the shape.
            DocRect band = new(
                Lerp(extent.Left, centre.X, 1 - f),
                Lerp(extent.Top, centre.Y, 1 - f),
                default,
                default);

            band = new DocRect(
                band.X,
                band.Y,
                Lerp(extent.Right, centre.X, 1 - f) - band.X,
                Lerp(extent.Bottom, centre.Y, 1 - f) - band.Y);

            if (band.Width.Emu <= 0 || band.Height.Emu <= 0) continue;

            sink.FillPath(GraphicsPath.Rectangle(band), Paint.Solid(colour));
        }
    }

    /// <summary>
    /// Wedges fanned about the centre, for a gradient whose colour varies with angle.
    /// </summary>
    /// <remarks>
    /// Angles run clockwise from the positive x axis, which is what a document's y-down space
    /// makes of the usual anticlockwise convention, and each wedge is drawn a whole step wide
    /// so that neighbours overlap rather than leave an unpainted seam between them.
    /// </remarks>
    private static void DrawWedges(
        IDrawingSink sink,
        GradientPaint gradient,
        DocRect extent,
        IReadOnlyList<GradientStop> stops,
        int steps)
    {
        DocPoint centre = gradient.Start;
        double radius = Radius(centre, extent);
        if (radius <= 0) return;

        for (int i = 0; i < steps; i++)
        {
            double from = i * 2 * Math.PI / steps;
            double to = (i + 1) * 2 * Math.PI / steps;
            Colour colour = Sample(stops, (i + 0.5) / steps);

            GraphicsPath wedge = new GraphicsPath()
                .MoveTo(centre)
                .LineTo(On(centre, radius, from))
                .LineTo(On(centre, radius, (from + to) / 2))
                .LineTo(On(centre, radius, to))
                .Close();

            sink.FillPath(wedge, Paint.Solid(colour));
        }
    }

    /// <summary>
    /// How many whole periods of a repeating ramp lie before its start and after its end
    /// before the shape it fills is covered.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Only the PDF backend asks. Skia states <see cref="SpreadMethod"/> as a shader tile mode
    /// and repeats for nothing, whereas PDF's <c>sh</c> has no tiling at all: a shading's
    /// <c>/Extend</c> clamps the parameter and cannot repeat it ([PDF 32000-1] 8.7.4.5.3
    /// computes <c>t</c> from a parameter already clipped to 0..1). The only way to spell a
    /// repeat is to <em>lengthen the axis</em> over as many periods as the shape needs and
    /// give the function a domain to match, which is what this counts.
    /// </para>
    /// <para>
    /// Counted in gradient space, so the caller must pass the extent already carried back
    /// through <see cref="GradientPaint.Transform"/> — the shading is painted inside that
    /// transform, so a rotated ramp's period count is a question about the untransformed box.
    /// </para>
    /// </remarks>
    /// <param name="gradient">The gradient.</param>
    /// <param name="extent">The box to cover, in gradient space.</param>
    /// <returns>Periods needed before the ramp's start and after its end, each at least zero.</returns>
    public static (int Before, int After) Periods(GradientPaint gradient, DocRect extent)
    {
        ArgumentNullException.ThrowIfNull(gradient);

        if (gradient.Spread == SpreadMethod.Pad) return (0, 0);

        double x0 = gradient.Start.X.Emu, y0 = gradient.Start.Y.Emu;
        double x1 = gradient.End.X.Emu, y1 = gradient.End.Y.Emu;

        double lowest = 0, highest = 1;

        if (gradient.Kind == GradientKind.Linear)
        {
            double dx = x1 - x0, dy = y1 - y0;
            double squared = (dx * dx) + (dy * dy);
            if (squared <= 0) return (0, 0);

            foreach ((Length px, Length py) in Corners(extent))
            {
                double s = (((px.Emu - x0) * dx) + ((py.Emu - y0) * dy)) / squared;
                lowest = Math.Min(lowest, s);
                highest = Math.Max(highest, s);
            }
        }
        else
        {
            // A radial ramp starts at its own centre, so nothing lies before it however large
            // the shape is; only the outward direction can need more periods.
            double radius = Math.Sqrt(((x1 - x0) * (x1 - x0)) + ((y1 - y0) * (y1 - y0)));
            if (radius <= 0) return (0, 0);

            highest = Math.Max(1, Radius(gradient.Start, extent) / radius);
        }

        return (
            Math.Clamp((int)Math.Ceiling(-lowest), 0, MaximumPeriods),
            Math.Clamp((int)Math.Ceiling(highest - 1), 0, MaximumPeriods));
    }

    /// <summary>
    /// The most periods either side of a repeating ramp that will be written.
    /// </summary>
    /// <remarks>
    /// Each period is one more sub-function in the shading's stitching function, so an
    /// unbounded count is an unbounded PDF from a gradient whose ramp is a rounding error
    /// wide. Beyond this the remaining space is left to <c>/Extend</c>, which is the wrong
    /// colour but a bounded amount of it.
    /// </remarks>
    private const int MaximumPeriods = 64;

    private static IEnumerable<(Length X, Length Y)> Corners(DocRect bounds) =>
    [
        (bounds.Left, bounds.Top), (bounds.Right, bounds.Top),
        (bounds.Left, bounds.Bottom), (bounds.Right, bounds.Bottom),
    ];

    /// <summary>The furthest any corner of a rectangle lies from a point.</summary>
    public static double Radius(DocPoint centre, DocRect bounds)
    {
        double furthest = 0;

        foreach ((Length x, Length y) in (IEnumerable<(Length, Length)>)
                 [
                     (bounds.Left, bounds.Top), (bounds.Right, bounds.Top),
                     (bounds.Left, bounds.Bottom), (bounds.Right, bounds.Bottom),
                 ])
        {
            double dx = x.Emu - centre.X.Emu;
            double dy = y.Emu - centre.Y.Emu;
            furthest = Math.Max(furthest, Math.Sqrt((dx * dx) + (dy * dy)));
        }

        return furthest;
    }

    /// <summary>
    /// The bounding box of a path, or null when it has no points.
    /// </summary>
    /// <remarks>
    /// Control points are included, so a curved path's box can be larger than the ink. That is
    /// the safe direction for everything here: a gradient extent that is too large paints
    /// under the clip, one that is too small leaves a corner unpainted.
    /// </remarks>
    public static DocRect? Bounds(GraphicsPath path)
    {
        ArgumentNullException.ThrowIfNull(path);

        bool any = false;
        long left = 0, top = 0, right = 0, bottom = 0;

        foreach (PathCommand command in path.Commands)
        {
            if (command.Verb == PathVerb.Close) continue;

            Include(command.Point);
            if (command.Verb != PathVerb.CubicTo) continue;

            Include(command.Control1);
            Include(command.Control2);
        }

        return any ? new DocRect(
            Length.FromEmu(left), Length.FromEmu(top),
            Length.FromEmu(right - left), Length.FromEmu(bottom - top)) : null;

        void Include(DocPoint point)
        {
            if (!any)
            {
                left = right = point.X.Emu;
                top = bottom = point.Y.Emu;
                any = true;
                return;
            }

            left = Math.Min(left, point.X.Emu);
            right = Math.Max(right, point.X.Emu);
            top = Math.Min(top, point.Y.Emu);
            bottom = Math.Max(bottom, point.Y.Emu);
        }
    }

    /// <summary>
    /// A rectangle carried back through a gradient's transform, so bands sized in gradient
    /// space still cover the shape once the transform has been applied to them.
    /// </summary>
    /// <remarks>
    /// A singular transform — one that collapses an axis — has no inverse and nothing sensible
    /// to draw through it, so the rectangle comes back unchanged and the bands are clipped
    /// away by the caller's path.
    /// </remarks>
    public static DocRect Untransformed(DocRect bounds, AffineTransform transform)
    {
        double determinant = (transform.A * transform.D) - (transform.B * transform.C);
        if (Math.Abs(determinant) < 1e-12) return bounds;

        double a = transform.D / determinant;
        double b = -transform.B / determinant;
        double c = -transform.C / determinant;
        double d = transform.A / determinant;
        double e = ((transform.C * transform.F) - (transform.D * transform.E)) / determinant;
        double f = ((transform.B * transform.E) - (transform.A * transform.F)) / determinant;

        double left = double.MaxValue, top = double.MaxValue;
        double right = double.MinValue, bottom = double.MinValue;

        foreach ((Length px, Length py) in (IEnumerable<(Length, Length)>)
                 [
                     (bounds.Left, bounds.Top), (bounds.Right, bounds.Top),
                     (bounds.Left, bounds.Bottom), (bounds.Right, bounds.Bottom),
                 ])
        {
            double x = (a * px.Emu) + (c * py.Emu) + e;
            double y = (b * px.Emu) + (d * py.Emu) + f;

            left = Math.Min(left, x);
            right = Math.Max(right, x);
            top = Math.Min(top, y);
            bottom = Math.Max(bottom, y);
        }

        return new DocRect(
            Length.FromEmu((long)left), Length.FromEmu((long)top),
            Length.FromEmu((long)(right - left)), Length.FromEmu((long)(bottom - top)));
    }

    private static DocPoint On(DocPoint centre, double radius, double angle) => new(
        Length.FromEmu(centre.X.Emu + (long)(radius * Math.Cos(angle))),
        Length.FromEmu(centre.Y.Emu + (long)(radius * Math.Sin(angle))));

    private static Length Lerp(Length from, Length to, double f)
        => Length.FromEmu(from.Emu + (long)((to.Emu - from.Emu) * f));

    private static Colour Mix(Colour from, Colour to, double f) => new(
        (byte)Math.Clamp(Math.Round(from.R + ((to.R - from.R) * f)), 0, 255),
        (byte)Math.Clamp(Math.Round(from.G + ((to.G - from.G) * f)), 0, 255),
        (byte)Math.Clamp(Math.Round(from.B + ((to.B - from.B) * f)), 0, 255),
        (byte)Math.Clamp(Math.Round(from.A + ((to.A - from.A) * f)), 0, 255));
}
