using Paperless.Core.Extraction;
using Paperless.Spreadsheets.Layout;
using Paperless.Spreadsheets.MsBinary;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// What widens a sheet's printed block beyond the cells it holds, and what does not.
/// </summary>
/// <remarks>
/// <para>
/// A string too wide for its column spills into the empty cells beside it and Calc prints all of
/// it, so the printed block is wider than the block that holds cells
/// (<c>ScTable::ExtendPrintArea</c>, <c>sc/source/core/data/table1.cxx:2127</c>). A cell that
/// <em>wraps</em> does the opposite: it folds the string into lines inside its own column and
/// needs nothing beyond it, which <c>ScColumn::GetNeededSize</c> states as
/// <c>if ( bWidth &amp;&amp; bBreak ) return 0;</c> (<c>column2.cxx:226</c>).
/// </para>
/// <para>
/// The distinction is worth a test of its own because it is the difference between a page and a
/// hundred. A wrapping column is usually a wide column of prose, so its strings measure to
/// thousands of points; extending the print area by that much adds column after column of empty
/// sheet, and every one of them becomes a band of pages holding nothing. Measured on
/// <c>AFS-400_Contacts.xlsx</c>, whose seven contact columns include three that wrap: 340 pages
/// against LibreOffice's 48, of which 289 of ours were blank and none of LibreOffice's were.
/// </para>
/// </remarks>
public sealed class SheetTextOverflowTests
{
    private static SheetLayout Sheet(string text, SheetCellFormat format)
    {
        ContentTable table = new();
        ContentTableRow row = new() { Index = 0 };
        ContentTableCell cell = new() { Row = 0, Column = 0, Value = text };
        ContentParagraph paragraph = new();
        paragraph.Children.Add(new ContentRun { Text = text });
        cell.Children.Add(paragraph);
        row.Children.Add(cell);
        table.Children.Add(row);

        SheetCellFormats.Builder formats = new();
        formats.SetCell(0, 0, formats.Intern(format));

        return new SheetLayout { Name = "Sheet1", Cells = table, Formats = formats.Build() };
    }

    [Fact]
    public void ALongStringWidensThePrintedBlockAndAWrappedOneDoesNot()
    {
        string prose = new('W', 400);

        SheetLayout spilling = Sheet(prose, SheetCellFormat.Default);
        SheetLayout wrapping = Sheet(prose, new SheetCellFormat { Wraps = true });

        // Both hold one cell in column A, so the used range is one column wide either way.
        spilling.UsedRange.LastColumn.ShouldBe(0);
        wrapping.UsedRange.LastColumn.ShouldBe(0);

        // Four hundred characters of ten-point text is far wider than one default column, so the
        // spilling sheet prints columns to the right of the one it occupies.
        spilling.PrintedRange.LastColumn.ShouldBeGreaterThan(4);

        // The wrapping sheet prints exactly the column it occupies. Justified text breaks for the
        // same reason and is covered by the same rule.
        wrapping.PrintedRange.LastColumn.ShouldBe(0);
        Sheet(prose, new SheetCellFormat { Horizontal = SheetHorizontalAlignment.Justify })
            .PrintedRange.LastColumn.ShouldBe(0);
    }
}

/// <summary>
/// What a BIFF <c>SETUP</c> record means when it says its own values are not the document's.
/// </summary>
/// <remarks>
/// <c>EXC_SETUP_INVALID</c> is usually described as covering the paper size and the orientation,
/// and LibreOffice reads it as covering the scale too — one assignment sets both flags
/// (<c>mbValidPaper = maData.mbValid = !get_flag(nFlags, EXC_SETUP_INVALID)</c>,
/// <c>sc/source/filter/excel/xipage.cxx:68</c>) and <c>ATTR_PAGE_SCALE</c> is written only under
/// <c>else if (maData.mbValid)</c> (<c>:274-276</c>). It is not a corner case: a quarter of the
/// corpus's <c>.xls</c> files set the bit, and the scale beside it is arbitrary — 255, 285, 300,
/// once 20480 — so honouring it multiplies the whole sheet. <c>P1636e.xls</c> states 285 and
/// renders on twelve pages against LibreOffice's two.
/// </remarks>
public sealed class XlsSheetSetupTests
{
    private static SheetPrintSetup Setup(int scale, ushort flags)
    {
        XlsSheetPrintState state = new();
        state.SetSetup(
            paperSize: 9, scale: scale, startPage: 1, fitToWidth: 1, fitToHeight: 1,
            flags: flags, headerMargin: 0.5, footerMargin: 0.5);
        return state.ToSetup();
    }

    [Fact]
    public void AValidSetupsScaleIsApplied()
    {
        // EXC_SETUP_PORTRAIT alone: the record is the document's own.
        SheetPrintSetup setup = Setup(75, 0x0002);

        setup.ScaleMode.ShouldBe(PrintScaleMode.Percentage);
        setup.ScalePercentage.ShouldBe(75);
    }

    [Fact]
    public void AnInvalidSetupsScaleIsIgnoredAlongWithItsPaper()
    {
        // EXC_SETUP_INVALID set: paper, orientation and scale all came from a printer.
        SheetPrintSetup setup = Setup(285, 0x0006);

        setup.ScalePercentage.ShouldBe(100);
        setup.IsLandscape.ShouldBeFalse();

        // A4, which is what the page style holds before any file is read.
        setup.PageSize.Width.Millimetres.ShouldBe(210, 0.5);
    }
}
