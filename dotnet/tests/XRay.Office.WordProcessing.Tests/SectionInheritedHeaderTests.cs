using XRay.Office.Core.Documents;
using XRay.Office.TestKit;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A section that names no default header of its own takes the previous section's — unless that header
/// holds nothing but tables, which is the one shape LibreOffice does not pass down.
/// </summary>
/// <remarks>
/// <para>
/// §17.10.1: a slot a <c>w:sectPr</c> does not name is inherited from the section before it, which is
/// what "link to previous" writes. <c>DocxReader.Furniture</c> implements that. The exception is stated
/// on <c>DocxReader.FurnitureCarry</c> and pinned by the shape variants in
/// <see cref="InheritedTableHeaderTests"/>; this file is the same rule on the corpus fixture that
/// produced it.
/// </para>
/// <para>
/// The fixture is the shape three corpus DOCX have. Its first section's header is a table with a cell
/// of title and a cell of revision and <strong>no paragraph outside the table</strong>; its second
/// section names an empty <c>even</c> and an empty <c>first</c> header, which is what Word writes into
/// slots the user never filled, and no default one. The document states neither
/// <c>w:evenAndOddHeaders</c> nor <c>w:titlePg</c>, so both named slots are inert and the only header in
/// play is the inherited one.
/// </para>
/// <para>
/// <strong>This file asserted the opposite until 2026-08-15, deliberately, and the reversal is the
/// point — so read why before turning it back.</strong> Round 43 established the mechanism from both
/// ends (<c>probes/words-r43/header-inherit-bisect.py</c> cuts the real document down until a bare
/// <c>&lt;w:p/&gt;</c> beside the header's table brings the running head back;
/// <c>header-inherit-content-shape.py</c> authors eight headers of which the only one not inherited is
/// the table with no paragraph beside it) and declined to reproduce it, under TODO.md's non-goal
/// "bug-for-bug reproduction of LibreOffice's own import defects". It is an import defect: LibreOffice
/// means to link — "should be 'linked' with the corresponding header or footer from the previous
/// section … so we just copy the content", <c>sw/source/writerfilter/dmapper/PropertyMap.cxx</c> — and
/// the copy silently yields nothing when the source header holds no top-level paragraph. The same
/// header content <em>named</em> by a section's own reference draws perfectly.
/// </para>
/// <para>
/// Two things changed the accounting. First, round 43 measured against <strong>24.2.7.2</strong>, and
/// CLAUDE.md lists "the reference's own table-only-header import defect" among the claims that need one
/// re-check against 26.2.4.2 before being relied on. Re-measured on 26.2.4.2 with a fresh two-section
/// probe, the behaviour is unchanged, so the mechanism survives the version move intact. Second, and
/// this is what round 43 could not see, <strong>the cost was understated by a page</strong>: it recorded
/// the fix as gaining one verdict and improving <c>UG.CAO.00006</c>'s words "without moving its
/// verdict", where in fact the spurious running head was also costing that document a page —
/// <c>30/29</c> pages and 8001 words against 7399 before, <c>29/29</c> and 7390 against 7399 after. A
/// page count is check one of the gate, so this is not a word-column trade.
/// </para>
/// <para>
/// The rest of the cost stands as round 43 stated it and is not hidden here:
/// <c>probes/words-r43/table-only-header-census.py</c> finds 3 of the 134 corpus DOCX inheriting a
/// table-only header and can see none of the 66 <c>.doc</c>; the third,
/// <c>docs-quality-MA.IMS.00001 …</c>, has its word error made worse (11973 against 12213) and its
/// verdict is unmoved, because it fails on pages at 43/44 either way.
/// </para>
/// <para>
/// A measurement trap worth keeping: this document's running head opens with "European Aviation Safety
/// Agency" and so does its <strong>footer</strong>, which prints on every page of both renderings. A
/// probe keyed on that phrase reports the head everywhere and closes the question in the wrong
/// direction. Key on "Approval Date", which appears in no footer.
/// </para>
/// </remarks>
public sealed class SectionInheritedHeaderTests
{
    /// <summary>The first section draws its own header, table and all.</summary>
    [Fact]
    public void ATableHeaderIsLaidOutOnItsOwnSection()
    {
        IReadOnlyList<LaidOutPage> pages = Paginate();

        pages.Count.ShouldBeGreaterThanOrEqualTo(2, "the section break starts a new page");
        Text(pages[0]).ShouldBe("Running head Rev 1");
    }

    /// <summary>
    /// The second section does not inherit it, because the header is nothing but a table.
    /// </summary>
    [Fact]
    public void ASectionWithNoDefaultHeaderDoesNotInheritATableOnlyOne()
    {
        IReadOnlyList<LaidOutPage> pages = Paginate();

        pages[1].Header.ShouldBeNull(
            "the copy that would carry it down has no paragraph to start from");
    }

    /// <summary>
    /// Naming an empty even or first header is still not a header of the section's own.
    /// </summary>
    /// <remarks>
    /// Both slots are inert here — the document sets neither <c>w:evenAndOddHeaders</c> nor
    /// <c>w:titlePg</c> — and what this pins is a reader that treats "the section names *a* header" as
    /// "the section has its own header". That reader would give page 2 the empty part it names, and
    /// arrive at the same bare page for the wrong reason; page 1 is what separates them, since a reader
    /// that let an inert slot displace the named default would strip page 1 as well.
    /// </remarks>
    [Fact]
    public void AnEmptyEvenOrFirstSlotDoesNotDisplaceTheSectionsOwnHeader()
    {
        IReadOnlyList<LaidOutPage> pages = Paginate();

        Text(pages[0]).ShouldNotBeEmpty("an inert slot is not a header of one's own");
    }

    private static IReadOnlyList<LaidOutPage> Paginate()
    {
        using IDocument document = new WordProcessingReader()
            .Read(DocumentSource.FromFile(Corpus.Require("inherited-table-header.docx")));

        return ((WordProcessingPages)((IPaginatedDocument)document).Layout()).Pages;
    }

    /// <summary>Everything the page's running head draws, joined — tables included.</summary>
    private static string Text(LaidOutPage page)
        => page.Header is null ? string.Empty : string.Join(' ', Words(page.Header.Blocks));

    private static IEnumerable<string> Words(IEnumerable<PageBlock> blocks)
    {
        foreach (PageBlock block in blocks)
        {
            switch (block)
            {
                case PageParagraph paragraph when paragraph.Text.Length > 0:
                    yield return paragraph.Text;
                    break;
                case PageTable table:
                    foreach (PageTableRow row in table.Rows)
                    {
                        foreach (PageTableCell cell in row.Cells)
                        {
                            foreach (string word in Words(cell.Blocks))
                            {
                                yield return word;
                            }
                        }
                    }

                    break;
            }
        }
    }
}
