using System.Xml.Linq;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Ooxml.DrawingML;

/// <summary>
/// Turns an <c>a:pattFill</c> into the paint LibreOffice draws for it.
/// </summary>
/// <remarks>
/// <para>
/// The whole of <c>fillproperties.cxx:755-783</c>, which is shorter than it looks and has one
/// branch worth stating outright: a pattern only becomes a hatch when it states
/// <em>both</em> a foreground colour and a preset. Failing either, its background colour is
/// painted on its own as a solid fill, and failing that too the shape is unfilled. So the
/// three outcomes are a hatch, a solid, or nothing, and the second is not a fallback for a
/// preset we do not know — an unknown preset still yields a hatch, with a distance of nought
/// and therefore no lines, which reaches the page as its background alone by a different
/// route.
/// </para>
/// <para>
/// Separate from <see cref="DrawingFill"/> because it is resolution rather than parsing: it
/// needs the theme, which <see cref="DrawingFill"/> deliberately does not take.
/// </para>
/// </remarks>
public static class DrawingHatch
{
    /// <summary>
    /// The paint an <c>a:pattFill</c> draws as, or null when the element is not one or states
    /// nothing that can be drawn.
    /// </summary>
    /// <param name="element">The candidate <c>a:pattFill</c>.</param>
    /// <param name="theme">The theme its colours resolve against.</param>
    /// <param name="placeholder">
    /// The colour a <c>phClr</c> inside it means, when the fill arrived from a style matrix.
    /// </param>
    public static Paint? Read(XElement? element, DrawingTheme? theme, Colour? placeholder = null)
    {
        if (DrawingFill.ReadPattern(element) is not { } pattern) return null;

        Colour? foreground = pattern.Foreground?.Resolve(theme, placeholder);
        Colour? background = pattern.Background?.Resolve(theme, placeholder);

        if (foreground is { } lines && pattern.Preset is not null)
        {
            (HatchKind kind, Length distance, double angle) =
                DrawingHatchPresets.Hatch(pattern.Preset) ?? (HatchKind.OneWay, Length.Zero, 0);

            // A background of no opacity at all is not painted: upstream sets FillBackground
            // from `getTransparency() != 100`, and a hatch over nothing shows the slide.
            return new HatchPaint(
                lines, kind, distance, angle, background is { A: > 0 } behind ? behind : null);
        }

        return background is { } solid ? Paint.Solid(solid) : null;
    }
}
