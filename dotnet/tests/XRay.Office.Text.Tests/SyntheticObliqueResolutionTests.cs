using XRay.Office.Core.Graphics;
using XRay.Office.Text.Fonts;
using Shouldly;

namespace XRay.Office.Text.Tests;

/// <summary>
/// Whether a slant has to be drawn is decided by font resolution, because it is the one place that
/// holds the request and the answer at the same time.
/// </summary>
/// <remarks>
/// <para>
/// <c>LogicalFontInstance::NeedsArtificialItalic()</c> is <em>the request is italic and the face is
/// not</em>, and nothing else. Two rival rules were refuted against the installed 26.2.4.2 on an
/// authored five-family deck, three sizes each: that the answer follows the family a document
/// <em>states</em> (it does not — <c>Verdana</c> is not installed, substitutes onto DejaVu Sans and
/// takes DejaVu Sans's answer), and that a real italic face is sheared too (it is not —
/// <c>Liberation Sans</c> and <c>Liberation Serif</c> both have installed italics and neither
/// leans). 15 of 15.
/// </para>
/// <para>
/// The cases below are stated in terms of <em>what is installed here</em> and skip when the
/// premise does not hold, rather than hard-coding a machine's font list into an assertion.
/// </para>
/// </remarks>
public class SyntheticObliqueResolutionTests
{
    [Theory]
    [InlineData("DejaVu Sans")]
    [InlineData("DejaVu Serif")]
    [InlineData("Verdana")]        // not installed; substitutes onto a family with no italic
    public void AFamilyWithNoInstalledItalicHasItsSlantDrawn(string family)
    {
        SystemFontResolver resolver = SystemFontResolver.Build();

        FontReference italic = resolver.Resolve(new FontRequest(family) { IsItalic = true });
        Assert.SkipWhen(italic.FaceKey.Length == 0, $"nothing resolves for {family}");
        Assert.SkipWhen(italic.IsItalic, $"an italic face for {family} is installed here");

        italic.SyntheticOblique.ShouldBeTrue();

        // And the roman half of the same family does not lean, which is what says the flag is
        // about the request rather than about the face.
        resolver.Resolve(new FontRequest(family)).SyntheticOblique.ShouldBeFalse();
    }

    /// <summary>
    /// The same rule, over a font set the test controls, so that it cannot be skipped away.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The three cases above are guarded on what the machine has installed, and once
    /// <see cref="BundledFonts"/> began shipping a complete DejaVu and Liberation set every one
    /// of them started skipping — the families they name all have a real italic now, and
    /// <c>Verdana</c> substitutes onto one that does. Three green skips, and the rule they exist
    /// to check covered by nothing.
    /// </para>
    /// <para>
    /// So this one builds an index over a directory holding exactly one roman face and no italic
    /// at all. It asserts the same two things and depends on no font outside its own temporary
    /// directory, which is the property the others lack: a test whose premise is "this is not
    /// installed" is a test that a later change to what ships can silently empty.
    /// </para>
    /// </remarks>
    [Fact]
    public void ARomanOnlyFamilyHasItsSlantDrawn()
    {
        string roman = SystemFontResolver.Build()
            .Resolve(new FontRequest("DejaVu Serif")).FaceKey.Split('#')[0];
        Assert.SkipWhen(!File.Exists(roman), "no DejaVu Serif to copy");

        string directory = Path.Combine(Path.GetTempPath(), $"pl-oblique-{Guid.NewGuid():N}");
        Directory.CreateDirectory(directory);
        try
        {
            File.Copy(roman, Path.Combine(directory, Path.GetFileName(roman)));

            SystemFontResolver resolver = new(SystemFontIndex.Build([directory]));
            FontReference italic =
                resolver.Resolve(new FontRequest("DejaVu Serif") { IsItalic = true });

            italic.IsItalic.ShouldBeFalse("the directory holds no italic face");
            italic.SyntheticOblique.ShouldBeTrue();
            resolver.Resolve(new FontRequest("DejaVu Serif")).SyntheticOblique.ShouldBeFalse();
        }
        finally
        {
            Directory.Delete(directory, recursive: true);
        }
    }

    [Theory]
    [InlineData("Liberation Sans")]
    [InlineData("Liberation Serif")]
    public void AFamilyWithAnInstalledItalicUsesIt(string family)
    {
        SystemFontResolver resolver = SystemFontResolver.Build();

        FontReference italic = resolver.Resolve(new FontRequest(family) { IsItalic = true });
        Assert.SkipUnless(italic.IsItalic, $"no italic face for {family} is installed here");

        italic.SyntheticOblique.ShouldBeFalse();
    }

    [Fact]
    public void TheShearIsTheReferencesOwnAndNotAThird()
    {
        // `ARTIFICIAL_ITALIC_SKEW` is float((1<<16)/3)/(1<<16) = 0.3333333432674408, and
        // `Matrix3::skew` takes it as an *angle*, so the number that reaches a page is its tangent.
        // A shear of exactly one third would be wrong in the fourth decimal — which is the decimal
        // a PDF number keeps — so this is not a pedantic distinction.
        FontReference.SyntheticObliqueShear.ShouldBe(
            Math.Tan(0.3333333432674408), tolerance: 5e-10);

        Math.Round(FontReference.SyntheticObliqueShear, 4)
            .ShouldNotBe(Math.Round(1.0 / 3.0, 4));
    }
}
