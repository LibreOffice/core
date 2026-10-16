using XRay.Office.Core.Graphics;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// An automatic font colour over a <em>semi-transparent</em> background, and the two corpus
/// witnesses that term reconciles.
/// </summary>
/// <remarks>
/// <para>
/// Round 59 measured that <c>docs-quality-MA.IMS.00001-…docx</c> page 9 and
/// <c>069_Work_Breakdown_Structure_Template_Professional_Format</c> draw <b>black</b> text on fills
/// that are dark by <c>Color::IsDark</c> — <c>#0070C0</c> at WCAG 39 and <c>#8496B0</c> at 76 — and
/// removed the arm that passes a shape's fill down as the background. Round 62 then measured, on
/// four inverted arms of <c>012</c>, that a text box's own fill <em>does</em> decide, and shipped
/// nothing because the two contradicted each other.
/// </para>
/// <para>
/// Neither measurement is wrong. <c>ApplyAutoColor</c> does not ask the fill for its colour; it
/// asks <c>SdrAllFillAttributesHelper::getAverageColor(aGlobalRetoucheColor)</c>, which blends the
/// fill toward white by its transparency. <b>Both witnesses state one</b> —
/// <c>&lt;a:alpha val="52941"/&gt;</c> and <c>&lt;v:fill opacity="26214f"/&gt;</c> — and blended
/// they are luminance 105 and 168, comfortably bright.
/// </para>
/// <para>
/// The numbers below are not fitted. <c>probes/words-r63/threshold.py</c> renders eleven arms
/// against LibreOffice 26.2.4.2 straddling three <em>different</em> predicted flip transparencies —
/// 9.571 %, 37.454 % and 62.222 % for those three colours — and all eleven land where the blend
/// says. A constant threshold cannot produce three different answers, which is the point of using
/// three colours rather than one.
/// </para>
/// </remarks>
public sealed class TranslucentAutoColourTests
{
    private static readonly Colour Ims = new(0x00, 0x70, 0xC0);
    private static readonly Colour Wbs = new(0x84, 0x96, 0xB0);

    /// <summary>Opaque, both witnesses are dark and their text reverses out. That is round 62.</summary>
    [Fact]
    public void AnOpaqueDarkFillStillReversesTheTextOut()
    {
        AutomaticColour.Over(Ims).ShouldBe(Colour.White);
        AutomaticColour.Over(Wbs).ShouldBe(Colour.White);
        Ims.IsDark.ShouldBeTrue("WCAG 39");
        Wbs.IsDark.ShouldBeTrue("WCAG 76");
    }

    /// <summary>
    /// At the transparency the two documents actually state, the text is black. That is round 59.
    /// </summary>
    /// <remarks>
    /// <c>a:alpha val="52941"</c> is 52.941 % opacity, which is alpha 135 of 255;
    /// <c>v:fill opacity="26214f"</c> is 26214/65536 = 0.4, which is alpha 102. The blends are
    /// <c>#78B3DE</c> and <c>#CED5DF</c>, luminance 105 and 168.
    /// </remarks>
    [Fact]
    public void TheTransparencyTheWitnessesStateMakesThemBright()
    {
        AutomaticColour.Over(Ims.WithAlpha(135)).ShouldBe(Colour.Black, "a:alpha val=\"52941\"");
        AutomaticColour.Over(Wbs.WithAlpha(102)).ShouldBe(Colour.Black, "v:fill opacity=\"26214f\"");

        AutomaticColour.Averaged(Ims.WithAlpha(135)).WcagLuminance.ShouldBe(105);
        AutomaticColour.Averaged(Wbs.WithAlpha(102)).WcagLuminance.ShouldBe(168);
    }

    /// <summary>
    /// Each colour flips at its own alpha, one step apart, and the three are not the same step.
    /// </summary>
    /// <remarks>
    /// The discriminating assertion of the whole change: a rule that ignored the fill colour, or
    /// applied a constant transparency threshold, would put all three flips in one place. Each pair
    /// is a single unit of alpha, so nothing here has room to be approximately right — and each is
    /// the byte-alpha neighbour of the transparency the reference itself flips at.
    /// </remarks>
    [Theory]
    [InlineData(0x84, 0x96, 0xB0, 231, 230)]
    [InlineData(0x00, 0x70, 0xC0, 160, 159)]
    [InlineData(0x00, 0x00, 0x00, 97, 96)]
    public void EachFillFlipsAtItsOwnAlpha(int r, int g, int b, int dark, int bright)
    {
        Colour fill = new((byte)r, (byte)g, (byte)b);

        AutomaticColour.Over(fill.WithAlpha((byte)dark))
            .ShouldBe(Colour.White, $"alpha {dark} still blends to a dark colour");
        AutomaticColour.Over(fill.WithAlpha((byte)bright))
            .ShouldBe(Colour.Black, $"one step more transparent, alpha {bright}, does not");

        AutomaticColour.Averaged(fill.WithAlpha((byte)dark)).WcagLuminance.ShouldBe(87);
        AutomaticColour.Averaged(fill.WithAlpha((byte)bright)).WcagLuminance.ShouldBe(88);
    }

    /// <summary>An opaque background is unchanged by the blend, which is every table cell.</summary>
    /// <remarks>
    /// The control that says this change cannot touch the shading path round 59 shipped: a
    /// <c>w:shd</c> fill is always opaque, so <see cref="AutomaticColour.Averaged"/> must be the
    /// identity on it — including for <c>COL_DEFAULT_SHAPE_FILLING</c>, the one colour
    /// <see cref="Colour.IsDark"/> answers with the other formula.
    /// </remarks>
    [Fact]
    public void AnOpaqueBackgroundIsUntouched()
    {
        foreach (Colour colour in (Colour[])
                 [Colour.Black, Colour.White, new(0x72, 0x9F, 0xCF), new(0x9E, 0x9E, 0x9E)])
        {
            AutomaticColour.Averaged(colour).ShouldBe(colour);
        }

        AutomaticColour.Over(new Colour(0x72, 0x9F, 0xCF))
            .ShouldBe(Colour.Black, "COL_DEFAULT_SHAPE_FILLING is asked the perceived luminance");
        AutomaticColour.Over(new Colour(0x6F, 0x9B, 0xCB))
            .ShouldBe(Colour.White, "and one sRGB step away is asked the WCAG one");
    }

    /// <summary>No background at all is black, however the caller spells it.</summary>
    [Fact]
    public void NoBackgroundIsBlack()
    {
        AutomaticColour.Over(default).ShouldBe(Colour.Black);
        AutomaticColour.Over(Colour.Black.WithAlpha(0)).ShouldBe(Colour.Black);
    }
}
