using Paperless.Core.Documents;
using Paperless.Spreadsheets.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Spreadsheets.Tests;

/// <summary>
/// A workbook's chart is measured and drawn in the face the chart states.
/// </summary>
/// <remarks>
/// <para>
/// <c>ChartPlot.TextFamily</c> was added on the slides track, which measured it there and left the
/// spreadsheet consumer deliberately unwired: turning it on changes the layout of every workbook
/// carrying a chart, so it wanted the round that sweeps this track. This is that wiring's test.
/// </para>
/// <para>
/// It matters for <em>layout</em> and not only for appearance. The widest axis label reserves the
/// plot area's left edge and the widest legend entry reserves its right, so a chart measured in one
/// face and drawn in another has its plot rectangle in the wrong place and every mark inside it
/// follows. Measured on the corpus: `Keywords_Mapping_Graphs_and_Charts.xlsx` embedded Liberation
/// Sans beside both Carlitos and now embeds exactly the reference's two Carlitos.
/// </para>
/// <para>
/// The fixture is <c>chart-bar-sheet.xlsx</c> with its eleven <c>a:latin typeface="Arial"</c>
/// rewritten to <c>Caladea</c>. Arial would not have discriminated: it resolves to Liberation Sans,
/// which is exactly the default the unwired consumer used, so the test would pass with the wiring
/// removed. Caladea is a serif and resolves to itself.
/// </para>
/// </remarks>
public sealed class SheetChartFaceTests
{
    private static List<DrawnGlyphRun> Drawn(string name)
    {
        using IPaginatedDocument document = (IPaginatedDocument)PaperlessDocument.Open(
            Corpus.Require(name));

        RecordingDrawingSink sink = new();
        ((SpreadsheetPages)document.Layout()).Pages[^1].Draw(sink);
        return sink.Pages[0].Runs;
    }

    [Fact]
    public void AStatedChartFaceReachesTheGlyphsItIsDrawnWith()
    {
        List<DrawnGlyphRun> drawn = Drawn("sheet-chart-face-stated.xlsx");

        // The page carries the sheet's own cells as well, which are Liberation Sans; what the
        // wiring decides is whether any Caladea reaches the page at all.
        drawn.ShouldNotBeEmpty();
        drawn.Select(run => run.Run.Font.FamilyName).ShouldContain("Caladea");
    }

    /// <summary>
    /// A chart stating Arial still lands on Liberation Sans, which is the control.
    /// </summary>
    /// <remarks>
    /// The same workbook before its rewrite. It passes with the wiring in place and with it
    /// removed, so it is a drift guard and is labelled as one: what it says is that the change
    /// did not disturb the common case, not that the change happened.
    /// </remarks>
    [Fact]
    public void AChartStatingArialIsStillDrawnInLiberationSans()
    {
        List<DrawnGlyphRun> drawn = Drawn("chart-bar-sheet.xlsx");

        drawn.ShouldNotBeEmpty();
        drawn.Select(run => run.Run.Font.FamilyName).Distinct().ShouldBe(["Liberation Sans"]);
        drawn.Select(run => run.Run.Font.FamilyName).ShouldNotContain("Caladea");
    }
}
