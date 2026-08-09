using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Layout;
using Shouldly;

namespace Paperless.Text.Tests;

/// <summary>
/// What proportional line spacing takes its percentage of when an inline picture has made the line
/// taller than its text.
/// </summary>
/// <remarks>
/// <para>
/// The answer is the <em>text</em> height, and the picture's contribution is added afterwards rather
/// than scaled. <c>SwTextFormatter::CalcRealHeight</c>
/// (<c>sw/source/core/text/itrform2.cxx</c>:2441-2453) says so in a comment — <em>"extend line height
/// by (nPropLineSpace - 100) percent of the font height"</em> — and takes the percentage of
/// <c>GetLineSpacingBaseHeight()</c>, which only a portion that <c>IsUsedToCalcLineSpacingHeight</c>
/// ever raises. A fly-in-content is not one, so <c>SwLineLayout::Height(nNew, bText)</c>
/// (<c>porlay.cxx</c>:110) grows the line and leaves the base alone.
/// </para>
/// <para>
/// Measured against the installed 24.2.7.2 on six authored DOCX probes, a 150 pt picture in a 12 pt
/// Liberation Serif paragraph: at 150% LibreOffice's gap between the paragraphs above and below is
/// 177.6 pt where scaling the line gives 252.6; at 200% it is 191.4 against 341.4; with 12 pt of text
/// beside the picture 180.2, and with 36 pt of text 199.15 — which is the same rule reading the text
/// height off the line rather than off the paragraph.
/// </para>
/// <para>
/// <strong>Below a hundred per cent the whole line really is scaled</strong>, and that is not an
/// exception invented to fit: Writer takes the other branch there — <c>SvxLineSpaceRule::Auto</c> under
/// <c>PROP_LINE_SPACING_SHRINKS_FIRST_LINE</c>, which multiplies <c>nLineHeight</c> — and the probe at
/// 75% comes back 122.85, which is 150 × 0.75 and not 150 − 25% of the text.
/// </para>
/// </remarks>
public class InlineObjectLineSpacingTests
{
    private static readonly Length Twelve = Length.FromPoints(12);
    private static readonly Length Picture = Length.FromPoints(150);

    /// <summary>
    /// The rule itself: half again of the <em>text</em>, added to a line a picture already fills.
    /// </summary>
    [Fact]
    public void ProportionalSpacingAddsAShareOfTheTextHeightRatherThanScalingTheLine()
    {
        Length text = Length.FromPoints(13.8);
        LineSpacingRule rule = LineSpacingRule.Multiple(1.5);

        rule.Apply(Picture, text).ShouldBe(Picture + Length.FromPoints(6.9));

        // The reading this replaces, kept so the difference is a number rather than a description.
        rule.Apply(Picture).ShouldBe(Length.FromPoints(225));
    }

    /// <summary>Two hundred per cent adds the text height whole, which fixes the slope of the rule.</summary>
    /// <remarks>
    /// One point on a line is a rule; two are a slope. Without this a rule of "add half the text height"
    /// would pass just as well as "add (prop − 100)% of it".
    /// </remarks>
    [Fact]
    public void TwiceSpacedAddsTheWholeTextHeight()
    {
        Length text = Length.FromPoints(13.8);

        LineSpacingRule.Multiple(2.0).Apply(Picture, text)
            .ShouldBe(Picture + text);
    }

    /// <summary>Below full spacing the picture is scaled with everything else.</summary>
    [Fact]
    public void BelowFullSpacingTheWholeLineIncludingTheObjectIsScaled()
    {
        Length text = Length.FromPoints(13.8);

        LineSpacingRule.Multiple(0.75).Apply(Picture, text)
            .ShouldBe(Length.FromPoints(112.5));
    }

    /// <summary>An ordinary line of text is untouched: its text height <em>is</em> its height.</summary>
    [Fact]
    public void APlainLineOfTextMeasuresTheSameHeightEitherWay()
    {
        OpenTypeFace face = Carlito();
        const string Text = "ordinary";

        (Length height, _, Length text) = MeasuredParagraph
            .Measure(Text, [new FormattedRun(0, Text.Length, face, Twelve)])
            .MeasureLine(0, Text.Length);

        text.ShouldBe(height);
        LineSpacingRule.Multiple(1.5).Apply(height, text)
            .ShouldBe(LineSpacingRule.Multiple(1.5).Apply(height));
    }

    /// <summary>
    /// A line an inline picture has made taller reports the two heights apart.
    /// </summary>
    [Fact]
    public void AnInlinePictureRaisesTheLineAndLeavesTheTextHeightAlone()
    {
        OpenTypeFace face = Carlito();
        const string Text = "x";

        (Length height, _, Length text) = MeasuredParagraph
            .Measure(
                Text,
                [new FormattedRun(0, 1, face, Twelve)],
                objects: [new InlineObject(0, Length.Zero, Picture)])
            .MeasureLine(0, Text.Length);

        height.ShouldBeGreaterThan(Picture);
        text.ShouldBeLessThan(Length.FromPoints(20));

        (Length plain, _, _) = MeasuredParagraph
            .Measure(Text, [new FormattedRun(0, 1, face, Twelve)])
            .MeasureLine(0, Text.Length);
        text.ShouldBe(plain);
    }

    /// <summary>
    /// And the layouter uses it, which is the part the corpus feels.
    /// </summary>
    /// <remarks>
    /// The per-run path is the one a paragraph with an inline object always takes, so this is the
    /// measurement that decides where the page breaks.
    /// </remarks>
    [Fact]
    public void TheLayouterGivesSuchALineTheTextHeightsShareAndNotTheLines()
    {
        OpenTypeFace face = Carlito();
        const string Text = "x";

        MeasuredParagraph measured = MeasuredParagraph.Measure(
            Text,
            [new FormattedRun(0, 1, face, Twelve)],
            objects: [new InlineObject(0, Length.Zero, Picture)]);

        (Length natural, _, Length text) = measured.MeasureLine(0, Text.Length);

        LaidOutParagraph laid = new ParagraphLayouter(face).Layout(
            measured,
            ParagraphFormat.Default with { LineSpacing = LineSpacingRule.Multiple(1.5) },
            Length.FromMillimetres(170));

        laid.Lines.Count.ShouldBe(1);
        laid.Lines[0].Height.ShouldBe(
            LineSpacingRule.Multiple(1.5).Apply(natural, text));
        laid.Lines[0].Height.ShouldBeLessThan(natural + Length.FromPoints(20));
    }

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
}
