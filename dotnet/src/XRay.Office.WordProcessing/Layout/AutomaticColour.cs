using XRay.Office.Core.Graphics;

namespace XRay.Office.WordProcessing.Layout;

/// <summary>
/// What an <em>automatic</em> font colour resolves to over a background — including the step every
/// previous round of this project left out, which is what the background's own transparency does.
/// </summary>
/// <remarks>
/// <para>
/// <c>SwDrawTextInfo::ApplyAutoColor</c> (<c>sw/source/core/txtnode/fntcache.cxx</c>:2369) asks the
/// frame chain for a background and answers <c>COL_WHITE</c> when it is dark and <c>COL_BLACK</c>
/// otherwise. With no background at all it falls back to the application's document colour, which
/// is white — so a transparent background here means black.
/// </para>
/// <para>
/// <strong>It does not ask the fill for its colour. It asks
/// <c>SdrAllFillAttributesHelper::getAverageColor(aGlobalRetoucheColor)</c></strong>, which
/// interpolates the fill toward the application's retouche colour — white — by the fill's
/// transparency, and only then is <see cref="Colour.IsDark"/> asked. A 40 %-opaque navy box is a
/// pale blue as far as this question is concerned, and its text stays black.
/// </para>
/// <para>
/// <strong>That one term reconciles two rounds that contradicted each other.</strong> Round 59
/// measured two documents whose reference draws black text on a fill that is dark by every rule
/// this code knows — <c>docs-quality-MA.IMS.00001-…docx</c> at <c>#0070C0</c>, WCAG luminance 39,
/// and <c>069_Work_Breakdown_Structure_Template_Professional_Format</c> at <c>#8496B0</c>, WCAG 76
/// — and concluded that such a shape's text must be drawn by editeng and never reach this function
/// at all; the arm was removed. Round 62 then established on four inverted arms of <c>012</c> that
/// a text box's own fill <em>does</em> decide, and shipped nothing because of the contradiction.
/// Both were right about their measurements. <b>Both witnesses state a transparency</b> —
/// <c>&lt;a:alpha val="52941"/&gt;</c> and <c>&lt;v:fill opacity="26214f"/&gt;</c> — and blended,
/// they are luminance 106 and 172, which is bright.
/// </para>
/// <para>
/// The blend is pinned rather than assumed, on three fill colours at once
/// (<c>probes/words-r63/threshold.py</c>): it predicts a <em>different</em> flip transparency for
/// every colour, with no free parameter, and eleven renderings against 26.2.4.2 land on all three —
/// <c>#8496B0</c> at 9.571 %, <c>#0070C0</c> at 37.454 %, <c>#000000</c> at 62.222 %. A constant
/// threshold and a threshold that ignores the colour are refuted by the same eleven.
/// </para>
/// <para>
/// The thresholds are where they are because <c>Color::GetWCAGLuminance</c> returns a
/// <c>sal_uInt8</c>: the comparison is against the <em>truncated</em> value, so the flip is where
/// the blend first reaches 88.0 and not 87.0. The probe's own first cut bisected on the continuous
/// value and mispredicted the one arm that sits between the two readings, which is how that was
/// found.
/// </para>
/// </remarks>
internal static class AutomaticColour
{
    /// <summary>The colour an automatic run is drawn in over <paramref name="background"/>.</summary>
    /// <param name="background">The brush behind the run, or transparent for none.</param>
    public static Colour Over(Colour background)
        => background.A != 0 && Averaged(background).IsDark ? Colour.White : Colour.Black;

    /// <summary>
    /// A background blended toward white by its own transparency —
    /// <c>SdrAllFillAttributesHelper::getAverageColor</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The fallback colour is <c>aGlobalRetoucheColor</c>, the application's document background,
    /// which is white in every configuration this project renders under; it is not the page's own
    /// fill, and a coloured page does not change this answer.
    /// </para>
    /// <para>
    /// Blended in gamma-encoded byte space rather than in linear light, because
    /// <c>basegfx::interpolate</c> works on the <c>BColor</c> channels as they are, and rounded
    /// half up, which is what <c>Color</c>'s <c>BColor</c> constructor does. Both choices are
    /// measurable and both are confirmed by the eleven-arm bracket: blending in linear light would
    /// move every predicted flip by several points.
    /// </para>
    /// </remarks>
    /// <param name="background">The brush behind the run.</param>
    public static Colour Averaged(Colour background)
    {
        if (background.A == byte.MaxValue) return background;

        double transparency = (byte.MaxValue - background.A) / 255.0;

        return new Colour(
            Towards(background.R, transparency),
            Towards(background.G, transparency),
            Towards(background.B, transparency));

        static byte Towards(byte channel, double transparency)
            => (byte)Math.Clamp(
                Math.Floor(channel + ((255.0 - channel) * transparency) + 0.5), 0.0, 255.0);
    }
}
