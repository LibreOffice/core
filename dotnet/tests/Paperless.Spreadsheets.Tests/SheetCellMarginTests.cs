using Paperless.Core.Documents;
using Paperless.Core.Units;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// An <c>.xls</c> cell's margin is 40 twips; every other format's is the pool's 20.
/// </summary>
/// <remarks>
/// <para>
/// <c>ATTR_MARGIN</c> is a cell attribute whose pool default is <c>SvxMarginItem(20, 20, 20, 20)</c>
/// (<c>svx/source/items/algitem.cxx:123-132</c>, installed at <c>docpool.cxx:145</c>), and
/// <c>XclImpXF::CreatePattern</c> ends by overriding it on <em>every</em> pattern the BIFF filter
/// builds — <c>SvxMarginItem aItem(40, 40, 40, 40, ATTR_MARGIN)</c>, under the comment "Excel's
/// cell margins are different from Calc's default margins"
/// (<c>sc/source/filter/excel/xistyle.cxx:1349-1351</c>). It is the only line in
/// <c>sc/source/filter</c> that touches the item, so SpreadsheetML, XLSB and ODF keep the 20.
/// </para>
/// <para>
/// The fixture triple is what makes this measurable rather than argued: <c>sheet-cell-text</c>
/// exists as <c>.xls</c>, <c>.xlsx</c> and <c>.fods</c> holding the same cells, and LibreOffice
/// 24.2.7.2's own PDFs of the three put the left-aligned <c>Lft</c> of A1 at <b>58.68 pt</b> in
/// the <c>.xls</c> and at <b>57.69 pt</b> in the other two — 0.99 pt apart, which is what 20
/// twips comes to once <see cref="SheetDeviceUnits"/> has snapped it. The same 0.99 pt separates
/// the two baselines, in the other direction, because the cells are bottom-aligned.
/// </para>
/// <para>
/// It reaches further than placement. The same <c>nTotalMargin</c> is what
/// <c>ScOutputData::GetOutputArea</c> subtracts to decide how much of a clipped string survives
/// (<c>output2.cxx:1841</c>, <c>:2219</c>), so a column of blocked text keeps one character fewer
/// per cell than it did — measured on
/// <c>underlying-holdings-…-state-street-emu-esg-screened-index-equity-fund.xls</c>, which went
/// from 4743 extractable words against 4991 to 4988 with nothing else changed.
/// </para>
/// </remarks>
public sealed class SheetCellMarginTests
{
    /// <summary>The three renderings of one workbook, and the margin each reader states.</summary>
    [Theory]
    [InlineData("sheet-cell-text.xls", 40)]
    [InlineData("sheet-cell-text.xlsx", 20)]
    [InlineData("sheet-cell-text.fods", 20)]
    public void OnlyTheBiffReaderStatesExcelsMargin(string name, int twips)
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require(name));

        SheetLayout sheet = ((SpreadsheetPages)document.Layout()).Sheets[0];

        // Every cell of the sheet, not merely the ones that state an alignment: the BIFF filter
        // puts the item on every pattern it builds, so a cell that states nothing about its font
        // or its alignment still carries it.
        sheet.Formats.At(0, 0).Margin.ShouldBe(Length.FromTwips(twips), $"{name}: A1");
        sheet.Formats.At(2, 4).Margin.ShouldBe(Length.FromTwips(twips), $"{name}: E3");

        // The format a cell states nothing at all about resolves to the same margin, which is the
        // half a lookup would miss: `SheetCellFormat.Default` carries the pool's 20.
        sheet.Formats.SheetDefault.Margin.ShouldBe(Length.FromTwips(twips), $"{name}: default");
    }

    /// <summary>
    /// Where the four alignments put their text in the <c>.xls</c>, against LibreOffice's own PDF.
    /// </summary>
    /// <remarks>
    /// Left, centred and right in one row, so that the sign of the margin is asserted in both
    /// directions from one document: a wider margin moves a left-aligned run right and a
    /// right-aligned one left, and leaves a centred one alone. Reading only the left-aligned case
    /// cannot tell a cell margin apart from a page origin — which is exactly the reading this
    /// difference carried for several rounds, recorded in <c>pdf-ops.py</c>'s own comment as
    /// "the two put their page origin in slightly different places".
    /// </remarks>
    [Fact]
    public void ABiffCellsTextStartsTwoPointsInsideItsColumn()
    {
        IReadOnlyList<DrawnGlyphRun> runs = FirstPage("sheet-cell-text.xls");

        Run(runs, "Lft").Origin.X.Points.ShouldBe(58.68, 0.05);
        Run(runs, "Ctr").Origin.X.Points.ShouldBe(172.35, 0.05);

        // The right-aligned run's *end* is what the margin holds off the column's edge.
        DrawnGlyphRun right = Run(runs, "Rgt");
        (right.Origin.X + right.Width).Points.ShouldBe(241.76, 0.1);

        // Bottom-aligned, so the extra margin lifts the baseline: 776.35 pt up an A4 page of
        // 841.89 is 65.54 pt down from its top.
        Run(runs, "Lft").Origin.Y.Points.ShouldBe(65.54, 0.05);
    }

    /// <summary>
    /// The same cells in the same workbook written as SpreadsheetML and as flat ODF, which the
    /// override does not reach.
    /// </summary>
    /// <remarks>
    /// The control, and it is not a drift guard: it is the assertion that the 0.99 pt is the BIFF
    /// filter's line rather than something about the sheet. Both formats put <c>Lft</c> at
    /// 57.69 pt and its baseline at 66.53 pt down the page in LibreOffice's own renderings.
    /// </remarks>
    [Theory]
    [InlineData("sheet-cell-text.xlsx")]
    [InlineData("sheet-cell-text.fods")]
    public void EveryOtherFormatKeepsThePoolsMargin(string name)
    {
        IReadOnlyList<DrawnGlyphRun> runs = FirstPage(name);

        Run(runs, "Lft").Origin.X.Points.ShouldBe(57.69, 0.05);
        Run(runs, "Lft").Origin.Y.Points.ShouldBe(66.53, 0.05);

        DrawnGlyphRun right = Run(runs, "Rgt");
        (right.Origin.X + right.Width).Points.ShouldBe(242.93, 0.1);
    }

    /// <summary>
    /// An indent is added to the cell's own margin rather than replacing it.
    /// </summary>
    /// <remarks>
    /// <c>GetLeftTotal()</c> is <c>pMargin->GetLeftMargin() + nIndent</c>
    /// (<c>output2.cxx:160</c>), so an indented cell in an <c>.xls</c> carries both. D2's
    /// <c>Ind</c> sits 265.72 pt in on the <c>.xls</c> and 261.38 on the <c>.xlsx</c>, and the
    /// gap is <em>not</em> the margin alone: the three exports of this workbook state three
    /// different indents — the flat ODF puts the same cell at 264.87 — so all this pins is that
    /// the sum matches LibreOffice's own rendering of each file. It is the case that would break
    /// if the margin were dropped where an indent is stated, which is a distinct site from the
    /// unindented one above.
    /// </remarks>
    [Fact]
    public void AnIndentIsMeasuredOnTopOfTheCellsOwnMargin()
    {
        Run(FirstPage("sheet-cell-text.xls"), "Ind").Origin.X.Points.ShouldBe(265.72, 0.05);
        Run(FirstPage("sheet-cell-text.xlsx"), "Ind").Origin.X.Points.ShouldBe(261.38, 0.05);
        Run(FirstPage("sheet-cell-text.fods"), "Ind").Origin.X.Points.ShouldBe(264.87, 0.05);
    }

    private static IReadOnlyList<DrawnGlyphRun> FirstPage(string name)
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require(name));

        RecordingDrawingSink sink = new();
        ((SpreadsheetPages)document.Layout()).Pages[0].Draw(sink);
        return [.. sink.Pages[0].Runs];
    }

    private static DrawnGlyphRun Run(IReadOnlyList<DrawnGlyphRun> runs, string text)
        => runs.First(r => r.Text.StartsWith(text, StringComparison.Ordinal));
}
