using XRay.Office.Core.Documents;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// An ODF conditional format whose style states no font replaces the cell's font anyway, with the
/// <c>Default</c> cell style's — and only where the condition actually holds.
/// </summary>
/// <remarks>
/// <para>
/// A conditional format applies a cell <em>style sheet</em>, and
/// <c>ScPatternAttr::fillFontOnly</c> takes every font item from it through
/// <c>lcl_populateresult</c> (<c>sc/source/core/data/patattr.cxx:608-637</c> in the reference C++
/// checkout), which asks <c>pCondSet-&gt;GetItemIfSet(nWhich)</c> — and that helper's
/// <c>bSrchInParent</c> defaults to <c>true</c> (<c>include/svl/itemset.hxx:201</c>). The search
/// therefore does not stop at the conditional style: it walks the parents, and every parentless
/// Calc cell style is re-parented to <c>Default</c>. So a conditional style stating only a colour
/// hands back <c>Default</c>'s face and size, and they beat the cell's own.
/// </para>
/// <para>
/// <strong>SpreadsheetML is not affected</strong>: <c>StylesBuffer::createDxfStyle</c> calls
/// <c>rStyleSheet.ResetParent()</c> before filling the style from the <c>&lt;dxf&gt;</c>
/// (<c>sc/source/filter/oox/stylesbuffer.cxx:3225-3234</c>), so there is no parent to search and a
/// silent rule really does leave the cell's font alone. Measured on one workbook in both
/// spellings: <c>sistem-rekod-markah-srm-_-rekod-master</c> read as <c>.ods</c> draws its
/// <c>#N/A</c> cells in <strong>Carlito-Regular 11.0</strong> and read as <c>.xlsx</c> draws them
/// in <strong>LiberationSans 9.01</strong>, while its neighbouring <c>E</c> cells — whose
/// condition does not hold — are LiberationSans 9.01 in both.
/// </para>
/// <para>
/// The fixture is four rows of Arial 9 in a <c>Default</c> of Calibri 40, so the two candidate
/// fonts are 31 points apart and nothing can fit both. Read out of 26.2.4.2's own
/// <c>--convert-to fods</c> — <c>style:row-height</c>, the number Calc computed rather than
/// anything measured off a page — it answers <strong>256.3, 256.3, 984.8, 984.8</strong> twips.
/// </para>
/// <para>
/// The two rows that do not move are the discriminating controls, and they are the half an
/// earlier attempt at this rule got wrong. Row 1 carries no conditional format. Row 2 carries one
/// whose condition is <c>&lt; 40</c> against the string <c>E</c>, and
/// <c>ScConditionEntry::IsValidStr</c> returns false outright for a numeric operand against a
/// string cell unless the operator is <em>not equal</em> (<c>conditio.cxx:1181-1183</c>) — so no
/// entry matches, <c>GetCondResult</c> hands <c>GetNeededSize</c> a null item set, and the cell is
/// measured in its own 9 pt.
/// </para>
/// <para>
/// Row 4 is the other end of it: its formula result is the error <c>#N/A</c>, and
/// <c>lcl_GetCellContent</c> (<c>conditio.cxx:766-800</c>) yields a <em>number</em> for any
/// formula cell whose result <c>IsValue()</c> — an error is a value, and it is zero. So the error
/// cells satisfy <c>&lt; 40</c> where the text cells beside them do not, which is exactly how the
/// witness document's six <c>#N/A</c> columns per row come to be drawn at 11 pt while its
/// <c>E</c> columns stay at 9.
/// </para>
/// </remarks>
public sealed class SheetConditionalFontSourceTests
{
    [Theory]
    [InlineData(0, 256)]  // no conditional format at all: the cell's own 9 pt, on the floor
    [InlineData(1, 256)]  // "E" < 40 is false for a string cell, so the cell's own 9 pt again
    [InlineData(2, 985)]  // 5 < 40 holds: one line of Default's 40 pt, not of the cell's 9
    [InlineData(3, 985)]  // an error result is the number zero, so it holds too
    public void AFiringConditionMeasuresTheCellInTheStyleItAppliesRatherThanTheCellsOwnFont(
        int row, int twips)
    {
        Rows().SizeAt(row).Twips.ShouldBe(twips);
    }

    /// <summary>The recomputed heights of the fixture's rows.</summary>
    private static SheetAxis Rows()
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-conditional-font-source.fods"));

        return ((SpreadsheetPages)document.Layout()).Sheets[0].Grid.Rows;
    }
}
