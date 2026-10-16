using XRay.Office.Core.Graphics;

namespace XRay.Office.Ooxml.DrawingML;

/// <summary>
/// Turns the picture effects <see cref="DrawingFill.ReadBlip"/> parses into the forms the
/// graphics layer carries, for the parts of that resolution that are the same in every family.
/// </summary>
/// <remarks>
/// <para>
/// <see cref="DrawingFill"/> is deliberately parsing only, and a <c>a:clrChange</c> cannot be
/// resolved there: it needs a theme to turn its colours into <see cref="Colour"/>s and the
/// image's own bytes to choose a tolerance. Both belong to the caller, so this sits beside the
/// parser rather than inside it — the same split <see cref="DrawingColour"/> already makes.
/// </para>
/// </remarks>
public static class DrawingPictureEffects
{
    /// <summary>
    /// The <see cref="ColourKnockout"/> a blip's <c>a:clrChange</c> asks for, or null when it
    /// states none, states one this cannot carry out, or asks for nothing.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>Only the knockout case is carried out.</strong> DrawingML's <c>a:clrChange</c> is
    /// a general "replace colour A with colour B at alpha C", and the reference implements the
    /// general case (<c>XGraphicTransformer::colorChange</c>). <see cref="ColourKnockout"/>
    /// carries only "make colour A fully transparent", which is the <c>C == 0</c> corner of it.
    /// </para>
    /// <para>
    /// That is a deliberate limit rather than an oversight, and it is sized from what the
    /// corpus <em>resolves to</em> rather than from what the schema permits: all
    /// <strong>93</strong> occurrences across all 944 zip-container corpus documents are
    /// <c>from == to</c> with <c>&lt;a:alpha val="0"/&gt;</c> — a pure knockout — and every one
    /// states its colours as <c>a:srgbClr</c>. A general recolour would need a new transform on
    /// <see cref="RasterImage"/> and a decoder that applies it, to serve zero documents. When a
    /// document does state one, this returns null and the picture is drawn as stored, which is
    /// the same thing we did before this method existed.
    /// </para>
    /// <para>
    /// <strong>Equal colours are not a no-op</strong>, and that is the whole point:
    /// <c>fillproperties.cxx</c>:240 applies the transform when the colours differ
    /// <em>or</em> the destination is transparent. Every corpus occurrence takes the second
    /// branch, so a reader that short-circuits on <c>from == to</c> implements exactly nothing
    /// while appearing to handle the element.
    /// </para>
    /// </remarks>
    /// <param name="blip">The parsed blip fill.</param>
    /// <param name="theme">The theme its colours resolve against, or null.</param>
    /// <param name="encoded">
    /// The image exactly as the file stored it. Both the tolerance and whether the knockout
    /// happens at all are decided from it.
    /// </param>
    public static ColourKnockout? Knockout(
        DrawingBlipFill? blip, DrawingTheme? theme, ReadOnlySpan<byte> encoded)
    {
        if (blip?.ColourChange is not { } change) return null;
        if (!change.UseAlpha) return null;
        // Not named `from`: it is a LINQ contextual keyword, and `from with { ... }` parses as
        // the start of a query expression.
        if (change.From.Resolve(theme, placeholder: null) is not { } matched) return null;
        if (change.To.Resolve(theme, placeholder: null) is not { } destination) return null;

        // The destination's opacity is an ordinary a:alpha transform inside the colour, applied
        // by DrawingColourTransforms. Reading the attribute again here would be a second
        // measurement of one value under one name.
        //
        // Anything short of fully transparent is a recolour, which ColourKnockout cannot carry:
        // it makes a matched pixel transparent and nothing else. Returning null then draws the
        // picture as stored, which is what we did before this method existed. The corpus states
        // zero of them -- all 93 occurrences are a knockout.
        if (destination.A != 0) return null;

        // A picture that already carries an alpha channel is NOT knocked out, and this is
        // measured rather than reasoned. `Graphic::colorChange` branches on
        // `aBitmap.HasAlpha()` (`vcl/source/graphic/UnoGraphic.cxx`:188-208): an alpha-bearing
        // bitmap takes `ChangeColorAlpha` and only a bitmap WITHOUT alpha reaches the
        // `CreateAlphaMask(aColorFrom, nTolerance)` branch that is the knockout.
        //
        // Confirmed against the installed 26.2.4.2 on two authored one-shape decks differing in
        // exactly one thing -- the same pixels and the same clrChange, saved once as an RGB PNG
        // and once as RGBA (`probes/slides-r51/probe-alpha/`). The RGB deck renders the colour
        // knocked out; the RGBA deck renders it untouched.
        //
        // Skipping this cost `vv_summit_SAIC-PRESENTATION*.pptx` page 13 its exact match: it is
        // an RGBA PNG that is 66.1% F4F4F4, and knocking that out where the reference does not
        // took the page from 0.00 to 0.28 unaccounted ink.
        if (HasAlphaChannel(encoded)) return null;

        return new ColourKnockout(matched with { A = 255 }, ToleranceFor(encoded));
    }

    /// <summary>
    /// Whether the stored image carries an alpha channel, which decides whether a
    /// <c>a:clrChange</c> knockout happens at all.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Only PNG is inspected, and that is sized from the corpus rather than from the format
    /// list: every alpha-bearing picture under a <c>clrChange</c> in all 944 corpus documents is
    /// a PNG, and the two formats that carry the rest — JPEG and BMP as stored here — have no
    /// alpha channel at all. A GIF's index transparency and a 32-bit BMP's alpha are not
    /// detected; both would be knocked out where the reference would not, and neither occurs.
    /// </para>
    /// <para>
    /// PNG states it in <c>IHDR</c>'s colour-type byte — 4 is grey+alpha and 6 is RGBA — and a
    /// palette or truecolour image can also carry a <c>tRNS</c> chunk. <c>IHDR</c> is required
    /// to be the first chunk, so the colour type is at a fixed offset; <c>tRNS</c> is searched
    /// for among the chunk headers up to the first <c>IDAT</c>, after which no <c>tRNS</c> may
    /// legally appear.
    /// </para>
    /// </remarks>
    /// <param name="encoded">The image exactly as the file stored it.</param>
    public static bool HasAlphaChannel(ReadOnlySpan<byte> encoded)
    {
        if (encoded.Length < 26) return false;
        if (encoded[0] != 0x89 || encoded[1] != 'P' || encoded[2] != 'N' || encoded[3] != 'G')
        {
            return false;
        }

        // 8 signature + 4 length + 4 "IHDR" + 4 width + 4 height + 1 bit depth = 25.
        byte colourType = encoded[25];
        if (colourType is 4 or 6) return true;

        int position = 8;
        while (position + 8 <= encoded.Length)
        {
            uint length = ((uint)encoded[position] << 24) | ((uint)encoded[position + 1] << 16)
                          | ((uint)encoded[position + 2] << 8) | encoded[position + 3];
            ReadOnlySpan<byte> type = encoded.Slice(position + 4, 4);

            if (type is [(byte)'t', (byte)'R', (byte)'N', (byte)'S']) return true;
            if (type is [(byte)'I', (byte)'D', (byte)'A', (byte)'T']) return false;

            // 4 length + 4 type + data + 4 CRC. Guarded against a length that would wrap or
            // run past the buffer, which a truncated or hostile file can state.
            if (length > int.MaxValue - 12 || position + 12 + (int)length <= position) return false;
            position += 12 + (int)length;
        }

        return false;
    }

    /// <summary>
    /// The per-channel tolerance the reference matches <c>a:clrFrom</c> with, chosen by the
    /// image's stored format.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>It is not one number, and using one is visibly wrong.</strong>
    /// <c>lclCheckAndApplyChangeColorTransform</c> starts at 9 and then overrides it from
    /// <c>GfxLink::GetType()</c> (<c>oox/source/drawingml/fillproperties.cxx</c>:245-264, the
    /// fix for tdf#149670): PNG and TIFF take <strong>1</strong>, JPEG <strong>15</strong>, BMP
    /// <strong>0</strong>, and anything else keeps 9.
    /// </para>
    /// <para>
    /// The reasoning behind those numbers is the codec, not the format: a lossy JPEG smears a
    /// flat background into a cloud of near-matches, so knocking out only the exact value leaves
    /// a halo — hence 15. A lossless PNG stores the flat colour exactly, so a wide tolerance
    /// eats real picture content — hence 1. Note this is a *different* number from the 9 the
    /// binary Escher path passes (<see cref="ColourKnockout.DefaultTolerance"/>), which is
    /// correct: that call site is <c>msdffimp.cxx</c>'s and genuinely does pass 9.
    /// </para>
    /// <para>
    /// Sniffed from the bytes rather than taken from the part's declared media type, because
    /// office files mislabel images as routinely as they mislabel themselves — the same reason
    /// <see cref="RasterImage"/> calls its own media type a hint.
    /// </para>
    /// </remarks>
    /// <param name="encoded">The image exactly as the file stored it.</param>
    public static int ToleranceFor(ReadOnlySpan<byte> encoded)
    {
        if (encoded.Length >= 8
            && encoded[0] == 0x89 && encoded[1] == 'P' && encoded[2] == 'N' && encoded[3] == 'G')
        {
            return 1;
        }

        if (encoded.Length >= 3
            && encoded[0] == 0xFF && encoded[1] == 0xD8 && encoded[2] == 0xFF)
        {
            return 15;
        }

        if (encoded.Length >= 2 && encoded[0] == 'B' && encoded[1] == 'M') return 0;

        // "II*\0" and "MM\0*" — TIFF little- and big-endian.
        if (encoded.Length >= 4
            && ((encoded[0] == 'I' && encoded[1] == 'I' && encoded[2] == 0x2A && encoded[3] == 0)
                || (encoded[0] == 'M' && encoded[1] == 'M' && encoded[2] == 0 && encoded[3] == 0x2A)))
        {
            return 1;
        }

        return ColourKnockout.DefaultTolerance;
    }
}
