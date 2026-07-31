using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Ooxml.DrawingML;
using Paperless.Presentations.Layout;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// Checks the preset geometry evaluator against the shapes LibreOffice's own PDF draws.
/// </summary>
/// <remarks>
/// The numbers are transcribed once from the reference render of
/// <c>slide-shape-features.pptx</c> and need no LibreOffice to check;
/// <c>Paperless.Fidelity.Tests</c> re-derives them. What is here that the comparison cannot say is
/// the <em>coverage</em>: every one of the 187 presets evaluates, and a name that is not one of
/// them still produces a rectangle rather than nothing.
/// </remarks>
public class SlidePresetGeometryTests
{
    /// <summary>A shape one and a half inches by one, which is what the corpus deck uses.</summary>
    private static readonly DocSize Box = new(Length.FromInches(1.5), Length.FromInches(1));

    [Fact]
    public void EveryPresetInTheTableEvaluates()
    {
        // 187 is the whole of presetShapeDefinitions.xml. The count is asserted because the
        // definitions are an embedded resource: a build that failed to embed it would otherwise
        // fall back to a rectangle for every shape and pass every other test here.
        PresetShapeGeometry.Names.Count.ShouldBe(187);

        foreach (string name in PresetShapeGeometry.Names)
        {
            GraphicsPath outline = SlidePresetGeometry.Outline(name, Box);
            outline.Commands.Count.ShouldBeGreaterThan(
                0, $"{name}: evaluated to an empty path");

            foreach (PathCommand command in outline.Commands)
            {
                double x = command.Point.X.Points;
                double y = command.Point.Y.Points;

                // No preset runs more than one shape-size outside its own box; a formula that
                // divided by zero or read an undefined guide lands at the origin or at infinity,
                // and both show here.
                double.IsFinite(x).ShouldBeTrue($"{name}: non-finite x");
                double.IsFinite(y).ShouldBeTrue($"{name}: non-finite y");
                Math.Abs(x).ShouldBeLessThan(1000, $"{name}: x {x} is far outside the shape");
                Math.Abs(y).ShouldBeLessThan(1000, $"{name}: y {y} is far outside the shape");
            }
        }
    }

    [Fact]
    public void AnUnknownPresetIsItsBoundingRectangle()
    {
        SlidePresetGeometry.IsKnown("noSuchShape").ShouldBeFalse();

        List<DocPoint> points = Vertices(SlidePresetGeometry.Outline("noSuchShape", Box));

        points.Count.ShouldBe(4);
        points[0].ShouldBe(new DocPoint(Length.Zero, Length.Zero));
        points[2].ShouldBe(new DocPoint(Box.Width, Box.Height));
    }

    [Fact]
    public void AHexagonHasTheSixVerticesLibreOfficeDraws()
    {
        // The reference PDF draws it at 71.972, 432.028 and five more, on a shape stated at
        // 72 pt, 72 pt sized 108 by 72 — so the expectations here are the file's own numbers.
        List<DocPoint> points = Vertices(SlidePresetGeometry.Outline("hexagon", Box));

        Expect(points, [(0, 36), (18, 0), (90, 0), (108, 36), (90, 72), (18, 72)]);
    }

    [Fact]
    public void APentagonNeedsTheTrigonometry()
    {
        // Its guides are sin and cos of 18 and 54 degrees, so a formula evaluator that handled
        // only the arithmetic operators would put its two upper corners on the box's own edges.
        // The reference draws the second vertex at 269.972, 468.028 on a shape at 216, 396 —
        // the apex — and the third at 323.943, 440.532.
        List<DocPoint> points = Vertices(SlidePresetGeometry.Outline("pentagon", Box));

        points.Count.ShouldBe(5);
        points[1].X.Points.ShouldBe(54, 0.05);
        points[1].Y.Points.ShouldBe(0, 0.05);
        points[2].X.Points.ShouldBe(108, 0.05);
        points[2].Y.Points.ShouldBe(27.503, 0.05);
    }

    [Fact]
    public void APiesArcEndsWhereTheEllipseCrossesTheStatedAngle()
    {
        // A 240 degree sweep on a three-by-two box. The angle names a *direction*, and where that
        // ray crosses the ellipse is 249 degrees of the ellipse's own parameter — so the arc ends
        // 7.6 pt from where the stated angle alone would put it. The reference ends it at
        // 394.583, 465.619 on a shape whose top left corner its own PDF puts at 359.972, 71.972 —
        // 34.58 pt across and 2.38 down from that corner, where the stated 240 degrees alone
        // would give 27.0 across and 4.8 down.
        Dictionary<string, double> adjustments = new(StringComparer.Ordinal)
        {
            ["adj1"] = 0,
            ["adj2"] = 14400000,
        };

        List<DocPoint> points = Vertices(
            SlidePresetGeometry.Outline("pie", Box, adjustments));

        // The last two are the arc's end and the centre it closes back to.
        points[^2].X.Points.ShouldBe(34.583, 0.05);
        points[^2].Y.Points.ShouldBe(2.381, 0.05);
        points[^1].X.Points.ShouldBe(54, 0.05);
        points[^1].Y.Points.ShouldBe(36, 0.05);
    }

    [Fact]
    public void AnEllipsesTextRectangleIsTheBoxInscribedAtFortyFiveDegrees()
    {
        // Which is why a caption inside a circle does not touch its edge. The preset states it
        // itself, as cos and sin of 45 degrees of the half-axes.
        DocRect text = SlidePresetGeometry.TextRectangle("ellipse", Box);

        text.X.Points.ShouldBe(54 - (54 * 0.7071), 0.05);
        text.Width.Points.ShouldBe(2 * 54 * 0.7071, 0.05);
        text.Height.Points.ShouldBe(2 * 36 * 0.7071, 0.05);
    }

    /// <summary>The on-curve points of a path, which for a polygon are its vertices.</summary>
    private static List<DocPoint> Vertices(GraphicsPath path)
        => [.. path.Commands.Where(c => c.Verb != PathVerb.Close).Select(c => c.Point)];

    private static void Expect(List<DocPoint> points, (double X, double Y)[] expected)
    {
        points.Count.ShouldBe(expected.Length);

        for (int i = 0; i < expected.Length; i++)
        {
            points[i].X.Points.ShouldBe(expected[i].X, 0.05, $"vertex {i + 1} across");
            points[i].Y.Points.ShouldBe(expected[i].Y, 0.05, $"vertex {i + 1} down");
        }
    }
}
