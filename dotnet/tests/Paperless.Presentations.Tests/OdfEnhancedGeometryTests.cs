using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Presentations.Layout;
using Paperless.TestKit;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// Checks that an ODF shape's own <c>draw:enhanced-path</c> is what gets drawn.
/// </summary>
/// <remarks>
/// <para>
/// <c>odp-shape-paths.fodp</c> is hand-written, and it has to be: everything LibreOffice
/// <em>converts</em> into ODF comes out as an <c>ooxml-</c> preset whose path is five straight
/// commands, so a converted file cannot exercise a single one of ODF's own fifteen extra command
/// letters. The six shapes here each pin one — an equation with a modifier, a whole ellipse as one
/// <c>U</c>, a partial sweep, four quarter ellipses as <c>X</c> and <c>Y</c>, two <c>G</c> arcs,
/// and two polygons in one subpath.
/// </para>
/// <para>
/// The expected numbers are worked out from the view box alone: every shape is 6 × 4.5 cm and
/// every vertex a round fraction of 21600, so a wrong scale, a wrong origin or a wrong angle is a
/// difference of centimetres rather than of a rounding.
/// </para>
/// </remarks>
public class OdfEnhancedGeometryTests
{
    /// <summary>A twentieth of a point, as everywhere else on this path.</summary>
    private const double TolerancePoints = 0.05;

    /// <summary>6 cm across and 4.5 cm down, in points.</summary>
    private const double ShapeWidth = 6 * 72 / 2.54;

    private const double ShapeHeight = 4.5 * 72 / 2.54;

    private static SlidePages Layout()
    {
        using IDocument document = new PresentationReader().Read(
            DocumentSource.FromFile(Corpus.Require("odp-shape-paths.fodp")));

        return (SlidePages)((IPaginatedDocument)document).Layout();
    }

    private static PlacedShape Named(string name)
        => Layout().Slides[0].Shapes.First(shape => shape.Name == name);

    private static List<(double X, double Y)> Vertices(PlacedShape shape)
        => [.. shape.Outline.Commands
            .Where(command => command.Verb != PathVerb.Close)
            .Select(command => (command.Point.X.Points, command.Point.Y.Points))];

    /// <summary>
    /// A shape's outline is its own path, not the rectangle its preset name would give.
    /// </summary>
    /// <remarks>
    /// The chevron is six vertices, three of them stated as <c>?f0</c> and <c>?f1</c> over the
    /// modifier <c>$0</c>, so it fails in the loudest possible way if the equations are not
    /// evaluated: a four-vertex box.
    /// </remarks>
    [Fact]
    public void AnEquationOverAModifierPlacesTheChevronsNotch()
    {
        List<(double X, double Y)> vertices = Vertices(Named("Chevron"));

        // 1.5 cm, 2 cm in points; 5400/21600 of the width is a quarter of 6 cm.
        double left = 1.5 * 72 / 2.54;
        double top = 2 * 72 / 2.54;
        double quarter = ShapeWidth / 4;

        vertices.Count.ShouldBe(6);
        vertices[0].X.ShouldBe(left, TolerancePoints);
        vertices[0].Y.ShouldBe(top, TolerancePoints);

        // "L ?f1 0" — the notch's top, three quarters across.
        vertices[1].X.ShouldBe(left + (3 * quarter), TolerancePoints);
        vertices[1].Y.ShouldBe(top, TolerancePoints);

        // "21600 10800" — the point, at the right edge, half way down.
        vertices[2].X.ShouldBe(left + ShapeWidth, TolerancePoints);
        vertices[2].Y.ShouldBe(top + (ShapeHeight / 2), TolerancePoints);

        // "?f0 10800" — the notch itself, a quarter in.
        vertices[5].X.ShouldBe(left + quarter, TolerancePoints);
        vertices[5].Y.ShouldBe(top + (ShapeHeight / 2), TolerancePoints);
    }

    /// <summary>
    /// <c>U</c> draws the whole ellipse, which is one command ODF has and DrawingML has not.
    /// </summary>
    [Fact]
    public void AWholeEllipseIsOneAngleEllipseCommand()
    {
        PlacedShape shape = Named("WholeEllipse");
        DocRect bounds = Bounds(shape);

        double left = 11 * 72 / 2.54;
        double top = 2 * 72 / 2.54;

        bounds.X.Points.ShouldBe(left, TolerancePoints);
        bounds.Y.Points.ShouldBe(top, TolerancePoints);
        bounds.Width.Points.ShouldBe(ShapeWidth, TolerancePoints);
        bounds.Height.Points.ShouldBe(ShapeHeight, TolerancePoints);

        // Four quadrant cubics rather than a polygon: the split is at the quadrants, so the
        // ellipse's own extreme points are on the path and the box above is exact.
        shape.Outline.Commands.Count(command => command.Verb == PathVerb.CubicTo).ShouldBe(4);
    }

    /// <summary>
    /// A partial sweep's end angle is converted against the radii the <em>file</em> states.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>The named trap, and it cost the best part of an hour.</strong> An arc's stated angle
    /// names a ray from the centre and the point drawn is where that ray crosses the ellipse, which
    /// is <c>tan t = (a/b)·tan θ</c> — but <em>which</em> a and b? LibreOffice converts with the
    /// radii as the path states them and scales afterwards
    /// (<c>EnhancedCustomShape2d.cxx:2325-2327</c>), so a <c>U</c> whose view box is square and
    /// whose radii are both 10800 has no conversion at all: the arc is a circle stretched into the
    /// shape's box, and its 240° point is at <c>centre + (a·cos 240°, b·sin 240°)</c>.
    /// </para>
    /// <para>
    /// Converting the <em>scaled</em> radii instead gives 246.6° in parameter terms, ends the arc
    /// 9.33 pt away and drops the shape's bounding box 3.31 pt — which is what happened, because
    /// no DrawingML preset states a subpath coordinate space and every ODF path does. LibreOffice's
    /// own PDF puts the end at x = 623.622, exactly half a radius left of the centre, and that is
    /// what settled it.
    /// </para>
    /// </remarks>
    [Fact]
    public void APartialSweepConvertsItsAngleAgainstTheStatedRadii()
    {
        List<(double X, double Y)> vertices = Vertices(Named("PieSweep"));

        double centreX = (20.5 * 72 / 2.54) + (ShapeWidth / 2);
        double centreY = (2 * 72 / 2.54) + (ShapeHeight / 2);

        double endX = centreX + (ShapeWidth / 2 * Math.Cos(240 * Math.PI / 180));
        double endY = centreY + (ShapeHeight / 2 * Math.Sin(240 * Math.PI / 180));

        // The arc's last point, before the "L 10800 10800" back to the centre.
        (double X, double Y) tip = vertices[^1];
        tip.X.ShouldBe(centreX, TolerancePoints);
        tip.Y.ShouldBe(centreY, TolerancePoints);

        vertices[^2].X.ShouldBe(endX, TolerancePoints);
        vertices[^2].Y.ShouldBe(endY, TolerancePoints);

        // And the two readings really are far apart: converting the scaled radii would put the
        // arc's end 9.33 pt away and the shape's bounding box 3.31 pt lower, both of them tens of
        // times the tolerance everything here is held to.
        double parameter = Math.Atan2(
            (ShapeWidth / 2) * Math.Sin(240 * Math.PI / 180),
            (ShapeHeight / 2) * Math.Cos(240 * Math.PI / 180));

        double wrongX = centreX + (ShapeWidth / 2 * Math.Cos(parameter));
        double wrongY = centreY + (ShapeHeight / 2 * Math.Sin(parameter));

        Math.Sqrt(((wrongX - endX) * (wrongX - endX)) + ((wrongY - endY) * (wrongY - endY)))
            .ShouldBe(9.33, 0.01);
    }

    /// <summary>
    /// <c>X</c> and <c>Y</c> round a rectangle's corners with quarter ellipses.
    /// </summary>
    /// <remarks>
    /// Each meets its two stated points exactly, because both sit on an axis of the quarter's own
    /// ellipse — which is the reason the eight cases LibreOffice spells out reduce to one rule.
    /// </remarks>
    [Fact]
    public void QuadrantCommandsRoundTheCorners()
    {
        PlacedShape shape = Named("QuadrantCorners");
        DocRect bounds = Bounds(shape);

        double left = 1.5 * 72 / 2.54;
        double top = 9 * 72 / 2.54;

        bounds.X.Points.ShouldBe(left, TolerancePoints);
        bounds.Y.Points.ShouldBe(top, TolerancePoints);
        bounds.Width.Points.ShouldBe(ShapeWidth, TolerancePoints);
        bounds.Height.Points.ShouldBe(ShapeHeight, TolerancePoints);

        // Four corners, four straight edges, one move.
        shape.Outline.Commands.Count(command => command.Verb == PathVerb.CubicTo).ShouldBe(4);
        shape.Outline.Commands.Count(command => command.Verb == PathVerb.LineTo).ShouldBe(4);
    }

    /// <summary>
    /// <c>G</c> is the one command the two vocabularies share, in different angular units.
    /// </summary>
    [Fact]
    public void ArcAngleToClosesTheStadium()
    {
        PlacedShape shape = Named("ArcAngleStadium");
        DocRect bounds = Bounds(shape);

        double left = 11 * 72 / 2.54;
        double top = 9 * 72 / 2.54;

        // The two half-turns reach the box's left and right edges; the flat edges are a quarter
        // of the height in from the top and bottom.
        bounds.X.Points.ShouldBe(left, TolerancePoints);
        bounds.Width.Points.ShouldBe(ShapeWidth, TolerancePoints);
        bounds.Y.Points.ShouldBe(top + (ShapeHeight / 4), TolerancePoints);
        bounds.Height.Points.ShouldBe(ShapeHeight / 2, TolerancePoints);
    }

    /// <summary>
    /// Two polygons in one subpath, which is what makes a hole.
    /// </summary>
    /// <remarks>
    /// <c>Z</c> closes a polygon and a following <c>M</c> starts another inside the same object;
    /// <c>N</c> would end the object and give LibreOffice two separate filled shapes instead
    /// (<c>EnhancedCustomShape2d::CreateSubPath</c> is called once per <c>N</c> and builds one
    /// <c>SdrPathObj</c> each time). Measured on the reference: the file with one <c>N</c> renders
    /// with a hole and the same file with two renders solid.
    /// </remarks>
    [Fact]
    public void TwoPolygonsInOneSubpathMakeAHole()
    {
        PlacedShape shape = Named("FrameWithHole");

        shape.Outline.Commands.Count(command => command.Verb == PathVerb.MoveTo).ShouldBe(2);
        shape.Outline.Commands.Count(command => command.Verb == PathVerb.Close).ShouldBe(2);

        DocRect bounds = Bounds(shape);
        bounds.Width.Points.ShouldBe(ShapeWidth, TolerancePoints);
        bounds.Height.Points.ShouldBe(ShapeHeight, TolerancePoints);
    }

    private static DocRect Bounds(PlacedShape shape)
    {
        List<(double X, double Y)> points = Vertices(shape);

        double left = points.Min(point => point.X);
        double right = points.Max(point => point.X);
        double top = points.Min(point => point.Y);
        double bottom = points.Max(point => point.Y);

        return new DocRect(
            Core.Units.Length.FromPoints(left),
            Core.Units.Length.FromPoints(top),
            Core.Units.Length.FromPoints(right - left),
            Core.Units.Length.FromPoints(bottom - top));
    }
}
