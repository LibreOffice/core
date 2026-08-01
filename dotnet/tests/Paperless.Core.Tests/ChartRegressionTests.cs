using Paperless.Core.Charts;
using Shouldly;

namespace Paperless.Core.Tests;

/// <summary>
/// The trendline fits, checked against numbers LibreOffice printed rather than against algebra.
/// </summary>
/// <remarks>
/// A regression is the kind of code that looks obviously right and is quietly wrong: an ordinary
/// least-squares line and the line <c>chart2</c> draws differ on which points are excluded, on
/// whether the intercept is free, and on what R² means when it is not. So the anchor case is the
/// one the reference corpus prints in full — <c>chart2/qa/extras/data/pptx/tdf127720.pptx</c>,
/// whose PDF reads <c>f(x) = 0.0174728496577696 x + 0.60719095698364</c> and
/// <c>R² = 0.999989640055375</c> — and the rest are properties that only hold if the port is the
/// port and not a rewrite.
/// </remarks>
public class ChartRegressionTests
{
    private static readonly double?[] ScatterX = [0.0, 21.0, 100.0];

    private static readonly double?[] ScatterY =
        [0.60975609756097671, 0.97087378640776678, 2.3551577955723042];

    /// <summary>
    /// The corpus scatter chart's own equation, to the digits the reference prints.
    /// </summary>
    /// <remarks>
    /// <strong>The last significant digit is LibreOffice's number formatter and not the fit.</strong>
    /// The reference writes <c>0.0174728496577696</c> where fifteen-significant-digit rounding of
    /// the same double gives <c>…697</c>: <c>rtl::math::doubleToUString</c> rounds the
    /// <em>shortest round-tripping decimal</em> that Dragonbox produced rather than the binary
    /// value, so the two disagree in the fifteenth digit and nowhere else. Asserting the first
    /// fourteen is asserting the fit; asserting the fifteenth would be asserting a rounding mode
    /// that no chart depends on.
    /// </remarks>
    [Fact]
    public void ALinearFitReproducesTheReferenceEquationAndItsRSquared()
    {
        ChartTrendline trendline = new()
        {
            Kind = ChartTrendlineKind.Linear,
            ShowEquation = true,
            ShowRSquared = true,
        };

        ChartRegression fit = ChartRegression.Fit(trendline, ScatterX, ScatterY);

        fit.IsUsable.ShouldBeTrue();
        fit.ValueAt(0.0).ShouldBe(0.60719095698364, 1e-12);
        fit.ValueAt(100.0).ShouldBe(0.60719095698364 + 1.74728496577696, 1e-10);

        string equation = fit.Equation(trendline).ShouldNotBeNull();
        equation.ShouldStartWith("f(x) = 0.017472849657769");
        equation.ShouldContain(" x + 0.6071909569836");
        equation.ShouldContain("R² = 0.999989640055375");
    }

    /// <summary>Neither half is written when the file asks for neither.</summary>
    /// <remarks>
    /// The flags are read as true when unstated — see <see cref="ChartTrendline"/> — so the case
    /// that has to work is the one where they are explicitly off, which is what a fitted line with
    /// no equation beside it is.
    /// </remarks>
    [Fact]
    public void ATrendlineWithNeitherFlagWritesNothing()
    {
        ChartTrendline trendline = new() { Kind = ChartTrendlineKind.Linear };
        ChartRegression.Fit(trendline, ScatterX, ScatterY).Equation(trendline).ShouldBeNull();
    }

    /// <summary>
    /// A forced intercept moves the line and changes what R² means.
    /// </summary>
    /// <remarks>
    /// <c>PolynomialRegressionCurveCalculator::computeCorrelationCoefficient</c> switches formula
    /// when the intercept is forced: <c>Σ(ŷ−c)² / (Σ(y−ŷ)² + Σ(ŷ−c)²)</c> instead of
    /// <c>1 − SSE/SST</c>. Reusing the free-fit formula gives a <em>negative</em> R² on a badly
    /// constrained fit, which then square-roots to zero and reads as "no correlation" rather than
    /// as the wrong formula.
    /// </remarks>
    [Fact]
    public void AForcedInterceptIsHonouredAndIsNotTheFreeFit()
    {
        double?[] xs = [1.0, 2.0, 3.0, 4.0];
        double?[] ys = [3.0, 5.0, 7.0, 9.0];

        ChartTrendline free = new() { Kind = ChartTrendlineKind.Linear, ShowEquation = true };
        ChartTrendline forced = free with { Intercept = 0.0 };

        ChartRegression.Fit(free, xs, ys).ValueAt(0.0).ShouldBe(1.0, 1e-9);
        ChartRegression.Fit(forced, xs, ys).ValueAt(0.0).ShouldBe(0.0, 1e-12);
        ChartRegression.Fit(forced, xs, ys).Correlation.ShouldBeGreaterThan(0.0);
    }

    /// <summary>
    /// A logarithmic fit throws away the points it cannot take a logarithm of.
    /// </summary>
    /// <remarks>
    /// <c>RegressionCalculationHelper::cleanup</c> with <c>isValidAndXPositive</c>. Fitting over
    /// the raw pairs instead produces <c>NaN</c> for the whole curve the moment one category sits
    /// at x = 0 — which on a category axis is impossible, the first category being 1, and on a
    /// scatter chart is the common case.
    /// </remarks>
    [Fact]
    public void ALogarithmicFitSkipsNonPositiveX()
    {
        ChartTrendline trendline = new()
        {
            Kind = ChartTrendlineKind.Logarithmic,
            ShowEquation = true,
        };

        ChartRegression fit = ChartRegression.Fit(
            trendline, [0.0, 1.0, 2.0, 4.0, 8.0], [99.0, 1.0, 2.0, 3.0, 4.0]);

        fit.IsUsable.ShouldBeTrue();
        double.IsNaN(fit.ValueAt(4.0)).ShouldBeFalse();
        fit.ValueAt(4.0).ShouldBe(3.0, 0.2);
        fit.Equation(trendline).ShouldNotBeNull().ShouldContain("ln(x)");
    }

    /// <summary>
    /// An exponential fit over data that is entirely negative is fitted to its mirror image.
    /// </summary>
    /// <remarks>
    /// <c>ExponentialRegressionCurveCalculator::recalculateRegression</c> retries with
    /// <c>isValidAndYNegative</c> and carries a sign, because <c>log(y)</c> of a negative number
    /// is not a number and the alternative is drawing nothing at all. The equation then opens with
    /// a minus sign.
    /// </remarks>
    [Fact]
    public void AnExponentialFitOverNegativeDataIsSignedRatherThanEmpty()
    {
        ChartTrendline trendline = new()
        {
            Kind = ChartTrendlineKind.Exponential,
            ShowEquation = true,
        };

        ChartRegression fit = ChartRegression.Fit(
            trendline, [1.0, 2.0, 3.0, 4.0], [-2.0, -4.0, -8.0, -16.0]);

        fit.IsUsable.ShouldBeTrue();
        fit.ValueAt(5.0).ShouldBe(-32.0, 0.01);
        fit.Equation(trendline).ShouldNotBeNull().ShouldContain("exp(");
    }

    /// <summary>
    /// A power fit is a straight line in both logarithms and writes an exponent.
    /// </summary>
    [Fact]
    public void APowerFitRecoversTheExponent()
    {
        ChartTrendline trendline = new() { Kind = ChartTrendlineKind.Power, ShowEquation = true };

        ChartRegression fit = ChartRegression.Fit(
            trendline, [1.0, 2.0, 3.0, 4.0], [3.0, 12.0, 27.0, 48.0]);

        fit.ValueAt(5.0).ShouldBe(75.0, 0.01);
        fit.Equation(trendline).ShouldNotBeNull().ShouldContain("x^2");
    }

    /// <summary>
    /// A polynomial above degree one goes through the QR solve and lands on the exact quadratic.
    /// </summary>
    [Fact]
    public void AQuadraticFitRecoversItsOwnCoefficients()
    {
        ChartTrendline trendline = new()
        {
            Kind = ChartTrendlineKind.Polynomial,
            Order = 2,
            ShowEquation = true,
            ShowRSquared = true,
        };

        // y = 2x² − 3x + 1 exactly.
        double?[] xs = [1.0, 2.0, 3.0, 4.0, 5.0];
        double?[] ys = [0.0, 3.0, 10.0, 21.0, 36.0];

        ChartRegression fit = ChartRegression.Fit(trendline, xs, ys);

        fit.ValueAt(6.0).ShouldBe(55.0, 1e-6);
        fit.Correlation.ShouldBe(1.0, 1e-9);

        string equation = fit.Equation(trendline).ShouldNotBeNull();
        equation.ShouldContain("x²");
        equation.ShouldContain("−");
    }

    /// <summary>
    /// A moving average is a run of points and not a function of x.
    /// </summary>
    /// <remarks>
    /// <c>MovingAverageRegressionCurveCalculator::getCurveValue</c> returns <c>NaN</c> for every
    /// x, and its <c>getCurveValues</c> ignores the range it is given entirely — the curve is the
    /// averages it computed and nothing else. Sampling it like the other five draws a flat line at
    /// no height at all.
    /// </remarks>
    [Fact]
    public void AMovingAverageIsItsOwnPointsAndNotASampledCurve()
    {
        ChartTrendline trendline = new()
        {
            Kind = ChartTrendlineKind.MovingAverage,
            Period = 2,
            ShowEquation = true,
        };

        ChartRegression fit = ChartRegression.Fit(
            trendline, [1.0, 2.0, 3.0, 4.0], [10.0, 20.0, 30.0, 40.0]);

        (double X, double Y)[] curve = [.. fit.Curve(0.0, 99.0, 100)];

        curve.Length.ShouldBe(3);
        curve[0].ShouldBe((2.0, 15.0));
        curve[2].ShouldBe((4.0, 35.0));

        fit.Equation(trendline).ShouldNotBeNull()
            .ShouldBe("Moving average trend line with period = 2");
    }

    /// <summary>A mean-value line is flat at the average and knows its own standard deviation.</summary>
    [Fact]
    public void AMeanValueLineIsFlatAtTheAverage()
    {
        ChartTrendline trendline = new() { Kind = ChartTrendlineKind.Mean, ShowEquation = true };

        ChartRegression fit = ChartRegression.Fit(trendline, [1.0, 2.0, 3.0], [2.0, 4.0, 6.0]);

        fit.ValueAt(-100.0).ShouldBe(4.0);
        fit.ValueAt(100.0).ShouldBe(4.0);
        fit.Equation(trendline).ShouldNotBeNull().ShouldBe("f(x) = 4");
    }

    /// <summary>A fit with nothing to fit says so rather than drawing a line through nowhere.</summary>
    [Fact]
    public void AFitWithTooFewPointsIsNotUsable()
    {
        ChartTrendline trendline = new() { Kind = ChartTrendlineKind.Logarithmic };
        ChartRegression.Fit(trendline, [0.0, -1.0], [1.0, 2.0]).IsUsable.ShouldBeFalse();
    }
}
