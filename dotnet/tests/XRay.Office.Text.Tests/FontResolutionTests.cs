using XRay.Office.Core.Graphics;
using XRay.Office.Text.Fonts;
using Shouldly;

namespace XRay.Office.Text.Tests;

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

    // ------------------------------------------------- the shape a failed chain falls back to

    [Theory]
    [InlineData("Tahoma", FontFamilyClass.SansSerif)]
    [InlineData("Verdana", FontFamilyClass.SansSerif)]
    [InlineData("Century Gothic", FontFamilyClass.SansSerif)]
    [InlineData("Garamond", FontFamilyClass.Serif)]
    [InlineData("Georgia", FontFamilyClass.Serif)]
    [InlineData("Times New Roman", FontFamilyClass.Serif)]
    [InlineData("Courier New", FontFamilyClass.Fixed)]
    [InlineData("Wingdings", FontFamilyClass.Symbol)]
    public void TheTableSaysWhatShapeAFamilyIs(string requested, FontFamilyClass expected)
    {
        // Read from the same VCL.xcu node as the chain, out of its FontType property. This is the
        // half of the entry that decides what happens when *none* of the chain is installed — which
        // on a Linux box is the usual outcome, since the chains name Microsoft and Agfa faces.
        FontSubstitutions.ClassOf(requested).ShouldBe(expected);
    }

    [Fact]
    public void AFamilyTheTableNeverHeardOfHasNoShape()
    {
        // Reported as unknown rather than guessed at, so the resolver decides what to do about it in
        // one place instead of the table pretending to a certainty it does not have.
        FontSubstitutions.ClassOf("Nonexistent Display Face").ShouldBe(FontFamilyClass.Unknown);
        FontSubstitutions.ClassOf(null).ShouldBe(FontFamilyClass.Unknown);
    }

    [Fact]
    public void AGrotesqueWhoseChainIsAbsentFallsBackToASansFace()
    {
        SystemFontResolver resolver = Resolver();
        Assert.SkipUnless(resolver.Index.Has("DejaVu Sans"), "DejaVu Sans is not installed");

        // Tahoma's chain names fourteen faces and this machine has none of them, so resolution
        // reaches the generic fallback. That fallback used to guess the shape from the family name,
        // and nothing in "Tahoma" says grotesque — so a sans-serif document was rendered in a roman.
        FontReference reference = resolver.Resolve(new FontRequest("Tahoma"));

        FontSubstitutions.ClassOf(reference.FamilyName).ShouldNotBe(FontFamilyClass.Serif);
        reference.FamilyName.ShouldBe("DejaVu Sans");
    }

    [Fact]
    public void AFamilyNobodyHasAtAllFallsBackToSansRatherThanSerif()
    {
        SystemFontResolver resolver = Resolver();
        Assert.SkipUnless(resolver.Index.Has("DejaVu Sans"), "DejaVu Sans is not installed");

        // Aptos is Microsoft 365's current default and postdates the table entirely, so neither half
        // of the entry exists. LibreOffice answers this case by asking fontconfig, whose reply for a
        // name it does not recognise is its default family — measured here as DejaVu Sans, for Aptos
        // and for every other unrecognised family probed.
        resolver.Resolve(new FontRequest("Aptos")).FamilyName.ShouldBe("DejaVu Sans");
        resolver.Resolve(new FontRequest("Segoe UI")).FamilyName.ShouldBe("DejaVu Sans");
    }

    [Fact]
    public void AGenericFallbackPrefersDejaVuOverLiberation()
    {
        SystemFontResolver resolver = Resolver();
        Assert.SkipUnless(
            resolver.Index.Has("DejaVu Serif") && resolver.Index.Has("Liberation Serif"),
            "both families are needed to tell the preference apart");

        // Liberation is the metric-compatible stand-in for Arial, Times New Roman and Courier New
        // specifically, and those three reach it through their chains. A request that got this far is
        // for something Liberation was never built to imitate, so its metrics carry no authority —
        // and LibreOffice, which asks fontconfig at this point, lands on DejaVu.
        resolver.Resolve(new FontRequest("Garamond")).FamilyName.ShouldBe("DejaVu Serif");

        // The chains still win where they name something installed, so the pairs that preserve every
        // line break are untouched by the reordering.
        resolver.Resolve(new FontRequest("Arial")).FamilyName.ShouldBe("Liberation Sans");
        resolver.Resolve(new FontRequest("Times New Roman")).FamilyName.ShouldBe("Liberation Serif");
    }

    [Fact]
    public void ARequestNamingNoFamilyGetsTheApplicationDefaultRatherThanTheUnknownFamilyAnswer()
    {
        SystemFontResolver resolver = Resolver();
        Assert.SkipUnless(resolver.Index.Has("Liberation Serif"), "Liberation Serif is not installed");

        // The two cases look alike and are not. "A font nobody has" is fontconfig's question, and its
        // answer is DejaVu Sans; "no font named" is the document expressing no preference, and its
        // answer is the default template's face. Conflating them sets every document that specifies
        // no font in DejaVu Sans — which is a reflow of the entire corpus, not a cosmetic difference.
        resolver.Resolve(new FontRequest("")).FamilyName.ShouldBe("Liberation Serif");
        resolver.Resolve(new FontRequest("   ")).FamilyName.ShouldBe("Liberation Serif");
    }

    [Fact]
    public void ADeclaredFixedPitchStillOutranksTheTablesShape()
    {
        SystemFontResolver resolver = Resolver();
        Assert.SkipUnless(resolver.Index.Has("DejaVu Sans Mono"), "DejaVu Sans Mono is not installed");

        // Tahoma is a grotesque by the table, but a document that marked the request fixed is relying
        // on its columns lining up, and that is the stronger claim of the two.
        FontReference reference = resolver.Resolve(
            new FontRequest("Tahoma", Pitch: FontPitch.Fixed));

        resolver.Index.Family(reference.FamilyName)[0].IsFixedPitch.ShouldBeTrue();
    }

    // ------------------------------------- the chain the running binary does not actually follow

    [Theory]
    [InlineData("Helv")]
    [InlineData("SansSerif")]
    [InlineData("Sans-serif")]
    [InlineData("CG Times")]
    [InlineData("Times-Roman")]
    [InlineData("MS Gothic")]
    public void AChainFontconfigOverridesIsNotConsulted(string requested)
    {
        SystemFontResolver resolver = Resolver();
        Assert.SkipUnless(
            resolver.Index.Has("DejaVu Sans") && resolver.Index.Has("Liberation Sans"),
            "both families are needed to tell the two answers apart");
        Assert.SkipUnless(FontconfigPreferences.Machine.IsConfigured, "no fontconfig on this machine");

        // VCL.xcu sends all of these to an installed Liberation face — the first three through
        // albanyamt/albany, CG Times and Times-Roman through liberationserif, MS Gothic through
        // ipagothic. The running binary reaches none of them: PhysicalFontCollection::FindFontFamily
        // asks the fontconfig pre-match hook at PhysicalFontCollection.cxx:1142 and returns its
        // answer at :1151, while ImplFontSubstitute — the .xcu chain — is only reached at :1180.
        // fontconfig has no <alias> naming any of these six, so it answers with its default family.
        //
        // Measured with a flat-ODS probe drawing "Hamburgefonstiv" and "0123456789" in each: 86.45 pt
        // and 63.64 pt, which is DejaVu Sans, against Liberation Sans's 75.61 and 55.63. On 24.2.7.2
        // for the first three and re-taken on the installed 26.2.4.2 for all six.
        //
        // MS Gothic is the one that looks like it should be exempt, since fontconfig answers by
        // character and this request carries none. It is not: 26.2.4.2 draws Latin text declared
        // MS Gothic in DejaVu Sans and *Japanese* text declared MS Gothic in WenQuanYi Zen Hei, and
        // the chain's IPAGothic is neither. The CJK coverage comes back through glyph fallback.
        resolver.Resolve(new FontRequest(requested)).FamilyName.ShouldBe("DejaVu Sans");
    }

    [Theory]
    [InlineData("Wingdings")]
    [InlineData("Wingdings 2")]
    [InlineData("Webdings")]
    public void APiFaceKeepsItsChain(string requested)
    {
        SystemFontResolver resolver = Resolver();
        Assert.SkipUnless(resolver.Index.Has("OpenSymbol"), "OpenSymbol is not installed");

        // The carve-out on the other side of the same rule, and the reason the table's own
        // classification cannot be deleted along with its use as a shape. A symbol-encoded request
        // makes the pre-match hook bail at fontsubst.cxx:101 before fontconfig is consulted at all,
        // so for a pi face the chain is what LibreOffice runs too — and fontconfig has no symbol
        // generic to have filed these under in any case.
        //
        // The 296-row face probe reads all four of these as DejaVu Sans and is the wrong instrument
        // for them: ODF states no charset, so the probe's own requests were not symbol-encoded. A
        // DOCX or XLSX font carrying charset="2" is.
        resolver.Resolve(new FontRequest(requested)).FamilyName.ShouldBe("OpenSymbol");
    }

    [Theory]
    [InlineData("Helvetica", "Liberation Sans")]
    [InlineData("Albany", "Liberation Sans")]
    [InlineData("Arial", "Liberation Sans")]
    [InlineData("Times", "Liberation Serif")]
    [InlineData("Times New Roman", "Liberation Serif")]
    [InlineData("Courier", "Liberation Mono")]
    [InlineData("Courier New", "Liberation Mono")]
    [InlineData("Calibri", "Carlito")]
    [InlineData("Cambria", "Caladea")]
    [InlineData("Garamond", "DejaVu Serif")]
    [InlineData("Georgia", "DejaVu Serif")]
    [InlineData("Constantia", "DejaVu Serif")]
    [InlineData("Consolas", "DejaVu Sans Mono")]
    [InlineData("Monospace", "DejaVu Sans Mono")]
    public void TheChainsFontconfigAgreesWithAreUntouched(string requested, string expected)
    {
        SystemFontResolver resolver = Resolver();
        Assert.SkipUnless(resolver.Index.Has(expected), $"{expected} is not installed");

        // The negative half, and the reason the chain is suppressed by a rule rather than for
        // everything: the same probe measured all fourteen of these and every one agrees with what
        // this resolver already answered. Suppressing the chain wholesale would move all of them —
        // Helvetica and Albany off Liberation Sans in particular, which are the substitutions that
        // preserve a document's line breaks.
        //
        // What separates these from the names in AChainFontconfigOverridesIsNotConsulted is exactly
        // what the rule tests: fontconfig has an <alias> naming every one of them and none naming
        // any of those, so here its own alias expansion reaches the same face the chain does and
        // there it has nothing to say but a generic.
        resolver.Resolve(new FontRequest(requested)).FamilyName.ShouldBe(expected);
    }

    // ----------------------------------- the shape fontconfig files a name under, not the table's

    [Theory]
    [InlineData("Century Schoolbook", FontFamilyClass.Serif, "DejaVu Sans")]
    [InlineData("NewCenturySchlbk", FontFamilyClass.Serif, "DejaVu Sans")]
    [InlineData("Century", FontFamilyClass.Serif, "DejaVu Sans")]
    [InlineData("Book Antiqua", FontFamilyClass.Serif, "DejaVu Sans")]
    [InlineData("Bookman Old Style", FontFamilyClass.Serif, "DejaVu Sans")]
    [InlineData("Lucida Console", FontFamilyClass.Fixed, "DejaVu Sans")]
    [InlineData("Palatino Linotype", FontFamilyClass.Unknown, "DejaVu Serif")]
    [InlineData("SimSun", FontFamilyClass.Unknown, "DejaVu Serif")]
    public void TheShapeThatDecidesIsFontconfigsRatherThanTheTables(
        string requested, FontFamilyClass tableSays, string expected)
    {
        SystemFontResolver resolver = Resolver();
        Assert.SkipUnless(resolver.Index.Has(expected), $"{expected} is not installed");
        Assert.SkipUnless(FontconfigPreferences.Machine.IsConfigured, "no fontconfig on this machine");

        // The defect this round is about, in one row each. `VCL.xcu` calls the first five romans and
        // Lucida Console monospaced, and has never heard of the last two — and on all eight the
        // running binary follows fontconfig instead. `fc-match "Century Schoolbook"` answers DejaVu
        // Sans, because 30-metric-aliases.conf defaults it to the concrete New Century Schoolbook and
        // nothing files *that* under a generic, so 49-sansserif.conf's sans-serif default is what it
        // gets. Palatino Linotype goes the other way: 45-latin.conf files it under serif.
        //
        // Measured on the installed 26.2.4.2 with a 296-row face probe, one row per family the
        // sample corpus names. The declared-class assertion below is the second half of the same
        // fact: `tableSays` records what the table's own classification is, so a regression that
        // quietly restored it would fail here rather than somewhere in the corpus.
        FontSubstitutions.ClassOf(requested).ShouldBe(tableSays);
        resolver.Resolve(new FontRequest(requested)).FamilyName.ShouldBe(expected);
    }

    [Theory]
    [InlineData("Century Schoolbook", "DejaVu Serif")]
    [InlineData("Lucida Console", "DejaVu Sans Mono")]
    public void WithNoFontconfigTheTablesShapeIsStillTheAnswer(string requested, string expected)
    {
        SystemFontIndex index = Index();
        Assert.SkipUnless(index.Has(expected), $"{expected} is not installed");

        // Windows and most macOS installations. There is no pre-match hook there either, so
        // `ImplFontSubstitute` really is what runs and `VCL.xcu`'s FontType really is the shape —
        // which is why the table's classification is kept rather than deleted. These are the two
        // rows above, answered the other way round.
        SystemFontResolver resolver = new(index, FontconfigPreferences.None);

        resolver.Resolve(new FontRequest(requested)).FamilyName.ShouldBe(expected);
    }

    [Theory]
    [InlineData("Century Schoolbook", FontFamilyClass.Serif, "DejaVu Serif")]
    [InlineData("Book Antiqua", FontFamilyClass.SansSerif, "DejaVu Sans")]
    [InlineData("Palatino Linotype", FontFamilyClass.SansSerif, "DejaVu Sans")]
    public void ADeclaredShapeStillBeatsFontconfigsClassificationOfTheName(
        string requested, FontFamilyClass declared, string expected)
    {
        SystemFontResolver resolver = Resolver();
        Assert.SkipUnless(resolver.Index.Has(expected), $"{expected} is not installed");

        // The two rules compose, and in this order. `FontConfigManager::Substitute` appends the
        // declared class's generic to the pattern *before* the match runs, so it sits ahead of
        // whatever generic fontconfig's own <default> rules would append for the name — which is why
        // a document declaring Century Schoolbook a roman gets DejaVu Serif although fontconfig files
        // the bare name under nothing, and one declaring Palatino Linotype a grotesque gets DejaVu
        // Sans although 45-latin.conf files it under serif.
        //
        // Written because the classification change could plausibly have made the declared-class
        // logic redundant, and it does not: these three rows are the only thing separating the two.
        resolver.Resolve(new FontRequest(requested, DeclaredClass: declared))
            .FamilyName.ShouldBe(expected);
    }

    [Theory]
    [InlineData("Garamond", FontFamilyClass.SansSerif, "DejaVu Sans")]
    [InlineData("Tahoma", FontFamilyClass.Serif, "DejaVu Serif")]
    public void WithNoFontconfigTheDeclaredShapeIsWhatRoutes(
        string requested, FontFamilyClass declared, string expected)
    {
        SystemFontIndex index = Index();
        Assert.SkipUnless(index.Has(expected), $"{expected} is not installed");

        // The declared-class arm has to survive the absence of a fontconfig, and nothing else here
        // pins that: every other row in this file runs against the machine's own configuration.
        // Windows and most macOS installations have no pre-match hook at all, so the fontconfig
        // filing this arm competes with does not exist and only `ImplFontSubstitute` and the
        // family type are left. These two rows are the ones directly above, answered against
        // `FontconfigPreferences.None` instead — same answer, different route, which is the claim.
        //
        // Written during the round that briefly made this branch conditional on
        // `IsConfigured`. That change was reverted — 26.2.4.2 is the target and it honours the
        // declaration — but the guard is worth keeping, because the branch it protects is the one
        // no reference in this container can measure.
        SystemFontResolver resolver = new(index, FontconfigPreferences.None);

        resolver.Resolve(new FontRequest(requested, DeclaredClass: declared))
            .FamilyName.ShouldBe(expected);
    }

    // ------------------------------------------------- the shape the document itself declares

    [Theory]
    [InlineData("Times", FontFamilyClass.Serif, "DejaVu Serif")]
    [InlineData("Times", FontFamilyClass.SansSerif, "DejaVu Sans")]
    [InlineData("Helvetica", FontFamilyClass.SansSerif, "DejaVu Sans")]
    [InlineData("Albany", FontFamilyClass.SansSerif, "DejaVu Sans")]
    [InlineData("Thorndale", FontFamilyClass.Serif, "DejaVu Serif")]
    public void ADeclaredShapeBeatsAWeakAliasTheChainWouldHaveTaken(
        string requested, FontFamilyClass declared, string expected)
    {
        SystemFontResolver resolver = Resolver();
        Assert.SkipUnless(resolver.Index.Has(expected), $"{expected} is not installed");

        // These five are the cases that fix where in the order the declaration is consulted, and they
        // are the reason it is consulted *before* the substitution chain rather than after it. Every
        // one of them has a chain entry that IS installed -- `times` names `liberationserif`,
        // `helvetica` and `albany` name `liberationsans`, `thorndale` names `liberationserif` -- so a
        // resolver that tried the chain first would answer Liberation and stop, and never reach the
        // declaration at all.
        //
        // LibreOffice does not. `FontConfigManager::Substitute`
        // (vcl/unx/generic/font/fontconfig.cxx) adds the requested name as FC_FAMILY and then appends
        // a *second* FC_FAMILY -- "serif" for FAMILY_ROMAN, "sans" for FAMILY_SWISS -- and it is the
        // pre-match substitution, so it runs before VCL.xcu is consulted at all. fontconfig then
        // answers with the generic's own preferred face for a name it aliases only weakly:
        // `fc-match "Times,serif"` is DejaVu Serif where `fc-match Times` is Liberation Serif.
        //
        // Measured against the installed 26.2.4.2 with one authored document per row, and it is worth
        // 11% of the characters on a line: `1447.doc` sets its body in Times declared roman, the
        // reference draws every paragraph of it in DejaVu Serif, and reading the declaration late left
        // us in Liberation Serif with nine lines of text fitted into seven.
        resolver.Resolve(new FontRequest(requested, DeclaredClass: declared))
            .FamilyName.ShouldBe(expected);
    }

    [Theory]
    [InlineData("Garamond", FontFamilyClass.SansSerif, "DejaVu Sans")]
    [InlineData("Georgia", FontFamilyClass.SansSerif, "DejaVu Sans")]
    [InlineData("Futura", FontFamilyClass.Serif, "DejaVu Serif")]
    [InlineData("Tahoma", FontFamilyClass.Serif, "DejaVu Serif")]
    [InlineData("TimesNewRomanPSMT", FontFamilyClass.Serif, "DejaVu Serif")]
    public void TheDeclaredShapeBeatsTheShapeTheTableFilesTheNameUnder(
        string requested, FontFamilyClass declared, string expected)
    {
        SystemFontResolver resolver = Resolver();
        Assert.SkipUnless(resolver.Index.Has(expected), $"{expected} is not installed");

        // A document's font table says what shape each family it names has, and LibreOffice believes
        // it over its own classification of the name. Measured on 26.2.4.2 by holding the family name
        // constant and varying only the declaration: Garamond, which the table files as a roman and
        // which falls back to DejaVu Serif undeclared, falls back to DejaVu Sans declared swiss; and
        // Futura and Tahoma, both grotesques by name, fall back to DejaVu Serif declared roman. Both
        // directions matter — a rule that only ever promoted serif would fix the first case and
        // manufacture the second.
        //
        // These five names have no installed chain entry, so they cannot tell where in the order the
        // declaration is read; ADeclaredShapeBeatsAWeakAliasTheChainWouldHaveTaken is what does that.
        resolver.Resolve(new FontRequest(requested, DeclaredClass: declared))
            .FamilyName.ShouldBe(expected);
    }

    [Theory]
    [InlineData("Times New Roman", FontFamilyClass.Serif, "Liberation Serif")]
    [InlineData("Times New Roman", FontFamilyClass.SansSerif, "Liberation Serif")]
    [InlineData("Arial", FontFamilyClass.Serif, "Liberation Sans")]
    [InlineData("Arial", FontFamilyClass.SansSerif, "Liberation Sans")]
    [InlineData("Calibri", FontFamilyClass.SansSerif, "Carlito")]
    [InlineData("Cambria", FontFamilyClass.Serif, "Caladea")]
    [InlineData("Courier New", FontFamilyClass.Serif, "Liberation Mono")]
    public void AStrongMetricAliasSurvivesTheDeclaredShape(
        string requested, FontFamilyClass declared, string expected)
    {
        SystemFontResolver resolver = Resolver();
        Assert.SkipUnless(resolver.Index.Has(expected), $"{expected} is not installed");

        // The half that makes the rule safe, and the reason it is not "a declaration always wins".
        // These are the substitutions that hold a document's line breaks, and routing them to DejaVu
        // would reflow essentially every Word document in the corpus — a font table calling Arial a
        // roman must not move it off Liberation Sans.
        //
        // What survives a generic family in fontconfig is the alias bound to the requested name
        // itself, in its own 30-metric-aliases.conf, so that is the test the resolver applies: does an
        // installed face declare itself the equivalent of *this* name. It is deliberately not
        // AreMetricCompatible, which is transitive — Liberation Sans is Helvetica's metric equal
        // through Arial, and Helvetica declared swiss still renders in DejaVu Sans (the row above).
        // Measured on 26.2.4.2: all of these answer Liberation, Carlito or Caladea with the class
        // declared in either direction.
        resolver.Resolve(new FontRequest(requested, DeclaredClass: declared))
            .FamilyName.ShouldBe(expected);
    }

    [Theory]
    [InlineData("Symbol", FontFamilyClass.Serif, "OpenSymbol")]
    [InlineData("Symbol", FontFamilyClass.SansSerif, "OpenSymbol")]
    public void APiFaceIsExemptFromTheDeclaredShape(
        string requested, FontFamilyClass declared, string expected)
    {
        SystemFontResolver resolver = Resolver();
        Assert.SkipUnless(resolver.Index.Has(expected), $"{expected} is not installed");

        // Every Word document that uses Symbol declares it roman — `ABCD-FE-01-00 Flight Envelope.docx`
        // and its sibling both do — and there is no roman equivalent of a font of arrows and Greek
        // letters. fontconfig agrees and binds the name hard enough to survive a generic:
        // `fc-match "Symbol,serif"` and `fc-match Symbol` both answer OpenSymbol, and so does 26.2.4.2
        // on an authored document declaring Symbol as a roman. Without the carve-out those runs came
        // out in DejaVu Serif, which draws the characters rather than the symbols they stand for.
        resolver.Resolve(new FontRequest(requested, DeclaredClass: declared))
            .FamilyName.ShouldBe(expected);
    }

    [Fact]
    public void ADeclaredFixedPitchBeatsADeclaredFamily()
    {
        SystemFontResolver resolver = Resolver();
        Assert.SkipUnless(resolver.Index.Has("DejaVu Sans Mono"), "DejaVu Sans Mono is not installed");

        // The document says two things about the family and they can disagree. LibreOffice takes the
        // pitch: a request for Garamond declared roman *and* fixed answers DejaVu Sans Mono, not
        // DejaVu Serif. A document declaring fixed pitch is relying on its columns lining up.
        resolver.Resolve(new FontRequest(
                "Garamond", Pitch: FontPitch.Fixed, DeclaredClass: FontFamilyClass.Serif))
            .FamilyName.ShouldBe("DejaVu Sans Mono");
    }

    [Fact]
    public void AFamilyWithNoDeclaredShapeIsUnaffected()
    {
        SystemFontResolver resolver = Resolver();
        Assert.SkipUnless(
            resolver.Index.Has("DejaVu Sans") && resolver.Index.Has("DejaVu Serif"),
            "the DejaVu family is not installed");

        // The common case, and the drift guard for it: most font tables declare nothing useful, and
        // an unknown declaration must leave the name's own class exactly where it was. `modern`,
        // `script` and `decorative` reach the resolver as Unknown and land here — the readers collapse
        // them, because LibreOffice appends no generic family for any of them. Mapping `modern` onto a
        // monospaced fallback is the tempting mistake; measured, a document naming Times as `modern`
        // still renders in Liberation Serif, the plain `fc-match Times` answer.
        resolver.Resolve(new FontRequest("Garamond", DeclaredClass: FontFamilyClass.Unknown))
            .FamilyName.ShouldBe("DejaVu Serif");
        resolver.Resolve(new FontRequest("Segoe UI", DeclaredClass: FontFamilyClass.Unknown))
            .FamilyName.ShouldBe("DejaVu Sans");
    }
}
