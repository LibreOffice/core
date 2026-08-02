using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.WordProcessing.Layout;
using Paperless.WordProcessing.Model;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// What happens to the body when a running foot needs more room than the bottom margin reserved for it.
/// </summary>
/// <remarks>
/// <para>
/// The mirror of <see cref="HeaderOverflowTests"/>, and the same mechanism upside down.
/// <c>SectionPropertyMap::PrepareHeaderFooterProperties</c> treats the two ends symmetrically — the
/// bottom margin becomes <c>w:footer</c>, the footer frame gets <c>w:bottom − w:footer</c> with the same
/// 1 mm floor, and the same dynamic height and dynamic spacing — and
/// <c>SwHeadFootFrame::FormatPrt</c> is written once for both. So a foot that fits inside the bottom
/// margin costs the body nothing and one that does not takes exactly its overflow.
/// </para>
/// <para>
/// The case that makes this bind on real documents is not a huge footer: it is a document whose
/// <c>w:footer</c> exceeds its <c>w:bottom</c>, which eight of the corpus's word-processing files do.
/// The footer then starts <em>above</em> where the body was allowed to end, and every page of the
/// document ran several points long.
/// </para>
/// </remarks>
public sealed class FooterOverflowTests
{
    /// <summary>A foot that fits inside the bottom margin leaves the body exactly where it was.</summary>
    [Fact]
    public void AFooterThatFitsMovesNothing()
    {
        LaidOutPage page = Paginate(footerLines: 1)[0];

        page.BodyArea.Top.ShouldBe(Geometry.Margins.Top);
        page.BodyArea.Height.ShouldBe(Geometry.TextHeight);
    }

    /// <summary>A foot taller than its reserved room raises the body's bottom by the overflow.</summary>
    [Fact]
    public void AFooterThatOverflowsPullsTheBodyUp()
    {
        List<LaidOutPage> pages = Paginate(footerLines: 12);
        DocRect body = pages[0].BodyArea;

        pages[0].Footer.ShouldNotBeNull();
        Length footStarts = Geometry.FooterDistance + pages[0].Footer!.Advance;

        footStarts.ShouldBeGreaterThan(Geometry.Margins.Bottom);
        body.Top.ShouldBe(Geometry.Margins.Top);
        body.Height.ShouldBe(Geometry.TextHeight - (footStarts - Geometry.Margins.Bottom));
    }

    /// <summary>
    /// A footer distance beyond the bottom margin shortens the body even for a one-line foot.
    /// </summary>
    /// <remarks>
    /// The corpus shape: <c>w:bottom</c> 454 and <c>w:footer</c> 567 twips, which says the footer starts
    /// above where <c>w:bottom</c> would have let the body end. Writer gives the body the smaller of the
    /// two; taking <c>w:bottom</c> on its own gave every page of such a document 5.65 pt too much.
    /// </remarks>
    [Fact]
    public void AFooterBelowTheMarginStillShortensTheBody()
    {
        PageGeometry overlapping = Geometry with
        {
            Margins = Geometry.Margins with { Bottom = Length.FromTwips(454) },
            FooterDistance = Length.FromTwips(567),
        };

        List<LaidOutPage> pages = Paginate(footerLines: 1, geometry: overlapping);

        pages[0].BodyArea.Height.ShouldBeLessThan(overlapping.TextHeight);
        pages[0].BodyArea.Bottom.ShouldBe(
            overlapping.Size.Height - overlapping.FooterDistance - pages[0].Footer!.Advance);
    }

    /// <summary>
    /// The room a tall first-page foot costs is not charged to the pages that do not draw it.
    /// </summary>
    [Fact]
    public void OnlyThePageDrawingTheTallFootPaysForIt()
    {
        List<PageBlock> body = [.. Enumerable.Range(0, 120).Select(i => Paragraph($"body line {i}"))];

        PageFurnitureSet furniture = new(
            footers: new Dictionary<PageFurnitureSlot, IReadOnlyList<PageBlock>>
            {
                [PageFurnitureSlot.First] = Foot(12),
                [PageFurnitureSlot.Default] = Foot(1),
            });

        List<LaidOutPage> pages = new Paginator(PaginationOptions.Word).Paginate(
            body,
            new WritingSection { Page = Geometry, HasDifferentFirstPage = true },
            furniture: furniture);

        pages.Count.ShouldBeGreaterThan(1);
        pages[0].BodyArea.Height.ShouldBeLessThan(Geometry.TextHeight);
        pages[1].BodyArea.Height.ShouldBe(Geometry.TextHeight);
    }

    private static List<LaidOutPage> Paginate(int footerLines, PageGeometry? geometry = null)
    {
        PageFurnitureSet furniture = new(
            footers: new Dictionary<PageFurnitureSlot, IReadOnlyList<PageBlock>>
            {
                [PageFurnitureSlot.Default] = Foot(footerLines),
            });

        return new Paginator(PaginationOptions.Word).Paginate(
            [Paragraph("body")],
            new WritingSection { Page = geometry ?? Geometry },
            furniture: furniture);
    }

    private static IReadOnlyList<PageBlock> Foot(int lines)
        => [.. Enumerable.Range(0, lines).Select(i => Paragraph($"running foot line {i}"))];

    /// <summary>An A4 page with a 1 inch bottom margin and the footer half an inch into it.</summary>
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
