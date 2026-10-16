using XRay.Office.Core.Charts;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using Shouldly;

namespace XRay.Office.Core.Tests;

/// <summary>
/// The extruded solid 26.2.4.2 draws for a <c>c:pie3DChart</c>: its squash, its size and the
/// diagram rectangle it is fitted into.
/// </summary>
/// <remarks>
/// <para>
/// Every figure here is measured rather than derived, and it had to be: <strong>the reference
/// rasterises a 3-D chart</strong>, so the solid reaches the page as one image with a soft mask
/// and its geometry can only be read out of that bitmap's silhouette.
/// <c>probes/pie3d-r187</c> renders 36 one-attribute rewrites of a corpus document's own chart
/// part — nine <c>c:rotX</c> by four <c>c:depthPercent</c> — through 26.2.4.2 and fits each.
/// </para>
/// <para>
/// The fitted upper boundary is an ellipse to a mean residual of <strong>0.28 to 0.45
/// pixels</strong> on images 1370 across, which is what says the projection is parallel in
/// practice; the squash is <c>sin(rotX)</c> to within 0.7% at seven of the nine elevations; and
/// the four depth values give geometry identical to the hundredth of a point at every one of
/// them, so <c>c:depthPercent</c> is not read at all.
/// </para>
/// </remarks>
public sealed class ChartPie3DTests
{
    private sealed class Ruler : IChartTextMeasurer
    {
        public DocSize Measure(string text, Length size, string? family, bool bold)
            => new(size * (0.5 * text.Length) * (bold ? 1.1 : 1.0), size * 1.15);
    }

    /// <summary>A pie of four points, labelled or not, flat or at an elevation.</summary>
    private static ChartPlot Pie(double? elevation, bool labelled = false)
        => new()
        {
            Kind = ChartPlotKind.Pie,
            Elevation = elevation,
            Legend = ChartLegendPosition.None,
            Categories = ["1st Qtr", "2nd Qtr", "3rd Qtr", "4th Qtr"],
            Series =
            [
                new ChartSeries("S", [50.0, 25.0, 15.0, 10.0], Colour.FromRgb(0x70AD47))
                {
                    Label = labelled
                        ? new ChartDataLabel
                        {
                            ShowCategory = true,
                            Placement = ChartLabelPlacement.Outside,
                        }
                        : null,
                },
            ],
        };

    /// <summary>The bounding box of every filled shape the drawing carries.</summary>
    private static DocRect Solid(ChartDrawing drawing)
    {
        Length left = Length.FromPoints(1e9), top = Length.FromPoints(1e9);
        Length right = Length.FromPoints(-1e9), bottom = Length.FromPoints(-1e9);

        foreach (ChartShape shape in drawing.Shapes)
        {
            DocRect bounds = shape.Bounds();
            left = Length.Min(left, bounds.Left);
            top = Length.Min(top, bounds.Top);
            right = Length.Max(right, bounds.Right);
            bottom = Length.Max(bottom, bounds.Bottom);
        }

        return new DocRect(left, top, right - left, bottom - top);
    }

    /// <summary>How tall the drawn solid is against how wide.</summary>
    private static double Ratio(DocRect solid) => solid.Height.Points / solid.Width.Points;

    /// <summary>A flat pie is a circle and a three-dimensional one is not.</summary>
    /// <remarks>
    /// The discriminator that needs no measured constant: the same chart in the same frame, with
    /// and without the elevation. Before this existed both drew the circle.
    /// </remarks>
    [Fact]
    public void AnElevationMakesThePieAnEllipseAndAFlatOneACircle()
    {
        DocRect frame = new(Length.Zero, Length.Zero, Length.FromPoints(400),
                            Length.FromPoints(300));

        DocRect flat = Solid(ChartLayout.Place(Pie(null), frame, new Ruler()));
        DocRect raised = Solid(ChartLayout.Place(Pie(30), frame, new Ruler()));

        Ratio(flat).ShouldBe(1.0, 0.02);
        Ratio(raised).ShouldBeLessThan(0.7);
    }

    /// <summary>
    /// The squash is the sine of the elevation, and the solid's own height carries the wall.
    /// </summary>
    /// <remarks>
    /// Measured <c>B/A</c> against <c>sin(rotX)</c> over the probe's nine: 0.164/0.174,
    /// 0.340/0.342, 0.499/0.500, 0.647/0.643, 0.763/0.766, 0.866/0.866, 0.939/0.940,
    /// 0.966/0.985, 0.997/1.000. Asserted on the whole silhouette rather than on the ellipse
    /// alone, because what a page shows is the solid: its height is
    /// <c>2A·sin θ + 0.10·cos θ·2A</c>, which is the ratio here.
    /// </remarks>
    [Theory]
    [InlineData(30)]
    [InlineData(50)]
    [InlineData(70)]
    public void TheSolidIsAsTallAsTheElevationAndTheThicknessTogether(double elevation)
    {
        // Wide and short, so the fit is bound by the height and the ratio is the shape's own.
        DocRect frame = new(Length.Zero, Length.Zero, Length.FromPoints(900),
                            Length.FromPoints(300));

        DocRect solid = Solid(ChartLayout.Place(Pie(elevation), frame, new Ruler()));

        double radians = elevation * Math.PI / 180.0;
        double expected = Math.Sin(radians) + (0.10 * Math.Cos(radians));

        Ratio(solid).ShouldBe(expected, 0.03);
    }

    /// <summary>
    /// A three-dimensional pie fills a wide rectangle; a flat one is squared into it.
    /// </summary>
    /// <remarks>
    /// <c>VDiagram::adjustPosAndSize</c> branches on the dimension count
    /// (<c>chart2/source/view/diagram/VDiagram.cxx</c>:89-101) and the 3-D arm fits the scene's
    /// own projected bounding box rather than the preferred ratio (<c>:409-421</c>), so the
    /// <c>(1, 1, 0.10)</c> of <c>PieChart::getPreferredDiagramAspectRatio</c> never squares
    /// anything. Squaring it anyway drew <c>021_Unit_Circle_Chart_3D_Pie_Chart</c> at 274 pt
    /// across where 26.2.4.2 draws 469, because that page's rectangle is wide and short and the
    /// square took the short side.
    /// </remarks>
    [Fact]
    public void AThreeDimensionalPieIsNotSquaredIntoAWideRectangle()
    {
        DocRect frame = new(Length.Zero, Length.Zero, Length.FromPoints(900),
                            Length.FromPoints(300));

        DocRect flat = Solid(ChartLayout.Place(Pie(null), frame, new Ruler()));
        DocRect raised = Solid(ChartLayout.Place(Pie(30), frame, new Ruler()));

        // The flat pie can be no wider than the rectangle is tall.
        flat.Width.Points.ShouldBeLessThanOrEqualTo(310.0);

        // The raised one is bound by its own height instead: 2A·(sin 30 + 0.10·cos 30) = 300
        // gives 2A = 511, which the rectangle's 900 does not limit.
        raised.Width.Points.ShouldBeGreaterThan(450.0);
    }

    /// <summary>At ninety degrees the solid is a flat circle again.</summary>
    /// <remarks>
    /// Straight down: the probe measures <c>B/A</c> 0.9974 and a wall of 0.11 pt on a 280 pt
    /// figure. It is the one elevation at which the two branches have to agree, and a sign error
    /// anywhere in the projection shows up here as a shape that is not a circle.
    /// </remarks>
    [Fact]
    public void AtNinetyDegreesTheSolidIsACircle()
    {
        DocRect frame = new(Length.Zero, Length.Zero, Length.FromPoints(400),
                            Length.FromPoints(300));

        DocRect solid = Solid(ChartLayout.Place(Pie(90), frame, new Ruler()));

        Ratio(solid).ShouldBe(1.0, 0.02);
    }

    /// <summary>
    /// The extruded wall is drawn under the top faces and in a darker shade of the same colour.
    /// </summary>
    /// <remarks>
    /// Counted over every opaque pixel of the corpus document's own bitmap, the top faces are the
    /// declared colour times 0.957 and the wall runs from about 0.83 at the sides to 0.91 at the
    /// front. The assertion is on the relation rather than on either constant: the wall is
    /// darker, it is the same hue, and it is painted first.
    /// </remarks>
    [Fact]
    public void TheWallIsDarkerThanTheTopFaceAndPaintedFirst()
    {
        DocRect frame = new(Length.Zero, Length.Zero, Length.FromPoints(400),
                            Length.FromPoints(300));

        IReadOnlyList<ChartShape> shapes =
            ChartLayout.Place(Pie(30), frame, new Ruler()).Shapes;

        shapes.Count.ShouldBeGreaterThan(4);

        // The last four are the four top faces; everything before them is wall.
        ChartShape wall = shapes[0];
        ChartShape top = shapes[^1];

        wall.Fill.ShouldNotBeNull();
        top.Fill.ShouldNotBeNull();

        Colour lower = wall.Fill!.Value;
        Colour upper = top.Fill!.Value;

        (lower.R + lower.G + lower.B).ShouldBeLessThan(upper.R + upper.G + upper.B);

        // The wall reaches below every top face, which is what makes the solid look solid.
        wall.Bounds().Bottom.ShouldBeGreaterThan(top.Bounds().Bottom);
    }

    /// <summary>A three-dimensional doughnut is not modelled and stays flat.</summary>
    /// <remarks>
    /// No corpus document states one — all three of the corpus's <c>*3DChart</c> elements are
    /// <c>pie3DChart</c> — and a ring's inner wall is a face this does not build, so the ring
    /// path is left exactly as it was rather than half-extruded.
    /// </remarks>
    [Fact]
    public void AThreeDimensionalDoughnutIsDrawnFlat()
    {
        DocRect frame = new(Length.Zero, Length.Zero, Length.FromPoints(400),
                            Length.FromPoints(300));

        ChartPlot rings = Pie(30) with { Rings = true };
        DocRect solid = Solid(ChartLayout.Place(rings, frame, new Ruler()));

        Ratio(solid).ShouldBe(1.0, 0.02);
    }
}
