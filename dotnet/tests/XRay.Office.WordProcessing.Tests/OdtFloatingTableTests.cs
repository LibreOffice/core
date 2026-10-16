using XRay.Office.Core.Documents;
using XRay.Office.TestKit;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A <c>draw:frame</c> holding nothing but a table is a floating table, and this engine already splits one.
/// </summary>
/// <remarks>
/// <para>
/// The object has two spellings and they are the same object. Word states it on the table
/// (<c>w:tblpPr</c>) and LibreOffice's importer makes a fly of it, always splittable —
/// <c>sw/source/writerfilter/dmapper/DomainMapperTableHandler.cxx</c>:1765, <em>"A text frame created for
/// floating tables is always allowed to split"</em>. ODF states it as the fly and marks the fly
/// <c>loext:may-break-between-pages="true"</c>, which
/// <c>xmloff/source/text/XMLTextFrameContext.cxx</c>:1104-1107 reads into <c>IsSplitAllowed</c> and
/// <c>SwFlyFrame::IsFlySplitAllowed</c> (<c>sw/source/core/layout/fly.cxx</c>:689-737) then honours for an
/// at-content anchor outside a header, a footer, a footnote and a multi-column section.
/// </para>
/// <para>
/// <b>The reader was the whole of the gap.</b> <c>Paginator.PlaceFloatedTable</c> has carried the rows a
/// page cannot take since the round that closed <c>Case-Study-Heathrow-Airport.docx</c>, and
/// <c>ContinueFloatedTables</c> places them at the top of the next page — starting one of its own when the
/// body has no more text. What the ODF reader did was route the same object through the frame path
/// instead, where the table is measured whole and its tail is drawn below the sheet: on
/// <c>Case-Study-Heathrow-Airport.odt</c> 2514 of the 3761 glyphs it drew were off the page, which reads
/// as missing text and is not.
/// </para>
/// <para>
/// Ground truth is LibreOffice 26.2.4.2's own PDF of the two fixtures. On the splittable one it draws two
/// pages: ROWA..ROWI at x = 54.100 from y = 54.508, and ROWJ..ROWL on page two at x = 54.100 from
/// y = 36.508 — the top margin exactly, with the fly's own <c>svg:y</c> <em>not</em> applied a second
/// time. On the control, with that one attribute taken away, it draws one page holding all twelve rows.
/// </para>
/// </remarks>
public sealed class OdtFloatingTableTests
{
    /// <summary>Half a point, the tolerance the rest of this suite compares a position at.</summary>
    private const double Tolerance = 0.5;

    /// <summary>The splittable fly is broken across two pages, where the reference breaks it.</summary>
    /// <remarks>
    /// Nine rows and three, which is what the 144 pt body takes at the 18 pt the fly's <c>svg:y</c>
    /// puts it down the page: the ninth row's baseline is the last that starts above the bottom.
    /// </remarks>
    [Fact]
    public void ASplittableFlyIsBrokenWhereTheReferenceBreaksIt()
    {
        List<Dictionary<string, DrawnWord>> pages = Words("odt-floating-table.fodt");

        pages.Count.ShouldBe(2);
        pages[0].Keys.ShouldContain("ROWI");
        pages[0].Keys.ShouldNotContain("ROWJ");
        pages[1].Keys.ShouldContain("ROWJ");
        pages[1].Keys.ShouldContain("ROWL");
    }

    /// <summary>
    /// And the continuation starts at the top margin, not at the fly's own offset a second time.
    /// </summary>
    /// <remarks>
    /// The one number that separates a real follow from a second copy of the fly: 26.2.4.2 draws ROWJ at
    /// y = 36.508, the top margin, while the first part starts 18 pt lower at 54.508 because
    /// <c>svg:y="0.25in"</c> positions the fly's <em>first</em> part alone.
    /// </remarks>
    [Fact]
    public void TheContinuationStartsAtTheTopMarginRatherThanAtTheFlysOffset()
    {
        List<Dictionary<string, DrawnWord>> pages = Words("odt-floating-table.fodt");

        pages[0]["ROWA"].Baseline.ShouldBe(54.508 + BaselineWithinLine, Tolerance);
        pages[1]["ROWJ"].Baseline.ShouldBe(36.508 + BaselineWithinLine, Tolerance);
    }

    /// <summary>
    /// The fly's <c>svg:x</c> is measured from the sheet's own edge, because it says <c>page</c>.
    /// </summary>
    /// <remarks>
    /// <c>svg:x="0.75in"</c> under <c>style:horizontal-rel="page"</c> is 54 pt from the sheet, which is
    /// 18 pt inside the half-inch margin. Read against the text area instead the table would sit at 90,
    /// and the reference draws it at 54.100 on both pages.
    /// </remarks>
    [Fact]
    public void ThePageAnchoredOffsetIsMeasuredFromTheSheet()
    {
        List<Dictionary<string, DrawnWord>> pages = Words("odt-floating-table.fodt");

        pages[0]["ROWA"].Left.ShouldBe(54.100, Tolerance);
        pages[1]["ROWJ"].Left.ShouldBe(54.100, Tolerance);
    }

    /// <summary>
    /// Without the attribute the same fly stays whole on one page, which is the control on all of it.
    /// </summary>
    /// <remarks>
    /// Measured rather than assumed: stripping <c>loext:may-break-between-pages</c> makes 26.2.4.2 itself
    /// draw one page with all twelve rows on it. The same experiment over the corpus is what says what
    /// splitting is worth — of the 20 failing rows holding a splittable fly the reference splits
    /// <b>six</b>, and on two of those its unsplit rendering reproduces this tree's own page and glyph
    /// counts exactly.
    /// </remarks>
    [Fact]
    public void AFlyThatMayNotBreakStaysWholeOnOnePage()
    {
        List<Dictionary<string, DrawnWord>> pages = Words("odt-floating-table-whole.fodt");

        pages.Count.ShouldBe(1);
        pages[0].Keys.ShouldContain("ROWA");
        pages[0].Keys.ShouldContain("ROWL");
    }

    /// <summary>
    /// The split fly is one object: it is a positioned block table and no longer a frame as well.
    /// </summary>
    /// <remarks>
    /// The failure this rules out is drawing both — the reader lifts the table out of the fly, and a fly
    /// left behind holding the same table would draw every row twice.
    /// </remarks>
    [Fact]
    public void TheFlyIsAPositionedTableAndNotAlsoAFrame()
    {
        using DocumentSource source =
            DocumentSource.FromFile(Corpus.Require("odt-floating-table.fodt"));
        using IDocument document = new WordProcessingReader().Read(source);
        WordProcessingPages pages = (WordProcessingPages)((IPaginatedDocument)document).Layout();

        PageTable table = pages.Blocks.OfType<PageTable>().ShouldHaveSingleItem();
        table.IsPositioned.ShouldBeTrue();
        table.Rows.Count.ShouldBe(12);

        pages.Pages.SelectMany(page => page.Frames).ShouldBeEmpty();
    }

    /// <summary>
    /// How far a 12 pt Liberation Serif line's baseline sits below the line box's top.
    /// </summary>
    /// <remarks>
    /// The fixtures' positions are read from the reference as text-box tops and asserted against drawn
    /// baselines, so one constant converts between them. It is the face's ascent at 12 pt and is the same
    /// for every line of both fixtures, which is why it can be a constant rather than a measurement.
    /// </remarks>
    private const double BaselineWithinLine = 11.086;

    /// <summary>Each page's words, by their text.</summary>
    private static List<Dictionary<string, DrawnWord>> Words(string fixture)
    {
        RecordingDrawingSink sink = new();

        using (DocumentSource source = DocumentSource.FromFile(Corpus.Require(fixture)))
        {
            using IDocument document = new WordProcessingReader().Read(source);
            IPageSequence pages = ((IPaginatedDocument)document).Layout();
            for (int index = 0; index < pages.Count; index++) pages[index].Draw(sink);
        }

        return [.. sink.Pages.Select(page =>
            DrawnWords.On(page).ToDictionary(word => word.Text, word => word))];
    }
}
