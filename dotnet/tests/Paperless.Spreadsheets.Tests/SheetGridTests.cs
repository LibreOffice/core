using Paperless.Core.Units;
using Paperless.Spreadsheets.Layout;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// Tests the run-length axes and the paginator directly, on sheets built by hand.
/// </summary>
/// <remarks>
/// The corpus tests prove the readers agree with LibreOffice; these prove the cases a corpus
/// document cannot reach cheaply — a print range covering a million rows, an axis of runs that
/// overlap, a sheet with nothing in it. Each of them is a page count that would otherwise only
/// be found by a caller.
/// </remarks>
public class SheetGridTests
{
    private static SheetAxis Uniform(Length size) => new(size);

    [Fact]
    public void AnAxisAnswersFromItsRunsAndFallsBackToItsDefault()
    {
        SheetAxis axis = new(Length.FromTwips(1000),
        [
            new SheetSizeRun(2, 4, Length.FromTwips(500), IsHidden: false),
            new SheetSizeRun(7, 7, Length.FromTwips(300), IsHidden: true),
        ]);

        axis.SizeAt(0).Twips.ShouldBe(1000);
        axis.SizeAt(3).Twips.ShouldBe(500);
        axis.SizeAt(9).Twips.ShouldBe(1000);

        // A hidden entry keeps its size and contributes none of it, which is the distinction
        // pagination turns on: Calc reads the width and then discards it for a hidden column.
        axis.SizeAt(7).Twips.ShouldBe(300);
        axis.PrintedSizeAt(7).ShouldBe(Length.Zero);
        axis.IsHidden(7).ShouldBeTrue();
    }

    [Fact]
    public void ALaterRunWinsAnOverlapAndNeighboursThatAgreeAreMerged()
    {
        // Both formats write a default run across the sheet and then narrower ones over the top
        // of it, so resolving overlap in the axis is what keeps every reader from having to.
        SheetAxis axis = new(Length.FromTwips(1000),
        [
            new SheetSizeRun(0, 9, Length.FromTwips(400), IsHidden: false),
            new SheetSizeRun(3, 4, Length.FromTwips(900), IsHidden: false),
        ]);

        axis.SizeAt(2).Twips.ShouldBe(400);
        axis.SizeAt(3).Twips.ShouldBe(900);
        axis.SizeAt(5).Twips.ShouldBe(400);
        axis.Runs.Count.ShouldBe(3);
    }

    [Fact]
    public void ATotalOverARangeIsWalkedByRunRatherThanByIndex()
    {
        // A repeated band's size is summed once per page, and a sheet has a million rows, so
        // this has to cost the number of runs and not the number of rows.
        SheetAxis axis = new(Length.FromTwips(256));

        axis.TotalPrintedSize(0, 999_999).Twips.ShouldBe(256L * 1_000_000);
    }

    [Fact]
    public void ASheetWithNothingInItPrintsNoPageAtAll()
    {
        // Measured: a two-sheet document whose second sheet is empty converts to a one-page PDF.
        // An empty sheet is not a blank page.
        SheetLayout sheet = new() { Name = "Empty" };

        SheetPagination.Paginate(sheet.Setup, sheet.Grid, sheet.UsedRange).ShouldBeEmpty();
    }

    [Fact]
    public void AWholeColumnPrintRangeIsCutBackToWhatTheSheetHolds()
    {
        // "A:D" covers a million rows. Paginating it literally would give a four-column sheet
        // twenty thousand blank pages, which is why Calc re-searches an axis whose range spans
        // the whole sheet.
        SheetPrintSetup setup = SheetPrintSetup.Default with
        {
            PrintAreas = [new SheetRange(0, 0, 3, SheetAddress.MaxRow)],
        };

        SheetGrid grid = new(Uniform(Length.FromTwips(1440)), Uniform(Length.FromTwips(256)));
        IReadOnlyList<SheetPagePlacement> pages =
            SheetPagination.Paginate(setup, grid, new SheetRange(0, 0, 3, 19));

        pages.Count.ShouldBe(1);
        pages[0].Cells.LastRow.ShouldBe(19);
    }

    [Fact]
    public void ARepeatedRowBandCostsEveryPageBelowItItsHeight()
    {
        // Eleven rows of 1300 twips fill an A4 page with two-centimetre margins, so twenty-two
        // of them take exactly two pages. Repeating the first row takes 1300 twips off every
        // page after the first, and the same twenty-two rows then need three. So the band
        // changes the page count, which is why it belongs in the break loop and not in the
        // drawing.
        Length rowHeight = Length.FromTwips(1300);
        SheetGrid grid = new(Uniform(Length.FromTwips(1440)), Uniform(rowHeight));

        SheetPrintSetup plain = SheetPrintSetup.Default;
        SheetPrintSetup repeating = plain with { RepeatRows = new SheetRange(0, 0, 0, 0) };

        SheetRange used = new(0, 0, 0, 21);
        int without = SheetPagination.Paginate(plain, grid, used).Count;
        int with = SheetPagination.Paginate(repeating, grid, used).Count;

        without.ShouldBe(2);
        with.ShouldBe(3);
    }

    [Fact]
    public void AManualBreakIsIgnoredOnTheAxisTheSheetIsBeingFittedAlong()
    {
        // fdo#40788 and i#54993: honouring both is contradictory, because the break fixes a page
        // count the scaling is trying to choose. Fitting to width suppresses only the column
        // breaks, so a row break still splits the sheet.
        SheetGrid grid = new(Uniform(Length.FromTwips(1440)), Uniform(Length.FromTwips(256)));
        SheetRange used = new(0, 0, 3, 19);

        SheetPrintSetup fitted = SheetPrintSetup.Default with
        {
            ScaleMode = PrintScaleMode.FitToPages,
            FitToPagesWide = 1,
            ManualColumnBreaks = [2],
            ManualRowBreaks = [10],
        };

        IReadOnlyList<SheetPagePlacement> pages = SheetPagination.Paginate(fitted, grid, used);

        pages.Count.ShouldBe(2);
        pages.ShouldAllBe(page => page.Cells.FirstColumn == 0 && page.Cells.LastColumn == 3);
        pages[1].Cells.FirstRow.ShouldBe(10);
    }

    [Theory]
    [InlineData("Sheet1!$A$1:$D$7", 0, 0, 3, 6)]
    [InlineData("Sales.$B$2:$D$10", 1, 1, 3, 9)]
    [InlineData("A1:B2", 0, 0, 1, 1)]
    [InlineData("'a:b'!$C$3", 2, 2, 2, 2)]
    public void ACellRangeParsesFromEitherFormatsSpelling(
        string text, int firstColumn, int firstRow, int lastColumn, int lastRow)
    {
        SheetAddress.TryParseRange(text, out SheetRange range).ShouldBeTrue();
        range.ShouldBe(new SheetRange(firstColumn, firstRow, lastColumn, lastRow));
    }

    [Fact]
    public void AWholeColumnReferenceSpansEveryRowAndAWholeRowEveryColumn()
    {
        // Which is what tells the two halves of a Print_Titles name apart.
        SheetAddress.TryParseRange("Sheet1!$A:$B", out SheetRange columns).ShouldBeTrue();
        columns.LastColumn.ShouldBe(1);
        columns.LastRow.ShouldBe(SheetAddress.MaxRow);

        SheetAddress.TryParseRange("Sheet1!$1:$2", out SheetRange rows).ShouldBeTrue();
        rows.LastRow.ShouldBe(1);
        rows.LastColumn.ShouldBe(SheetAddress.MaxColumn);
    }

    [Fact]
    public void ColumnLettersAreBijectiveBaseTwentySix()
    {
        // AA is 27 and not 0, which is the mistake that puts a print area in the wrong place on
        // any sheet wider than Z.
        SheetAddress.TryParseCell("AA1", out int column, out int row).ShouldBeTrue();
        column.ShouldBe(26);
        row.ShouldBe(0);

        SheetAddress.TryParseCell("XFD1048576", out column, out row).ShouldBeTrue();
        column.ShouldBe(SheetAddress.MaxColumn);
        row.ShouldBe(SheetAddress.MaxRow);
    }
}
