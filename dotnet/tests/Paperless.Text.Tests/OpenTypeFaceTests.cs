using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Shouldly;

namespace Paperless.Text.Tests;

/// <summary>
/// Tests the hand-rolled OpenType reader against fonts actually installed on the machine.
/// </summary>
/// <remarks>
/// <para>
/// Real font files rather than synthesised ones, because the point of reading fonts by hand is to get
/// the awkward parts of real files right: which metric set a font asks to be believed, an advance
/// table that stops early and repeats its last entry, a character map in one of four formats, a
/// units-per-em that is 1000 for some fonts and 2048 for others.
/// </para>
/// <para>
/// Carlito and Caladea are the metric-compatible substitutes for Calibri and Cambria, and the
/// Liberation family for Arial, Times New Roman and Courier New. Those substitutions are the whole
/// reason Paperless can match LibreOffice's line breaks on an OOXML document, so they are also the
/// right faces to test the reader against. A machine without them skips rather than fails: these
/// tests are about the reader, and checking the environment is <c>check-env.sh</c>'s job.
/// </para>
/// </remarks>
public class OpenTypeFaceTests
{
    private static readonly string[] SearchDirectories =
    [
        "/usr/share/fonts/truetype/crosextra",
        "/usr/share/fonts/truetype/liberation",
        "/usr/share/fonts/truetype/liberation2",
        "/usr/share/fonts",
    ];

    /// <summary>Finds an installed font file by name, or null when it is not present.</summary>
    private static string? Find(string fileName)
    {
        foreach (string directory in SearchDirectories)
        {
            if (!Directory.Exists(directory)) continue;

            string direct = Path.Combine(directory, fileName);
            if (File.Exists(direct)) return direct;

            string[] found = Directory.GetFiles(directory, fileName, SearchOption.AllDirectories);
            if (found.Length > 0) return found[0];
        }
        return null;
    }

    private static OpenTypeFace Require(string fileName)
    {
        string? path = Find(fileName);
        Assert.SkipWhen(path is null, $"{fileName} is not installed; see check-env.sh");

        OpenTypeFace? face = OpenTypeFace.ReadFile(path!);
        face.ShouldNotBeNull($"{path} should be readable as a font");
        return face!;
    }

    [Fact]
    public void AFaceReportsItsOwnNameAndDesignGrid()
    {
        OpenTypeFace carlito = Require("Carlito-Regular.ttf");

        carlito.FamilyName.ShouldBe("Carlito");
        carlito.SubfamilyName.ShouldBe("Regular");

        // Not a constant: 2048 for most TrueType fonts and 1000 for most CFF ones, so a reader that
        // assumes either scales half the fonts on the machine wrongly.
        carlito.UnitsPerEm.ShouldBe(2048);
        carlito.Weight.ShouldBe(400);
        carlito.IsItalic.ShouldBeFalse();
    }

    [Fact]
    public void AFaceKnowsItsWeightAndSlant()
    {
        OpenTypeFace bold = Require("Carlito-Bold.ttf");
        bold.Weight.ShouldBe(700);
        bold.IsItalic.ShouldBeFalse();

        OpenTypeFace italic = Require("Carlito-Italic.ttf");
        italic.Weight.ShouldBe(400);
        italic.IsItalic.ShouldBeTrue();

        OpenTypeFace boldItalic = Require("Carlito-BoldItalic.ttf");
        boldItalic.Weight.ShouldBe(700);
        boldItalic.IsItalic.ShouldBeTrue();
    }

    [Fact]
    public void TheCharacterMapCoversWhatTheFontHasAndNotWhatItDoesNot()
    {
        OpenTypeFace carlito = Require("Carlito-Regular.ttf");

        carlito.HasGlyphFor('A').ShouldBeTrue();
        carlito.HasGlyphFor('ß').ShouldBeTrue();
        carlito.HasGlyphFor('Ω').ShouldBeTrue();
        carlito.HasGlyphFor('д').ShouldBeTrue();

        // A Latin font has no CJK, which is what makes fallback necessary — so a reader that reports
        // coverage optimistically produces a page of missing-glyph boxes rather than a fallback.
        carlito.HasGlyphFor('日').ShouldBeFalse();
        carlito.Characters.Count.ShouldBeGreaterThan(500);
    }

    [Fact]
    public void AdvanceWidthsComeFromTheFontAndDifferBetweenGlyphs()
    {
        OpenTypeFace carlito = Require("Carlito-Regular.ttf");

        int narrow = carlito.AdvanceForCharacter('i');
        int wide = carlito.AdvanceForCharacter('W');
        int space = carlito.AdvanceForCharacter(' ');

        narrow.ShouldBeGreaterThan(0);
        wide.ShouldBeGreaterThan(narrow, "a proportional font's W is wider than its i");
        space.ShouldBeGreaterThan(0);

        // An uncovered character has no advance, which is the signal to fall back rather than to
        // measure it as zero-width.
        carlito.AdvanceForCharacter('日').ShouldBe(0);
    }

    [Fact]
    public void AMonospacedFontMeasuresEveryGlyphTheSame()
    {
        OpenTypeFace mono = Require("LiberationMono-Regular.ttf");

        mono.IsFixedPitch.ShouldBeTrue();
        mono.AdvanceForCharacter('i').ShouldBe(mono.AdvanceForCharacter('W'));
        mono.AdvanceForCharacter(' ').ShouldBe(mono.AdvanceForCharacter('m'));
    }

    [Fact]
    public void TwoMetricCompatibleSubstitutesUseDifferentDesignGrids()
    {
        OpenTypeFace carlito = Require("Carlito-Regular.ttf");
        OpenTypeFace caladea = Require("Caladea-Regular.ttf");

        // Carlito is on 2048 units and Caladea on 1000 — both metric-compatible with the Microsoft
        // fonts they stand in for, on grids that differ by a factor of two. Metric compatibility is
        // about the *scaled* advance, not the raw number, so every measurement has to go through
        // unitsPerEm. Comparing raw advances between two faces is meaningless, and a reader that
        // assumes one grid mis-measures every document set in the other font.
        carlito.UnitsPerEm.ShouldBe(2048);
        caladea.UnitsPerEm.ShouldBe(1000);

        // Scaled, both put a capital A between two-fifths and four-fifths of the em, which is where
        // any Latin face puts one. That is the invariant a grid-blind reader would break.
        foreach (OpenTypeFace face in new[] { carlito, caladea })
        {
            double emFraction = (double)face.AdvanceForCharacter('A') / face.UnitsPerEm;
            emFraction.ShouldBeInRange(0.4, 0.8);
        }
    }

    // ------------------------------------------------------------------- line metrics

    [Fact]
    public void LineMetricsComeFromTheSetTheFontAsksToBeBelieved()
    {
        OpenTypeFace carlito = Require("Carlito-Regular.ttf");
        LineMetrics metrics = LineSpacing.Resolve(carlito);

        // Which set was used is reported rather than hidden: a line-height difference is one of the
        // most visible ways two renderers diverge, and knowing which set was believed turns an
        // unexplained half-page offset into a one-line answer.
        metrics.Source.ShouldBeOneOf(
            LineMetricSource.WindowsMetrics, LineMetricSource.TypographicMetrics);

        metrics.Ascent.ShouldBeGreaterThan(0);
        metrics.Descent.ShouldBeGreaterThan(0);
        metrics.UnitsPerEm.ShouldBe(carlito.UnitsPerEm);
        metrics.LineHeight.ShouldBeGreaterThan(carlito.UnitsPerEm);
    }

    [Fact]
    public void TheTypoMetricsFlagDecidesWhichHalfOfOs2Wins()
    {
        OpenTypeFace carlito = Require("Carlito-Regular.ttf");
        Os2Table os2 = carlito.Os2.ShouldNotBeNull();

        LineMetrics metrics = LineSpacing.Resolve(carlito);

        // The precedence is the point: the Windows metrics by default, the typographic ones only when
        // fsSelection bit 7 asks for them. Reading whichever field came first would be right for
        // roughly half the fonts installed.
        if (os2.UseTypoMetrics)
        {
            metrics.Source.ShouldBe(LineMetricSource.TypographicMetrics);
            metrics.Ascent.ShouldBe(os2.TypoAscender);
            metrics.Descent.ShouldBe(-os2.TypoDescender);
        }
        else
        {
            metrics.Source.ShouldBe(LineMetricSource.WindowsMetrics);
            metrics.Ascent.ShouldBe(os2.WindowsAscent);
            metrics.Descent.ShouldBe(os2.WindowsDescent);
        }
    }

    [Fact]
    public void MetricsScaleLinearlyWithTheEmSize()
    {
        OpenTypeFace carlito = Require("Carlito-Regular.ttf");
        LineMetrics metrics = LineSpacing.Resolve(carlito);

        Length ten = metrics.ScaledLineHeight(Length.FromPoints(10));
        Length twenty = metrics.ScaledLineHeight(Length.FromPoints(20));

        // Within a rounding unit: the whole point of holding lengths as exact integers is that
        // doubling a size doubles a measurement instead of drifting.
        Math.Abs((twenty.Emu / 2) - ten.Emu).ShouldBeLessThanOrEqualTo(1);
        ten.Points.ShouldBeGreaterThan(10, "a line is taller than the em it is set in");
    }

    [Fact]
    public void InternalLeadingIsDerivedRatherThanRead()
    {
        OpenTypeFace carlito = Require("Carlito-Regular.ttf");
        LineMetrics metrics = LineSpacing.Resolve(carlito);

        Length em = Length.FromPoints(12);
        Length leading = metrics.ScaledInternalLeading(em);

        // Ascent plus descent minus the requested size — the classic Windows definition, and what
        // single line spacing consumes. Most fonts' Windows metrics exceed their em square, which is
        // exactly where "single-spaced" lines get the gap they visibly have.
        Length expected = metrics.ScaledAscent(em) + metrics.ScaledDescent(em) - em;
        leading.ShouldBe(expected);
        leading.Emu.ShouldBeGreaterThan(0);
    }

    [Fact]
    public void DecorationMetricsFallBackRatherThanDrawingNothing()
    {
        OpenTypeFace carlito = Require("Carlito-Regular.ttf");
        FontVerticalMetrics decorations =
            LineSpacing.ResolveDecorations(carlito, LineSpacing.Resolve(carlito));

        // A zero-thickness line draws nothing at all, so neither may ever be zero however little the
        // font says about them.
        decorations.UnderlineThickness.ShouldBeGreaterThan(0);
        decorations.StrikeoutThickness.ShouldBeGreaterThan(0);

        // The underline sits below the baseline and the strikethrough above it, which is the sign
        // convention the font records and the one a renderer has to honour.
        decorations.UnderlinePosition.ShouldBeLessThan(0);
        decorations.StrikeoutPosition.ShouldBeGreaterThan(0);
    }

    [Fact]
    public void AFaceWithNoUsableMetricsFallsBackToTheEmSquare()
    {
        // A synthesised face with the minimum a reader can parse: no metrics at all. The fallback is
        // not about correctness — a font this broken will not lay out well — but about producing a
        // line with a height, since a zero-height line makes every page infinitely long.
        OpenTypeFace face = OpenTypeFace.Read(MinimalFont()).ShouldNotBeNull();
        LineMetrics metrics = LineSpacing.Resolve(face);

        metrics.Source.ShouldBe(LineMetricSource.Fallback);
        (metrics.Ascent + metrics.Descent).ShouldBe(face.UnitsPerEm);
        metrics.Ascent.ShouldBeGreaterThan(metrics.Descent);
    }

    // ------------------------------------------------------------------- malformed input

    [Fact]
    public void SomethingThatIsNotAFontIsRejectedRatherThanMisread()
    {
        OpenTypeFace.Read([]).ShouldBeNull();
        OpenTypeFace.Read([1, 2, 3]).ShouldBeNull();
        OpenTypeFace.Read(new byte[512]).ShouldBeNull();

        // A ZIP, which is what a reader gets when a caller hands it the wrong part of a document.
        OpenTypeFace.Read([0x50, 0x4B, 0x03, 0x04, .. new byte[100]]).ShouldBeNull();
    }

    [Fact]
    public void ATagIsPaddedToFourBytesAsTheFormatStoresIt()
    {
        // "OS/2" is four characters but "cvt" is three, and the format pads with spaces — so a tag
        // built without the padding never matches the table it names.
        SfntFile.TagOf("head").ShouldBe(0x68656164u);
        SfntFile.TagOf("OS/2").ShouldBe(0x4F532F32u);
        SfntFile.TagOf("cvt").ShouldBe(SfntFile.TagOf("cvt "));
    }

    /// <summary>
    /// The smallest thing this reader accepts as a font: a directory with one empty table.
    /// </summary>
    private static byte[] MinimalFont()
    {
        byte[] font = new byte[12 + 16];

        // sfnt version 1.0, one table.
        font[1] = 0x01;
        font[5] = 0x01;

        // One entry, tagged "head", of zero length — enough to be a directory and nothing more.
        font[12] = (byte)'h';
        font[13] = (byte)'e';
        font[14] = (byte)'a';
        font[15] = (byte)'d';
        return font;
    }
}
