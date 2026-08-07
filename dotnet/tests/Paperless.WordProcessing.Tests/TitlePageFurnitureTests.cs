using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.TestKit;
using Paperless.Text.Fonts;
using Paperless.WordProcessing.Layout;
using Paperless.WordProcessing.Model;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// What a section draws on its first page when it says that page is different and supplies nothing for it.
/// </summary>
/// <remarks>
/// <para>
/// "Different first page" — DOCX's <c>w:titlePg</c>, RTF's <c>\titlepg</c>, WW8's
/// <c>sprmSFTitlePage</c>, ODF's separate first-page master — asserts only that the first page is
/// <em>different</em>. A section that turns it on and names no first-page header therefore draws none,
/// rather than falling back to the header its other pages use: Writer keeps a distinct page descriptor
/// for that page and an empty header on it stays empty.
/// </para>
/// <para>
/// The fall-through was worth two things at once, which is why it went unnoticed. It put the running head
/// on a title page, and it charged that page for a header it should not have had. Measured on
/// <c>words/batch-007/final-technical-report-template.docx</c>, whose first <c>w:sectPr</c> names a
/// default header and a <c>w:titlePg</c> and no <c>w:type="first"</c> reference anywhere: LibreOffice's
/// page one carries no running head, ours carried the default one, and the document's extractable words
/// went 1123 to 1116 against the reference's 1135.
/// </para>
/// </remarks>
public sealed class TitlePageFurnitureTests
{
    /// <summary>
    /// A section with a title page and no first-page header draws nothing there — and still draws its
    /// ordinary header on every later page.
    /// </summary>
    [Fact]
    public void ATitlePageWithNoFirstPageHeaderDrawsNoHeader()
    {
        List<LaidOutPage> pages = Paginate(hasTitlePage: true, withFirstSlot: false);

        pages.Count.ShouldBeGreaterThan(1, "the fixture has to reach a second page to be worth asserting");
        pages[0].Header.ShouldBeNull("a title page with nothing stated for it draws no header");
        HeaderText(pages[1]).ShouldBe("running head");
    }

    /// <summary>
    /// The same section with a first-page header draws that one, which is the rule the fix must not undo.
    /// </summary>
    [Fact]
    public void ATitlePageWithAFirstPageHeaderDrawsIt()
    {
        List<LaidOutPage> pages = Paginate(hasTitlePage: true, withFirstSlot: true);

        HeaderText(pages[0]).ShouldBe("title head");
        HeaderText(pages[1]).ShouldBe("running head");
    }

    /// <summary>
    /// Without the flag the first page is not special, so it takes the ordinary header even though a
    /// first-page slot exists.
    /// </summary>
    [Fact]
    public void WithoutTheFlagTheFirstPageTakesTheOrdinaryHeader()
    {
        List<LaidOutPage> pages = Paginate(hasTitlePage: false, withFirstSlot: true);

        HeaderText(pages[0]).ShouldBe("running head");
        HeaderText(pages[1]).ShouldBe("running head");
    }

    /// <summary>
    /// The same rule on a real document, in the four formats that state the flag.
    /// </summary>
    /// <remarks>
    /// <c>title-page-header</c> is a 12 cm page whose first paragraph carries its own master page and
    /// whose ordinary one carries a running head. LibreOffice 24.2.7.2 exports that as a lone
    /// <c>w:headerReference w:type="default"</c> beside a <c>w:titlePg</c> with no first-page reference
    /// anywhere — the shape the corpus document has — and its own PDF of all four leaves page one bare and
    /// heads pages two and three.
    ///
    /// <para>
    /// The <c>.odt</c> of the same document is committed and deliberately <em>not</em> asserted here: it
    /// states the distinction as two master pages joined by <c>style:next-style-name</c> rather than as a
    /// flag, nothing reads that, and every page of it therefore takes the title page's master and comes
    /// out bare. That is a separate defect in the ODF reader with its own fixture already in place, and
    /// folding it into this assertion would hide it behind a rule it has nothing to do with.
    /// </para>
    /// </remarks>
    [Theory]
    [InlineData("title-page-header.docx")]
    [InlineData("title-page-header.doc")]
    [InlineData("title-page-header.rtf")]
    public void TheCorpusFixtureLeavesItsTitlePageBare(string name)
    {
        using IDocument document =
            new WordProcessingReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        pages.Pages.Count.ShouldBeGreaterThan(1, $"{name} runs past its first page");

        pages.Pages[0].Header.ShouldBeNull($"{name}: the title page states no header of its own");
        pages.Pages[1].Header.ShouldNotBeNull($"{name}: every later page keeps the running head");
    }

    private static List<LaidOutPage> Paginate(bool hasTitlePage, bool withFirstSlot)
    {
        List<PageBlock> blocks =
            [.. Enumerable.Range(0, 24).Select(i => Paragraph($"body line {i:00}"))];

        Dictionary<PageFurnitureSlot, IReadOnlyList<PageBlock>> headers = new()
        {
            [PageFurnitureSlot.Default] = [Paragraph("running head")],
        };

        if (withFirstSlot) headers[PageFurnitureSlot.First] = [Paragraph("title head")];

        WritingSection section = new()
        {
            Page = Geometry,
            HasDifferentFirstPage = hasTitlePage,
        };

        return new Paginator(PaginationOptions.Word).Paginate(
            blocks, [new PaginatedSection(section, new PageFurnitureSet(headers))]);
    }

    private static string HeaderText(LaidOutPage page)
    {
        page.Header.ShouldNotBeNull();
        return ((PageParagraph)page.Header!.Blocks[0]).Text;
    }

    /// <summary>A page short enough that two dozen lines need several of them.</summary>
    private static PageGeometry Geometry => new()
    {
        Size = new DocSize(Length.FromTwips(11906), Length.FromTwips(4200)),
        Margins = new PageMargins(
            Length.FromTwips(720), Length.FromTwips(720),
            Length.FromTwips(720), Length.FromTwips(720)),
        HeaderDistance = Length.FromTwips(360),
        FooterDistance = Length.FromTwips(360),
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
