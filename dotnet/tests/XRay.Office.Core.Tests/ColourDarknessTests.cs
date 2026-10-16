using XRay.Office.Core.Graphics;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// Whether text on a colour is reversed out of it — <c>Color::IsDark</c>.
/// </summary>
/// <remarks>
/// <para>
/// Every number here was measured on the reference binary rather than read off the C++ tree in this
/// checkout, which is 27.2 and not 26.2.4.2: <c>dotnet/probes/words-r59/autocolour.py</c> authors a
/// one-cell table per fill and reads the colour its glyphs are drawn in, and round 58's
/// <c>label-and-autocolour.py</c> did the same over a 22-step ramp.
/// </para>
/// <para>
/// <strong>The point of the file is the exception.</strong> <c>IsDark</c> is two formulas, and the
/// second applies to exactly one colour in the whole domain.
/// </para>
/// <para>
/// Reintroducing the bug to check these fail: drop the <c>DefaultShapeFilling</c> arm from
/// <c>Colour.IsDark</c>, or replace <c>WcagLuminance</c> with <c>PerceivedLuminance</c>.
/// </para>
/// </remarks>
public sealed class ColourDarknessTests
{
    /// <summary>
    /// The WCAG threshold, at the single sRGB step where the reference changes its answer.
    /// </summary>
    [Theory]
    [InlineData(0x9C9C9Cu, true)]
    [InlineData(0x9D9D9Du, true)]
    [InlineData(0x9E9E9Eu, true)]
    [InlineData(0x9F9F9Fu, false)]
    [InlineData(0xA0A0A0u, false)]
    [InlineData(0x000000u, true)]
    [InlineData(0xFFFFFFu, false)]
    public void TheGreyRampTurnsBetween9EAnd9F(uint rgb, bool dark)
    {
        Colour.FromRgb(rgb).IsDark.ShouldBe(dark);
    }

    /// <summary>Each primary, where a perceived-luminance rule would answer differently for green.</summary>
    [Theory]
    [InlineData(0xFF0000u, true)]
    [InlineData(0x00FF00u, false)]
    [InlineData(0x0000FFu, true)]
    [InlineData(0x008000u, true)]
    [InlineData(0x000080u, true)]
    [InlineData(0xFFFF00u, false)]
    [InlineData(0x00FFFFu, false)]
    public void ThePrimariesAgreeWithTheReference(uint rgb, bool dark)
    {
        Colour.FromRgb(rgb).IsDark.ShouldBe(dark);
    }

    /// <summary>
    /// <c>COL_DEFAULT_SHAPE_FILLING</c> is the one colour asked the other question.
    /// </summary>
    /// <remarks>
    /// <c>0x729FCF</c> has WCAG luminance 83 — under the 87 threshold, so every other colour with that
    /// luminance is dark — and perceived luminance 151, which is far over the 62 the special case asks.
    /// So it is the single input on which the two functions disagree, and the reference draws its text
    /// black. <c>0x6F9BCB</c> is the same colour a few steps away, where the special case does not
    /// apply and the WCAG rule gives white. A port that took either formula alone fails one of these two.
    /// </remarks>
    [Fact]
    public void TheDefaultShapeFillingIsTheOneColourThatTakesThePerceivedRule()
    {
        Colour defaultFilling = Colour.FromRgb(0x729FCF);

        defaultFilling.WcagLuminance.ShouldBeLessThanOrEqualTo(87);
        defaultFilling.PerceivedLuminance.ShouldBeGreaterThan(62);
        defaultFilling.IsDark.ShouldBeFalse();

        Colour neighbour = Colour.FromRgb(0x6F9BCB);
        neighbour.WcagLuminance.ShouldBeLessThanOrEqualTo(87);
        neighbour.IsDark.ShouldBeTrue();
    }

    /// <summary>The exception is the colour and not the alpha it carries.</summary>
    [Fact]
    public void TheExceptionIgnoresAlpha()
    {
        Colour.FromRgb(0x729FCF).WithAlpha(128).IsDark.ShouldBeFalse();
    }

    /// <summary>Both luminances are the C++ functions, at values that pin their arithmetic.</summary>
    [Theory]
    [InlineData(0x000000u, 0, 0)]
    [InlineData(0xFFFFFFu, 255, 255)]
    [InlineData(0x9E9E9Eu, 87, 158)]
    [InlineData(0x9F9F9Fu, 88, 159)]
    [InlineData(0x729FCFu, 83, 151)]
    public void TheTwoLuminancesAreTheirOwnFunctions(uint rgb, int wcag, int perceived)
    {
        Colour colour = Colour.FromRgb(rgb);
        colour.WcagLuminance.ShouldBe(wcag);
        colour.PerceivedLuminance.ShouldBe(perceived);
    }
}
