using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// A line series is drawn as the part of its polyline that falls inside the plot, and a point
/// outside the plot carries no mark of its own.
/// </summary>
/// <remarks>
/// <para>
/// Every chart2 plotter clips its polygon at
/// <c>PlottingPositionHelper::getScaledLogicClipDoubleRect</c> before it makes a shape of it —
/// <c>Clipping::clipPolygonAtRectangle</c>, <c>chart2/source/view/charttypes/AreaChart.cxx</c>:359
/// for a straight line and eight more callers beside it — and <c>AreaChart::createShapes</c>
/// skips a point <c>isLogicVisible</c> rejects before any symbol, error bar or data label is
/// created (<c>AreaChart.cxx</c>:715, 760-761).
/// </para>
/// <para>
/// <strong>Measured on <c>slides/done-011/pptx/171128IPAP.pptx</c> slide 38</strong>, a line
/// chart over a date axis stated as 40179…43831 whose cached points begin in 2006: 26.2.4.2
/// draws all three series between x = 119.54 and x = 615.49, its own plot rectangle, and this
/// tree drew them from x = −866.50 to x = 758.03 on a 720 pt page. Three of that deck's forty
/// pages carried vector ink outside the page and none of the reference's did.
/// </para>
/// </remarks>
public sealed class ChartPolylineClipTests
{
    /// <summary>A measurer with no fonts: half an em per character, 1.15 em a line.</summary>
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
            => new(size * (0.5 * text.Length), size * 1.15);
    }

    private static readonly DocRect Frame =
        new(Length.Zero, Length.Zero, Length.FromPoints(400), Length.FromPoints(300));

    private static readonly Colour Blue = new(0x00, 0x70, 0xC0);

    private static Length Pt(double points) => Length.FromPoints(points);

    private static DocRect Rect(double x, double y, double width, double height)
        => new(Pt(x), Pt(y), Pt(width), Pt(height));

    private static DocPoint At(double x, double y) => new(Pt(x), Pt(y));

    /// <summary>A line chart whose stated range excludes some of its own points.</summary>
    private static ChartPlot Line(IReadOnlyList<double?> values, double minimum, double maximum)
        => new()
        {
            Kind = ChartPlotKind.Line,
            Categories = ["A", "B", "C", "D", "E"],
            ValueScale = new ChartScaleRequest(minimum, maximum),
            Series = [new ChartSeries("S", values, Fill: Blue, Line: Blue)],
        };

    private static IEnumerable<DocPoint> Vertices(ChartShape shape)
    {
        foreach (PathCommand command in shape.Path.Commands)
        {
            if (command.Verb != PathVerb.Close) yield return command.Point;
        }
    }

    // ---- the clipper itself -------------------------------------------------------------

    /// <summary>A polyline wholly inside its rectangle is handed back untouched.</summary>
    /// <remarks>
    /// The bounding-box short circuit of <c>Clipping::clipPolygonAtRectangle</c>
    /// (<c>chart2/source/view/main/Clipping.cxx</c>:350-365). It is what makes clipping free for
    /// every chart that already fitted, and it is why only two of the corpus' 176 chart-bearing
    /// documents moved when this landed.
    /// </remarks>
    [Fact]
    public void APolylineInsideTheRectangleIsReturnedUnchanged()
    {
        DocPoint[] points = [At(10, 10), At(20, 30), At(40, 20)];

        IReadOnlyList<IReadOnlyList<DocPoint>> pieces =
            ChartClipping.ClipPolyline(points, Rect(0, 0, 100, 100));

        pieces.Count.ShouldBe(1);
        pieces[0].ShouldBe(points);
    }

    /// <summary>A polyline wholly outside its rectangle survives not at all.</summary>
    [Fact]
    public void APolylineOutsideTheRectangleIsDroppedWhole()
    {
        DocPoint[] points = [At(-50, 10), At(-40, 30), At(-20, 20)];

        ChartClipping.ClipPolyline(points, Rect(0, 0, 100, 100)).ShouldBeEmpty();
    }

    /// <summary>
    /// A segment leaving the rectangle is cut at the edge it leaves through, not at its own
    /// bounding box.
    /// </summary>
    /// <remarks>
    /// This is the half that separates *clipping the geometry* from *clipping by the bounding
    /// box*: the surviving vertex is the intersection point, so the line keeps its slope.
    /// </remarks>
    [Fact]
    public void ASegmentLeavingTheRectangleIsCutAtTheEdge()
    {
        DocPoint[] points = [At(50, 50), At(150, 50)];

        IReadOnlyList<IReadOnlyList<DocPoint>> pieces =
            ChartClipping.ClipPolyline(points, Rect(0, 0, 100, 100));

        pieces.Count.ShouldBe(1);
        pieces[0].Count.ShouldBe(2);
        pieces[0][0].ShouldBe(At(50, 50));
        pieces[0][1].X.Points.ShouldBe(100.0, 0.001);
        pieces[0][1].Y.Points.ShouldBe(50.0, 0.001);
    }

    /// <summary>
    /// A line that leaves the plot and comes back is two strokes with no chord across the gap.
    /// </summary>
    /// <remarks>
    /// <c>bSplitPiecesToDifferentPolygons</c>, which defaults to <c>true</c>
    /// (<c>chart2/source/view/inc/Clipping.hxx</c>:51) and is what every line caller takes;
    /// only the two that clip a filled polygon pass <c>false</c>. Joining the pieces instead
    /// would draw a straight segment along the edge that the data never had.
    /// </remarks>
    [Fact]
    public void ALineThatLeavesAndReturnsIsTwoSeparatePieces()
    {
        DocPoint[] points = [At(50, 50), At(150, 50), At(150, 80), At(50, 80)];

        IReadOnlyList<IReadOnlyList<DocPoint>> pieces =
            ChartClipping.ClipPolyline(points, Rect(0, 0, 100, 100));

        pieces.Count.ShouldBe(2);
        pieces[0][0].ShouldBe(At(50, 50));
        pieces[1][^1].ShouldBe(At(50, 80));
    }

    /// <summary>A segment crossing the rectangle end to end keeps both intersections.</summary>
    [Fact]
    public void ASegmentCrossingRightThroughKeepsBothIntersections()
    {
        DocPoint[] points = [At(-50, 50), At(150, 50)];

        IReadOnlyList<IReadOnlyList<DocPoint>> pieces =
            ChartClipping.ClipPolyline(points, Rect(0, 0, 100, 100));

        pieces.Count.ShouldBe(1);
        pieces[0][0].X.Points.ShouldBe(0.0, 0.001);
        pieces[0][1].X.Points.ShouldBe(100.0, 0.001);
    }

    // ---- the line plotter ---------------------------------------------------------------

    /// <summary>Nothing a line series draws reaches outside the plot rectangle.</summary>
    [Fact]
    public void ALineLeavingTheValueRangeIsClippedToThePlot()
    {
        ChartDrawing drawing =
            ChartLayout.Place(Line([1.0, 50.0, -40.0, 20.0, 90.0], 0.0, 60.0), Frame, new Ruler());

        drawing.Shapes.ShouldNotBeEmpty();

        foreach (ChartShape shape in drawing.Shapes)
        {
            foreach (DocPoint vertex in Vertices(shape))
            {
                vertex.Y.ShouldBeGreaterThanOrEqualTo(drawing.PlotArea.Top);
                vertex.Y.ShouldBeLessThanOrEqualTo(drawing.PlotArea.Bottom);
            }
        }
    }

    /// <summary>
    /// A point outside the value range gets no marker, although the polyline still passes
    /// through where it would have been.
    /// </summary>
    /// <remarks>
    /// <c>AreaChart::createShapes</c> adds the point to the series polygon and only then asks
    /// <c>isLogicVisible</c>, so the two answers genuinely differ: the line is drawn as far as
    /// the plot's edge and the symbol is not drawn at all.
    /// </remarks>
    [Fact]
    public void APointOutsideTheValueRangeGetsNoMarker()
    {
        ChartPlot plot = Line([10.0, 500.0, 30.0], 0.0, 60.0) with
        {
            Categories = ["A", "B", "C"],
            Series =
            [
                new ChartSeries("S", [10.0, 500.0, 30.0], Fill: Blue, Line: Blue)
                {
                    Marker = ChartMarker.Square,
                },
            ],
        };

        ChartDrawing drawing = ChartLayout.Place(plot, Frame, new Ruler());

        // One stroke for the line, one square for each of the two visible points.
        drawing.Shapes.Count(shape => shape.Fill is not null).ShouldBe(2);
    }

    /// <summary>A chart whose points all fit is drawn exactly as it was before.</summary>
    /// <remarks>
    /// The control, and the same one <c>ChartBarClipTests</c> carries: an automatic scale is
    /// derived from the data, so nothing can be outside it and the clip must be a no-op.
    /// </remarks>
    [Fact]
    public void ALineWhoseValuesAllFitIsUnchanged()
    {
        ChartPlot plot = new()
        {
            Kind = ChartPlotKind.Line,
            Categories = ["A", "B", "C", "D"],
            Series = [new ChartSeries("S", [4.0, 9.0, 2.0, 7.0], Fill: Blue, Line: Blue)],
        };

        ChartDrawing drawing = ChartLayout.Place(plot, Frame, new Ruler());

        ChartShape line = drawing.Shapes.Single(shape => shape.Fill is null);
        line.Path.Commands.Count.ShouldBe(4);
        line.Path.Commands[0].Verb.ShouldBe(PathVerb.MoveTo);
        line.Path.Commands.Count(c => c.Verb == PathVerb.MoveTo).ShouldBe(1);
    }

    /// <summary>A gap in the values still breaks the line, as it did before any clipping.</summary>
    [Fact]
    public void AMissingValueStillBreaksTheLine()
    {
        ChartPlot plot = new()
        {
            Kind = ChartPlotKind.Line,
            Categories = ["A", "B", "C", "D"],
            Series = [new ChartSeries("S", [4.0, null, 2.0, 7.0], Fill: Blue, Line: Blue)],
        };

        ChartDrawing drawing = ChartLayout.Place(plot, Frame, new Ruler());

        ChartShape line = drawing.Shapes.Single(shape => shape.Fill is null);
        line.Path.Commands.Count(c => c.Verb == PathVerb.MoveTo).ShouldBe(2);
    }
}
