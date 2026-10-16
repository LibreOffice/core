using System.Globalization;
using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Numbers;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// A value axis running along the bottom: which of its labels decide its interval, and what it
/// does when the labels that follow them collide.
/// </summary>
/// <remarks>
/// <para>
/// Two rules, both of <c>VCartesianAxis</c>'s and both measured at 26.2.4.2 on
/// <c>027_Simple_personal_cash_flow_statement</c>'s savings chart — a horizontal bar chart over
/// 0…12,000 whose money axis runs along the bottom in <c>"$"#,##0</c>.
/// </para>
/// <list type="number">
/// <item><description>
/// <strong>The interval cap is measured over at most the first three tick labels.</strong>
/// <c>createMaximumLabels</c> builds shapes for a <c>MaxLabelTickIter</c>
/// (<c>chart2/source/view/axes/VCartesianAxis.cxx</c>:455-511, :1517-1530) rather than for the
/// whole tick array, and that iterator is seeded with zero for a value axis, so
/// <c>m_nMaximumTextWidthSoFar</c> is the widest of <c>{0, 1, 2}</c> and every later label,
/// however wide, is never measured.
/// </description></item>
/// <item><description>
/// <strong>A value axis then turns its labels 45° when they collide</strong>, by the same
/// <c>while (!createTextShapes(…)) {}</c> every axis goes through — reachable at all only
/// because round 111 gave the value axis chart2's own <c>TextBreak</c> false, which is what
/// <c>canAutoAdjustLabelPlacement</c> (:539-556) requires.
/// </description></item>
/// </list>
/// <para>
/// <strong>Rule 1 was measured by making the two halves of one axis disagree.</strong> The
/// document as authored is labelled <c>$0 $2,000 … $14,000</c> — seven intervals. With its
/// <c>c:numFmt</c> alone changed so that every tick from 6,000 up carries an eight-character
/// suffix, 26.2.4.2 draws <strong>the same seven intervals</strong>; with the condition reversed,
/// so that only <c>$0</c>, <c>$2,000</c> and <c>$4,000</c> carry it, it collapses to
/// <c><b>$0 $10,000 $20,000</b></c>. Nothing but the width of the first three labels moved the
/// cap. `probes/chart-axis-r112` §2 has the two renderings.
/// </para>
/// </remarks>
public class ChartValueAxisArrangementTests
{
    /// <summary>A ruler whose width is proportional to the character count.</summary>
    /// <remarks>
    /// Which is all these tests need: the question is which <em>strings</em> are measured, not
    /// how wide a particular face makes them, and a proportional ruler makes the two candidate
    /// rules answer visibly different numbers.
    /// </remarks>
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
            => new(size * (0.55 * text.Length), size * 1.15);
    }

    /// <summary>Only the ticks below 5,000 carry the suffix — the first three of them.</summary>
    private const string EarlyWide =
        "[<5000]\"$\"#,##0\"WWWWWWWW\";[>=5000]\"$\"#,##0;General";

    /// <summary>Only the ticks at 5,000 and above carry it — every one but the first three.</summary>
    private const string LateWide =
        "[<5000]\"$\"#,##0;[>=5000]\"$\"#,##0\"WWWWWWWW\";General";

    /// <summary>
    /// The witness' shape: a horizontal bar chart whose money axis runs along the bottom.
    /// </summary>
    private static ChartPlot Savings(string format) => new()
    {
        Kind = ChartPlotKind.Bar,
        Direction = ChartBarDirection.Bar,
        Categories = ["401(k)/Etc", "Savings/Investment", "Cash Reserves", "Other 1", "Other 2"],
        Series =
        [
            new ChartSeries(
                "Total", [12000.0, 11000.0, 2400.0, 0.0, 0.0], Colour.FromRgb(0x4472C4)),
        ],
        LabelSize = Length.FromPoints(10),
        ValueFormat = NumberFormatCode.Parse(format),
    };

    private static DocRect Frame()
        => new(Length.Zero, Length.Zero, Length.FromPoints(420), Length.FromPoints(200));

    /// <summary>Every value the axis labels, in order along it.</summary>
    private static double[] ValueTicks(ChartPlot plot)
        => [.. ChartLayout.Place(plot, Frame(), new Ruler())
            .Labels
            .Select(label => Number(label.Text))
            .Where(value => !double.IsNaN(value))
            .Distinct()
            .Order()];

    /// <summary>The number a money label carries, with the currency and the suffix taken off.</summary>
    private static double Number(string text)
    {
        string digits = new([.. text.Where(c => char.IsAsciiDigit(c))]);
        return digits.Length > 0 && text.StartsWith('$') ? double.Parse(digits, CultureInfo.InvariantCulture) : double.NaN;
    }

    /// <summary>
    /// Widening every label past the third does not change the interval.
    /// </summary>
    /// <remarks>
    /// The arm that separates <c>MaxLabelTickIter</c> from "the widest label on the axis": the
    /// two formats below produce the identical set of tick <em>values</em> under the reference's
    /// rule and quite different ones under the rule this tree had.
    /// </remarks>
    /// <summary>
    /// Widening every label past the third does not change the interval.
    /// </summary>
    /// <remarks>
    /// The arm that separates <c>MaxLabelTickIter</c> from "the widest label on the axis": the
    /// two formats below produce the identical set of tick <em>values</em> under the reference's
    /// rule and quite different ones under the rule this tree had.
    /// </remarks>
    [Fact]
    public void WideningTheLabelsPastTheThirdDoesNotChangeTheInterval()
    {
        double[] plain = ValueTicks(Savings("\"$\"#,##0"));
        double[] late = ValueTicks(Savings(LateWide));

        plain.ShouldBe([0.0, 2000.0, 4000.0, 6000.0, 8000.0, 10000.0, 12000.0, 14000.0]);
        late.ShouldBe(plain);
    }

    /// <summary>
    /// And widening the first three alone does change it — the control that makes the test above
    /// mean something.
    /// </summary>
    /// <remarks>
    /// A ruler is not a font, so the exact interval this lands on is the ruler's; what is
    /// asserted is the direction and the fact that it moves at all. Under "the widest label on
    /// the axis" the two formats above and this one would all give the same coarse answer.
    /// </remarks>
    [Fact]
    public void WideningTheFirstThreeAloneDoesChangeIt()
    {
        double[] early = ValueTicks(Savings(EarlyWide));

        early.Length.ShouldBeLessThan(8);
        early[^1].ShouldBeGreaterThan(14000.0);
    }

    /// <summary>
    /// A horizontal value axis whose labels collide is turned 45°, not left overlapping.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Measured on <c>026_Monthly_cash_flow_statement</c> page 7, whose first chart 26.2.4.2
    /// labels <c>$0 $20,000 … $100,000</c> at 45° — as 39 glyph-sized filled paths rather than
    /// as text, because a turned run inside an anisotropically squeezed chart is outlined — where
    /// this tree drew three upright labels.
    /// </para>
    /// <para>
    /// <strong>This is the one test of the five that does not use <see cref="Frame"/>, and the
    /// 380 is chosen rather than incidental.</strong> Round 117 corrected what a <em>turned</em>
    /// value label reserves at the axis' far end — <c>h·sin/2</c> rather than
    /// <c>(w·cos + h·sin)/2</c>, see <see cref="ChartTurnedValueLabelOverhangTests"/> — which
    /// widens the plot rectangle that this test's own collision is then decided in, and 420 sat
    /// within a few points of the threshold on this ruler. The rule under test is unchanged and
    /// so is the assertion; what moved is whether the fixture still exercises it. Swept over
    /// 300…460 pt, the axis turns at 360 and 380 under either reserve.
    /// </para>
    /// </remarks>
    [Fact]
    public void AHorizontalValueAxisTurnsItsLabelsWhenTheyCollide()
    {
        DocRect narrow =
            new(Length.Zero, Length.Zero, Length.FromPoints(380), Length.FromPoints(200));

        ChartDrawing drawing = ChartLayout.Place(
            Savings("\"$\"#,##0"), narrow, new Ruler());

        ChartLabel[] money =
            [.. drawing.Labels.Where(label => !double.IsNaN(Number(label.Text)))];

        money.Length.ShouldBe(8);
        money.ShouldAllBe(label => label.Rotation > 0.7 && label.Rotation < 0.8);
    }

    /// <summary>
    /// A value axis running down the left is not arranged here, whatever its labels do.
    /// </summary>
    /// <remarks>
    /// One label per tick on separate lines cannot collide with itself, and
    /// <see cref="ChartLayout"/>'s rotation path for a value axis is written for the bottom edge
    /// alone. The same numbers as a column chart must therefore stay upright.
    /// </remarks>
    [Fact]
    public void AVerticalValueAxisIsNotArrangedHere()
    {
        ChartPlot columns = Savings("\"$\"#,##0") with { Direction = ChartBarDirection.Column };

        ChartDrawing drawing = ChartLayout.Place(columns, Frame(), new Ruler());

        drawing.Labels
            .Where(label => !double.IsNaN(Number(label.Text)))
            .ShouldAllBe(label => label.Rotation == 0.0);
    }

    /// <summary>
    /// A stated rotation still wins: the file saying so outranks any arrangement.
    /// </summary>
    [Fact]
    public void AStatedRotationIsNotOverriddenByTheArrangement()
    {
        ChartPlot stated = Savings("\"$\"#,##0") with
        {
            ValueAxisText = new ChartAxisText(Rotation: -Math.PI / 4.0),
        };

        ChartDrawing drawing = ChartLayout.Place(stated, Frame(), new Ruler());

        drawing.Labels
            .Where(label => !double.IsNaN(Number(label.Text)))
            .ShouldAllBe(label => label.Rotation < -0.7 && label.Rotation > -0.8);
    }
}
