using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.Spreadsheets.Layout;
using XRay.Office.TestKit;
using Shouldly;

namespace XRay.Office.Spreadsheets.Tests;

/// <summary>
/// A data label written on two lines is drawn as two lines.
/// </summary>
/// <remarks>
/// <para>
/// A label showing a percentage beside a category or a series name is joined by a newline rather
/// than by <c>"; "</c> — Office's own rule, which
/// <c>oox/source/drawingml/chart/seriesconverter.cxx:168-172</c> reproduces and
/// <see cref="ChartDataLabel.Separator"/> already carries. Shaping the joined string as one glyph
/// run draws the newline as a zero-width nothing and runs the halves together, so <c>East</c> and
/// <c>26%</c> came out as the single token <c>East26%</c> on
/// <c>005_Contextures_chart_sample_6e279b08.xlsx</c> — four labels, four words lost against the
/// reference.
/// </para>
/// <para>
/// The words track fixed the identical defect in <c>FrameChart</c> in round 52 and left this one
/// deliberately for the sheets round. <c>SlideChart</c> still carries it.
/// </para>
/// <para>
/// <strong>Driven through <see cref="SheetChart"/> directly</strong>, for the reason
/// <see cref="SheetChartFaceTests"/> gives: two plots differing in one field are what separates
/// "the break is honoured" from "this chart happens to draw two runs".
/// </para>
/// </remarks>
public sealed class SheetChartLabelLineTests
{
    /// <summary>The text of every glyph run a one-series pie draws, under the given separator.</summary>
    private static List<string> Drawn(string separator)
    {
        ChartPlot plot = new()
        {
            Kind = ChartPlotKind.Pie,
            Categories = ["East", "West"],
            Series =
            [
                new ChartSeries("Sales", [3.0, 1.0])
                {
                    Label = new ChartDataLabel
                    {
                        ShowCategory = true,
                        ShowPercent = true,
                        Separator = separator,
                    },
                },
            ],
        };

        RecordingDrawingSink sink = new();
        sink.BeginPage(new DocSize(Length.FromPoints(400), Length.FromPoints(300)));
        SheetChart.Draw(
            sink,
            plot,
            new DocRect(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300)),
            1.0);
        sink.EndPage();

        return [.. sink.Pages[0].Runs.Select(run => run.Run.Text)];
    }

    /// <summary>The corpus shape: the two halves reach the page as two runs.</summary>
    [Fact]
    public void ALabelJoinedByANewlineIsDrawnAsTwoRuns()
    {
        List<string> drawn = Drawn("\n");

        drawn.ShouldContain("East");
        drawn.ShouldContain("75%");
        drawn.ShouldNotContain(text => text.Contains("East7", StringComparison.Ordinal));
    }

    /// <summary>
    /// The control: the same label joined by <c>"; "</c> is one run, and stays one run.
    /// </summary>
    /// <remarks>
    /// It is the separator that decides, not the presence of two fields — a label whose parts are
    /// joined by a semicolon is a single line in LibreOffice too, and splitting it would break
    /// every non-pie label the corpus has.
    /// </remarks>
    [Fact]
    public void ALabelJoinedBySemicolonIsStillOneRun()
    {
        List<string> drawn = Drawn("; ");

        drawn.ShouldContain("East; 75%");
        drawn.ShouldNotContain("East");
    }

    /// <summary>
    /// Both lines are placed, and the second sits below the first rather than on top of it.
    /// </summary>
    /// <remarks>
    /// Stacking is the half a text-only assertion cannot see: two runs drawn at the same origin
    /// extract as two tokens and look right to the gate while overprinting on the page.
    /// </remarks>
    [Fact]
    public void TheSecondLineIsDrawnBelowTheFirst()
    {
        ChartPlot plot = new()
        {
            Kind = ChartPlotKind.Pie,
            Categories = ["East", "West"],
            Series =
            [
                new ChartSeries("Sales", [3.0, 1.0])
                {
                    Label = new ChartDataLabel
                    {
                        ShowCategory = true,
                        ShowPercent = true,
                        Separator = "\n",
                    },
                },
            ],
        };

        RecordingDrawingSink sink = new();
        sink.BeginPage(new DocSize(Length.FromPoints(400), Length.FromPoints(300)));
        SheetChart.Draw(
            sink,
            plot,
            new DocRect(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300)),
            1.0);
        sink.EndPage();

        DrawnGlyphRun east = sink.Pages[0].Runs.First(run => run.Run.Text == "East");
        DrawnGlyphRun share = sink.Pages[0].Runs.First(run => run.Run.Text == "75%");

        share.Run.Origin.Y.ShouldBeGreaterThan(east.Run.Origin.Y);
    }
}
