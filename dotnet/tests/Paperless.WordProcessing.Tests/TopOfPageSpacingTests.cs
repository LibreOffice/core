using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Layout;
using Paperless.WordProcessing.Layout;
using Paperless.WordProcessing.Model;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// Where a paragraph's space-before survives the top of a page and where it does not.
/// </summary>
/// <remarks>
/// <para>
/// Writer's <c>PARA_SPACE_MAX_AT_PAGES</c> is not the switch it reads as. It decides whether the
/// question is asked; <c>SwFlowFrame::HasParaSpaceAtPages</c> (<c>flowfrm.cxx</c>:1415) then answers it
/// per paragraph, and in the document body it says yes only on the first page and after an explicit
/// break.
/// </para>
/// <para>
/// Measured against LibreOffice 24.2.7.2 on a DOCX whose paragraphs carry 20 pt of space-before and
/// nothing else, on A4 with a 1 inch margin: page one's first line sits at 92.03 pt and page two's at
/// 72.03 pt. Keeping the space on every page put every page after the first 20 pt low, which on a long
/// document is a page.
/// </para>
/// <para>
/// The explicit-break half of that has a second rule over it, and the same measurement on
/// <c>compatibilityMode</c> 15 finds the opposite answer: <c>SwFrame::IsCollapseUpper</c>
/// (<c>calcmove.cxx</c>:1120) takes the space back on every page but the first, so from Word 2013
/// onwards a <c>w:pageBreakBefore</c> keeps nothing either. See
/// <see cref="PaginationOptions.CollapsesUpperAtPageTop"/>, which carries the eleven-fixture
/// measurement.
/// </para>
/// </remarks>
public sealed class TopOfPageSpacingTests
{
    /// <summary>The document's first page keeps the space its first paragraph asks for.</summary>
    [Fact]
    public void TheFirstPageKeepsIt()
    {
        List<LaidOutPage> pages = Paginate();

        pages[0].Lines[0].Top.ShouldBe(SpaceBefore);
    }

    /// <summary>A page reached by running out of room does not.</summary>
    [Fact]
    public void APageReachedByOverflowDoesNot()
    {
        List<LaidOutPage> pages = Paginate();

        pages.Count.ShouldBeGreaterThan(1);
        pages[1].Lines[0].Top.ShouldBe(Length.Zero);
    }

    /// <summary>
    /// A page reached by an explicit break keeps it, in a file written before Word 2013.
    /// </summary>
    /// <remarks>
    /// <c>HasParaSpaceAtPages</c> returns true for <c>IsPageBreak(true)</c> before it ever looks at which
    /// page the frame is on, so a paragraph that asked to start a page is treated as though it were the
    /// first — which is how a heading styled with both a page break and space above keeps its gap.
    /// Measured on <c>compatibilityMode</c> 14, 12 and absent alike: the reference puts page two's first
    /// word at 92.35 pt against 72.35 for an automatic break.
    /// </remarks>
    [Fact]
    public void APageReachedByAnExplicitBreakKeepsIt()
    {
        List<LaidOutPage> pages = PaginateWithBreak(PaginationOptions.Word);

        pages.Count.ShouldBe(2);
        pages[1].Lines[0].Top.ShouldBe(SpaceBefore);
    }

    /// <summary>
    /// From Word 2013 it does not, which is the rule that sits on top of the one above.
    /// </summary>
    /// <remarks>
    /// The same two-paragraph document at <c>compatibilityMode</c> 15 puts page two's first word at
    /// 72.35 pt, not 92.35 — <c>SwFrame::IsCollapseUpper</c> zeroing the upper space that
    /// <c>HasParaSpaceAtPages</c> had just granted.
    /// </remarks>
    [Fact]
    public void FromWord2013AnExplicitBreakDoesNot()
    {
        List<LaidOutPage> pages = PaginateWithBreak(
            PaginationOptions.Word with { CollapsesUpperAtPageTop = true });

        pages.Count.ShouldBe(2);
        pages[1].Lines[0].Top.ShouldBe(Length.Zero);
    }

    /// <summary>
    /// The collapse spares the document's own first page, which is what makes it a page rule rather
    /// than a paragraph one.
    /// </summary>
    [Fact]
    public void TheFirstPageKeepsItEvenFromWord2013()
    {
        List<LaidOutPage> pages = PaginateWithBreak(
            PaginationOptions.Word with { CollapsesUpperAtPageTop = true });

        pages[0].Lines[0].Top.ShouldBe(SpaceBefore);
    }

    private static List<LaidOutPage> PaginateWithBreak(PaginationOptions options)
    {
        List<PageBlock> blocks =
        [
            Paragraph("first"),
            Paragraph("second", startsNewPage: true),
        ];

        return new Paginator(options).Paginate(blocks, new WritingSection { Page = Geometry });
    }

    /// <summary>An ODF document drops it everywhere, which is Writer's own behaviour.</summary>
    [Fact]
    public void WriterDropsItOnEveryPage()
    {
        List<LaidOutPage> pages = new Paginator(PaginationOptions.Default).Paginate(
            [.. Enumerable.Range(0, 120).Select(i => Paragraph($"line {i}"))],
            new WritingSection { Page = Geometry });

        pages[0].Lines[0].Top.ShouldBe(Length.Zero);
        pages[1].Lines[0].Top.ShouldBe(Length.Zero);
    }

    private static List<LaidOutPage> Paginate()
        => new Paginator(PaginationOptions.Word).Paginate(
            [.. Enumerable.Range(0, 120).Select(i => Paragraph($"line {i}"))],
            new WritingSection { Page = Geometry });

    private static Length SpaceBefore { get; } = Length.FromPoints(20);

    private static PageParagraph Paragraph(string text, bool startsNewPage = false) => new()
    {
        Text = text,
        Face = Face,
        EmSize = Length.FromPoints(11),
        Format = new ParagraphFormat
        {
            SpaceBefore = SpaceBefore,
            StartsNewPage = startsNewPage,
        },
    };

    /// <summary>An A4 page with a one inch margin all round.</summary>
    private static PageGeometry Geometry { get; } = new()
    {
        Size = new DocSize(Length.FromTwips(11906), Length.FromTwips(16838)),
        Margins = PageMargins.Uniform(Length.FromTwips(1440)),
    };

    private static OpenTypeFace Face { get; } = Resolve();

    private static OpenTypeFace Resolve()
    {
        SystemFontResolver resolver = new(SystemFontIndex.Build());
        return resolver.LoadOpenType(
            resolver.Resolve(new FontRequest("Liberation Serif", 400, false)));
    }
}
