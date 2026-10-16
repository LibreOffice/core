using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Model;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// What a continuous section break does with the page properties its own section states.
/// </summary>
/// <remarks>
/// <para>
/// A continuous section starts no page, so Writer gives it no page descriptor — and a page descriptor
/// is where the paper size, the margins, the running head, the running foot, the numbering format and
/// the number offset all live. So all six are inert together, and the section wears the descriptor
/// still in force. Both readers build one only for a section that names furniture of its own, and then
/// hang it on the first hard page break <em>inside</em> the section, dropping it when there is none:
/// <c>SectionPropertyMap::CloseSectionGroup</c> (<c>dmapper/PropertyMap.cxx</c>:1746-1801) and
/// <c>wwSectionManager::InsertSegments</c> (<c>ww8par.cxx</c>:4515-4560).
/// </para>
/// <para>
/// These assert the mechanism rather than a page count, because the corpus symptom — a running head on
/// nine pages of <c>hdss-bulletin-issue-285-25-june-2025.docx</c> that 26.2.4.2 draws on none, and a
/// body 43.54 pt low from page 2 on — is one consequence of it among several, and the document passes
/// the gate with the defect present. <c>dotnet/probes/words-continuous-r72/</c> has the measurements.
/// </para>
/// </remarks>
public sealed class ContinuousSectionDescriptorTests
{
    /// <summary>
    /// A continuous section naming no furniture of its own never gets a page style, so its
    /// <c>w:pgMar</c> reaches nothing.
    /// </summary>
    /// <remarks>
    /// The guard is writerfilter's own: the search for somewhere to hang the descriptor sits behind
    /// <c>!m_bDefaultHeaderLinkToPrevious</c> and its five siblings, so a section that inherits every
    /// slot is never offered one. Measured on the corpus witness by putting its third section's
    /// <c>w:top</c> to 3000 twips — a section that contains a hard page break and names no furniture:
    /// 26.2.4.2's page 2 does not move by a point.
    /// </remarks>
    [Fact]
    public void AContinuousSectionThatNamesNoFurnitureKeepsThePreviousMargins()
    {
        List<LaidOutPage> pages = Paginate(statesOwnFurniture: false, hardBreakInside: false);

        BodyTops(pages).ShouldAllBe(top => top == FirstSectionBodyTop);
    }

    /// <summary>
    /// Nor does a hard page break inside it help, while it names no furniture of its own.
    /// </summary>
    /// <remarks>
    /// The two conditions are separate and both are necessary — the break is only searched for once a
    /// descriptor exists to hang on it. This is the pair of the test above and the reason the flag is
    /// on the section rather than derived from the blocks.
    /// </remarks>
    [Fact]
    public void AHardBreakDoesNotRescueASectionThatNamesNoFurniture()
    {
        List<LaidOutPage> pages = Paginate(statesOwnFurniture: false, hardBreakInside: true);

        BodyTops(pages).ShouldAllBe(top => top == FirstSectionBodyTop);
        pages.Count.ShouldBeGreaterThan(1);
    }

    /// <summary>
    /// A continuous section naming a header of its own, with no hard page break anywhere inside it,
    /// draws neither that header nor its own margins.
    /// </summary>
    /// <remarks>
    /// The descriptor is built and then reaches no paragraph. <c>InsertSegments</c> is explicit about
    /// putting the old one back — <c>if (bFailed) aIter-&gt;mpPage = pOrig</c> — and dmapper simply
    /// never sets <c>PageDescName</c>, leaving the style in the document and on no page. This is the
    /// corpus witness's own shape.
    /// </remarks>
    [Fact]
    public void AContinuousSectionWithNoHardBreakDrawsNeitherItsHeaderNorItsMargins()
    {
        List<LaidOutPage> pages = Paginate(statesOwnFurniture: true, hardBreakInside: false);

        BodyTops(pages).ShouldAllBe(top => top == FirstSectionBodyTop);
        pages.Select(HeaderText).ShouldAllBe(text => text == "first head");
    }

    /// <summary>
    /// With a hard page break inside it, the same section's header and margins both take effect.
    /// </summary>
    /// <remarks>
    /// Geometry and furniture travel together because they are one descriptor, which is the half a rule
    /// stated as "ignore a continuous section's furniture" would get wrong. Measured on the corpus
    /// witness by giving its second section's first paragraph a <c>w:pageBreakBefore</c>: 26.2.4.2 then
    /// draws that section's header from that page on <em>and</em> moves the body down to its
    /// 1418-twip top margin, page for page with ours.
    /// </remarks>
    [Fact]
    public void AHardBreakInsideItLetsBothTheHeaderAndTheMarginsThrough()
    {
        List<LaidOutPage> pages = Paginate(statesOwnFurniture: true, hardBreakInside: true);

        pages.Count.ShouldBeGreaterThan(1);
        pages[0].BodyArea.Y.ShouldBe(FirstSectionBodyTop);
        HeaderText(pages[0]).ShouldBe("first head");

        pages[^1].BodyArea.Y.ShouldBe(SecondSectionBodyTop);
        HeaderText(pages[^1]).ShouldBe("second head");
    }

    /// <summary>
    /// The number offset dies with the descriptor that carries it.
    /// </summary>
    /// <remarks>
    /// dmapper sets <c>PageNumberOffset</c> in the same two branches that set <c>PageDescName</c>
    /// (<c>PropertyMap.cxx</c>:1794-1801), so a restart stated on a continuous section that lands
    /// nowhere is not applied late — it is not applied at all.
    /// </remarks>
    [Fact]
    public void ARestartStatedOnASectionThatLandsNowhereIsDropped()
    {
        List<LaidOutPage> pages =
            Paginate(statesOwnFurniture: true, hardBreakInside: false, restartAt: 50);

        pages.Select(page => page.Number).ShouldBe(Enumerable.Range(1, pages.Count));
    }

    /// <summary>
    /// The columns are the exception, because they are what a Writer text section really carries.
    /// </summary>
    /// <remarks>
    /// The continuous branch of <c>CloseSectionGroup</c> calls <c>appendTextSectionAfter</c> and puts
    /// the column properties on <em>that</em> (<c>PropertyMap.cxx</c>:1726-1734), which is why a
    /// stretch of two-column text in the middle of a page works at all — it is the whole reason a
    /// continuous break exists. So inheriting the page descriptor must not take the columns with it.
    /// </remarks>
    [Fact]
    public void TheColumnsAreKeptWhenTheRestOfTheDescriptorIsNot()
    {
        IReadOnlyList<PaginatedSection> resolved = ContinuousPageDescriptors.Resolve(
            [
                new PaginatedSection(new WritingSection { Page = FirstGeometry }),
                new PaginatedSection(new WritingSection
                {
                    Page = SecondGeometry with { Columns = 2, ColumnGap = Length.FromTwips(340) },
                    Break = SectionBreak.Continuous,
                }),
            ],
            []);

        resolved[1].Section.Page.Columns.ShouldBe(2);
        resolved[1].Section.Page.ColumnGap.ShouldBe(Length.FromTwips(340));
        resolved[1].Section.Page.Margins.Top.ShouldBe(FirstGeometry.Margins.Top);
    }

    /// <summary>
    /// A <c>.doc</c>'s continuous section keeps its own side margins, and only those.
    /// </summary>
    /// <remarks>
    /// The one place the two Word readers diverge, and it is in the C++ rather than in a guess:
    /// <c>wwSectionManager::InsertSection</c> puts <c>section.left − page.left</c> on the text section
    /// as an <c>SvxLRSpaceItem</c> (<c>ww8par6.cxx</c>:758-770), so it is not on the descriptor and
    /// does not fall with it; writerfilter's <c>ApplySectionProperties</c> sets no indent at all
    /// (<c>dmapper/PropertyMap.cxx</c>:792-812). Getting this wrong cost
    /// <c>644730BRI0mna000BOX361539B00public0.doc</c> a page — 4 against 26.2.4.2's 5 — by pulling
    /// pages 2 to 5 from a 72 pt left margin to the cover section's 30 pt.
    /// </remarks>
    [Fact]
    public void AWw8SectionsSideMarginsSurviveTheDescriptorAndItsVerticalOnesDoNot()
    {
        PageGeometry stated = SecondGeometry with
        {
            Margins = SecondGeometry.Margins with { Left = Length.FromTwips(4320) },
        };

        IReadOnlyList<PaginatedSection> resolved = ContinuousPageDescriptors.Resolve(
            [
                new PaginatedSection(new WritingSection { Page = FirstGeometry }),
                new PaginatedSection(
                    new WritingSection { Page = stated, Break = SectionBreak.Continuous },
                    Furniture: null,
                    StatesOwnFurniture: false,
                    SideMarginsAreASectionIndent: true),
            ],
            []);

        resolved[1].Section.Page.Margins.Left.ShouldBe(Length.FromTwips(4320));
        resolved[1].Section.Page.Margins.Top.ShouldBe(FirstGeometry.Margins.Top);
    }

    /// <summary>A DOCX's do not, because they are on the page style and nowhere else.</summary>
    /// <remarks>
    /// Measured: 26.2.4.2 leaves page two of the synthetic
    /// <see cref="ContinuousSectionGeometryTests"/> builds at the first section's 36.1 pt although
    /// the continuous section states a one-inch left margin.
    /// </remarks>
    [Fact]
    public void ADocxSectionsSideMarginsFallWithTheDescriptor()
    {
        PageGeometry stated = SecondGeometry with
        {
            Margins = SecondGeometry.Margins with { Left = Length.FromTwips(4320) },
        };

        IReadOnlyList<PaginatedSection> resolved = ContinuousPageDescriptors.Resolve(
            [
                new PaginatedSection(new WritingSection { Page = FirstGeometry }),
                new PaginatedSection(
                    new WritingSection { Page = stated, Break = SectionBreak.Continuous }),
            ],
            []);

        resolved[1].Section.Page.Margins.Left.ShouldBe(FirstGeometry.Margins.Left);
    }

    /// <summary>A section that is not continuous keeps everything it states.</summary>
    /// <remarks>
    /// The control on all of the above: the rule is about the break type and nothing else, and a
    /// <c>nextPage</c> section builds its own page style unconditionally
    /// (<c>PropertyMap.cxx</c>:1887-1900, which calls <c>GetPageStyle</c> and
    /// <c>HandleMarginsHeaderFooter</c> outright).
    /// </remarks>
    [Fact]
    public void ANextPageSectionKeepsItsOwnDescriptor()
    {
        List<LaidOutPage> pages = Paginate(
            statesOwnFurniture: true, hardBreakInside: false, kind: SectionBreak.NextPage);

        pages[^1].BodyArea.Y.ShouldBe(SecondSectionBodyTop);
        HeaderText(pages[^1]).ShouldBe("second head");
    }

    private static List<LaidOutPage> Paginate(
        bool statesOwnFurniture,
        bool hardBreakInside,
        int? restartAt = null,
        SectionBreak kind = SectionBreak.Continuous)
    {
        List<PageBlock> blocks =
        [
            Paragraph("body of the first section", section: 0),
            Paragraph("first paragraph of the second section", section: 1),
            Paragraph("second paragraph of the second section", section: 1, startsNewPage: hardBreakInside),
        ];

        List<PaginatedSection> sections =
        [
            new PaginatedSection(new WritingSection { Page = FirstGeometry }, Furniture("first")),
            new PaginatedSection(
                new WritingSection
                {
                    Page = SecondGeometry,
                    Break = kind,
                    RestartPageNumberAt = restartAt,
                },
                Furniture("second"),
                statesOwnFurniture),
        ];

        return new Paginator(PaginationOptions.Word).Paginate(blocks, sections);
    }

    private static IEnumerable<Length> BodyTops(IEnumerable<LaidOutPage> pages)
        => pages.Select(page => page.BodyArea.Y);

    private static string HeaderText(LaidOutPage page)
    {
        page.Header.ShouldNotBeNull();
        return ((PageParagraph)page.Header!.Blocks[0]).Text;
    }

    private static PageFurnitureSet Furniture(string which) => new(
        new Dictionary<PageFurnitureSlot, IReadOnlyList<PageBlock>>
        {
            [PageFurnitureSlot.Default] = [Paragraph($"{which} head", section: 0)],
        });

    /// <summary>The first section's body origin, which every inheriting page has to share.</summary>
    private static Length FirstSectionBodyTop => Length.FromTwips(1440);

    /// <summary>The second section's, which only a landed descriptor reaches.</summary>
    private static Length SecondSectionBodyTop => Length.FromTwips(2880);

    private static PageGeometry FirstGeometry => new()
    {
        Size = new DocSize(Length.FromTwips(11906), Length.FromTwips(16838)),
        Margins = new PageMargins(
            Length.FromTwips(1440), Length.FromTwips(1440),
            Length.FromTwips(1440), Length.FromTwips(1440)),
        HeaderDistance = Length.FromTwips(720),
        FooterDistance = Length.FromTwips(720),
    };

    private static PageGeometry SecondGeometry => FirstGeometry with
    {
        Margins = FirstGeometry.Margins with { Top = Length.FromTwips(2880) },
    };

    private static PageParagraph Paragraph(string text, int section, bool startsNewPage = false) => new()
    {
        Text = text,
        Face = Face,
        EmSize = Length.FromPoints(11),
        SectionIndex = section,
        Format = new Text.Layout.ParagraphFormat { StartsNewPage = startsNewPage },
    };

    private static OpenTypeFace Face { get; } = Resolve();

    private static OpenTypeFace Resolve()
    {
        SystemFontResolver resolver = new(SystemFontIndex.Build());
        return resolver.LoadOpenType(
            resolver.Resolve(new FontRequest("Liberation Serif", 400, false)));
    }
}
