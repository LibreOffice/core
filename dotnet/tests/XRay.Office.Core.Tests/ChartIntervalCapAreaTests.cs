using System.Globalization;
using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Numbers;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// The <em>rectangle</em> a horizontal value axis' interval cap is taken against — which is not
/// the plot area as drawn.
/// </summary>
/// <remarks>
/// <para>
/// Round 112 pinned the cap's denominator (<c>MaxLabelTickIter</c>, the first three value
/// labels) and left the numerator bracketed at [125.4, 138) pt on
/// <c>027_Simple_personal_cash_flow_statement</c>'s savings chart, where the drawn axis is
/// 115.7 pt on the reference and 107.2 here. It is
/// <c>VDiagram::adjustInnerSize(aConsumedOuterRect)</c>'s answer at the moment the second
/// <c>doAutoScaling</c> runs (<c>chart2/source/view/main/ChartView.cxx</c>:556-604), and the
/// consumed rectangle is the diagram plus the <em>maximum labels</em> — three per axis,
/// unwrapped and unturned, because <c>createMaximumLabels</c> forces
/// <c>m_bOverlapAllowed</c> true and <c>canAutoAdjustLabelPlacement</c> refuses while it is.
/// </para>
/// <para>
/// <strong>Both arms below were measured at 26.2.4.2</strong> by rewriting one category at a
/// time in that chart's own <c>c:strCache</c> (<c>probes/chart-cap-r113/cat-set-027.py</c>).
/// Its five categories are <c>Other 1</c>, <c>Other 2</c>, <c>Cash Reserves</c>,
/// <c>Savings/Investment</c>, <c>401(k)/Etc</c>; the longest is index 3, which is
/// <c>nMaxIndex-1</c>, so <c>MaxLabelTickIter</c> resets to zero and holds <c>{0, 1, 2}</c>.
/// Eight characters onto index 3 or index 4 leaves the value axis at <c>$0 $2,000 … $14,000</c>
/// while the drawn axis collapses from 115.67 pt to <strong>75.10</strong>; eight onto index 0,
/// 1 or 2 coarsens it to <c>$0 $5,000 $10,000 $15,000</c>.
/// </para>
/// </remarks>
public class ChartIntervalCapAreaTests
{
    /// <summary>
    /// A ruler that is <em>not</em> proportional to the character count.
    /// </summary>
    /// <remarks>
    /// Which is what
    /// <see cref="TheLongestCategoryIsTheOneWithTheMostCharactersAndNotTheWidestOne"/> needs: the
    /// rule under test picks the longest label by <c>OUString::getLength()</c> and the widest of
    /// the three it then holds by measurement, so a ruler on which those two disagree is the only
    /// one that can tell the readings apart.
    /// </remarks>
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
        {
            double units = 0.0;
            foreach (char c in text) units += c == 'M' ? 1.0 : c == 'i' ? 0.2 : 0.55;
            return new DocSize(size * units, size * 1.15);
        }
    }

    /// <summary>
    /// <c>027</c>'s savings chart, in the file's own category order.
    /// </summary>
    private static ChartPlot Savings(params string[] categories) => new()
    {
        Kind = ChartPlotKind.Bar,
        Direction = ChartBarDirection.Bar,
        Categories = categories.Length > 0
            ? categories
            : ["Other 1", "Other 2", "Cash Reserves", "Savings/Investment", "401(k)/Etc"],
        Series =
        [
            new ChartSeries("Total", [0.0, 0.0, 5000.0, 6000.0, 12000.0], Colour.FromRgb(0x4472C4)),
        ],
        LabelSize = Length.FromPoints(9),
        ValueFormat = NumberFormatCode.Parse("\"$\"#,##0"),
    };

    private static DocRect Frame()
        => new(Length.Zero, Length.Zero, Length.FromPoints(311), Length.FromPoints(340));

    /// <summary>Every value the axis labels, in order along it.</summary>
    private static double[] ValueTicks(ChartPlot plot)
        => [.. ChartLayout.Place(plot, Frame(), new Ruler())
            .Labels
            .Select(label => Money(label.Text))
            .Where(value => !double.IsNaN(value))
            .Distinct()
            .Order()];

    private static double Money(string text)
    {
        string digits = new([.. text.Where(char.IsAsciiDigit)]);
        return digits.Length > 0 && text.StartsWith('$')
            ? double.Parse(digits, CultureInfo.InvariantCulture)
            : double.NaN;
    }

    /// <summary>
    /// Widening the axis' <em>widest</em> category label does not change the value axis'
    /// interval, because that label is not one of the three the maximum pass measures.
    /// </summary>
    /// <remarks>
    /// <strong>Fails at the base</strong>, where the cap was taken against the drawn plot area:
    /// a wider category band makes that rectangle narrower and coarsens the interval, which is
    /// the opposite of what 26.2.4.2 does.
    /// </remarks>
    [Fact]
    public void WideningTheWidestCategoryLabelDoesNotChangeTheInterval()
    {
        double[] plain = ValueTicks(Savings());
        double[] widened = ValueTicks(Savings(
            "Other 1", "Other 2", "Cash Reserves", "Savings/InvestmentWWWWWWWW", "401(k)/Etc"));

        plain.ShouldBe([0.0, 2000.0, 4000.0, 6000.0, 8000.0, 10000.0, 12000.0, 14000.0]);
        widened.ShouldBe(plain);
    }

    /// <summary>
    /// And the same eight characters on the <em>last</em> category change nothing either — the
    /// reset that sends <c>MaxLabelTickIter</c> back to zero covers both of the last two indices.
    /// </summary>
    [Fact]
    public void WideningTheLastCategoryDoesNotChangeItEither()
    {
        ValueTicks(Savings(
            "Other 1", "Other 2", "Cash Reserves", "Savings/Investment", "401(k)/EtcWWWWWWWW"))
            .ShouldBe(ValueTicks(Savings()));
    }

    /// <summary>
    /// Widening a category <em>inside</em> the measured set does change it — the control that
    /// makes the two tests above mean something.
    /// </summary>
    [Fact]
    public void WideningACategoryInsideTheMeasuredSetDoesChangeIt()
    {
        double[] coarse = ValueTicks(Savings(
            "Other 1", "Other 2", "Cash ReservesWWWWWWWW", "Savings/Investment", "401(k)/Etc"));

        coarse.Length.ShouldBeLessThan(8);
    }

    /// <summary>
    /// "The longest label" is a character count and not a width, and the first of two equally
    /// long ones wins.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>VAxisBase::getIndexOfLongestLabel</c>
    /// (<c>chart2/source/view/axes/VAxisBase.cxx</c>:195-212) compares <c>getLength()</c> under
    /// its own <c>//todo: get real text width (without creating shape) instead of character
    /// count</c>, with a strict <c>&gt;</c>.
    /// </para>
    /// <para>
    /// <strong>The witness is <c>026_Monthly_cash_flow_statement</c>'s expenses chart</strong>,
    /// whose two longest categories are <c>Disability premiums</c> at index 12 and
    /// <c>Federal/SS/Medicare</c> at index 16 — nineteen characters each, and the second is
    /// <c>nMaxIndex-1</c>. Choosing by width picks the second, the reset then sends the set to
    /// <c>{0, 1, 2}</c>, and the numerator comes out one whole interval too large: eight where
    /// 26.2.4.2 draws four. This tree read it that way for one build and moved that document's
    /// second chart from the reference's <c>$0 $5,000 … $20,000</c> to <c>$0 $2,000 … $16,000</c>.
    /// </para>
    /// <para>
    /// The two plots below differ only at index 2, which the character count never reaches —
    /// index 4 is longer than everything in both — so the measured set is <c>{3, 4, 5}</c> in
    /// both and the interval must be identical. Under a width rule index 2's run of <c>M</c> is
    /// the widest label on the axis, the set becomes <c>{1, 2, 3}</c>, and the numerator loses
    /// 70 pt.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheLongestCategoryIsTheOneWithTheMostCharactersAndNotTheWidestOne()
    {
        ChartPlot Expenses(string third) => new()
        {
            Kind = ChartPlotKind.Bar,
            Direction = ChartBarDirection.Bar,
            Categories = ["aa", "bb", third, "dd", new string('i', 11), "ee", "ff", "gg"],
            Series =
            [
                new ChartSeries(
                    "Total",
                    [0.0, 0.0, 150.0, 200.0, 250.0, 600.0, 12500.0, 15000.0],
                    Colour.FromRgb(0x4472C4)),
            ],
            LabelSize = Length.FromPoints(9),
            ValueFormat = NumberFormatCode.Parse("\"$\"#,##0"),
        };

        double[] wideThird = ValueTicks(Expenses(new string('M', 10)));
        double[] narrowThird = ValueTicks(Expenses("cc"));

        narrowThird.ShouldBe(
            [0.0, 2000.0, 4000.0, 6000.0, 8000.0, 10000.0, 12000.0, 14000.0, 16000.0]);
        wideThird.ShouldBe(narrowThird);
    }

    /// <summary>
    /// A chart that states its own <em>inner</em> rectangle is capped on that rectangle, because
    /// nothing resizes it.
    /// </summary>
    /// <remarks>
    /// <c>mbUseFixedInnerSize</c> guards <c>reduceToMinimumSize</c> and every
    /// <c>adjustInnerSize</c> alike (<c>ChartView.cxx</c>:559, :594, :619, :690), and it is the
    /// diagram's <c>PosSizeExcludeAxes</c> — which <c>c:layoutTarget val="inner"</c> and ODF's
    /// <c>chart:coordinate-region</c> both set. So for those charts the cap's numerator and the
    /// drawn plot area are the same length, and widening a category label off the measured set
    /// <em>does</em> narrow it.
    /// </remarks>
    [Fact]
    public void AStatedInnerRectangleIsCappedOnItself()
    {
        ChartPlot stated = Savings() with { PlotAreaFraction = (0.30, 0.05, 0.66, 0.80) };
        ChartPlot widened = Savings(
            "Other 1", "Other 2", "Cash Reserves", "Savings/InvestmentWWWWWWWW", "401(k)/Etc")
            with
            { PlotAreaFraction = (0.30, 0.05, 0.66, 0.80) };

        ValueTicks(widened).ShouldBe(ValueTicks(stated));
    }
}
