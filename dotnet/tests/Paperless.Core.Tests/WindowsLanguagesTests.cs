using Paperless.Core.Globalization;
using Shouldly;

namespace Paperless.Core.Tests;

/// <summary>
/// Tests the Windows language identifier table.
/// </summary>
/// <remarks>
/// The table itself is generated from LibreOffice's <c>i18nlangtag</c> data, so these tests are about
/// the two things generation cannot get right on its own: that the lookup finds what the table holds,
/// and that the fallbacks and sentinels behave as the readers assume.
/// </remarks>
public class WindowsLanguagesTests
{
    [Theory]
    [InlineData(0x0409, "en-US")]
    [InlineData(0x0809, "en-GB")]
    [InlineData(0x0407, "de-DE")]
    [InlineData(0x0807, "de-CH")]
    [InlineData(0x040C, "fr-FR")]
    [InlineData(0x0C0A, "es-ES")]
    [InlineData(0x0410, "it-IT")]
    [InlineData(0x0419, "ru-RU")]
    [InlineData(0x0408, "el-GR")]
    [InlineData(0x041F, "tr-TR")]
    [InlineData(0x0411, "ja-JP")]
    [InlineData(0x0804, "zh-CN")]
    [InlineData(0x0412, "ko-KR")]
    [InlineData(0x0416, "pt-BR")]
    [InlineData(0x0816, "pt-PT")]
    [InlineData(0x040D, "he-IL")]
    [InlineData(0x041E, "th-TH")]
    public void AKnownIdentifierResolvesToItsTag(int identifier, string expected)
        => WindowsLanguages.TagOf((ushort)identifier).ShouldBe(expected);

    [Fact]
    public void TheTableCoversFarMoreThanTheWesternLocales()
    {
        // The point of generating it is coverage: a hand-written table stops at the languages whoever
        // wrote it happened to think of, and every document in the ones it missed is mislabelled.
        WindowsLanguages.Count.ShouldBeGreaterThan(400);
    }

    [Fact]
    public void TheSentinelsAreNotLanguages()
    {
        // Zero means "no language" and 0x0400 means "whatever the process default is". Both are
        // claims about the environment rather than about the text, so neither may become a tag.
        WindowsLanguages.TagOf(WindowsLanguages.None).ShouldBeNull();
        WindowsLanguages.TagOf(WindowsLanguages.SystemDefault).ShouldBeNull();
    }

    [Fact]
    public void AnUnknownRegionFallsBackToItsLanguage()
    {
        // A sublanguage nobody has assigned still names a language in its low ten bits, and an
        // unrecognised region is far likelier than an unrecognised language. 0xFC07 is a German
        // sublanguage that does not exist.
        WindowsLanguages.TagOf(0xFC07).ShouldBe("de-DE");
    }

    [Fact]
    public void AnUnknownLanguageIsReportedAsUnknownRatherThanGuessed()
    {
        // No default: a wrong tag is a claim about the text, while no tag is the absence of one. The
        // primary language of 0x03FF is unassigned, so neither the identifier nor its fallback
        // resolves.
        WindowsLanguages.TagOf(0x03FF).ShouldBeNull();
    }

    [Theory]
    [InlineData(0x0409, "en")]
    [InlineData(0x0807, "de")]
    [InlineData(0x0816, "pt")]
    [InlineData(0x0000, null)]
    public void TheLanguageAloneDropsTheRegion(int identifier, string? expected)
        => WindowsLanguages.LanguageOf((ushort)identifier).ShouldBe(expected);

    [Fact]
    public void ATagWithAScriptKeepsIt()
    {
        // Some identifiers need a script subtag to be unambiguous — Serbian is written in two — and
        // the language-and-region table cannot express one, which is why the generated table reads
        // LibreOffice's script table first.
        WindowsLanguages.TagOf(0x081A).ShouldBe("sr-Latn-CS");
        WindowsLanguages.LanguageOf(0x081A).ShouldBe("sr");
    }
}
