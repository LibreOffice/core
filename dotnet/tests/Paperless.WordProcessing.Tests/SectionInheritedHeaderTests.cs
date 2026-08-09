using Paperless.Core.Documents;
using Paperless.TestKit;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// A section that names no default header of its own takes the previous section's, whatever that
/// header holds.
/// </summary>
/// <remarks>
/// <para>
/// §17.10.1: a slot a <c>w:sectPr</c> does not name is inherited from the section before it, which
/// is what "link to previous" writes. <c>DocxReader.Furniture</c> implements that, and these tests
/// pin it — including the one case where **LibreOffice 24.2.7.2 does something else**, so that
/// following the reference there cannot happen without deleting a test that says why not to.
/// </para>
/// <para>
/// The fixture is the shape three corpus documents have and no authored probe of round 42 had. Its
/// first section's header is a table with a cell of title and a cell of revision and **no paragraph
/// outside the table**; its second section names an empty <c>even</c> and an empty <c>first</c>
/// header, which is what Word writes into slots the user never filled, and no default one. The
/// document states neither <c>w:evenAndOddHeaders</c> nor <c>w:titlePg</c>, so both named slots are
/// inert and the only header in play is the inherited one.
/// </para>
/// <para>
/// <strong>What the reference does, and why we do not follow it.</strong> LibreOffice's PDF of this
/// fixture heads page 1 and leaves page 2 bare. Its DOCX import means to link — "should be 'linked'
/// with the corresponding header or footer from the previous section … so we just copy the content",
/// <c>sw/source/writerfilter/dmapper/PropertyMap.cxx</c> — and the copy silently yields nothing when
/// the source header holds no top-level paragraph. It is an import defect rather than a decision:
/// LibreOffice's own flat-ODF export of the corpus document gives the inheriting sections a
/// <c>&lt;style:header&gt;</c> that is present and empty, which is header-on-with-no-content, and the
/// *identical* header content drawn from a section's own <c>w:headerReference</c> renders perfectly
/// four pages later in the same file. Named it draws; copied it vanishes.
/// </para>
/// <para>
/// Round 43 established the rule from both ends and the probes are committed:
/// <c>dotnet/probes/words-r43/header-inherit-bisect.py</c> cuts the real document down until adding a
/// bare <c>&lt;w:p/&gt;</c> before or after the header's table brings the running head back, and
/// <c>header-inherit-content-shape.py</c> authors eight headers of which the only one not inherited
/// is the table with no paragraph beside it. The cost of not reproducing it is one corpus verdict —
/// <c>UG.CAO.00133 … Language.docx</c>, 3944 extracted words against the reference's 3700, the
/// surplus being the head on thirteen pages the reference leaves bare — and CLAUDE.md's rule is to
/// record a reference import defect as a known deviation rather than contort Paperless into
/// reproducing it.
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
    /// The second section inherits it, although the header is nothing but a table.
    /// </summary>
    /// <remarks>
    /// This is the deliberate deviation described on the class. LibreOffice leaves this page bare;
    /// Word draws the head, §17.10.1 says to, and LibreOffice's own source says it means to.
    /// </remarks>
    [Fact]
    public void ASectionWithNoDefaultHeaderInheritsATableOnlyOne()
    {
        IReadOnlyList<LaidOutPage> pages = Paginate();

        pages[1].Header.ShouldNotBeNull("the second section links to the first");
        Text(pages[1]).ShouldBe("Running head Rev 1", "and takes its content, table and all");
    }

    /// <summary>
    /// Naming an empty even or first header does not replace the inherited one.
    /// </summary>
    /// <remarks>
    /// Both slots are inert here — the document sets neither <c>w:evenAndOddHeaders</c> nor
    /// <c>w:titlePg</c> — and the failure mode this pins is a reader that treats "the section names
    /// *a* header" as "the section has its own header", which would give page 2 the empty part it
    /// names instead of the head it links to. Measured on the reference: filling those two parts
    /// with text draws that text on no page of the corpus document at all.
    /// </remarks>
    [Fact]
    public void AnEmptyEvenOrFirstSlotDoesNotDisplaceTheInheritedHeader()
    {
        IReadOnlyList<LaidOutPage> pages = Paginate();

        Text(pages[1]).ShouldNotBeEmpty("an inert slot is not a header of one's own");
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
