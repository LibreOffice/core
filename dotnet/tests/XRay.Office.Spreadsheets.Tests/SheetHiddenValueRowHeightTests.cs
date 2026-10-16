using XRay.Office.Core.Documents;
using XRay.Office.Core.Extraction;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A cell an icon set hides is hidden at <em>paint</em> time, so its string still measures the
/// row.
/// </summary>
/// <remarks>
/// <para>
/// <c>ScOutputData::DrawStrings</c> clears <c>bDoCell</c> for a cell whose icon set or data bar
/// says <c>showValue="0"</c> — <c>sc/source/ui/view/output2.cxx</c>:1691-1698 <strong>in this
/// tree</strong>, which declares 27.2.0.0.alpha0+ and is not the 26.2.4.2 binary's source — and
/// that runs long after <c>ScColumn::GetOptimalHeight</c> has settled the row. This tree had two
/// answers to the same question: <c>SheetFormatting.HidesValue</c>, which is the paint-time
/// one and is right, and a second reader that dropped the cell's text while the
/// <c>ContentTable</c> was being built. The second one shortened the row with it.
/// </para>
/// <para>
/// The number the row should reach is the same 298 twips
/// <see cref="SheetConditionalRowHeightTests"/> pins, and for the same reason: the rule covers the
/// cells, so their pattern carries a conditional format and <c>bStdOnly</c> is cleared whatever
/// the string is. Confirmed against 26.2.4.2's own resolved view rather than against a rendering
/// — <c>--convert-to fods</c> of this fixture writes <c>style:row-height="0.2071in"</c>, which is
/// <b>14.911 pt</b>, on all five rows; this tree drew its icons on a <b>13.78 pt</b> pitch before
/// and a 14.88 pt one after.
/// </para>
/// <para>
/// <strong>Corpus reach is nil and that is measured, not assumed.</strong> Three of the 307
/// corpus spreadsheets state <c>showValue</c> off — <c>066_Agile_Gantt_chart</c> and
/// <c>077_Inventory_list_with_highlighting</c> on an <c>x14:iconSet</c>,
/// <c>036_Simple_to-do_list</c> on a main-namespace <c>dataBar</c>
/// (<c>probes/sheet-draw-r107/showvalue-census.txt</c>) — and all three render byte-identically
/// either way, because none of their hidden cells sits in a row whose height is recomputed. What
/// does move on them is <em>extraction</em>: <c>077</c> gains the twelve <c>1</c>s that
/// 26.2.4.2's own csv export has always written.
/// </para>
/// </remarks>
public sealed class SheetHiddenValueRowHeightTests
{
    [Fact]
    public void AHiddenValuesRowIsMeasuredAsThoughTheValueWereShown()
    {
        Sheet().Grid.Rows.SizeAt(1).Twips.ShouldBe(298);
    }

    [Fact]
    public void TheHiddenCellKeepsItsTextAndTheDrawingIsWhatDropsIt()
    {
        SheetLayout sheet = Sheet();

        // Row 1 column A holds 0; the rule's second bucket is a `shapes-diamond` with
        // `showValue="0"`, so the number is in the model and off the page.
        sheet.Formatting.HidesValue(1, 0).ShouldBeTrue();
        CellText(sheet, 1, 0).ShouldNotBeNullOrEmpty();

        // Row 0's bucket is `NoIcons`, which is no icon information at all, so its `-1` is drawn
        // — the control that separates "the text is back" from "nothing is hidden any more".
        sheet.Formatting.HidesValue(0, 0).ShouldBeFalse();
        CellText(sheet, 0, 0).ShouldNotBeNullOrEmpty();
    }

    private static string? CellText(SheetLayout sheet, int row, int column)
    {
        foreach (ContentTableRow tableRow in (sheet.Cells?.Children ?? []).OfType<ContentTableRow>())
        {
            foreach (ContentTableCell cell in tableRow.Children.OfType<ContentTableCell>())
            {
                if (cell.Row == row && cell.Column == column) return cell.GetOwnText();
            }
        }

        return null;
    }

    private static SheetLayout Sheet()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-cf-icon-set-x14-custom.xlsx"));

        return ((SpreadsheetPages)document.Layout()).Sheets[0];
    }
}
