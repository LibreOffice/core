using XRay.Office.Core.Documents;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A row is measured across the columns the sheet <em>allocates</em>, not the ones its print
/// area reaches.
/// </summary>
/// <remarks>
/// <para>
/// <c>ScTable::aCol</c> holds only the columns that have been allocated, and anything applying a
/// pattern to a column allocates it — a cell that states a format and holds no value exactly as
/// much as one holding a value (<c>ScTable::ApplyPatternArea</c>,
/// <c>sc/source/core/data/table2.cxx</c>:2980-2999).
/// <c>GetOptimalHeightsInColumn</c> (<c>table1.cxx</c>:88-127) then walks every allocated column,
/// and <c>ScColumn::GetOptimalHeight</c> (<c>column2.cxx</c>:899-1110) writes the pattern's
/// arithmetic height over the whole range each pattern covers. A column holding nothing has the
/// sheet's default pattern over every row, so <strong>one formatted blank cell anywhere makes the
/// default cell style's font a floor for every row of the sheet.</strong>
/// </para>
/// <para>
/// The exception that keeps this from being 16383 on every file Calc writes is in the same
/// function: a pattern applied out to the sheet's last column allocates nothing, which is exactly
/// the <c>table:table-cell table:style-name="Default"</c> repeat that pads every row of an
/// <c>.ods</c>. See <see cref="SheetCellFormats.LastAllocatedColumn"/>.
/// </para>
/// <para>
/// Measured on <c>Aviation_Abbreviations.ods</c>, whose <c>Sheet1</c> carries one empty
/// <c>ce57</c> cell in column C of row 681 and whose cells are 9 pt against an 11 pt document
/// default: every row of it is 276 twips in 26.2.4.2 and was 256 here, which is 80 pages against
/// 94. Truncating that sheet just above the empty cell makes both renderers say 256, and just
/// below it makes both say 276.
/// </para>
/// <para>
/// The fixture's numbers are 26.2.4.2's own, read from its flat-ODF export of the file.
/// </para>
/// </remarks>
public sealed class SheetRowHeightAllocatedColumnTests
{
    private static SheetLayout Sheet(int index)
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require("sheet-row-height-allocated-column.fods"));

        return ((SpreadsheetPages)document.Layout()).Sheets[index];
    }

    /// <summary>
    /// A column brought into existence by a formatted blank cell is measured for every row.
    /// </summary>
    [Fact]
    public void AFormattedBlankCellMakesTheDefaultPatternAFloor()
    {
        SheetAxis rows = Sheet(0).Grid.Rows;

        // 20 pt: trunc(400 x 1.18) = 472, plus 40 twips of margin, less 23.
        for (int row = 0; row < 4; row++)
        {
            rows.SizeAt(row).Twips.ShouldBe(
                489, $"LibreOffice writes 0.3398in for row {row + 1} of S1");
        }

        // The last row states a format in all three columns, so nothing in it falls through to the
        // default at all and its own 9 pt asks for less than the sheet's minimum.
        rows.SizeAt(4).Twips.ShouldBe(256, "LibreOffice writes 0.178in for row 5 of S1");
    }

    /// <summary>The same sheet without that cell, where the third column never exists.</summary>
    [Fact]
    public void AColumnNothingAllocatesIsNotMeasured()
    {
        SheetAxis rows = Sheet(1).Grid.Rows;

        for (int row = 0; row < 5; row++)
        {
            rows.SizeAt(row).Twips.ShouldBe(
                256, $"LibreOffice writes 0.178in for row {row + 1} of S2");
        }
    }
}
