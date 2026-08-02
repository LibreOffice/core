using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.WordProcessing.Layout;
using Paperless.WordProcessing.Model;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Where a header and a footer go when the page reserves no room for them.
/// </summary>
/// <remarks>
/// <para>
/// A Word document whose <c>w:top</c> equals its <c>w:header</c> — or whose <c>w:bottom</c> equals its
/// <c>w:footer</c> — reserves nothing, and that is ordinary rather than malformed: it says the furniture
/// may grow into the body's room. Writer renders one, because its header and footer are dynamic-height
/// frames with a 1 mm floor
/// (<c>SectionPropertyMap::PrepareHeaderFooterProperties</c>, <c>PropertyMap.cxx:1148</c>).
/// </para>
/// <para>
/// Paperless used to return an empty rectangle for that case, and a flow laid out into an empty
/// rectangle is no flow at all — so the header's lines, the footer's lines, and every frame anchored in
/// either vanished. On the corpus that lost a running head from all six pages of a document whose
/// pagination was otherwise exact.
/// </para>
/// </remarks>
public sealed class FurnitureAreaTests
{
    /// <summary>An A4 page whose margins leave the furniture no room of its own.</summary>
    private static PageGeometry NoRoom => new()
    {
        Size = new DocSize(Length.FromTwips(11906), Length.FromTwips(16838)),
        Margins = new PageMargins(
            Length.FromTwips(720), Length.FromTwips(720),
            Length.FromTwips(720), Length.FromTwips(720)),
        HeaderDistance = Length.FromTwips(720),
        FooterDistance = Length.FromTwips(720),
    };

    /// <summary>
    /// The header still has a rectangle, at the distance the document states.
    /// </summary>
    [Fact]
    public void AHeaderWithNoReservedHeightKeepsItsPlace()
    {
        DocRect area = NoRoom.HeaderArea;

        area.Y.ShouldBe(Length.FromTwips(720));
        area.Height.ShouldBe(Length.Zero);
        area.Width.ShouldBe(NoRoom.TextWidth);
    }

    /// <summary>The footer likewise, starting where the body's text area ends.</summary>
    [Fact]
    public void AFooterWithNoReservedHeightKeepsItsPlace()
    {
        DocRect area = NoRoom.FooterArea;

        area.Bottom.ShouldBe(NoRoom.Size.Height - NoRoom.FooterDistance);
        area.Height.ShouldBe(Length.Zero);
        area.Width.ShouldBe(NoRoom.TextWidth);
    }

    /// <summary>
    /// An A4 landscape page whose footer sits <em>below</em> where the body's text area ends.
    /// </summary>
    /// <remarks>
    /// <c>easa-form-1.docx</c>'s numbers: <c>w:bottom</c> 357 and <c>w:footer</c> 488. Nothing forbids a
    /// document from stating a footer distance larger than its bottom margin, and Word draws the footer
    /// at the distance it states either way.
    /// </remarks>
    private static PageGeometry CrossedFooter => new()
    {
        Size = new DocSize(Length.FromTwips(16840), Length.FromTwips(11907)),
        Margins = new PageMargins(
            Length.FromTwips(1134), Length.FromTwips(1134),
            Length.FromTwips(851), Length.FromTwips(357)),
        HeaderDistance = Length.FromTwips(720),
        FooterDistance = Length.FromTwips(488),
    };

    /// <summary>
    /// A footer stated further from the edge than the bottom margin still lands at its stated distance.
    /// </summary>
    /// <remarks>
    /// The rectangle's <em>bottom</em> is what the distance fixes and its top is what gives way. Growing
    /// it downwards from the body's edge instead starts it past where the footer belongs, and a
    /// nought-height one then lands there outright: measured against the reference on
    /// <c>easa-form-1.docx</c>, LibreOffice ends the footer's text at 570.9 pt of a 595.35 pt page — the
    /// stated 488 twips from the edge — and we drew it at 577.3.
    /// </remarks>
    [Fact]
    public void AFooterStatedBelowTheBodyEdgeStillSitsAtItsStatedDistance()
    {
        DocRect area = CrossedFooter.FooterArea;

        area.Bottom.ShouldBe(CrossedFooter.Size.Height - CrossedFooter.FooterDistance);
        area.Height.ShouldBe(Length.Zero, "the margins reserve it nothing");
        area.Y.ShouldBeLessThan(
            CrossedFooter.Margins.Top + CrossedFooter.TextHeight,
            "the body's own edge is lower than the footer's, which is the whole of the case");
    }

    /// <summary>
    /// A bottom-aligned flow rests its last line on the area's bottom even when it does not fit.
    /// </summary>
    /// <remarks>
    /// Word's footer is bottom-aligned at <c>pageHeight − w:footer</c> and grows <em>upwards</em> when it
    /// outgrows its room, which is what a dynamic-height frame does. Clamping the shift at nought instead
    /// pushed the whole footer down by its own height — off the bottom of the page for the documents
    /// where the room is nought to begin with.
    /// </remarks>
    [Fact]
    public void ABottomAlignedFlowGrowsUpwardsWhenItDoesNotFit()
    {
        DocRect area = NoRoom.FooterArea;

        PlacedFlow? placed = FlowLayouter.LayOut(
            [Paragraph("a footer line"), Paragraph("and another")], area, offsetFromTop: null);

        placed.ShouldNotBeNull();
        placed!.Lines.Count.ShouldBe(2);

        Length bottom = placed.Lines[^1].Top + placed.Lines[^1].Box.Height;
        bottom.ShouldBe(area.Height);
        placed.Lines[0].Top.ShouldBeLessThan(Length.Zero);
    }

    private static PageParagraph Paragraph(string text) => new()
    {
        Text = text,
        Face = Face,
        EmSize = Length.FromPoints(11),
    };

    /// <summary>
    /// A real face, because the flow's height is a measurement rather than a constant.
    /// </summary>
    private static OpenTypeFace Face { get; } = Resolve();

    private static OpenTypeFace Resolve()
    {
        SystemFontResolver resolver = new(SystemFontIndex.Build());
        return resolver.LoadOpenType(
            resolver.Resolve(new FontRequest("Liberation Serif", 400, false)));
    }
}
