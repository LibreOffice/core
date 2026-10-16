using XRay.Office.Core.Graphics;

namespace XRay.Office.MsBinary.Escher;

/// <summary>
/// An Escher <c>MSO_CLR</c>: four bytes that are a literal colour, an index into the host's
/// palette, or a system colour.
/// </summary>
/// <remarks>
/// <para>
/// <strong>A literal colour is the rare case, not the common one.</strong> The top byte decides
/// how the other three are read, and reading only the literal form — which is what this tree did
/// until round 92 — resolves <strong>14 of the 106</strong> fill and line colours the corpus's
/// 64 <c>.xls</c> state on a worksheet shape. The other 92 are palette indices, so a reader that
/// insists on the literal form draws no fill and no outline on nearly every legacy worksheet
/// shape there is. <c>probes/sheet-shapefill-r92/msoclr-census.txt</c>.
/// </para>
/// <para>
/// This is <c>SvxMSDffManager::MSO_CLR_ToColor</c>
/// (<c>filter/source/msfilter/msdffimp.cxx</c>:3420-3641), whose branches are, in order: a
/// <c>0xfe</c> header, which PowerPoint uses for text and which means "the low three bytes are
/// the colour"; the scheme and system forms, selected together by <c>nUpper &amp; 0x19</c>; a
/// second scheme form for a bare <c>nUpper &amp; 4</c> with no low bits; and, failing all of
/// those, a literal <c>0x00BBGGRR</c>.
/// </para>
/// <para>
/// <strong>The palette is the host's and cannot be reached from here</strong> — Excel's is the
/// workbook's <c>PALETTE</c> record over the BIFF8 defaults, PowerPoint's the slide's colour
/// scheme — so it arrives as a callback. A host that has none passes null and every scheme
/// reference falls back, which is what <c>SvxMSDffManager::GetColorFromPalette</c>'s own base
/// implementation does (<c>:3388</c>, it answers false).
/// </para>
/// <para>
/// <strong>The system-colour branch is deliberately not implemented.</strong> It is twenty
/// desktop-theme colours and six recursive "use this shape's other colour" forms, and its answer
/// depends on the machine's widget theme rather than on the file — the reference reads
/// <c>Application::GetSettings()</c> for it. No worksheet shape in the corpus states one
/// (measured: zero of 106), so it falls back with everything else rather than inventing a theme.
/// </para>
/// </remarks>
public static class EscherColour
{
    /// <summary>
    /// The colour a <c>MSO_CLR</c> names.
    /// </summary>
    /// <param name="value">The four bytes as the property table holds them.</param>
    /// <param name="fallback">
    /// What an unresolvable scheme or system reference means. The reference's own answer depends
    /// on which property is being read — white for a fill, a fill background, a shadow and a
    /// picture's knockout colour, black for a line (<c>msdffimp.cxx</c>:3440-3453) — so the
    /// caller states it rather than this deciding from a property identifier it was not given.
    /// </param>
    /// <param name="scheme">
    /// Resolves a palette index; null when the host has no palette. Answering null for an index
    /// it does not know is what makes <paramref name="fallback"/> apply.
    /// </param>
    public static Colour Resolve(uint value, Colour fallback, Func<int, Colour?>? scheme)
    {
        // PowerPoint writes a text colour as 0xfeRRGGBB. The reference's own comment doubts the
        // header is used for anything else, and masks it off before looking at the top byte.
        if ((value & 0xFE000000u) == 0xFE000000u) value &= 0x00FFFFFFu;

        uint upper = value >> 24;

        if ((upper & 0x19u) != 0)
        {
            // Scheme when the fSchemeIndex bit is set, or when fSysIndex is not; the index is the
            // low *word* under fSchemeIndex and the top byte itself otherwise.
            if ((upper & 0x08u) != 0 || (upper & 0x10u) == 0)
            {
                int index = (upper & 0x08u) != 0 ? (int)(value & 0xFFFFu) : (int)upper;
                return scheme?.Invoke(index) ?? fallback;
            }

            // A system colour. See the remarks: not implemented, and not reached by the corpus.
            return fallback;
        }

        // "case of nUpper == 4 powerpoint takes this as argument for a colorschemecolor".
        if ((upper & 0x04u) != 0 && (value & 0x00FFFFF8u) == 0)
        {
            return scheme?.Invoke((int)upper) ?? fallback;
        }

        return Literal(value);
    }

    /// <summary>The literal form, which stores the channels in <c>0x00BBGGRR</c> order.</summary>
    public static Colour Literal(uint value)
        => Colour.FromRgb(
            ((value & 0x000000FFu) << 16) | (value & 0x0000FF00u) | ((value & 0x00FF0000u) >> 16));
}
