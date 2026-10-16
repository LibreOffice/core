using XRay.Office.Core.Graphics;
using XRay.Office.Spreadsheets.MsBinary;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A BIFF hatch is drawn as one blended colour, and the weights are not SpreadsheetML's.
/// </summary>
/// <remarks>
/// <para>
/// The same rule as <see cref="XlsxPatternFillTests"/> in a different notation: Calc has no
/// hatched cell background, so <c>XclImpCellArea::FillToItemSet</c> hands the two colours and the
/// pattern number to <c>XclTools::GetPatternColor</c>, whose nineteen-entry ratio table decides
/// the mix. Both BIFF readers used to take one of the two colours whole —
/// <c>XlsDecorationTable.FormatOf</c> the background, <c>XlsConditionalFormats.ReadAreaBlock</c>
/// the foreground — which is right at <c>solid</c> and wrong at every hatch.
/// </para>
/// <para>
/// <strong>The ratio runs the opposite way from SpreadsheetML's and the two tables are not each
/// other's complement.</strong> Here <c>0x00</c> is the full pattern colour, so <c>solid</c> is
/// the mix at nought; BIFF's 12.5 % grey is <c>0x70</c> where SpreadsheetML's <c>gray125</c> is
/// <c>0x10</c>. Transcribing one from the other is the mistake this file exists to catch.
/// </para>
/// <para>
/// The witness is <c>PC1000.xls</c>, the corpus's only workbook whose cells reach a hatch: 82 of
/// them state pattern <c>0x11</c>, black over white, and 26.2.4.2's own <c>--convert-to fods</c>
/// gives their cell style <c>fo:background-color="#dfdfdf"</c>.
/// </para>
/// </remarks>
public sealed class XlsPatternFillTests
{
    [Theory]
    // Black over white, so each pattern reads as its own ratio out of 0x80: `trunc(255 * r / 128)`.
    [InlineData(1, "#000000")]      // solid — the pattern colour whole
    [InlineData(2, "#7F7F7F")]      // 0x40
    [InlineData(3, "#3F3F3F")]      // 0x20
    [InlineData(4, "#BFBFBF")]      // 0x60
    [InlineData(15, "#8F8F8F")]     // 0x48
    [InlineData(16, "#9F9F9F")]     // 0x50
    [InlineData(17, "#DFDFDF")]     // 0x70 — the witness' 12.5 % grey
    [InlineData(18, "#EFEFEF")]     // 0x78
    // Past the table's nineteen entries the reference returns the pattern colour whole.
    [InlineData(19, "#000000")]
    [InlineData(63, "#000000")]
    public void EachPatternNumberCarriesItsOwnWeight(int pattern, string expected)
        => XlsPatternFill.Resolve(pattern, Colour.Black, Colour.White)
            .ToString().ShouldBe(expected);

    [Fact]
    public void TheWitnessesGreyIsTheReferencesOwn()
        // `PC1000.xls` pattern 0x11 over the default palette's black and white. 26.2.4.2 writes
        // `#dfdfdf` for it and draws `#DFDFDF` in its own PDF; this tree now draws the same.
        => XlsPatternFill.Resolve(0x11, Colour.Black, Colour.White)
            .ToString().ShouldBe("#DFDFDF");

    [Fact]
    public void ASolidPatternIsTheForegroundAndNotAMixOfAnything()
    {
        // `solid`'s ratio is 0x00, so the mix is the pattern colour exactly — which is what makes
        // this a general rule rather than a special case beside one.
        Colour red = Colour.FromRgb(0xFF0000);
        Colour blue = Colour.FromRgb(0x0000FF);

        XlsPatternFill.Resolve(XlsPatternFill.Solid, red, blue).ShouldBe(red);
    }

    [Fact]
    public void TheMixTruncatesTowardsZeroInBothDirections()
    {
        // C++ integer division truncates towards zero, so the sign of `background − pattern`
        // decides which way a component rounds and the two directions are not symmetric.
        // At ratio 0x70: 0 → 255 gives trunc(255 × 112 / 128) = 223; 255 → 0 gives
        // 255 + trunc(−255 × 112 / 128) = 255 − 223 = 32.
        XlsPatternFill.Resolve(0x11, Colour.Black, Colour.White).R.ShouldBe((byte)223);
        XlsPatternFill.Resolve(0x11, Colour.White, Colour.Black).R.ShouldBe((byte)32);
    }
}
