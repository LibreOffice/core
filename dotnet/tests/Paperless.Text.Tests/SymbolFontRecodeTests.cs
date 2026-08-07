using Paperless.Text.Fonts;
using Shouldly;

namespace Paperless.Text.Tests;

/// <summary>
/// Checks the ported <c>fontcvt.cxx</c> recode tables against values read out of LibreOffice's
/// own source and out of the installed OpenSymbol's <c>cmap</c>.
/// </summary>
/// <remarks>
/// <para>
/// Every expectation here is a specific code point rather than a property, because the failure
/// this guards against is a table that is *plausibly* wrong. The generator's first draft parsed
/// each 224-entry array with a <c>0x[0-9a-f]+</c> regex, which silently skipped the one entry
/// written as a bare <c>0</c> and shifted every index after it — <c>aWingDingsTab</c> then came
/// out at 223 entries and sent U+F0D8 to U+E49F instead of U+E49E. Nothing about that reading
/// looks wrong; only a pinned value catches it.
/// </para>
/// <para>
/// Reintroducing the bug to check these fail: change <c>SymbolFontRecode.TryRecode</c> to return
/// false, and every assertion below that names a destination fails.
/// </para>
/// </remarks>
public class SymbolFontRecodeTests
{
    [Fact]
    public void AWingdingsSlotBecomesTheOpenSymbolCodePointHoldingTheSamePicture()
    {
        // fontcvt.cxx:185-257, row `// F0d0`, ninth entry. The filled arrowhead this corpus
        // deck bullets every line with: `2015-Civil-Rights-Website-training.ppt`, whose
        // reference PDF extracts U+F0D8 from a subset of OpenSymbol.
        SymbolFontRecode.TryRecode("Wingdings", '\uF0D8', out char recoded).ShouldBeTrue();
        recoded.ShouldBe('\uE49E');
    }

    [Fact]
    public void TheUnaliasedSlotRecodesTheSameWayAsItsPrivateUseAreaForm()
    {
        // RecodeChar subtracts 0xF000 only when the high byte is set, so 0x00D8 and 0xF0D8 are
        // the same slot. Files state it both ways: the corpus has `char="Ø"` and `char="&#xF0D8;"`.
        SymbolFontRecode.TryRecode("Wingdings", '\u00D8', out char plain).ShouldBeTrue();
        SymbolFontRecode.TryRecode("Wingdings", '\uF0D8', out char aliased).ShouldBeTrue();
        plain.ShouldBe(aliased);
        plain.ShouldBe('\uE49E');
    }

    [Fact]
    public void TheFilledCircleFramingEuropeBulletsWithIsU2022sReplacement()
    {
        // `Framing Europe.ppt` states slot 0x6E. Measured against the reference PDF: the glyph
        // this produces is 13.73 pt wide at the same pen, where U+2022 was 6.30.
        SymbolFontRecode.TryRecode("Wingdings", '\uF06E', out char recoded).ShouldBeTrue();
        recoded.ShouldBe('\uE439');
    }

    [Fact]
    public void AdobeSymbolsBulletIsOpenSymbolsOwnBullet()
    {
        // The commonest symbol bullet in the corpus after Wingdings — 123 uses across 8 decks.
        // U+E12C is also what RecodeChar forces for a table hole, which is a consistency check
        // on the parse rather than a coincidence.
        SymbolFontRecode.TryRecode("Symbol", '\uF0B7', out char recoded).ShouldBeTrue();
        recoded.ShouldBe('\uE12C');
    }

    [Fact]
    public void AHoleInATableBecomesOpenSymbolsBulletRatherThanNotdef()
    {
        // aWingDingsTab's only hole is slot 0x7F, the last entry of the `// F070` row.
        // RecodeChar substitutes U+E12C whenever the entry is zero and the substitute face is
        // OpenSymbol or StarSymbol (fontcvt.cxx:1259-1267).
        SymbolFontRecode.TryRecode("Wingdings", '\uF07F', out char recoded).ShouldBeTrue();
        recoded.ShouldBe('\uE12C');
    }

    [Fact]
    public void ACodePointOutsideTheSymbolRangeIsNotASlotAndIsLeftAlone()
    {
        // One corpus deck states U+0178 against an a:buFont of Wingdings. It is above 0x00FF and
        // below 0xF020, so RecodeString skips it (fontcvt.cxx:1288-1291) and so does this.
        SymbolFontRecode.TryRecode("Wingdings", '\u0178', out char recoded).ShouldBeFalse();
        recoded.ShouldBe('\u0178');
    }

    [Fact]
    public void AFaceWithNoTableIsLeftAlone()
    {
        SymbolFontRecode.TryRecode("Calibri", '\uF0D8', out char recoded).ShouldBeFalse();
        recoded.ShouldBe('\uF0D8');
        SymbolFontRecode.IsRecodeable("Calibri").ShouldBeFalse();
    }

    [Theory]
    [InlineData("Wingdings")]
    [InlineData("wingdings")]
    [InlineData("Wingdings 2")]
    [InlineData("Wingdings 3")]
    [InlineData("Webdings")]
    [InlineData("Symbol")]
    [InlineData("Monotype Sorts")]
    [InlineData("ZapfDingbats")]
    [InlineData("StarBats")]
    [InlineData("StarMath")]
    [InlineData("MT Extra")]
    public void EveryFaceLibreOfficeListsIsReachedThroughTheSameNameNormalisation(string family)
    {
        // aStarSymbolRecodeTable is keyed on GetEnglishSearchFontName's output, which
        // FontSubstitutions.Normalise reproduces: lower case, punctuation and spaces removed.
        // "Wingdings 2" and "Monotype Sorts" only match once that has happened.
        SymbolFontRecode.IsRecodeable(family).ShouldBeTrue();
    }

    [Fact]
    public void TheTwoNamesForTheSubstituteFaceAreBothAccepted()
    {
        // IsOpenSymbol takes StarSymbol and OpenSymbol as one face
        // (unotools/source/misc/fontdefs.cxx:408-413). The tables were written for the first and
        // the second is what is installed.
        SymbolFontRecode.IsSubstituteFamily("OpenSymbol").ShouldBeTrue();
        SymbolFontRecode.IsSubstituteFamily("StarSymbol").ShouldBeTrue();
        SymbolFontRecode.IsSubstituteFamily("Wingdings").ShouldBeFalse();
        SymbolFontRecode.IsSubstituteFamily(null).ShouldBeFalse();
    }

    [Fact]
    public void TwoDifferentDingbatFacesDisagreeAboutTheSameSlot()
    {
        // The point of keying on the face at all. Slot 0xD8 is an arrowhead in Wingdings and a
        // different picture in Monotype Sorts, so one table for all symbol faces would draw the
        // wrong glyph for every face but one.
        SymbolFontRecode.TryRecode("Wingdings", '\uF0D8', out char wingdings).ShouldBeTrue();
        SymbolFontRecode.TryRecode("Monotype Sorts", '\uF0D8', out char sorts).ShouldBeTrue();
        wingdings.ShouldBe('\uE49E');
        sorts.ShouldBe('\uE25D');
        wingdings.ShouldNotBe(sorts);
    }

    [Fact]
    public void TheDigitsOfAdobeSymbolAreLeftAsDigits()
    {
        // Not every slot is a picture. aAdobeSymbolTab maps 0x30-0x39 to U+0030-U+0039, so a
        // symbol-encoded run of digits stays readable — and a table that shifted by one index
        // would break this before it broke anything visible.
        for (char digit = '0'; digit <= '9'; digit++)
        {
            SymbolFontRecode.TryRecode("Symbol", digit, out char recoded).ShouldBeTrue();
            recoded.ShouldBe(digit);
        }
    }
}
