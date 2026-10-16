using System.Xml.Linq;
using XRay.Office.Text.Fonts;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// Which of Writer's three character-font items a DOCX run is set from, and what that item carries.
/// </summary>
/// <remarks>
/// <para>
/// <strong><c>WordFallbackClass.ForDeclared</c>'s roman default is the <em>western</em> item's, and
/// two of the corpus's documents draw <c>U+2610</c> in runs that are not on it.</strong> Writer
/// keeps <c>RES_CHRATR_FONT</c>, <c>RES_CHRATR_CJK_FONT</c> and <c>RES_CHRATR_CTL_FONT</c> side by
/// side and <c>SwScriptInfo::WhichFont</c> selects one per script item of the text
/// (<c>sw/source/core/text/porlay.cxx</c>:879-901). <c>w:rFonts/@w:hint</c> is what moves a
/// <em>weak</em> character onto one of the other two —
/// <c>DomainMapper::lcl_attribute</c>:969-988 turns it into <c>RES_CHRATR_SCRIPT_HINT</c> and
/// <c>GreedyScriptChangeScanner</c> reads it only where <c>GetScriptClass</c> answered
/// <c>WEAK</c>.
/// </para>
/// <para>
/// The item decides two things and only the first was ever modelled here: the family class, which
/// through this filter only the western item carries, and the <em>language</em>, which
/// <c>MsLangId::resolveSystemLanguageByScriptType</c> defaults to <c>zh-CN</c> and <c>hi-IN</c> for
/// the other two. Measured on 26.2.4.2, one DOCX per cell with the face read out of the PDF
/// (<c>probes/fonts-r65/gen-scriptitem.py</c>): a <c>w:hint="eastAsia"</c> run drawing
/// <c>U+2610</c> answers Unifont and a complex-script run answers FreeSans, both under every
/// declared class.
/// </para>
/// </remarks>
public sealed class WordScriptItemTests
{
    [Theory]
    [InlineData("eastAsia", WriterScriptHint.Asian)]
    [InlineData("cs", WriterScriptHint.Complex)]
    [InlineData("default", WriterScriptHint.Automatic)]
    public void TheHintIsReadFromTheInnermostLayerThatStatesOne(string value, WriterScriptHint expected)
    {
        // The same inheritance every other `w:rFonts` attribute has: the innermost layer that
        // states one wins, and `default` states one -- it means automatic, so it stops the search.
        WordParagraphFormats.HintOf([Hint(value), Hint("eastAsia")]).ShouldBe(expected);
        WordParagraphFormats.HintOf([Bare(), Hint(value)]).ShouldBe(expected);
    }

    [Fact]
    public void NoLayerStatingAHintIsAutomatic()
    {
        WordParagraphFormats.HintOf([Bare(), Bare()]).ShouldBe(WriterScriptHint.Automatic);
        WordParagraphFormats.HintOf([]).ShouldBe(WriterScriptHint.Automatic);
    }

    [Fact]
    public void AHintedWeakRunTakesTheEastAsianItemAndNotTheWesternDefault()
    {
        // `150-5370-10H.docx`'s shape exactly: `<w:rFonts w:ascii="MS Gothic"
        // w:eastAsia="MS Gothic" w:hAnsi="MS Gothic" w:hint="eastAsia"/>` over a lone `U+2610`.
        WordTextStyle style = Style("MS Gothic", FontFamilyClass.Serif, WriterScriptHint.Asian)
            .OnScript("☐");

        style.Script.ShouldBe(WriterScript.Asian);
        style.FontItem.DeclaredClass.ShouldBe(FontFamilyClass.Unknown);
        style.FontItem.Language.ShouldBe("zh-CN");
    }

    [Fact]
    public void AComplexCharacterTakesTheComplexItemWithNoHintAtAll()
    {
        WordTextStyle style = Style("Calibri", FontFamilyClass.SansSerif, WriterScriptHint.Automatic)
            .OnScript("א");

        style.Script.ShouldBe(WriterScript.Complex);
        style.FontItem.DeclaredClass.ShouldBe(FontFamilyClass.Unknown);
        style.FontItem.Language.ShouldBe("hi-IN");
    }

    [Fact]
    public void AWesternRunKeepsTheDeclaredClassAndTheDocumentsOwnLanguage()
    {
        // The control: everything above has to leave the ordinary path exactly as it was.
        WordTextStyle style = Style("Calibri", FontFamilyClass.SansSerif, WriterScriptHint.Asian)
            with { Language = "en-GB" };

        style = style.OnScript("Text");

        style.Script.ShouldBe(WriterScript.Western);
        style.FontItem.DeclaredClass.ShouldBe(FontFamilyClass.SansSerif);
        style.FontItem.Language.ShouldBe("en-GB");
    }

    [Fact]
    public void AStatedLanguageBeatsWritersDefaultForItsOwnItem()
    {
        // Word writes `<w:lang w:val="en-US" w:eastAsia="en-US" w:bidi="ar-SA"/>` into
        // `docDefaults` for nearly every file, and both corpus witnesses carry it -- which is why
        // their East Asian runs answer DejaVu Sans rather than the Unifont a document stating no
        // language gets.
        WordTextStyle style =
            Style("MS Gothic", FontFamilyClass.Unknown, WriterScriptHint.Asian)
                with { AsianLanguage = "en-US", ComplexLanguage = "ar-SA" };

        style.OnScript("☐").ItemLanguage.ShouldBe("en-US");
        style.OnScript("א").ItemLanguage.ShouldBe("ar-SA");
    }

    [Fact]
    public void TheItemIsPartOfTheFaceKey()
    {
        // Two requests with the same family, weight and slant and different items are two questions
        // with two answers, so sharing a cache entry between them is a collision rather than a
        // saving.
        WordTextStyle western = Style("MS Gothic", FontFamilyClass.Serif, WriterScriptHint.Asian)
            .OnScript("Text");
        WordTextStyle asian = Style("MS Gothic", FontFamilyClass.Serif, WriterScriptHint.Asian)
            .OnScript("☐");

        western.FaceKey.ShouldNotBe(asian.FaceKey);
    }

    [Fact]
    public void ARunNamingNoFamilyStatesNoItem()
    {
        // The same distinction `WordFallbackClass.ForDeclared` carries: "no font named" is answered
        // by `DefaultFonts` and not by a fallback shape, so such a run keeps the generic the face it
        // resolved to is filed under rather than asking fontconfig about an empty family.
        WordTextStyle style = Style("", FontFamilyClass.Serif, WriterScriptHint.Asian).OnScript("☐");

        style.FontItem.IsStated.ShouldBeFalse();
    }

    [Fact]
    public void TwoRunsOnDifferentItemsAreNotTheSameFormatting()
    {
        // The uniform-paragraph shortcut folds runs that agree with the paragraph mark, and it
        // compares what it can see. Two runs can resolve to the *same* face off different items --
        // a `w:hint="eastAsia"` run naming the paragraph's own family is the shape -- so the item
        // has to be one of the things it compares, or the run is folded away and asks its glyph
        // fallback under the mark's western item.
        WordTextStyle mark = Style("Calibri", FontFamilyClass.SansSerif, WriterScriptHint.Asian);

        mark.OnScript("Text").FontItem.ShouldNotBe(mark.OnScript("☐").FontItem);
        mark.OnScript("Text").FontItem.ShouldBe(mark.OnScript("More text").FontItem);
    }

    private static WordTextStyle Style(string family, FontFamilyClass declared, WriterScriptHint hint)
        => new(
            family, Core.Units.Length.FromPoints(10), 400, false, Language: null,
            DeclaredClass: declared, Hint: hint);

    private static XElement Hint(string value)
        => new(Word.Name("rFonts"), new XAttribute(Word.Name("hint"), value));

    private static XElement Bare() => new(Word.Name("rFonts"));
}
