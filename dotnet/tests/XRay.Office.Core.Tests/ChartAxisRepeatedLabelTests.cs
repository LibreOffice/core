using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Numbers;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// An axis whose first tick formats to nothing has its automatic interval cap halved.
/// </summary>
/// <remarks>
/// <para>
/// tdf#48041's arm of <c>VCartesianAxis::estimateMaximumAutoMainIncrementCount</c>
/// (<c>chart2/source/view/axes/VCartesianAxis.cxx</c>:1578-1600): every tick of the pass just
/// finished is formatted through the axis' number format, the longest run of consecutive equal
/// strings is counted, and the estimate is lowered to
/// <c>m_aAllTickInfos[0].size() / (nMaxSameLabel + 1)</c> when that is smaller. Its comparison
/// starts against an <em>empty</em> <c>OUString</c> (:1582), so a first tick that draws nothing
/// scores a repeat although no two of the axis' labels are alike.
/// </para>
/// <para>
/// The witness is <c>048_Expense_trends_budget</c>, whose value axis states
/// <c>c:numFmt formatCode="#,##0;;"</c> — a positive section and two empty ones, so zero and
/// every negative draw as nothing — over a 0…500 range. 26.2.4.2 steps it by <b>100</b> and this
/// tree stepped it by 50. Eleven ticks, one repeat, <c>11 / 2 = 5</c>, and five intervals of a
/// 0…500 range is the 1/2/5 ladder's 100.
/// </para>
/// <para>
/// <b>One attribute at a time at 26.2.4.2</b> (<c>probes/chart-fit-r97/axis-variants.py</c>)
/// separates this from the length-over-label-height cap that <c>probes/chart-axis-r87</c>
/// corrected: the document as authored steps by 100; with <c>formatCode</c> alone changed to
/// <c>#,##0</c> it steps by <b>50</b>; with the sheet's <c>fitToPage</c> and
/// <c>pageSetup/@scale</c> alone removed it still steps by <b>100</b> — at an axis 180.29 pt long
/// with 13.42 pt labels, where the length cap saturates at ten just as it does at 151 pt with
/// 9.04 pt labels.
/// </para>
/// </remarks>
public class ChartAxisRepeatedLabelTests
{
    /// <summary>A measurer whose labels are small enough that the length cap always saturates.</summary>
    private sealed class SmallRuler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
            => new(size * (0.55 * text.Length), size * 1.15);
    }

    /// <summary>The witness' shape: a 0…500 range on a plot tall enough for ten intervals.</summary>
    private static ChartPlot Trends(string? format) => new()
    {
        Categories = ["Jan", "Feb", "Mar", "Apr"],
        Series =
        [
            new ChartSeries(
                "Actual", [420.0, 380.0, 460.0, 310.0], Colour.FromRgb(0x4472C4)),
        ],
        LabelSize = Length.FromPoints(9),
        ValueFormat = format is null ? null : NumberFormatCode.Parse(format),
    };

    private static double[] ValueTicks(ChartPlot plot)
        => [.. ChartLayout.Place(
                plot,
                new DocRect(Length.Zero, Length.Zero,
                    Length.FromPoints(420), Length.FromPoints(200)),
                new SmallRuler())
            .Labels
            .Select(label => double.TryParse(label.Text, out double value) ? value : double.NaN)
            .Where(value => !double.IsNaN(value))
            .Distinct()
            .Order()];

    /// <summary>
    /// With two empty sections the axis steps by 100, which is 26.2.4.2's own rendering.
    /// </summary>
    [Fact]
    public void AnEmptyFirstTickHalvesTheIntervalCount()
    {
        double[] ticks = ValueTicks(Trends("#,##0;;"));

        ticks.ShouldBe([100.0, 200.0, 300.0, 400.0, 500.0]);
    }

    /// <summary>
    /// And with the same format code minus its two empty sections it steps by 50, which is what
    /// 26.2.4.2 draws for that one-attribute variant.
    /// </summary>
    [Fact]
    public void TheEmptySectionsAreWhatChangeIt()
    {
        double[] ticks = ValueTicks(Trends("#,##0"));

        ticks.Length.ShouldBe(11);
        ticks[1].ShouldBe(50.0);
    }

    /// <summary>
    /// A format that repeats a label in the <em>middle</em> of the axis lowers the cap by the same
    /// arithmetic, which is the case the ticket was filed for.
    /// </summary>
    /// <remarks>
    /// <c>0,,"M"</c> scales by a million, so every tick of a 0…500 axis rounds to <c>0M</c> and
    /// ten of the eleven are repeats: <c>11 / 11 = 1</c>, raised to the floor of two by
    /// <c>ScaleAutomatism::setMaximumAutoMainIncrementCount</c>'s clamp
    /// (<c>chart2/source/view/axes/ScaleAutomatism.cxx</c>:142-150).
    /// </remarks>
    [Fact]
    public void RepeatsInsideTheAxisLowerItToo()
    {
        ValueTicks(Trends("0,,")).Length.ShouldBeLessThanOrEqualTo(3);
    }
}
