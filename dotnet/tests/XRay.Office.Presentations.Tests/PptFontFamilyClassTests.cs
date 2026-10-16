using XRay.Office.Presentations.Layout;
using XRay.Office.Text.Fonts;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// The family class a <c>.ppt</c> declares in <c>lfPitchAndFamily</c> is deliberately not read, and
/// this pins the measurement that says leaving it unread is right.
/// </summary>
/// <remarks>
/// <para>
/// <c>architecture6.ppt</c> page 10 was seated for three rounds as a line-breaking defect — the
/// reference fits 22 lines into a table cell where this tree fits 19 — and then as a possible
/// character-spacing record this reader ignores. It is neither. <strong>The extra width is not
/// stated by the file and it is not spacing: 26.2.4.2 lays that deck's table text out at
/// <em>DejaVu Sans</em>'s advances while drawing <em>Liberation Sans</em>'s glyphs.</strong> It is
/// the seventh confound (L1, <c>probes/title-font-r92</c>) reaching the binary format, and
/// <c>probes/ppt-spacing-r115</c> is the measurement.
/// </para>
/// <para>
/// <strong>No <c>.ppt</c> can state character spacing.</strong> <c>PPTStyleTextPropReader</c>'s
/// character branch (<c>filter/source/msfilter/svdfppt.cxx</c>:5096-5185, the reference's own
/// reader) reads the whole of a <c>TextCFException</c> — the flag word, <c>cfTypeface</c>,
/// <c>cfFEOldTypeface</c>, <c>cfANSITypeface</c>, <c>cfSymbolTypeface</c>, <c>cfSize</c>,
/// <c>cfColor</c>, <c>cfPosition</c> — and there is no tracking field in the record and no
/// <c>PPT_CharAttr_*</c> constant for one (<c>include/filter/msfilter/svdfppt.hxx</c>:1416-1428).
/// </para>
/// <para>
/// <strong>Two one-attribute variants of the deck itself, rendered by 26.2.4.2.</strong> The deck
/// names <c>Helvetica</c> at <c>lfPitchAndFamily = 0x22</c> — <c>FF_SWISS</c> plus variable pitch —
/// in both of its <c>FontEntityAtom</c>s. Changing <em>only</em> that byte to <c>0x02</c>, and
/// separately changing <em>only</em> the name to <c>Arial</c>, each collapse the reference onto this
/// tree: <c>Description</c> goes 89.19 → 77.07 against our 77.03 pt, <c>Disadvantages</c>
/// 116.34 → 98.90 against our 98.83, and over the whole 31-page deck the worst common-run origin
/// shift goes 322.07 → 1.60 pt. Per glyph, 767 of 767 adjacent-pair advances at 14 pt in the
/// reference as authored are nearer DejaVu Sans' <c>hmtx</c> than Liberation Sans' (mean 0.00107 em
/// against 0.07278); with either byte changed, 768 of 772 are nearer Liberation's, exactly as ours
/// are.
/// </para>
/// </remarks>
public class PptFontFamilyClassTests
{
    /// <summary>The size the deck states for its table text.</summary>
    private const double Points = 14.0;

    /// <summary>
    /// A hundredth of a point. The quantities are sums of <c>hmtx</c> entries, so nothing but the
    /// last bit of a division is in play; the term under test is 17%.
    /// </summary>
    private const double Tolerance = 0.01;

    private static readonly string[] SearchDirectories =
    [
        "/usr/share/fonts/truetype/liberation",
        "/usr/share/fonts/truetype/liberation2",
        "/usr/share/fonts/truetype/dejavu",
        "/usr/share/fonts",
    ];

    [Theory]
    [InlineData(0x22, FontPitch.Variable)]   // Helvetica as architecture6.ppt states it: SWISS
    [InlineData(0x02, FontPitch.Variable)]   // the same pitch with the family nibble cleared
    [InlineData(0x12, FontPitch.Variable)]   // ROMAN, the other class fontconfig is sent a generic for
    [InlineData(0x31, FontPitch.Fixed)]      // MODERN and fixed, which does survive into the drawing
    public void TheFamilyNibbleIsDiscardedAndOnlyThePitchSurvives(int declared, FontPitch expected)
    {
        // The whole of the seat. `SlideFonts.PitchIn` masks 0x03, so 0x22 and 0x02 are the same
        // request — which is what makes this tree agree with the face 26.2.4.2 *draws* rather than
        // with the one it *measures*. Reading the high nibble here would move every Helvetica run
        // in the slides track onto DejaVu Sans, which is the reference's other, equally
        // self-inconsistent, horn.
        SlideFonts.PitchIn(declared).ShouldBe(expected);
    }

    [Fact]
    public void TheFamilyClassIsNotEvenAskedFor()
    {
        // A pitch reaches the resolver and a class does not. `SlideFonts.Resolve` builds its
        // `FontRequest` with `pitch` and never with `DeclaredClass`, so no `.ppt` byte can put a
        // slide run on the measured face. The words and sheets paths do pass one — see
        // `FontResolutionTests.ADeclaredShapeBeatsAWeakAliasTheChainWouldHaveTaken` — because there
        // the reference measures and draws in the same face and honouring it is a fix.
        new FontRequest("Helvetica", 700, false, FontPitch.Variable)
            .DeclaredClass.ShouldBe(FontFamilyClass.Unknown);
    }

    [Theory]
    // The five identical strings measured out of both PDFs of architecture6.ppt page 10.
    [InlineData("Name", 38.1309, 45.2471)]
    [InlineData("Description", 77.0137, 90.5488)]
    [InlineData("Example", 57.5859, 66.9443)]
    [InlineData("Advantages", 79.3584, 93.3857)]
    [InlineData("Disadvantages", 98.8203, 116.7510)]
    public void TheTwoHornsOfTheConfoundAreSeventeenPercentApart(
        string text, double drawnFace, double measuredFace)
    {
        // The measurement O57 is being closed on, pinned so the next round does not re-derive it.
        // `drawnFace` is Liberation Sans Bold's own advance sum — what this tree lays out, what the
        // reference *draws*, and what 26.2.4.2 also lays out once the family byte is cleared.
        // `measuredFace` is DejaVu Sans Bold's — what the reference lays that same text out at.
        // Neither is a fitted constant: both are sums of `hmtx` entries.
        OpenTypeFace drawn = Require("LiberationSans-Bold.ttf");
        OpenTypeFace measured = Require("DejaVuSans-Bold.ttf");

        Sum(drawn, text).ShouldBe(drawnFace, Tolerance);
        Sum(measured, text).ShouldBe(measuredFace, Tolerance);

        // And they are far enough apart that no rounding rule reaches from one to the other. A
        // 96 dpi whole-pixel advance — the hypothesis round 113 carried — is worth at most half a
        // pixel a glyph, 0.375 pt, where this is 1.1 to 1.4 pt a glyph.
        (measuredFace / drawnFace).ShouldBeGreaterThan(1.16);
    }

    /// <summary>The design advance of a string, in points at the size the deck states.</summary>
    private static double Sum(OpenTypeFace face, string text)
        => text.Sum(character => (double)face.AdvanceForCharacter(character))
           / face.UnitsPerEm * Points;

    private static OpenTypeFace Require(string fileName)
    {
        string? path = null;
        foreach (string directory in SearchDirectories)
        {
            if (!Directory.Exists(directory)) continue;

            string direct = Path.Combine(directory, fileName);
            if (File.Exists(direct)) { path = direct; break; }

            string[] found = Directory.GetFiles(directory, fileName, SearchOption.AllDirectories);
            if (found.Length > 0) { path = found[0]; break; }
        }

        // The whole effect is contingent on which faces this container has: install `urw-base35` and
        // `Helvetica` answers Nimbus Sans on both of the reference's paths and the confound vanishes.
        // A machine without these two faces cannot witness it and skips rather than fails.
        Assert.SkipWhen(path is null, $"{fileName} is not installed; see check-env.sh");

        OpenTypeFace? face = OpenTypeFace.ReadFile(path!);
        face.ShouldNotBeNull($"{path} should be readable as a font");
        return face!;
    }
}
