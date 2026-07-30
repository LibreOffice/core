using Paperless.Core.Geometry;
using Paperless.Core.Graphics;

namespace Paperless.Ooxml.DrawingML;

/// <summary>
/// Expands a DrawingML preset shape into an explicit outline.
/// </summary>
/// <remarks>
/// <para>
/// There are roughly 190 preset geometries (<c>rect</c>, <c>roundRect</c>,
/// <c>ellipse</c>, <c>rightArrow</c>, <c>star5</c>, and so on). Each is defined as a
/// parameterised path: a set of guide formulas evaluated against the shape's bounding
/// box and its adjustment handles, then a path built from the results. Producing the
/// outline therefore means evaluating a small expression language, not looking up a
/// fixed shape.
/// </para>
/// <para>
/// LibreOffice stores the same definitions as generated data tables
/// (<c>oox/source/drawingml/customshapepresetdata.cxx</c>) and evaluates them with one
/// shared engine (<c>EnhancedCustomShape2d</c>) that also serves the ODF and legacy
/// binary syntaxes. Paperless should follow that structure: port the data tables
/// mechanically, then write one evaluator all three front ends feed into. See
/// <c>dotnet/research/04-impress.md</c> section A.
/// </para>
/// </remarks>
public interface IPresetGeometryExpander
{
    /// <summary>True when the named preset is known.</summary>
    bool IsKnownPreset(string presetName);

    /// <summary>
    /// Builds the outline of a preset shape.
    /// </summary>
    /// <param name="presetName">The DrawingML preset name, e.g. <c>roundRect</c>.</param>
    /// <param name="bounds">The shape's bounding box.</param>
    /// <param name="adjustments">
    /// Adjustment values by guide name, from the shape's <c>avLst</c>. Presets supply
    /// defaults for any that are absent.
    /// </param>
    GraphicsPath Expand(string presetName, DocRect bounds, IReadOnlyDictionary<string, double> adjustments);
}
