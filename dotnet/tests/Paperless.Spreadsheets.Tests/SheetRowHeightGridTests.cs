using System.Xml.Linq;
using Paperless.Core.Documents;
using Paperless.Spreadsheets.Layout;
using Paperless.Spreadsheets.Ooxml;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// A row height Excel wrote is read on a 0.75 pt grid, and one anybody else wrote is not.
/// </summary>
/// <remarks>
/// <para>
/// LibreOffice's OOXML filter rounds a row height <em>down</em> to a multiple of 0.75 pt —
/// <c>fHeight -= fmod(fHeight, 0.75)</c> — in two places: on <c>sheetFormatPr/@defaultRowHeight</c>
/// (<c>sc/source/filter/oox/worksheetfragment.cxx:681-684</c>) and on every <c>row/@ht</c>
/// (<c>sc/source/filter/oox/sheetdatacontext.cxx:316-319</c>). Both are gated on
/// <c>isMSODocument()</c>, which is <c>docProps/app.xml</c>'s <c>Application</c> beginning with
/// "Microsoft" and nothing else (<c>oox/source/core/xmlfilterbase.cxx:241-245</c>).
/// </para>
/// <para>
/// <strong>The two fixtures differ in that one element and in nothing else</strong>, which is
/// what makes this a test of the gate rather than of the arithmetic: a wrong implementation that
/// rounds every SpreadsheetML workbook passes every assertion about
/// <c>sheet-row-height-grid.xlsx</c> and fails on its twin. Both sets of expected twips are
/// LibreOffice 24.2's own, read out of its flat-ODF export of each fixture — <c>0.25in</c>,
/// <c>0.4063in</c>, <c>0.4165in</c>, <c>0.1665in</c>, <c>0.198in</c> against <c>0.2583in</c>,
/// <c>0.4083in</c>, <c>0.4165in</c>, <c>0.172in</c>, <c>0.2in</c>.
/// </para>
/// <para>
/// The five rows separate what the rounding touches. Rows 1, 2 and 4 state heights off the grid
/// and <c>customHeight</c>, so nothing re-measures them and the twips are the read height alone;
/// row 3 states 30 pt, which is already on the grid, and must come out the same on both sides —
/// it is the control that stops a test passing because the fixtures differ at all. Row 5 states
/// no height and takes the sheet default, which is rounded in the other place and then becomes
/// the floor its re-measured height is held up to.
/// </para>
/// </remarks>
public sealed class SheetRowHeightGridTests
{
    private const string Namespace = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";

    private static SheetLayout Sheet(string name)
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require(name));

        return ((SpreadsheetPages)document.Layout()).Sheets[0];
    }

    [Fact]
    public void ExcelsRowHeightsAreSnappedDownToTheGrid()
    {
        SheetAxis rows = Sheet("sheet-row-height-grid.xlsx").Grid.Rows;

        // 18.6 pt is read as 18.0, 29.4 as 29.25, 12.4 as 12.0.
        rows.SizeAt(0).Twips.ShouldBe(360, "LibreOffice writes 0.25in for this row");
        rows.SizeAt(1).Twips.ShouldBe(585, "LibreOffice writes 0.4063in for this row");
        rows.SizeAt(3).Twips.ShouldBe(240, "LibreOffice writes 0.1665in for this row");
    }

    [Fact]
    public void TheSheetDefaultIsSnappedDownAndBecomesTheFloor()
    {
        SheetLayout sheet = Sheet("sheet-row-height-grid.xlsx");

        // defaultRowHeight 14.4 is read as 14.25, which is 285 twips. Row 5 states no height, so
        // it takes that, is re-measured, and is held back up to the floor the same number sets.
        sheet.Grid.OptimalMinimumRowHeight.Twips.ShouldBe(285);
        sheet.Grid.Rows.SizeAt(4).Twips.ShouldBe(285, "LibreOffice writes 0.198in for this row");
    }

    [Fact]
    public void AWorkbookNobodyMicrosoftWroteKeepsItsStatedHeights()
    {
        SheetLayout sheet = Sheet("sheet-row-height-grid-other.xlsx");
        SheetAxis rows = sheet.Grid.Rows;

        // The same bytes in the same sheet, read whole because the generator is not Microsoft.
        rows.SizeAt(0).Twips.ShouldBe(372, "LibreOffice writes 0.2583in for this row");
        rows.SizeAt(1).Twips.ShouldBe(588, "LibreOffice writes 0.4083in for this row");
        rows.SizeAt(3).Twips.ShouldBe(248, "LibreOffice writes 0.172in for this row");

        sheet.Grid.OptimalMinimumRowHeight.Twips.ShouldBe(288);
        rows.SizeAt(4).Twips.ShouldBe(288, "LibreOffice writes 0.2in for this row");
    }

    [Fact]
    public void AHeightAlreadyOnTheGridIsUntouchedOnBothSides()
    {
        // 30 pt is 40 x 0.75, so the two fixtures must agree about it. Without this the pair
        // would also pass an implementation that rounded every height to something.
        Sheet("sheet-row-height-grid.xlsx").Grid.Rows.SizeAt(2).Twips.ShouldBe(600);
        Sheet("sheet-row-height-grid-other.xlsx").Grid.Rows.SizeAt(2).Twips.ShouldBe(600);
    }

    [Fact]
    public void TheRoundingLandsOnAnExactMultipleRatherThanAUnitBelowIt()
    {
        // `h - h % 0.75` is exact in binary floating point where `floor(h / 0.75) * 0.75` is
        // free to land an ulp low; 29.4 is the case that separates them, at 585 twips against
        // 584. Asserted against the reader rather than the fixture so that it says which
        // arithmetic is meant.
        (_, SheetGrid grid) = XlsxPrintSetup.Read(
            XElement.Parse(
                $"<worksheet xmlns=\"{Namespace}\"><sheetFormatPr defaultRowHeight=\"15\"/>"
                + "<sheetData><row r=\"1\" ht=\"29.4\" customHeight=\"1\"/></sheetData></worksheet>"),
            [], null, null, null, isMicrosoftGenerated: true);

        grid.Rows.SizeAt(0).Twips.ShouldBe(585);
    }
}
