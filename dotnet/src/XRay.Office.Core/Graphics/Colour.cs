namespace XRay.Office.Core.Graphics;

/// <summary>
/// A straight (non-premultiplied) 8-bit-per-channel sRGB colour with alpha.
/// </summary>
/// <remarks>
/// Office formats are uniformly 8-bit sRGB, so there is nothing to gain from a
/// wider representation here. Alpha is stored straight rather than premultiplied
/// because that is how every format expresses transparency, and because
/// premultiplying early loses colour information in transparent pixels.
/// <para>
/// Note that the legacy binary formats express transparency as a percentage in a
/// separate attribute, not as an alpha channel; readers fold that into
/// <see cref="A"/> when constructing colours.
/// </para>
/// </remarks>
/// <param name="R">Red channel.</param>
/// <param name="G">Green channel.</param>
/// <param name="B">Blue channel.</param>
/// <param name="A">Alpha channel; 255 is fully opaque.</param>
public readonly record struct Colour(byte R, byte G, byte B, byte A = 255)
{
    /// <summary>Fully transparent.</summary>
    public static readonly Colour Transparent = new(0, 0, 0, 0);

    /// <summary>Opaque black.</summary>
    public static readonly Colour Black = new(0, 0, 0);

    /// <summary>Opaque white.</summary>
    public static readonly Colour White = new(255, 255, 255);

    /// <summary>Creates an opaque colour from a 0xRRGGBB value.</summary>
    public static Colour FromRgb(uint rgb) => new(
        (byte)((rgb >> 16) & 0xFF),
        (byte)((rgb >> 8) & 0xFF),
        (byte)(rgb & 0xFF));

    /// <summary>Creates a colour from a 0xAARRGGBB value.</summary>
    public static Colour FromArgb(uint argb) => new(
        (byte)((argb >> 16) & 0xFF),
        (byte)((argb >> 8) & 0xFF),
        (byte)(argb & 0xFF),
        (byte)((argb >> 24) & 0xFF));

    /// <summary>The colour as 0xAARRGGBB.</summary>
    public uint ToArgb() => ((uint)A << 24) | ((uint)R << 16) | ((uint)G << 8) | B;

    /// <summary>True when the colour is fully transparent.</summary>
    public bool IsTransparent => A == 0;

    /// <summary>True when the colour is fully opaque.</summary>
    public bool IsOpaque => A == 255;

    /// <summary>Returns this colour with a different alpha.</summary>
    public Colour WithAlpha(byte alpha) => new(R, G, B, alpha);

    /// <summary>
    /// Whether text drawn on this colour should be reversed out of it — <c>Color::IsDark</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// This is what decides an <em>automatic</em> font colour: LibreOffice resolves
    /// <c>COL_AUTO</c> against whatever brush the frame chain supplies and answers white when the
    /// answer is dark (<c>SwDrawTextInfo::ApplyAutoColor</c>,
    /// <c>sw/source/core/txtnode/fntcache.cxx</c>:2429).
    /// </para>
    /// <para>
    /// <strong>It is two formulas and not one, and the second is not a curiosity.</strong>
    /// <c>tools/source/generic/color.cxx</c>:52 tests the exact value <c>0x729FCF</c> —
    /// <c>COL_DEFAULT_SHAPE_FILLING</c>, the colour every unstyled Draw shape is born — and asks
    /// the <em>perceived</em> luminance <c>&lt;= 62</c> for it, where every other colour is asked
    /// the WCAG relative luminance <c>&lt;= 87</c>. The two disagree on that one input and on no
    /// other: <c>0x729FCF</c> has WCAG luminance 83, which is dark, and perceived luminance 151,
    /// which is bright. So a default-filled shape keeps black text and a shape one sRGB step away
    /// from that colour does not.
    /// </para>
    /// <para>
    /// Measured on 26.2.4.2 rather than read off the 27.2 tree in this checkout, because that tree
    /// is not the reference: <c>probes/words-r59/autocolour.py</c> draws
    /// <c>&lt;w:shd w:fill="729FCF"/&gt;</c> and <c>w:fill="6F9BCB"</c> as two otherwise identical
    /// packages, and the reference draws the first's text black and the second's white.
    /// </para>
    /// <para>
    /// Alpha plays no part. LibreOffice's <c>Color</c> carries transparency in the same word and
    /// neither luminance function reads it; a caller that means "no background at all" must not
    /// ask this question in the first place.
    /// </para>
    /// </remarks>
    public bool IsDark
        => (ToArgb() & 0x00FFFFFFu) == DefaultShapeFilling
            ? PerceivedLuminance <= 62
            : WcagLuminance <= 87;

    /// <summary>
    /// The WCAG 2.1 relative luminance, scaled to a byte — <c>Color::GetWCAGLuminance</c>.
    /// </summary>
    /// <remarks>
    /// Gamma-decoded per channel and weighted 0.2126 / 0.7152 / 0.0722, then multiplied by 255 and
    /// truncated. The truncation is part of the answer rather than a detail: the threshold is an
    /// integer comparison against 87, so grey <c>0x9E</c> at 87.2 is dark and grey <c>0x9F</c> at
    /// 88.4 is not, which round 58 confirmed against the reference to that single step.
    /// </remarks>
    public int WcagLuminance
        => (int)((Decoded(R) * 0.2126 + Decoded(G) * 0.7152 + Decoded(B) * 0.0722) * 255);

    /// <summary>
    /// The perceived luminance — <c>Color::GetLuminance</c>, <c>include/tools/color.hxx</c>:274.
    /// </summary>
    /// <remarks>
    /// Integer arithmetic on the gamma-encoded channels, which is a different quantity from
    /// <see cref="WcagLuminance"/> and not an approximation of it. Used by <see cref="IsDark"/> for
    /// one colour, and by the presentation and binary-PowerPoint readers for their own rules.
    /// </remarks>
    public int PerceivedLuminance => ((B * 29) + (G * 151) + (R * 76)) >> 8;

    /// <summary><c>COL_DEFAULT_SHAPE_FILLING</c>, the one colour <see cref="IsDark"/> excepts.</summary>
    private const uint DefaultShapeFilling = 0x729FCF;

    /// <summary>One channel, gamma-decoded — <c>NormalizeRGB</c>, <c>color.cxx</c>:35.</summary>
    private static double Decoded(byte channel)
    {
        double value = channel / 255.0;
        return value < 0.04045 ? value / 12.92 : Math.Pow((value + 0.055) / 1.055, 2.4);
    }

    /// <inheritdoc/>
    public override string ToString() => IsOpaque
        ? $"#{R:X2}{G:X2}{B:X2}"
        : $"#{A:X2}{R:X2}{G:X2}{B:X2}";
}
