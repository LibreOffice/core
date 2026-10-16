using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.TestKit;
using XRay.Office.WordProcessing.Layout;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A data label holding a line break is drawn as a stack of lines rather than as one run.
/// </summary>
/// <remarks>
/// <para>
/// A label showing a percentage and no value is written on two lines — Office's own separator,
/// <c>seriesconverter.cxx:168-172</c>, which <c>ChartDataLabel.Separator</c> already defaulted to
/// <c>"\n"</c>. Shaping the whole string as a single run drew that newline as a zero-width
/// nothing and ran the two halves together, so the reference's <c>Leaf 11</c> / <c>15%</c> came
/// out of <c>pdftotext</c> as the single token <c>Leaf 1115%</c>.
/// </para>
/// <para>
/// Measured on LibreOffice 26.2.4.2, once the categories were being read at every level:
/// <c>027_Unit_Circle_Chart_Graphical_Chart</c> 370 words against 378 before this and 376 after —
/// the difference between <c>words</c> and <c>match</c>. Three more documents moved to exact on
/// the same change: <c>023</c> and <c>021_Unit_Circle_Chart</c> each 108 against 107 to 107, and
/// <c>pie-chart-result.docx</c> 30 against 40 to 36.
/// </para>
/// </remarks>
public class FrameChartLabelLinesTests
{
    private static readonly DocRect Box =
        new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300));

    /// <summary>A one-series pie whose labels carry a category and a percentage.</summary>
    private static ChartPlot Pie(string separator) => new()
    {
        Kind = ChartPlotKind.Pie,
        Categories = ["Leaf 11", "Leaf 12"],
        Series =
        [
            new ChartSeries(null, [3.0, 17.0])
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

    private static List<DrawnGlyphRun> Draw(ChartPlot plot)
    {
        RecordingDrawingSink sink = new();
        sink.BeginPage(new DocSize(Length.FromPoints(400), Length.FromPoints(300)));
        FrameChart.Draw(sink, plot, Box, "Liberation Sans");
        sink.EndPage();

        return [.. sink.Pages.ShouldHaveSingleItem().Runs];
    }

    /// <summary>The two halves become two runs, on two baselines, one line apart.</summary>
    [Fact]
    public void ALabelWithALineBreakIsDrawnAsTwoRuns()
    {
        List<DrawnGlyphRun> runs = Draw(Pie("\n"));

        DrawnGlyphRun name = runs.Single(run => run.Text == "Leaf 11");
        DrawnGlyphRun share = runs.Single(run => run.Text == "15%");

        runs.ShouldNotContain(
            run => run.Text.Contains('\n'),
            "the newline must break the label rather than be shaped as a glyph");

        share.Origin.Y.ShouldBeGreaterThan(
            name.Origin.Y, "the percentage goes under the name, not beside it");
        (share.Origin.Y - name.Origin.Y).Points.ShouldBeInRange(
            8.0, 20.0, "one line of an 10 pt label");
    }

    /// <summary>
    /// The control: a label joined by the ordinary separator is still one run.
    /// </summary>
    /// <remarks>
    /// Without this the change would be indistinguishable from splitting every label on every
    /// space, which would break a category holding one.
    /// </remarks>
    [Fact]
    public void ALabelWithoutALineBreakIsStillOneRun()
    {
        List<DrawnGlyphRun> runs = Draw(Pie("; "));

        runs.ShouldContain(run => run.Text == "Leaf 11; 15%");
        runs.ShouldNotContain(run => run.Text == "15%");
    }
}
