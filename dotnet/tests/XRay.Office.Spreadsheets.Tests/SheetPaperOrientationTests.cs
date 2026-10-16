using System.Xml.Linq;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.Spreadsheets.Ooxml;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// An orientation only turns a paper the application recognises.
/// </summary>
/// <remarks>
/// <para>
/// Both Excel formats state the paper as an index into a table Windows defines, and real files
/// carry indices no table holds — a printer driver's own sizes start where the DMPAPER
/// enumeration stops, at 118. The interesting part is what LibreOffice does with one: it writes
/// no size onto the page style at all, so the locale default is left standing, and it is left
/// standing <em>in portrait</em>. The <c>orientation</c> attribute is discarded along with the
/// size rather than applied to the fallback.
/// </para>
/// <para>
/// Measured on the installed 26.2.4.2 rather than read out of the source tree, by rendering a
/// one-cell probe workbook at every index from 0 to 135 with <c>orientation="landscape"</c> and
/// reading the page box out of the PDF. Every index it resolves swaps — 9 gives 841.89 x 595.30
/// and 8 gives 1190.55 x 841.89 — and every index it does not resolve renders 595.304 x 841.89,
/// A4 portrait, having asked for landscape.
/// </para>
/// <para>
/// <c>ODs-February-2022-Airbus-Commercial-Aircraft.xlsx</c> is what this costs. Eight of its
/// thirteen sheets state <c>paperSize="121"</c> with <c>orientation="landscape"</c>; swapping the
/// fallback put 143 of its pages on their side and paginated it at 154 pages against 175, on the
/// same five A3 pages either side.
/// </para>
/// </remarks>
public sealed class SheetPaperOrientationTests
{
    private const string Namespace = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";

    private static readonly Length A4Width = Length.FromTwips(11906);
    private static readonly Length A4Height = Length.FromTwips(16838);

    [Fact]
    public void AnIndexTheTableHoldsIsTurnedByTheOrientation()
    {
        // Index 9 is A4 and index 8 is A3, and both swap: the reference renders them
        // 841.89 x 595.30 and 1190.55 x 841.89.
        ExcelPaperSizes.Page(9, landscape: true).ShouldBe(new DocSize(A4Height, A4Width));
        ExcelPaperSizes.Page(9, landscape: false).ShouldBe(new DocSize(A4Width, A4Height));

        DocSize a3 = ExcelPaperSizes.Page(8, landscape: true);
        a3.Width.ShouldBeGreaterThan(a3.Height);
    }

    /// <summary>
    /// An index past the table takes the application's paper and loses the orientation with it.
    /// </summary>
    /// <remarks>
    /// 121 is Airbus's. 0 means "not stated" and the table's own zeroth entry is a pair of zeroes
    /// for that reason. The rest are indices the reference was measured to leave unresolved.
    /// </remarks>
    [Theory]
    [InlineData(121)]
    [InlineData(0)]
    [InlineData(48)]
    [InlineData(71)]
    [InlineData(91)]
    [InlineData(118)]
    [InlineData(135)]
    [InlineData(-1)]
    public void AnIndexPastTheTableIsNotTurned(int index)
        => ExcelPaperSizes.Page(index, landscape: true)
                          .ShouldBe(new DocSize(A4Width, A4Height));

    [Fact]
    public void TheDefaultPaperIsPortraitWhateverTheFileAsked()
        => ExcelPaperSizes.Default.ShouldBe(new DocSize(A4Width, A4Height));

    [Fact]
    public void TryPortraitReportsWhetherTheTableKnewTheIndex()
    {
        ExcelPaperSizes.TryPortrait(9, out (Length Width, Length Height) a4).ShouldBeTrue();
        a4.ShouldBe((A4Width, A4Height));

        // Still A4, but the caller now knows not to turn it.
        ExcelPaperSizes.TryPortrait(121, out (Length Width, Length Height) unknown).ShouldBeFalse();
        unknown.ShouldBe((A4Width, A4Height));
    }

    [Fact]
    public void AirbusLandscapeOnAnUnknownPaperStaysPortrait()
    {
        // Verbatim from `xl/worksheets/sheet5.xml` of
        // ODs-February-2022-Airbus-Commercial-Aircraft.xlsx.
        SheetPrintSetup setup =
            Read("<pageSetup paperSize=\"121\" orientation=\"landscape\"/>");

        setup.PageSize.ShouldBe(new DocSize(A4Width, A4Height));
    }

    [Fact]
    public void TheSameWorkbooksA3SheetsAreStillTurned()
    {
        // Sheets 3, 4 and 7 state index 8, which the table does hold — and the reference's five
        // A3 pages are landscape, so these must not be caught by the rule above.
        SheetPrintSetup setup = Read("<pageSetup paperSize=\"8\" orientation=\"landscape\"/>");

        setup.PageSize.Width.ShouldBeGreaterThan(setup.PageSize.Height);
        setup.IsLandscape.ShouldBeTrue();
    }

    /// <summary>
    /// Deferring to the printer discards the paper and the orientation together.
    /// </summary>
    /// <remarks>
    /// And it does so even for an index LibreOffice resolves perfectly well: measured,
    /// <c>usePrinterDefaults="1"</c> alongside <c>paperSize="8"</c> or <c>"9"</c> and
    /// <c>orientation="landscape"</c> renders A4 portrait in all three combinations.
    /// </remarks>
    [Theory]
    [InlineData("<pageSetup paperSize=\"9\" orientation=\"landscape\" usePrinterDefaults=\"1\"/>")]
    [InlineData("<pageSetup paperSize=\"8\" orientation=\"landscape\" usePrinterDefaults=\"1\"/>")]
    [InlineData("<pageSetup orientation=\"landscape\" usePrinterDefaults=\"1\"/>")]
    public void APrinterDefaultKeepsTheApplicationsOwnPortraitPaper(string pageSetup)
        => Read(pageSetup).PageSize.ShouldBe(new DocSize(A4Width, A4Height));

    [Fact]
    public void AnExplicitMeasureIsStillTurned()
    {
        // paperWidth/paperHeight are a real measurement rather than an index, so there is nothing
        // for the table to fail to resolve and the orientation applies normally.
        SheetPrintSetup setup = Read(
            "<pageSetup paperWidth=\"100mm\" paperHeight=\"200mm\" orientation=\"landscape\"/>");

        setup.PageSize.Width.ShouldBe(Length.FromMillimetres(200));
        setup.PageSize.Height.ShouldBe(Length.FromMillimetres(100));
    }

    private static SheetPrintSetup Read(string body)
    {
        (SheetPrintSetup setup, _) = XlsxPrintSetup.Read(
            XElement.Parse($"<worksheet xmlns=\"{Namespace}\">{body}</worksheet>"),
            [], null, null, null);
        return setup;
    }
}
