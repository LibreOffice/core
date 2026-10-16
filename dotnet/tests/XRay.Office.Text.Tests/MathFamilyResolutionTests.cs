using XRay.Office.Core.Graphics;
using XRay.Office.Text.Fonts;
using Shouldly;

namespace XRay.Office.Text.Tests;

/// <summary>
/// A family fontconfig files under <c>math</c> is answered by the mathematical orthography, not by
/// a shape.
/// </summary>
/// <remarks>
/// <para>
/// <c>45-generic.conf</c> files <c>Cambria Math</c> and six siblings under <c>math</c>, and in the
/// same file prepends <c>lang=und-zmth</c> to any pattern whose family is <c>math</c>. fontconfig
/// ranks a language requirement above weight, above slant and above the generic
/// <c>FontConfigManager::Substitute</c> appends for a declared family class, so the answer is the
/// same whatever the document said. Only one installed file declares that orthography here:
/// <c>fc-list :lang=und-zmth</c> answers <c>FreeSerif.ttf</c> — the regular, not the bold.
/// </para>
/// <para>
/// Measured on 26.2.4.2 over the four corpus documents that name Cambria Math —
/// <c>DRX-Ascend System Course Description.docx</c> (a <c>U+2010</c> hyphen, seven times),
/// <c>075_Storyboard_Template_Fillable_Format…docx</c> (a 48 pt <c>w:b</c> title),
/// <c>084_Printable_Graph_Paper_Template…docx</c> and
/// <c>AAC-AD-No-2021-01-Boeing-737-8-and-737-9-MAX.doc</c>. The reference draws all four in
/// FreeSerif; we drew them in DejaVu Serif, and the bold one in FreeSerif <em>Bold</em> at the
/// intermediate stage of the fix.
/// </para>
/// <para>
/// These read the machine's own fontconfig and font set, so each states its premise first rather
/// than asserting into thin air: a machine without GNU FreeFont has no face declaring the maths
/// orthography and the pattern correctly falls through to the configuration's overall default.
/// </para>
/// </remarks>
public sealed class MathFamilyResolutionTests
{
    private static readonly SystemFontResolver Resolver = SystemFontResolver.Build();

    private static bool FreeSerifIsInstalled =>
        SystemFontIndex.Build().Has("FreeSerif");

    private static bool CambriaMathIsFiledUnderMath =>
        FontconfigPreferences.Machine.GenericNameOf("Cambria Math") == "math";

    [Fact]
    public void TheConfigurationFilesCambriaMathUnderMath()
    {
        if (!FontconfigPreferences.Machine.IsConfigured) return;

        CambriaMathIsFiledUnderMath.ShouldBeTrue(
            "45-generic.conf files Cambria Math under the math generic");
    }

    [Fact]
    public void ARomanDeclarationDoesNotTurnAMathsFamilyIntoASerif()
    {
        if (!CambriaMathIsFiledUnderMath || !FreeSerifIsInstalled) return;

        // The DOCX, DOC and RTF filters default a named family's class to roman, which is what put
        // this on DejaVu Serif. A lang requirement outranks the generic that declaration appends.
        FontReference resolved = Resolver.Resolve(
            new FontRequest("Cambria Math", 400, false, DeclaredClass: FontFamilyClass.Serif));

        resolved.FamilyName.ShouldBe("FreeSerif");
    }

    [Fact]
    public void ASwissDeclarationDoesNotTurnItIntoAGrotesqueEither()
    {
        if (!CambriaMathIsFiledUnderMath || !FreeSerifIsInstalled) return;

        FontReference resolved = Resolver.Resolve(
            new FontRequest("Cambria Math", 400, false, DeclaredClass: FontFamilyClass.SansSerif));

        resolved.FamilyName.ShouldBe("FreeSerif");
    }

    [Fact]
    public void ABoldRequestStillTakesTheFaceThatDeclaresTheOrthography()
    {
        if (!CambriaMathIsFiledUnderMath || !FreeSerifIsInstalled) return;

        FontReference resolved = Resolver.Resolve(
            new FontRequest("Cambria Math", 700, false, DeclaredClass: FontFamilyClass.Serif));

        // fc-match "Cambria Math:bold" answers FreeSerif Regular where fc-match "FreeSerif:bold"
        // answers FreeSerif Bold: only the regular file declares und-zmth, and lang outranks weight.
        resolved.FamilyName.ShouldBe("FreeSerif");
        resolved.Weight.ShouldBeLessThan(700);
    }

    [Fact]
    public void AnOrdinaryUnknownFamilyIsUnaffected()
    {
        if (!FontconfigPreferences.Machine.IsConfigured) return;

        // The control. Nothing files "Zzzz Nonexistent Family" anywhere, so it still takes the
        // declared class's shape — which is the rule the whole resolver rests on.
        FontReference resolved = Resolver.Resolve(
            new FontRequest("Zzzz Nonexistent Family", 400, false,
                DeclaredClass: FontFamilyClass.Serif));

        resolved.FamilyName.ShouldNotBe("FreeSerif");
    }
}
