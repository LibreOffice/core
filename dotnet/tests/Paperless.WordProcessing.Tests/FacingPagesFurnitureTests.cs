using Paperless.Core.Documents;
using Paperless.TestKit;
using Paperless.WordProcessing.Layout;
using Shouldly;

namespace Paperless.WordProcessing.Tests;

/// <summary>
/// What a DOC does with the even-page half of its running heads.
/// </summary>
/// <remarks>
/// <para>
/// "Different odd and even pages" is a <em>document</em> flag in WW8 — <c>fFacingPages</c>, the lowest
/// bit of the <c>Dop</c>'s first word — and it decides two things at once. It is the condition under
/// which <c>wwSectionManager::SetSegmentToPageDesc</c> puts <c>WW8_HEADER_EVEN | WW8_FOOTER_EVEN</c>
/// into the <c>grpfIhdt</c> it synthesises (<c>ww8par6.cxx</c>:1234), and it is the condition under
/// which <c>wwSectionManager::SetUseOn</c> stops the left page sharing the right one's head — it adds
/// <c>UseOnPage::HeaderShare</c> exactly when the flag is <em>clear</em> (<c>ww8par.cxx</c>:4319).
/// </para>
/// <para>
/// Nothing read it, so every DOC section had one running head for every page and the even stories,
/// though read, could never reach a sheet. Eight of the sample corpus's sixty-six DOC files set the
/// flag and all eight carry even stories, <c>150_5300_13_chg8.doc</c> among them — whose whole title
/// block sat 14 pt too high for want of the blank odd head the same rule creates.
/// </para>
/// <para>
/// The fixture is committed as both formats it can be stated in. It names one <c>headerReference</c>
/// of type <c>even</c> and no other, and LibreOffice 24.2.7.2's own PDF of the DOC leaves page one
/// bare, heads page two, and runs to three pages.
/// </para>
/// </remarks>
public sealed class FacingPagesFurnitureTests
{
    /// <summary>The even header reaches even pages, and only those.</summary>
    [Theory]
    [InlineData("facing-pages-even-header.doc")]
    [InlineData("facing-pages-even-header.docx")]
    public void AnEvenHeaderIsDrawnOnEvenPagesOnly(string name)
    {
        IReadOnlyList<LaidOutPage> pages = Paginate(name);

        pages.Count.ShouldBeGreaterThanOrEqualTo(3, $"{name} runs to three pages");

        Text(pages[1]).ShouldBe("Even pages only", $"{name}: page two is an even page");
        Text(pages[0]).ShouldBeEmpty($"{name}: page one is odd and has no even head");
        Text(pages[2]).ShouldBeEmpty($"{name}: page three is odd too");
    }

    /// <summary>
    /// The odd pages carry a head all the same — an empty one, which draws nothing and takes room.
    /// </summary>
    /// <remarks>
    /// Two rules put it there and either would. The section's odd-header story exists and holds one
    /// empty paragraph, which <c>Read_HdFt</c> reads because its length is two; and had the story been
    /// of no length at all, "cannot have left without right" (<c>#i17196#</c>) would have given the
    /// master page a blank head because the even slot turned one on. A DOC written by Word states the
    /// second case and one written by LibreOffice states the first, so both are live.
    /// </remarks>
    [Fact]
    public void TheOddPagesStillHaveAHead()
    {
        IReadOnlyList<LaidOutPage> pages = Paginate("facing-pages-even-header.doc");

        pages[0].Header.ShouldNotBeNull("an empty head is not the same as no head");
        Text(pages[0]).ShouldBeEmpty("and it draws nothing");
        // One empty line of the default style, which is what takes the room.
        pages[0].Header!.Advance.Points.ShouldBe(13.8, tolerance: 0.05);
    }


    /// <summary>
    /// A running head that holds one empty paragraph is a blank line to lay out, not a placeholder.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A DOC slot the section does not use has a story of <em>no length at all</em>, and that is the only
    /// thing that separates it from a slot holding one blank line: <c>SetSegmentToPageDesc</c> clears the
    /// slot's bit only when the length is zero (<c>ww8par6.cxx</c>:1247). Dropping the blank paragraph
    /// instead loses the whole head, and with it the room the head takes — which on a section reserving
    /// no band, <c>dyaTop == dyaHdrTop</c>, is the body's own starting height.
    /// </para>
    /// <para>
    /// The fixture states exactly that and nothing else, so no other rule can supply the head in its
    /// place. LibreOffice 24.2.7.2 draws its first body line at 259.16 pt from the foot of a 340.15 pt
    /// page; dropping the paragraph draws it about 11 pt higher, which is one line of the default style
    /// less the millimetre the export reserved.
    /// </para>
    /// <para>
    /// Fourteen DOC files of the sample corpus hold a story of length two. <c>A_320.doc</c> holds six of
    /// them in its first section and paginates 119 pages against the reference's 150 without this, and
    /// 141 with it.
    /// </para>
    /// </remarks>
    [Fact]
    public void ABlankRunningHeadStillTakesItsLine()
    {
        IReadOnlyList<LaidOutPage> pages = Paginate("blank-running-head.doc");

        pages[0].Header.ShouldNotBeNull("the story exists, so the head does");
        Text(pages[0]).ShouldBeEmpty("and it draws nothing");

        // 56.70 pt of header distance and one 13.80 pt line, against a stated top margin of 59.55 —
        // so the head outgrows the band and the body follows it down.
        pages[0].BodyArea.Y.Points.ShouldBe(70.50, tolerance: 0.05);
    }


    /// <summary>
    /// A section whose only header is the even one still has a blank one on its odd pages.
    /// </summary>
    /// <remarks>
    /// <para>
    /// "Cannot have left without right" — <c>#i17196#</c> in <c>Read_HdFt</c>. Every slot the section
    /// turns on sets the header on the <em>master</em> page format as well as on its own, so a section
    /// that names only an even-page head has an empty one on its odd pages. It draws nothing and
    /// occupies the header band, which on a section reserving none is the body's starting height.
    /// </para>
    /// <para>
    /// <c>blank-odd-head.doc</c> is <c>facing-pages-even-header.doc</c> with the odd-header story's
    /// length struck to nought in its <c>PlcfHdd</c> — a hand edit, because <em>no</em> exporter writes
    /// that shape. LibreOffice's own DOC filter always writes a story of one paragraph mark for a slot
    /// it does not use, which is length two and reads as a blank head by the ordinary route; only Word
    /// writes a story of no length at all, which is what <c>150_5300_13_chg8.doc</c> and
    /// <c>150_5300_13_chg12.doc</c> do and what leaves this rule the only thing that can supply the
    /// head. LibreOffice 24.2.7.2's flat-ODF export of the patched file is the reading being matched:
    /// <c>&lt;style:header&gt;&lt;text:p text:style-name="Header"/&gt;&lt;/style:header&gt;</c> beside a
    /// populated <c>&lt;style:header-left&gt;</c>.
    /// </para>
    /// </remarks>
    [Fact]
    public void AnEvenOnlyHeaderStillGivesTheOddPagesABlankOne()
    {
        IReadOnlyList<LaidOutPage> pages = Paginate("blank-odd-head.doc");

        Text(pages[1]).ShouldBe("Even pages only", "the even head is unaffected");

        pages[0].Header.ShouldNotBeNull("the master page gets one because the even slot turned it on");
        Text(pages[0]).ShouldBeEmpty("and it is blank");
        pages[0].Header!.Advance.Points.ShouldBe(13.8, tolerance: 0.05);
    }

    /// <summary>A title page with no first-page header of its own does not borrow the master's.</summary>
    /// <remarks>
    /// <c>title-page-no-head.doc</c> is a title-page section stating a default header and a first-page
    /// <em>footer</em>, with its first-page header story struck to nought — again by hand, and again
    /// because only Word writes that. LibreOffice's export of it gives the first page a blank header of
    /// its own rather than the running head, which is what the corpus's
    /// <c>150_5300_13_chg12.doc</c> needs: falling through to the Default slot printed
    /// "9/29/06 AC 150/5300-13 CHG 10" across the top of its title page.
    /// </remarks>
    [Fact]
    public void ATitlePageWithNoHeadOfItsOwnDrawsNoRunningHead()
    {
        IReadOnlyList<LaidOutPage> pages = Paginate("title-page-no-head.doc");

        Text(pages[0]).ShouldBeEmpty("the title page's head is blank, not the master's");
        Text(pages[1]).ShouldBe(
            "Running head on every page but the first", "and every later page keeps the master's");
    }

    private static IReadOnlyList<LaidOutPage> Paginate(string name)
    {
        using IDocument document =
            new WordProcessingReader().Read(DocumentSource.FromFile(Corpus.Require(name)));

        return ((WordProcessingPages)((IPaginatedDocument)document).Layout()).Pages;
    }

    /// <summary>Everything the page's running head draws, joined — empty when it draws nothing.</summary>
    private static string Text(LaidOutPage page)
        => page.Header is null
            ? string.Empty
            : string.Join(
                ' ',
                page.Header.Blocks.OfType<PageParagraph>().Select(p => p.Text).Where(t => t.Length > 0));
}
