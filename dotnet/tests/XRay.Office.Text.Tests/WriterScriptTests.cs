using XRay.Office.Text.Fonts;
using Shouldly;

namespace XRay.Office.Text.Tests;

/// <summary>
/// Which of Writer's three character-font items a run's text selects, and what that item carries.
/// </summary>
/// <remarks>
/// The mechanism rather than any one document: <c>SwScriptInfo::WhichFont</c> maps the script class
/// of the text onto <c>SwFontScript</c> (<c>sw/source/core/text/porlay.cxx</c>:879-901), the
/// classification is <c>i18nutil::GetScriptClass</c>'s block table, and a <em>weak</em> character
/// is the only one <c>w:rFonts/@w:hint</c> can move
/// (<c>i18nutil/source/utility/scriptchangescanner.cxx</c>:246-268).
/// </remarks>
public class WriterScriptTests
{
    [Theory]
    // Latin and its neighbours in the block table.
    [InlineData('A', WriterScriptClass.Latin)]
    [InlineData('α', WriterScriptClass.Latin)]   // Greek
    [InlineData('а', WriterScriptClass.Latin)]   // Cyrillic
    [InlineData('ა', WriterScriptClass.Latin)]   // Georgian
    // Complex: Hebrew through Myanmar, Ethiopic, Khmer and Mongolian.
    [InlineData('א', WriterScriptClass.Complex)] // Hebrew
    [InlineData('ا', WriterScriptClass.Complex)] // Arabic
    [InlineData('ก', WriterScriptClass.Complex)] // Thai
    [InlineData('अ', WriterScriptClass.Complex)] // Devanagari
    [InlineData('ሀ', WriterScriptClass.Complex)] // Ethiopic
    [InlineData('ﭐ', WriterScriptClass.Complex)] // Arabic presentation forms
    // Asian: the CJK blocks, Hangul and the fullwidth forms.
    [InlineData('一', WriterScriptClass.Asian)]
    [InlineData('あ', WriterScriptClass.Asian)]   // Hiragana
    [InlineData('가', WriterScriptClass.Asian)]   // Hangul syllables
    [InlineData('ᄀ', WriterScriptClass.Asian)]   // Hangul Jamo
    [InlineData('Ａ', WriterScriptClass.Asian)]   // fullwidth A
    // Weak: the space, the non-breaking space and everything the block table leaves out.
    [InlineData(' ', WriterScriptClass.Weak)]
    [InlineData(' ', WriterScriptClass.Weak)]
    [InlineData('²', WriterScriptClass.Weak)]    // superscript two, tdf#52577
    [InlineData('☐', WriterScriptClass.Weak)]    // ballot box
    [InlineData('✓', WriterScriptClass.Weak)]    // check mark
    [InlineData('‑', WriterScriptClass.Weak)]    // non-breaking hyphen
    [InlineData('Ⅰ', WriterScriptClass.Weak)]    // number forms
    public void TheBlockTableClassifiesACharacter(char character, WriterScriptClass expected)
        => WriterScripts.ClassOf(character).ShouldBe(expected);

    [Fact]
    public void AWeakOnlyRunTakesItsHint()
    {
        // What both corpus witnesses write: one weak character in a run of its own, marked
        // `w:hint="eastAsia"`. `GreedyScriptChangeScanner` consults the hint exactly where
        // `GetScriptClass` answered WEAK.
        WriterScripts.ForRun("☐", WriterScriptHint.Asian).ShouldBe(WriterScript.Asian);
        WriterScripts.ForRun("☐ ", WriterScriptHint.Asian).ShouldBe(WriterScript.Asian);
        WriterScripts.ForRun("☐", WriterScriptHint.Complex).ShouldBe(WriterScript.Complex);
    }

    [Fact]
    public void WithoutAHintAWeakOnlyRunIsWestern()
    {
        // The scanner's own answer for a paragraph with no non-weak character in it: the
        // application language's script, which is Latin.
        WriterScripts.ForRun("☐", WriterScriptHint.Automatic).ShouldBe(WriterScript.Western);
        WriterScripts.ForRun("", WriterScriptHint.Asian).ShouldBe(WriterScript.Western);
    }

    [Fact]
    public void ACharacterWithAScriptOfItsOwnBeatsTheHint()
    {
        // The hint moves weak characters and nothing else, so a run that holds any character with a
        // script of its own keeps that script -- which is what stops a hinted symbol dragging the
        // prose beside it onto the East Asian item.
        WriterScripts.ForRun("☐ Item", WriterScriptHint.Asian).ShouldBe(WriterScript.Western);
        WriterScripts.ForRun("一", WriterScriptHint.Automatic).ShouldBe(WriterScript.Asian);
        WriterScripts.ForRun("א", WriterScriptHint.Asian).ShouldBe(WriterScript.Complex);
    }

    [Fact]
    public void EachItemHasItsOwnDefaultLanguage()
    {
        // `MsLangId::resolveSystemLanguageByScriptType`
        // (`i18nlangtag/source/isolang/mslangid.cxx`:135-165), which `SwDoc::SwDoc` puts on the
        // document as its three default language items. It is what sends a Hebrew character in a
        // run that states no language to a face carrying Devanagari.
        WriterScripts.DefaultLanguage(WriterScript.Western).ShouldBe("en-US");
        WriterScripts.DefaultLanguage(WriterScript.Asian).ShouldBe("zh-CN");
        WriterScripts.DefaultLanguage(WriterScript.Complex).ShouldBe("hi-IN");
    }

    [Theory]
    [InlineData("hi-IN", 0x0905)]
    [InlineData("hi", 0x0905)]
    [InlineData("he-IL", 0x05D0)]
    [InlineData("th-TH", 0x0E01)]
    [InlineData("ar-SA", 0x0627)]
    [InlineData("ja-JP", 0x3042)]
    [InlineData("ko-KR", 0xAC00)]
    [InlineData("ru-RU", 0x0430)]
    public void ALanguageIsModelledByOneCharacterOfItsScript(string tag, int expected)
        => FontLanguages.ExemplarOf(tag).ShouldBe(expected);

    [Fact]
    public void ChineseReadsPastItsPrimarySubtag()
    {
        // The one language whose two orthographies separate the installed faces: a Japanese face
        // covers the shared ideographs and fontconfig files it under `ja` alone, so a
        // simplified-only character is what a `zh-CN` pattern actually asks for.
        FontLanguages.ExemplarOf("zh-CN").ShouldBe(0x8FD9);
        FontLanguages.ExemplarOf("zh-Hans").ShouldBe(0x8FD9);
        FontLanguages.ExemplarOf("zh-TW").ShouldBe(0x4E00);
        FontLanguages.ExemplarOf("zh").ShouldBe(0x4E00);
    }

    [Theory]
    [InlineData("en-US")]
    [InlineData("de-DE")]
    [InlineData("vi-VN")]
    [InlineData("")]
    [InlineData(null)]
    public void ALatinLanguageRanksNothing(string? tag)
        // Every text face covers Latin, so modelling `en` could only demote a face fontconfig would
        // have kept. Unlisted tags answer the same way and leave the generic's list to decide.
        => FontLanguages.ExemplarOf(tag).ShouldBe(FontLanguages.Neutral);
}
