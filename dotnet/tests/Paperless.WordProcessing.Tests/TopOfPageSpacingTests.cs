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
    /// A page reached by an explicit break keeps it.
    /// </summary>
    /// <remarks>
    /// <c>HasParaSpaceAtPages</c> returns true for <c>IsPageBreak(true)</c> before it ever looks at which
    /// page the frame is on, so a paragraph that asked to start a page is treated as though it were the
    /// first — which is how a heading styled with both a page break and space above keeps its gap.
    /// </remarks>
    [Fact]
    public void APageReachedByAnExplicitBreakKeepsIt()
    {
        List<PageBlock> blocks =
        [
            Paragraph("first"),
            Paragraph("second", startsNewPage: true),
        ];

        List<LaidOutPage> pages = new Paginator(PaginationOptions.Word).Paginate(
            blocks, new WritingSection { Page = Geometry });

        pages.Count.ShouldBe(2);
        pages[1].Lines[0].Top.ShouldBe(SpaceBefore);
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
