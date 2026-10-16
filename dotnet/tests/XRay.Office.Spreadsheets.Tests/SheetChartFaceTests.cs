using XRay.Office.Core.Charts;
using XRay.Office.Core.Documents;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

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
        using IPaginatedDocument document = (IPaginatedDocument)OfficeDocument.Open(
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

    /// <summary>
    /// A chart's bold text reaches the page in the family's bold face.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>ChartPlot.IsTitleBold</c> arrived by the same route <c>TextFamily</c> did — measured on
    /// the slides track, handed to this consumer, and dropped by it — and it was dropped one
    /// round longer, with the comment saying so. The corpus said what that cost plainly: the
    /// reference draws <c>Template Pilot Logbook JAR-FCL V3.0.xls</c> page 17's chart title and
    /// both its axis titles in Liberation Sans <em>Bold</em>, marked <c>&lt;b&gt;</c> by
    /// <c>pdftohtml -xml</c>, and we drew all three regular while the model already said bold.
    /// </para>
    /// <para>
    /// <strong>The weight is asserted, not the family.</strong> Both faces of Liberation Sans
    /// report the same <c>name</c>-table family, so only <c>FontReference.Weight</c> and the face
    /// key tell them apart — which is also why the defect survived a face-level test.
    /// </para>
    /// <para>
    /// <strong>Driven through <c>SheetChart</c> directly rather than through a document</strong>,
    /// because no fixture in the corpus states a bold chart title: <c>chart-bar-sheet.xlsx</c>
    /// writes <c>b="0"</c> on its own, which is what the case below stands for. Two plots that
    /// differ in one <c>bool</c> are what separates "the weight is honoured" from "some text on
    /// this page happens to be bold".
    /// </para>
    /// </remarks>
    [Fact]
    public void ABoldChartTitleIsDrawnInTheBoldFace()
    {
        Weights(Bold: true).ShouldContain(700);

        // The axis labels beside it state nothing and stay regular, so this is a weight reaching
        // the text that carries one rather than the whole chart turning bold.
        Weights(Bold: true).ShouldContain(400);
    }

    /// <summary>The same chart with the weight cleared draws nothing bold.</summary>
    /// <remarks>
    /// The control for the case above and for the corpus: an OOXML chart that writes
    /// <c>b="0"</c> means regular, and before this wiring every chart on every sheet was drawn
    /// that way whatever it said.
    /// </remarks>
    [Fact]
    public void AChartWhoseTitleStatesRegularDrawsNothingBold()
    {
        Weights(Bold: false).ShouldNotContain(700);
    }

    /// <summary>The weights of every glyph run a one-title chart draws.</summary>
    private static List<int> Weights(bool Bold)
    {
        ChartPlot plot = new()
        {
            Title = "Regional revenue",
            Categories = ["North", "South"],
            Series = [new ChartSeries("Revenue", [1.0, 2.0])],
            IsTitleBold = Bold,
        };

        RecordingDrawingSink sink = new();
        sink.BeginPage(new DocSize(Length.FromPoints(400), Length.FromPoints(300)));
        SheetChart.Draw(
            sink,
            plot,
            new DocRect(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300)),
            1.0);
        sink.EndPage();

        return [.. sink.Pages[0].Runs.Select(run => run.Run.Font.Weight)];
    }
}