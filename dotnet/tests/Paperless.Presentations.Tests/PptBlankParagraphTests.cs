using Paperless.Core.Extraction;
using Paperless.Core.Units;
using Paperless.Presentations.Layout;
using Paperless.Presentations.MsBinary;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// A blank line between two paragraphs of a binary PowerPoint shape.
/// </summary>
/// <remarks>
/// <para>
/// It is a blank <em>line</em>, not nothing. An empty paragraph covers no characters, so the run
/// builder placed nothing in it, so slide layout — which drops a paragraph that resolves no face
/// at all — dropped it whole and everything below it moved up by a line. PowerPoint decks use
/// empty paragraphs as spacing constantly, and the PPTX reader has always emitted one run for
/// them; only the binary path did not.
/// </para>
/// <para>
/// Its height is the height of the character run it sits inside rather than the outline level's
/// default, which is the second half of the same defect: authors shrink their blank lines, and
/// taking the level's size instead made every gap too tall. On the fourth page of
/// <c>slides/batch-001/ppt/WC_Update-Aug03.ppt</c>, which separates all eleven of its bullets that
/// way, the level's size overflowed the last item off the bottom of the slide.
/// </para>
/// </remarks>
public class PptBlankParagraphTests
{
    private static readonly PptColourScheme Scheme = PptColourScheme.Default;

    private static PptFontTable Fonts => PptFontTable.Empty;

    /// <summary>Three paragraphs, the middle one empty, with one 12 pt character run over all.</summary>
    private static SlideTextBody Body(int fontHeight)
    {
        // The mask bit for a stated font height, so the run's own size wins over the level's.
        PptTextRun run = new(
            PptTextKind.Other,
            $"first{PptTextReader.ParagraphSeparator}{PptTextReader.ParagraphSeparator}last",
            [],
            [new PptCharacterRun(
                Length: 11, RunEmphasis.None, RunEmphasis.None,
                Mask: 0x0002_0000, FontHeight: (ushort)fontHeight)]);

        return PptTextBody.Build(
            run,
            styles: null,
            Scheme,
            Fonts,
            SlideTextBody.DefaultInsets,
            TextAnchor.Top,
            wraps: true).ShouldNotBeNull();
    }

    [Fact]
    public void AnEmptyParagraphBetweenTwoOthersSurvives()
    {
        SlideTextBody body = Body(12);

        body.Paragraphs.Count.ShouldBe(3);
        body.Paragraphs[1].Text.ShouldBe(string.Empty);

        // With a run, so it resolves a face and layout gives it a line's height.
        body.Paragraphs[1].Runs.Count.ShouldBe(1);
        body.Paragraphs[1].Runs[0].Length.ShouldBe(0);
    }

    [Fact]
    public void ItTakesItsHeightFromTheRunItSitsInsideRatherThanTheLevel()
    {
        // The stated run is 12 pt where the fallback level is 18; a blank line set at the level's
        // size is half again as tall as the reference draws it.
        Body(12).Paragraphs[1].Runs[0].Size.Points.ShouldBe(12, 0.01);
        Body(40).Paragraphs[1].Runs[0].Size.Points.ShouldBe(40, 0.01);
    }

    [Fact]
    public void TheBlankLineIsAsTallAsARealOne()
    {
        SlideTextBody body = Body(12);
        SlideFonts fonts = new();

        Length withGap = SlideTextLayout.Height(body, Length.FromPoints(400), fonts);

        SlideTextBody without = body with
        {
            Paragraphs = [body.Paragraphs[0], body.Paragraphs[2]],
        };
        Length withoutGap = SlideTextLayout.Height(without, Length.FromPoints(400), fonts);

        // A whole line taller, which is what a dropped paragraph costs.
        (withGap - withoutGap).Points.ShouldBeGreaterThan(12 * 1.1);
    }

    /// <summary>
    /// A trailing empty paragraph is still dropped, because it is the terminator's artefact rather
    /// than a line the author wrote.
    /// </summary>
    [Fact]
    public void ATrailingEmptyParagraphIsStillNotALine()
    {
        PptTextRun run = new(
            PptTextKind.Other,
            $"only{PptTextReader.ParagraphSeparator}",
            [],
            [new PptCharacterRun(5, RunEmphasis.None, RunEmphasis.None)]);

        SlideTextBody body = PptTextBody.Build(
            run, styles: null, Scheme, Fonts, SlideTextBody.DefaultInsets,
            TextAnchor.Top, wraps: true).ShouldNotBeNull();

        body.Paragraphs.Count.ShouldBe(1);
    }
}
