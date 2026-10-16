using XRay.Office.Core.Documents;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// Which cells outside a page's own column band still put ink on it.
/// </summary>
/// <remarks>
/// <para>
/// <strong>The rule is asymmetric, and both halves are pinned here because the symmetric reading
/// is the natural mistake.</strong> A run anchored to the <em>left</em> of a page's column band
/// is not painted on it at all, however far its output area reaches; a run anchored to the
/// <em>right</em> of the band is. Calc's string output looks one column past <c>mnX2</c>
/// unconditionally (<c>output2.cxx:1660-1678</c>) but one column before <c>mnX1</c> only
/// <c>if (mnX1 &gt; 0 &amp;&amp; !bTaggedPDF)</c> (<c>:1541-1543</c>) — and <c>UseTaggedPDF</c>
/// defaults to <c>true</c>, so the reference never takes that branch.
/// </para>
/// <para>
/// Measured against 26.2.4.2 rather than read off the source, because the C++ tree beside this
/// one is a later development version. Rendering <c>essd-16-3433-2024-t02.xlsx</c> through the
/// same binary twice, changing nothing but that one filter option, gives words per page
/// <c>439 / 0 / 0 / 0</c> tagged and <c>439 / 315 / 152 / 49</c> untagged. Painting the lead-in
/// anyway cost 617 surplus words on <c>RCO_VOR_Master_List_082824.xlsx</c>, spread over five
/// pages the reference leaves blank, and 514 on <c>essd</c> itself.
/// </para>
/// <para>
/// The second half of the rule is <c>bOutside</c> (<c>:2037</c>): of the cell found past the
/// band, one whose output area — its own column, widened through the empty cells beside it —
/// does not overlap the block at all is skipped. Each fixture's rows differ in nothing but the
/// length or the neighbours of one string, so a renderer cannot satisfy them by choosing a side.
/// </para>
/// <para>
/// A merged block anchored in a hidden column is a third case and is here for a related reason:
/// nothing that walks the columns a page places can reach its anchor, so Calc reaches it from
/// the first covered cell whose path back is entirely hidden
/// (<c>ScOutputData::GetMergeOrigin</c>, <c>:953</c>).
/// </para>
/// </remarks>
public sealed class SheetOffPageCellTests
{
    private static IReadOnlyList<DrawnPage> Draw(string name)
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require(name));

        SpreadsheetPages pages = (SpreadsheetPages)document.Layout();

        RecordingDrawingSink sink = new();
        foreach (SheetPage page in pages.Pages) page.Draw(sink);

        return sink.Pages;
    }

    private static string TextOf(DrawnPage page)
        => string.Join(" ", page.Runs.Select(run => run.Text));

    [Fact]
    public void AStringSpillingRightwardsIsNotDrawnOnTheNextPage()
    {
        IReadOnlyList<DrawnPage> pages = Draw("sheet-lead-in.fods");
        pages.Count.ShouldBe(3, "the long string widens the printed block past column D");

        // Page 1 holds columns A to C, so both strings are on it in their own right.
        string first = TextOf(pages[0]);
        first.ShouldContain("SHORTC2");
        first.ShouldContain("column C");

        // Page 2 is column D alone, and holds column D and nothing else. The long string's
        // output area does reach it — that is why page 3 exists at all — and it is still not
        // painted here, because the loop that would have found its anchor never runs.
        string second = TextOf(pages[1]);
        second.ShouldContain("DDDD");
        second.ShouldNotContain("column C");
        second.ShouldNotContain("SHORTC2");

        // Page 3 is bought by the spill and then left blank, which is the whole shape of the
        // rule: what decides the paper and what decides the ink are different questions.
        TextOf(pages[2]).ShouldNotContain("beyond");
        TextOf(pages[2]).Trim().ShouldBeEmpty();
    }

    [Fact]
    public void AStringSpillingLeftwardsIsDrawnOnThePreviousPage()
    {
        IReadOnlyList<DrawnPage> pages = Draw("sheet-trail-in.fods");
        pages.Count.ShouldBe(2, "a right-aligned overflow costs no columns at the right-hand end");

        string first = TextOf(pages[0]);

        // Row 1's string is anchored in column D, which is page 2, and reaches back into page 1.
        // This is the direction that carries no bTaggedPDF guard, so it survives into the PDF.
        first.ShouldContain("long way back");

        // Row 2's short string fits column D and never leaves page 2.
        first.ShouldNotContain("SHORTD2");

        // Exactly one of the two long strings reaches page 1. Row 3's is the same string, but
        // GUARDC3 sits in the column beside it, so the leftward walk stops there and its output
        // area never leaves column D. Counting is what discriminates: the two strings share
        // every word, so only their number can say whether one or both were painted.
        first.ShouldContain("GUARDC3");
        Occurrences(first, "into whatever lies").ShouldBe(1);

        // Both long strings, and the short one, are on the page that owns column D.
        string second = TextOf(pages[1]);
        second.ShouldContain("SHORTD2");
        Occurrences(second, "before it").ShouldBe(2);
    }

    private static int Occurrences(string text, string needle)
    {
        int count = 0;
        for (int at = text.IndexOf(needle, StringComparison.Ordinal);
             at >= 0;
             at = text.IndexOf(needle, at + needle.Length, StringComparison.Ordinal))
        {
            count++;
        }

        return count;
    }

    [Fact]
    public void AMergeAnchoredInAHiddenColumnIsDrawnFromTheFirstColumnItCovers()
    {
        IReadOnlyList<DrawnPage> pages = Draw("sheet-hidden-merge.fods");
        pages.Count.ShouldBe(1);

        string drawn = TextOf(pages[0]);
        drawn.ShouldContain("Merged heading");

        // Once only: the block's other two covered columns each stop at a visible neighbour.
        drawn.Split("Merged heading").Length.ShouldBe(2);

        // And an ordinary cell in the hidden column is still not drawn, which is the half of
        // the rule that keeps a collapsed column collapsed.
        drawn.ShouldNotContain("Inside");
        drawn.ShouldContain("Cee");
    }
}
