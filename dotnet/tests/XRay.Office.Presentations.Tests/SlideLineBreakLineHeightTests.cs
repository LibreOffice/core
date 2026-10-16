using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Presentations.Layout;
using XRay.Office.Text.Layout;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// A hard line break does not size the line it ends, and a line that is nothing but one does.
/// </summary>
/// <remarks>
/// <para>
/// EditEngine's line-metrics loop walks every portion from <c>GetStartPortion()</c> to
/// <c>GetEndPortion()</c> and skips exactly one kind —
/// <c>if ( rTP.GetKind() != PortionKind::LINEBREAK )</c>, with the comment beside it naming this
/// case: <em>"problem with hard font height attribute, when everything but the line break has this
/// attribute"</em> (<c>editeng/source/editeng/impedit3.cxx</c>:1498-1516). The kind is set by
/// <c>EE_FEATURE_LINEBR</c> alone (<c>:1088-1099</c>) and the break is the line's last character
/// (<c>pLine-&gt;SetEnd(nPortionStart + 1)</c>, <c>:1441-1449</c>). A line holding nothing else
/// measures zero — <c>EditLine::CalcTextSize</c> adds nothing for that kind
/// (<c>editeng/source/editeng/EditLine.cxx</c>:69-71) — and takes the fallback above the loop,
/// which seeks the font at <c>GetStart()+1</c>, the break's own character
/// (<c>impedit3.cxx</c>:1478-1491). Read in a checkout declaring <c>27.2.0.0.alpha0+</c>, so the
/// measurement below is the leg that stands.
/// </para>
/// <para>
/// <strong>Measured against 26.2.4.2 rather than derived</strong>, on
/// <c>probes/slides-r100/make-break-probe.py</c>: five boxes per slide, 18 pt text and one
/// construct whose size is swept 18, 24, 28, 36, 54, 72 pt. The box holding
/// <c>"AAA"</c> + break + <c>"BBB"</c> draws its two baselines at <strong>462.019 and 440.419 on
/// all six slides</strong>; the boxes holding a trailing blank and a visible glyph at the swept
/// size both move on every one; and the box holding two breaks in a row gives its middle line
/// <c>fround(1.2 × em)</c> of the swept size exactly, 1524 hundredths of a millimetre at 18 pt
/// through 3810 at 72. The corpus witness is
/// <c>slides/done-003/ppt/Inducement-to-Insurance-Business.ppt</c> page 14, a bottom-anchored
/// title whose paragraph ends in a line break set in 54 pt after 28 pt text: 26.2.4.2 gives that
/// line the same 30.246 pt pitch as the rest of the block and we gave it 52.696.
/// </para>
/// <para>
/// The body below is that shape in miniature, and shares its arithmetic with
/// <see cref="SlideTrailingBlankLineHeightTests"/>: font-independent line spacing, so a line is
/// <c>fround(1.2 × em)</c> and its ascent is the em. 12 pt is 423 hundredths of a millimetre and
/// 36 pt is 1270, so a second line's baseline sits <c>fround(423 × 1.2) + 423 = 931</c> units —
/// <strong>26.39 pt</strong> — below the box top when the 36 pt break is skipped, and
/// <c>fround(1270 × 1.2) + 423 = 1947</c> units, 55.19 pt, when it is measured.
/// </para>
/// </remarks>
public class SlideLineBreakLineHeightTests
{
    private const string Face = "Liberation Sans";

    /// <summary>U+000B, which is what a binary PowerPoint's line break arrives as.</summary>
    private const char PptBreak = '\u000B';

    /// <summary>U+2028, which is what <c>a:br</c> and <c>text:line-break</c> arrive as.</summary>
    private const char OoxmlBreak = '\u2028';

    private static List<double> Baselines(string text, IEnumerable<SlideTextRun> runs)
    {
        SlideTextBody body = new()
        {
            Insets = new Margins(Length.Zero, Length.Zero, Length.Zero, Length.Zero),
            Wraps = false,
            Anchor = TextAnchor.Top,
            FontIndependentLineSpacing = true,
            Paragraphs = [new SlideParagraph(text, [.. runs], TextAlignment.Start)],
        };

        List<PlacedGlyphRun> placed = SlideTextLayout.Place(
            body,
            new DocRect(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(400)),
            new SlideFonts());

        return [.. placed.Select(run => run.Run.Origin.Y.Points).Distinct().Order()];
    }

    /// <summary>A run of <paramref name="length"/> characters — the record's second field is a
    /// length and not an end, which is easy to get wrong here.</summary>
    private static SlideTextRun Run(int start, int length, double points)
        => new(start, length, Face, Length.FromPoints(points), 400, false, Colour.Black);

    [Theory]
    [InlineData('\u000B')]
    [InlineData('\u2028')]
    public void AHardBreakDoesNotSizeTheLineItEnds(char separator)
    {
        List<double> ys = Baselines(
            $"Hxy{separator}Abc",
            [Run(0, 3, 12), Run(3, 1, 36), Run(4, 3, 12)]);

        ys.Count.ShouldBe(2);
        ys[0].ShouldBe(11.99, 0.05);
        ys[1].ShouldBe(26.39, 0.05);
    }

    /// <summary>
    /// The control: the same paragraph with the break at the text's own size, where the rule can
    /// make no difference.
    /// </summary>
    [Fact]
    public void ABreakAtTheTextsOwnSizeChangesNothing()
    {
        List<double> ys = Baselines(
            $"Hxy{PptBreak}Abc",
            [Run(0, 7, 12)]);

        ys.Count.ShouldBe(2);
        ys[1].ShouldBe(26.39, 0.05);
    }

    /// <summary>
    /// A line whose only portion is the break keeps it: <c>CalcTextSize</c> gives that line zero
    /// and the fallback sizes it from the break's own character.
    /// </summary>
    /// <remarks>
    /// The middle line draws no glyph, so it contributes no baseline; what it contributes is its
    /// height, and the third line's baseline is where that shows. 26.2.4.2 gives that line
    /// <c>fround(1.2 x em)</c> of the break's own size — measured over six sizes on the probe's
    /// <c>TWICE</c> box, first-to-third baseline distance
    /// <c>127 + fround(1.2 x em) + 635</c> hundredths of a millimetre at all six.
    /// </remarks>
    [Fact]
    public void ALineThatIsNothingButABreakIsSizedByIt()
    {
        List<double> ys = Baselines(
            $"Hxy{PptBreak}{PptBreak}Abc",
            [Run(0, 3, 12), Run(3, 2, 36), Run(5, 3, 12)]);

        ys.Count.ShouldBe(2);
        ys[0].ShouldBe(11.99, 0.05);

        // 508 of the first line, fround(1.2 x 1270) = 1524 of the break's own line, 423 of the
        // last line's ascent.
        ys[1].ShouldBe(69.59, 0.05);
    }

    /// <summary>
    /// The control for it: the same paragraph with both breaks at the text's own size, where the
    /// middle line is an ordinary 12 pt one.
    /// </summary>
    [Fact]
    public void ALineThatIsNothingButABreakAtTheTextsOwnSizeIsAnOrdinaryLine()
    {
        List<double> ys = Baselines($"Hxy{PptBreak}{PptBreak}Abc", [Run(0, 8, 12)]);

        ys.Count.ShouldBe(2);

        // 508 + 508 + 423.
        ys[1].ShouldBe(40.79, 0.05);
    }
}
