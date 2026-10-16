using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Layout;
using XRay.Office.Text.Shaping;
using Shouldly;

namespace XRay.Office.Text.Tests;

/// <summary>
/// Tracking suppresses the optional ligatures — <c>Font::IsFixKerning()</c> becoming
/// <c>SalLayoutFlags::DisableLigatures</c>, <c>vcl/source/outdev/text.cxx</c>:996-998.
/// </summary>
/// <remarks>
/// <para>
/// The typographic argument is that a ligature fixes a collision between two letters set at their
/// natural distance, and once a designer has pushed them apart there is no collision left to fix.
/// The measurable argument is sharper, and it is why this has tests of its own rather than a line
/// in <see cref="ShapingTests"/>.
/// </para>
/// <para>
/// A ligature is <em>one glyph covering several characters</em>, so a PDF writer has to give it a
/// <c>ToUnicode</c> entry mapping one code to several. Poppler responds to a multi-character entry
/// by dropping its intra-word gap tolerance from 0.400 em to 0.100 em — measured by byte-surgery on
/// two real PDFs, sweeping the <c>TJ</c> adjustments and watching where <c>pdftotext</c> starts
/// splitting. Tracking of 0.300 em sits between those two thresholds. So one wrong ligature turns
/// every glyph on its line into a separate word.
/// </para>
/// <para>
/// Measured on <c>words/batch-008/docx/FAA-2017-0628-0002_attachment_1.docx</c>, whose cover footer
/// is 10 pt Carlito-Bold at <c>w:spacing w:val="60"</c>: 666 extracted words against the
/// reference's 638, on a document whose whitespace-stripped character stream was byte-identical to
/// the reference's, and whose one-character token count was 46 against the reference's 12. With the
/// rule in place all three agree.
/// </para>
/// </remarks>
public class TrackingLigatureTests
{
    // ------------------------------------------------------------------------------ the rule itself

    /// <summary>Zero tracking leaves the options exactly as they were.</summary>
    /// <remarks>
    /// Identity rather than equivalence: the overwhelming majority of runs are untracked, and they
    /// must reach HarfBuzz in the call they made before this rule existed.
    /// </remarks>
    [Fact]
    public void UntrackedOptionsAreUnchanged()
    {
        ShapingOptions options = new(Language: "en-GB", Script: "Latn", DisableKerning: true);

        options.WithTracking(Length.Zero).ShouldBe(options);
    }

    /// <summary>Tracking of either sign disables the optional ligatures.</summary>
    /// <remarks>
    /// Negative tracking is the commoner case in this corpus — a designer pulling a heading in — and
    /// <c>IsFixKerning()</c> is <c>mnSpacing != 0</c> rather than a sign test, so it counts too.
    /// </remarks>
    [Theory]
    [InlineData(60)]
    [InlineData(-16)]
    [InlineData(1)]
    public void AnyNonZeroTrackingDisablesLigatures(int twips)
    {
        new ShapingOptions().WithTracking(Length.FromTwips(twips))
            .DisableLigatures.ShouldBeTrue();
    }

    /// <summary>Nothing else about the options moves.</summary>
    [Fact]
    public void TrackingChangesOnlyTheLigatureFlag()
    {
        ShapingOptions tracked = new ShapingOptions(
                Language: "de-DE", Script: "Latn", DisableKerning: true, RightToLeft: true)
            .WithTracking(Length.FromTwips(60));

        tracked.Language.ShouldBe("de-DE");
        tracked.Script.ShouldBe("Latn");
        tracked.DisableKerning.ShouldBeTrue();
        tracked.RightToLeft.ShouldBeTrue();
        tracked.DisableLigatures.ShouldBeTrue();
    }

    /// <summary>A run states the rule for itself, so no caller has to remember it.</summary>
    [Fact]
    public void AFormattedRunAppliesItsOwnTracking()
    {
        OpenTypeFace face = CarlitoBold();

        new FormattedRun(0, 2, face, Size).EffectiveShaping.DisableLigatures.ShouldBeFalse();
        new FormattedRun(0, 2, face, Size, Tracking: Length.FromTwips(60))
            .EffectiveShaping.DisableLigatures.ShouldBeTrue();
    }

    // ------------------------------------------------------------------------ what it does to glyphs

    /// <summary>Carlito ligates <c>ti</c>, and it is the pair the corpus document tripped over.</summary>
    /// <remarks>
    /// Stated as a precondition rather than assumed: <c>t</c>+<c>i</c> is an unusual member of
    /// <c>liga</c> — Carlito inherits it from Lato — and a face without it would make every
    /// assertion below vacuously true.
    /// </remarks>
    [Fact]
    public void TheFaceLigatesTiByDefault()
    {
        ShapedText shaped = TextShaper.Default.Shape(CarlitoBold(), "ti");

        shaped.Glyphs.Count.ShouldBe(1, "Carlito's liga lookup maps t+i to one glyph");
        shaped.TextLength.ShouldBe(2);
    }

    /// <summary>A tracked run gets one glyph per character, because the ligature is suppressed.</summary>
    [Fact]
    public void ATrackedRunDoesNotLigate()
    {
        ShapedText shaped = TextShaper.Default.Shape(
            CarlitoBold(), "ti", new ShapingOptions().WithTracking(Length.FromTwips(60)));

        shaped.Glyphs.Count.ShouldBe(2);
    }

    /// <summary>
    /// The whole footer line of <c>FAA-2017-0628-0002_attachment_1.docx</c>, through the measurement
    /// path a document takes.
    /// </summary>
    /// <remarks>
    /// The glyph count is the assertion because it is what the PDF writer turns into
    /// <c>ToUnicode</c> entries, and a single multi-character entry is what cost that document 28
    /// words. 45 glyphs for 46 characters was the defect; 46 for 46 is the reference's own count,
    /// read out of its PDF with <c>pdf-ops.py</c>.
    /// </remarks>
    [Fact]
    public void TheCorpusFooterShapesOneGlyphPerCharacter()
    {
        const string Footer = "PADM 533: Policy Formation – Dr. Marcia Godwin";
        Footer.Length.ShouldBe(46);

        MeasuredParagraph tracked = MeasuredParagraph.Measure(
            Footer,
            [new FormattedRun(0, Footer.Length, CarlitoBold(), Length.FromPoints(10),
                Tracking: Length.FromTwips(60))]);

        tracked.Runs.Sum(run => run.Shaped.Glyphs.Count).ShouldBe(46);

        // And the control: untracked, the same text ligates and is one glyph short. Without this the
        // test above would keep passing if the face stopped ligating for some unrelated reason.
        MeasuredParagraph plain = MeasuredParagraph.Measure(
            Footer,
            [new FormattedRun(0, Footer.Length, CarlitoBold(), Length.FromPoints(10))]);

        plain.Runs.Sum(run => run.Shaped.Glyphs.Count).ShouldBe(45);
    }

    /// <summary>Breaking the ligature widens the run, so measurement has to see it too.</summary>
    /// <remarks>
    /// Carlito-Bold's <c>ti</c> ligature advances 1199 units against <c>t</c>+<c>i</c>'s 1213, so a
    /// tracked run is wider than "untracked plus the gaps" by that difference. A layout that
    /// suppressed the ligature only when <em>drawing</em> would break its lines at the narrower
    /// width and paint them at the wider one.
    /// </remarks>
    [Fact]
    public void TheSuppressedLigatureIsChargedToTheWidth()
    {
        OpenTypeFace face = CarlitoBold();
        Length size = Length.FromPoints(10);
        Length gap = Length.FromTwips(60);
        const string Text = "Formation";

        Length plain = MeasuredParagraph
            .Measure(Text, [new FormattedRun(0, Text.Length, face, size)])
            .WidthBetween(0, Text.Length);
        Length tracked = MeasuredParagraph
            .Measure(Text, [new FormattedRun(0, Text.Length, face, size, Tracking: gap)])
            .WidthBetween(0, Text.Length);

        (tracked - plain).ShouldBeGreaterThan(gap * (Text.Length - 1),
            "the run is the gaps wider plus whatever the ligature was saving");
    }

    // ---------------------------------------------------------------------------------------- setup

    private static readonly Length Size = Length.FromPoints(12);

    private static OpenTypeFace CarlitoBold()
    {
        string? path = FindFont("Carlito-Bold.ttf");
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
}
