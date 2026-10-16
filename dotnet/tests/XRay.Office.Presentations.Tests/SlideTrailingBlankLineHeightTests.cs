using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Presentations.Layout;
using XRay.Office.Text.Layout;
using Shouldly;

namespace XRay.Office.Presentations.Tests;

/// <summary>
/// A line's height counts the blank that ends it, at whatever size that blank is set in.
/// </summary>
/// <remarks>
/// <para>
/// EditEngine sizes a line by walking <em>every</em> portion on it —
/// <c>GetStartPortion()</c> to <c>GetEndPortion()</c> inclusive, skipping only a
/// <c>PortionKind::LINEBREAK</c> — and taking the largest ascent and the largest descent it finds
/// (<c>editeng/source/editeng/impedit3.cxx:1496-1519</c>). A portion that happens to hold nothing
/// but a space is not exempt. Trailing blanks are left out of a line's <em>width</em>, which is
/// what <see cref="XRay.Office.Text.Layout.TextLine.VisibleEnd"/> is for and is still what the line
/// is drawn and aligned by; nothing leaves them out of its <em>height</em>.
/// </para>
/// <para>
/// <strong>Measured against 26.2.4.2 rather than derived.</strong> The witness is
/// <c>slides/done-005/ppt/pres_ioc_phuket.ppt</c> page 26's white box, whose wrapped paragraph
/// ends in a single space set in 28 pt Arial Black after 19 pt italic text. The reference draws
/// that space — a one-glyph 28.01 pt show at the end of the last line — and sizes the line by it:
/// its second-to-third baseline pitch is <strong>31.81 pt</strong>, which is
/// <c>(fround(670 × 1.2) − 670) + 988</c> hundredths of a millimetre, the 19 pt line's descent plus
/// the <em>28 pt</em> line's ascent. The box states <c>draw:auto-grow-height="true"</c> with
/// <c>fo:min-height="0cm"</c>, so the block's height is the shape's: 26.2.4.2's own flat-ODP export
/// gives it <c>svg:height="3.267cm"</c>, which is its three lines
/// <c>1016 + 804 + 1186</c> plus 0.13 cm of padding at each end. Measuring the last line at 19 pt
/// instead gives 2.884 cm, and that is the 81.77 pt we drew.
/// </para>
/// <para>
/// The body below is the same shape in miniature: font-independent line spacing, so a line is
/// <c>fround(1.2 × em)</c> and its ascent is the em, and a first paragraph whose visible text is
/// 12 pt and whose trailing blank is 36 pt. 36 pt is 1270 hundredths of a millimetre and 12 pt is
/// 423, so the second paragraph's baseline sits <c>fround(1270 × 1.2) + 423 = 1947</c> units —
/// <strong>55.19 pt</strong> — below the box top when the blank is measured, and
/// <c>fround(423 × 1.2) + 423 = 931</c> units, 26.39 pt, when it is not.
/// </para>
/// </remarks>
public class SlideTrailingBlankLineHeightTests
{
    private const string Face = "Liberation Sans";

    private static List<double> Baselines(string first, IEnumerable<SlideTextRun> runs)
    {
        SlideTextBody body = new()
        {
            Insets = new Margins(Length.Zero, Length.Zero, Length.Zero, Length.Zero),
            Wraps = false,
            Anchor = TextAnchor.Top,
            FontIndependentLineSpacing = true,
            Paragraphs =
            [
                new SlideParagraph(first, [.. runs], TextAlignment.Start),
                new SlideParagraph(
                    "Hxy",
                    [new SlideTextRun(0, 3, Face, Length.FromPoints(12), 400, false, Colour.Black)],
                    TextAlignment.Start),
            ],
        };

        List<PlacedGlyphRun> placed = SlideTextLayout.Place(
            body,
            new DocRect(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(400)),
            new SlideFonts());

        return [.. placed.Select(run => run.Run.Origin.Y.Points).Distinct().Order()];
    }

    [Fact]
    public void ATrailingBlankIsMeasuredAtItsOwnSize()
    {
        List<double> ys = Baselines(
            "Hxy ",
            [
                new SlideTextRun(0, 3, Face, Length.FromPoints(12), 400, false, Colour.Black),
                new SlideTextRun(3, 4, Face, Length.FromPoints(36), 400, false, Colour.Black),
            ]);

        ys.Count.ShouldBe(2);
        ys[1].ShouldBe(55.19, 0.05);
    }

    /// <summary>
    /// The control: the same two paragraphs with the trailing blank at the text's own size, where
    /// the rule can make no difference.
    /// </summary>
    [Fact]
    public void ATrailingBlankAtTheTextsOwnSizeChangesNothing()
    {
        List<double> ys = Baselines(
            "Hxy ",
            [new SlideTextRun(0, 4, Face, Length.FromPoints(12), 400, false, Colour.Black)]);

        ys.Count.ShouldBe(2);
        ys[1].ShouldBe(26.39, 0.05);
    }
}
