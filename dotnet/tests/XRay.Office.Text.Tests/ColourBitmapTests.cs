using XRay.Office.Text.Fonts;
using Shouldly;

namespace XRay.Office.Text.Tests;

/// <summary>
/// Tests the <c>CBLC</c>/<c>CBDT</c> reader that colour bitmap faces are drawn from.
/// </summary>
/// <remarks>
/// <para>
/// The placement assertions are against LibreOffice 26.2.4.2's own numbers rather than against
/// re-derived ones. Its PDF of a <c>U+2714</c> probe draws the glyph in a Type 3 char proc as
/// <c>q 1247.55859375 0 0 1174.31640625 0 -247.55859375 cm /Im12 Do Q</c> under a
/// <c>/FontMatrix[0.001 …]</c>, which over Noto Color Emoji's 2048 units per em is a box 2555 wide
/// and 2405 tall whose bottom is 507 units below the baseline. Those three integers are what this
/// asserts, so a reader that scales or rounds differently fails here rather than on a page.
/// </para>
/// <para>
/// Noto Color Emoji because it is the only colour face installed and the one glyph fallback
/// actually answers with; a machine without it skips.
/// </para>
/// </remarks>
public class ColourBitmapTests
{
    private static OpenTypeFace Require()
    {
        string[] candidates =
        [
            "/usr/share/fonts/truetype/noto/NotoColorEmoji.ttf",
            "/usr/share/fonts/truetype/noto/NotoColorEmoji-Regular.ttf",
        ];

        string? path = Array.Find(candidates, File.Exists);
        Assert.SkipWhen(path is null, "Noto Color Emoji is not installed; see check-env.sh");

        OpenTypeFace? face = OpenTypeFace.ReadFile(path!);
        face.ShouldNotBeNull();
        return face!;
    }

    /// <summary>The face this reader exists for has no outlines at all — the strikes are all it has.</summary>
    [Fact]
    public void AColourFaceCarriesStrikesAndNoOutlines()
    {
        OpenTypeFace face = Require();

        ColourBitmaps.Has(face).ShouldBeTrue();
        face.File.Has("glyf").ShouldBeFalse();
        face.File.Has("CFF ").ShouldBeFalse();
    }

    /// <summary>A Liberation face has no strikes, so the reader answers nothing for it.</summary>
    [Fact]
    public void AnOutlineFaceHasNoStrikes()
    {
        string[] candidates =
        [
            "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
            "/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf",
        ];

        string? path = Array.Find(candidates, File.Exists);
        Assert.SkipWhen(path is null, "Liberation Sans is not installed; see check-env.sh");

        OpenTypeFace? face = OpenTypeFace.ReadFile(path!);
        face.ShouldNotBeNull();

        ColourBitmaps.Has(face).ShouldBeFalse();
        ColourBitmaps.Of(face, face!.Characters.GlyphFor('A')).ShouldBeNull();
    }

    /// <summary>The strike is a whole PNG, at the strike's own resolution and bearings.</summary>
    [Theory]
    [InlineData(0x2714)]
    [InlineData(0x2611)]
    [InlineData(0x263A)]
    public void AColourGlyphDecodesToAPngWithItsOwnMetrics(int codePoint)
    {
        OpenTypeFace face = Require();
        ushort glyph = face.Characters.GlyphFor(codePoint);
        glyph.ShouldNotBe((ushort)0);

        ColourBitmap? bitmap = ColourBitmaps.Of(face, glyph);
        bitmap.ShouldNotBeNull();

        bitmap!.MediaType.ShouldBe("image/png");
        bitmap.Image.Length.ShouldBeGreaterThan(8);
        bitmap.Image.Span[..4].ToArray().ShouldBe([0x89, (byte)'P', (byte)'N', (byte)'G']);

        bitmap.PixelsPerEmX.ShouldBeGreaterThan(0);
        bitmap.PixelsPerEmY.ShouldBeGreaterThan(0);
        bitmap.PixelWidth.ShouldBeGreaterThan(0);
        bitmap.PixelHeight.ShouldBeGreaterThan(0);
    }

    /// <summary>A character the face does not cover has no strike either.</summary>
    [Fact]
    public void ACharacterTheFaceLacksHasNoStrike()
    {
        OpenTypeFace face = Require();

        face.Characters.GlyphFor('A').ShouldBe((ushort)0);
        ColourBitmaps.Of(face, 0).ShouldBeNull();
    }

    /// <summary>The box the strike goes in is the reference's own, to the design unit.</summary>
    [Fact]
    public void ThePlacementIsTheOneLibreOfficeWrites()
    {
        OpenTypeFace face = Require();
        ColourBitmap? bitmap = ColourBitmaps.Of(face, face.Characters.GlyphFor(0x2714));
        bitmap.ShouldNotBeNull();

        face.UnitsPerEm.ShouldBe(2048);
        (int left, int bottom, int width, int height) = bitmap!.PlacementIn(face.UnitsPerEm);

        left.ShouldBe(0);
        width.ShouldBe(2555);
        height.ShouldBe(2405);
        bottom.ShouldBe(-507);
    }

    /// <summary>
    /// The box scales with the em, so the same strike states the same fraction of any face size.
    /// </summary>
    /// <remarks>
    /// A control on the arithmetic rather than on the face: doubling the units per em doubles every
    /// number, which a reader that had folded the strike's own ppem into a constant would fail. The
    /// numbers are not exactly twice the 2048 ones and must not be — each is rounded to a whole
    /// design unit at its own em, so 2555 is 5110.6 rounded once and 5111 rounded once, and asserting
    /// exact doubling here would be asserting that the rounding is not happening.
    /// </remarks>
    [Fact]
    public void ThePlacementScalesWithTheEm()
    {
        OpenTypeFace face = Require();
        ColourBitmap? bitmap = ColourBitmaps.Of(face, face.Characters.GlyphFor(0x2714));
        bitmap.ShouldNotBeNull();

        (int _, int bottom, int width, int height) = bitmap!.PlacementIn(4096);

        width.ShouldBe(5111);
        height.ShouldBe(4810);
        bottom.ShouldBe(-1015);
    }

    /// <summary>A colour face is paintable through its strikes and an outline face through its outlines.</summary>
    [Fact]
    public void PaintabilityFollowsWhicheverTableHoldsTheGlyph()
    {
        OpenTypeFace face = Require();

        GlyphPainting.CanPaintCharacter(face, 0x2714).ShouldBeTrue();

        // Covered by nothing in this face, so there is neither a glyph nor a strike to paint.
        GlyphPainting.CanPaintCharacter(face, 'A').ShouldBeFalse();
    }
}
