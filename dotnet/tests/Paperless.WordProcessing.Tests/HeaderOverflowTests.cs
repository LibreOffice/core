using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.WordProcessing.Layout;
using Paperless.WordProcessing.Model;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// What happens to the body when a running head needs more room than the top margin reserved for it.
/// </summary>
/// <remarks>
/// <para>
/// A Word header is not clipped to the room between <c>w:header</c> and <c>w:top</c>. LibreOffice's DOCX
/// importer gives the header frame a dynamic height and dynamic spacing
/// (<c>SectionPropertyMap::PrepareHeaderFooterProperties</c>), which
/// <c>SwHeadFootFrame::FormatPrt</c> reads as: growth eats the gap between header and body first, and
/// once that gap is gone the body moves down. So a header that still fits inside the top margin costs
/// the body nothing, and one that does not takes exactly its overflow.
/// </para>
/// <para>
/// Measured on <c>words/batch-017/docx/AWR OPS-AOC 044 …docx</c>, whose three-row header table needs
/// 77 pt of a 67 pt top margin: LibreOffice starts page two's body at 98.2 pt and Paperless started it
/// at 70.5 pt, overlapping the header by most of its height and fitting two extra rows on every page.
/// </para>
/// </remarks>
public sealed class HeaderOverflowTests
{
    /// <summary>A header that fits inside the top margin leaves the body exactly where it was.</summary>
    [Fact]
    public void AHeaderThatFitsMovesNothing()
    {
        LaidOutPage page = Paginate(headerLines: 1)[0];

        page.BodyArea.Top.ShouldBe(Geometry.Margins.Top);
        page.BodyArea.Height.ShouldBe(Geometry.TextHeight);
    }

    /// <summary>A header taller than its reserved room pushes the body down by the overflow.</summary>
    [Fact]
    public void AHeaderThatOverflowsPushesTheBodyDown()
    {
        List<LaidOutPage> pages = Paginate(headerLines: 12);
        DocRect body = pages[0].BodyArea;

        // The head starts at HeaderDistance and the body starts where the head ends, so the two meet
        // rather than overlap — and the page loses exactly what the head gained.
        pages[0].Header.ShouldNotBeNull();
        Length headEnds = Geometry.HeaderDistance + pages[0].Header!.Advance;

        headEnds.ShouldBeGreaterThan(Geometry.Margins.Top);
        body.Top.ShouldBe(headEnds);
        body.Height.ShouldBe(Geometry.TextHeight - (headEnds - Geometry.Margins.Top));
    }

    /// <summary>
    /// The room a tall first-page head costs is not charged to the pages that do not draw it.
    /// </summary>
    /// <remarks>
    /// The reason the measurement is per page rather than per section. A section settling on the tallest
    /// of its header slots shortens every page for the sake of one, which is how an earlier attempt at
    /// this over-grew two documents while fixing none.
    /// </remarks>
    [Fact]
    public void OnlyThePageDrawingTheTallHeadPaysForIt()
    {
        List<PageBlock> body = [.. Enumerable.Range(0, 120).Select(i => Paragraph($"body line {i}"))];

        PageFurnitureSet furniture = new(
            new Dictionary<PageFurnitureSlot, IReadOnlyList<PageBlock>>
            {
                [PageFurnitureSlot.First] = Head(12),
                [PageFurnitureSlot.Default] = Head(1),
            });

        List<LaidOutPage> pages = new Paginator(PaginationOptions.Word).Paginate(
            body,
            new WritingSection { Page = Geometry, HasDifferentFirstPage = true },
            furniture: furniture);

        pages.Count.ShouldBeGreaterThan(1);
        pages[0].BodyArea.Top.ShouldBeGreaterThan(Geometry.Margins.Top);
        pages[1].BodyArea.Top.ShouldBe(Geometry.Margins.Top);
    }

    private static List<LaidOutPage> Paginate(int headerLines)
    {
        PageFurnitureSet furniture = new(
            new Dictionary<PageFurnitureSlot, IReadOnlyList<PageBlock>>
            {
                [PageFurnitureSlot.Default] = Head(headerLines),
            });

        return new Paginator(PaginationOptions.Word).Paginate(
            [Paragraph("body")],
            new WritingSection { Page = Geometry },
            furniture: furniture);
    }

    private static IReadOnlyList<PageBlock> Head(int lines)
        => [.. Enumerable.Range(0, lines).Select(i => Paragraph($"running head line {i}"))];

    /// <summary>An A4 page with a 1 inch top margin and the header half an inch into it.</summary>
    private static PageGeometry Geometry { get; } = new()
    {
        Size = new DocSize(Length.FromTwips(11906), Length.FromTwips(16838)),
        Margins = PageMargins.Uniform(Length.FromTwips(1440)),
        HeaderDistance = Length.FromTwips(720),
        FooterDistance = Length.FromTwips(720),
    };

    private static PageParagraph Paragraph(string text) => new()
    {
        Text = text,
        Face = Face,
        EmSize = Length.FromPoints(11),
    };

    private static OpenTypeFace Face { get; } = Resolve();

    private static OpenTypeFace Resolve()
    {
        SystemFontResolver resolver = new(SystemFontIndex.Build());
        return resolver.LoadOpenType(
            resolver.Resolve(new FontRequest("Liberation Serif", 400, false)));
    }
}
