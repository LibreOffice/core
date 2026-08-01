using System.Text;
using Paperless.Core.Numbers;

namespace Paperless.Core.Charts;

/// <summary>
/// A trendline fitted to a series: the curve it draws and the equation it writes.
/// </summary>
/// <remarks>
/// <para>
/// A port of <c>chart2/source/tools/RegressionCurveCalculator.cxx</c> and its six subclasses,
/// flattened into one type because the polymorphism there buys a UNO service name and nothing
/// else — every subclass is a <c>recalculateRegression</c>, a <c>getCurveValue</c> and an
/// <c>ImplGetRepresentation</c>, which are the three members here.
/// </para>
/// <para>
/// <strong>Each fit throws away different points, and that is the part a reimplementation gets
/// wrong.</strong> <c>RegressionCalculationHelper::cleanup</c> takes a predicate per curve type:
/// a linear fit keeps every finite pair, a logarithmic one keeps only <c>x &gt; 0</c>, a power one
/// only <c>x &gt; 0 &amp;&amp; y &gt; 0</c>, and an exponential one keeps <c>y &gt; 0</c> —
/// falling back to <c>y &lt; 0</c> for the whole series and negating, which is how LibreOffice
/// fits an exponential through data that is entirely below the axis. Fitting over the raw pairs
/// instead produces <c>NaN</c> where the reference produces a curve.
/// </para>
/// </remarks>
public sealed class ChartRegression
{
    private readonly ChartTrendlineKind _kind;
    private readonly double[] _coefficients;
    private readonly double[] _movingX;
    private readonly double[] _movingY;
    private readonly double _slope;
    private readonly double _intercept;
    private readonly double _sign;
    private readonly int _order;
    private readonly int _period;
    private readonly bool _forced;

    private ChartRegression(
        ChartTrendlineKind kind,
        double[] coefficients,
        double[] movingX,
        double[] movingY,
        double slope,
        double intercept,
        double sign,
        double correlation,
        int order,
        int period,
        bool forced)
    {
        _kind = kind;
        _coefficients = coefficients;
        _movingX = movingX;
        _movingY = movingY;
        _slope = slope;
        _intercept = intercept;
        _sign = sign;
        _order = order;
        _period = period;
        _forced = forced;
        Correlation = correlation;
    }

    /// <summary>
    /// The correlation coefficient <em>r</em>, whose square is what the equation writes.
    /// </summary>
    /// <remarks>
    /// Named as LibreOffice names it — <c>m_fCorrelationCoefficient</c> — because for a polynomial
    /// fit it is <em>not</em> a correlation but the square root of the coefficient of
    /// determination, computed from the residuals directly
    /// (<c>PolynomialRegressionCurveCalculator::computeCorrelationCoefficient</c>), and the label
    /// squares it again. Keeping the same name keeps the port checkable against the original.
    /// </remarks>
    public double Correlation { get; }

    /// <summary>Whether the fit produced anything drawable.</summary>
    public bool IsUsable => _kind switch
    {
        ChartTrendlineKind.MovingAverage => _movingX.Length >= 2,
        ChartTrendlineKind.Polynomial or ChartTrendlineKind.Linear => _coefficients.Length > 0,
        _ => !double.IsNaN(_slope) && !double.IsNaN(_intercept),
    };

    /// <summary>Fits a trendline to a series' points.</summary>
    /// <param name="trendline">What curve to fit and with what constraints.</param>
    /// <param name="xs">The X values, one per point; a category chart's are 1, 2, 3 …</param>
    /// <param name="ys">The Y values, one per point. Null and non-finite entries are gaps.</param>
    public static ChartRegression Fit(
        ChartTrendline trendline, IReadOnlyList<double?> xs, IReadOnlyList<double?> ys)
    {
        ArgumentNullException.ThrowIfNull(trendline);
        ArgumentNullException.ThrowIfNull(xs);
        ArgumentNullException.ThrowIfNull(ys);

        return trendline.Kind switch
        {
            ChartTrendlineKind.Mean => FitMean(ys),
            ChartTrendlineKind.MovingAverage => FitMoving(trendline, xs, ys),
            ChartTrendlineKind.Logarithmic => FitLogarithmic(xs, ys),
            ChartTrendlineKind.Power => FitPower(xs, ys),
            ChartTrendlineKind.Exponential => FitExponential(trendline, xs, ys),
            ChartTrendlineKind.Polynomial => FitPolynomial(trendline, trendline.Order, xs, ys),
            _ => FitPolynomial(trendline, 1, xs, ys),
        };
    }

    /// <summary>The fitted Y at one X, or <c>NaN</c> where the curve has no value there.</summary>
    public double ValueAt(double x)
    {
        switch (_kind)
        {
            case ChartTrendlineKind.Mean:
                return _intercept;

            case ChartTrendlineKind.MovingAverage:
                return double.NaN;

            case ChartTrendlineKind.Logarithmic:
                return double.IsNaN(_slope) || x <= 0.0
                    ? double.NaN
                    : (_slope * Math.Log(x)) + _intercept;

            case ChartTrendlineKind.Power:
                return double.IsNaN(_slope) ? double.NaN : _intercept * Math.Pow(x, _slope);

            case ChartTrendlineKind.Exponential:
                return double.IsNaN(_slope) || double.IsNaN(_intercept)
                    ? double.NaN
                    : _sign * Math.Exp(_intercept + (x * _slope));

            default:
                if (_coefficients.Length == 0) return double.NaN;

                // Horner's method, as PolynomialRegressionCurveCalculator::getCurveValue.
                double result = 0.0;
                for (int at = _coefficients.Length - 1; at >= 0; at--)
                    result = _coefficients[at] + (x * result);

                return result;
        }
    }

    /// <summary>
    /// The points the curve is drawn through, between two X values.
    /// </summary>
    /// <remarks>
    /// <c>RegressionCurveCalculator::getCurveValues</c> samples 100 points across the range, and
    /// each subclass short-circuits to the two endpoints when the axis scaling makes the curve a
    /// straight line in screen space. A moving average ignores the range entirely and returns the
    /// averages it computed, which is why it is the one kind whose curve does not pass through a
    /// function of X.
    /// </remarks>
    /// <param name="minimum">The first X.</param>
    /// <param name="maximum">The last X.</param>
    /// <param name="count">How many samples; at least two.</param>
    public IEnumerable<(double X, double Y)> Curve(double minimum, double maximum, int count)
    {
        if (_kind == ChartTrendlineKind.MovingAverage)
        {
            int shared = Math.Min(_movingX.Length, _movingY.Length);
            for (int at = 0; at < shared; at++) yield return (_movingX[at], _movingY[at]);
            yield break;
        }

        if (count < 2) count = 2;

        // A straight line in the plot's own space needs no sampling, and both endpoints are exact
        // where a sampled midpoint is not.
        if (_kind is ChartTrendlineKind.Linear or ChartTrendlineKind.Mean
            || (_kind == ChartTrendlineKind.Polynomial && _coefficients.Length <= 2))
        {
            yield return (minimum, ValueAt(minimum));
            yield return (maximum, ValueAt(maximum));
            yield break;
        }

        double step = (maximum - minimum) / (count - 1);
        for (int at = 0; at < count; at++)
        {
            double x = minimum + (at * step);
            yield return (x, ValueAt(x));
        }
    }

    /// <summary>
    /// The equation and R² as the chart writes them, or null when neither is shown.
    /// </summary>
    /// <remarks>
    /// <c>VSeriesPlotter::createRegressionCurveEquationShapes</c>: the equation first, then a
    /// newline, then <c>R² = </c> and the <em>square</em> of <see cref="Correlation"/>, both
    /// through the equation's own number format — which defaults to key 0, the formatter's
    /// standard index, and so to <see cref="NumberFormatter.General"/> here.
    /// </remarks>
    /// <param name="trendline">Which of the two halves are shown.</param>
    /// <param name="format">The equation's number format, or null for the general one.</param>
    /// <param name="xName">The name the equation gives the independent variable.</param>
    /// <param name="yName">The name it gives the dependent one.</param>
    public string? Equation(
        ChartTrendline trendline,
        NumberFormatCode? format = null,
        string xName = "x",
        string yName = "f(x)")
    {
        ArgumentNullException.ThrowIfNull(trendline);

        if (!trendline.ShowEquation && !trendline.ShowRSquared) return null;

        StringBuilder text = new();

        if (trendline.ShowEquation)
        {
            text.Append(Representation(format, xName, yName));
            if (trendline.ShowRSquared) text.Append('\n');
        }

        if (trendline.ShowRSquared)
        {
            text.Append("R² = ");
            text.Append(Write(Correlation * Correlation, format));
        }

        return text.ToString();
    }

    /// <summary>The equation alone, without R².</summary>
    private string Representation(NumberFormatCode? format, string xName, string yName)
        => _kind switch
        {
            ChartTrendlineKind.Mean => yName + " = " + Write(_intercept, format),
            ChartTrendlineKind.MovingAverage => "Moving average trend line with period = "
                + _period.ToString(System.Globalization.CultureInfo.InvariantCulture),
            ChartTrendlineKind.Logarithmic => Logarithmic(format, xName, yName),
            ChartTrendlineKind.Power => Power(format, xName, yName),
            ChartTrendlineKind.Exponential => Exponential(format, xName, yName),
            _ => Polynomial(format, xName, yName),
        };

    /// <summary>The minus sign LibreOffice writes in an equation.</summary>
    /// <remarks>
    /// <c>aMinusSign</c> is U+2212 MINUS SIGN and not the hyphen
    /// (<c>chart2/source/inc/SpecialCharacters.hxx</c>), which is what the reference PDF draws.
    /// </remarks>
    private const char Minus = '−';

    /// <summary>The superscript digits a polynomial's powers are written with.</summary>
    /// <remarks><c>aSuperscriptFigures</c>, <c>SpecialCharacters.hxx</c>.</remarks>
    private const string Superscripts = "⁰¹²³⁴⁵⁶⁷⁸⁹";

    private static string Write(double value, NumberFormatCode? format)
        => double.IsNaN(value) ? "NaN" : ChartDataLabel.Write(value, format);

    /// <summary><c>PolynomialRegressionCurveCalculator::ImplGetRepresentation</c>.</summary>
    private string Polynomial(NumberFormatCode? format, string xName, string yName)
    {
        StringBuilder text = new(yName + " = ");
        int start = text.Length;
        bool any = false;

        for (int at = _coefficients.Length - 1; at >= 0; at--)
        {
            double value = _coefficients[at];
            if (value == 0.0) continue;

            if (value < 0.0)
            {
                if (any) text.Append(' ');
                text.Append(Minus).Append(' ');
                value = -value;
            }
            else if (any)
            {
                text.Append(" + ");
            }

            any = true;

            string written = Write(value, format);
            if (at == 0 || written != "1")
            {
                text.Append(written);
                if (at > 0) text.Append(' ');
            }

            if (at > 0)
            {
                text.Append(xName);
                if (at > 1) Superscript(text, at);
            }
        }

        if (text.Length == start) text.Append('0');
        return text.ToString();
    }

    private static void Superscript(StringBuilder text, int power)
    {
        if (power < 10)
        {
            text.Append(Superscripts[power]);
            return;
        }

        foreach (char digit in power.ToString(System.Globalization.CultureInfo.InvariantCulture))
            text.Append(Superscripts[digit - '0']);
    }

    /// <summary><c>LogarithmicRegressionCurveCalculator::ImplGetRepresentation</c>.</summary>
    private string Logarithmic(NumberFormatCode? format, string xName, string yName)
    {
        bool hasSlope = !Approximately(Math.Abs(_slope), 1.0);
        StringBuilder text = new(yName + " = ");
        int start = text.Length;

        if (_slope != 0.0)
        {
            if (_slope < 0.0) text.Append(Minus).Append(' ');

            if (hasSlope)
            {
                string written = Write(Math.Abs(_slope), format);
                if (written != "1") text.Append(written).Append(' ');
            }

            text.Append("ln(").Append(xName).Append(") ");
            if (_intercept > 0.0) text.Append("+ ");
        }

        if (_intercept < 0.0) text.Append(Minus).Append(' ');

        string interceptText = Write(Math.Abs(_intercept), format);
        if (interceptText != "0") text.Append(interceptText);

        if (text.Length == start) text.Append('0');
        return text.ToString();
    }

    /// <summary><c>PotentialRegressionCurveCalculator::ImplGetRepresentation</c>.</summary>
    private string Power(NumberFormatCode? format, string xName, string yName)
    {
        StringBuilder text = new(yName + " = ");

        if (_intercept == 0.0)
        {
            text.Append('0');
            return text.ToString();
        }

        if (_intercept < 0.0) text.Append(Minus).Append(' ');

        if (!Approximately(Math.Abs(_intercept), 1.0))
        {
            string written = Write(Math.Abs(_intercept), format);
            if (written != "1") text.Append(written).Append(' ');
        }

        if (_slope != 0.0)
            text.Append(xName).Append('^').Append(Write(_slope, format));

        return text.ToString();
    }

    /// <summary><c>ExponentialRegressionCurveCalculator::ImplGetRepresentation</c>.</summary>
    private string Exponential(NumberFormatCode? format, string xName, string yName)
    {
        double intercept = Math.Exp(_intercept);
        bool hasSlope = !Approximately(Math.Exp(_slope), 1.0);
        bool hasLogSlope = !Approximately(Math.Abs(_slope), 1.0);
        bool hasIntercept = !Approximately(intercept, 1.0) && intercept != 0.0;

        StringBuilder text = new(yName + " = ");
        if (_sign < 0.0) text.Append(Minus).Append(' ');

        if (hasIntercept)
        {
            string written = Write(intercept, format);
            if (written != "1") text.Append(written).Append(' ');
        }

        text.Append("exp( ");

        if (!hasIntercept && (intercept == 0.0 || (!hasSlope && _intercept != 0.0)))
        {
            string written = Write(_intercept, format);
            if (written != "0") text.Append(written).Append(_slope < 0.0 ? " " : " + ");
        }

        if (_slope < 0.0) text.Append(Minus).Append(' ');

        if (hasLogSlope)
        {
            string written = Write(Math.Abs(_slope), format);
            if (written != "1") text.Append(written).Append(' ');
        }

        text.Append(xName).Append(" )");
        return text.ToString();
    }

    /// <summary>
    /// <c>rtl::math::approxEqual</c>, reduced to the case the equation writers use.
    /// </summary>
    /// <remarks>
    /// The C++ compares against the eleventh significant bit from the end; comparing the relative
    /// difference against 2⁻⁴⁸ is the same test for the magnitudes an equation coefficient has and
    /// is what decides whether a coefficient of one is written at all.
    /// </remarks>
    private static bool Approximately(double a, double b)
    {
        if (a == b) return true;
        if (double.IsNaN(a) || double.IsNaN(b)) return false;

        double difference = Math.Abs(a - b);
        double scale = Math.Max(Math.Abs(a), Math.Abs(b));
        return difference <= scale * 3.55e-15;
    }

    private static ChartRegression Empty(ChartTrendlineKind kind)
        => new(kind, [], [], [], double.NaN, double.NaN, 1.0, double.NaN, 1, 2, false);

    /// <summary>Every pair both sequences have a finite value for, in order.</summary>
    /// <remarks><c>RegressionCalculationHelper::cleanup</c> with a predicate.</remarks>
    private static (List<double> X, List<double> Y) Clean(
        IReadOnlyList<double?> xs, IReadOnlyList<double?> ys, Func<double, double, bool> keep)
    {
        List<double> cleanX = [];
        List<double> cleanY = [];

        int count = Math.Min(xs.Count, ys.Count);
        for (int at = 0; at < count; at++)
        {
            if (xs[at] is not { } x || !double.IsFinite(x)) continue;
            if (ys[at] is not { } y || !double.IsFinite(y)) continue;
            if (!keep(x, y)) continue;

            cleanX.Add(x);
            cleanY.Add(y);
        }

        return (cleanX, cleanY);
    }

    /// <summary><c>MeanValueRegressionCurveCalculator::recalculateRegression</c>.</summary>
    private static ChartRegression FitMean(IReadOnlyList<double?> ys)
    {
        int count = 0;
        double sum = 0.0;

        foreach (double? point in ys)
        {
            if (point is not { } y || !double.IsFinite(y)) continue;
            sum += y;
            count++;
        }

        if (count == 0) return Empty(ChartTrendlineKind.Mean);

        double mean = sum / count;
        double deviation = 0.0;

        if (count > 1)
        {
            double error = 0.0;
            foreach (double? point in ys)
            {
                if (point is not { } y || !double.IsFinite(y)) continue;
                error += (mean - y) * (mean - y);
            }

            deviation = Math.Sqrt(error / (count - 1));
        }

        return new ChartRegression(
            ChartTrendlineKind.Mean, [], [], [], 0.0, mean, 1.0, deviation, 1, 2, false);
    }

    /// <summary><c>MovingAverageRegressionCurveCalculator::recalculateRegression</c>.</summary>
    private static ChartRegression FitMoving(
        ChartTrendline trendline, IReadOnlyList<double?> xs, IReadOnlyList<double?> ys)
    {
        (List<double> x, List<double> y) = Clean(xs, ys, static (_, _) => true);

        int period = Math.Max(1, trendline.Period);
        List<double> outX = [];
        List<double> outY = [];

        if (trendline.Moving == ChartMovingAverage.Central)
        {
            for (int at = period - 1; at < y.Count; at++)
            {
                double sum = 0.0;
                for (int back = 0; back < period; back++) sum += y[at - back];
                outY.Add(sum / period);
            }

            int half = period % 2 == 0 ? period / 2 : (period - 1) / 2;
            for (int at = half; at < x.Count - 1; at++) outX.Add(x[at]);
        }
        else
        {
            bool averaged = trendline.Moving == ChartMovingAverage.AveragedAbscissa;

            for (int at = period - 1; at < x.Count; at++)
            {
                double sumX = 0.0;
                double sumY = 0.0;

                for (int back = 0; back < period; back++)
                {
                    sumX += x[at - back];
                    sumY += y[at - back];
                }

                outY.Add(sumY / period);
                outX.Add(averaged ? sumX / period : x[at]);
            }
        }

        return new ChartRegression(
            ChartTrendlineKind.MovingAverage, [], [.. outX], [.. outY],
            double.NaN, double.NaN, 1.0, double.NaN, 1, period, false);
    }

    /// <summary><c>LogarithmicRegressionCurveCalculator::recalculateRegression</c>.</summary>
    private static ChartRegression FitLogarithmic(
        IReadOnlyList<double?> xs, IReadOnlyList<double?> ys)
    {
        (List<double> x, List<double> y) = Clean(xs, ys, static (px, _) => px > 0.0);
        if (x.Count <= 1) return Empty(ChartTrendlineKind.Logarithmic);

        List<double> logs = new(x.Count);
        foreach (double point in x) logs.Add(Math.Log(point));

        (double slope, double intercept, double correlation) = LeastSquares(logs, y);
        return new ChartRegression(
            ChartTrendlineKind.Logarithmic, [], [], [],
            slope, intercept, 1.0, correlation, 1, 2, false);
    }

    /// <summary><c>PotentialRegressionCurveCalculator::recalculateRegression</c>.</summary>
    private static ChartRegression FitPower(IReadOnlyList<double?> xs, IReadOnlyList<double?> ys)
    {
        double sign = 1.0;
        (List<double> x, List<double> y) =
            Clean(xs, ys, static (px, py) => px > 0.0 && py > 0.0);

        if (x.Count <= 1)
        {
            (x, y) = Clean(xs, ys, static (px, py) => px > 0.0 && py < 0.0);
            if (x.Count <= 1) return Empty(ChartTrendlineKind.Power);
            sign = -1.0;
        }

        List<double> logX = new(x.Count);
        List<double> logY = new(y.Count);
        foreach (double point in x) logX.Add(Math.Log(point));
        foreach (double point in y) logY.Add(Math.Log(sign * point));

        (double slope, double intercept, double correlation) = LeastSquares(logX, logY);
        return new ChartRegression(
            ChartTrendlineKind.Power, [], [], [],
            slope, sign * Math.Exp(intercept), sign, correlation, 1, 2, false);
    }

    /// <summary><c>ExponentialRegressionCurveCalculator::recalculateRegression</c>.</summary>
    private static ChartRegression FitExponential(
        ChartTrendline trendline, IReadOnlyList<double?> xs, IReadOnlyList<double?> ys)
    {
        double sign = 1.0;
        (List<double> x, List<double> y) = Clean(xs, ys, static (_, py) => py > 0.0);

        if (x.Count <= 1)
        {
            (x, y) = Clean(xs, ys, static (_, py) => py < 0.0);
            if (x.Count <= 1) return Empty(ChartTrendlineKind.Exponential);
            sign = -1.0;
        }

        bool forced = trendline.Intercept is { } stated && sign * stated > 0.0;
        double logIntercept = forced ? Math.Log(sign * trendline.Intercept!.Value) : 0.0;

        List<double> logY = new(y.Count);
        foreach (double point in y) logY.Add(Math.Log(sign * point) - (forced ? logIntercept : 0.0));

        double averageX = 0.0;
        double averageY = 0.0;

        if (!forced)
        {
            foreach (double point in x) averageX += point;
            foreach (double point in logY) averageY += point;
            averageX /= x.Count;
            averageY /= logY.Count;
        }

        double qx = 0.0;
        double qy = 0.0;
        double qxy = 0.0;

        for (int at = 0; at < x.Count; at++)
        {
            double dx = x[at] - averageX;
            double dy = logY[at] - averageY;
            qx += dx * dx;
            qy += dy * dy;
            qxy += dx * dy;
        }

        double slope = qxy / qx;
        return new ChartRegression(
            ChartTrendlineKind.Exponential, [], [], [],
            slope,
            forced ? logIntercept : averageY - (slope * averageX),
            sign,
            qxy / Math.Sqrt(qx * qy),
            1, 2, forced);
    }

    /// <summary>The centred least-squares fit the three logarithm-space curves share.</summary>
    private static (double Slope, double Intercept, double Correlation) LeastSquares(
        List<double> xs, List<double> ys)
    {
        double averageX = 0.0;
        double averageY = 0.0;

        foreach (double point in xs) averageX += point;
        foreach (double point in ys) averageY += point;
        averageX /= xs.Count;
        averageY /= ys.Count;

        double qx = 0.0;
        double qy = 0.0;
        double qxy = 0.0;

        for (int at = 0; at < xs.Count; at++)
        {
            double dx = xs[at] - averageX;
            double dy = ys[at] - averageY;
            qx += dx * dx;
            qy += dy * dy;
            qxy += dx * dy;
        }

        double slope = qxy / qx;
        return (slope, averageY - (slope * averageX), qxy / Math.Sqrt(qx * qy));
    }

    /// <summary>
    /// <c>PolynomialRegressionCurveCalculator::recalculateRegression</c>, degrees 1 and above.
    /// </summary>
    /// <remarks>
    /// Degree 1 has a closed form of its own in the original — the same one Calc's <c>LINEST</c>
    /// uses — and it is not merely an optimisation: it centres the data before the dot products,
    /// which is what makes a fit over X values in the thousands agree with the reference's to the
    /// digit an equation prints. Above degree 1 the fit is a QR decomposition by Householder
    /// reflectors over the Vandermonde matrix, transcribed from the same file.
    /// </remarks>
    private static ChartRegression FitPolynomial(
        ChartTrendline trendline, int degree, IReadOnlyList<double?> xs, IReadOnlyList<double?> ys)
    {
        (List<double> x, List<double> y) = Clean(xs, ys, static (_, _) => true);
        int count = x.Count;
        if (count == 0) return Empty(ChartTrendlineKind.Polynomial);

        degree = Math.Clamp(degree, 1, 20);
        bool forced = trendline.Intercept is not null;
        double interceptValue = trendline.Intercept ?? 0.0;
        int powers = forced ? degree : degree + 1;

        double[] coefficients = new double[powers];
        double[] target = new double[count];
        double averageY = 0.0;

        for (int at = 0; at < count; at++)
        {
            double value = forced ? y[at] - interceptValue : y[at];
            target[at] = value;
            averageY += value;
        }

        averageY /= count;

        if (degree == 1)
        {
            double[] centred = new double[count];
            double averageX = 0.0;

            for (int at = 0; at < count; at++)
            {
                centred[at] = x[at];
                averageX += x[at];
            }

            averageX /= count;

            if (!forced)
            {
                for (int at = 0; at < count; at++)
                {
                    centred[at] -= averageX;
                    target[at] -= averageY;
                }
            }

            double sumXY = 0.0;
            double sumX2 = 0.0;
            for (int at = 0; at < count; at++)
            {
                sumXY += centred[at] * target[at];
                sumX2 += centred[at] * centred[at];
            }

            double slope = sumX2 == 0.0 ? 0.0 : sumXY / sumX2;

            coefficients = forced
                ? [interceptValue, slope]
                : [averageY - (slope * averageX), slope];

            return Polynomial(
                trendline.Kind, coefficients, x, y, averageY, forced, interceptValue);
        }

        // QR decomposition by Householder reflectors, transposed exactly as the original stores it.
        double[] qr = new double[count * powers];
        for (int column = 0; column < powers; column++)
        {
            int power = forced ? column + 1 : column;
            for (int row = 0; row < count; row++)
                qr[row + (column * count)] = Math.Pow(x[row], power);
        }

        int minor = Math.Min(count, powers);
        double[] diagonal = new double[minor];

        for (int step = 0; step < minor; step++)
        {
            double norm = 0.0;
            for (int row = step; row < count; row++)
            {
                double c = qr[row + (step * count)];
                norm += c * c;
            }

            double a = qr[step + (step * count)] > 0.0 ? -Math.Sqrt(norm) : Math.Sqrt(norm);
            diagonal[step] = a;
            if (a == 0.0) continue;

            qr[step + (step * count)] -= a;

            for (int column = step + 1; column < powers; column++)
            {
                double alpha = 0.0;
                for (int row = step; row < count; row++)
                    alpha -= qr[row + (column * count)] * qr[row + (step * count)];

                alpha /= a * qr[step + (step * count)];

                for (int row = step; row < count; row++)
                    qr[row + (column * count)] -= alpha * qr[row + (step * count)];
            }
        }

        for (int step = 0; step < minor; step++)
        {
            double dot = 0.0;
            for (int row = step; row < count; row++) dot += target[row] * qr[row + (step * count)];
            dot /= diagonal[step] * qr[step + (step * count)];

            for (int row = step; row < count; row++)
                target[row] += dot * qr[row + (step * count)];
        }

        for (int row = diagonal.Length - 1; row >= 0; row--)
        {
            target[row] /= diagonal[row];
            double solved = target[row];
            coefficients[row] = solved;

            for (int above = 0; above < row; above++)
                target[above] -= solved * qr[above + (row * count)];
        }

        if (forced) coefficients = [interceptValue, .. coefficients];

        return Polynomial(trendline.Kind, coefficients, x, y, averageY, forced, interceptValue);
    }

    /// <summary>
    /// A polynomial fit with its coefficient of determination.
    /// </summary>
    /// <remarks><c>PolynomialRegressionCurveCalculator::computeCorrelationCoefficient</c>.</remarks>
    private static ChartRegression Polynomial(
        ChartTrendlineKind kind,
        double[] coefficients,
        List<double> xs,
        List<double> ys,
        double averageY,
        bool forced,
        double interceptValue)
    {
        ChartRegression fit = new(
            kind, coefficients, [], [], double.NaN, double.NaN, 1.0, double.NaN, 1, 2, forced);

        double error = 0.0;
        double total = 0.0;
        double predicted = 0.0;

        for (int at = 0; at < xs.Count; at++)
        {
            double actual = ys[at];
            double estimate = fit.ValueAt(xs[at]);
            total += (actual - averageY) * (actual - averageY);
            error += (actual - estimate) * (actual - estimate);
            if (forced) predicted += (estimate - interceptValue) * (estimate - interceptValue);
        }

        double determination = 0.0;
        if (forced)
        {
            double divisor = error + predicted;
            if (divisor != 0.0) determination = predicted / divisor;
        }
        else if (total != 0.0)
        {
            determination = 1.0 - (error / total);
        }

        return new ChartRegression(
            kind, coefficients, [], [], double.NaN, double.NaN, 1.0,
            determination > 0.0 ? Math.Sqrt(determination) : 0.0, 1, 2, forced);
    }
}
