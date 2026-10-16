using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Layout;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// The uniform-paragraph shortcut is taken only where it measures what the page draws.
/// </summary>
/// <remarks>
/// <para>
/// A paragraph whose formatting does not vary carries no runs, and the two layouters measured it by
/// shaping the whole text once in the paragraph's own face — no itemisation, and so no glyph
/// fallback. The <em>drawing</em> pass has never had that shortcut: <c>PageDrawing.RunsIn</c> sends
/// every paragraph through <c>FontItemiser.Split</c> whether it has runs or not, and its own comment
/// says the cut is made "as the measurement cut them".
/// </para>
/// <para>
/// For a paragraph of Chinese in a Latin face that is false in the worst way. The drawing pass finds
/// a CJK face and draws an em per character; the measuring pass shapes every character to Liberation
/// Serif's <c>.notdef</c> and charges 1593 of 2048 units, 0.778 em. Measured on
/// <c>手机免提系统TSB.doc</c>: a line filled to 44 characters where 34 fit, 410.70 pt of measured
/// width painted as 528, running 6.8 pt past the edge of the page itself — and 179 of the document's
/// 1530 characters were pushed off the page and out of the extracted text altogether.
/// </para>
/// <para>
/// <see cref="GlyphFallbackWiringTests"/> already covers the split and never caught this, because it
/// asks <c>PageParagraph.Measure()</c> for the measurement directly — and <c>Measure()</c> was right
/// all along. What was wrong was that nothing called it. These tests go through the layouters, which
/// is where the choice is made.
/// </para>
/// </remarks>
public sealed class UniformParagraphMeasurementTests
{
    private const string Chinese = "由于部分车辆手机免提系统模块的故障可能会造成手机免提系统通话不畅";

    private static readonly Length Size = Length.FromPoints(12);

    private static OpenTypeFace LatinFace()
    {
        string? path = Find("LiberationSerif-Regular.ttf");
        Assert.SkipWhen(path is null, "Liberation Serif is not installed; see check-env.sh");
        return OpenTypeFace.ReadFile(path!).ShouldNotBeNull();
    }

    private static string? Find(string fileName)
    {
        foreach (string directory in (string[])
                 ["/usr/share/fonts/truetype/liberation", "/usr/share/fonts"])
        {
            if (!Directory.Exists(directory)) continue;

            string? hit = Directory
                .EnumerateFiles(directory, fileName, SearchOption.AllDirectories)
                .FirstOrDefault();
            if (hit is not null) return hit;
        }

        return null;
    }

    private static PageParagraph Paragraph(bool withFallback)
    {
        IGlyphFallbackResolver resolver = SystemFontResolver.Build();

        return new PageParagraph
        {
            Text = Chinese,
            Face = LatinFace(),
            EmSize = Size,
            Fallback = withFallback ? resolver : null,
        };
    }

    [Fact]
    public void TheFixtureIsAParagraphItsOwnFaceCannotDraw()
    {
        // The premise. It has no runs — so it is uniform by every test the layouters make — and its
        // face has no glyph for a single character of it.
        PageParagraph paragraph = Paragraph(withFallback: true);

        paragraph.HasRuns.ShouldBeFalse();
        paragraph.Face.HasGlyphFor(Chinese[0]).ShouldBeFalse();
        paragraph.NeedsGlyphFallback.ShouldBeTrue();
    }

    [Fact]
    public void AParagraphWhoseFaceCanDrawItKeepsTheShortcut()
    {
        // The shortcut is not being removed, only conditioned. Latin prose in a Latin face still
        // takes the single-face path, which is the common case and by far the cheaper one.
        PageParagraph latin = new()
        {
            Text = "ordinary latin prose that needs no fallback at all",
            Face = LatinFace(),
            EmSize = Size,
            Fallback = SystemFontResolver.Build(),
        };

        latin.NeedsGlyphFallback.ShouldBeFalse();
        latin.HasScriptSpace.ShouldBeFalse();
    }

    [Fact]
    public void WithoutAResolverThereIsNothingToRouteFor()
        // A caller that supplies no resolver cannot benefit from the per-run path, so it must not be
        // sent down it — the answer would be the same and cost a prefix table.
        => Paragraph(withFallback: false).NeedsGlyphFallback.ShouldBeFalse();

    [Fact]
    public void TheLaidOutLineIsAsWideAsTheGlyphsTheFallbackFaceDraws()
    {
        // The defect in one number. Liberation Serif's .notdef is 1593/2048 of the em, so measuring
        // this text through it gives 0.778 em a character; the CJK face the drawing pass resolves
        // gives an em apiece. At 12 pt over 32 characters that is 298.7 pt against 384.
        PageParagraph paragraph = Paragraph(withFallback: true);
        Length wide = Length.FromPoints(1000);

        PlacedFlow? flow = FlowLayouter.LayOut(
            [paragraph], new DocRect(Length.Zero, Length.Zero, wide, wide), Length.Zero);

        flow.ShouldNotBeNull();
        flow!.Lines.Count.ShouldBe(1);

        // Not asserted against a font's name: which face covers CJK depends on what is installed.
        // What is asserted is that the width is the drawn one and not the missing-glyph one.
        double perCharacter = flow.Lines[0].Box.Line.Width.Points / Chinese.Length;
        perCharacter.ShouldBeGreaterThan(11.0);
        perCharacter.ShouldBeLessThan(13.0);
    }

    [Fact]
    public void ANarrowColumnBreaksTheLineWhereTheDrawnGlyphsPutIt()
    {
        // The consequence that costs pages. In a 120 pt column the drawn glyphs fit ten to a line and
        // .notdef's 0.778 em fits twelve — so measuring through .notdef fills every line two
        // characters past the margin and the overflow is painted off the page.
        PageParagraph paragraph = Paragraph(withFallback: true);
        Length column = Length.FromPoints(120);

        PlacedFlow? flow = FlowLayouter.LayOut(
            [paragraph],
            new DocRect(Length.Zero, Length.Zero, column, Length.FromPoints(1000)),
            Length.Zero);

        flow.ShouldNotBeNull();

        // Ten drawn characters to a 120 pt line and 32 characters in all, so four lines. Measuring
        // through .notdef's 0.778 em fits twelve to a line and gives three — and paints the twelfth
        // two characters past the margin, because the pen advances by what is drawn.
        flow!.Lines.Count.ShouldBe(4);
        flow.Lines[0].Box.Line.End.ShouldBe(10);

        // The line the *pen* draws, which is the half the measurement used to disagree with. Every
        // glyph run on every line has to end within the column it was broken to.
        foreach (PlacedLine line in flow.Lines)
        {
            Drawn(paragraph, line, column).ShouldBeLessThanOrEqualTo(column.Points + 0.01);
        }
    }

    /// <summary>How far right the glyphs of one line actually reach, in points.</summary>
    private static double Drawn(PageParagraph paragraph, PlacedLine line, Length column)
    {
        DocRect area = new(Length.Zero, Length.Zero, column, Length.FromPoints(1000));

        return PageDrawing
            .RunsIn(area, line, paragraph, highlights: null, rules: null)
            .Select(pair => pair.Run)
            .Select(run => run.Origin.X.Points
                           + run.Glyphs.Sum(glyph => glyph.Advance.Points))
            .DefaultIfEmpty(0)
            .Max();
    }

    [Fact]
    public void EveryCharacterSurvivesTheBreak()
    {
        // What the corpus document lost: the lines have to partition the text, with no gap between
        // one line's end and the next line's start.
        PageParagraph paragraph = Paragraph(withFallback: true);

        PlacedFlow? flow = FlowLayouter.LayOut(
            [paragraph],
            new DocRect(Length.Zero, Length.Zero, Length.FromPoints(120), Length.FromPoints(1000)),
            Length.Zero);

        flow.ShouldNotBeNull();

        int at = 0;
        foreach (PlacedLine line in flow!.Lines)
        {
            line.Box.Line.Start.ShouldBe(at);
            at = line.Box.Line.End;
        }

        at.ShouldBe(Chinese.Length);
    }
}
