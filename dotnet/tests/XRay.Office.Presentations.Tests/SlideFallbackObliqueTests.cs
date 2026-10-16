using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Presentations.Layout;
using XRay.Office.Text.Fonts;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// A slide character drawn from a fallback face keeps the lean of the run it came out of.
/// </summary>
/// <remarks>
/// <para>
/// The words track found this and it is not a words defect: <c>SlideTextLayout.Block.FontFor</c>
/// names a substituted face through the same reverse lookup, which has no request to compare
/// against, so an italic run whose glyph came from a fallback face was drawn upright.
/// </para>
/// <para>
/// Measured on the slides corpus before the change: 101 of 302 decks draw a face no document names,
/// and we leaned <b>4</b> of those 5 530 glyphs against the reference's <b>345</b> of 5 242 —
/// <c>outlook_of_nigerian_pension_sector.ppt</c> alone accounting for 341. The rule was measured
/// through <c>.pptx</c> and <c>.fodp</c> among four other filters
/// (<c>probes/words-r58/fallback-oblique-ooxml.py</c>): six sheared glyphs on the reference and
/// none here, with an upright control at nought on both sides.
/// </para>
/// </remarks>
public sealed class SlideFallbackObliqueTests
{
    /// <summary>U+25D8 ◘, which Carlito does not cover.</summary>
    private const string Missing = "◘";

    /// <summary>The face the fixture names, and which has an italic of its own installed.</summary>
    private const string LatinFamily = "Carlito";

    private static readonly Length Size = Length.FromPoints(18);

    [Fact]
    public void TheFixtureFaceCoversNoneOfTheFixtureTextAndTheSubstituteHasNoItalic()
    {
        // Both premises. If Carlito grew U+25D8 nothing would fall back; if the covering face had
        // an italic the right answer would be that face rather than a synthetic lean.
        LatinFace.HasGlyphFor(0x25D8).ShouldBeFalse();
        Fonts.FallbackFor(0x25D8).ShouldNotBeNull().IsItalic.ShouldBeFalse();
    }

    [Fact]
    public void AFallbackFaceInAnItalicRunIsDrawnLeaning()
        => Fallback(Place(italic: true)).Font.SyntheticOblique.ShouldBeTrue();

    [Fact]
    public void AFallbackFaceInAnUprightRunIsNotDrawnLeaning()
        // The control that separates the fix from "the fallback face always leans".
        => Fallback(Place(italic: false)).Font.SyntheticOblique.ShouldBeFalse();

    [Fact]
    public void TheRunsAroundItKeepTheirOwnAnswer()
    {
        // Carlito's italic *is* installed, so the Latin pieces resolve to a real italic face and
        // must not also be sheared. A change that leaned everything passes the first test and not
        // this one.
        List<GlyphRun> drawn = Place(italic: true);

        drawn.Count.ShouldBeGreaterThan(1);
        drawn.Where(run => run.Text != Missing).ShouldAllBe(run => !run.Font.SyntheticOblique);
    }

    [Fact]
    public void TheLeaningFallbackIsStillNamedByAReferenceThatCanBeOpened()
    {
        FontReference reference = Fallback(Place(italic: true)).Font;

        reference.FamilyName.ShouldNotBe(LatinFamily);
        File.Exists(reference.FaceKey.Split('#')[0]).ShouldBeTrue();
    }

    private static GlyphRun Fallback(List<GlyphRun> drawn) => drawn.Single(run => run.Text == Missing);

    private static List<GlyphRun> Place(bool italic)
    {
        string text = $"ab{Missing}cd";
        SlideTextBody body = new()
        {
            Paragraphs =
            [
                new SlideParagraph(
                    text,
                    [new SlideTextRun(0, text.Length, LatinFamily, Size, 400, italic, Colour.Black)]),
            ],
        };

        DocRect area = new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(200));

        return [.. SlideTextLayout.Place(body, area, new SlideFonts(Fonts)).Select(run => run.Run)];
    }

    private static SystemFontResolver Fonts { get; } = new(SystemFontIndex.Build());

    private static OpenTypeFace LatinFace { get; } =
        Fonts.LoadOpenType(Fonts.Resolve(new FontRequest(LatinFamily, 400, false)));
}
