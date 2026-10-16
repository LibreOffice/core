using XRay.Office.Core.Documents;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// An ODF sheet recalculates its automatic row heights for its first 200 rows only.
/// </summary>
/// <remarks>
/// <para>
/// A row height in a spreadsheet file is usually the writing application's own measurement rather
/// than a statement, and Calc throws it away and measures again on load — except here.
/// <c>ScXMLTableRowContext::endFastElement</c> adds each row block to the recalculation ranges and
/// then takes it straight back out when the block ends past row 200 and its style carries a
/// stored height alongside the optimal flag (<c>sc/source/filter/xml/xmlrowi.cxx</c>:215-244,
/// under the comment <em>"recalc only the first 200 row in case of optimal document loading"</em>).
/// The row indices there are zero-based, so the boundary the test below reads off 26.2.4.2 — rows
/// 0 to 200 recalculated, 201 onwards not — is <c>nCurrentRow &gt; 200</c> exactly.
/// </para>
/// <para>
/// It is an <em>importer's</em> rule and not a layout one: the BIFF and SpreadsheetML filters
/// recalculate every automatic row and have no such limit, which is why it sits in
/// <c>OdsPrintSetup</c> and reaches <see cref="SheetOptimalRowHeights"/> only as a row's optimal
/// flag being false.
/// </para>
/// <para>
/// Reach on the converted-ODF corpus is wide and the cost is pages:
/// <c>Laser Report 2024 FOIA __Oct (1).ods</c> states <c>style:row-height="0.2189in"</c> with the
/// optimal flag on every one of its rows, and printed 486 pages against the reference's 506
/// because all of them were recomputed to a shorter 15.0 pt.
/// <c>dotnet/probes/ods-resid-r80/results.md</c> has the measurement.
/// </para>
/// </remarks>
public sealed class SheetOdfRowHeightLimitTests
{
    private static SheetGrid Grid(string name)
    {
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
            Corpus.Require(name));

        return ((SpreadsheetPages)document.Layout()).Pages[0].Sheet.Grid;
    }

    /// <summary>The rows up to and including row 200 take their recomputed height.</summary>
    /// <remarks>
    /// Every row of the fixture states 0.3in — 21.6 pt — with the optimal flag, so a row that
    /// still measures 21.6 was not recalculated. 26.2.4.2 draws rows 0 to 200 on a 12.78 pt pitch.
    /// </remarks>
    [Theory]
    [InlineData(0)]
    [InlineData(1)]
    [InlineData(199)]
    [InlineData(200)]
    public void ARowInsideTheLimitIsRecalculated(int row)
        => Grid("sheet-row-height-limit.fods").Rows.SizeAt(row).Points.ShouldBeLessThan(21.0);

    /// <summary>Row 201 onwards keeps the height the file states.</summary>
    /// <remarks>
    /// The stated 0.3in is 21.6 pt, and it is what 26.2.4.2 leaves those rows at although their
    /// style asks for an optimal height and their text needs far less.
    /// </remarks>
    [Theory]
    [InlineData(201)]
    [InlineData(202)]
    [InlineData(205)]
    public void ARowPastTheLimitKeepsItsStatedHeight(int row)
        => Grid("sheet-row-height-limit.fods").Rows.SizeAt(row).Points.ShouldBe(21.6, 0.05);

    /// <summary>
    /// The boundary is exactly where the reference puts it, and not one row either side.
    /// </summary>
    /// <remarks>
    /// Stated as the step itself rather than as two heights, because that is the quantity the
    /// reference's PDF shows: the baseline gap goes 12.78, 12.78, … and then 21.60 between
    /// <c>r200</c> and <c>r201</c>.
    /// </remarks>
    [Fact]
    public void TheStepIsBetweenRowTwoHundredAndRowTwoHundredAndOne()
    {
        SheetAxis rows = Grid("sheet-row-height-limit.fods").Rows;

        rows.SizeAt(200).Points.ShouldBe(rows.SizeAt(199).Points);
        rows.SizeAt(201).Points.ShouldBeGreaterThan(rows.SizeAt(200).Points);
        rows.SizeAt(201).Points.ShouldBe(rows.SizeAt(202).Points);
    }
}
