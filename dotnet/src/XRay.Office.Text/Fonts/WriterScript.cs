namespace XRay.Office.Text.Fonts;

/// <summary>
/// Which of a word processor's three character-font items a stretch of text is set from.
/// </summary>
/// <remarks>
/// Writer keeps <c>RES_CHRATR_FONT</c>, <c>RES_CHRATR_CJK_FONT</c> and <c>RES_CHRATR_CTL_FONT</c>
/// side by side and selects one per script item of the text —
/// <c>SwScriptInfo::WhichFont</c> maps <c>i18n::ScriptType</c> onto <c>SwFontScript</c>
/// (<c>sw/source/core/text/porlay.cxx</c>:879-901). The three carry their own family, their own
/// family <em>class</em> and their own <em>language</em>, and all three reach the font matcher.
/// </remarks>
public enum WriterScript
{
    /// <summary>The western item, <c>RES_CHRATR_FONT</c>.</summary>
    Western,

    /// <summary>The East Asian item, <c>RES_CHRATR_CJK_FONT</c>.</summary>
    Asian,

    /// <summary>The complex-script item, <c>RES_CHRATR_CTL_FONT</c>.</summary>
    Complex,
}

/// <summary>
/// What a document says an ambiguous character's script is, when it says anything.
/// </summary>
/// <remarks>
/// <c>w:rFonts/@w:hint</c> in a DOCX, which <c>DomainMapper::lcl_attribute</c> turns into
/// <c>PROP_CHAR_SCRIPT_HINT</c> (<c>sw/source/writerfilter/dmapper/DomainMapper.cxx</c>:969-988)
/// and Writer carries as <c>RES_CHRATR_SCRIPT_HINT</c>. It applies to <em>weak</em> characters
/// only: <c>GreedyScriptChangeScanner::AdvanceOnce</c> consults it where
/// <c>GetScriptClass</c> answered <c>WEAK</c> and nowhere else
/// (<c>i18nutil/source/utility/scriptchangescanner.cxx</c>:246-268).
/// </remarks>
public enum WriterScriptHint
{
    /// <summary>Nothing stated, or <c>w:hint="default"</c>.</summary>
    Automatic,

    /// <summary><c>w:hint="eastAsia"</c>.</summary>
    Asian,

    /// <summary><c>w:hint="cs"</c>.</summary>
    Complex,
}

/// <summary>
/// The script class a character belongs to, in Writer's four-way division.
/// </summary>
public enum WriterScriptClass
{
    /// <summary>Takes its script from the text around it.</summary>
    Weak,

    /// <summary>Latin, Greek, Cyrillic and the rest of the western scripts.</summary>
    Latin,

    /// <summary>Han, Kana, Hangul and the other East Asian scripts.</summary>
    Asian,

    /// <summary>The complex scripts: Hebrew, Arabic, the Indic family, Thai and their neighbours.</summary>
    Complex,
}

/// <summary>
/// Which font item a run's text selects, and what that item carries when the document is silent.
/// </summary>
/// <remarks>
/// <para>
/// <strong>The item decides the glyph-fallback answer, and it does so through two properties the
/// western item does not share.</strong> Measured on LibreOffice 26.2.4.2 with one DOCX per cell
/// and the drawn face read out of the PDF (<c>probes/fonts-r65/gen-scriptitem.py</c>), the family
/// always <c>Calibri</c> so that every cell falls back:
/// </para>
/// <list type="table">
/// <listheader><term>run</term><description>face 26.2.4.2 drew</description></listheader>
/// <item><term>western, <c>U+2610</c></term><description>FreeSerif — roman default, DejaVu Sans when the table declares <c>swiss</c></description></item>
/// <item><term><c>w:hint="eastAsia"</c>, <c>U+2610</c> or <c>U+2713</c></term><description><b>Unifont</b>, and the declared class does not move it</description></item>
/// <item><term>complex, <c>U+05D0</c> or a <c>w:hint="cs"</c> <c>U+2610</c></term><description><b>FreeSans</b>, and the declared class does not move it</description></item>
/// <item><term>complex, <c>U+0E01</c> or <c>U+0627</c></term><description><b>FreeSerif</b></description></item>
/// </list>
/// <para>
/// <strong>First: the class never reaches the CJK or CTL item.</strong> <c>LN_CT_Fonts_eastAsia</c>
/// and <c>LN_CT_Fonts_cs</c> insert the font <em>name</em> and never
/// <c>PROP_CHAR_FONT_FAMILY</c> — only <c>LN_CT_Fonts_ascii</c> does
/// (<c>sw/source/writerfilter/dmapper/DomainMapper.cxx</c>:436-508) — so those two items keep the
/// pool default's family type, and <c>OutputDevice::GetDefaultFont</c> sets
/// <c>FAMILY_SYSTEM</c> for <c>CJK_TEXT</c> and <c>CTL_TEXT</c> with the comment <em>"don't care,
/// but don't use font subst config later"</em> (<c>vcl/source/outdev/font.cxx</c>). A
/// <c>FAMILY_SYSTEM</c> pattern gets no generic appended by
/// <c>FontConfigManager::Substitute</c>'s switch, which is the same switch that appends
/// <c>serif</c> for <c>FAMILY_ROMAN</c> (<c>vcl/unx/generic/font/fontconfig.cxx</c>:1075-1088).
/// </para>
/// <para>
/// <strong>Second, and it is the half that decides the answer: each item carries its own
/// language.</strong> <c>SwDoc::SwDoc</c> resolves the document's three default languages through
/// <c>MsLangId::resolveSystemLanguageByScriptType</c>
/// (<c>sw/source/core/doc/docnew.cxx</c>:383-398), which answers <c>LANGUAGE_ENGLISH_US</c> for
/// the western item, <c>LANGUAGE_CHINESE_SIMPLIFIED</c> for the Asian one and
/// <c>LANGUAGE_HINDI</c> for the complex one
/// (<c>i18nlangtag/source/isolang/mslangid.cxx</c>:135-165). <c>Substitute</c> puts that language
/// in the pattern as <c>FC_LANG</c>, and fontconfig scores <c>PRI_LANG</c> <em>above</em>
/// <c>PRI_FAMILY_WEAK</c> — so the language outranks the generic's preference list. That is the
/// whole of the Unifont and FreeSans rows above: <c>fc-match "Calibri:lang=zh-cn:charset=2610"</c>
/// answers Unifont and <c>fc-match "Calibri:lang=hi:charset=5d0"</c> answers FreeSans, while the
/// same patterns without a language answer DejaVu Sans.
/// </para>
/// <para>
/// A document that states <c>w:lang</c> overrides those defaults, and Word writes one into
/// <c>docDefaults</c> for nearly every file — <c>&lt;w:lang w:val="en-US" w:eastAsia="en-US"
/// w:bidi="ar-SA"/&gt;</c> is what both corpus witnesses carry, which is why their East Asian runs
/// answer DejaVu Sans rather than Unifont.
/// </para>
/// </remarks>
public static class WriterScripts
{
    /// <summary>
    /// The script class of one character, in Writer's four-way division.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Ported from <c>getCompatibilityScriptClassByBlock</c>
    /// (<c>i18nutil/source/utility/scriptclass.cxx</c>:56-127), whose table is expressed in ICU
    /// block codes; the ranges below are those blocks' code points.
    /// </para>
    /// <para>
    /// <strong>Where that table answers "unknown", LibreOffice defers to the character's UAX #24
    /// script and this answers <see cref="WriterScriptClass.Weak"/>.</strong> The deferral is not
    /// available here — .NET exposes no script property — but the two agree on everything the
    /// table does not name that a document is likely to hold: <c>USCRIPT_COMMON</c>,
    /// <c>USCRIPT_INHERITED</c>, <c>USCRIPT_SYMBOLS</c> and <c>USCRIPT_UNKNOWN</c> all answer
    /// <c>WEAK</c> in <c>getScriptClassFromUScriptCode</c>
    /// (<c>i18nutil/source/utility/unicode.cxx</c>), and that is every symbol, arrow, dingbat and
    /// punctuation mark — which is the whole of the region the table leaves out below
    /// <c>U+2E80</c>. The scripts that would differ are ones added to Unicode after the block
    /// table was written, and a weak answer for one of them puts it on the item the run's other
    /// characters chose rather than on a wrong one.
    /// </para>
    /// </remarks>
    public static WriterScriptClass ClassOf(int codePoint)
    {
        // The named exceptions, which the C++ tests before it looks at the block at all.
        switch (codePoint)
        {
            // #102975: a western space and a non-breaking space are weak; 0x01 and 0x02 are
            // Writer's own in-text markers.
            case 0x01 or 0x02 or 0x20 or 0xA0:
            // Spacing modifier letters that can be Bopomofo tonal marks.
            case 0x2CA or 0x2CB or 0x2C7 or 0x2D9:
            // tdf#52577: superscript digits are weak.
            case 0xB2 or 0xB3 or 0xB9:
                return WriterScriptClass.Weak;

            // The Coptic workaround, which the C++ spells out for the same reason.
            case >= 0x2C80 and <= 0x2CE3:
                return WriterScriptClass.Latin;
        }

        return codePoint switch
        {
            // Basic Latin .. Spacing Modifier Letters
            <= 0x02FF => WriterScriptClass.Latin,
            // Greek .. Armenian
            >= 0x0370 and <= 0x058F => WriterScriptClass.Latin,
            // Hebrew .. Myanmar
            >= 0x0590 and <= 0x109F => WriterScriptClass.Complex,
            // Georgian
            >= 0x10A0 and <= 0x10FF => WriterScriptClass.Latin,
            // Hangul Jamo
            >= 0x1100 and <= 0x11FF => WriterScriptClass.Asian,
            // Ethiopic
            >= 0x1200 and <= 0x137F => WriterScriptClass.Complex,
            // Cherokee .. Runic
            >= 0x13A0 and <= 0x16FF => WriterScriptClass.Latin,
            // Khmer .. Mongolian
            >= 0x1780 and <= 0x18AF => WriterScriptClass.Complex,
            // Latin Extended Additional .. Greek Extended
            >= 0x1E00 and <= 0x1FFF => WriterScriptClass.Latin,
            // Number Forms
            >= 0x2150 and <= 0x218F => WriterScriptClass.Weak,
            // Latin Extended-C, and the rest of the Coptic block the exception above did not take
            >= 0x2C60 and <= 0x2C7F => WriterScriptClass.Latin,
            >= 0x2CE4 and <= 0x2CFF => WriterScriptClass.Latin,
            // CJK Radicals Supplement .. Yi Radicals, the code points the block table's
            // `UBLOCK_CJK_RADICALS_SUPPLEMENT .. UBLOCK_HANGUL_SYLLABLES` entry reaches
            >= 0x2E80 and <= 0xA4CF => WriterScriptClass.Asian,
            // Latin Extended-D
            >= 0xA720 and <= 0xA7FF => WriterScriptClass.Latin,
            // Hangul Jamo Extended-A, Hangul Syllables and Hangul Jamo Extended-B
            >= 0xA960 and <= 0xA97F => WriterScriptClass.Asian,
            >= 0xAC00 and <= 0xD7FF => WriterScriptClass.Asian,
            // CJK Compatibility Ideographs
            >= 0xF900 and <= 0xFAFF => WriterScriptClass.Asian,
            // Arabic Presentation Forms-A
            >= 0xFB50 and <= 0xFDFF => WriterScriptClass.Complex,
            // CJK Compatibility Forms
            >= 0xFE30 and <= 0xFE4F => WriterScriptClass.Asian,
            // Arabic Presentation Forms-B
            >= 0xFE70 and <= 0xFEFF => WriterScriptClass.Complex,
            // Halfwidth and Fullwidth Forms
            >= 0xFF00 and <= 0xFFEF => WriterScriptClass.Asian,
            // CJK Unified Ideographs Extension B .. CJK Compatibility Ideographs Supplement
            >= 0x20000 and <= 0x2FA1F => WriterScriptClass.Asian,
            _ => WriterScriptClass.Weak,
        };
    }

    /// <summary>
    /// The item a run's text selects, reduced from a per-character rule to a per-run one.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>GreedyScriptChangeScanner</c> itemises a whole paragraph: a character keeps its own
    /// script class, a <em>weak</em> one takes the hint where the run states one and otherwise the
    /// script of the text before it, and a paragraph that opens weak takes the first non-weak
    /// script anywhere in it
    /// (<c>i18nutil/source/utility/scriptchangescanner.cxx</c>:200-300, 336-360).
    /// </para>
    /// <para>
    /// <strong>This resolves one item per run, because a run resolves one face.</strong> The
    /// reduction is deliberately the conservative half of the rule: the hint decides only where
    /// <em>every</em> character of the run is weak, so a run mixing a hinted symbol with ordinary
    /// prose keeps the western item it has always had rather than moving its Latin text onto the
    /// East Asian one. Both corpus witnesses put their <c>U+2610</c> in a run of its own, which is
    /// what Word writes for a hinted character.
    /// </para>
    /// </remarks>
    /// <param name="text">The run's text.</param>
    /// <param name="hint">What <c>w:rFonts/@w:hint</c> states, if anything.</param>
    public static WriterScript ForRun(ReadOnlySpan<char> text, WriterScriptHint hint)
    {
        bool anyWeak = false;

        for (int at = 0; at < text.Length;)
        {
            int codePoint = text[at];
            int width = 1;
            if (char.IsHighSurrogate(text[at]) && at + 1 < text.Length
                && char.IsLowSurrogate(text[at + 1]))
            {
                codePoint = char.ConvertToUtf32(text[at], text[at + 1]);
                width = 2;
            }

            switch (ClassOf(codePoint))
            {
                case WriterScriptClass.Latin: return WriterScript.Western;
                case WriterScriptClass.Asian: return WriterScript.Asian;
                case WriterScriptClass.Complex: return WriterScript.Complex;
                default: anyWeak = true; break;
            }

            at += width;
        }

        return anyWeak
            ? hint switch
            {
                WriterScriptHint.Asian => WriterScript.Asian,
                WriterScriptHint.Complex => WriterScript.Complex,
                _ => WriterScript.Western,
            }
            : WriterScript.Western;
    }

    /// <summary>
    /// The language an item carries when the document states none for it.
    /// </summary>
    /// <remarks>
    /// <c>MsLangId::resolveSystemLanguageByScriptType</c>
    /// (<c>i18nlangtag/source/isolang/mslangid.cxx</c>:135-165): a language whose own script is not
    /// the item's is replaced, by <c>LANGUAGE_CHINESE_SIMPLIFIED</c> for the Asian item and
    /// <c>LANGUAGE_HINDI</c> for the complex one. <c>SwDoc::SwDoc</c> puts the three answers on the
    /// document as its default language items, so a file that names no language still sends every
    /// complex-script run to fontconfig under <c>hi</c>.
    /// </remarks>
    public static string DefaultLanguage(WriterScript script) => script switch
    {
        WriterScript.Asian => "zh-CN",
        WriterScript.Complex => "hi-IN",
        _ => "en-US",
    };
}
