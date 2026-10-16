using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// Two things a chart draws that no word count can see: a percent stack's axis, and the sample a
/// line series puts in the legend.
/// </summary>
/// <remarks>
/// Both were found by reading a page rather than a number. On
/// <c>8_P-Pavese_AIRBUS-ATB-journee-CRATB.pptx</c> the value axis read <c>0% … 70000%</c> because
/// a percent stack was drawn as an ordinary one and the axis' <c>0%</c> format then multiplied
/// the raw total by a hundred; on
/// <c>southern-classic-kennesaw-state-university-final.pptx</c> the legend drew four hollow
/// rectangles in a key sized for a 22.7 pt rule, where the reference draws four line samples and
/// three of them dotted.
/// </remarks>
public class ChartPercentAndLineKeyTests
{
    /// <summary>Half an em per character, 1.15 em a line — the same stand-in the other layout tests use.</summary>
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
            => new(size * (0.5 * text.Length) * (bold ? 1.1 : 1.0), size * 1.15);
    }

    private static readonly DocRect Frame =
        new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300));

    private static ChartDrawing Place(ChartPlot plot) => ChartLayout.Place(plot, Frame, new Ruler());

    // ------------------------------------------------------------- percent stack

    /// <summary>
    /// A percent stack's value axis is 0 to 1 in ten steps, whatever its numbers are.
    /// </summary>
    /// <remarks>
    /// chart2 gives the axis <c>AxisType::PERCENT</c>, whose scale is fixed rather than derived.
    /// The automatic rule would round the normalised maximum of exactly 1 up to 1.2 and step it
    /// by 0.2, which is six ticks reading <c>0% … 120%</c> against the reference's eleven.
    /// </remarks>
    [Fact]
    public void APercentStackIsDrawnZeroToOneHundredInTenSteps()
    {
        ChartPlot plot = new()
        {
            Categories = ["Thérapeutique", "Examens"],
            Series =
            [
                new ChartSeries("Suivi", [548.0, 317.0], Colour.FromRgb(0x4472C4)),
                new ChartSeries("Non suivi", [73.0, 122.0], Colour.FromRgb(0xED7D31)),
            ],
            IsStacked = true,
            IsPercentStacked = true,
            ValueFormat = XRay.Office.Core.Numbers.NumberFormatCode.Parse("0%"),
        };

        List<string> ticks =
            [.. Place(plot).Labels.Select(l => l.Text).Where(t => t.EndsWith('%'))];

        ticks.ShouldBe(
            ["0%", "10%", "20%", "30%", "40%", "50%", "60%", "70%", "80%", "90%", "100%"],
            ignoreOrder: true);
    }

    /// <summary>
    /// Every category fills the plot, and the split inside it is the ratio and not the count.
    /// </summary>
    /// <remarks>
    /// 548 of 621 is 88.24% and 317 of 439 is 72.21% — the two percentages the file's own
    /// annotations state, arrived at from the geometry alone.
    /// </remarks>
    [Fact]
    public void EveryPercentStackedColumnIsTheSameHeightAndSplitByRatio()
    {
        ChartPlot plot = new()
        {
            Categories = ["A", "B"],
            Series =
            [
                new ChartSeries("Suivi", [548.0, 317.0], Colour.FromRgb(0x4472C4)),
                new ChartSeries("Non suivi", [73.0, 122.0], Colour.FromRgb(0xED7D31)),
            ],
            IsStacked = true,
            IsPercentStacked = true,
            Overlap = 100.0,
        };

        ChartDrawing drawing = Place(plot);
        DocRect area = drawing.PlotArea;

        // The four bars, in the order AddBars emits them: series 1 over both categories, then
        // series 2 over both.
        List<DocRect> bars =
            [.. drawing.Shapes.Where(b => b.Fill is not null && b.Bounds().Width > Length.Zero)
                              .Select(b => b.Bounds())];

        bars.Count.ShouldBe(4);

        // Both columns reach the top of the plot area.
        double firstColumn = (bars[0].Height + bars[2].Height).Emu;
        double secondColumn = (bars[1].Height + bars[3].Height).Emu;

        firstColumn.ShouldBe(area.Height.Emu, tolerance: 2.0);
        secondColumn.ShouldBe(area.Height.Emu, tolerance: 2.0);

        // And the lower segment is its own share of it.
        (bars[0].Height.Emu / (double)area.Height.Emu).ShouldBe(548.0 / 621.0, 1e-3);
        (bars[1].Height.Emu / (double)area.Height.Emu).ShouldBe(317.0 / 439.0, 1e-3);
    }

    // ------------------------------------------------------------- legend keys

    private static ChartPlot Lines(IReadOnlyList<Length>? dash)
        => new()
        {
            Kind = ChartPlotKind.Line,
            Categories = ["Q1", "Q2", "Q3"],
            Series =
            [
                new ChartSeries("Closing Price", [1.0, 2.0, 3.0], Line: Colour.FromRgb(0x1A2557))
                {
                    DashPattern = dash,
                },
            ],
            Legend = ChartLegendPosition.Right,
        };

    /// <summary>
    /// A line series' legend key is a horizontal sample of the line, not a hollow box.
    /// </summary>
    /// <remarks>
    /// <c>LegendSymbolStyle::Line</c> against <c>Box</c>. The key was already sized for a line —
    /// 800 hundredths of a millimetre, 22.7 pt — and a rectangle was being drawn inside it, which
    /// is the "hollow box swatch" a reviewer reported.
    /// </remarks>
    [Fact]
    public void ALineSeriesLegendKeyIsALineAndNotABox()
    {
        ChartDrawing drawing = Place(Lines(dash: null));

        // No box carries the series' colour: the only boxes left are the frame, if any.
        drawing.Boxes.ShouldNotContain(b => b.Line == Colour.FromRgb(0x1A2557));

        ChartLine key = drawing.Lines.First(l => l.Colour == Colour.FromRgb(0x1A2557));

        // Horizontal, and the key's own width — 800 hundredths of a millimetre.
        key.From.Y.ShouldBe(key.To.Y);
        (key.To.X - key.From.X).ShouldBe(Length.FromMm100(800));
    }

    /// <summary>A dashed series' key carries the dash and is twice as wide.</summary>
    /// <remarks>
    /// <c>getPreferredLegendKeyAspectRatio</c> returns 800 for a line and 1600 when it is dashed,
    /// because three dots in 22.7 pt do not read as a pattern.
    /// </remarks>
    [Fact]
    public void ADashedSeriesLegendKeyIsDashedAndTwiceAsWide()
    {
        IReadOnlyList<Length> dash = [Length.FromPoints(1), Length.FromPoints(3)];
        ChartDrawing drawing = Place(Lines(dash));

        ChartLine key = drawing.Lines.First(l => l.Colour == Colour.FromRgb(0x1A2557));

        key.DashPattern.ShouldNotBeNull().ShouldBe(dash);
        (key.To.X - key.From.X).ShouldBe(Length.FromMm100(1600));
    }

    /// <summary>A bar series' key stays the filled box it has always been.</summary>
    [Fact]
    public void ABarSeriesLegendKeyIsStillABox()
    {
        ChartPlot plot = new()
        {
            Categories = ["Q1", "Q2", "Q3"],
            Series = [new ChartSeries("Revenue", [1.0, 2.0, 3.0], Colour.FromRgb(0x99CCFF))],
            Legend = ChartLegendPosition.Right,
        };

        ChartDrawing drawing = Place(plot);

        drawing.Boxes.ShouldContain(b => b.Fill == Colour.FromRgb(0x99CCFF)
                                         && b.Bounds.Height > Length.Zero);
    }
}
