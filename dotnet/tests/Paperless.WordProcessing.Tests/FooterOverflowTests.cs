using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.WordProcessing.Layout;
using Paperless.WordProcessing.Model;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// What happens to the body when a footer needs more room than the bottom margin reserved for it.
/// </summary>
/// <remarks>
/// <para>
/// The mirror of <see cref="HeaderOverflowTests"/>, and the same mechanism read at the other end of the
/// page. <c>SectionPropertyMap::PrepareHeaderFooterProperties</c> makes the bottom margin
/// <c>w:footer</c> and the footer frame's height <c>w:bottom − w:footer</c>, with dynamic spacing;
/// <c>SwHeadFootFrame::FormatPrt</c> then lets the footer eat the gap above it and keep growing, and it
/// grows <em>upwards</em> because a footer frame's lower edge is pinned at the footer distance. The body
/// ends at <c>min(pageHeight − w:bottom, pageHeight − w:footer − footer height)</c>.
/// </para>
/// <para>
/// Measured on <c>words/batch-018/doc/120509coss.doc</c>, whose footer is one 60.7 pt logo in the 21.3 pt
/// its margins reserve: LibreOffice ends the body at 695.8 pt and Paperless ended it at 735.3 pt, four
/// lines lower, on every page of the document.
/// </para>
/// </remarks>
public sealed class FooterOverflowTests
{
    /// <summary>A footer that fits inside the bottom margin leaves the body exactly where it was.</summary>
    [Fact]
    public void AFooterThatFitsMovesNothing()
    {
        LaidOutPage page = Paginate(footerLines: 1)[0];

        page.BodyArea.Height.ShouldBe(Geometry.TextHeight);
        page.BodyArea.Bottom.ShouldBe(Geometry.Size.Height - Geometry.Margins.Bottom);
    }

    /// <summary>A footer taller than its reserved room raises the body's bottom by the overflow.</summary>
    [Fact]
    public void AFooterThatOverflowsRaisesTheBodysBottom()
    {
        List<LaidOutPage> pages = Paginate(footerLines: 12);
        DocRect body = pages[0].BodyArea;

        pages[0].Footer.ShouldNotBeNull();
        Length needed = Geometry.FooterDistance + pages[0].Footer!.Advance;

        needed.ShouldBeGreaterThan(Geometry.Margins.Bottom);

        // The foot's top and the body's bottom meet rather than overlap, and the page loses exactly what
        // the foot gained. The body's *top* is untouched — the two ends are independent.
        body.Top.ShouldBe(Geometry.Margins.Top);
        body.Bottom.ShouldBe(Geometry.Size.Height - needed);
    }

    /// <summary>
    /// A footer whose height the document fixed does not move the body, however much it holds.
    /// </summary>
    /// <remarks>
    /// Writer's <c>SwFrameSize::Fixed</c>, which only ODF can ask for — <c>svg:height</c> against
    /// <c>fo:min-height</c>. It is not a corner case: a fixed-height footer holding one ordinary line
    /// already overruns the room its own style reserves, so growing it would shorten every page of every
    /// such document. Three fidelity fixtures do exactly that.
    /// </remarks>
    [Fact]
    public void AFixedHeightFooterMovesNothing()
    {
        PageGeometry fixedHeight = Geometry with { HasFixedFooterHeight = true };
        LaidOutPage page = Paginate(footerLines: 12, geometry: fixedHeight)[0];

        page.BodyArea.Height.ShouldBe(fixedHeight.TextHeight);
    }

    /// <summary>And a fixed-height header does not push the body down either.</summary>
    [Fact]
    public void AFixedHeightHeaderMovesNothing()
    {
        PageGeometry fixedHeight = Geometry with { HasFixedHeaderHeight = true };

        PageFurnitureSet furniture = new(
            new Dictionary<PageFurnitureSlot, IReadOnlyList<PageBlock>>
            {
                [PageFurnitureSlot.Default] = Lines(12, "running head"),
            });

        List<LaidOutPage> pages = new Paginator(PaginationOptions.Word).Paginate(
            [Paragraph("body")], new WritingSection { Page = fixedHeight }, furniture: furniture);

        pages[0].BodyArea.Top.ShouldBe(fixedHeight.Margins.Top);
    }

    private static List<LaidOutPage> Paginate(int footerLines, PageGeometry? geometry = null)
    {
        PageGeometry page = geometry ?? Geometry;

        PageFurnitureSet furniture = new(
            footers: new Dictionary<PageFurnitureSlot, IReadOnlyList<PageBlock>>
            {
                [PageFurnitureSlot.Default] = Lines(footerLines, "running foot"),
            });

        return new Paginator(PaginationOptions.Word).Paginate(
            [Paragraph("body")], new WritingSection { Page = page }, furniture: furniture);
    }

    private static IReadOnlyList<PageBlock> Lines(int count, string what)
        => [.. Enumerable.Range(0, count).Select(i => Paragraph($"{what} line {i}"))];

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
