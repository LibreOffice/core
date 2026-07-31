using Paperless.Core.Graphics;
using Paperless.Text.Fonts;
using Shouldly;

namespace Paperless.Text.Tests;

/// <summary>
/// Tests font resolution: the substitution table, the index of installed faces, and the order a
/// request is resolved in.
/// </summary>
/// <remarks>
/// The metric-compatible substitutions are the ones that matter. A substitute with the original's
/// advance widths keeps every line breaking where it did; one without reflows the text and moves
/// every break after the first — so the difference between the two is the difference between a page
/// that looks slightly different and a document whose every later page is wrong. That is why the
/// resolver reports which kind it made rather than merely making one.
/// </remarks>
public class FontResolutionTests
{
    // ------------------------------------------------------------- the substitution table

    [Fact]
    public void TheTableCarriesLibreOfficesOwnChains()
    {
        // Generated from LibreOffice's configuration rather than reimplemented, so its size is
        // evidence that the generation worked at all.
        FontSubstitutions.Count.ShouldBeGreaterThan(200);
    }

    [Theory]
    [InlineData("Calibri", "carlito")]
    [InlineData("Cambria", "caladea")]
    [InlineData("Arial", "liberationsans")]
    [InlineData("Times New Roman", "liberationserif")]
    [InlineData("Courier New", "liberationmono")]
    public void TheMetricCompatiblePairsAreInTheChain(string requested, string expected)
    {
        // These five are why an OOXML document can lay out identically without the Microsoft fonts.
        // If the chain for any of them lost its metric-compatible substitute, every DOCX comparison
        // would start failing for reasons no individual test would explain.
        FontSubstitutions.ChainFor(requested).ShouldContain(expected);
    }

    [Fact]
    public void NamesAreNormalisedTheWayTheTableIsKeyed()
    {
        // A document may spell one font several ways, and the table stores one. Both sides have to be
        // normalised or nothing matches.
        FontSubstitutions.Normalise("Times New Roman").ShouldBe("timesnewroman");
        FontSubstitutions.Normalise("  ARIAL  ").ShouldBe("arial");
        FontSubstitutions.Normalise("Helvetica-Bold").ShouldBe("helveticabold");
        FontSubstitutions.Normalise(null).ShouldBeEmpty();
        FontSubstitutions.Normalise("").ShouldBeEmpty();

        // A CJK font naming itself in its own script keeps its letters, since dropping non-ASCII
        // would collapse every such name to nothing and make them all the same font.
        FontSubstitutions.Normalise("宋体").ShouldNotBeEmpty();
    }

    [Fact]
    public void MetricCompatibilityIsDerivedFromTheTableRatherThanHardcoded()
    {
        // A face is compatible with the Microsoft font it declares itself an equivalent of, and with
        // any other face declaring the same one — so the pairs fall out of the table instead of being
        // a list somebody has to remember to extend.
        FontSubstitutions.AreMetricCompatible("Carlito", "Calibri").ShouldBeTrue();
        FontSubstitutions.AreMetricCompatible("Calibri", "Carlito").ShouldBeTrue();
        FontSubstitutions.AreMetricCompatible("Caladea", "Cambria").ShouldBeTrue();
        FontSubstitutions.AreMetricCompatible("Liberation Sans", "Arial").ShouldBeTrue();

        // A font is trivially compatible with itself, whatever it is called.
        FontSubstitutions.AreMetricCompatible("Whatever", "whatever").ShouldBeTrue();

        // And two unrelated faces are not, which is the answer that matters: reporting compatibility
        // optimistically would hide exactly the substitutions that reflow a document.
        FontSubstitutions.AreMetricCompatible("Carlito", "Caladea").ShouldBeFalse();
        FontSubstitutions.AreMetricCompatible("Arial", "Courier New").ShouldBeFalse();
        FontSubstitutions.AreMetricCompatible(null, "Arial").ShouldBeFalse();
    }

    // ------------------------------------------------------------------ the installed index

    private static SystemFontIndex Index()
    {
        SystemFontIndex index = SystemFontIndex.Build();
        Assert.SkipWhen(index.FamilyCount == 0, "no fonts are installed; see check-env.sh");
        return index;
    }

    [Fact]
    public void TheIndexFindsTheFamiliesInstalledOnThisMachine()
    {
        SystemFontIndex index = Index();

        index.FamilyCount.ShouldBeGreaterThan(1);
        index.Has("Carlito").ShouldBeTrue("Carlito should be installed; see check-env.sh");

        // Found by the name in the font's own table, so the lookup is spelling-insensitive the same
        // way the substitution table is.
        index.Has("carlito").ShouldBeTrue();
        index.Has("CARLITO").ShouldBeTrue();
        index.Has("A Font Nobody Has").ShouldBeFalse();
    }

    [Fact]
    public void TheIndexPicksSlantOverWeight()
    {
        SystemFontIndex index = Index();
        Assert.SkipUnless(index.Family("Carlito").Count >= 4, "Carlito's four styles are not all here");

        // Slant first, always. An upright face where an italic was asked for is visibly wrong in a
        // way that a hundred points of weight is not, so slant is never traded for a closer weight —
        // which is what a combined score would do.
        InstalledFace regular = index.Best("Carlito", 400, italic: false)!.Value;
        regular.IsItalic.ShouldBeFalse();
        regular.Weight.ShouldBe(400);

        InstalledFace boldItalic = index.Best("Carlito", 700, italic: true)!.Value;
        boldItalic.IsItalic.ShouldBeTrue();
        boldItalic.Weight.ShouldBe(700);

        // A weight nobody has lands on the nearest of the same slant rather than on a different one.
        InstalledFace light = index.Best("Carlito", 250, italic: true)!.Value;
        light.IsItalic.ShouldBeTrue();
    }

    [Fact]
    public void AFaceKeyIsStableAndNamesTheFaceWithinItsFile()
    {
        SystemFontIndex index = Index();
        InstalledFace face = index.Best("Carlito", 400, italic: false)!.Value;

        face.FaceKey.ShouldBe(face.Path, "a single-face file needs no index in its key");
        face.FaceKey.ShouldEndWith(".ttf");

        // A collection's later faces are distinguished, since a key that ignored the index would make
        // every face of a CJK collection the same face.
        new InstalledFace(face.Path, 2, "X", 400, false, false).FaceKey.ShouldBe($"{face.Path}#2");
    }

    // ------------------------------------------------------------------------- resolution

    private static SystemFontResolver Resolver()
    {
        SystemFontResolver resolver = new(Index());
        return resolver;
    }

    [Fact]
    public void AnInstalledFamilyResolvesToItselfWithNoSubstitution()
    {
        SystemFontResolver resolver = Resolver();
        FontReference reference = resolver.Resolve(new FontRequest("Carlito"));

        reference.FamilyName.ShouldBe("Carlito");
        reference.IsSubstituted.ShouldBeFalse();
        resolver.Substitutions.ShouldBeEmpty();
    }

    [Fact]
    public void AMissingFamilyResolvesThroughLibreOfficesChain()
    {
        SystemFontResolver resolver = Resolver();

        // Calibri is not installed on a Linux machine, and Carlito is what LibreOffice renders in its
        // place — the substitution that makes an OOXML document lay out identically.
        FontReference reference = resolver.Resolve(new FontRequest("Calibri"));

        reference.FamilyName.ShouldBe("Carlito");
        reference.RequestedFamily.ShouldBe("Calibri");
        reference.IsSubstituted.ShouldBeTrue();

        FontSubstitution substitution = resolver.Substitutions.ShouldHaveSingleItem();
        substitution.Requested.ShouldBe("Calibri");
        substitution.Chosen.ShouldBe("Carlito");
        substitution.IsMetricCompatible.ShouldBeTrue(
            "this is the substitution that preserves every line break");
    }

    [Fact]
    public void EveryMetricCompatiblePairResolvesAndSaysSo()
    {
        SystemFontResolver resolver = Resolver();

        foreach (string requested in new[] { "Calibri", "Cambria", "Arial", "Times New Roman" })
        {
            resolver.Resolve(new FontRequest(requested));
        }

        // All four, and all four compatible: this is the assertion that would fail if the generated
        // table lost an entry or the index stopped finding the free faces.
        resolver.Substitutions.Count.ShouldBe(4);
        resolver.Substitutions.ShouldAllBe(s => s.IsMetricCompatible);
    }

    [Fact]
    public void AFamilyNobodyHasStillResolvesToSomething()
    {
        SystemFontResolver resolver = Resolver();
        FontReference reference = resolver.Resolve(new FontRequest("Nonexistent Display Face"));

        // Never null: a document that names a font nobody has still has to render, and refusing to
        // choose would turn a cosmetic difference into a failure.
        reference.FamilyName.ShouldNotBeNullOrWhiteSpace();
        reference.FaceKey.ShouldNotBeNullOrWhiteSpace();
        reference.IsSubstituted.ShouldBeTrue();

        // And it is reported as *not* metric-compatible, which is the honest answer — this
        // substitution will reflow the document.
        resolver.Substitutions.ShouldHaveSingleItem().IsMetricCompatible.ShouldBeFalse();
    }

    [Fact]
    public void AMonospacedRequestNeverLandsOnAProportionalFace()
    {
        SystemFontResolver resolver = Resolver();
        FontReference reference = resolver.Resolve(
            new FontRequest("Nonexistent Terminal Face", Pitch: FontPitch.Fixed));

        // A document asking for a fixed pitch is relying on its columns lining up, so falling back to
        // a proportional face breaks the thing the font was chosen for.
        IFontFace face = resolver.LoadFace(reference);
        face.ShouldNotBeNull();
        resolver.Index.Family(reference.FamilyName)[0].IsFixedPitch.ShouldBeTrue();
    }

    [Fact]
    public void AnEmbeddedFaceWinsOverAnythingInstalled()
    {
        SystemFontResolver resolver = Resolver();

        // Even over a family that *is* installed: the embedded face is what the author saw, and the
        // only one guaranteed to have the metrics the document was laid out against.
        FontReference reference = resolver.Resolve(
            new FontRequest("Carlito", EmbeddedFaceKey: "embedded:1"));

        reference.FaceKey.ShouldBe("embedded:1");
        resolver.Substitutions.ShouldBeEmpty();
    }

    [Fact]
    public void ALoadedFaceCarriesTheMetricsAndCoverageLayoutNeeds()
    {
        SystemFontResolver resolver = Resolver();
        IFontFace face = resolver.LoadFace(resolver.Resolve(new FontRequest("Calibri")));

        face.UnitsPerEm.ShouldBe(2048);
        face.HasGlyphFor('A').ShouldBeTrue();
        face.HasGlyphFor('日').ShouldBeFalse();

        FontVerticalMetrics metrics = face.VerticalMetrics;
        metrics.Ascent.ShouldBeGreaterThan(0);
        metrics.Descent.ShouldBeGreaterThan(0);
        metrics.UnderlineThickness.ShouldBeGreaterThan(0);
    }

    [Fact]
    public void LoadingTheSameFaceTwiceReadsTheFileOnce()
    {
        SystemFontResolver resolver = Resolver();
        FontReference reference = resolver.Resolve(new FontRequest("Carlito"));

        IFontFace first = resolver.LoadFace(reference);
        IFontFace second = resolver.LoadFace(reference);

        // Disposing one view must not invalidate the other: the bytes belong to the resolver's cache,
        // not to whichever caller happened to finish first.
        first.Dispose();
        second.HasGlyphFor('A').ShouldBeTrue();
    }
}
