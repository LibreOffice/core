using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Numbers;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// What the last label of a horizontal value axis overhangs the axis' far end by, upright and
/// turned — which is what the plot rectangle has to give up on that side.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Turned, it has no width in it at all.</strong> A label's shape is autogrown around a
/// centred paragraph, so its top <em>centre</em> is on the tick before any correction —
/// <c>ShapeFactory::makeTransformation</c>'s own comment, <em>"as autogrow is active the
/// rectangle is automatically expanded to that side to which the text is not adjusted"</em>.
/// Rotating that box about the tick puts its right edge at <c>w·cos/2 + h·sin</c>, and
/// <c>lcl_correctRotation_Bottom</c> (<c>chart2/source/view/main/LabelPositionHelper.cxx</c>:241-256)
/// then takes <c>(w·cos + h·sin)/2</c> off it — leaving <strong><c>h·sin/2</c></strong>. Upright
/// that function does nothing at all (its <c>fAnglePositiveDegree==0.0</c> branch is empty), so
/// the centred box stands and the plain half-width overhangs.
/// </para>
/// <para>
/// <strong>Measured against 26.2.4.2 on <c>027_Simple_personal_cash_flow_statement</c>'s savings
/// chart</strong>, whose scale is pinned in every variant so the tick set cannot move
/// (<c>probes/chart-slide-r117/turn027.py</c>, and §2 of that round's <c>results.md</c>):
/// </para>
/// <list type="bullet">
/// <item><description>
/// widening every value label from 19.18 pt of ink to 40.69 by its number format alone moves the
/// drawn plot width by <strong>0.02 pt</strong> in 8 of 8 turned variants, where half the width
/// would have moved it 10.8 — and the last label's right edge stays 2.00 pt past its own tick
/// throughout;
/// </description></item>
/// <item><description>
/// over three stated sizes (6, 9, 14 pt) and four angles (22.5°, 45°, 67.5°, 90°) the reserve is
/// <c>h·sin/2</c> with <c>h</c> the chart's own line height, to within 1.6 %;
/// </description></item>
/// <item><description>
/// the <strong>upright control</strong> — the same chart with a pinned major unit of 14,000, so
/// its two labels do not collide and are not turned — has the last label centred on its tick to
/// 0.04 pt at four label widths, and its plot width moves by half of every widening.
/// </description></item>
/// </list>
/// <para>
/// The turned assertion below fails at <c>88209aa4b</c>, where the reserve was
/// <c>(w·cos + h·sin)/2</c>; the upright one passes there and is the mutation control that says
/// the corrected rule did not simply delete the overhang.
/// </para>
/// </remarks>
public class ChartTurnedValueLabelOverhangTests
{
    /// <summary>A ruler whose width is proportional to the character count.</summary>
    /// <remarks>
    /// The same one <see cref="ChartValueAxisArrangementTests"/> uses, and for the same reason:
    /// what is asserted is which <em>quantity</em> the plot gives up, not how wide a particular
    /// face makes a string.
    /// </remarks>
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
            => new(size * (0.55 * text.Length), size * 1.15);
    }

    /// <summary>The witness' shape: a bar chart whose money axis runs along the bottom.</summary>
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

    /// <summary>
    /// A frame narrow enough that the money labels collide with room to spare.
    /// </summary>
    /// <remarks>
    /// <strong>The width is chosen, and it has to be.</strong> The far-end reserve is subtracted
    /// from the plot rectangle that the collision test is then run against, so on this ruler a
    /// frame within a few points of the threshold can be turned under one reserve and upright
    /// under the other. Swept over 300…460 pt, 360 and 380 turn under both rules and under both
    /// formats; <see cref="ChartValueAxisArrangementTests"/>'s own 420 does not.
    /// </remarks>
    private static DocRect Frame()
        => new(Length.Zero, Length.Zero, Length.FromPoints(380), Length.FromPoints(200));

    private static ChartDrawing Draw(ChartPlot plot)
        => ChartLayout.Place(plot, Frame(), new Ruler());

    /// <summary>Whether the money labels came out turned.</summary>
    private static bool Turned(ChartDrawing drawing)
        => drawing.Labels.Any(label => label.Text.StartsWith('$') && label.Rotation > 0.7);

    /// <summary>
    /// Widening a turned value axis' labels does not move the plot's right edge.
    /// </summary>
    /// <remarks>
    /// The measured arm. Both formats below label the same eight ticks and collide, so both axes
    /// are turned 45°; the second's labels are eight characters wider apiece. Under the rule this
    /// tree had, the plot's right edge would move by <c>8 × 0.55 × 10 × cos45 / 2 ≈ 15.6 pt</c>.
    /// </remarks>
    [Fact]
    public void WideningATurnedValueAxisLabelDoesNotMoveThePlotsFarEdge()
    {
        ChartDrawing plain = Draw(Savings("\"$\"#,##0"));
        ChartDrawing wide = Draw(Savings("\"$\"#,##0\"WWWWWWWW\""));

        Turned(plain).ShouldBeTrue();
        Turned(wide).ShouldBeTrue();

        (wide.PlotArea.Right - plain.PlotArea.Right).Points
            .ShouldBe(0.0, 0.01);
    }

    /// <summary>
    /// And what it does give up is half the label's <em>height</em> times the sine of the angle.
    /// </summary>
    /// <remarks>
    /// Read against the axis with its labels deleted, which reserves nothing for them at all, so
    /// the difference between the two right edges is the reserve itself.
    /// </remarks>
    [Fact]
    public void ATurnedValueAxisGivesUpHalfItsLabelHeightTimesTheSine()
    {
        ChartPlot plot = Savings("\"$\"#,##0");
        ChartDrawing turned = Draw(plot);
        ChartDrawing bare = Draw(plot with { ValueLabelsVisible = false });

        Turned(turned).ShouldBeTrue();

        // The ruler's line height for a 10 pt label, halved and turned through 45 degrees.
        double expected = 10.0 * 1.15 * Math.Sin(Math.PI / 4.0) / 2.0;

        (bare.PlotArea.Right - turned.PlotArea.Right).Points
            .ShouldBe(expected, 0.01);
    }

    /// <summary>
    /// The control: an <em>upright</em> horizontal value axis does give up half its label's width.
    /// </summary>
    /// <remarks>
    /// <strong>Without this the corrected rule would be indistinguishable from deleting the
    /// overhang.</strong> A coarse format keeps the labels short enough not to collide, so the
    /// axis is never turned and <c>lcl_correctRotation_Bottom</c> is never reached; widening them
    /// then moves the plot's right edge by exactly half the widening, which is what 26.2.4.2 does
    /// on the same chart with its major unit pinned so that only two labels are drawn.
    /// </remarks>
    [Fact]
    public void AnUprightValueAxisStillGivesUpHalfItsLabelWidth()
    {
        ChartDrawing plain = Draw(Savings("0,"));
        ChartDrawing wide = Draw(Savings("0,\"WW\""));

        Turned(plain).ShouldBeFalse();
        Turned(wide).ShouldBeFalse();

        // Two more characters of a 10 pt label on the 0.55-em ruler, halved.
        double expected = 2 * 0.55 * 10.0 / 2.0;

        (plain.PlotArea.Right - wide.PlotArea.Right).Points
            .ShouldBe(expected, 0.01);
    }
}
