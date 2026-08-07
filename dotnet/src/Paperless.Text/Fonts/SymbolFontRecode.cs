namespace Paperless.Text.Fonts;

/// <summary>
/// Recodes a legacy symbol face's slot to the code point the same picture occupies in
/// <c>OpenSymbol</c>.
/// </summary>
/// <remarks>
/// <para>
/// <strong>A symbol face addresses glyphs by byte position, not by character.</strong> A document
/// states a Wingdings bullet as <c>0xD8</c> and means "the glyph in slot 0xD8", not <c>Ø</c>;
/// PowerPoint and Word both write them that way. LibreOffice first moves the byte into the
/// Private Use Area — <c>(c &amp; 0x00ff) | 0xf000</c> — which is where a symbol-encoded face
/// really maps its glyphs, and both of our presentation readers already do that much.
/// </para>
/// <para>
/// That alone draws nothing on a machine without the face, and Wingdings is not a font Linux
/// has. The step this type adds is LibreOffice's second one: recode the slot through a per-face
/// table into <c>OpenSymbol</c>, which ships with LibreOffice and is therefore always installed.
/// <c>U+F0D8</c> in Wingdings is a filled arrowhead, and the same picture is <c>U+E49E</c> in
/// OpenSymbol.
/// </para>
/// <para>
/// <strong>Asking for the Private Use Area code point directly is not a shortcut, it is tofu.</strong>
/// OpenSymbol's whole <c>F000</c>–<c>F0FF</c> coverage is ten code points — <c>F030</c>–<c>F039</c>,
/// the digits — so a shaper asked for <c>U+F0D8</c> in OpenSymbol returns <c>.notdef</c>. The
/// table is the only route from the one to the other, which is why it is ported rather than
/// approximated.
/// </para>
/// <para>
/// The tables are LibreOffice's own, read from <c>unotools/source/misc/fontcvt.cxx</c> by
/// <c>scripts/generate-symbol-recode.py</c>; the face-name list is its
/// <c>aStarSymbolRecodeTable</c> and the recode rule is <c>ConvertChar::RecodeChar</c>
/// (<c>fontcvt.cxx:1239-1273</c>), with the range guard from <c>RecodeString</c>
/// (<c>fontcvt.cxx:1277-1299</c>).
/// </para>
/// <para>
/// <strong>What this deliberately does not do is decide the extracted text.</strong> LibreOffice
/// draws the OpenSymbol glyph and writes the *original* code point into the PDF's
/// <c>ToUnicode</c>, so its text layer still reads <c>U+F0D8</c>. Both that and ours are Private
/// Use Area code points that mean nothing outside one font, so this changes what is drawn and
/// leaves what a caller extracts alone.
/// </para>
/// </remarks>
public static partial class SymbolFontRecode
{
    /// <summary>The face every table in this type targets.</summary>
    /// <remarks>
    /// LibreOffice's tables name <c>StarSymbol</c>, OpenSymbol's predecessor, and its own
    /// <c>IsOpenSymbol</c> treats the two names as one (<c>unotools/source/misc/fontdefs.cxx:408</c>).
    /// OpenSymbol is what is actually installed, so that is what is asked for.
    /// </remarks>
    public const string SubstituteFamily = "OpenSymbol";

    /// <summary>
    /// What a slot with no mapping becomes: OpenSymbol's own bullet.
    /// </summary>
    /// <remarks>
    /// <c>RecodeChar</c> forces this whenever a table entry is a hole and the substitute face is
    /// OpenSymbol or StarSymbol (<c>fontcvt.cxx:1259-1267</c>), rather than leaving the slot to
    /// draw <c>.notdef</c>. The tables carry between 1 and 79 holes each, so the case is real.
    /// </remarks>
    private const char SubstituteBullet = '\uE12C';

    /// <summary>Whether a face is one of the fourteen this type knows a table for.</summary>
    public static bool IsRecodeable(string? familyName)
        => Tables.ContainsKey(FontSubstitutions.Normalise(familyName));

    /// <summary>
    /// Whether a <em>resolved</em> face is the one these tables target.
    /// </summary>
    /// <remarks>
    /// This is the condition that makes recoding correct rather than merely available.
    /// LibreOffice only recodes when the substitution actually landed on StarSymbol or
    /// OpenSymbol (<c>ConvertChar::GetRecodeData</c>, <c>fontcvt.cxx:1345-1356</c>) — on a
    /// machine that really has Wingdings installed the Private Use Area code point is drawn
    /// from Wingdings directly and recoding it would pick the wrong glyph. Mirrors
    /// <c>IsOpenSymbol</c>, which accepts both names
    /// (<c>unotools/source/misc/fontdefs.cxx:408-413</c>).
    /// </remarks>
    public static bool IsSubstituteFamily(string? familyName)
        => FontSubstitutions.Normalise(familyName) is "opensymbol" or "starsymbol";

    /// <summary>
    /// Recodes one symbol slot, reporting whether the face and the code point were both in range.
    /// </summary>
    /// <remarks>
    /// False leaves <paramref name="recoded"/> at <paramref name="character"/> and means the
    /// caller should keep doing whatever it did before: either the face has no table, or the code
    /// point is not a symbol slot at all. The second is not hypothetical — one corpus deck states
    /// <c>U+0178</c> against an <c>a:buFont</c> of Wingdings, which is outside both the
    /// <c>0x20</c>–<c>0xFF</c> range and its <c>F020</c>–<c>F0FF</c> alias, and LibreOffice's
    /// <c>RecodeString</c> skips it for the same reason.
    /// </remarks>
    /// <param name="familyName">The face the file named for the symbol, before normalisation.</param>
    /// <param name="character">The slot, either as <c>0x20</c>–<c>0xFF</c> or aliased into
    /// <c>F020</c>–<c>F0FF</c>.</param>
    /// <param name="recoded">The OpenSymbol code point, or <paramref name="character"/> unchanged.</param>
    public static bool TryRecode(string? familyName, char character, out char recoded)
    {
        recoded = character;

        if (!Tables.TryGetValue(FontSubstitutions.Normalise(familyName), out ushort[]? table))
        {
            return false;
        }

        // RecodeString's guard: a symbol slot, or its Private Use Area alias, and nothing else.
        bool plain = character is >= '\u0020' and <= '\u00FF';
        bool aliased = character is >= '\uF020' and <= '\uF0FF';
        if (!plain && !aliased) return false;

        int slot = (aliased ? character - 0xF000 : character) - 0x20;
        ushort value = table[slot];

        recoded = value != 0 ? (char)value : SubstituteBullet;
        return true;
    }
}
