using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Ooxml.DrawingML;

namespace Paperless.Presentations.Layout;

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

    /// <summary>The bounding rectangle, which is what an unknown preset draws.</summary>
    private static GraphicsPath Rectangle(DocSize size)
        => new GraphicsPath()
            .MoveTo(new DocPoint(Length.Zero, Length.Zero))
            .LineTo(new DocPoint(size.Width, Length.Zero))
            .LineTo(new DocPoint(size.Width, size.Height))
            .LineTo(new DocPoint(Length.Zero, size.Height))
            .Close();
}
