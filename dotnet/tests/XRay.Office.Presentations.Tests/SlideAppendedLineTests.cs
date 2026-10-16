using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Presentations.Layout;
using XRay.Office.Presentations.MsBinary;
using XRay.Office.Text.Layout;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// The line EditEngine appends — for an empty paragraph, and after a trailing hard line break —
/// is measured by a different rule from every other line, and takes no part of the
/// shrink-to-fit's line-spacing scale.
/// </summary>
/// <remarks>
/// <para>
/// <c>ImpEditEngine::CreateAndInsertEmptyLine</c>
/// (<c>editeng/source/editeng/impedit3.cxx</c>:1851-1996) is reached from
/// <c>createLinesForEmptyParagraph</c> (<c>:514-524</c>) and from the end of <c>CreateLines</c>
/// (<c>:1843-1844</c>), and it carries its own copy of the line-spacing arms: <strong>no
/// <c>SvxInterLineSpaceRule::Off</c> arm at all</strong>, no <c>scaleYSpacingValue</c> anywhere,
/// whole-percent integer arithmetic for <c>Prop</c>, and the <c>Prop</c> arm skipped outright for
/// the body's first paragraph. The ordinary arms (<c>:1528-1602</c>) do all four.
/// </para>
/// <para>
/// <strong>Measured at the reference on page 2 of
/// <c>slides/done-013/ppt/2015-Civil-Rights-Website-training.ppt</c></strong>, whose outline
/// placeholder holds four such lines among ten, all at 32 pt with 90 per cent line spacing and
/// 8 pt of space above each paragraph, in a box of 12870 hundredths of a millimetre. 26.2.4.2
/// draws it at 30 pt with bullet-to-bullet distances of 75.912 pt and 68.741 pt, which resolve to
/// a text line of 1029 units, an appended line of 1143 and a space of 253 — the text line
/// carrying both the paragraph's 90 per cent and the fit's nine tenths, the appended line
/// carrying only the 90 per cent. Reproducing that puts the fit's first level at 13239 units
/// against 12871 available, so it overflows and the reference's second level is taken; treating
/// the appended lines like the others puts it at 12758, which fits by 113 units and draws 32 pt.
/// With both rules in place this tree's five text baselines on that page agree with the
/// reference's to <strong>0.010 pt</strong>.
/// </para>
/// </remarks>
public class SlideAppendedLineTests
{
    /// <summary>Liberation Sans at 32 pt, which is what the witness sets its outline in.</summary>
    private const double Points = 32;

    /// <summary>
    /// 32 pt through the draw layer's grid and the reference device is 1130 hundredths of a
    /// millimetre, whose 1.2 em line is 1356.
    /// </summary>
    private const long UnscaledLine = 1356;

    /// <summary>
    /// The line an appended line keeps at 90 per cent: <c>1356 × 90 / 100</c>, truncated.
    /// </summary>
    private const long AppendedLine = 1220;

    /// <summary>
    /// A text line at the fit's first row — the paragraph's 90 per cent and the fit's nine tenths
    /// in one rounding, <c>fround(1356 × 0.81)</c>.
    /// </summary>
    private const long TightenedLine = 1098;

    /// <summary>The same em at the fit's second row, 0.925: <c>round(32 × 0.925) = 30</c> pt.</summary>
    private const long ShrunkEm = 1058;

    private static SlideTextRun Run(string text)
        => new(0, text.Length, "Liberation Sans", Length.FromPoints(Points), 400, false,
               Colour.Black);

    private static SlideParagraph Paragraph(
        string text, double proportion, double spaceBeforePoints = 0)
        => new(
            text,
            [Run(text)],
            SpaceBefore: Length.FromPoints(spaceBeforePoints),
            LineSpacing: proportion is 1.0 ? default : LineSpacingRule.Multiple(proportion));

    private static SlideTextBody Body(bool autoFit, params SlideParagraph[] paragraphs) => new()
    {
        AutoFit = autoFit,
        FontIndependentLineSpacing = true,
        Insets = new Margins(Length.Zero, Length.Zero, Length.Zero, Length.Zero),
        Paragraphs = paragraphs,
    };

    private static List<PlacedGlyphRun> Placed(SlideTextBody body, long boxMm100)
    {
        DocRect area = new(
            Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromMm100(boxMm100));

        List<PlacedGlyphRun> placed = SlideTextLayout.Place(body, area, new SlideFonts());
        placed.ShouldNotBeEmpty();

        return placed;
    }

    private static List<Length> Baselines(SlideTextBody body, long boxMm100)
        => [.. Placed(body, boxMm100).Select(p => p.Run.Origin.Y).Distinct().Order()];

    /// <summary>
    /// An empty paragraph between two bulleted ones keeps its own 90 per cent and is not
    /// tightened by the fit, so the distance across it is not twice the text line's.
    /// </summary>
    /// <remarks>
    /// The box is chosen so the walk stops on <c>constScaleLevels</c>' first row — full font size,
    /// nine-tenths spacing — which is the row where the two readings differ by the whole of the
    /// scale. Treating the appended line like the others gives 2196 across the pair.
    /// </remarks>
    [Fact]
    public void AnEmptyParagraphsLineTakesNoneOfTheFitsSpacingScale()
    {
        SlideTextBody body = Body(
            autoFit: true,
            Paragraph("Alpha", 0.9), Paragraph(string.Empty, 0.9), Paragraph("Beta", 0.9));

        // 3 x 1220 unscaled overflows a 3500-unit box; 1098 + 1220 + 1098 fits it.
        List<Length> baselines = Baselines(body, 3500);

        baselines.Count.ShouldBe(2);
        (baselines[1] - baselines[0]).Mm100.ShouldBe(TightenedLine + AppendedLine);
    }

    /// <summary>
    /// The line a trailing hard break appends is the same line, by the same route.
    /// </summary>
    /// <remarks>
    /// <c>CreateLines</c> sets <c>bLineBreak</c> on an <c>EE_FEATURE_LINEBR</c> portion
    /// (<c>impedit3.cxx</c>:1088-1094) and calls <c>CreateAndInsertEmptyLine</c> once the
    /// paragraph is broken (<c>:1843-1844</c>) — the same function, with the cursor sought to the
    /// paragraph's end rather than to its start. <see cref="PptTextReader.LineBreak"/> is the
    /// vertical tab the binary format spells such a break with.
    /// <para>
    /// The broken paragraph is the <em>second</em> deliberately: the "not the very first line"
    /// guard is on the paragraph index and would otherwise leave the appended line unproportioned
    /// as well as unscaled, which is the next test rather than this one.
    /// </para>
    /// </remarks>
    [Fact]
    public void ALineAppendedByATrailingBreakIsMeasuredTheSameWay()
    {
        string broken = "Beta" + PptTextReader.LineBreak;

        SlideTextBody body = Body(
            autoFit: true, Paragraph("Alpha", 0.9), Paragraph(broken, 0.9),
            Paragraph("Gamma", 0.9));

        // 4 x 1220 unscaled overflows a 4600-unit box; 1098 + 1098 + 1220 + 1098 fits it.
        List<Length> baselines = Baselines(body, 4600);

        baselines.Count.ShouldBe(3);
        (baselines[2] - baselines[1]).Mm100.ShouldBe(TightenedLine + AppendedLine);
    }

    /// <summary>
    /// The <c>Prop</c> arm is skipped for the body's <em>first</em> paragraph, so an empty line
    /// that opens a body is a full 1.2 em tall.
    /// </summary>
    /// <remarks>
    /// <c>if (nPara || pTmpLine-&gt;GetStartPortion())</c>, under the comment "Not the very first
    /// line" (<c>impedit3.cxx</c>:1953-1955). A freshly constructed <c>EditLine</c> has a start
    /// portion of zero, so the test reduces to the paragraph index. Measured here without a fit,
    /// so nothing but that guard can move the answer.
    /// </remarks>
    [Fact]
    public void TheFirstParagraphsAppendedLineIsNotProportionedAtAll()
    {
        SlideTextBody opening = Body(
            autoFit: true, Paragraph(string.Empty, 0.9), Paragraph("Alpha", 0.9),
            Paragraph(string.Empty, 0.9), Paragraph("Beta", 0.9));

        // A box nothing overflows, so the fit answers no scaling and only the guard can move it.
        List<Length> baselines = Baselines(opening, 20000);

        baselines.Count.ShouldBe(2);

        // Alpha to Beta crosses one text line and one *interior* appended line, both at 90 per
        // cent; the leading one is not proportioned at all, and shows in the first baseline.
        (baselines[1] - baselines[0]).Mm100.ShouldBe(2 * AppendedLine);
        baselines[0].Mm100.ShouldBe(UnscaledLine + 976);
    }

    /// <summary>
    /// The whole of the witness: a body of alternating text and empty paragraphs lands one
    /// <c>constScaleLevels</c> row lower than it does when the appended lines are tightened too.
    /// </summary>
    /// <remarks>
    /// Five paragraphs of 32 pt at 90 per cent — text, empty, text, empty, text — in a box of
    /// 5600 units. Unscaled they are 6100; the first row gives <c>3 × 1098 + 2 × 1220 = 5734</c>
    /// and overflows, and the second gives <c>3 × 1029 + 2 × 1143 = 5373</c> and fits, so the
    /// drawn em is <c>round(32 × 0.925) = 30</c> pt. Tightening the appended lines gives
    /// <c>5 × 1098 = 5490</c>, which fits, and draws 32.
    /// </remarks>
    [Fact]
    public void AlternatingEmptyParagraphsPushTheFitOntoTheNextRow()
    {
        SlideTextBody body = Body(
            autoFit: true,
            Paragraph("Alpha", 0.9), Paragraph(string.Empty, 0.9),
            Paragraph("Beta", 0.9), Paragraph(string.Empty, 0.9),
            Paragraph("Gamma", 0.9));

        Placed(body, 5600)[0].Run.FontSize.Mm100.ShouldBe(ShrunkEm);
    }

    /// <summary>
    /// A paragraph's own space is a whole hundredth of a millimetre and the fit's scale truncates
    /// it, rather than rounding.
    /// </summary>
    /// <remarks>
    /// <c>sal_uInt16 nUpper = scaleYSpacingValue(rULItem.GetUpper())</c>
    /// (<c>editeng/source/editeng/impedit2.cxx</c>:4792-4797) drops the fraction of a
    /// <c>double</c>. The witness states 8 pt — 282 units — and the reference's own bullet pitch
    /// puts the scaled value at <strong>253</strong>, where <c>fround(253.8)</c> would be 254.
    /// </remarks>
    [Fact]
    public void TheFitsSpacingScaleTruncatesAParagraphsSpace()
    {
        SlideTextBody body = Body(
            autoFit: true,
            Paragraph("Alpha", 0.9), Paragraph("Beta", 0.9, spaceBeforePoints: 8));

        // 2 x 1220 + 282 = 2722 unscaled overflows a 2600-unit box; 2 x 1098 + 253 fits it.
        List<Length> baselines = Baselines(body, 2600);

        baselines.Count.ShouldBe(2);
        (baselines[1] - baselines[0]).Mm100.ShouldBe(TightenedLine + 253);
    }
}
