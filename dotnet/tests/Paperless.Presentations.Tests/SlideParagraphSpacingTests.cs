using System.Xml.Linq;
using Paperless.Core.Units;
using Paperless.Presentations.Layout;
using Paperless.Presentations.Ooxml;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// Paragraph spacing stated as a percentage, and where it is and is not applied.
/// </summary>
/// <remarks>
/// <para>
/// <strong>The percentage form is the only one real files use.</strong> Of the 324 <c>a:pPr</c>
/// in the baked diagram drawings of LibreOffice's <c>sd/qa</c> corpus, all 324 state their
/// spacing as a percentage and none in points — so a reader that honoured only <c>a:spcPts</c>
/// set every multi-paragraph node tighter than the reference. On <c>tdf93830.pptx</c> that was
/// 14.4 pt on the worst line; with the percentage resolved every text baseline agrees with
/// LibreOffice's to 0.03 pt.
/// </para>
/// <para>
/// It is a percentage of the paragraph's own <em>character height</em>, not of the line height —
/// which is what the name suggests and what reading it as a line-spacing rule would give.
/// </para>
/// </remarks>
public class SlideParagraphSpacingTests
{
    private const string Drawing = "http://schemas.openxmlformats.org/drawingml/2006/main";

    /// <summary>
    /// A percentage spacing is resolved against the tallest run in its own paragraph.
    /// </summary>
    /// <remarks>
    /// 35% of 32 pt is 11.2 pt, and LibreOffice arrives at it the same way — the product is
    /// truncated to hundredths of a point before it leaves points
    /// (<c>TextSpacing::toMargin</c>, <c>oox/inc/drawingml/textspacing.hxx:54</c>). The tallest
    /// run rather than the first is what <c>textparagraph.cxx:131</c> takes, which is why the
    /// paragraph below states two sizes.
    /// </remarks>
    [Fact]
    public void APercentageIsAFractionOfTheTallestRun()
    {
        SlideTextBody body = PptxTextBody.Read(Body(
            Paragraph(spaceBeforePercent: 20000, spaceAfterPercent: 35000, sizes: [2400, 3200])));

        body.Paragraphs[0].SpaceBefore.Points.ShouldBe(6.4, 0.001);
        body.Paragraphs[0].SpaceAfter.Points.ShouldBe(11.2, 0.001);
    }

    /// <summary>A spacing stated in points still wins, and is read as hundredths of one.</summary>
    [Fact]
    public void PointsAreStillReadAsPoints()
    {
        SlideTextBody body = PptxTextBody.Read(Body(
            Paragraph(spaceBeforePoints: 1250, sizes: [3200])));

        body.Paragraphs[0].SpaceBefore.Points.ShouldBe(12.5, 0.001);
    }

    /// <summary>
    /// The outer two spacings do not count: paragraph spacing is a gap, not padding.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>ImpEditEngine::CalcHeight</c> guards the upper spacing with <c>if (nPortion)</c> and the
    /// lower with <c>if (nPortion != lastIndex())</c> under the comment "not in the last"
    /// (<c>editeng/source/editeng/impedit2.cxx:4792-4802</c>). So three paragraphs each stating
    /// 35% after and 20% before are separated by two gaps and padded by none, and the body grows
    /// by exactly those two gaps rather than by six spacings.
    /// </para>
    /// <para>
    /// The difference is not academic. On <c>tdf125551.pptx</c>, whose diagram labels are each a
    /// single 32 pt paragraph stating <c>spcAft</c> of 35%, counting the trailing spacing made
    /// every middle-anchored label 11.2 pt taller than its text and drew it 5.6 pt — half of that
    /// — above where LibreOffice draws it.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheFirstSpaceBeforeAndTheLastSpaceAfterAreNotCounted()
    {
        SlideFonts fonts = new();
        Length width = Length.FromPoints(400);

        Length bare = SlideTextLayout.Height(
            PptxTextBody.Read(Body(Paragraph(sizes: [3200]), Paragraph(sizes: [3200]),
                                   Paragraph(sizes: [3200]))),
            width,
            fonts);

        Length spaced = SlideTextLayout.Height(
            PptxTextBody.Read(Body(
                Paragraph(spaceBeforePercent: 20000, spaceAfterPercent: 35000, sizes: [3200]),
                Paragraph(spaceBeforePercent: 20000, spaceAfterPercent: 35000, sizes: [3200]),
                Paragraph(spaceBeforePercent: 20000, spaceAfterPercent: 35000, sizes: [3200]))),
            width,
            fonts);

        // Two gaps of 11.2 after plus 6.4 before, and nothing outside the first and last.
        (spaced - bare).Points.ShouldBe(2 * (11.2 + 6.4), 0.001);
    }

    /// <summary>
    /// A single-paragraph body gets no spacing at all, however much it states.
    /// </summary>
    /// <remarks>
    /// The degenerate case of the rule above, and the one a diagram hits constantly: almost every
    /// SmartArt node is one paragraph in one box, and each of those boxes would otherwise be
    /// padded top and bottom by a spacing that is meant to separate it from a neighbour it does
    /// not have.
    /// </remarks>
    [Fact]
    public void OneParagraphIsNeverPadded()
    {
        SlideFonts fonts = new();
        Length width = Length.FromPoints(400);

        Length bare = SlideTextLayout.Height(
            PptxTextBody.Read(Body(Paragraph(sizes: [3200]))), width, fonts);

        Length spaced = SlideTextLayout.Height(
            PptxTextBody.Read(Body(
                Paragraph(spaceBeforePercent: 20000, spaceAfterPercent: 35000, sizes: [3200]))),
            width,
            fonts);

        spaced.ShouldBe(bare);
    }

    // ----------------------------------------------------------------- set-up

    private static XElement Body(params XElement[] paragraphs)
        => new(XName.Get("txBody", Drawing), new XElement(XName.Get("bodyPr", Drawing)), paragraphs);

    private static XElement Paragraph(
        int spaceBeforePercent = 0,
        int spaceAfterPercent = 0,
        int spaceBeforePoints = 0,
        int[]? sizes = null,
        string text = "Node")
    {
        XElement properties = new(XName.Get("pPr", Drawing));

        if (spaceBeforePercent != 0) properties.Add(Spacing("spcBef", "spcPct", spaceBeforePercent));
        if (spaceBeforePoints != 0) properties.Add(Spacing("spcBef", "spcPts", spaceBeforePoints));
        if (spaceAfterPercent != 0) properties.Add(Spacing("spcAft", "spcPct", spaceAfterPercent));

        XElement paragraph = new(XName.Get("p", Drawing), properties);

        foreach (int size in sizes ?? [1800])
        {
            paragraph.Add(new XElement(
                XName.Get("r", Drawing),
                new XElement(
                    XName.Get("rPr", Drawing),
                    new XAttribute("sz", size.ToString(System.Globalization.CultureInfo.InvariantCulture)),
                    new XElement(
                        XName.Get("latin", Drawing),
                        new XAttribute("typeface", "Liberation Sans"))),
                new XElement(XName.Get("t", Drawing), text)));
        }

        return paragraph;
    }

    private static XElement Spacing(string outer, string inner, int value)
        => new(
            XName.Get(outer, Drawing),
            new XElement(
                XName.Get(inner, Drawing),
                new XAttribute("val", value.ToString(System.Globalization.CultureInfo.InvariantCulture))));
}
