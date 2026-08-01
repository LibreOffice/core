using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Core.Charts;

/// <summary>What curve a trendline fits through its series' points.</summary>
/// <remarks>
/// The six OOXML spells in <c>c:trendlineType</c> plus the mean-value line ODF alone can state.
/// Each names one of <c>chart2/source/tools/</c>'s <c>RegressionCurveCalculator</c> subclasses,
/// and <see cref="ChartRegression"/> is a port of those rather than a reimplementation — the
/// arithmetic is short enough that reinventing it looks safe and is not: an ordinary least
/// squares fit and the fit LibreOffice draws differ on which points are excluded, which is what
/// decides whether a logarithmic trendline over a series containing a zero draws at all.
/// </remarks>
public enum ChartTrendlineKind
{
    /// <summary>A straight line — <c>c:trendlineType val="linear"</c>, ODF <c>linear</c>.</summary>
    Linear = 0,

    /// <summary>A polynomial of <see cref="ChartTrendline.Order"/> — <c>poly</c>.</summary>
    Polynomial,

    /// <summary>An exponential, <c>b·exp(a·x)</c> — <c>exp</c>.</summary>
    Exponential,

    /// <summary>A logarithm, <c>a·ln(x) + b</c> — <c>log</c>.</summary>
    Logarithmic,

    /// <summary>A power law, <c>b·x^a</c> — <c>power</c>, ODF's <c>power</c>.</summary>
    Power,

    /// <summary>A moving average over <see cref="ChartTrendline.Period"/> points — <c>movingAvg</c>.</summary>
    MovingAverage,

    /// <summary>
    /// A horizontal line at the series' mean, which only ODF states.
    /// </summary>
    /// <remarks>
    /// <c>chart:mean-value</c> rather than <c>chart:regression-curve</c>, and it is not a
    /// regression at all — <c>RegressionCurveHelper::isMeanValueLine</c> is tested before every
    /// property read in <c>VSeriesPlotter::createRegressionCurvesShapes</c> precisely so that the
    /// degree, the period and the extrapolation are never consulted for one.
    /// </remarks>
    Mean,
}

/// <summary>
/// How a moving average places the average it computes.
/// </summary>
/// <remarks>
/// ODF's <c>chart:regression-moving-type</c>; OOXML has no counterpart and always means
/// <see cref="Prior"/>. The three cases are the ones
/// <c>MovingAverageRegressionCurveCalculator::recalculateRegression</c> switches on.
/// </remarks>
public enum ChartMovingAverage
{
    /// <summary>The average of the last <em>n</em> points, drawn at the last of them.</summary>
    Prior = 0,

    /// <summary>The average of the last <em>n</em> points, drawn at the middle of them.</summary>
    Central,

    /// <summary>The average of the last <em>n</em> points, drawn at their average X.</summary>
    AveragedAbscissa,
}

/// <summary>
/// One trendline over one series: what curve, how far it runs, and what it writes beside itself.
/// </summary>
/// <remarks>
/// <para>
/// <strong>An unstated <c>c:dispEq</c> means "show it".</strong> <c>TrendlineModel</c>'s
/// constructor takes <c>mbDispEquation( !bMSO2007Doc )</c> and <c>mbDispRSquared( !bMSO2007Doc )</c>
/// (<c>oox/source/drawingml/chart/seriesmodel.cxx:86-92</c>), which is the same rule the five data
/// label flags follow and for the same reason: what Excel writes when it means "no equation" is
/// an explicit <c>val="0"</c>, so a reader that defaults to false is right on every file that
/// states the flag and wrong on every file that does not. ODF is the other way round and states
/// both on <c>chart:equation</c>, whose absence means neither is shown.
/// </para>
/// <para>
/// <strong>The extrapolation is in the X unit and not in points.</strong> <c>c:forward</c> and
/// <c>c:backward</c> extend the fitted range past the data — for a category chart the unit is one
/// category, for a scatter chart it is one X. <c>VSeriesPlotter</c> adds them to the series' own
/// minimum and maximum before asking for the curve and then clips the result to the plot area
/// (<c>VSeriesPlotter.cxx:1352-1356</c>), so a trendline extended past the axis simply stops at
/// the wall rather than drawing outside it.
/// </para>
/// </remarks>
public sealed record ChartTrendline
{
    /// <summary>What curve is fitted.</summary>
    public ChartTrendlineKind Kind { get; init; }

    /// <summary>
    /// The polynomial's degree, for <see cref="ChartTrendlineKind.Polynomial"/>.
    /// </summary>
    /// <remarks>
    /// <c>c:order</c>, ODF's <c>chart:regression-max-degree</c>; both default to 2, which is what
    /// <c>TrendlineModel</c> and <c>RegressionCurveCalculator</c> initialise it to.
    /// </remarks>
    public int Order { get; init; } = 2;

    /// <summary>How many points a moving average covers — <c>c:period</c>; the default is 2.</summary>
    public int Period { get; init; } = 2;

    /// <summary>Where a moving average's average is drawn.</summary>
    public ChartMovingAverage Moving { get; init; }

    /// <summary>How far past the last point the curve runs, in the X unit.</summary>
    public double Forward { get; init; }

    /// <summary>How far before the first point the curve runs, in the X unit.</summary>
    public double Backward { get; init; }

    /// <summary>
    /// The Y intercept the fit is forced through, or null when it is free.
    /// </summary>
    /// <remarks>
    /// <c>c:intercept</c>, ODF's <c>chart:regression-force-intercept</c> plus
    /// <c>chart:regression-intercept-value</c>. A forced intercept changes the R² definition as
    /// well as the fit — <c>PolynomialRegressionCurveCalculator::computeCorrelationCoefficient</c>
    /// takes it as <c>Σ(ŷ−c)² / (Σ(y−ŷ)² + Σ(ŷ−c)²)</c> rather than as <c>1 − SSE/SST</c>, which
    /// is the only place in the file where the two disagree.
    /// </remarks>
    public double? Intercept { get; init; }

    /// <summary>Whether the equation is written beside the curve.</summary>
    public bool ShowEquation { get; init; }

    /// <summary>Whether R² is written beside the curve.</summary>
    public bool ShowRSquared { get; init; }

    /// <summary>The trendline's own name, or null — <c>c:name</c>, <c>chart:regression-name</c>.</summary>
    public string? Name { get; init; }

    /// <summary>The colour the curve is stroked in, or null to take the series'.</summary>
    public Colour? Line { get; init; }

    /// <summary>The curve's width; zero is a hairline.</summary>
    public Length LineWidth { get; init; }

    /// <summary>
    /// Where the equation goes in the chart's own space, or null to place it at the curve.
    /// </summary>
    /// <remarks>
    /// ODF states it outright on <c>chart:equation/@svg:x</c> and <c>@svg:y</c>, which is the same
    /// asymmetry <c>chart:coordinate-region</c> has: the ODF writer bakes the last laid-out
    /// position into the file and OOXML states nothing, so the OOXML path falls back to
    /// <c>VSeriesPlotter</c>'s own default — the top-left of the curve's bounding box
    /// (<c>VSeriesPlotter.cxx:1424-1431</c>, <c>aDefaultPos</c>).
    /// </remarks>
    public (Length X, Length Y)? EquationAt { get; init; }
}
