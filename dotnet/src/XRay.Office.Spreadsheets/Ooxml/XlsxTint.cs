using XRay.Office.Core.Graphics;

namespace XRay.Office.Spreadsheets.Ooxml;

/// <summary>
/// SpreadsheetML's <c>tint</c> attribute, which lightens or darkens a stated colour.
/// </summary>
/// <remarks>
/// <para>
/// <strong>It is a luminance modulation in HSL, not a blend towards white and not a shift
/// applied to the RGB channels.</strong> Excel turns a <c>tint</c> into a pair of DrawingML
/// transforms — <c>lumMod(1 − t)</c> followed by <c>lumOff(t)</c> for a positive tint, and
/// <c>lumMod(1 + t)</c> alone for a negative one (<c>Color::addExcelTintTransformation</c>,
/// <c>oox/source/drawingml/color.cxx:497</c>) — and both act on the HSL luminance, because
/// every <c>Lum*</c> case in <c>Color::getColor</c> calls <c>toHsl()</c> first
/// (<c>oox/source/drawingml/color.cxx:780-792</c>). Hue and saturation are carried through
/// untouched and the colour is converted back to RGB afterwards.
/// </para>
/// <para>
/// The same transform applies to every Excel colour, whatever it decorates: <c>XlsColor</c>'s
/// <c>setRgb</c>, <c>setTheme</c> and <c>setIndexed</c> each call it
/// (<c>sc/source/filter/oox/stylesbuffer.cxx:255-279</c>), and one <c>XlsColor</c> serves fonts,
/// fills and borders alike. So there is one implementation here rather than one per call site.
/// </para>
/// <para>
/// <strong>Applying the luminance change as an RGB offset instead is what this replaces, and it
/// distorts hue by clamping.</strong> Shifting every channel by the same amount drives whichever
/// channel is already brightest past 255, where it sticks, while the others keep moving — so the
/// result is both wrong and wrong in a direction that varies with the colour. Measured on
/// <c>template-ECSPR-notifications.xlsx</c>, whose fills are the stock Office accents at
/// <c>tint="0.79998168889431442"</c>:
/// </para>
/// <list type="table">
///   <listheader><term>base</term><description>offset form, HSL form, reference</description></listheader>
///   <item>
///     <term>accent1 <c>#4472C4</c></term>
///     <description>
///       offset <c>#A6D4FF</c> (blue clamped at 255) — HSL <c>#DAE3F3</c> — reference
///       <c>#DAE3F3</c>. Another fill in the same workbook states <c>#D9E2F3</c> literally,
///       one off, because Excel truncates where LibreOffice rounds; see
///       <see cref="FromHsl"/>.
///     </description>
///   </item>
///   <item>
///     <term>accent4 <c>#FFC000</c></term>
///     <description>
///       offset <c>#FFFF66</c> (red and green both clamped, turning gold into lemon) — HSL
///       <c>#FFF2CC</c> — reference <c>#FFF2CC</c>.
///     </description>
///   </item>
/// </list>
/// </remarks>
internal static class XlsxTint
{
    /// <summary>
    /// A colour with its HSL luminance modulated by a tint, or the colour itself when the tint
    /// is zero.
    /// </summary>
    /// <param name="colour">The colour the file states.</param>
    /// <param name="tint">
    /// The <c>tint</c> attribute: positive lightens towards white, negative darkens towards
    /// black, and zero leaves the colour alone. Values outside −1..1 are clamped, as
    /// <c>getLimitedValue</c> does.
    /// </param>
    /// <returns>The tinted colour, with alpha carried through unchanged.</returns>
    public static Colour Apply(Colour colour, double tint)
    {
        if (double.IsNaN(tint) || Math.Abs(tint) < 0.0001) return colour;

        double factor = Math.Clamp(tint, -1.0, 1.0);

        (double hue, double saturation, double luminance) = ToHsl(colour);

        // lumMod(1 - t) then lumOff(t) for a positive tint; lumMod(1 + t) alone for a negative
        // one. Written out rather than as two steps because the second is a no-op when negative.
        luminance = factor < 0
            ? luminance * (1 + factor)
            : (luminance * (1 - factor)) + factor;

        return FromHsl(hue, saturation, Math.Clamp(luminance, 0, 1), colour.A);
    }

    /// <summary>Hue in degrees, saturation and luminance in 0..1.</summary>
    private static (double Hue, double Saturation, double Luminance) ToHsl(Colour colour)
    {
        double r = colour.R / 255.0;
        double g = colour.G / 255.0;
        double b = colour.B / 255.0;

        double max = Math.Max(r, Math.Max(g, b));
        double min = Math.Min(r, Math.Min(g, b));
        double luminance = (max + min) / 2;
        double chroma = max - min;

        if (chroma <= 0) return (0, 0, luminance);

        double saturation = luminance > 0.5
            ? chroma / (2 - max - min)
            : chroma / (max + min);

        double hue;
        if (max == r) hue = ((g - b) / chroma) + (g < b ? 6 : 0);
        else if (max == g) hue = ((b - r) / chroma) + 2;
        else hue = ((r - g) / chroma) + 4;

        return (hue * 60, saturation, luminance);
    }

    /// <summary>The inverse, back to eight-bit channels.</summary>
    /// <remarks>
    /// <para>
    /// <strong>Channels are rounded, not truncated, and the difference is visible.</strong> The
    /// two candidates disagree by one on both colours measured, and only rounding reproduces the
    /// reference's pixels: accent1 at 0.8 renders <c>#DAE3F3</c> where truncation gives
    /// <c>#D9E2F3</c>, and accent4 renders <c>#FFF2CC</c> where truncation gives <c>#FFF2CB</c>.
    /// </para>
    /// <para>
    /// Worth stating because the workbook itself is a false witness here. Another of its fills
    /// states <c>#D9E2F3</c> literally — the truncated value — because that is what *Excel*
    /// computed for the same accent. The reference we are matching is LibreOffice's rendering,
    /// which rounds, so the literal in the file agrees with the wrong candidate.
    /// </para>
    /// </remarks>
    private static Colour FromHsl(double hue, double saturation, double luminance, byte alpha)
    {
        if (saturation <= 0)
        {
            byte grey = Component(luminance);
            return new Colour(grey, grey, grey, alpha);
        }

        double q = luminance < 0.5
            ? luminance * (1 + saturation)
            : luminance + saturation - (luminance * saturation);
        double p = (2 * luminance) - q;

        double h = hue / 360.0;

        return new Colour(
            Component(Channel(p, q, h + (1.0 / 3.0))),
            Component(Channel(p, q, h)),
            Component(Channel(p, q, h - (1.0 / 3.0))),
            alpha);

        static double Channel(double p, double q, double t)
        {
            if (t < 0) t += 1;
            if (t > 1) t -= 1;
            if (t < 1.0 / 6.0) return p + ((q - p) * 6 * t);
            if (t < 1.0 / 2.0) return q;
            if (t < 2.0 / 3.0) return p + ((q - p) * ((2.0 / 3.0) - t) * 6);
            return p;
        }

        static byte Component(double value)
            => (byte)Math.Clamp(Math.Round(value * 255, MidpointRounding.AwayFromZero), 0, 255);
    }
}
