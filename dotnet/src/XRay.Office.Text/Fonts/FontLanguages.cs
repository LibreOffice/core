namespace XRay.Office.Text.Fonts;

/// <summary>
/// Whether a face can be said to support a language, as far as glyph fallback needs to know.
/// </summary>
/// <remarks>
/// <para>
/// <strong>fontconfig scores the language above the family list, and that is what decides a
/// complex-script or East Asian glyph fallback.</strong> <c>FontConfigManager::Substitute</c> puts
/// the run's language in the pattern as <c>FC_LANG</c>
/// (<c>vcl/unx/generic/font/fontconfig.cxx</c>:1092, 1118-1119) and <c>fcmatch.c</c> ranks
/// <c>PRI_LANG</c> above <c>PRI_FAMILY_WEAK</c> — so among the faces that cover the character, the
/// ones that support the language come first and only then does the generic's <c>&lt;prefer&gt;</c>
/// list break the tie. Measured on this machine: <c>fc-match "Calibri:charset=5d0"</c> answers
/// DejaVu Sans, <c>fc-match "Calibri:lang=hi:charset=5d0"</c> answers <b>FreeSans</b>, and
/// LibreOffice 26.2.4.2 draws a Hebrew character in a complex-script run in FreeSans — because
/// Writer's default CTL language is Hindi.
/// </para>
/// <para>
/// <strong>This is a model of <c>FcCompareLang</c> and not a reimplementation of it.</strong>
/// fontconfig derives a face's language set from its character coverage against an orthography per
/// language, and that data is compiled into the library rather than published in the configuration
/// this tree reads. What is asked here instead is whether the face covers the language's
/// <em>script</em>, keyed by one exemplar character. Checked against <c>fc-list :lang=X</c> on this
/// machine over the 25 languages below, comparing the families each answers: <b>24 agree exactly</b>,
/// face for face. The twenty-fifth, Gurmukhi, differs by naming two <em>fewer</em> — FreeMono, which
/// fontconfig files under <c>pa</c> although it holds none of the Gurmukhi letters checked here. Two
/// exemplars are deliberately not the first letter of their alphabet, because the first letter does
/// not discriminate: an accented Greek vowel excludes a face carrying only the mathematical Greek,
/// and a simplified-only Chinese ideograph excludes a Japanese face carrying only the shared ones.
/// </para>
/// <para>
/// <strong>A language whose script is Latin is deliberately not modelled, and that is the safe
/// direction.</strong> Every text face on a Linux machine covers Latin, so <c>en</c> ranks nothing
/// and modelling it could only ever demote a face fontconfig would have kept. Unlisted languages
/// answer <see cref="Neutral"/>, which ranks every face alike and leaves the generic's preference
/// list to decide — exactly what this resolver did before languages were carried at all.
/// </para>
/// <para>
/// The table is the inverse of LibreOffice's own <c>getExemplarLanguageForUScriptCode</c>
/// (<c>i18nutil/source/utility/unicode.cxx</c>:428ff), which maps a script onto the language
/// fontconfig should be asked about for it; read the other way it says which script a language
/// implies. The exemplar is the first letter of that script's alphabet.
/// </para>
/// </remarks>
public static class FontLanguages
{
    /// <summary>No character to check: every face ranks alike.</summary>
    public const int Neutral = -1;

    /// <summary>
    /// A character a face must have to support a language, or <see cref="Neutral"/>.
    /// </summary>
    /// <param name="languageTag">
    /// A BCP 47 tag such as <c>hi-IN</c>. Only the primary subtag is read, which is what
    /// <c>mapToFontConfigLangTag</c> ends at for every tag fontconfig does not know as a whole —
    /// <c>hi-IN</c> is not in <c>FcGetLangs()</c> here and <c>hi</c> is, so the pattern LibreOffice
    /// builds says <c>hi</c>.
    /// </param>
    public static int ExemplarOf(string? languageTag)
    {
        if (string.IsNullOrWhiteSpace(languageTag)) return Neutral;

        string tag = languageTag.Replace('_', '-').ToLowerInvariant();
        int end = tag.IndexOf('-', StringComparison.Ordinal);
        string primary = end < 0 ? tag : tag[..end];

        // Chinese is the one language whose two orthographies genuinely separate the installed
        // faces, so it is the one that reads past its primary subtag. A simplified-only character
        // is what excludes a Japanese face that covers the shared ideographs and is filed under
        // `ja` alone; `zh` unqualified keeps the shared one, which excludes nothing.
        if (primary == "zh")
        {
            return tag.Contains("-cn", StringComparison.Ordinal)
                   || tag.Contains("-sg", StringComparison.Ordinal)
                   || tag.Contains("-hans", StringComparison.Ordinal)
                ? 0x8FD9
                : 0x4E00;
        }

        return primary switch
        {
            // Arabic script: Arabic, Persian, Urdu, Pashto, Sindhi, Uyghur, Kurdish (Sorani).
            "ar" or "fa" or "ur" or "ps" or "sd" or "ug" or "ckb" => 0x0627,
            "hy" => 0x0561,                       // Armenian
            "bn" or "as" => 0x0985,               // Bengali
            "yue" => 0x4E00,                      // Han
            "chr" => 0x13A0,                      // Cherokee
            "ru" or "uk" or "bg" or "sr" or "be" or "mk" or "kk" or "ky" or "mn" => 0x0430, // Cyrillic
            "hi" or "mr" or "ne" or "sa" or "kok" => 0x0905, // Devanagari
            "am" or "ti" => 0x1200,               // Ethiopic
            "ka" => 0x10D0,                       // Georgian
            "el" => 0x03AC,                       // Greek, accented so that a face carrying only the
                                                  // shared mathematical letters does not qualify
            "gu" => 0x0A85,                       // Gujarati
            "pa" => 0x0A05,                       // Gurmukhi
            "ko" => 0xAC00,                       // Hangul
            "he" or "yi" => 0x05D0,               // Hebrew
            "ja" => 0x3042,                       // Hiragana
            "kn" => 0x0C85,                       // Kannada
            "km" => 0x1780,                       // Khmer
            "lo" => 0x0EC1,                       // Lao
            "ml" => 0x0D05,                       // Malayalam
            "my" => 0x1000,                       // Myanmar
            "or" => 0x0B05,                       // Oriya
            "si" => 0x0D85,                       // Sinhala
            "syr" => 0x0710,                      // Syriac
            "ta" => 0x0B85,                       // Tamil
            "te" => 0x0C05,                       // Telugu
            "dv" => 0x0780,                       // Thaana
            "th" => 0x0E01,                       // Thai
            "bo" or "dz" => 0x0F40,               // Tibetan

            // Every Latin-script language, and every tag this table does not name.
            _ => Neutral,
        };
    }
}
