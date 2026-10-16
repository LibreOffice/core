using XRay.Office.Text.Fonts;

namespace XRay.Office.WordProcessing.Layout;

/// <summary>
/// The generic class a word-processing filter hands the font matcher for a family it cannot find.
/// </summary>
/// <remarks>
/// <para>
/// <strong>This is a property of the filter, not of the resolver, and that is the whole point of
/// the type.</strong> Asked the same question — an unrecognised family, which DejaVu? — the
/// LibreOffice 26.2.4.2 filters give two different answers, so a rule put in
/// <c>SystemFontResolver</c> would be right for one set of formats and wrong for the other.
/// Measured 2026-08-21 with 98 authored one-line files plus 28 cross-format ones, every one
/// converted by the installed <c>soffice</c> with the drawn face read out of the PDF
/// (<c>probes/words-r54/font-fallback-rule.py</c>, <c>probes/words-r54/cross-format-fallback.py</c>):
/// </para>
/// <list type="table">
/// <listheader><term>filter</term><description>an unrecognised family, nothing declared</description></listheader>
/// <item><term>DOCX</term><description>DejaVu <b>Serif</b>; only <c>w:family="swiss"</c> moves it, to DejaVu Sans</description></item>
/// <item><term>DOC</term><description>DejaVu <b>Serif</b>; the <c>FFN</c>'s swiss code moves it the same way</description></item>
/// <item><term>RTF</term><description>DejaVu <b>Serif</b>; <c>\fnil</c>, <c>\froman</c>, <c>\fswiss</c> and <c>\fmodern</c> all answer Serif</description></item>
/// <item><term>ODF text</term><description>fontconfig's own generic — <c>Aptos</c> → Sans, <c>Consolas</c> → <b>Mono</b>, <c>Garamond</c> → Serif</description></item>
/// <item><term>XLSX, PPTX, FODS</term><description>fontconfig's own generic, matching <c>fc-match</c> exactly</description></item>
/// </list>
/// <para>
/// So the answer does not depend on the request — bold, italic, 8 pt, 40 pt and an east-Asian hint
/// all answer the same family — nor on the shape of the name, nor on what fontconfig files the name
/// under. Twenty-one further names probed through the DOCX filter all answer DejaVu Serif,
/// including the four <c>Times</c>, <c>Helvetica</c>, <c>Albany</c> and <c>Thorndale</c> whose chain
/// entries <em>are</em> installed, and including <c>Consolas</c>, which fontconfig files under
/// <c>monospace</c>. The only three that escape are the two strong metric aliases
/// (<c>Times New Roman</c> → Liberation Serif, <c>Arial</c> → Liberation Sans) and the pi face
/// (<c>Symbol</c> → OpenSymbol), which is exactly what <c>SystemFontResolver.DeclaredGenericFor</c>
/// already exempts.
/// </para>
/// <para>
/// The mechanism this states is Writer's own: <c>SvxFontItem</c>'s family defaults to
/// <c>FAMILY_ROMAN</c>, and <c>FontConfigManager::Substitute</c> appends <c>"serif"</c> as a second
/// <c>FC_FAMILY</c> for it — so the pre-match asks fontconfig for <c>"Aptos"</c>-or-<c>"serif"</c>
/// and gets DejaVu Serif, where a bare <c>fc-match Aptos</c> gets DejaVu Sans. The DOCX and DOC
/// filters overwrite that default from their font tables; the RTF filter does not, which is why its
/// <c>\fswiss</c> is inert. The ODF and spreadsheet/presentation filters do not go through this
/// default at all.
/// </para>
/// <para>
/// <strong>And the declared class is inherited rather than looked up per name — measured in round
/// 55, after round 54 shipped the per-name reading and lost a verdict to it.</strong> Through a
/// DOCX the class is set only where <c>w:rFonts/@w:ascii</c> names a font the font table files
/// under <c>roman</c> or <c>swiss</c>; <c>auto</c>, <c>modern</c>, a pitch-only entry, an absent
/// entry and <c>w:asciiTheme</c> all leave whatever an ancestor put there, and nothing anywhere
/// stating one leaves it roman. So this method's second argument is <em>the class in force at the
/// run</em>, which <see cref="Ooxml.WordParagraphFormats.StatedClass"/> resolves from the layer
/// stack — not the class of the family the run names. The DOC arm keeps handing over the
/// named font's own <c>FFN</c> class, because <c>SwWW8ImplReader</c> builds an
/// <c>SvxFontItem</c> per font there and there is no inheritance to model.
/// </para>
/// <para>
/// <b>Only the declared class is read, never the declared pitch.</b> Both are in every one of these
/// font tables and only the first moves the reference: <c>Aptos</c> declared <c>pitch="fixed"</c>
/// answers DejaVu Serif, and so does <c>Consolas</c> declared <c>modern</c>. Passing the pitch on
/// once put a corpus document into DejaVu Sans Mono that the reference sets in DejaVu Sans — see
/// <c>DocxLayoutSource.Face</c>.
/// </para>
/// </remarks>
internal static class WordFallbackClass
{
    /// <summary>
    /// The class to hand <see cref="FontRequest"/> for a run asking for
    /// <paramref name="familyName"/>, which the document declared <paramref name="declared"/> about.
    /// </summary>
    /// <param name="familyName">
    /// The family the run asks for. <strong>A run that names none at all is the one case the roman
    /// default must not reach</strong>, and getting that wrong cost a whole sweep: "no font named"
    /// and "a font nobody has" are different questions, and the first is answered by LibreOffice's
    /// <c>DefaultFonts</c> — Liberation Serif here — not by a fallback shape.
    /// <c>SystemFontResolver.GenericFallbacks</c> makes exactly that distinction, but a declared
    /// class is consulted <em>before</em> it, in the pre-match step, so handing one over for an
    /// empty family bypasses the rule entirely. Measured: a DOCX whose <c>docDefaults</c> states an
    /// empty <c>w:rFonts</c> renders in Liberation Serif on 26.2.4.2, and applying the default here
    /// regardless of the name moved <b>29 corpus <c>.doc</c> documents</b> from Liberation Serif to
    /// DejaVu Serif and lost 17 verdicts.
    /// </param>
    /// <param name="declared">
    /// <strong>The class in force at this run</strong>, or <see cref="FontFamilyClass.Unknown"/>
    /// when nothing in the layer stack states one — which is the common case, and is the case this
    /// method exists for. Through a DOCX that is
    /// <see cref="Ooxml.WordParagraphFormats.StatedClass"/>'s answer and <em>not</em> the font
    /// table's entry for <paramref name="familyName"/>; through DOC and RTF, which have no
    /// inheritance to model, it is the named font's own.
    /// </param>
    /// <returns>
    /// <see cref="FontFamilyClass.Unknown"/> for a run naming no family at all;
    /// <see cref="FontFamilyClass.SansSerif"/> for a family the document declares sans-serif;
    /// <see cref="FontFamilyClass.Serif"/> for every other named family, including one the font
    /// table never mentions.
    /// </returns>
    public static FontFamilyClass ForDeclared(string? familyName, FontFamilyClass declared)
        => string.IsNullOrWhiteSpace(familyName) ? FontFamilyClass.Unknown
            : declared == FontFamilyClass.SansSerif ? FontFamilyClass.SansSerif
            : FontFamilyClass.Serif;

    /// <summary>
    /// The same answer, told which of Writer's three character-font items the run is set from.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong><see cref="ForDeclared"/>'s roman default is the <em>western</em> item's, and a run
    /// taken from the East Asian or complex-script item must not be given it.</strong> Writer keeps
    /// <c>RES_CHRATR_FONT</c>, <c>RES_CHRATR_CJK_FONT</c> and <c>RES_CHRATR_CTL_FONT</c> side by
    /// side and <c>SwScriptInfo::WhichFont</c> selects one per script item of the text
    /// (<c>sw/source/core/text/porlay.cxx</c>:879-901). Only the western one ever carries a class
    /// through the DOCX filter: <c>LN_CT_Fonts_ascii</c> inserts
    /// <c>PROP_CHAR_FONT_FAMILY</c> and <c>LN_CT_Fonts_eastAsia</c> and <c>LN_CT_Fonts_cs</c> insert
    /// the name alone (<c>sw/source/writerfilter/dmapper/DomainMapper.cxx</c>:436-508), so the other
    /// two keep the pool default's family type — and <c>OutputDevice::GetDefaultFont</c> sets
    /// <c>FAMILY_SYSTEM</c> for <c>CJK_TEXT</c> and <c>CTL_TEXT</c>, which appends no generic to the
    /// fontconfig pattern at all (<c>vcl/source/outdev/font.cxx</c>,
    /// <c>vcl/unx/generic/font/fontconfig.cxx</c>:1075-1088).
    /// </para>
    /// <para>
    /// Measured on 26.2.4.2, one DOCX per cell with the face read out of the PDF
    /// (<c>probes/fonts-r65/gen-scriptitem.py</c>): a <c>w:hint="eastAsia"</c> run drawing
    /// <c>U+2610</c> answers <b>Unifont</b> and a complex-script run drawing <c>U+05D0</c> answers
    /// <b>FreeSans</b> — under <em>every</em> declared class, including the <c>swiss</c> that moves
    /// the western row from FreeSerif to DejaVu Sans. What separates those two answers from each
    /// other is the item's <em>language</em> and not its class; see
    /// <see cref="Ooxml.WordTextStyle.ItemLanguage"/>.
    /// </para>
    /// </remarks>
    /// <param name="familyName">The family the run asks for, or null when it names none.</param>
    /// <param name="declared">The class in force at the run, which only the western item reads.</param>
    /// <param name="script">Which item the run's text selects.</param>
    public static FontFamilyClass ForScript(
        string? familyName, FontFamilyClass declared, Text.Fonts.WriterScript script)
        => script == Text.Fonts.WriterScript.Western
            ? ForDeclared(familyName, declared)
            : FontFamilyClass.Unknown;

    /// <summary>
    /// The class to hand <see cref="FontRequest"/> for a <c>.doc</c> run, whose <c>FFN</c> states one
    /// per font.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>The WW8 filter is the one of the three that can reach
    /// <see cref="FontFamilyClass.Unknown"/>, so it does not take the roman default.</strong>
    /// <c>SwWW8ImplReader::SetNewFontAttr</c> builds an <c>SvxFontItem</c> per font from the
    /// <c>FFN</c>'s <c>ff</c> nibble, and <c>GetFontParams</c>'s table maps 0, 6 and 7 onto
    /// <c>FAMILY_DONTKNOW</c> — which is *set on the item*, where the DOCX filter would have left an
    /// inherited value and the RTF filter never sets one at all. A <c>DONTKNOW</c> family appends no
    /// generic to the fontconfig pre-match, so the answer is fontconfig's own.
    /// </para>
    /// <para>
    /// Measured on 26.2.4.2 with nine flat-ODF fixtures exported to Word 97 and back
    /// (<c>probes/words-r55/doc-family-code.py</c>) — a route that reaches the WW8 import with a
    /// genuinely undeclared <c>FFN</c>, which round 54's DOCX round trip could not:
    /// <b>only <c>roman</c> draws DejaVu Serif</b>; no code at all, <c>modern</c> and
    /// <c>decorative</c> all draw DejaVu Sans, and so does <c>swiss</c>.
    /// </para>
    /// <para>
    /// So this hands the <c>FFN</c>'s own answer through untouched, and the one thing it does is the
    /// guard <see cref="ForDeclared"/> carries for the same reason: a run naming <em>no</em> family
    /// is answered by <c>DefaultFonts</c> — Liberation Serif — and must not be given a class at all.
    /// </para>
    /// </remarks>
    /// <param name="familyName">The family the run asks for, or null when it names none.</param>
    /// <param name="declared">The class the <c>FFN</c> states, including its overrides by name.</param>
    public static FontFamilyClass ForWw8Font(string? familyName, FontFamilyClass declared)
        => string.IsNullOrWhiteSpace(familyName) ? FontFamilyClass.Unknown : declared;
}
