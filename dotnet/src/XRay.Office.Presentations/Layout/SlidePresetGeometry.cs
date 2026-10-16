using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Ooxml.DrawingML;

namespace XRay.Office.Presentations.Layout;

/// <summary>
/// Expands a shape's preset geometry into an outline and a text rectangle.
/// </summary>
/// <remarks>
/// <para>
/// All 187 of DrawingML's presets, evaluated rather than transcribed. Each is a small program —
/// guide formulas over the bounding box and the adjustment handles, then a path built from the
/// results — and <see cref="CustomShapeGeometry"/> runs it against the shape's own size. Six of
/// them used to be transcribed by hand and everything else drew its bounding rectangle; what that
/// cost was not the 181 missing outlines so much as the impossibility of ever finishing, since
/// every deck in the world uses a different handful.
/// </para>
/// <para>
/// A preset name this does not know still falls back to the bounding rectangle: in the right
/// place, in the right colour, with the wrong outline. That is a far better failure than drawing
/// nothing, because it is <em>visible</em> in a comparison rather than silently absent.
/// </para>
/// </remarks>
public static class SlidePresetGeometry
{
    /// <summary>True when the preset is one this expands rather than approximates.</summary>
    public static bool IsKnown(string? preset) => PresetShapeGeometry.Find(preset) is not null;

    /// <summary>
    /// The outline of a preset shape, in the shape's own coordinates — origin at its top left.
    /// </summary>
    /// <param name="preset">The <c>a:prstGeom/@prst</c> value, or null for a plain box.</param>
    /// <param name="size">The shape's extent.</param>
    /// <param name="adjustments">
    /// The <c>a:avLst</c> values the shape states, by name, overriding the preset's defaults.
    /// </param>
    public static GraphicsPath Outline(
        string? preset, DocSize size, IReadOnlyDictionary<string, double>? adjustments = null)
        => CustomShapeGeometry.Preset(preset, size, adjustments) is { } geometry
            ? geometry.Outline
            : Rectangle(size);

    /// <summary>
    /// The rectangle text is laid out in, in the shape's own coordinates.
    /// </summary>
    /// <remarks>
    /// Not always the bounding box, and the presets say so themselves: an ellipse's
    /// <c>a:rect</c> is the box inscribed at 45°, a rounded rectangle's is inset by the corner
    /// radius, and a callout's excludes its tail. That is why a caption inside a circle does not
    /// touch its edge. An unknown preset gets the whole box, which is what LibreOffice falls back
    /// to as well.
    /// </remarks>
    /// <param name="preset">The <c>a:prstGeom/@prst</c> value, or null.</param>
    /// <param name="size">The shape's extent.</param>
    /// <param name="adjustments">The stated adjustment values, by name.</param>
    public static DocRect TextRectangle(
        string? preset, DocSize size, IReadOnlyDictionary<string, double>? adjustments = null)
        => CustomShapeGeometry.Preset(preset, size, adjustments) is { } geometry
            ? geometry.TextRectangle
            : new DocRect(Length.Zero, Length.Zero, size.Width, size.Height);

    /// <summary>
    /// The whole geometry of a preset — subpaths included — or the bounding rectangle.
    /// </summary>
    /// <remarks>
    /// <see cref="Outline"/> answers with one path and loses what each subpath said about itself,
    /// which is the half a painter needs: an <c>a:path</c> states its own <c>fill</c> and
    /// <c>stroke</c>, and 69 of the 187 presets carry at least one that is not the default.
    /// </remarks>
    /// <param name="preset">The <c>a:prstGeom/@prst</c> value, or null for a plain box.</param>
    /// <param name="size">The shape's extent.</param>
    /// <param name="adjustments">The stated adjustment values, by name.</param>
    public static CustomShapeGeometry.Geometry Of(
        string? preset, DocSize size, IReadOnlyDictionary<string, double>? adjustments = null)
        => CustomShapeGeometry.Preset(preset, size, adjustments)
            ?? new CustomShapeGeometry.Geometry(
                Rectangle(size), new DocRect(Length.Zero, Length.Zero, size.Width, size.Height));

    /// <summary>
    /// The parts of a placed geometry that are filled and stroked.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A subpath states whether it is filled and whether it is stroked, and painting the
    /// whole outline both ways ignores both.</strong> Every connector preset is one open subpath
    /// declaring <c>fill="none"</c> — filling it draws a solid blob across the chord of the bend —
    /// and the shading faces of <c>cube</c>, <c>can</c>, <c>bevel</c> and <c>foldedCorner</c> are
    /// filled and not stroked, so stroking them rules a solid into a wireframe. LibreOffice splits
    /// the same way: <c>Path2D::getFillMode</c> feeds <c>EnhancedCustomShape2d::CreateSubPath</c>,
    /// which pushes a <c>NONE</c> subpath into the stroke-only list
    /// (<c>svx/source/customshapes/EnhancedCustomShape2d.cxx</c>).
    /// </para>
    /// <para>
    /// The whole outline is returned unchanged — the same instance — whenever every subpath agrees,
    /// which is the common case and every shape whose reader reports no subpaths at all. So the
    /// split costs a pass over a short list and nothing else.
    /// </para>
    /// </remarks>
    /// <param name="geometry">The geometry, in the shape's own coordinates.</param>
    /// <param name="placement">The matrix taking it onto the slide.</param>
    /// <param name="placed">That geometry's whole outline, already placed.</param>
    public static PaintedGeometry Painted(
        CustomShapeGeometry.Geometry geometry, AffineTransform placement, GraphicsPath placed)
    {
        if (geometry.Subpaths is not { Count: > 0 } subpaths) return new PaintedGeometry(placed, placed);

        bool everyPartPlain = true;
        bool everyPartStroked = true;

        foreach (PresetSubpath subpath in subpaths)
        {
            everyPartPlain &= subpath.Fill == PresetPathFill.Normal;
            everyPartStroked &= subpath.Stroke;
        }

        GraphicsPath stroke = everyPartStroked
            ? placed
            : ShapeTransform.Apply(placement, geometry.StrokeOutline);

        if (everyPartPlain) return new PaintedGeometry(placed, stroke);

        GraphicsPath plain = new();
        List<SlideShadedPart> shaded = [];

        foreach (PresetSubpath subpath in subpaths)
        {
            if (subpath.Fill == PresetPathFill.None) continue;

            if (subpath.Fill == PresetPathFill.Normal)
            {
                Append(plain, ShapeTransform.Apply(placement, subpath.Outline));
                continue;
            }

            shaded.Add(new SlideShadedPart(
                ShapeTransform.Apply(placement, subpath.Outline), Brightness(subpath.Fill)));
        }

        return new PaintedGeometry(plain, stroke, shaded);
    }

    /// <summary>
    /// How far towards white or black a subpath's own fill mode takes the shape's fill.
    /// </summary>
    /// <remarks>
    /// The four magnitudes are LibreOffice's, from
    /// <c>EnhancedCustomShape2d::CreateSubPath</c> (<c>EnhancedCustomShape2d.cxx</c>:2112-2121):
    /// <c>darken</c> is <c>-0.4</c>, <c>darkenLess</c> <c>-0.2</c>, <c>lighten</c> <c>+0.4</c> and
    /// <c>lightenLess</c> <c>+0.2</c>. Confirmed against 26.2.4.2's own PDF on a probe deck whose
    /// shapes are all <c>4472C4</c>: it draws the cube's two faces <c>365B9C</c> and <c>698ECF</c>,
    /// which is exactly <c>c × 0.8</c> and <c>c × 0.8 + 51</c> truncated.
    /// </remarks>
    private static double Brightness(PresetPathFill fill) => fill switch
    {
        PresetPathFill.Darken => -0.4,
        PresetPathFill.DarkenLess => -0.2,
        PresetPathFill.Lighten => 0.4,
        PresetPathFill.LightenLess => 0.2,
        _ => 0.0,
    };

    /// <summary>Copies one path's commands onto the end of another.</summary>
    private static void Append(GraphicsPath into, GraphicsPath from)
    {
        foreach (PathCommand command in from.Commands)
        {
            switch (command.Verb)
            {
                case PathVerb.MoveTo: into.MoveTo(command.Point); break;
                case PathVerb.LineTo: into.LineTo(command.Point); break;
                case PathVerb.CubicTo:
                    into.CubicTo(command.Control1, command.Control2, command.Point);
                    break;
                case PathVerb.Close: into.Close(); break;
                default: break;
            }
        }
    }

    /// <summary>The bounding rectangle, which is what an unknown preset draws.</summary>
    private static GraphicsPath Rectangle(DocSize size)
        => new GraphicsPath()
            .MoveTo(new DocPoint(Length.Zero, Length.Zero))
            .LineTo(new DocPoint(size.Width, Length.Zero))
            .LineTo(new DocPoint(size.Width, size.Height))
            .LineTo(new DocPoint(Length.Zero, size.Height))
            .Close();
}

/// <summary>
/// A placed geometry split into what is filled, what is stroked, and what is shaded.
/// </summary>
/// <param name="Fill">
/// The subpaths taking the shape's fill unchanged. The whole outline when every subpath agrees.
/// </param>
/// <param name="Stroke">The subpaths the pen runs along. The whole outline when every subpath agrees.</param>
/// <param name="Shaded">
/// The subpaths taking the shape's fill lightened or darkened, in the order the preset states them
/// — which is paint order, and which is why they are a list rather than a path per mode. Empty for
/// all but 27 of the 187 presets.
/// </param>
public readonly record struct PaintedGeometry(
    GraphicsPath Fill,
    GraphicsPath Stroke,
    IReadOnlyList<SlideShadedPart>? Shaded = null)
{
    /// <summary>The shaded subpaths, never null.</summary>
    public IReadOnlyList<SlideShadedPart> ShadedParts => Shaded ?? [];
}

/// <summary>One subpath drawn in a lightened or darkened version of the shape's own fill.</summary>
/// <param name="Outline">The subpath, in slide coordinates.</param>
/// <param name="Brightness">
/// How far towards white (positive) or black (negative) the shape's fill is taken, as
/// <c>EnhancedCustomShape2d</c> states it: ±0.2 or ±0.4.
/// </param>
public readonly record struct SlideShadedPart(GraphicsPath Outline, double Brightness);
