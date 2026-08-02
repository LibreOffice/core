using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.WordProcessing.Layout;
using Paperless.WordProcessing.Model;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Which section's header a page draws when two sections share one sheet.
/// </summary>
/// <remarks>
/// <para>
/// A continuous section break changes the geometry without ending the page, so a sheet can hold the end
/// of one section and the start of the next. It still has one running head, and Word gives it to the
/// section the page <em>starts</em> in. LibreOffice agrees: on a corpus document whose first section is
/// a title page and whose second begins part way down it, page one is bare and the new head appears on
/// page two.
/// </para>
/// <para>
/// Taking the section current at the moment the page was emitted instead put the second section's head
/// on the title page — invisible while headers were being dropped altogether, and a fresh defect the
/// moment they were not.
/// </para>
/// </remarks>
public sealed class SectionFurnitureTests
{
    /// <summary>
    /// A page whose content starts in the first section keeps that section's header.
    /// </summary>
    [Fact]
    public void APageKeepsTheHeaderOfTheSectionItStartsIn()
    {
        List<LaidOutPage> pages = Paginate();

        pages.Count.ShouldBe(1);
        HeaderText(pages[0]).ShouldBe("first section head");
    }

    /// <summary>
    /// A page whose content starts <em>after</em> the break takes the new section's header.
    /// </summary>
    /// <remarks>
    /// The other half of the rule, and the reason it is stated as "where the page's first content came
    /// from" rather than "the section current when the page began": a break landing on an empty page
    /// hands that page over.
    /// </remarks>
    [Fact]
    public void APageWhoseContentAllFollowsTheBreakTakesTheNewHeader()
    {
        List<LaidOutPage> pages = Paginate(secondSectionBreak: SectionBreak.NextPage);

        pages.Count.ShouldBe(2);
        HeaderText(pages[0]).ShouldBe("first section head");
        HeaderText(pages[1]).ShouldBe("second section head");
    }

    private static List<LaidOutPage> Paginate(
        SectionBreak secondSectionBreak = SectionBreak.Continuous)
    {
        List<PageBlock> blocks =
        [
            Paragraph("body of the first section", section: 0),
            Paragraph("body of the second section", section: 1),
        ];

        List<PaginatedSection> sections =
        [
            new PaginatedSection(new WritingSection { Page = Geometry }, Furniture("first")),
            new PaginatedSection(
                new WritingSection { Page = Geometry, Break = secondSectionBreak },
                Furniture("second")),
        ];

        return new Paginator(PaginationOptions.Word).Paginate(blocks, sections);
    }

    private static PageFurnitureSet Furniture(string which) => new(
        new Dictionary<PageFurnitureSlot, IReadOnlyList<PageBlock>>
        {
            [PageFurnitureSlot.Default] = [Paragraph($"{which} section head", section: 0)],
        });

    private static string HeaderText(LaidOutPage page)
    {
        page.Header.ShouldNotBeNull();
        return ((PageParagraph)page.Header!.Blocks[0]).Text;
    }

    private static PageGeometry Geometry => new()
    {
        Size = new DocSize(Length.FromTwips(11906), Length.FromTwips(16838)),
        Margins = new PageMargins(
            Length.FromTwips(1440), Length.FromTwips(1440),
            Length.FromTwips(1440), Length.FromTwips(1440)),
        HeaderDistance = Length.FromTwips(720),
        FooterDistance = Length.FromTwips(720),
    };

    private static PageParagraph Paragraph(string text, int section) => new()
    {
        Text = text,
        Face = Face,
        EmSize = Length.FromPoints(11),
        SectionIndex = section,
    };

    private static OpenTypeFace Face { get; } = Resolve();

    private static OpenTypeFace Resolve()
    {
        SystemFontResolver resolver = new(SystemFontIndex.Build());
        return resolver.LoadOpenType(
            resolver.Resolve(new FontRequest("Liberation Serif", 400, false)));
    }
}
