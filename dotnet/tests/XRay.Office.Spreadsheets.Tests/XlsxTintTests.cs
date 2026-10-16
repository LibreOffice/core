using XRay.Office.Core.Graphics;
using XRay.Office.Spreadsheets.Ooxml;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A SpreadsheetML <c>tint</c> modulates HSL luminance; it does not shift the RGB channels.
/// </summary>
/// <remarks>
/// <para>
/// The two are close on unsaturated colours and far apart on saturated ones, because shifting
/// every channel by one offset drives whichever is already brightest into the clamp at 255 while
/// the others keep moving. The hue then depends on how many channels clamped, which is why the
/// defect showed up as a *hue* change rather than as a brightness error: the stock Office gold
/// accent came out lemon.
/// </para>
/// <para>
/// The expected values are the reference's own pixels, sampled from LibreOffice 26.2.4.2's
/// rendering of <c>sheets/batch-008/xlsx/template-ECSPR-notifications.xlsx</c> page 19, whose
/// fills are <c>&lt;fgColor theme="4" tint="0.79998168889431442"/&gt;</c> and
/// <c>theme="7"</c> at the same tint. They are not derived from the formula under test.
/// </para>
/// </remarks>
public sealed class XlsxTintTests
{
    /// <summary>Excel writes 80% as this, and the exact value matters to the last channel.</summary>
    private const double Lighter80 = 0.79998168889431442;

    /// <summary>
    /// The stock accents at "lighter 80%" come out as the reference paints them.
    /// </summary>
    /// <remarks>
    /// Both halves are needed. Accent1 clamps one channel under the offset form and accent4
    /// clamps two, so a fix that merely stopped clamping — by scaling instead of offsetting, say
    /// — would still fail one of them.
    /// </remarks>
    [Theory]
    // accent1: the offset form gave #A6D4FF, blue clamped at 255.
    [InlineData(0x4472C4u, Lighter80, 0xDAE3F3u)]
    // accent4: the offset form gave #FFFF66, red and green both clamped — gold turned lemon.
    [InlineData(0xFFC000u, Lighter80, 0xFFF2CCu)]
    public void AStockAccentTintedLighterMatchesTheReference(
        uint basis, double tint, uint expected)
    {
        XlsxTint.Apply(Colour.FromRgb(basis), tint)
            .ShouldBe(Colour.FromRgb(expected));
    }

    /// <summary>A negative tint darkens by scaling the luminance, leaving hue alone.</summary>
    /// <remarks>
    /// <para>
    /// <c>addExcelTintTransformation</c> emits <c>lumMod</c> alone for a negative tint and no
    /// <c>lumOff</c> (<c>oox/source/drawingml/color.cxx:497-509</c>), so the colour travels
    /// towards black along its own hue rather than being blended with black.
    /// </para>
    /// <para>
    /// All three values are the reference's, counted rather than sampled: rendering
    /// <c>sheets/batch-*/xlsx/fse_identification_form.xlsx</c> — whose fills are these three
    /// theme-and-tint pairs — at 100 dpi puts <c>#2F5597</c> on 64,680 pixels of the reference
    /// and 64,587 of ours, with <c>#535353</c> and <c>#F2F2F2</c> likewise present in both.
    /// </para>
    /// <para>
    /// <strong>An earlier revision of this test asserted <c>#1F3864</c> for accent1 at −0.5</strong>
    /// — Excel's own documented "Darker 50%" — and it is wrong by one on red. Excel is not the
    /// reference here and its published constants are a false witness for the same reason the
    /// literal fill in <c>template-ECSPR-notifications.xlsx</c> is: it rounds the other way.
    /// </para>
    /// </remarks>
    [Theory]
    // accent1 at "darker 25%".
    [InlineData(0x4472C4u, -0.249977111117893, 0x2F5597u)]
    // accent3, a neutral, at "darker 50%" — it must stay neutral.
    [InlineData(0xA5A5A5u, -0.499984740745262, 0x535353u)]
    // lt1, white, nudged off white.
    [InlineData(0xFFFFFFu, -4.9989318521683403E-2, 0xF2F2F2u)]
    public void ANegativeTintDarkensAsTheReferencePaintsIt(
        uint basis, double tint, uint expected)
    {
        XlsxTint.Apply(Colour.FromRgb(basis), tint)
            .ShouldBe(Colour.FromRgb(expected));
    }

    /// <summary>A tint of zero is the colour itself, and alpha survives either way.</summary>
    [Fact]
    public void AZeroTintIsTheColourItselfAndAlphaIsCarried()
    {
        Colour translucent = new(0x44, 0x72, 0xC4, 0x80);

        XlsxTint.Apply(translucent, 0).ShouldBe(translucent);
        XlsxTint.Apply(translucent, Lighter80).A.ShouldBe((byte)0x80);
    }

    /// <summary>
    /// A grey has no hue to preserve and stays neutral, at both ends and in between.
    /// </summary>
    /// <remarks>
    /// The saturation-zero branch is its own code path — the general inverse divides by the
    /// chroma — so a neutral is the case most likely to come back tinged rather than grey.
    /// </remarks>
    [Theory]
    [InlineData(0x808080u, 0.5)]
    [InlineData(0x000000u, 0.5)]
    [InlineData(0xFFFFFFu, -0.5)]
    public void AGreyStaysNeutral(uint basis, double tint)
    {
        Colour tinted = XlsxTint.Apply(Colour.FromRgb(basis), tint);

        tinted.R.ShouldBe(tinted.G);
        tinted.G.ShouldBe(tinted.B);
    }

    /// <summary>
    /// White cannot be lightened and black cannot be darkened, and neither overflows.
    /// </summary>
    [Fact]
    public void TheEndsOfTheScaleAreFixedPoints()
    {
        XlsxTint.Apply(Colour.White, Lighter80).ShouldBe(Colour.White);
        XlsxTint.Apply(Colour.Black, -Lighter80).ShouldBe(Colour.Black);
    }

    /// <summary>
    /// No tint ever clamps a channel that the offset form would have clamped.
    /// </summary>
    /// <remarks>
    /// The guard for the actual defect, stated as a property rather than as a value: lightening
    /// a fully saturated primary must keep the other two channels apart, because equal channels
    /// are what a clamped result looks like. Under the offset form <c>#FFC000</c> at 0.8 came
    /// back with red and green both at 255.
    /// </remarks>
    [Fact]
    public void LighteningASaturatedColourDoesNotFlattenItsChannelsTogether()
    {
        Colour tinted = XlsxTint.Apply(Colour.FromRgb(0xFFC000), Lighter80);

        tinted.R.ShouldNotBe(tinted.G);
        tinted.G.ShouldNotBe(tinted.B);
    }
}
