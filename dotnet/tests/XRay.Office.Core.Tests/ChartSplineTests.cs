using System.Globalization;
using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// A smoothed series is drawn as a flattened natural cubic spline, parameterised by point index
/// and sampled twenty times per interval.
/// </summary>
/// <remarks>
/// <para>
/// <strong>The fixture is 26.2.4.2's own output and nothing here is fitted to it.</strong>
/// <c>sheets/chartset-004/xlsx/microsoft_learn_multi_chart_examples.xlsx</c> holds a scatter
/// chart of eleven points whose x values are unevenly spaced, and the reference's PDF of it
/// strokes that series as <strong>200 line segments</strong> — ten intervals of twenty — whose
/// 201 vertices are read back here verbatim. <see cref="Knots"/> is every twentieth of those
/// vertices, which is where <c>Splines.cxx</c>:600-606 copies the stated point through without
/// evaluating it, so the input and the expected output come from the same measurement and the
/// only thing under test is the curve between them.
/// </para>
/// <para>
/// <strong>It separates the three hypotheses, which is why this series and not another.</strong>
/// Nine of the corpus's eleven smoothed OOXML witnesses hold arithmetic-progression data, whose
/// cubic spline <em>is</em> the straight polyline — the reference's own flattened polyline for
/// <c>002_advanced_excel_line.xlsx</c> departs from its chord by 0.04 pt over 221 points — so
/// they cannot tell any two of these apart. Against this one, the maximum distance from
/// 26.2.4.2's polyline is:
/// </para>
/// <code>
///   parameterised by index, natural ends    0.0495 pt   &lt;- what this implements
///   parameterised by index, clamped ends    0.1700 pt
///   parameterised by x value, natural ends  1.8219 pt
///   no spline at all, the straight polyline 1.8183 pt
/// </code>
/// <para>
/// and 0.0495 pt is <strong>1.75 hundredths of a millimetre</strong>, which is the grid the
/// reference's own geometry sits on: every one of those 201 vertices is a whole number of
/// hundredths of a millimetre from the first, to within the three decimal places its content
/// stream is written to. So the residual is the channel and not the model.
/// </para>
/// </remarks>
public sealed class ChartSplineTests
{
    /// <summary>How close two flattenings of the same series have to be, in points.</summary>
    /// <remarks>
    /// Two hundredths of a millimetre, 0.0567 pt — the quantisation of the reference's own
    /// coordinates, doubled because both the knots fed in and the vertices compared against
    /// carry it.
    /// </remarks>
    private const double Grid = 2.0 * 2540.0 / 100.0 / 72.0;

    private static readonly double[] Knots =
    [
        428.513, 450.001, 457.625, 446.599, 486.737, 442.036,
        515.820, 438.634, 530.391, 435.204, 544.932, 430.640,
        574.044, 427.210, 588.586, 423.809, 603.128, 421.513,
        632.268, 419.217, 661.380, 415.815,
    ];

    private static readonly double[] Reference =
    [
        428.513, 450.001, 429.959, 449.888, 431.433, 449.746, 432.907, 449.576, 434.381, 449.434,
        435.827, 449.264, 437.301, 449.122, 438.775, 448.952, 440.249, 448.810, 441.694, 448.640,
        443.169, 448.470, 444.614, 448.300, 446.060, 448.130, 447.506, 447.960, 448.980, 447.762,
        450.425, 447.592, 451.871, 447.393, 453.317, 447.223, 454.762, 447.025, 456.180, 446.826,
        457.625, 446.599, 459.043, 446.401, 460.488, 446.174, 461.877, 445.947, 463.294, 445.721,
        464.740, 445.494, 466.157, 445.267, 467.575, 445.040, 469.020, 444.785, 470.438, 444.558,
        471.883, 444.332, 473.329, 444.077, 474.775, 443.850, 476.249, 443.595, 477.694, 443.368,
        479.169, 443.141, 480.643, 442.914, 482.145, 442.688, 483.647, 442.461, 485.178, 442.234,
        486.737, 442.036, 488.268, 441.866, 489.855, 441.639, 491.414, 441.469, 492.973, 441.270,
        494.561, 441.072, 496.120, 440.902, 497.707, 440.703, 499.266, 440.533, 500.797, 440.363,
        502.328, 440.193, 503.830, 440.023, 505.332, 439.881, 506.778, 439.711, 508.195, 439.541,
        509.556, 439.399, 510.917, 439.229, 512.220, 439.088, 513.468, 438.946, 514.687, 438.776,
        515.820, 438.634, 516.926, 438.464, 517.975, 438.322, 518.967, 438.181, 519.902, 438.010,
        520.809, 437.869, 521.660, 437.699, 522.454, 437.557, 523.219, 437.387, 523.956, 437.217,
        524.636, 437.047, 525.288, 436.877, 525.940, 436.707, 526.564, 436.536, 527.159, 436.366,
        527.726, 436.168, 528.293, 435.998, 528.831, 435.799, 529.342, 435.601, 529.880, 435.403,
        530.391, 435.204, 530.901, 434.977, 531.383, 434.751, 531.921, 434.524, 532.431, 434.297,
        532.970, 434.099, 533.509, 433.872, 534.076, 433.645, 534.643, 433.390, 535.266, 433.163,
        535.918, 432.908, 536.598, 432.681, 537.307, 432.455, 538.072, 432.199, 538.894, 431.973,
        539.745, 431.746, 540.652, 431.519, 541.616, 431.292, 542.665, 431.066, 543.770, 430.867,
        544.932, 430.640, 546.180, 430.442, 547.483, 430.244, 548.872, 430.045, 550.290, 429.875,
        551.764, 429.677, 553.266, 429.507, 554.797, 429.336, 556.328, 429.166, 557.915, 428.996,
        559.474, 428.826, 561.061, 428.684, 562.649, 428.514, 564.208, 428.344, 565.739, 428.203,
        567.241, 428.032, 568.715, 427.862, 570.132, 427.721, 571.465, 427.551, 572.797, 427.381,
        574.044, 427.210, 575.206, 427.040, 576.312, 426.870, 577.361, 426.700, 578.324, 426.502,
        579.231, 426.332, 580.110, 426.190, 580.904, 426.020, 581.669, 425.821, 582.406, 425.651,
        583.087, 425.481, 583.710, 425.283, 584.334, 425.113, 584.929, 424.943, 585.496, 424.773,
        586.035, 424.603, 586.573, 424.432, 587.055, 424.262, 587.565, 424.121, 588.076, 423.951,
        588.586, 423.809, 589.096, 423.667, 589.606, 423.525, 590.145, 423.384, 590.683, 423.242,
        591.250, 423.129, 591.817, 423.015, 592.413, 422.873, 593.036, 422.760, 593.660, 422.647,
        594.340, 422.533, 595.049, 422.420, 595.786, 422.335, 596.551, 422.221, 597.345, 422.108,
        598.195, 422.023, 599.102, 421.910, 600.038, 421.825, 601.030, 421.711, 602.079, 421.626,
        603.128, 421.513, 604.290, 421.428, 605.509, 421.314, 606.756, 421.229, 608.060, 421.116,
        609.392, 421.031, 610.781, 420.918, 612.198, 420.804, 613.672, 420.691, 615.146, 420.606,
        616.649, 420.492, 618.180, 420.379, 619.682, 420.266, 621.241, 420.152, 622.828, 420.010,
        624.387, 419.897, 625.975, 419.784, 627.562, 419.642, 629.150, 419.500, 630.709, 419.358,
        632.268, 419.217, 633.827, 419.075, 635.329, 418.933, 636.831, 418.792, 638.362, 418.621,
        639.836, 418.451, 641.339, 418.338, 642.813, 418.168, 644.258, 417.998, 645.732, 417.828,
        647.178, 417.658, 648.624, 417.459, 650.041, 417.289, 651.458, 417.119, 652.876, 416.921,
        654.321, 416.751, 655.739, 416.552, 657.128, 416.382, 658.545, 416.184, 659.962, 416.014,
        661.380, 415.815,
    ];

    private static DocPoint[] PointsOf(double[] flat)
    {
        DocPoint[] points = new DocPoint[flat.Length / 2];

        for (int at = 0; at < points.Length; at++)
        {
            points[at] = new DocPoint(
                Length.FromPoints(flat[at * 2]), Length.FromPoints(flat[(at * 2) + 1]));
        }

        return points;
    }

    private static double Distance(DocPoint a, DocPoint b)
    {
        double dx = a.X.Points - b.X.Points;
        double dy = a.Y.Points - b.Y.Points;
        return Math.Sqrt((dx * dx) + (dy * dy));
    }

    [Fact]
    public void TheFlatteningReproducesTheReferencesOwnPolyline()
    {
        IReadOnlyList<DocPoint> drawn = ChartSpline.Flatten(PointsOf(Knots));
        DocPoint[] expected = PointsOf(Reference);

        drawn.Count.ShouldBe(expected.Length);

        double worst = 0.0;
        for (int at = 0; at < drawn.Count; at++)
            worst = Math.Max(worst, Distance(drawn[at], expected[at]));

        worst.ShouldBeLessThan(Grid);
    }

    [Fact]
    public void EveryIntervalIsTwentySegmentsAndTheStatedPointsAreCopiedThrough()
    {
        DocPoint[] knots = PointsOf(Knots);
        IReadOnlyList<DocPoint> drawn = ChartSpline.Flatten(knots);

        drawn.Count.ShouldBe(((knots.Length - 1) * ChartSpline.Granularity) + 1);

        for (int at = 0; at < knots.Length; at++)
            drawn[at * ChartSpline.Granularity].ShouldBe(knots[at]);
    }

    /// <summary>
    /// Collinear points give back the straight line, which is why nine of the eleven corpus
    /// witnesses do not move.
    /// </summary>
    /// <remarks>
    /// The nine <c>advanced_excel</c> witnesses state values in arithmetic progression against
    /// evenly spaced categories. A natural cubic spline through such points has every second
    /// derivative zero, so the flattening is the chord subdivided — and the reference agrees:
    /// its 221-vertex polyline for <c>002_advanced_excel_line.xlsx</c> series 1 departs from its
    /// own chord by at most 0.0403 pt.
    /// </remarks>
    [Fact]
    public void ASeriesWhoseValuesRiseByAConstantIsFlattenedToItsOwnChord()
    {
        List<DocPoint> points = [];
        for (int at = 0; at < 12; at++)
        {
            points.Add(new DocPoint(
                Length.FromPoints(100.0 + (at * 40.0)),
                Length.FromPoints(300.0 - (at * 17.0))));
        }

        IReadOnlyList<DocPoint> drawn = ChartSpline.Flatten(points);

        DocPoint first = drawn[0];
        DocPoint last = drawn[^1];

        double dx = last.X.Points - first.X.Points;
        double dy = last.Y.Points - first.Y.Points;
        double length = Math.Sqrt((dx * dx) + (dy * dy));

        double worst = 0.0;
        foreach (DocPoint point in drawn)
        {
            double side = Math.Abs(
                ((point.X.Points - first.X.Points) * dy)
                - ((point.Y.Points - first.Y.Points) * dx)) / length;
            worst = Math.Max(worst, side);
        }

        worst.ShouldBeLessThan(0.001);
    }

    /// <summary>Two points are a line and one is not a curve at all.</summary>
    /// <remarks>
    /// <c>if( rInput[nOuter].size() &lt;= 1 ) continue; //we need at least two points</c> —
    /// <c>Splines.cxx</c>:540-541. Two points give <c>n = 1</c>, whose tridiagonal solve leaves
    /// both second derivatives zero, so the reference subdivides the segment rather than
    /// curving it.
    /// </remarks>
    [Fact]
    public void AOnePointSeriesIsHandedBackUnchanged()
    {
        IReadOnlyList<DocPoint> one =
            [new DocPoint(Length.FromPoints(10), Length.FromPoints(20))];

        ChartSpline.Flatten(one).ShouldBe(one);
    }

    [Fact]
    public void ATwoPointSeriesIsSubdividedAndNotCurved()
    {
        IReadOnlyList<DocPoint> two =
        [
            new DocPoint(Length.FromPoints(0), Length.FromPoints(0)),
            new DocPoint(Length.FromPoints(200), Length.FromPoints(100)),
        ];

        IReadOnlyList<DocPoint> drawn = ChartSpline.Flatten(two);

        drawn.Count.ShouldBe(ChartSpline.Granularity + 1);
        drawn[10].X.Points.ShouldBe(100.0, 0.001);
        drawn[10].Y.Points.ShouldBe(50.0, 0.001);
    }

    // ---- and what the layout does with it -------------------------------------------------

    /// <summary>A measurer with no fonts: half an em per character, 1.15 em a line.</summary>
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
            => new(size * (0.5 * text.Length), size * 1.15);
    }

    private static ChartPlot Series(bool smooth, params double?[] values)
        => new()
        {
            Kind = ChartPlotKind.Line,
            Categories = [.. values.Select((_, at) => at.ToString(CultureInfo.InvariantCulture))],
            Series =
            [
                new ChartSeries("S", values, Line: Colour.Black)
                {
                    Smooth = smooth,
                },
            ],
        };

    private static int SegmentsOf(ChartDrawing drawing)
        => drawing.Shapes
            .Where(shape => shape.Fill is null && shape.Line is not null)
            .Sum(shape => shape.Path.Commands.Count(c => c.Verb == PathVerb.LineTo));

    /// <summary>
    /// A smoothed line series is stroked as twenty segments per interval where a plain one is
    /// stroked as one.
    /// </summary>
    /// <remarks>
    /// The whole-drawing counterpart of the fixture above: 26.2.4.2 strokes each of the nine
    /// twelve-point <c>advanced_excel</c> witnesses' series as 220 <c>l</c> operators and this
    /// tree drew 11.
    /// </remarks>
    [Fact]
    public void ASmoothedSeriesIsStrokedTwentyTimesPerIntervalAndAPlainOneOnce()
    {
        double?[] values = [3.0, 9.0, 4.0, 11.0, 6.0, 12.0];
        DocRect frame = new(
            Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300));

        SegmentsOf(ChartLayout.Place(Series(smooth: false, values), frame, new Ruler()))
            .ShouldBe(values.Length - 1);
        SegmentsOf(ChartLayout.Place(Series(smooth: true, values), frame, new Ruler()))
            .ShouldBe((values.Length - 1) * ChartSpline.Granularity);
    }

    /// <summary>
    /// A gap breaks the series into runs and each run is smoothed on its own.
    /// </summary>
    /// <remarks>
    /// The reference splines <c>*pSeriesPoly</c>, which is already one polygon per unbroken run
    /// — <c>CalculateCubicSplines</c>'s outer loop is over those runs
    /// (<c>Splines.cxx</c>:538-539) — so a category with no value ends the curve rather than
    /// being interpolated across.
    /// </remarks>
    [Fact]
    public void AGapEndsTheCurveRatherThanBeingSmoothedAcross()
    {
        double?[] values = [3.0, 9.0, 4.0, null, 6.0, 12.0, 5.0];
        DocRect frame = new(
            Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300));

        // Two runs of three points: two intervals each, twenty segments apiece.
        SegmentsOf(ChartLayout.Place(Series(smooth: true, values), frame, new Ruler()))
            .ShouldBe(4 * ChartSpline.Granularity);
    }
}
