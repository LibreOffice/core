namespace Paperless.Core.Graphics;

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

    /// <inheritdoc/>
    public override string ToString() => IsOpaque
        ? $"#{R:X2}{G:X2}{B:X2}"
        : $"#{A:X2}{R:X2}{G:X2}{B:X2}";
}
