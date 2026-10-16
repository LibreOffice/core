using System.Xml.Linq;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.Spreadsheets.Ooxml;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A chart sheet's scale, orientation and grid are decided by what the sheet is, not by what its
/// <c>pageSetup</c> states.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Measured on LibreOffice 24.2.7.2 and 26.2.4.2, which agree here.</strong> Found by
/// grouping the corpus gate's mismatches by cause: the corpus holds exactly <strong>two</strong>
/// workbooks carrying an <c>xl/chartsheets/</c> part — <c>062_Run_chart_cb7476ea.xlsx</c> and
/// <c>057_Simple_balance_sheet_Use_this_template_e2d4cbb2.xlsx</c> — and <strong>both</strong>
/// were failing the gate, each on page count, each printing exactly one page more than the
/// reference with a sliver of the chart's rotated category labels on it.
/// </para>
/// <para>
/// The rule is <c>PageSettingsConverter::writePageSettingsProperties</c>
/// (<c>sc/source/filter/oox/pagesettings.cxx:905-972</c>), which branches on
/// <c>eSheetType == WorksheetType::Chart</c> three times:
/// </para>
/// <list type="bullet">
///   <item><description>
///     <c>ScaleToPages = 1</c> — "always fit chart sheet to 1 page" (<c>:910-914</c>). It is
///     unconditional: a chart sheet's own <c>scale</c> and <c>fitToPage</c> never reach the
///     printout.
///   </description></item>
///   <item><description>
///     landscape unless the file explicitly says otherwise — "chart sheets default to landscape"
///     (<c>:931-932</c>).
///   </description></item>
///   <item><description>
///     <c>PrintGrid</c> and <c>PrintHeaders</c> forced off — "no gridlines in chart sheets"
///     (<c>:971-972</c>).
///   </description></item>
/// </list>
/// <para>
/// <strong>The scale is the half with teeth, and the mechanism is geometric.</strong> A chart
/// sheet's chart arrives as an <c>xdr:absoluteAnchor</c> with an explicit extent —
/// 8656320 × 6278880 EMU, or 681.6 × 494.4 pt, on <c>057</c> — and its rotated category labels
/// hang outside that box. Printed at 100 % on a 792 × 612 pt page with 0.7 in side margins the
/// overhang crosses the right edge of the printable area, so the sheet paginates into a second
/// page column carrying nothing but the tail of two labels. Fitting to one page removes it.
/// </para>
/// <para>
/// <strong>Measured reach, whole corpus, at <c>2f4709c08</c>:</strong> <c>062</c> goes
/// <c>pages,words</c> → <c>match</c> (3 pages → 2 against the reference's 2; 680 glyphs → 643
/// against 645). <c>057</c> goes <c>pages,words</c> → <c>words</c> (4 → 3 against 3). Its
/// residual is not this defect and cannot be closed by drawing less: the reference draws that
/// chart's rotated category labels as vector outlines with no text layer behind them — its chart
/// page yields 112 alphanumeric characters to <c>pdftotext</c> against our 398 — so ours is the
/// searchable output and the word gate scores it as the failure. That is the outlining ceiling
/// <c>TODO.raster-ceiling.md</c> describes, not a defect of ours.
/// </para>
/// <para>
/// Detected from the part's root element name. The reader is handed the loaded sheet part and
/// nothing else, and a <c>chartsheet</c> part has a <c>chartsheet</c> root by schema, so the root
/// name and the relationship type are the same fact.
/// </para>
/// </remarks>
public sealed class SheetChartSheetPrintTests
{
    private const string Ns = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";

    private const string Margins =
        "<pageMargins left=\"0.7\" right=\"0.7\" top=\"0.75\" bottom=\"0.75\" "
        + "header=\"0.3\" footer=\"0.3\"/>";

    private static SheetPrintSetup ChartSheet(string body)
        => XlsxPrintSetup.Read(
            XElement.Parse($"<chartsheet xmlns=\"{Ns}\">{Margins}{body}</chartsheet>"),
            [], null, null).Setup;

    private static SheetPrintSetup Worksheet(string body)
        => XlsxPrintSetup.Read(
            XElement.Parse($"<worksheet xmlns=\"{Ns}\">{Margins}{body}</worksheet>"),
            [], null, null).Setup;

    /// <summary>
    /// The rule with teeth: one page, whatever the sheet's own scaling says.
    /// </summary>
    [Fact]
    public void AChartSheetIsAlwaysFittedToOnePage()
    {
        SheetPrintSetup setup = ChartSheet("<pageSetup orientation=\"landscape\"/>");

        setup.ScaleMode.ShouldBe(PrintScaleMode.FitToPageCount);
        setup.FitToPageCount.ShouldBe(1);
    }

    /// <summary>
    /// <c>scale</c> is what an ordinary sheet would be printed at, and a chart sheet ignores it.
    /// Reading it would leave the sheet at <c>PrintScaleMode.Percentage</c> and never fit.
    /// </summary>
    [Fact]
    public void AStatedPercentageScaleDoesNotSurviveOnAChartSheet()
    {
        SheetPrintSetup setup = ChartSheet("<pageSetup scale=\"60\" orientation=\"landscape\"/>");

        setup.ScaleMode.ShouldBe(PrintScaleMode.FitToPageCount);
        setup.FitToPageCount.ShouldBe(1);
        setup.ScalePercentage.ShouldBe(100);
    }

    /// <summary>
    /// Nor does <c>fitToPage</c>, which would otherwise put the sheet on
    /// <c>PrintScaleMode.FitToPages</c> with a width and a height rather than a page count.
    /// </summary>
    [Fact]
    public void AStatedFitToPageDoesNotSurviveOnAChartSheet()
    {
        SheetPrintSetup setup = ChartSheet(
            "<sheetPr><pageSetUpPr fitToPage=\"1\"/></sheetPr>"
            + "<pageSetup fitToWidth=\"2\" fitToHeight=\"3\" orientation=\"landscape\"/>");

        setup.ScaleMode.ShouldBe(PrintScaleMode.FitToPageCount);
        setup.FitToPageCount.ShouldBe(1);
        setup.FitToPagesWide.ShouldBe(0);
        setup.FitToPagesTall.ShouldBe(0);
    }

    /// <summary>A chart sheet stating no orientation prints landscape.</summary>
    [Fact]
    public void AChartSheetDefaultsToLandscape()
    {
        ChartSheet("<pageSetup/>").IsLandscape.ShouldBeTrue();
        ChartSheet(string.Empty).IsLandscape.ShouldBeTrue();
    }

    /// <summary>
    /// The default is a default, not an override: a chart sheet that names portrait gets portrait.
    /// </summary>
    [Fact]
    public void AChartSheetThatNamesPortraitKeepsIt()
    {
        ChartSheet("<pageSetup orientation=\"portrait\"/>").IsLandscape.ShouldBeFalse();
    }

    /// <summary>No gridlines and no row or column headings, however they are declared.</summary>
    [Fact]
    public void AChartSheetPrintsNeitherGridNorHeadings()
    {
        SheetPrintSetup setup = ChartSheet(
            "<printOptions gridLines=\"1\" headings=\"1\"/><pageSetup orientation=\"landscape\"/>");

        setup.PrintsGrid.ShouldBeFalse();
        setup.PrintsHeadings.ShouldBeFalse();
    }

    /// <summary>
    /// The control, and the half that matters most: an ordinary worksheet is untouched by any of
    /// it. Everything above is keyed on the root element, so a worksheet keeps its own scaling,
    /// its portrait default and its declared grid.
    /// </summary>
    [Fact]
    public void AWorksheetKeepsItsOwnScaleOrientationAndGrid()
    {
        SheetPrintSetup percentage = Worksheet(
            "<printOptions gridLines=\"1\" headings=\"1\"/><pageSetup scale=\"60\"/>");

        percentage.ScaleMode.ShouldBe(PrintScaleMode.Percentage);
        percentage.ScalePercentage.ShouldBe(60);
        percentage.FitToPageCount.ShouldBe(0);
        percentage.IsLandscape.ShouldBeFalse();
        percentage.PrintsGrid.ShouldBeTrue();
        percentage.PrintsHeadings.ShouldBeTrue();

        SheetPrintSetup fitted = Worksheet(
            "<sheetPr><pageSetUpPr fitToPage=\"1\"/></sheetPr>"
            + "<pageSetup fitToWidth=\"2\" fitToHeight=\"3\"/>");

        fitted.ScaleMode.ShouldBe(PrintScaleMode.FitToPages);
        fitted.FitToPagesWide.ShouldBe(2);
        fitted.FitToPagesTall.ShouldBe(3);
        fitted.FitToPageCount.ShouldBe(0);
    }
}
