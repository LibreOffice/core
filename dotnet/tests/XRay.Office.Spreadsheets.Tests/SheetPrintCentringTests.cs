using XRay.Office.Core.Documents;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A centred block is centred whether or not it fits, so one wider than the page hangs off
/// <em>both</em> edges.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Calc halves the remainder without looking at its sign.</strong>
/// <c>ScPrintFunc::PrintPage</c> writes
/// <c>nLeftSpace += ( aPageRect.GetWidth() - nDataWidth ) / 2</c> and
/// <c>nTopSpace += ( aPageRect.GetHeight() - nDataHeight ) / 2</c> with no clamp on either
/// (<c>sc/source/ui/view/printfun.cxx</c>:2165 and :2188), so the offset goes negative exactly
/// when the columns do not fit. Guarding the addition on a positive remainder turns centring into
/// left-alignment on precisely the sheets where the flag shows most.
/// </para>
/// <para>
/// <strong>A single column cannot be split, which is what makes the case reachable.</strong> A
/// block of several columns wider than the page paginates into a second page column instead of
/// overflowing; one column wider than the page has nowhere to break, so it is drawn overflowing
/// and the centring decides where. That is
/// <c>048_Expense_trends_budget_18d1e8ba.xlsx</c>'s <c>tips</c> sheet — one column of 152.33
/// characters under <c>printOptions/@horizontalCentered</c> — where every line of page 1 sat
/// <strong>167.3 pt</strong> right of 26.2.4.2's, the same constant on all nineteen, and the
/// reference lost <c>TEMPLAT</c> off the left edge while we chopped only at the right. 4088
/// alphanumerics against the reference's 3986, and 3999 now.
/// </para>
/// <para>
/// <strong>The fixture measures the centre rather than the edge, so no character-width conversion
/// is in the way.</strong> Two sheets, each one column holding one <c>horizontal="center"</c>
/// marker: the marker's own centre is the column's centre and therefore the block's, and the rule
/// says that lands on the printable area's centre — 306 pt on Letter with equal 0.7 inch margins —
/// whether the block is 160 characters wide or 20. Under the clamp the wide sheet's marker sits
/// near 470 pt instead.
/// </para>
/// <para>
/// Both expectations are the reference's own. LibreOffice <b>26.2.4.2</b> renders the committed
/// workbook with the marker's ink centred at <b>305.97</b> on the overflowing sheet and
/// <b>305.95</b> on the fitting one; ours are 306.00 and 306.00.
/// </para>
/// <para>
/// Reach: <b>73 of the corpus's 243 xlsx-family workbooks</b> carry at least one sheet stating
/// <c>horizontalCentered</c>, and 9 state <c>verticalCentered</c>. The clamp only bit the subset
/// whose block also overflows, which is why the flag looked implemented.
/// </para>
/// </remarks>
public sealed class SheetPrintCentringTests
{
    private const string Fixture = "sheet-print-centred-overflow.xlsx";

    /// <summary>Half the page across, which is also half the printable area on this fixture.</summary>
    private static readonly Length PageCentre = Length.FromPoints(306.0);

    private static List<DrawnPage> Draw()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require(Fixture));

        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();

        RecordingDrawingSink sink = new();
        foreach (SheetPage page in pages.Pages) page.Draw(sink);

        sink.Pages.Count.ShouldBe(2, "the fixture is two sheets and each is one page");
        return [.. sink.Pages];
    }

    private static double CentreOf(DrawnPage page, string marker)
    {
        DrawnGlyphRun run = page.Runs.Single(candidate => candidate.Text == marker);
        return (run.Origin.X + (run.Width / 2)).Points;
    }

    /// <summary>
    /// A block wider than the page is still centred on it, not pushed against the left margin.
    /// </summary>
    /// <remarks>
    /// The tolerance is a point because the marker's pen width includes its last glyph's right side
    /// bearing while the reference figure above is an ink extent. It cannot let the clamp through:
    /// the clamped answer is 164 pt away.
    /// </remarks>
    [Fact]
    public void AColumnWiderThanThePageIsCentredOffBothEdges()
    {
        CentreOf(Draw()[0], "WIDEBLOCK").ShouldBe(PageCentre.Points, 1.0);
    }

    /// <summary>
    /// A block that fits is centred exactly as it was, so the guard's removal took nothing with it.
    /// </summary>
    /// <remarks>
    /// Asserted beside the first because a layout that had simply stopped centring would pass that
    /// one: this sheet's 20-character column has 300 pt of spare and the marker still lands on the
    /// page's centre rather than at its left margin.
    /// </remarks>
    [Fact]
    public void ABlockThatFitsIsStillCentred()
    {
        CentreOf(Draw()[1], "NARROWBLOCK").ShouldBe(PageCentre.Points, 1.0);
    }

    /// <summary>
    /// The two sheets agree, which is the sign-independence of the rule stated directly.
    /// </summary>
    /// <remarks>
    /// One block overflows the printable area and one is dwarfed by it, and the same expression
    /// places both. A rule that branched on the sign could satisfy either assertion above on its
    /// own and cannot satisfy this one.
    /// </remarks>
    [Fact]
    public void OverflowAndSpareAreTheSameArithmetic()
    {
        List<DrawnPage> pages = Draw();

        CentreOf(pages[0], "WIDEBLOCK")
            .ShouldBe(CentreOf(pages[1], "NARROWBLOCK"), 0.5);
    }
}
