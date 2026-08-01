using System.Xml.Linq;
using Paperless.Spreadsheets.Layout;
using Paperless.Spreadsheets.Ooxml;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// What a worksheet that states nothing is read as.
/// </summary>
/// <remarks>
/// <para>
/// Both defaults here are invisible on every workbook LibreOffice wrote, because it writes
/// <c>defaultColWidth</c> and a full <c>pageSetup</c> on every sheet, and both decide the page
/// count of a workbook Excel wrote — which is most of the world's. They are asserted against
/// <c>XlsxPrintSetup</c> directly rather than through a package: the whole question is what an
/// absent attribute means, so the smallest possible sheet is the clearest statement of it.
/// </para>
/// </remarks>
public sealed class XlsxSheetDefaultsTests
{
    private const string Namespace = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";

    [Fact]
    public void ASheetStatingNoColumnWidthTakesBaseColWidthAndNotCalcsOwnDefault()
    {
        (_, SheetGrid grid) = Read("<sheetFormatPr defaultRowHeight=\"15\"/>");

        // Eight digits of the default font plus five screen pixels of padding —
        // `#i3006# add 5 pixels padding to the width`, worksheethelper.cxx:745 — which is
        // 8 × 111 + 75 twips. Calc's own default column is 1280 twips, and reading the absent
        // attribute as that fits seven columns to a page where LibreOffice fits ten.
        grid.Columns.DefaultSize.Twips.ShouldBe(963);
    }

    [Fact]
    public void AStatedDefaultColumnWidthWinsOverTheBaseWidth()
    {
        (_, SheetGrid grid) = Read(
            "<sheetFormatPr baseColWidth=\"8\" defaultColWidth=\"11.53515625\" defaultRowHeight=\"12.8\"/>");

        // setDefaultColumnWidth "overrides a width set with setBaseColumnWidth()", and it carries
        // no padding: 11.53515625 × 111 rounded.
        grid.Columns.DefaultSize.Twips.ShouldBe(1280);
    }

    [Fact]
    public void ASheetStatingNoPageSetupKeepsTheApplicationsOwnPaper()
    {
        (SheetPrintSetup setup, _) = Read("<sheetFormatPr defaultRowHeight=\"15\"/>");

        // PageSettingsModel::mbValidSettings starts *true* and only importPageSetup lowers it, and
        // the paper size is written onto the page style only when it is false — so an absent
        // pageSetup leaves Calc's locale default standing rather than applying paperSize's own
        // default of 1, which is Letter. Every chart workbook in chart2/qa/extras/data/xlsx states
        // no pageSetup, and LibreOffice renders all of them A4.
        setup.PageSize.Width.Millimetres.ShouldBe(210, 0.5);
        setup.PageSize.Height.Millimetres.ShouldBe(297, 0.5);
    }

    [Fact]
    public void AStatedPageSetupAppliesItsPaperIndex()
    {
        (SheetPrintSetup setup, _) = Read("<pageSetup paperSize=\"1\" orientation=\"portrait\"/>");

        // Index 1 is Letter, and a stated pageSetup is what makes the index count at all.
        setup.PageSize.Width.Inches.ShouldBe(8.5, 0.01);
        setup.PageSize.Height.Inches.ShouldBe(11, 0.01);
    }

    [Fact]
    public void UsePrinterDefaultsPutsThePaperBackToTheApplicationsOwn()
    {
        (SheetPrintSetup setup, _) = Read(
            "<pageSetup paperSize=\"1\" usePrinterDefaults=\"true\"/>");

        setup.PageSize.Width.Millimetres.ShouldBe(210, 0.5);
    }

    private static (SheetPrintSetup Setup, SheetGrid Grid) Read(string body)
        => XlsxPrintSetup.Read(
            XElement.Parse($"<worksheet xmlns=\"{Namespace}\">{body}</worksheet>"),
            [], null, null);
}
