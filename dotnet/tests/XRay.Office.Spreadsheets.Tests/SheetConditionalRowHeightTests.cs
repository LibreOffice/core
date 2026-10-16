using XRay.Office.Core.Documents;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// Two things put a cell on Calc's <em>measured</em> row-height branch that are not properties of
/// its string, and neither was read.
/// </summary>
/// <remarks>
/// <para>
/// <c>ScColumn::GetOptimalHeight</c> takes the cheap arithmetic answer —
/// <c>lcl_GetAttribHeight</c>, the font size times 1.18 plus the margins less 23 twips — only
/// while <c>bStdOnly</c> holds, and clears it for a cell whose pattern carries a conditional
/// format: <c>if (bStdOnly &amp;&amp; !pPattern-&gt;GetItem(ATTR_CONDITIONAL).GetCondFormatData().empty())
/// bStdOnly = false;</c> (<c>sc/source/core/data/column2.cxx:937-941</c>, under the comment
/// <em>"conditional formatting: loop all cells"</em>). Separately, a cell whose content Calc
/// replaced with a <c>SvxURLField</c> is an <c>EditTextObject</c>, which
/// <see cref="SheetEditCellTests"/> already covers for a rich or hard-broken string and which a
/// hyperlink reaches by a different door. Either way the row is measured through
/// <c>GetNeededSize</c>, and one line there is the face's ascent and descent quantised to whole
/// 96 dpi pixels plus a pixel of margin each side — <strong>298</strong> twips for Calibri 11
/// against the arithmetic's <strong>276</strong>.
/// </para>
/// <para>
/// The fixture is four rows of Calibri 11 in a three-inch column, differing in one property each:
/// plain, hyperlinked, covered by a conditional format, plain again. Measured by asking 26.2.4.2
/// for its own answer — <c>--convert-to fods</c>, whose <c>style:row-height</c> is the number Calc
/// computed rather than anything read off a page — it writes <strong>276, 298, 298, 276</strong>.
/// The fourth row is the control that matters most: it stands after the conditional one and takes
/// the arithmetic answer, so the trigger is the cell's own pattern and not the sheet's.
/// </para>
/// <para>
/// The corpus cost of missing them is two whole documents of the converted <c>.ods</c> column.
/// <c>hdss-bulletin-index-2019-2022.ods</c> is a table of hyperlinks: 26.2.4.2 gives its rows
/// 1–200 <strong>298</strong> twips and row 0 — its header, the one row holding no
/// <c>text:a</c> — 276, and rewriting every <c>&lt;text:a&gt;</c> to its own text makes all 201 of
/// them 276. <c>Special-Procedures_2025-07-10.ods</c> holds no hyperlink and five conditional
/// formats over whole columns, and deleting its <c>calcext:conditional-formats</c> element moves
/// its rows 6–200 from 298 to 276 the same way. Both were three and one pages short of the
/// reference before this and are page-exact after it.
/// </para>
/// </remarks>
public sealed class SheetConditionalRowHeightTests
{
    [Theory]
    [InlineData(0, 276)]  // plain: the arithmetic answer, trunc(220 x 1.18) + 40 - 23
    [InlineData(1, 298)]  // a hyperlink makes the cell one field, so it is measured
    [InlineData(2, 298)]  // a conditional format clears bStdOnly, so it is measured
    [InlineData(3, 276)]  // and the row after it is not, which is the discriminating control
    public void ACellCalcMeasuresTakesAnEditEngineLineRatherThanTheArithmeticHeight(
        int row, int twips)
    {
        Rows().SizeAt(row).Twips.ShouldBe(twips);
    }

    /// <summary>The recomputed heights of the fixture's rows.</summary>
    private static SheetAxis Rows()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-conditional-row-height.fods"));

        return ((SpreadsheetPages)document.Layout()).Sheets[0].Grid.Rows;
    }
}
