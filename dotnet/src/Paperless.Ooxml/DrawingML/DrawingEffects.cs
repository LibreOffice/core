using System.Xml.Linq;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Ooxml.DrawingML;

/// <summary>
/// A DrawingML <c>a:effectLst</c>, reduced to the one effect that changes what reaches a page:
/// the drop shadow.
/// </summary>
/// <remarks>
/// <para>
/// LibreOffice reads the same one and no other. <c>EffectProperties::pushToPropMap</c>
/// (<c>oox/source/drawingml/effectproperties.cxx:61-114</c>) walks the effect list and acts on
/// <c>outerShdw</c> alone; <c>innerShdw</c>, <c>reflection</c>, <c>softEdge</c> and the rest are
/// parsed and dropped. Matching that is deliberate — the reference renderer is what a corpus
/// comparison is measured against, and an effect it does not draw is one we must not draw either.
/// </para>
/// <para>
/// Two conversions carry all the geometry and both are LibreOffice's, line for line. The
/// direction is in sixty-thousandths of a degree and the distance in EMUs, and the offset is
/// <c>cos(dir) × dist</c> across and <c>sin(dir) × dist</c> down — down, because the drawing
/// space has y growing downwards, which is why the commonest shadow in Office's own themes
/// (<c>dir="5400000"</c>, ninety degrees) falls straight below the shape rather than to its
/// right.
/// </para>
/// </remarks>
public static class DrawingEffects
{
    /// <summary>Sixty-thousandths of a degree, DrawingML's angular unit.</summary>
    private const double PerDegree = 60000.0;

    /// <summary>
    /// The drop shadow an effect list states, or null when it states none.
    /// </summary>
    /// <param name="effects">An <c>a:effectLst</c> element, or null.</param>
    /// <param name="theme">The theme its colours resolve against.</param>
    /// <param name="placeholder">
    /// What a <c>phClr</c> inside it stands for, when the list came from a theme's format scheme
    /// through an <c>a:effectRef</c>.
    /// </param>
    public static DrawingShadow? OuterShadow(
        XElement? effects, DrawingTheme? theme, Colour? placeholder = null)
    {
        if (Drawing.Child(effects, "outerShdw") is not { } shadow) return null;

        double angle = (Drawing.Number(shadow, "dir") ?? 0) / PerDegree * Math.PI / 180.0;
        double distance = Drawing.Number(shadow, "dist") ?? 0;

        Colour? colour = null;
        foreach (XElement child in shadow.Elements())
        {
            if (DrawingColour.Read(child)?.Resolve(theme, placeholder) is not { } resolved) continue;
            colour = resolved;
            break;
        }

        // Black at full strength is what a shadow with no colour of its own means; a shape can
        // state an outerShdw with nothing inside it, and PowerPoint draws it black.
        Colour paint = colour ?? Colour.Black;

        return new DrawingShadow(
            Length.FromEmu((long)Math.Round(Math.Cos(angle) * distance)),
            Length.FromEmu((long)Math.Round(Math.Sin(angle) * distance)),
            paint.WithAlpha(255),

            // The alpha is the shadow's transparency rather than part of its colour: LibreOffice
            // takes the two off the same element separately (`getColor` and `getTransparency`,
            // effectproperties.cxx:104-105) and applies the second to the whole shadow at once.
            paint.A / 255.0,
            Length.FromEmu(Drawing.Number(shadow, "blurRad") ?? 0));
    }

    /// <summary>
    /// The effect list a shape's <c>p:style</c> names in the theme's format scheme, or null.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The theme is where most shadows in real decks come from, and by a wide margin: across the
    /// 112 <c>pptx</c> decks of the slides corpus, 1120 slide shapes take a shadow through an
    /// <c>a:effectRef</c> against 352 that state one on their own <c>p:spPr</c>. A reader that
    /// only looks at <c>spPr</c> finds less than a quarter of them.
    /// </para>
    /// <para>
    /// The <c>phClr</c> of an <c>a:effectRef</c> is <strong>not</strong> substituted, which is
    /// LibreOffice's behaviour and is marked in its own source as unfinished — *"TODO: use ph
    /// color when applying effect properties"*, <c>oox/source/drawingml/shape.cxx:1556</c>. It
    /// costs nothing on real files: every theme Office ships writes its effect styles in literal
    /// <c>srgbClr</c> black with an <c>a:alpha</c>, so there is no placeholder to substitute.
    /// </para>
    /// </remarks>
    /// <param name="matrix">The theme's format scheme, or null.</param>
    /// <param name="style">The shape's <c>p:style</c> element, or null.</param>
    public static XElement? ThemeEffects(DrawingStyleMatrix? matrix, XElement? style)
        => matrix?.Effect(Drawing.Child(style, "effectRef"));
}

/// <summary>
/// An <c>a:outerShdw</c> resolved into geometry and colour.
/// </summary>
/// <param name="OffsetX">How far right the shadow sits; negative is left.</param>
/// <param name="OffsetY">How far down it sits; negative is up.</param>
/// <param name="Colour">Its colour, opaque — the transparency is <paramref name="Opacity"/>.</param>
/// <param name="Opacity">How opaque the shadow is as a whole, from 0 to 1.</param>
/// <param name="Blur">The blur radius the file states, or zero.</param>
public readonly record struct DrawingShadow(
    Length OffsetX,
    Length OffsetY,
    Colour Colour,
    double Opacity,
    Length Blur);
