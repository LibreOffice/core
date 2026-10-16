using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Layout;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// The size of a run that holds nothing but a frame anchor does not reach the line's height.
/// </summary>
/// <remarks>
/// <para>
/// U+0001 is what every word-processing reader puts where a floating frame, an as-character picture
/// or a comment mark sits. The run around it still carries a font size and documents routinely state
/// a large one — a logo run set at 26 pt because that is what the heading beside it was. Writer
/// builds a line out of portions and none of the three is a text portion, so that size never reaches
/// <c>SwLineLayout::Height</c>.
/// </para>
/// <para>
/// Measured against LibreOffice 26.2.4.2 on ten authored variants of the block-1 paragraph of
/// <c>097_Business_Case_Template_Elegant_Layout</c> — anchored against as-character, 10 pt against
/// 26 pt, alone against with text beside it — reading the height the paragraph adds over an empty
/// one:
/// </para>
/// <code>
///   case                                       reference   before   after
///   a run of text at 26 pt                         20.60    19.10    19.10
///   anchored drawing, run at 10 pt                  0.00    -1.10     0.00
///   anchored drawing, run at 26 pt                  0.00    17.25     0.00
///   anchored drawing at 10 pt, text beside it       0.00     0.00     0.00
///   anchored drawing at 26 pt, text beside it       0.00    17.25     0.00
///   as-character drawing, run at 10 pt              7.00     6.95     6.95
///   as-character drawing, run at 26 pt              7.00    17.25     6.95
///   as-character at 10 pt, text beside it           9.70     9.70     9.70
///   as-character at 26 pt, text beside it           9.70    17.25     9.70
/// </code>
/// <para>
/// <b>The reference's answer is the same at both sizes on every row.</b> The rows we were already
/// exact on are the rows where the run's size happened to match the paragraph's, which is why this
/// never read as a systematic error: it is worth 34 pt on one document and nothing on most.
/// </para>
/// <para>
/// Two halves, and both are needed. The word-processing half — <c>PageParagraph.Measure</c> — gives
/// such a run the paragraph's own face and size, which is the reference's answer when the anchor is
/// alone on its line. The text half — <c>MeasuredParagraph.Fold</c> — passes over it while anything
/// else is on the line, which is the reference's answer when it is not. With only the first, the two
/// rows that were exact came out 0.20 and 0.80 pt wrong.
/// </para>
/// </remarks>
public class AnchorRunHeightTests
{
    private const string Anchor = "\u0001";
    private static readonly Length Small = Length.FromPoints(11);
    private static readonly Length Large = Length.FromPoints(26);

    /// <summary>An anchor alone on its line is as tall as the paragraph, at any run size.</summary>
    /// <remarks>
    /// Three sizes rather than one, so the assertion is that the answer does not depend on the size
    /// rather than that it happens to equal one particular number.
    /// </remarks>
    [Theory]
    [InlineData(11)]
    [InlineData(26)]
    [InlineData(72)]
    public void AnAnchorAloneIsAsTallAsTheParagraphWhateverTheRunSays(int points)
    {
        Length height = HeightOf(Paragraph(Anchor, [Run(0, 1, Length.FromPoints(points))]), 0, 1);

        height.ShouldBe(HeightOf(Paragraph("x", [Run(0, 1, Small)]), 0, 1));
    }

    /// <summary>With text beside it, the text decides and the anchor's run says nothing.</summary>
    [Theory]
    [InlineData(11)]
    [InlineData(26)]
    [InlineData(72)]
    public void TextBesideAnAnchorDecidesTheLineWhateverTheAnchorsRunSays(int points)
    {
        PageParagraph paragraph = Paragraph(
            Anchor + "Y", [Run(0, 1, Length.FromPoints(points)), Run(1, 1, Small)]);

        HeightOf(paragraph, 0, 2).ShouldBe(HeightOf(Paragraph("Y", [Run(0, 1, Small)]), 0, 1));
    }

    /// <summary>
    /// The text beside an anchor decides the line even when it is not the paragraph's own size.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The discriminating case between the two halves of the rule, and it is the one that caught the
    /// first cut. Giving the anchor's run the paragraph's face and size is enough whenever the
    /// paragraph's size is the text's; where they differ — a paragraph whose body style is 11 pt
    /// Carlito, a picture run, and a 12 pt Cambria run of text beside it — the substituted size then
    /// becomes a floor the reference does not have. Measured on the authored variants: with the
    /// word-processing half alone, the two rows the reference puts at 0.00 and 9.70 came out 0.80 and
    /// 9.90, and with <c>Fold</c> passing over the run as well they are 0.00 and 9.70 again.
    /// </para>
    /// <para>
    /// Two directions, so it cannot be satisfied by a rule that always takes the smaller or always
    /// takes the larger.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData(8)]
    [InlineData(20)]
    public void TheTextsOwnSizeDecidesEvenWhenItIsNotTheParagraphs(int points)
    {
        Length size = Length.FromPoints(points);

        PageParagraph paragraph = Paragraph(
            Anchor + "Y", [Run(0, 1, Large), Run(1, 1, size)]);

        HeightOf(paragraph, 0, 2).ShouldBe(HeightOf(Paragraph("Y", [Run(0, 1, size)]), 0, 1));
    }

    /// <summary>
    /// A run holding an anchor <em>and</em> text keeps its own size, because then it is a text portion.
    /// </summary>
    /// <remarks>
    /// The control that makes the rule falsifiable. Without it the rule could be "any run touching an
    /// anchor is ignored", which would silence the size of every run holding a picture next to its own
    /// caption — and those are common.
    /// </remarks>
    [Fact]
    public void ARunHoldingAnAnchorAndTextKeepsItsSize()
    {
        Length mixed = HeightOf(Paragraph(Anchor + "Y", [Run(0, 2, Large)]), 0, 2);

        mixed.ShouldBe(HeightOf(Paragraph("xY", [Run(0, 2, Large)]), 0, 2));
        mixed.ShouldBeGreaterThan(HeightOf(Paragraph("xY", [Run(0, 2, Small)]), 0, 2));
    }

    /// <summary>An ordinary large run is still large, which is the other half of the control.</summary>
    [Fact]
    public void AnOrdinaryRunIsUnaffected()
        => HeightOf(Paragraph("xY", [Run(0, 1, Large), Run(1, 1, Small)]), 0, 2)
            .ShouldBe(HeightOf(Paragraph("xY", [Run(0, 2, Large)]), 0, 2));

    /// <summary>
    /// An as-character picture's own height decides its line, and still does not depend on the run.
    /// </summary>
    /// <remarks>
    /// The inline arm, where something real does raise the line: the object is folded in after the runs
    /// are, so silencing the run must not silence the picture. Both sizes again.
    /// </remarks>
    [Theory]
    [InlineData(11)]
    [InlineData(26)]
    public void AnAsCharacterPictureIsAsTallAsThePictureWhateverTheRunSays(int points)
    {
        MeasuredParagraph measured = MeasuredParagraph.Measure(
            Anchor,
            [new FormattedRun(0, 1, Face, Small)],
            objects: [new InlineObject(0, Length.FromPoints(60), Length.FromPoints(40))]);

        MeasuredParagraph stated = MeasuredParagraph.Measure(
            Anchor,
            [new FormattedRun(0, 1, Face, Length.FromPoints(points))],
            objects: [new InlineObject(0, Length.FromPoints(60), Length.FromPoints(40))]);

        // The run is silenced in both, so the two agree; and the picture still raises the line well
        // past what an 11 pt line would be.
        stated.MeasureLine(0, 1).Height.ShouldBe(measured.MeasureLine(0, 1).Height);
        stated.MeasureLine(0, 1).Height.ShouldBeGreaterThan(Length.FromPoints(30));
    }

    /// <summary>
    /// The rule is the run's <em>range</em>, not the paragraph's text: an anchor inside a text run
    /// changes nothing.
    /// </summary>
    [Fact]
    public void AnAnchorInTheMiddleOfATextRunDoesNotSilenceIt()
        => HeightOf(Paragraph("x" + Anchor + "Y", [Run(0, 3, Large)]), 0, 3)
            .ShouldBe(HeightOf(Paragraph("xxY", [Run(0, 3, Large)]), 0, 3));

    private static Length HeightOf(PageParagraph paragraph, int start, int end)
        => paragraph.Measure().MeasureLine(start, end).Height;

    private static PageRun Run(int start, int length, Length size)
        => new(start, length, Face, size);

    private static PageParagraph Paragraph(string text, IReadOnlyList<PageRun> runs)
        => new()
        {
            Text = text,
            Face = Face,
            EmSize = Small,
            Runs = runs,
        };

    private static OpenTypeFace Face { get; } = Resolve();

    private static OpenTypeFace Resolve()
    {
        SystemFontResolver resolver = new(SystemFontIndex.Build());
        return resolver.LoadOpenType(
            resolver.Resolve(new FontRequest("Liberation Serif", 400, false)));
    }
}
