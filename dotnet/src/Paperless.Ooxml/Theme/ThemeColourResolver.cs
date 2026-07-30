using Paperless.Core.Graphics;

namespace Paperless.Ooxml.Theme;

/// <summary>
/// Resolves a DrawingML colour reference to a concrete <see cref="Colour"/>.
/// </summary>
/// <remarks>
/// <para>
/// A DrawingML colour is rarely a literal value. It is usually a reference into the
/// theme's colour scheme with a chain of transforms applied — <c>lumMod</c>,
/// <c>lumOff</c>, <c>shade</c>, <c>tint</c>, <c>satMod</c>, <c>alpha</c> — and the
/// transforms must be applied in document order, in the right colour space, to land on
/// the right value.
/// </para>
/// <para>
/// Getting this wrong is the most visible single failure mode in PPTX rendering: every
/// themed shape on every slide comes out the wrong colour. LibreOffice's
/// implementation is <c>oox/source/drawingml/color.cxx</c>; see
/// <c>dotnet/research/04-impress.md</c> section B for the exact arithmetic.
/// </para>
/// </remarks>
public interface IThemeColourResolver
{
    /// <summary>
    /// Resolves a scheme colour name such as <c>accent1</c>, <c>tx1</c>, <c>bg1</c>,
    /// <c>phClr</c>, applying the given transforms in order.
    /// </summary>
    Colour Resolve(string schemeColourName, IReadOnlyList<ColourTransform> transforms);
}

/// <summary>One DrawingML colour transform.</summary>
/// <param name="Kind">Which transform to apply.</param>
/// <param name="Value">
/// The transform's argument. Percentage-valued transforms are normalised to 0-1 here,
/// not left in DrawingML's 1000ths-of-a-percent encoding.
/// </param>
public readonly record struct ColourTransform(ColourTransformKind Kind, double Value);

/// <summary>The DrawingML colour transforms.</summary>
public enum ColourTransformKind
{
    /// <summary>Multiply luminance.</summary>
    LuminanceModulation,

    /// <summary>Offset luminance.</summary>
    LuminanceOffset,

    /// <summary>Darken towards black.</summary>
    Shade,

    /// <summary>Lighten towards white.</summary>
    Tint,

    /// <summary>Multiply saturation.</summary>
    SaturationModulation,

    /// <summary>Offset saturation.</summary>
    SaturationOffset,

    /// <summary>Multiply hue.</summary>
    HueModulation,

    /// <summary>Offset hue.</summary>
    HueOffset,

    /// <summary>Set alpha outright.</summary>
    Alpha,

    /// <summary>Multiply alpha.</summary>
    AlphaModulation,

    /// <summary>Invert the colour.</summary>
    Invert,

    /// <summary>Convert to greyscale.</summary>
    Greyscale,
}
