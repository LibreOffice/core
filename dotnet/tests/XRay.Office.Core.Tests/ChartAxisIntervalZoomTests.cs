using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// A worksheet's print zoom must not change which ticks a value axis draws.
/// </summary>
/// <remarks>
/// <para>
/// <c>chart2</c> composes a chart on its own draw page at the type size the file states and
/// knows nothing about what scales the picture afterwards — Calc's print zoom is applied by
/// <c>ScPrintFunc</c> to the output device, long after the axis has chosen its interval. This
/// tree takes the zoom through the type sizes instead, so that every glyph run stays in page
/// coordinates (<c>SheetChart.Sized</c>), and one measurement then has to be taken at the size
/// the file states rather than at the zoomed one: the interval cap, which is the axis' drawn
/// length over a <em>device-quantised</em> label height
/// (<c>VCartesianAxis::estimateMaximumAutoMainIncrementCount</c>,
/// <c>chart2/source/view/axes/VCartesianAxis.cxx</c>:1559-1618).
/// </para>
/// <para>
/// Every other length in the composition is scaled on both sides of that division and cancels.
/// This one does not, because a chart's text is measured on a 96 dpi device: a face is
/// instantiated at a whole number of pixels, so its line height is a step function of the size
/// rather than a fixed fraction of the em. The ruler below reproduces exactly that step, which
/// is what makes the two calls below able to disagree at all.
/// </para>
/// <para>
/// The witness is <c>040_Blood_pressure_tracker</c>: a fit-to-width sheet drawn at 64%, whose
/// secondary value axis 26.2.4.2 labels <c>60 65 70 75 80</c> and which this tree labelled
/// <c>62 64 … 80</c> — a cap of 10 where the reference's is 8, from that rounding alone.
/// </para>
/// <para>
/// <strong>Round 110 removed the reason the sheet path needs this, and kept the property and
/// these tests.</strong> <c>SheetChart</c> no longer scales the type: it lays the chart out on
/// the chart's own page — the anchor before the sheet's zoom, which is what
/// <c>&lt;chart:chart svg:width&gt;</c> says the reference does — and scales the finished
/// drawing, so the zoom reaches no measurement at all and <see cref="ChartPlot.TypeScale"/> is
/// 1 everywhere. What is asserted below is still exactly true of the property, and it is the
/// statement of the hazard for any caller that does scale a chart's type.
/// </para>
/// </remarks>
public class ChartAxisIntervalZoomTests
{
    /// <summary>
    /// A measurer with <c>chart2</c>'s own quantisation in it: DejaVu Sans' ascent and descent,
    /// each rounded to a whole 96 dpi pixel at the size asked for.
    /// </summary>
    /// <remarks>
    /// <c>DrawModelWrapper</c> measures on <c>Application::GetDefaultDevice()</c>, which is
    /// 96 dpi (<c>SvpSalGraphics::GetResolution</c>, <c>vcl/headless/svpgdi.cxx</c>:44), and a
    /// device instantiates a font at a whole number of pixels. DejaVu Sans answers 1.2266 em at
    /// 11 pt and 1.0670 em at 7.04 pt — the same type at a 64% zoom.
    /// </remarks>
    private sealed class DeviceRuler : IChartTextMeasurer
    {
        private const double Ascent = 1901.0 / 2048.0;
        private const double Descent = 483.0 / 2048.0;

        public DocSize Measure(string text, Length size, string? family, bool bold)
        {
            double pixels = Math.Round(size.Points * 96.0 / 72.0, MidpointRounding.AwayFromZero);
            double height = Math.Round(Ascent * pixels, MidpointRounding.AwayFromZero)
                + Math.Round(Descent * pixels, MidpointRounding.AwayFromZero);
            return new DocSize(
                size * (0.55 * text.Length) * (bold ? 1.1 : 1.0),
                Length.FromPoints(height * 0.75));
        }
    }

    /// <summary>The witness' shape: a short axis whose values sit well above zero.</summary>
    private static ChartPlot Pulse() => new()
    {
        Categories = ["1", "2", "3", "4", "5", "6", "7", "8"],
        Series =
        [
            new ChartSeries(
                "Heart rate",
                [72.0, 75.0, 70.0, 68.0, 70.0, 72.0, 78.0, 69.0],
                Colour.FromRgb(0x99CCFF)),
        ],
        LabelSize = Length.FromPoints(11),
    };

    private static ChartPlot Zoomed(ChartPlot plot, double zoom) => plot with
    {
        TypeScale = zoom,
        TitleSize = plot.TitleSize * zoom,
        AxisTitleSize = plot.AxisTitleSize * zoom,
        LabelSize = plot.LabelSize * zoom,
    };

    private static DocRect Frame(double zoom) => new(
        Length.Zero, Length.Zero,
        Length.FromPoints(320 * zoom), Length.FromPoints(144 * zoom));

    private static string[] ValueTicks(ChartPlot plot, double zoom)
        => [.. ChartLayout.Place(plot, Frame(zoom), new DeviceRuler())
            .Labels
            .Where(label => label.Text.Length > 0 && char.IsAsciiDigit(label.Text[0])
                && double.TryParse(label.Text, out double value) && value >= 50.0)
            .Select(label => label.Text)
            .Distinct()
            .Order()];

    /// <summary>
    /// The same chart at 100% and at 64% draws the same value-axis ticks.
    /// </summary>
    /// <remarks>
    /// The frame is the witness' shape: a plot 111 pt tall, which is 8.2 label heights, so the
    /// axis is capped at 8 and the interval ladder settles on 5 rather than 2.
    /// </remarks>
    [Fact]
    public void APrintZoomDoesNotChangeWhichTicksAnAxisDraws()
    {
        string[] full = ValueTicks(Pulse(), 1.0);
        full.ShouldBe(["60", "65", "70", "75", "80"]);

        ValueTicks(Zoomed(Pulse(), 0.64), 0.64).ShouldBe(full);
    }

    /// <summary>
    /// And the zoom is what would have changed them: the same geometry with the scale unstated
    /// — which is what this tree did before <see cref="ChartPlot.TypeScale"/> existed — reads
    /// the label height at the zoomed size and draws a denser axis.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The control matters because the invariance above is trivially true under a measurer whose
    /// height is proportional to the size, and every other ruler in these tests is one.
    /// </para>
    /// <para>
    /// It is not a claim that the two are invariant at <em>every</em> size. The rest of the
    /// composition still measures its label reservations at the zoomed size, so the plot
    /// rectangle the cap divides is not exactly the unzoomed one scaled, and a geometry sitting
    /// on a cap boundary can still flip — see <c>probes/chart-axis-r87/results.md</c> §7. This
    /// frame is a whole label height clear of one.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheZoomIsWhatWouldHaveChangedThem()
    {
        ChartPlot unstated = Zoomed(Pulse(), 0.64) with { TypeScale = 1.0 };

        ValueTicks(unstated, 0.64).ShouldBe(
            ["62", "64", "66", "68", "70", "72", "74", "76", "78", "80"]);
    }
}
