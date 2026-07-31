using Paperless.Core.Graphics;

namespace Paperless.Ooxml.DrawingML;

/// <summary>
/// The DrawingML colour transforms, named as the elements that carry them.
/// </summary>
/// <remarks>
/// All of them, rather than the six that appear in practice, because the chain has to be walked
/// in document order and an unrecognised member of it cannot be skipped without changing the
/// answer — <c>lumMod</c> after a <c>gray</c> is not <c>lumMod</c> on the original colour.
/// </remarks>
public enum ColourTransformKind
{
    /// <summary>Set the red component (<c>a:red</c>).</summary>
    Red,

    /// <summary>Multiply the red component (<c>a:redMod</c>).</summary>
    RedModulation,

    /// <summary>Offset the red component (<c>a:redOff</c>).</summary>
    RedOffset,

    /// <summary>Set the green component (<c>a:green</c>).</summary>
    Green,

    /// <summary>Multiply the green component (<c>a:greenMod</c>).</summary>
    GreenModulation,

    /// <summary>Offset the green component (<c>a:greenOff</c>).</summary>
    GreenOffset,

    /// <summary>Set the blue component (<c>a:blue</c>).</summary>
    Blue,

    /// <summary>Multiply the blue component (<c>a:blueMod</c>).</summary>
    BlueModulation,

    /// <summary>Offset the blue component (<c>a:blueOff</c>).</summary>
    BlueOffset,

    /// <summary>Set the hue (<c>a:hue</c>), in sixtieth-thousandths of a degree.</summary>
    Hue,

    /// <summary>Multiply the hue (<c>a:hueMod</c>).</summary>
    HueModulation,

    /// <summary>Offset the hue (<c>a:hueOff</c>).</summary>
    HueOffset,

    /// <summary>Set the saturation (<c>a:sat</c>).</summary>
    Saturation,

    /// <summary>Multiply the saturation (<c>a:satMod</c>).</summary>
    SaturationModulation,

    /// <summary>Offset the saturation (<c>a:satOff</c>).</summary>
    SaturationOffset,

    /// <summary>Set the luminance (<c>a:lum</c>).</summary>
    Luminance,

    /// <summary>Multiply the luminance (<c>a:lumMod</c>).</summary>
    LuminanceModulation,

    /// <summary>Offset the luminance (<c>a:lumOff</c>).</summary>
    LuminanceOffset,

    /// <summary>Darken towards black (<c>a:shade</c>); 0% is black and 100% the original.</summary>
    Shade,

    /// <summary>Lighten towards white (<c>a:tint</c>); 0% is white and 100% the original.</summary>
    Tint,

    /// <summary>Convert to grey (<c>a:gray</c>).</summary>
    Grey,

    /// <summary>Rotate the hue by half a turn (<c>a:comp</c>).</summary>
    Complement,

    /// <summary>Invert the colour (<c>a:inv</c>).</summary>
    Invert,

    /// <summary>Increase gamma (<c>a:gamma</c>).</summary>
    Gamma,

    /// <summary>Decrease gamma (<c>a:invGamma</c>).</summary>
    InverseGamma,

    /// <summary>Set the alpha outright (<c>a:alpha</c>).</summary>
    Alpha,

    /// <summary>Multiply the alpha (<c>a:alphaMod</c>).</summary>
    AlphaModulation,

    /// <summary>Offset the alpha (<c>a:alphaOff</c>).</summary>
    AlphaOffset,
}

/// <summary>
/// One transform in a colour's chain.
/// </summary>
/// <remarks>
/// The value is kept in DrawingML's own units — thousandths of a percent for everything except
/// hue, which is in sixtieth-thousandths of a degree — rather than normalised to a fraction.
/// The arithmetic is integer in LibreOffice and the intermediate truncations are visible in the
/// answer, so converting to a fraction on the way in and back on the way out moves results by a
/// unit or two.
/// </remarks>
/// <param name="Kind">Which transform.</param>
/// <param name="Value">Its argument, in DrawingML units.</param>
public readonly record struct ColourTransform(ColourTransformKind Kind, int Value)
{
    /// <summary>The transform an element's local name names, or null when it is not one.</summary>
    public static ColourTransformKind? KindOf(string localName) => localName switch
    {
        "red" => ColourTransformKind.Red,
        "redMod" => ColourTransformKind.RedModulation,
        "redOff" => ColourTransformKind.RedOffset,
        "green" => ColourTransformKind.Green,
        "greenMod" => ColourTransformKind.GreenModulation,
        "greenOff" => ColourTransformKind.GreenOffset,
        "blue" => ColourTransformKind.Blue,
        "blueMod" => ColourTransformKind.BlueModulation,
        "blueOff" => ColourTransformKind.BlueOffset,
        "hue" => ColourTransformKind.Hue,
        "hueMod" => ColourTransformKind.HueModulation,
        "hueOff" => ColourTransformKind.HueOffset,
        "sat" => ColourTransformKind.Saturation,
        "satMod" => ColourTransformKind.SaturationModulation,
        "satOff" => ColourTransformKind.SaturationOffset,
        "lum" => ColourTransformKind.Luminance,
        "lumMod" => ColourTransformKind.LuminanceModulation,
        "lumOff" => ColourTransformKind.LuminanceOffset,
        "shade" => ColourTransformKind.Shade,
        "tint" => ColourTransformKind.Tint,
        "gray" => ColourTransformKind.Grey,
        "comp" => ColourTransformKind.Complement,
        "inv" => ColourTransformKind.Invert,
        "gamma" => ColourTransformKind.Gamma,
        "invGamma" => ColourTransformKind.InverseGamma,
        "alpha" => ColourTransformKind.Alpha,
        "alphaMod" => ColourTransformKind.AlphaModulation,
        "alphaOff" => ColourTransformKind.AlphaOffset,
        _ => null,
    };
}

/// <summary>
/// Applies a DrawingML transform chain to a literal colour.
/// </summary>
/// <remarks>
/// <para>
/// A port of <c>oox::drawingml::Color::getColor</c>
/// (<c>oox/source/drawingml/color.cxx</c>:723), which is the authority because it is what
/// LibreOffice renders with — and every fidelity comparison in this project is against
/// LibreOffice's rendering.
/// </para>
/// <para>
/// Two things about the chain are load-bearing and neither is guessable from the element names.
/// </para>
/// <para>
/// <b>It is not commutative, so the order in the file is the order of application.</b> A colour
/// carrying <c>lumMod</c> then <c>shade</c> is a different colour from the same two the other
/// way round, because they work in different spaces: <c>lumMod</c> scales luminance in HSL and
/// <c>shade</c> scales the components in gamma-decoded RGB. Sorting the transforms, or applying
/// them by looking each one up in turn, produces a plausible colour rather than an obviously
/// wrong one.
/// </para>
/// <para>
/// <b>Each transform names the space it works in, and the value is converted between spaces as
/// the chain runs.</b> There are three: 8-bit sRGB, "CRGB" (thousandths of a percent per
/// component, gamma-decoded with an exponent of 2.3), and HSL (hue in sixtieth-thousandths of a
/// degree, saturation and luminance in thousandths of a percent). <c>shade</c> and <c>tint</c>
/// are CRGB, so they carry a gamma round trip that the specification's plain multiply does not
/// — a 50% shade of mid grey is not half of mid grey.
/// </para>
/// </remarks>
public static class DrawingColourTransforms
{
    /// <summary>One hundred percent, in DrawingML's thousandths of a percent.</summary>
    public const int MaxPercent = 100000;

    /// <summary>One percent, in the same units.</summary>
    public const int PerPercent = 1000;

    /// <summary>One degree, in DrawingML's sixtieth-thousandths.</summary>
    public const int PerDegree = 60000;

    /// <summary>A full turn, in the same units.</summary>
    public const int MaxDegree = 360 * PerDegree;

    private const double DecodingGamma = 2.3;
    private const double EncodingGamma = 1.0 / DecodingGamma;

    private enum Space
    {
        Rgb,
        Crgb,
        Hsl,
    }

    /// <summary>
    /// Applies a chain to a colour.
    /// </summary>
    /// <param name="colour">The literal colour the chain starts from.</param>
    /// <param name="transforms">The chain, in the order the file states it.</param>
    public static Colour Apply(Colour colour, IReadOnlyList<ColourTransform>? transforms)
    {
        if (transforms is null || transforms.Count == 0) return colour;

        Space space = Space.Rgb;
        int c1 = colour.R;
        int c2 = colour.G;
        int c3 = colour.B;

        // Alpha never changes the other three and is never changed by them, so it rides
        // alongside rather than through the space conversions — which is also what LibreOffice
        // does, by executing the alpha transforms at parse time (color.cxx:465).
        int alpha = colour.A * MaxPercent / 255;

        foreach (ColourTransform transform in transforms)
        {
            switch (transform.Kind)
            {
                case ColourTransformKind.Red:
                    ToCrgb(ref space, ref c1, ref c2, ref c3);
                    Set(ref c1, transform.Value, MaxPercent);
                    break;
                case ColourTransformKind.RedModulation:
                    ToCrgb(ref space, ref c1, ref c2, ref c3);
                    Modulate(ref c1, transform.Value, MaxPercent);
                    break;
                case ColourTransformKind.RedOffset:
                    ToCrgb(ref space, ref c1, ref c2, ref c3);
                    Offset(ref c1, transform.Value, MaxPercent);
                    break;
                case ColourTransformKind.Green:
                    ToCrgb(ref space, ref c1, ref c2, ref c3);
                    Set(ref c2, transform.Value, MaxPercent);
                    break;
                case ColourTransformKind.GreenModulation:
                    ToCrgb(ref space, ref c1, ref c2, ref c3);
                    Modulate(ref c2, transform.Value, MaxPercent);
                    break;
                case ColourTransformKind.GreenOffset:
                    ToCrgb(ref space, ref c1, ref c2, ref c3);
                    Offset(ref c2, transform.Value, MaxPercent);
                    break;
                case ColourTransformKind.Blue:
                    ToCrgb(ref space, ref c1, ref c2, ref c3);
                    Set(ref c3, transform.Value, MaxPercent);
                    break;
                case ColourTransformKind.BlueModulation:
                    ToCrgb(ref space, ref c1, ref c2, ref c3);
                    Modulate(ref c3, transform.Value, MaxPercent);
                    break;
                case ColourTransformKind.BlueOffset:
                    ToCrgb(ref space, ref c1, ref c2, ref c3);
                    Offset(ref c3, transform.Value, MaxPercent);
                    break;

                case ColourTransformKind.Hue:
                    ToHsl(ref space, ref c1, ref c2, ref c3);
                    Set(ref c1, transform.Value, MaxDegree);
                    break;
                case ColourTransformKind.HueModulation:
                    ToHsl(ref space, ref c1, ref c2, ref c3);
                    Modulate(ref c1, transform.Value, MaxDegree);
                    break;
                case ColourTransformKind.HueOffset:
                    ToHsl(ref space, ref c1, ref c2, ref c3);
                    Offset(ref c1, transform.Value, MaxDegree);
                    break;
                case ColourTransformKind.Saturation:
                    ToHsl(ref space, ref c1, ref c2, ref c3);
                    Set(ref c2, transform.Value, MaxPercent);
                    break;
                case ColourTransformKind.SaturationModulation:
                    ToHsl(ref space, ref c1, ref c2, ref c3);
                    Modulate(ref c2, transform.Value, MaxPercent);
                    break;
                case ColourTransformKind.SaturationOffset:
                    ToHsl(ref space, ref c1, ref c2, ref c3);
                    Offset(ref c2, transform.Value, MaxPercent);
                    break;

                case ColourTransformKind.Luminance:
                    ToHsl(ref space, ref c1, ref c2, ref c3);
                    Set(ref c3, transform.Value, MaxPercent);
                    Desaturate(c3, ref c2);
                    break;
                case ColourTransformKind.LuminanceModulation:
                    ToHsl(ref space, ref c1, ref c2, ref c3);
                    Modulate(ref c3, transform.Value, MaxPercent);
                    Desaturate(c3, ref c2);
                    break;
                case ColourTransformKind.LuminanceOffset:
                    ToHsl(ref space, ref c1, ref c2, ref c3);
                    Offset(ref c3, transform.Value, MaxPercent);
                    Desaturate(c3, ref c2);
                    break;

                case ColourTransformKind.Shade:
                    ToCrgb(ref space, ref c1, ref c2, ref c3);
                    if (transform.Value is >= 0 and <= MaxPercent)
                    {
                        double shade = (double)transform.Value / MaxPercent;
                        c1 = (int)(c1 * shade);
                        c2 = (int)(c2 * shade);
                        c3 = (int)(c3 * shade);
                    }
                    break;

                case ColourTransformKind.Tint:
                    ToCrgb(ref space, ref c1, ref c2, ref c3);
                    if (transform.Value is >= 0 and <= MaxPercent)
                    {
                        double tint = (double)transform.Value / MaxPercent;
                        c1 = (int)(MaxPercent - ((MaxPercent - c1) * tint));
                        c2 = (int)(MaxPercent - ((MaxPercent - c2) * tint));
                        c3 = (int)(MaxPercent - ((MaxPercent - c3) * tint));
                    }
                    break;

                case ColourTransformKind.Grey:
                    // The weights are DrawingML's, not the usual luma ones: 22/72/6.
                    ToRgb(ref space, ref c1, ref c2, ref c3);
                    c1 = c2 = c3 = ((c1 * 22) + (c2 * 72) + (c3 * 6)) / 100;
                    break;

                case ColourTransformKind.Complement:
                    ToHsl(ref space, ref c1, ref c2, ref c3);
                    c1 = (c1 + (180 * PerDegree)) % MaxDegree;
                    break;

                case ColourTransformKind.Invert:
                    ToCrgb(ref space, ref c1, ref c2, ref c3);
                    c1 = MaxPercent - c1;
                    c2 = MaxPercent - c2;
                    c3 = MaxPercent - c3;
                    break;

                case ColourTransformKind.Gamma:
                    ToCrgb(ref space, ref c1, ref c2, ref c3);
                    c1 = Gamma(c1, EncodingGamma);
                    c2 = Gamma(c2, EncodingGamma);
                    c3 = Gamma(c3, EncodingGamma);
                    break;

                case ColourTransformKind.InverseGamma:
                    ToCrgb(ref space, ref c1, ref c2, ref c3);
                    c1 = Gamma(c1, DecodingGamma);
                    c2 = Gamma(c2, DecodingGamma);
                    c3 = Gamma(c3, DecodingGamma);
                    break;

                case ColourTransformKind.Alpha:
                    Set(ref alpha, transform.Value, MaxPercent);
                    break;
                case ColourTransformKind.AlphaModulation:
                    Modulate(ref alpha, transform.Value, MaxPercent);
                    break;
                case ColourTransformKind.AlphaOffset:
                    Offset(ref alpha, transform.Value, MaxPercent);
                    break;

                default:
                    break;
            }
        }

        ToRgb(ref space, ref c1, ref c2, ref c3);

        // LibreOffice stores transparency rather than alpha and truncates on the way in
        // (color.cxx:872); this reproduces the same byte so a comparison against its output is
        // exact rather than off by one at the ends.
        int transparency = alpha >= MaxPercent
            ? 0
            : (int)(255.0 * (MaxPercent - alpha) / MaxPercent);

        return new Colour(
            (byte)Math.Clamp(c1, 0, 255),
            (byte)Math.Clamp(c2, 0, 255),
            (byte)Math.Clamp(c3, 0, 255),
            (byte)Math.Clamp(255 - transparency, 0, 255));
    }

    /// <summary>
    /// Zeroes the saturation once luminance has reached an extreme.
    /// </summary>
    /// <remarks>
    /// LibreOffice's comment says it: "if color changes to black or white, it will stay gray if
    /// luminance changes again" (color.cxx:782). Without it, a colour driven to black by one
    /// <c>lumMod</c> comes back saturated when a following <c>lumOff</c> raises it, so a
    /// <c>lumMod</c>/<c>lumOff</c> pair on a very dark colour yields a coloured grey.
    /// </remarks>
    private static void Desaturate(int luminance, ref int saturation)
    {
        if (luminance == 0 || luminance == MaxPercent) saturation = 0;
    }

    private static void Set(ref int value, int replacement, int max)
    {
        if (replacement >= 0 && replacement <= max) value = replacement;
    }

    private static void Modulate(ref int value, int modulation, int max)
        => value = (int)Math.Clamp((double)value * modulation / MaxPercent, 0, max);

    private static void Offset(ref int value, int offset, int max)
        => value = Math.Clamp(value + offset, 0, max);

    private static int Gamma(int component, double gamma)
        => (int)((Math.Pow((double)component / MaxPercent, gamma) * MaxPercent) + 0.5);

    private static void ToRgb(ref Space space, ref int c1, ref int c2, ref int c3)
    {
        switch (space)
        {
            case Space.Rgb:
                break;

            case Space.Crgb:
                space = Space.Rgb;
                c1 = Gamma(c1, EncodingGamma) * 255 / MaxPercent;
                c2 = Gamma(c2, EncodingGamma) * 255 / MaxPercent;
                c3 = Gamma(c3, EncodingGamma) * 255 / MaxPercent;
                break;

            case Space.Hsl:
                space = Space.Rgb;
                HslToRgb(ref c1, ref c2, ref c3);
                break;
        }
    }

    private static void ToCrgb(ref Space space, ref int c1, ref int c2, ref int c3)
    {
        if (space == Space.Hsl) ToRgb(ref space, ref c1, ref c2, ref c3);
        if (space != Space.Rgb) return;

        space = Space.Crgb;
        c1 = Gamma(c1 * MaxPercent / 255, DecodingGamma);
        c2 = Gamma(c2 * MaxPercent / 255, DecodingGamma);
        c3 = Gamma(c3 * MaxPercent / 255, DecodingGamma);
    }

    private static void ToHsl(ref Space space, ref int c1, ref int c2, ref int c3)
    {
        if (space == Space.Crgb) ToRgb(ref space, ref c1, ref c2, ref c3);
        if (space != Space.Rgb) return;

        space = Space.Hsl;

        double r = c1 / 255.0;
        double g = c2 / 255.0;
        double b = c3 / 255.0;
        double min = Math.Min(Math.Min(r, g), b);
        double max = Math.Max(Math.Max(r, g), b);
        double delta = max - min;

        if (delta == 0.0)
        {
            c1 = 0;
        }
        else if (max == r)
        {
            c1 = (int)((((g - b) / delta * 60.0) + 360.0) * PerDegree + 0.5) % MaxDegree;
        }
        else if (max == g)
        {
            c1 = (int)((((b - r) / delta * 60.0) + 120.0) * PerDegree + 0.5);
        }
        else
        {
            c1 = (int)((((r - g) / delta * 60.0) + 240.0) * PerDegree + 0.5);
        }

        int luminance = (int)(((min + max) / 2.0 * MaxPercent) + 0.5);

        int saturation;
        if (luminance == 0 || luminance == MaxPercent) saturation = 0;
        else if (luminance <= 50 * PerPercent) saturation = (int)((delta / (min + max) * MaxPercent) + 0.5);
        else saturation = (int)((delta / (2.0 - max - min) * MaxPercent) + 0.5);

        c2 = saturation;
        c3 = luminance;
    }

    /// <summary>
    /// HSL to 8-bit RGB, exactly as <c>Color::toRgb</c> spells it.
    /// </summary>
    /// <remarks>
    /// Written as "hue gives a fully saturated colour, saturation pulls it towards mid grey,
    /// luminance then shades towards black or tints towards white" rather than as the textbook
    /// <c>c</c>/<c>x</c>/<c>m</c> formulation. The two agree, but keeping LibreOffice's shape
    /// keeps its rounding: every intermediate is a double and only the final component is
    /// rounded, so the answer matches its output byte for byte.
    /// </remarks>
    private static void HslToRgb(ref int c1, ref int c2, ref int c3)
    {
        double r = 0.0;
        double g = 0.0;
        double b = 0.0;

        if (c2 == 0 || c3 == MaxPercent)
        {
            r = g = b = (double)c3 / MaxPercent;
        }
        else if (c3 > 0)
        {
            double hue = (double)c1 / MaxDegree * 6.0;
            if (hue <= 1.0) { r = 1.0; g = hue; }
            else if (hue <= 2.0) { r = 2.0 - hue; g = 1.0; }
            else if (hue <= 3.0) { g = 1.0; b = hue - 2.0; }
            else if (hue <= 4.0) { g = 4.0 - hue; b = 1.0; }
            else if (hue <= 5.0) { r = hue - 4.0; b = 1.0; }
            else { r = 1.0; b = 6.0 - hue; }

            double saturation = (double)c2 / MaxPercent;
            r = ((r - 0.5) * saturation) + 0.5;
            g = ((g - 0.5) * saturation) + 0.5;
            b = ((b - 0.5) * saturation) + 0.5;

            double luminance = (2.0 * c3 / MaxPercent) - 1.0;
            if (luminance < 0.0)
            {
                double shade = luminance + 1.0;
                r *= shade;
                g *= shade;
                b *= shade;
            }
            else if (luminance > 0.0)
            {
                double tint = 1.0 - luminance;
                r = 1.0 - ((1.0 - r) * tint);
                g = 1.0 - ((1.0 - g) * tint);
                b = 1.0 - ((1.0 - b) * tint);
            }
        }

        c1 = (int)((r * 255.0) + 0.5);
        c2 = (int)((g * 255.0) + 0.5);
        c3 = (int)((b * 255.0) + 0.5);
    }
}
