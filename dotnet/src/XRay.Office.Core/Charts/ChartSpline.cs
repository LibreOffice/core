using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;

namespace XRay.Office.Core.Charts;

/// <summary>
/// The polyline LibreOffice draws for a smoothed line or scatter series.
/// </summary>
/// <remarks>
/// <para>
/// <strong>A smoothed series is not drawn as a curve, it is drawn as a flattened one, and the
/// flattening is the thing to match.</strong> <c>AreaChart::impl_createLine</c> hands the
/// series' points to <c>SplineCalculator::CalculateCubicSplines</c> and strokes the
/// <em>polygon</em> that comes back (<c>chart2/source/view/charttypes/AreaChart.cxx</c>:331-336,
/// this tree) — there is no Bézier anywhere in the path. So the whole of the reference's
/// geometry is: a natural cubic spline through the points, sampled at
/// <see cref="Granularity"/> equal steps per interval.
/// </para>
/// <para>
/// Three decisions are what the picture depends on, and each is stated in
/// <c>chart2/source/view/charttypes/Splines.cxx</c>:520-624 (this tree) and confirmed against
/// 26.2.4.2's own PDF in <c>probes/chart-smooth-r102</c>:
/// </para>
/// <list type="number">
/// <item>
/// <description>
/// <strong>The parameter is the point's index, not its x value.</strong>
/// <c>aParameter[nIndex] = aParameter[nIndex-1] + 1</c> (<c>Splines.cxx</c>:548-552) under the
/// function's own comment, <em>"uniform parametric splines with subinterval length 1, according
/// ODF1.2 part 1, chapter 'chart interpolation'"</em>. Both coordinates are then splined
/// <em>against that parameter</em> — <c>aSplineX</c> and <c>aSplineY</c> are two separate
/// calculations (<c>:571-611</c>). On a scatter series with unevenly spaced x this is visibly
/// not the same curve as splining y against x: the x coordinate itself is interpolated, so the
/// curve may double back.
/// </description>
/// </item>
/// <item>
/// <description>
/// <strong>The end condition is natural.</strong> <c>Splines.cxx</c>:584-591 passes
/// <c>std::numeric_limits&lt;double&gt;::infinity()</c> as both first derivatives, and
/// <c>lcl_SplineCalculation::Calculate</c> reads an infinite derivative as
/// <em>"natural spline"</em> — second derivative zero at both ends (<c>:150-155</c>). The one
/// exception is a series whose first and last points coincide in all three coordinates, which
/// gets a <em>periodic</em> spline instead (<c>:578-583</c>); see <see cref="Flatten"/>.
/// </description>
/// </item>
/// <item>
/// <description>
/// <strong>Twenty segments per interval.</strong> The result is sized
/// <c>nMaxIndexPoints * nGranularity + 1</c> and each interval is stepped by
/// <c>(aParameter[ni+1] - aParameter[ni]) / nGranularity</c> (<c>:594-623</c>), with
/// <c>nGranularity</c> the chart type's <c>CurveResolution</c> —
/// <c>AreaChart</c>'s <c>m_nCurveResolution(20)</c> (<c>AreaChart.cxx</c>:64) and
/// <c>ScatterChartTypeTemplate</c>'s <c>CURVE_RESOLUTION, 20</c>
/// (<c>chart2/source/model/template/ScatterChartTypeTemplate.cxx</c>:64).
/// </description>
/// </item>
/// </list>
/// <para>
/// <strong>The 20 is measured, not read.</strong> The C++ tree is 27.2.0.0.alpha0 and not the
/// 26.2.4.2 binary's source, so the constant was counted in 26.2.4.2's own PDF content streams
/// instead: every smoothed series in the corpus is stroked as exactly
/// <c>(points − 1) × 20</c> <c>l</c> operators — 220 for each of the twelve-point series in the
/// nine <c>advanced_excel</c> witnesses, 60 for a four-point series and 200 for an eleven-point
/// one in <c>microsoft_learn_multi_chart_examples.xlsx</c>. Eleven documents, twenty-nine
/// series, no other granularity fits any of them.
/// </para>
/// <para>
/// <strong>What is deliberately not reproduced: the decimation.</strong> The reference runs
/// <c>lcl_removeDuplicatePoints</c> over the flattened polygon before clipping it
/// (<c>AreaChart.cxx</c>:335), dropping any point that falls in the same cell as the last kept
/// one on a grid of <c>2000 × plotWidth/pageWidth</c> by
/// <c>2000 × plotHeight/pageHeight</c> cells
/// (<c>VCoordinateSystem::getCoordinateSystemResolution</c>, and
/// <c>PlottingPositionHelper::isSameForGivenResolution</c>,
/// <c>chart2/source/view/inc/PlottingPositionHelper.hxx</c>:292-320). It is a drawing-cost
/// optimisation whose worst-case geometric effect is one grid cell — under a twentieth of a
/// point on a corpus chart — and it shows in the operator count and nowhere in the ink: the
/// reference emits 801 segments where the un-decimated flattening of
/// <c>171128IPAP.pptx</c>'s 145-point series would emit 2880. Reproducing it would change the
/// count and not the picture, and this project ranks on ink.
/// </para>
/// </remarks>
public static class ChartSpline
{
    /// <summary>
    /// How many straight segments the reference draws per interval between two data points.
    /// </summary>
    /// <remarks>
    /// <c>CurveResolution</c>, whose default is 20 in both the chart type
    /// (<c>AreaChart.cxx</c>:64) and the template that sets it
    /// (<c>ScatterChartTypeTemplate.cxx</c>:64), and which no corpus document overrides — ODF
    /// spells it <c>chart:spline-resolution</c> and not one of the twelve smoothed renderings in
    /// <c>/home/user/corpus-odf</c> states it. Counted in 26.2.4.2's own output, see the type's
    /// own remarks.
    /// </remarks>
    public const int Granularity = 20;

    /// <summary>
    /// The flattened cubic spline through <paramref name="points"/>, or the points themselves
    /// when there are too few to curve.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The output holds <c>(points.Count − 1) × <see cref="Granularity"/> + 1</c> points, of
    /// which every twentieth is one of the input points copied through verbatim rather than
    /// evaluated — which is what <c>Splines.cxx</c>:600-606 does, under the comment <em>"given
    /// point is surely a curve point"</em>, and what keeps a marker sitting exactly on its own
    /// vertex.
    /// </para>
    /// <para>
    /// <strong>Flattening in page coordinates rather than in the axis' own is the same
    /// curve.</strong> The reference splines the <em>scaled logic</em> positions and transforms
    /// the result afterwards (<c>AreaChart.cxx</c>:331-366); the map from a scaled logic
    /// coordinate to a page one is affine in each axis — including a logarithmic axis, whose
    /// scaling is applied before the spline on both sides — and a cubic spline commutes with an
    /// affine map of either coordinate. Doing it here keeps one code path for the clip,
    /// the markers and the labels.
    /// </para>
    /// </remarks>
    /// <param name="points">The series' points, in order, with no gaps.</param>
    /// <returns>The flattened curve.</returns>
    public static IReadOnlyList<DocPoint> Flatten(IReadOnlyList<DocPoint> points)
    {
        ArgumentNullException.ThrowIfNull(points);

        // "we need at least two points" — Splines.cxx:540-541. One point is not a curve and the
        // reference leaves such a polygon empty rather than sampling it.
        if (points.Count <= 1) return points;

        int last = points.Count - 1;

        double[] parameter = new double[points.Count];
        for (int at = 1; at <= last; at++) parameter[at] = parameter[at - 1] + 1.0;

        double[] xs = new double[points.Count];
        double[] ys = new double[points.Count];
        for (int at = 0; at <= last; at++)
        {
            xs[at] = points[at].X.Points;
            ys[at] = points[at].Y.Points;
        }

        // A closed series gets a periodic spline instead of a natural one — Splines.cxx:578-583,
        // which tests all three coordinates of the first point against the last and needs at
        // least three points. On a category axis the parameter *is* the x coordinate and the two
        // ends can never coincide, so this can only fire on a scatter series that closes.
        bool periodic = last >= 2
            && points[0].X == points[last].X
            && points[0].Y == points[last].Y;

        double[] secondX = periodic
            ? PeriodicSecondDerivatives(parameter, xs)
            : NaturalSecondDerivatives(parameter, xs);
        double[] secondY = periodic
            ? PeriodicSecondDerivatives(parameter, ys)
            : NaturalSecondDerivatives(parameter, ys);

        DocPoint[] curve = new DocPoint[(last * Granularity) + 1];
        int written = 0;

        for (int interval = 0; interval < last; interval++)
        {
            // The stated point is copied, not evaluated.
            curve[written++] = points[interval];

            double step = (parameter[interval + 1] - parameter[interval]) / Granularity;

            for (int step_ = 1; step_ < Granularity; step_++)
            {
                double at = parameter[interval] + (step * step_);

                curve[written++] = new DocPoint(
                    Length.FromPoints(Interpolate(parameter, xs, secondX, at)),
                    Length.FromPoints(Interpolate(parameter, ys, secondY, at)));
            }
        }

        curve[written] = points[last];
        return curve;
    }

    /// <summary>
    /// The second derivatives of the natural cubic spline through a sorted sequence.
    /// </summary>
    /// <remarks>
    /// <c>lcl_SplineCalculation::Calculate</c> with both end derivatives infinite
    /// (<c>Splines.cxx</c>:141-206) — the <c>spline</c> routine of <em>Numerical Recipes in
    /// C</em> §3.3, whose natural branch sets the first second-derivative and the first working
    /// value to zero and takes <c>qn = un = 0</c> at the far end. Written out with the
    /// reference's own index order rather than reshaped, because the backward substitution runs
    /// <c>k = n … 1</c> writing <c>k − 1</c> and an unsigned loop counter is why.
    /// </remarks>
    private static double[] NaturalSecondDerivatives(double[] at, double[] value)
    {
        int n = at.Length - 1;
        double[] second = new double[n + 1];
        double[] work = new double[Math.Max(n, 1)];

        for (int i = 1; i < n; i++)
        {
            double sig = (at[i] - at[i - 1]) / (at[i + 1] - at[i - 1]);
            double p = (sig * second[i - 1]) + 2.0;

            second[i] = (sig - 1.0) / p;

            double u = ((value[i + 1] - value[i]) / (at[i + 1] - at[i]))
                       - ((value[i] - value[i - 1]) / (at[i] - at[i - 1]));

            work[i] = (((6.0 * u) / (at[i + 1] - at[i - 1])) - (sig * work[i - 1])) / p;
        }

        // qn and un are both zero for a natural spline, which reduces the far end to this.
        second[n] = 0.0;

        for (int k = n; k > 0; k--)
            second[k - 1] = (second[k - 1] * second[k]) + work[k - 1];

        return second;
    }

    /// <summary>
    /// The second derivatives of the periodic cubic spline through a closed sequence.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>lcl_SplineCalculation::CalculatePeriodic</c> (<c>Splines.cxx</c>:208-353), which is
    /// algorithm 4.76 of Engeln-Müllges' <em>Numerik-Algorithmen</em>: build the cyclic
    /// tridiagonal system, factor it as <c>Rᵀ D R</c>, solve, and then — the step that makes
    /// this function's output comparable with the natural one — <strong>double every
    /// coefficient</strong> (<c>:347-351</c>), because the periodic solver works in the
    /// polynomial's own <c>c</c> coefficients and <c>GetInterpolatedValue</c> wants second
    /// derivatives. The three-point and four-point cases are closed forms in the reference and
    /// are kept as such here.
    /// </para>
    /// <para>
    /// <strong>No corpus document reaches this.</strong> The branch needs a series whose first
    /// and last points coincide, which a category axis makes impossible and which none of the
    /// four smoothed scatter witnesses does. It is written because the alternative is a silent
    /// difference on a document the corpus does not happen to hold, not because it was measured.
    /// </para>
    /// </remarks>
    private static double[] PeriodicSecondDerivatives(double[] at, double[] value)
    {
        int n = at.Length - 1;
        double[] second = new double[n + 1];

        if (n < 4)
        {
            if (n == 3)
            {
                double d0 = at[1] - at[0];
                double d1 = at[2] - at[1];
                double d2 = at[3] - at[2];
                double factor = 1.5 / ((d0 * d1) + (d1 * d2) + (d2 * d0));
                double y0 = (value[1] - value[0]) / d0;
                double y1 = (value[2] - value[1]) / d1;
                double y2 = (value[0] - value[2]) / d2;

                second[1] = factor * ((y1 * (d2 + d1)) - (y0 * (d0 + d2)));
                second[2] = factor * ((y2 * (d0 + d2)) - (y1 * (d1 + d0)));
                second[3] = factor * ((y0 * (d1 + d0)) - (y2 * (d2 + d1)));
                second[0] = second[3];
            }
            else if (n == 2)
            {
                double d0 = at[1] - at[0];
                double d1 = at[2] - at[1];
                double help = 3.0 * (value[0] - value[1]) / (d0 * d1);

                second[1] = help;
                second[2] = -help;
                second[0] = second[2];
            }

            // n < 2 "should be handled with natural spline, periodic not possible" — the
            // reference leaves the coefficients zero, which is a straight line.
        }
        else
        {
            double[] diagonal = new double[n + 1];
            double[] upper = new double[n + 1];
            double[] u = new double[n + 1];
            double[] d = new double[n + 1];
            double[] right = new double[n - 1];
            double[] r = new double[n];

            for (int i = 1; i < n; i++)
            {
                double before = at[i] - at[i - 1];
                double after = at[i + 1] - at[i];

                diagonal[i] = 2 * (before + after);
                upper[i] = after;
                u[i] = 3 * (((value[i + 1] - value[i]) / after)
                            - ((value[i] - value[i - 1]) / before));
            }

            double lastGap = at[n] - at[n - 1];
            double firstGap = at[1] - at[0];

            diagonal[n] = 2 * (lastGap + firstGap);
            upper[n] = firstGap;
            u[n] = 3 * (((value[1] - value[0]) / firstGap)
                        - ((value[n] - value[n - 1]) / lastGap));

            d[1] = diagonal[1];
            r[1] = upper[1] / d[1];
            right[1] = upper[n] / d[1];

            for (int i = 2; i <= n - 2; i++)
            {
                d[i] = diagonal[i] - (upper[i - 1] * r[i - 1]);
                r[i] = upper[i] / d[i];
                right[i] = -right[i - 1] * upper[i - 1] / d[i];
            }

            d[n - 1] = diagonal[n - 1] - (upper[n - 2] * r[n - 2]);
            r[n - 1] = (upper[n - 1] - (upper[n - 2] * right[n - 2])) / d[n - 1];

            double sum = 0.0;
            for (int i = 1; i <= n - 2; i++) sum += d[i] * right[i] * right[i];

            d[n] = diagonal[n] - sum - (d[n - 1] * r[n - 1] * r[n - 1]);

            for (int i = 2; i <= n - 1; i++) u[i] -= u[i - 1] * r[i - 1];

            sum = 0.0;
            for (int i = 1; i <= n - 2; i++) sum += right[i] * u[i];

            u[n] = u[n] - sum - (r[n - 1] * u[n - 1]);

            for (int i = 1; i <= n; i++) u[i] /= d[i];

            second[n] = u[n];
            second[n - 1] = u[n - 1] - (r[n - 1] * second[n]);

            for (int i = n - 2; i >= 1; i--)
                second[i] = u[i] - (r[i] * second[i + 1]) - (right[i] * second[n]);

            second[0] = second[n];
        }

        for (int i = 0; i <= n; i++) second[i] *= 2.0;

        return second;
    }

    /// <summary>
    /// One point of the spline, by the <c>splint</c> formula of <em>Numerical Recipes</em> §3.3.
    /// </summary>
    /// <remarks>
    /// <c>lcl_SplineCalculation::GetInterpolatedValue</c> (<c>Splines.cxx</c>:355-400). The
    /// reference caches the bracketing pair between calls because it is stepped monotonically;
    /// the search here is a plain bisection, which answers the same pair.
    /// </remarks>
    private static double Interpolate(double[] at, double[] value, double[] second, double x)
    {
        int low = 0;
        int high = at.Length - 1;

        while (high - low > 1)
        {
            int middle = (high + low) / 2;
            if (at[middle] > x) high = middle; else low = middle;
        }

        double h = at[high] - at[low];
        if (h == 0.0) return value[low];

        double a = (at[high] - x) / h;
        double b = (x - at[low]) / h;

        return (a * value[low])
               + (b * value[high])
               + (((((a * a * a) - a) * second[low]) + ((((b * b * b) - b) * second[high])))
                  * h * h / 6.0);
    }
}
