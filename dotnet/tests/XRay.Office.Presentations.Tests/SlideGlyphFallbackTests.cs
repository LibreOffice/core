using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Presentations.Layout;
using XRay.Office.Text.Fonts;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// A character a slide run's face cannot draw is measured and drawn in a face that can.
/// </summary>
/// <remarks>
/// <para>
/// The words track wired glyph fallback and the slides track did not, so every deck was laid out
/// with no coverage check at all: <c>SlideTextLayout.Measure</c> called
/// <c>MeasuredParagraph.Measure</c> with no <c>ItemisationOptions</c>, whose default is
/// deliberately no fallback. Both halves of the machinery already existed and were already in use
/// by the word processor and the spreadsheet — <see cref="XRay.Office.Text.Itemisation.FontItemiser"/>
/// splits a run at the characters its face has no glyph for and <see cref="SystemFontResolver"/>
/// answers "what face has this character" — and one call site never joined them.
/// </para>
/// <para>
/// <strong>No gate column can see the symptom, which is why it is asserted here.</strong> An
/// uncovered character shapes to <c>.notdef</c>; a face that declines to draw a missing-glyph box
/// draws nothing at all, and the code point still reaches the PDF with a correct
/// <c>ToUnicode</c>, so <c>pdftotext</c> extracts it and the word count is unmoved. Measured on
/// the corpus deck <c>southern-classic-kennesaw-state-university-final.pptx</c>, whose body text
/// holds 132 U+25D8 inverse bullets: the reference falls back to DejaVu Sans and embeds it, and we
/// kept Carlito and drew 132 blanks — visible only in <c>pdffonts</c>, at six embedded faces
/// against the reference's seven.
/// </para>
/// <para>
/// The embedding is asserted separately from the drawing because they fail apart. A face on its
/// own is enough to shape with and not enough to embed: a PDF writer opens the font program
/// through the reference's face key, so a fallback face named only by its family is announced in
/// the file with no program behind it — which the corpus gate scores as a failure, correctly,
/// because a reader without that font installed sees nothing. Neither assertion names a font:
/// which face covers a given character depends on what is installed, so the test asks the
/// resolved face whether it has the glyph rather than asking what it is called.
/// </para>
/// </remarks>
public sealed class SlideGlyphFallbackTests
{
    /// <summary>U+25D8 ◘, the inverse bullet the corpus deck draws 132 of.</summary>
    private const string Missing = "◘";

    /// <summary>The face the fixture names, and which LibreOffice resolves Calibri to here.</summary>
    private const string LatinFamily = "Carlito";

    [Fact]
    public void TheFixtureFaceCoversNoneOfTheFixtureText()
        // The premise everything below rests on. Were Carlito to grow U+25D8, every assertion here
        // would pass while testing nothing, and this says so first.
        => LatinFace.HasGlyphFor(0x25D8).ShouldBeFalse();

    [Fact]
    public void ACharacterTheRunsFaceLacksIsDrawnFromAFaceThatHasIt()
    {
        List<GlyphRun> drawn = Place($"ab{Missing}cd");

        // Every glyph drawn and none of them .notdef. Asserting the face alone is not enough: a
        // run handed to the right face at the wrong offsets still draws boxes.
        drawn.SelectMany(run => run.Glyphs).ShouldAllBe(glyph => glyph.GlyphId != 0);
        drawn.Sum(run => run.Glyphs.Count).ShouldBe(5);
    }

    [Fact]
    public void TheFallbackFaceIsNamedByAReferenceThatCanBeOpened()
    {
        List<GlyphRun> drawn = Place($"ab{Missing}cd");

        // The sub-run holding the fallback is the one whose family is not the one the run asked
        // for; its face key has to be a path a backend can open rather than the family name again.
        GlyphRun fallback = drawn.Single(run => run.Text == Missing);

        fallback.Font.FamilyName.ShouldNotBe(LatinFamily);
        fallback.Font.FaceKey.ShouldNotBe(fallback.Font.FamilyName);
        File.Exists(fallback.Font.FaceKey).ShouldBeTrue();
    }

    [Fact]
    public void TextTheFaceCoversIsStillOneRun()
        // The no-op case has to stay a genuine no-op: a paragraph split into sub-runs it does not
        // need loses the shaping context at each boundary and measures very slightly wide, which is
        // enough to move a line break.
        => Place("abcde").Count.ShouldBe(1);

    private static List<GlyphRun> Place(string text)
    {
        SlideTextBody body = new()
        {
            Paragraphs =
            [
                new SlideParagraph(
                    text,
                    [new SlideTextRun(0, text.Length, LatinFamily, Size, 400, false, Colour.Black)]),
            ],
        };

        DocRect area = new(
            Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(200));

        return [.. SlideTextLayout.Place(body, area, new SlideFonts(Fonts)).Select(run => run.Run)];
    }

    private static readonly Length Size = Length.FromPoints(18);

    private static SystemFontResolver Fonts { get; } = new(SystemFontIndex.Build());

    private static OpenTypeFace LatinFace { get; } =
        Fonts.LoadOpenType(Fonts.Resolve(new FontRequest(LatinFamily, 400, false)));
}
