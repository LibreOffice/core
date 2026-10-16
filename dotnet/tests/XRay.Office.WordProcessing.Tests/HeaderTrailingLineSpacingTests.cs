using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Layout;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Model;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A running head grows by its <em>last</em> paragraph's proportional line spacing, which nothing
/// inside the flow ever collects.
/// </summary>
/// <remarks>
/// <para>
/// Inside a flow the gap is unambiguous and invisible: <see cref="ParagraphLeading"/> hands it to the
/// line below, Writer's newer builds keep it under the line above, and both put the same distance in
/// the same place. At the flow's <em>end</em> the two answers differ by the whole gap, and which is
/// right depends on the frame — a header frame grows by it, a body's last paragraph on a page does
/// not, because there the space belongs to the page break.
/// </para>
/// <para>
/// Measured against the installed 26.2.4.2 by <c>probes/words-w-pitch/mkhdr.py</c>, a two-paragraph
/// header whose second paragraph is empty, at <c>w:top</c> 720 and <c>w:header</c> 709, with the
/// body's first baseline reporting the header band:
/// </para>
/// <code>
///   paragraph mark   w:line        reference   this engine before
///   10 pt            240 (100%)       774.04   774.05
///   10 pt            480 (200%)       762.54   774.05
///   12 pt            240 (100%)       771.74   771.75
///   12 pt            360 (150%)       764.89   771.75
///   12 pt            480 (200%)       757.99   771.75
///   20 pt            480 (200%)       739.54   762.55
/// </code>
/// <para>
/// A control the same probe authors: a further paragraph <em>after</em> the empty one matched before
/// the fix and after it, and so did the same empty paragraph in the body rather than in a header.
/// It is 13.75 pt per page on <c>OM template for non-complex NCC operators</c>, whose running head
/// ends with an empty 12 pt paragraph at <c>w:line="480"</c>.
/// </para>
/// </remarks>
public sealed class HeaderTrailingLineSpacingTests
{
    /// <summary>The flow reports the gap its last paragraph would have handed on.</summary>
    [Fact]
    public void AFlowReportsItsLastParagraphsProportionalGap()
    {
        PlacedFlow single = Flow(LineSpacingRule.SingleSpaced).ShouldNotBeNull();
        PlacedFlow doubled = Flow(LineSpacingRule.Multiple(2.0)).ShouldNotBeNull();

        single.TrailingLineSpacing.ShouldBe(Length.Zero);

        // A whole line's worth at 200 %, which for six identical single-spaced lines is a sixth of
        // what they stack to. The five gaps *between* the paragraphs are already inside `Advance`;
        // this is the sixth, which has nothing below it to be charged to.
        doubled.TrailingLineSpacing.ShouldBe(single.Advance / 6);
        doubled.Advance.ShouldBe(single.Advance + (5 * single.Advance / 6));
    }

    /// <summary>And a header that overflows takes that gap with it into the body's top.</summary>
    [Fact]
    public void ADoubleSpacedHeadPushesTheBodyDownByTheTrailingGap()
    {
        LaidOutPage single = Paginate(LineSpacingRule.SingleSpaced)[0];
        LaidOutPage doubled = Paginate(LineSpacingRule.Multiple(2.0))[0];

        single.Header.ShouldNotBeNull();
        doubled.Header.ShouldNotBeNull();

        // Both heads overflow the top margin, so each body starts exactly where its head ends — and
        // the head ends a trailing gap below what it stacked.
        doubled.Header!.TrailingLineSpacing.ShouldBeGreaterThan(Length.Zero);
        single.Header!.TrailingLineSpacing.ShouldBe(Length.Zero);

        single.BodyArea.Top.ShouldBe(Geometry.HeaderDistance + single.Header!.Advance);
        doubled.BodyArea.Top.ShouldBe(
            Geometry.HeaderDistance + doubled.Header!.Advance + doubled.Header!.TrailingLineSpacing);
    }

    /// <summary>
    /// A head that still fits inside the top margin once the gap is counted moves nothing.
    /// </summary>
    /// <remarks>
    /// The gap is added to what the head needs, not to where the body goes: the growth eats the
    /// spacing between the head and the body first, exactly as an overflowing head does.
    /// </remarks>
    [Fact]
    public void AShortDoubleSpacedHeadStillMovesNothing()
    {
        LaidOutPage page = Paginate(LineSpacingRule.Multiple(2.0), headerLines: 1)[0];

        page.Header.ShouldNotBeNull();
        page.Header!.TrailingLineSpacing.ShouldBeGreaterThan(Length.Zero);
        page.BodyArea.Top.ShouldBe(Geometry.Margins.Top);
    }

    private static PlacedFlow? Flow(LineSpacingRule spacing)
        => FlowLayouter.LayOut(
            Head(6, spacing),
            new DocRect(Length.Zero, Length.Zero, Length.FromTwips(9000), Length.Zero),
            Length.Zero);

    private static List<LaidOutPage> Paginate(LineSpacingRule spacing, int headerLines = 12)
    {
        PageFurnitureSet furniture = new(
            new Dictionary<PageFurnitureSlot, IReadOnlyList<PageBlock>>
            {
                [PageFurnitureSlot.Default] = Head(headerLines, spacing),
            });

        return new Paginator(PaginationOptions.Word).Paginate(
            [Paragraph("body", LineSpacingRule.SingleSpaced)],
            new WritingSection { Page = Geometry },
            furniture: furniture);
    }

    private static IReadOnlyList<PageBlock> Head(int lines, LineSpacingRule spacing)
        => [.. Enumerable.Range(0, lines).Select(i => Paragraph($"running head line {i}", spacing))];

    /// <summary>An A4 page with a 1 inch top margin and the header half an inch into it.</summary>
    private static PageGeometry Geometry { get; } = new()
    {
        Size = new DocSize(Length.FromTwips(11906), Length.FromTwips(16838)),
        Margins = PageMargins.Uniform(Length.FromTwips(1440)),
        HeaderDistance = Length.FromTwips(720),
        FooterDistance = Length.FromTwips(720),
    };

    private static PageParagraph Paragraph(string text, LineSpacingRule spacing) => new()
    {
        Text = text,
        Face = Face,
        EmSize = Length.FromPoints(11),
        Format = new ParagraphFormat { LineSpacing = spacing },
    };

    private static OpenTypeFace Face { get; } = Resolve();

    private static OpenTypeFace Resolve()
    {
        SystemFontResolver resolver = new(SystemFontIndex.Build());
        return resolver.LoadOpenType(
            resolver.Resolve(new FontRequest("Liberation Serif", 400, false)));
    }
}
