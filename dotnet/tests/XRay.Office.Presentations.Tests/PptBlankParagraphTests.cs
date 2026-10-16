using XRay.Office.Core.Extraction;
using XRay.Office.Core.Units;
using XRay.Office.Presentations.Layout;
using XRay.Office.Presentations.MsBinary;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

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
    /// The blank line's own run is found even when it BEGINS at the blank paragraph — which is
    /// where every real deck puts it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The fixture above states one character run over the whole text, so the empty paragraph sits
    /// strictly inside it and the run walk finds it on the first iteration. Real binary decks do
    /// not look like that: PowerPoint writes the blank paragraph's own carriage return as its own
    /// one-character run, so the run that <em>contains</em> the blank paragraph <em>begins</em>
    /// exactly where the run that precedes it ends.
    /// </para>
    /// <para>
    /// The walk used to stop as soon as its running position reached the paragraph's end, which
    /// for a zero-length paragraph is the boundary between those two runs — so it stopped one run
    /// short every time and every blank line in the corpus fell back to the outline level's size.
    /// This is the case the theory above cannot see.
    /// </para>
    /// <para>
    /// The layout of the text is <c>first</c>, CR, CR, <c>last</c>: the blank paragraph is the
    /// zero-length span at index 6, and the run boundaries are 0–6, 6–7, 7–11 — the shape
    /// <c>slides/done-005/ppt/ITE106-Chapter 4.ppt</c> writes on every one of its bullet slides.
    /// The reference draws that deck's blank lines at the middle run's 12 pt against a level
    /// default of 32.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData(12)]
    [InlineData(27)]
    public void TheRunThatBeginsAtABlankParagraphIsStillItsRun(int blankHeight)
    {
        const uint statesHeight = 0x0002_0000;

        PptTextRun run = new(
            PptTextKind.Other,
            $"first{PptTextReader.ParagraphSeparator}{PptTextReader.ParagraphSeparator}last",
            [],
            [
                new PptCharacterRun(6, RunEmphasis.None, RunEmphasis.None, statesHeight, FontHeight: 24),
                new PptCharacterRun(1, RunEmphasis.None, RunEmphasis.None, statesHeight,
                                    FontHeight: (ushort)blankHeight),
                new PptCharacterRun(4, RunEmphasis.None, RunEmphasis.None, statesHeight, FontHeight: 24),
            ]);

        SlideTextBody body = PptTextBody.Build(
            run, styles: null, Scheme, Fonts, SlideTextBody.DefaultInsets,
            TextAnchor.Top, wraps: true).ShouldNotBeNull();

        body.Paragraphs.Count.ShouldBe(3);
        body.Paragraphs[1].Text.ShouldBe(string.Empty);
        body.Paragraphs[1].Runs.Count.ShouldBe(1);
        body.Paragraphs[1].Runs[0].Size.Points.ShouldBe(blankHeight, 0.01);

        // …and the two paragraphs that do carry text are unaffected by the wider walk.
        body.Paragraphs[0].Runs[0].Size.Points.ShouldBe(24, 0.01);
        body.Paragraphs[2].Runs[0].Size.Points.ShouldBe(24, 0.01);
    }

    /// <summary>
    /// A trailing return leaves an empty paragraph behind, and it is a line.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>This asserted the opposite for as long as the reader has existed, and it was never
    /// measured.</strong> <c>PPTStyleTextPropReader::Init</c> reads the string under
    /// <c>while (nCharReadCnt &lt; nStringLen)</c> and then, once the loop is done, appends one
    /// more portion in one more paragraph whenever the last portion it made belongs to the
    /// paragraph before the counter — <c>filter/source/msfilter/svdfppt.cxx</c>:5403-5409 — which
    /// is exactly the case where the text ended on a newline marker.
    /// </para>
    /// <para>
    /// Read out of the reference's own resolved view as well: <c>--convert-to fodp</c> on
    /// <c>pres_ioc_phuket.ppt</c> gives page 26's banner, whose whole text is a single <c>\r</c>,
    /// <b>two</b> <c>&lt;text:p&gt;</c> elements and a shape 3.647 cm — 103.38 pt — tall against
    /// the 2.289 cm its own anchor states. Dropping the second paragraph drew that bar 38.5 pt
    /// short.
    /// </para>
    /// </remarks>
    [Fact]
    public void ATrailingEmptyParagraphIsALine()
    {
        PptTextRun run = new(
            PptTextKind.Other,
            $"only{PptTextReader.ParagraphSeparator}",
            [],
            [new PptCharacterRun(5, RunEmphasis.None, RunEmphasis.None)]);

        SlideTextBody body = PptTextBody.Build(
            run, styles: null, Scheme, Fonts, SlideTextBody.DefaultInsets,
            TextAnchor.Top, wraps: true).ShouldNotBeNull();

        body.Paragraphs.Count.ShouldBe(2);
        body.Paragraphs[1].Text.ShouldBe(string.Empty);
    }

    /// <summary>
    /// That paragraph is measured in the run BEFORE it, not in the run covering its own position.
    /// </summary>
    /// <remarks>
    /// A <c>.ppt</c> states one more character run than it has characters, and PowerPoint writes
    /// something different in it — <c>pres_ioc_phuket.ppt</c> page 26's banner states 40 pt and
    /// then 8 pt for a text of one carriage return. The reference never reads the second: its
    /// loop stops at <c>nCharReadCnt &lt; nStringLen</c> and the portion it appends afterwards is
    /// a copy of <c>aCharPropList.back()</c> (<c>svdfppt.cxx</c>:5403-5409). 26.2.4.2 resolves
    /// that shape to 3.647 cm, which is two lines of 40 pt and not one of 40 and one of 8.
    /// </remarks>
    [Fact]
    public void TheTrailingEmptyParagraphTakesThePrecedingRunsSizeAndNotTheOneAfterIt()
    {
        const uint statesHeight = 0x0002_0000;

        PptTextRun run = new(
            PptTextKind.Other,
            $"{PptTextReader.ParagraphSeparator}",
            [],
            [
                new PptCharacterRun(1, RunEmphasis.None, RunEmphasis.None, statesHeight,
                                    FontHeight: 40),
                new PptCharacterRun(1, RunEmphasis.None, RunEmphasis.None, statesHeight,
                                    FontHeight: 8),
            ]);

        SlideTextBody body = PptTextBody.Build(
            run, styles: null, Scheme, Fonts, SlideTextBody.DefaultInsets,
            TextAnchor.Top, wraps: true).ShouldNotBeNull();

        body.Paragraphs.Count.ShouldBe(2);
        body.Paragraphs[0].Runs[0].Size.Points.ShouldBe(40, 0.01);
        body.Paragraphs[1].Runs[0].Size.Points.ShouldBe(40, 0.01);
    }
}
