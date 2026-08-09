using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.WordProcessing.Layout;
using Paperless.WordProcessing.Model;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// A frame anchored inside a table in a running head, and a page field inside a text box.
/// </summary>
/// <remarks>
/// <para>
/// Both defects these were written against were found on one corpus document,
/// <c>words/batch-014/docx/UG.CAO.00133 Foreign Part 145 approvals - Language.docx</c>, whose header is
/// a two-column table with a 42.75 pt logo in the first cell and whose footer is a group of text boxes
/// with the <c>PAGE</c> field inside one of them.
/// </para>
/// <para>
/// <c>FrameLayout.FlowsOn</c> walked <c>page.Tables</c> and not a furniture flow's own
/// <see cref="PlacedFlow.Tables"/>, so the logo had no rectangle resolved and was drawn nowhere.
/// <c>PageFields</c> descended into a <see cref="PageTable"/>'s cells and not into a paragraph's
/// <see cref="PageParagraph.Frames"/>, so the footer's number was never substituted — and, worse, the
/// same walk is what decides whether a running head may be cached across pages, so every page was given
/// page one's copy of it.
/// </para>
/// <para>
/// Each was verified by putting the defect back and watching these fail:
/// restoring the body-tables-only walk fails <see cref="APictureInAHeaderTableIsPlacedOnThePage"/> and
/// <see cref="APictureInAFooterTableIsPlacedOnThePage"/>; restoring the frame-blind
/// <c>PageFields</c> fails the three below it.
/// </para>
/// </remarks>
public sealed class FurnitureFrameTests
{
    /// <summary>A logo in a header table cell reaches the page's frame list.</summary>
    [Fact]
    public void APictureInAHeaderTableIsPlacedOnThePage()
    {
        List<LaidOutPage> pages = Paginate(header: [FurnitureTable()]);

        pages.Count.ShouldBeGreaterThan(1);
        foreach (LaidOutPage page in pages)
        {
            page.Frames.Count.ShouldBe(1);
            page.Frames[0].Frame.Size.Width.ShouldBe(Logo.Width);
        }
    }

    /// <summary>And the same table used as a footer, which is the other furniture slot.</summary>
    [Fact]
    public void APictureInAFooterTableIsPlacedOnThePage()
    {
        List<LaidOutPage> pages = Paginate(footer: [FurnitureTable()]);

        pages[0].Frames.Count.ShouldBe(1);
    }

    /// <summary>
    /// The control: a table in the *body* was already walked, and still is.
    /// </summary>
    /// <remarks>
    /// Here so that the fix cannot be mistaken for having introduced the body case, and so that a
    /// future rewrite of <c>FlowsOn</c> that loses it fails something.
    /// </remarks>
    [Fact]
    public void APictureInABodyTableIsStillPlaced()
    {
        List<LaidOutPage> pages = Paginate(body: [FurnitureTable()]);

        pages.SelectMany(page => page.Frames).Count().ShouldBe(1);
    }

    /// <summary>A page number inside a text box in the running head is seen by the resolver.</summary>
    [Fact]
    public void ANumberInsideATextBoxIsFound()
    {
        PageFields.CarriesPageNumber([FramedNumber()]).ShouldBeTrue();
    }

    /// <summary>And substituted, leaving the anchoring paragraph's own text alone.</summary>
    [Fact]
    public void ANumberInsideATextBoxIsResolved()
    {
        PageParagraph resolved =
            (PageParagraph)PageFields.Resolve([FramedNumber()], 4, NoteNumberFormat.Arabic)[0];

        resolved.Text.ShouldBe("");
        Inside(resolved).ShouldBe("Page 4");
    }

    /// <summary>
    /// And every page gets its own, which is the caching half of the defect.
    /// </summary>
    /// <remarks>
    /// The substitution alone is not enough. <c>PageFurnitureSet</c> keys its cache on the slot when the
    /// head does not vary and on the slot plus the page number when it does, and it asks
    /// <c>CarriesPageNumber</c> which of the two this is. A footer whose number lives in a text box
    /// answered "does not vary", so page one's laid-out footer was returned for every page and no
    /// substitution could have shown.
    /// </remarks>
    [Fact]
    public void EveryPageGetsItsOwnNumberFromATextBox()
    {
        List<LaidOutPage> pages = Paginate(footer: [FramedNumber()]);

        pages.Count.ShouldBeGreaterThan(2);
        for (int at = 0; at < pages.Count; at++)
        {
            pages[at].Footer.ShouldNotBeNull();
            Inside((PageParagraph)pages[at].Footer!.Blocks[0]).ShouldBe($"Page {at + 1}");
        }
    }

    /// <summary>The text of the first block of the paragraph's first frame.</summary>
    private static string Inside(PageParagraph paragraph)
        => ((PageParagraph)paragraph.Frames[0].Blocks[0]).Text;

    /// <summary>A one-cell table whose only paragraph anchors a picture frame.</summary>
    private static PageTable FurnitureTable()
        => new()
        {
            ColumnWidths = [Length.FromTwips(4000)],
            Rows =
            [
                new PageTableRow
                {
                    Cells =
                    [
                        new PageTableCell
                        {
                            Blocks =
                            [
                                new PageParagraph
                                {
                                    Text = "",
                                    Face = Face,
                                    EmSize = Length.FromPoints(11),
                                    Frames =
                                    [
                                        new PageFrame
                                        {
                                            Size = Logo,
                                            Anchor = FrameAnchor.AsCharacter,
                                            AnchorOffset = 0,
                                            IsImage = true,
                                        },
                                    ],
                                },
                            ],
                        },
                    ],
                },
            ],
        };

    /// <summary>A paragraph anchoring a text box that holds a cached <c>Page 10</c>.</summary>
    private static PageParagraph FramedNumber()
        => new()
        {
            Text = "",
            Face = Face,
            EmSize = Length.FromPoints(11),
            Frames =
            [
                new PageFrame
                {
                    Size = new DocSize(Length.FromPoints(120), Length.FromPoints(14)),
                    Anchor = FrameAnchor.AsCharacter,
                    AnchorOffset = 0,
                    Blocks =
                    [
                        new PageParagraph
                        {
                            Text = "Page 10",
                            Face = Face,
                            EmSize = Length.FromPoints(11),
                            Fields = [new PageFieldSpan(5, 2, PageFieldKind.PageNumber)],
                        },
                    ],
                },
            ],
        };

    private static List<LaidOutPage> Paginate(
        IReadOnlyList<PageBlock>? header = null,
        IReadOnlyList<PageBlock>? footer = null,
        IReadOnlyList<PageBlock>? body = null)
    {
        PageFurnitureSet? furniture = header is null && footer is null
            ? null
            : new PageFurnitureSet(
                headers: header is null
                    ? null
                    : new Dictionary<PageFurnitureSlot, IReadOnlyList<PageBlock>>
                    {
                        [PageFurnitureSlot.Default] = header,
                    },
                footers: footer is null
                    ? null
                    : new Dictionary<PageFurnitureSlot, IReadOnlyList<PageBlock>>
                    {
                        [PageFurnitureSlot.Default] = footer,
                    });

        return new Paginator(PaginationOptions.Word).Paginate(
            [.. body ?? [], .. Body()],
            new WritingSection { Page = Geometry, PageNumberFormat = NoteNumberFormat.Arabic },
            furniture: furniture);
    }

    /// <summary>Enough body to fill three pages of the geometry below.</summary>
    private static IReadOnlyList<PageBlock> Body()
        => [.. Enumerable.Range(0, 120).Select(i => new PageParagraph
        {
            Text = $"Body paragraph {i}",
            Face = Face,
            EmSize = Length.FromPoints(11),
        })];

    private static DocSize Logo { get; } =
        new(Length.FromPoints(42.75), Length.FromPoints(42.75));

    /// <summary>An A4 page with an inch of margin and the furniture half an inch into it.</summary>
    private static PageGeometry Geometry { get; } = new()
    {
        Size = new DocSize(Length.FromTwips(11906), Length.FromTwips(16838)),
        Margins = PageMargins.Uniform(Length.FromTwips(1440)),
        HeaderDistance = Length.FromTwips(720),
        FooterDistance = Length.FromTwips(720),
    };

    private static OpenTypeFace Face { get; } = Resolve();

    private static OpenTypeFace Resolve()
    {
        SystemFontResolver resolver = new(SystemFontIndex.Build());
        return resolver.LoadOpenType(
            resolver.Resolve(new FontRequest("Liberation Serif", 400, false)));
    }
}
