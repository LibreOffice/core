using System.Globalization;
using System.Xml.Linq;
using Paperless.Core.Graphics;

namespace Paperless.Ooxml.DrawingML;

/// <summary>How a DrawingML colour states its base value.</summary>
public enum DrawingColourKind
{
    /// <summary>A literal 8-bit sRGB triple (<c>a:srgbClr</c>).</summary>
    Rgb,

    /// <summary>Component percentages (<c>a:scrgbClr</c>).</summary>
    ScRgb,

    /// <summary>Hue, saturation and luminance (<c>a:hslClr</c>).</summary>
    Hsl,

    /// <summary>A reference into the theme's colour scheme (<c>a:schemeClr</c>).</summary>
    Scheme,

    /// <summary>The placeholder colour a style matrix reference substitutes (<c>schemeClr val="phClr"</c>).</summary>
    Placeholder,

    /// <summary>A system colour such as <c>windowText</c> (<c>a:sysClr</c>).</summary>
    System,

    /// <summary>One of DrawingML's named colours (<c>a:prstClr</c>).</summary>
    Preset,
}

/// <summary>
/// A DrawingML colour reference: where the base colour comes from, and the chain of transforms
/// over it.
/// </summary>
/// <remarks>
/// <para>
/// Kept as a reference rather than resolved on read, because it cannot be resolved on read: a
/// <c>schemeClr</c> needs the theme, and a <c>phClr</c> needs the colour the style matrix
/// reference that contains it was applied with. Both arrive later than the element does.
/// </para>
/// <para>
/// This is the shared type the three families' readers produce and
/// <see cref="DrawingColourTransforms"/> consumes, so the transform chain — the part that is
/// easy to get wrong and wrong everywhere at once — exists in exactly one place.
/// </para>
/// </remarks>
public sealed record DrawingColour
{
    private DrawingColour(DrawingColourKind kind)
    {
        Kind = kind;
        Transforms = [];
    }

    /// <summary>Where the base colour comes from.</summary>
    public DrawingColourKind Kind { get; private init; }

    /// <summary>The literal value, for the kinds that state one.</summary>
    public Colour Literal { get; private init; }

    /// <summary>The scheme-colour name, for <see cref="DrawingColourKind.Scheme"/>.</summary>
    public string? SchemeName { get; private init; }

    /// <summary>The transforms, in the order the file states them.</summary>
    public IReadOnlyList<ColourTransform> Transforms { get; private init; }

    /// <summary>A colour that is a literal value with no transforms.</summary>
    public static DrawingColour FromRgb(Colour colour)
        => new(DrawingColourKind.Rgb) { Literal = colour };

    /// <summary>
    /// A scheme reference with a transform chain, for readers whose format spells the chain some
    /// other way — WordprocessingML's <c>w:themeTint</c> above all.
    /// </summary>
    /// <param name="schemeName">The scheme-colour name, such as <c>accent1</c> or <c>text1</c>.</param>
    /// <param name="transforms">The chain, in application order.</param>
    public static DrawingColour FromScheme(string schemeName, IReadOnlyList<ColourTransform>? transforms = null)
        => new(DrawingColourKind.Scheme)
        {
            SchemeName = schemeName,
            Transforms = transforms ?? [],
        };

    /// <summary>
    /// Reads one colour-reference element, or null when the element is not one.
    /// </summary>
    /// <remarks>
    /// The caller passes the reference element itself — <c>a:srgbClr</c> and friends — rather
    /// than its parent, because the parents are many (<c>a:solidFill</c>, <c>a:fgClr</c>,
    /// <c>a:gs</c>, a colour-scheme slot) and all of them hold exactly one of these seven.
    /// </remarks>
    /// <param name="element">The candidate element.</param>
    public static DrawingColour? Read(XElement? element)
    {
        if (element is null || element.Name.NamespaceName != OoxmlNamespaces.DrawingML) return null;

        DrawingColour? colour = element.Name.LocalName switch
        {
            "srgbClr" => Rgb(Drawing.Attribute(element, "val")),
            "scrgbClr" => ScRgb(element),
            "hslClr" => Hsl(element),
            "schemeClr" => Scheme(Drawing.Attribute(element, "val")),
            "sysClr" => System(element),
            "prstClr" => Preset(Drawing.Attribute(element, "val")),
            _ => null,
        };

        return colour?.WithTransforms(ReadTransforms(element));
    }

    /// <summary>
    /// Resolves to a concrete colour.
    /// </summary>
    /// <param name="theme">The theme in force, or null when the document has none.</param>
    /// <param name="placeholder">
    /// What <c>phClr</c> stands for here, or null outside a style matrix reference.
    /// </param>
    /// <returns>The colour, or null when the reference cannot be resolved at all.</returns>
    public Colour? Resolve(DrawingTheme? theme, Colour? placeholder = null)
    {
        Colour? seed = Kind switch
        {
            DrawingColourKind.Scheme => theme?.Lookup(SchemeName),
            DrawingColourKind.Placeholder => placeholder,
            _ => Literal,
        };

        return seed is { } value ? DrawingColourTransforms.Apply(value, Transforms) : null;
    }

    /// <summary>This colour with a different transform chain.</summary>
    public DrawingColour WithTransforms(IReadOnlyList<ColourTransform> transforms)
        => transforms.Count == 0 && Transforms.Count == 0 ? this : this with { Transforms = transforms };

    private static List<ColourTransform> ReadTransforms(XElement element)
    {
        List<ColourTransform> transforms = [];

        foreach (XElement child in element.Elements())
        {
            if (child.Name.NamespaceName != OoxmlNamespaces.DrawingML) continue;
            if (ColourTransform.KindOf(child.Name.LocalName) is not { } kind) continue;

            transforms.Add(new ColourTransform(kind, Units(Drawing.Attribute(child, "val"))));
        }

        return transforms;
    }

    /// <summary>
    /// A transform's argument in DrawingML units.
    /// </summary>
    /// <remarks>
    /// Two spellings, and both appear: ECMA-376 1st edition writes a percentage as an integer in
    /// thousandths (<c>val="60000"</c>) while ISO/IEC 29500 strict writes it with a sign
    /// (<c>val="60%"</c>). Reading only the first turns a strict file's 60% into a zero — which
    /// for a <c>lumMod</c> is a black run rather than a mid one, visible but not obviously a
    /// parsing failure. Angles are in sixtieth-thousandths of a degree in both and never carry
    /// the sign, so one branch covers them.
    /// </remarks>
    private static int Units(string? value)
    {
        if (string.IsNullOrEmpty(value)) return 0;

        if (value[^1] == '%')
        {
            return double.TryParse(
                value.AsSpan(0, value.Length - 1), NumberStyles.Float, CultureInfo.InvariantCulture,
                out double percent)
                ? (int)Math.Round(percent * DrawingColourTransforms.PerPercent)
                : 0;
        }

        return int.TryParse(value, NumberStyles.Integer, CultureInfo.InvariantCulture, out int units)
            ? units
            : 0;
    }

    private static DrawingColour? Rgb(string? value)
    {
        if (value is null) return null;

        ReadOnlySpan<char> digits = value.AsSpan().TrimStart('#');
        return digits.Length == 6
               && uint.TryParse(digits, NumberStyles.HexNumber, CultureInfo.InvariantCulture, out uint rgb)
            ? new DrawingColour(DrawingColourKind.Rgb) { Literal = Colour.FromRgb(rgb) }
            : null;
    }

    private static DrawingColour ScRgb(XElement element)
    {
        int r = Units(Drawing.Attribute(element, "r"));
        int g = Units(Drawing.Attribute(element, "g"));
        int b = Units(Drawing.Attribute(element, "b"));

        // The components are gamma-decoded percentages, which is exactly the CRGB space the
        // transform chain works in, so they go back through the same conversion rather than
        // being scaled to bytes directly.
        return new DrawingColour(DrawingColourKind.ScRgb)
        {
            Literal = DrawingColourTransforms.Apply(
                Colour.Black,
                [
                    new ColourTransform(ColourTransformKind.Red, Math.Clamp(r, 0, DrawingColourTransforms.MaxPercent)),
                    new ColourTransform(ColourTransformKind.Green, Math.Clamp(g, 0, DrawingColourTransforms.MaxPercent)),
                    new ColourTransform(ColourTransformKind.Blue, Math.Clamp(b, 0, DrawingColourTransforms.MaxPercent)),
                ]),
        };
    }

    private static DrawingColour Hsl(XElement element)
    {
        int hue = Units(Drawing.Attribute(element, "hue"));
        int saturation = Units(Drawing.Attribute(element, "sat"));
        int luminance = Units(Drawing.Attribute(element, "lum"));

        return new DrawingColour(DrawingColourKind.Hsl)
        {
            Literal = DrawingColourTransforms.Apply(
                Colour.Black,
                [
                    new ColourTransform(ColourTransformKind.Hue, Math.Clamp(hue, 0, DrawingColourTransforms.MaxDegree)),
                    new ColourTransform(ColourTransformKind.Saturation, Math.Clamp(saturation, 0, DrawingColourTransforms.MaxPercent)),
                    new ColourTransform(ColourTransformKind.Luminance, Math.Clamp(luminance, 0, DrawingColourTransforms.MaxPercent)),
                ]),
        };
    }

    private static DrawingColour? Scheme(string? value)
    {
        if (value is null) return null;

        return value == "phClr"
            ? new DrawingColour(DrawingColourKind.Placeholder)
            : new DrawingColour(DrawingColourKind.Scheme) { SchemeName = value };
    }

    /// <summary>
    /// A system colour, which is read from its <c>lastClr</c> rather than from the host.
    /// </summary>
    /// <remarks>
    /// <c>lastClr</c> is the value the producer saw and is what every theme Word ships states
    /// for <c>dk1</c> and <c>lt1</c>. Asking the host instead would make the same document
    /// render differently under a dark desktop theme, which is not what a document reader is
    /// for; LibreOffice's headless path does the same.
    /// </remarks>
    private static DrawingColour System(XElement element)
    {
        if (Rgb(Drawing.Attribute(element, "lastClr")) is { } known)
        {
            return known with { Kind = DrawingColourKind.System };
        }

        // Without one, only the two names a document actually uses are worth knowing; anything
        // else falls back to black, which is what an unresolvable text colour has to be.
        Colour fallback = Drawing.Attribute(element, "val") switch
        {
            "window" or "highlightText" or "btnHighlight" => Colour.White,
            _ => Colour.Black,
        };

        return new DrawingColour(DrawingColourKind.System) { Literal = fallback };
    }

    private static DrawingColour? Preset(string? value)
        => value is not null && PresetColours.Lookup(value) is { } colour
            ? new DrawingColour(DrawingColourKind.Preset) { Literal = colour }
            : null;
}
