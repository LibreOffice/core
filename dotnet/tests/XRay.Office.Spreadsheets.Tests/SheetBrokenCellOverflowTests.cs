using XRay.Office.Core.Documents;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A cell of several paragraphs spills as far as its widest paragraph, not as far as their sum.
/// </summary>
/// <remarks>
/// <para>
/// A string too wide for its column spills into the empty cells beside it and Calc widens the
/// print area to cover all of it (<c>ScTable::ExtendPrintArea</c>,
/// <c>sc/source/core/data/table1.cxx</c>:2127, per cell in <c>MaybeAddExtraColumn</c> at
/// <c>:2218</c>). How wide "all of it" is comes from <c>ScColumn::GetNeededSize</c>, which sends
/// an <c>EditTextObject</c> cell through the EditEngine (<c>column2.cxx</c>:297-300), formats it
/// against a paper 1000000 units wide so that nothing is broken for width (<c>:447</c>), and reads
/// <c>pEngine-&gt;CalcTextWidth()</c> (<c>:565</c>) — which is a maximum over the paragraphs
/// (<c>ImpEditEngine::CalcTextWidth</c>, <c>editeng/source/editeng/impedit2.cxx</c>:3507-3525).
/// </para>
/// <para>
/// Which cells have several paragraphs is the importer's answer: Calc's ODF filter never puts the
/// engine into single-line mode, so a hard break makes a paragraph, while the BIFF and
/// SpreadsheetML filters do and the break stays a character inside one line. That is the same
/// distinction <see cref="SheetLayout.CellBreaksStartLines"/> carries for the drawing.
/// </para>
/// <para>
/// <c>sheet-cell-break-overflow.fods</c> holds the same sixty M's twice, in a 2 cm no-wrap column
/// on a 10 cm body, separated by newlines on one sheet and by spaces on the other. 26.2.4.2
/// renders it in <strong>four</strong> pages — one and three — and each sheet alone in one and
/// three. Measuring the whole string in both is what put
/// <c>CIS_Debian_Linux_8_Benchmark_v1.0.0.ods</c> at 88 pages against 61.
/// </para>
/// </remarks>
public sealed class SheetBrokenCellOverflowTests
{
    private static SpreadsheetPages Pages()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-cell-break-overflow.fods"));

        return (SpreadsheetPages)document.Layout();
    }

    /// <summary>The broken cell costs one page and the joined one three.</summary>
    [Fact]
    public void ABrokenCellSpillsAsFarAsItsWidestParagraph()
    {
        SpreadsheetPages pages = Pages();

        pages.Pages.Count(page => page.Sheet.Name == "Broken").ShouldBe(1);
        pages.Pages.Count(page => page.Sheet.Name == "Joined").ShouldBe(3);
    }

    /// <summary>
    /// And the two sheets' print areas say the same thing one step earlier.
    /// </summary>
    /// <remarks>
    /// Asserted beside the page count because the page count is a consequence: the joined sheet's
    /// area reaches further right, and it is the columns rather than the pages that the rule is
    /// about.
    /// </remarks>
    [Fact]
    public void TheJoinedSheetsPrintAreaReachesFurtherRight()
    {
        SpreadsheetPages pages = Pages();

        SheetLayout broken = pages.Pages.First(page => page.Sheet.Name == "Broken").Sheet;
        SheetLayout joined = pages.Pages.First(page => page.Sheet.Name == "Joined").Sheet;

        broken.PrintedRange.LastColumn.ShouldBeLessThan(joined.PrintedRange.LastColumn);
    }
}
