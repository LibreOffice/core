using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Shaping;
using Shouldly;

namespace Paperless.Text.Tests;

/// <summary>
/// Tests shaping: the glyphs a face produces for a run of text, and the widths that follow.
/// </summary>
/// <remarks>
/// Shaping exists here for one reason, which is that its output decides line breaks. A shaper that
/// picked the right glyphs but the wrong advances would render correctly and paginate wrongly, so these
/// tests are mostly about the advances — and specifically about the two things that make a shaped
/// advance differ from the sum of the font's own widths: kerning, and ligatures.
/// </remarks>
public class ShapingTests
{
    private static OpenTypeFace Carlito()
    {
        string? path = FindFont("Carlito-Regular.ttf");
        Assert.SkipWhen(path is null, "Carlito is not installed; see check-env.sh");
        return OpenTypeFace.ReadFile(path!).ShouldNotBeNull();
    }

    private static string? FindFont(string fileName)
    {
        foreach (string directory in new[]
                 {
                     "/usr/share/fonts/truetype/crosextra",
                     "/usr/share/fonts/truetype/liberation",
                     "/usr/share/fonts",
                 })
        {
            if (!Directory.Exists(directory)) continue;

            string[] found = Directory.GetFiles(directory, fileName, SearchOption.AllDirectories);
            if (found.Length > 0) return found[0];
        }
        return null;
    }

    // ------------------------------------------------------------------ the metrics-only shaper

    [Fact]
    public void TheMetricsShaperGivesOneGlyphPerCodePoint()
    {
        OpenTypeFace face = Carlito();
        ShapedText shaped = MetricsShaper.Instance.Shape(face, "abc");

        shaped.Glyphs.Count.ShouldBe(3);
        shaped.TextLength.ShouldBe(3);
        shaped.UnitsPerEm.ShouldBe(face.UnitsPerEm);

        // The advances are the font's own, which is the whole of what this shaper knows.
        shaped.AdvanceInDesignUnits.ShouldBe(
            face.AdvanceForCharacter('a') + face.AdvanceForCharacter('b')
            + face.AdvanceForCharacter('c'));
    }

    [Fact]
    public void ASurrogatePairIsOneGlyphAtTheClusterOfItsFirstUnit()
    {
        OpenTypeFace face = Carlito();

        // A character above the basic plane is two UTF-16 units and one glyph, so a shaper that
        // iterated units would measure it twice and produce a glyph for a lone surrogate.
        ShapedText shaped = MetricsShaper.Instance.Shape(face, "a\U0001F600b");

        shaped.Glyphs.Count.ShouldBe(3);
        shaped.Glyphs[1].Cluster.ShouldBe(1);
        shaped.Glyphs[2].Cluster.ShouldBe(3);
        shaped.TextLength.ShouldBe(4);
    }

    // ---------------------------------------------------------------------------- prefix widths

    [Fact]
    public void PrefixWidthsAddUpToTheWhole()
    {
        OpenTypeFace face = Carlito();
        const string text = "the quick brown fox";
        ShapedText shaped = TextShaper.Default.Shape(face, text);

        shaped.AdvanceUpTo(0).ShouldBe(0);
        shaped.AdvanceUpTo(text.Length).ShouldBe(shaped.AdvanceInDesignUnits);

        // Split anywhere and the halves add up, which is what lets a filler measure candidate lines by
        // subtraction instead of by shaping each one.
        for (int at = 0; at <= text.Length; at++)
        {
            (shaped.AdvanceUpTo(at) + shaped.AdvanceBetween(at, text.Length))
                .ShouldBe(shaped.AdvanceInDesignUnits);
        }
    }

    [Fact]
    public void PrefixWidthsAreMonotonicAndClampOutOfRange()
    {
        OpenTypeFace face = Carlito();
        ShapedText shaped = TextShaper.Default.Shape(face, "widths");

        long previous = -1;
        for (int at = 0; at <= 6; at++)
        {
            long width = shaped.AdvanceUpTo(at);
            width.ShouldBeGreaterThan(previous);
            previous = width;
        }

        // A caller asking past the end gets the whole run rather than an exception: layout indexes into
        // a paragraph from several directions and an off-by-one there should not throw out of a page.
        shaped.AdvanceUpTo(-5).ShouldBe(0);
        shaped.AdvanceUpTo(500).ShouldBe(shaped.AdvanceInDesignUnits);
    }

    [Fact]
    public void ScalingHappensOnceRatherThanPerGlyph()
    {
        OpenTypeFace face = Carlito();
        Length em = Length.FromPoints(12);

        // Two hundred of the widest lower-case letter: if each glyph were scaled and rounded before
        // being added, the halves would no longer sum to the whole, and the error would be largest at
        // exactly the length where it decides a break.
        string text = new('m', 200);
        ShapedText shaped = TextShaper.Default.Shape(face, text);

        Length whole = shaped.Width(em);
        Length halves = shaped.WidthBetween(0, 100, em) + shaped.WidthBetween(100, 200, em);

        Math.Abs(whole.Emu - halves.Emu).ShouldBeLessThanOrEqualTo(1);
    }

    [Fact]
    public void AnEmptyRunMeasuresZeroWithoutDividingByZero()
    {
        ShapedText.Empty.AdvanceInDesignUnits.ShouldBe(0);
        ShapedText.Empty.TextLength.ShouldBe(0);
        ShapedText.Empty.Width(Length.FromPoints(12)).ShouldBe(Length.Zero);

        OpenTypeFace face = Carlito();
        TextShaper.Default.Shape(face, "").AdvanceInDesignUnits.ShouldBe(0);
    }

    // -------------------------------------------------------------------------------- HarfBuzz

    [Fact]
    public void TheDefaultShaperIsHarfBuzzWhenItsNativeLibraryIsThere()
    {
        // Not a given — the native package is per-platform. Whichever way it goes, the answer has to be
        // reported, because an unkerned measurement is a legitimate reason for a comparison against
        // LibreOffice to disagree and a silent fallback would make that undiagnosable.
        Assert.SkipUnless(HarfBuzzShaper.IsAvailable, "the native harfbuzz library is not present");

        TextShaper.IsShapingAvailable.ShouldBeTrue();
        TextShaper.Default.ShouldBeOfType<HarfBuzzShaper>();
    }

    [Fact]
    public void KerningMakesARunNarrowerThanTheSumOfItsAdvances()
    {
        Assert.SkipUnless(HarfBuzzShaper.IsAvailable, "the native harfbuzz library is not present");
        OpenTypeFace face = Carlito();
        using HarfBuzzShaper shaper = new();

        // "AV" is the textbook kern pair. Shaped, it is narrower than the two advances added up; that
        // difference is the whole reason shaping is on the measurement path rather than beside it.
        long unkerned = face.AdvanceForCharacter('A') + face.AdvanceForCharacter('V');
        long kerned = shaper.Shape(face, "AV").AdvanceInDesignUnits;

        kerned.ShouldBeLessThan(unkerned);

        // And switching kerning off gets the plain sum back, which is what a document asking for no
        // kerning has to measure as.
        shaper.Shape(face, "AV", new ShapingOptions(DisableKerning: true))
            .AdvanceInDesignUnits.ShouldBe(unkerned);
    }

    [Fact]
    public void DefaultOptionsMeanKerningAndLigaturesOn()
    {
        Assert.SkipUnless(HarfBuzzShaper.IsAvailable, "the native harfbuzz library is not present");
        OpenTypeFace face = Carlito();
        using HarfBuzzShaper shaper = new();

        // `default` has to mean what LibreOffice means by default, since that is what a caller who says
        // nothing gets. Naming the options after what they switch off is what makes that true.
        ShapedText assumed = shaper.Shape(face, "AV Waverley");
        ShapedText spelledOut = shaper.Shape(
            face, "AV Waverley", new ShapingOptions(DisableKerning: false, DisableLigatures: false));

        assumed.AdvanceInDesignUnits.ShouldBe(spelledOut.AdvanceInDesignUnits);
        assumed.AdvanceInDesignUnits.ShouldBeLessThan(
            shaper.Shape(face, "AV Waverley", new ShapingOptions(DisableKerning: true))
                .AdvanceInDesignUnits);
    }

    [Fact]
    public void ALigatureIsOneGlyphCoveringSeveralCharacters()
    {
        Assert.SkipUnless(HarfBuzzShaper.IsAvailable, "the native harfbuzz library is not present");
        OpenTypeFace face = Carlito();
        using HarfBuzzShaper shaper = new();

        // Carlito ligates "fi". So the shaped run has fewer glyphs than characters, and the glyph's
        // cluster names the first character it covers rather than being a running index.
        ShapedText shaped = shaper.Shape(face, "fi");
        Assert.SkipWhen(shaped.Glyphs.Count != 1, "this face does not ligate fi");

        shaped.Glyphs[0].Cluster.ShouldBe(0);
        shaped.TextLength.ShouldBe(2);

        // The whole ligature's width is credited to its first character, so a position inside it
        // measures as the ligature's end. That is LibreOffice's rule too — GetPartialTextWidth adds a
        // glyph's whole width when its charPos falls in range — and it is the one that matters for
        // breaking lines, as opposed to the ligature-caret arithmetic LibreOffice uses for placing a
        // cursor inside one.
        shaped.AdvanceUpTo(1).ShouldBe(shaped.AdvanceInDesignUnits);
        shaped.AdvanceUpTo(2).ShouldBe(shaped.AdvanceInDesignUnits);

        // Switched off, the two glyphs come back.
        shaper.Shape(face, "fi", new ShapingOptions(DisableLigatures: true))
            .Glyphs.Count.ShouldBe(2);
    }

    [Fact]
    public void ShapingIsIndependentOfHowManyTimesAFaceIsShapedWith()
    {
        Assert.SkipUnless(HarfBuzzShaper.IsAvailable, "the native harfbuzz library is not present");
        OpenTypeFace face = Carlito();
        using HarfBuzzShaper shaper = new();

        // The face's harfbuzz objects are cached and reused, and the buffer is not. A shaper that
        // leaked state between calls would give a different answer the second time — the sort of bug
        // that shows up as one page in a document being subtly wrong.
        long first = shaper.Shape(face, "the quick brown fox").AdvanceInDesignUnits;
        for (int i = 0; i < 5; i++)
        {
            shaper.Shape(face, "something else entirely");
            shaper.Shape(face, "the quick brown fox").AdvanceInDesignUnits.ShouldBe(first);
        }
    }

    [Fact]
    public void ARunLongerThanOneShapingCallStillMeasuresConsistently()
    {
        Assert.SkipUnless(HarfBuzzShaper.IsAvailable, "the native harfbuzz library is not present");
        OpenTypeFace face = Carlito();
        using HarfBuzzShaper shaper = new();

        // Past the segment cap the run is shaped in pieces, split at spaces. The clusters have to stay
        // absolute across the boundary or every measurement past it is credited to the wrong character.
        string text = string.Join(' ', Enumerable.Repeat("Waverley AVAST typeface", 400));
        ShapedText shaped = shaper.Shape(face, text);

        shaped.TextLength.ShouldBe(text.Length);
        shaped.AdvanceUpTo(text.Length).ShouldBe(shaped.AdvanceInDesignUnits);

        long previous = -1;
        for (int at = 0; at <= text.Length; at += 97)
        {
            long width = shaped.AdvanceUpTo(at);
            width.ShouldBeGreaterThan(previous);
            previous = width;
        }
    }

    [Fact]
    public void AnUnknownLanguageTagDoesNotThrowOutOfAMeasurement()
    {
        Assert.SkipUnless(HarfBuzzShaper.IsAvailable, "the native harfbuzz library is not present");
        OpenTypeFace face = Carlito();
        using HarfBuzzShaper shaper = new();

        // Documents carry made-up locale tags. Shaping without the language differs only for the few
        // language-specific features, which is a far better outcome than failing to lay the page out.
        shaper.Shape(face, "text", new ShapingOptions(Language: "not-a-language-tag-at-all"))
            .AdvanceInDesignUnits.ShouldBeGreaterThan(0);
    }

    [Fact]
    public void AFaceWithNoKerningShapesTheSameWayAsPlainMetrics()
    {
        Assert.SkipUnless(HarfBuzzShaper.IsAvailable, "the native harfbuzz library is not present");
        OpenTypeFace face = Carlito();
        using HarfBuzzShaper shaper = new();

        // Digits and spaces: nothing to kern and nothing to ligate. The two shapers have to agree here,
        // which is what says the metrics-only fallback is a real fallback rather than a different
        // answer that happens to be close.
        const string text = "1234567890 0987654321";
        shaper.Shape(face, text).AdvanceInDesignUnits
            .ShouldBe(MetricsShaper.Instance.Shape(face, text).AdvanceInDesignUnits);
    }
}
