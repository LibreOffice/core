using XRay.Office.Text.Fonts;

namespace XRay.Office.Spreadsheets.Layout;

/// <summary>
/// Recodes a cell's text when the face it asked for is a legacy pi font that is not installed and
/// the substitution has landed on OpenSymbol.
/// </summary>
/// <remarks>
/// <para>
/// <strong>This is a device-level rule in LibreOffice, not a filter one, which is why a
/// spreadsheet gets it too.</strong> <c>ImplFontCache::GetFontInstance</c> attaches a
/// <c>ConvertChar</c> to the logical font instance whenever the resolved face is OpenSymbol or
/// StarSymbol and the requested name differs from it
/// (<c>vcl/source/font/fontcache.cxx</c>:165-169), and <c>OutputDevice::ImplLayout</c> then
/// rewrites every string drawn through that instance
/// (<c>vcl/source/outdev/text.cxx</c>:1157-1158). Nothing about that is specific to Writer or
/// Impress, so a Calc cell set in <c>Symbol</c> is recoded on exactly the same terms — and
/// without this the cell asks OpenSymbol for a code point it does not hold, which is
/// <c>.notdef</c>, and glyph fallback then draws the character in DejaVu Sans instead of the
/// picture the file asked for.
/// </para>
/// <para>
/// Measured on 26.2.4.2, three corpus workbooks and three glyphs:
/// <c>REDAC_SCHEDULE_RPD_135.xls</c> and <c>REDAC_SCHEDULE_RPD_137.xls</c> each draw
/// <c>U+00C4</c> — <c>Symbol</c>'s circled times — from OpenSymbol at <c>U+E136</c> where we drew
/// it from DejaVu Sans, and <c>021_Control_Chart_Template…xlsx</c> draws <c>s</c>, which is
/// <c>Symbol</c>'s sigma, from OpenSymbol at <c>U+03C3</c>.
/// </para>
/// <para>
/// <strong>Only <c>Symbol</c> itself, and that is the narrow reading on purpose.</strong> A cell's
/// format carries no character set here, so every request this path makes is one that fontconfig
/// gets to answer — and fontconfig does not know a name was meant as a symbol font. It answers
/// <c>Wingdings</c> with DejaVu Sans and LibreOffice draws the slot from DejaVu Sans; <c>Symbol</c>
/// is the one family it binds to OpenSymbol by name, through <c>30-opensymbol.conf</c>, which
/// ships with the font. That is the same split
/// <see cref="XRay.Office.Text.Fonts.SymbolFontRecode.IsAliasedToSubstitute"/> already draws for the
/// slide path's non-symbol-encoded arm, and it is used here rather than restated.
/// </para>
/// </remarks>
internal static class SheetSymbolText
{
    /// <summary>
    /// Whether text set in this face has to be recoded before it is shaped.
    /// </summary>
    /// <param name="face">The resolved face, carrying both the name asked for and the name got.</param>
    internal static bool Recodes(SheetFace face)
        => SymbolFontRecode.IsRecodeable(face.Reference.RequestedFamily)
           && SymbolFontRecode.IsAliasedToSubstitute(face.Reference.RequestedFamily)
           && SymbolFontRecode.IsSubstituteFamily(face.Reference.FamilyName);

    /// <summary>
    /// The string as it is actually drawn, or the same instance when nothing is recoded.
    /// </summary>
    /// <remarks>
    /// One code point for one code point — <c>ConvertChar::RecodeChar</c> is a table lookup, not a
    /// mapping — so the result has the requested string's length and every index into it still
    /// means what it did. That is what lets the caller shape this and keep the original as the
    /// segment's text: the cluster map lines up, and the PDF's <c>ToUnicode</c> then carries the
    /// character the document holds, which is what the reference writes too.
    /// </remarks>
    /// <param name="text">The cell's own characters.</param>
    /// <param name="face">The face they are set in.</param>
    internal static string Recode(string text, SheetFace face)
    {
        if (text.Length == 0 || !Recodes(face)) return text;

        string? requested = face.Reference.RequestedFamily;
        char[]? rewritten = null;

        for (int index = 0; index < text.Length; index++)
        {
            if (!SymbolFontRecode.TryRecode(requested, text[index], out char recoded)) continue;
            if (recoded == text[index]) continue;

            rewritten ??= text.ToCharArray();
            rewritten[index] = recoded;
        }

        return rewritten is null ? text : new string(rewritten);
    }
}
