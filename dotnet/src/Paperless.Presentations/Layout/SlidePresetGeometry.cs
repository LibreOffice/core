using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Presentations.Layout;

/// <summary>
/// Expands the handful of DrawingML preset geometries slides actually use.
/// </summary>
/// <remarks>
/// <para>
/// There are 187 presets, each a small program: guide formulas evaluated against the bounding
/// box and the adjustment handles, then a path built from the results
/// (<c>oox/source/drawingml/customshapes/presetShapeDefinitions.xml</c>, which LibreOffice
/// compiles into data tables and evaluates with one shared engine). Porting that engine is a
/// separate piece of work, deliberately not started here — see the note in
/// <c>src/Paperless.Presentations/TODO.md</c>.
/// </para>
/// <para>
/// What is here instead is the six whose paths are one line each and which between them cover
/// almost every shape a real deck draws: <c>rect</c> and its aliases, <c>roundRect</c>,
/// <c>ellipse</c>, <c>triangle</c>, <c>rtTriangle</c> and <c>diamond</c>. Each is transcribed
/// from that file rather than guessed, and anything else falls back to its bounding rectangle —
/// which is where the shape is, in the right colour, with the wrong outline. That is a far
/// better failure than drawing nothing, and it is what makes an unknown preset visible in a
/// comparison instead of silently absent.
/// </para>
/// <para>
/// The <em>text</em> rectangle is carried alongside the outline because it is not always the
/// bounding box: an ellipse's is the box inscribed at 45°, which is
/// <c>Transform2DContext</c>'s <c>ConstructPresetTextRectangle</c>
/// (<c>oox/source/drawingml/transform2dcontext.cxx:66-73</c>) and the reason a caption inside a
/// circle does not touch its edge.
/// </para>
/// </remarks>
public static class SlidePresetGeometry
{
    /// <summary>
    /// The Bezier control-point distance that approximates a quarter circle, as a fraction of
    /// the radius.
    /// </summary>
    /// <remarks>
    /// 4(√2 − 1)/3. Every renderer approximates arcs this way — LibreOffice's
    /// <c>basegfx::B2DPolygon</c> included — so an ellipse drawn from cubics agrees with the
    /// reference to well under a printer dot rather than being an independent approximation.
    /// </remarks>
    private const double QuarterCircle = 0.5522847498307933;

    /// <summary>True when the preset is one this expands rather than approximates.</summary>
    public static bool IsKnown(string? preset) => Normalise(preset) is not null;

    /// <summary>
    /// The outline of a preset shape, in the shape's own coordinates — origin at its top left.
    /// </summary>
    /// <param name="preset">The <c>a:prstGeom/@prst</c> value, or null for a plain box.</param>
    /// <param name="size">The shape's extent.</param>
    /// <param name="adjustment">
    /// The single adjustment value the supported presets take, in thousandths of a percent, or
    /// null for the preset's own default.
    /// </param>
    public static GraphicsPath Outline(string? preset, DocSize size, int? adjustment = null)
    {
        double width = size.Width.Emu;
        double height = size.Height.Emu;

        return Normalise(preset) switch
        {
            "ellipse" => Ellipse(width, height),
            "roundRect" => RoundRect(width, height, adjustment ?? 16667),
            "triangle" => Triangle(width, height, adjustment ?? 50000),
            "rtTriangle" => RightTriangle(width, height),
            "diamond" => Diamond(width, height),
            _ => Rectangle(width, height),
        };
    }

    /// <summary>
    /// The rectangle text is laid out in, in the shape's own coordinates.
    /// </summary>
    /// <remarks>
    /// The bounding box for everything but the two presets whose own <c>&lt;rect&gt;</c> is
    /// smaller. Falling back to the bounding box for an unknown preset is what LibreOffice does
    /// too: the preset's text rectangle defaults to the whole box.
    /// </remarks>
    /// <param name="preset">The <c>a:prstGeom/@prst</c> value, or null.</param>
    /// <param name="size">The shape's extent.</param>
    /// <param name="adjustment">The adjustment value, or null for the default.</param>
    public static DocRect TextRectangle(string? preset, DocSize size, int? adjustment = null)
    {
        double width = size.Width.Emu;
        double height = size.Height.Emu;

        switch (Normalise(preset))
        {
            case "ellipse":
            {
                // The preset's own guides are cos/sin at 45 degrees of the half-axes, which is
                // the inscribed box: half a side is the half-axis over root two.
                const double Root2Reciprocal = 0.7071067811865476;
                double insetX = width * (1 - Root2Reciprocal) / 2;
                double insetY = height * (1 - Root2Reciprocal) / 2;
                return Rect(insetX, insetY, width - (2 * insetX), height - (2 * insetY));
            }

            case "roundRect":
            {
                double radius = Radius(width, height, adjustment ?? 16667);
                double inset = radius * 29289 / 100000.0;
                return Rect(inset, inset, width - (2 * inset), height - (2 * inset));
            }

            default:
                return Rect(0, 0, width, height);
        }
    }

    /// <summary>
    /// Maps a preset name onto the one this expands it as, or null when it is not supported.
    /// </summary>
    /// <remarks>
    /// The rectangle aliases are not a convenience: <c>flowChartProcess</c>, <c>actionButton*</c>
    /// and a text box's implicit geometry are all literally a rectangle in the preset file, so
    /// treating them as one is exact rather than approximate.
    /// </remarks>
    private static string? Normalise(string? preset) => preset switch
    {
        null or "" or "rect" or "flowChartProcess" or "flowChartPredefinedProcess"
            or "flowChartInternalStorage" or "actionButtonBlank" => "rect",
        "ellipse" or "flowChartConnector" => "ellipse",
        "roundRect" or "flowChartAlternateProcess" => "roundRect",
        "triangle" or "flowChartExtract" => "triangle",
        "rtTriangle" => "rtTriangle",
        "diamond" or "flowChartDecision" => "diamond",
        _ => null,
    };

    private static double Radius(double width, double height, int adjustment)
    {
        // "ss" in the preset's guides: the shorter side. A radius derived from the width alone
        // rounds a wide, shallow box into a stadium.
        double shorterSide = Math.Min(width, height);
        return shorterSide * Math.Clamp(adjustment, 0, 50000) / 100000.0;
    }

    private static GraphicsPath Rectangle(double width, double height)
        => new GraphicsPath()
            .MoveTo(Point(0, 0))
            .LineTo(Point(width, 0))
            .LineTo(Point(width, height))
            .LineTo(Point(0, height))
            .Close();

    private static GraphicsPath RightTriangle(double width, double height)
        => new GraphicsPath()
            .MoveTo(Point(0, height))
            .LineTo(Point(0, 0))
            .LineTo(Point(width, height))
            .Close();

    private static GraphicsPath Triangle(double width, double height, int adjustment)
    {
        double apex = width * Math.Clamp(adjustment, 0, 100000) / 100000.0;
        return new GraphicsPath()
            .MoveTo(Point(0, height))
            .LineTo(Point(apex, 0))
            .LineTo(Point(width, height))
            .Close();
    }

    private static GraphicsPath Diamond(double width, double height)
        => new GraphicsPath()
            .MoveTo(Point(0, height / 2))
            .LineTo(Point(width / 2, 0))
            .LineTo(Point(width, height / 2))
            .LineTo(Point(width / 2, height))
            .Close();

    private static GraphicsPath Ellipse(double width, double height)
    {
        double halfWidth = width / 2;
        double halfHeight = height / 2;
        double controlX = halfWidth * QuarterCircle;
        double controlY = halfHeight * QuarterCircle;

        // Starting at the left and going clockwise, which is the preset's own path order.
        return new GraphicsPath()
            .MoveTo(Point(0, halfHeight))
            .CubicTo(
                Point(0, halfHeight - controlY),
                Point(halfWidth - controlX, 0),
                Point(halfWidth, 0))
            .CubicTo(
                Point(halfWidth + controlX, 0),
                Point(width, halfHeight - controlY),
                Point(width, halfHeight))
            .CubicTo(
                Point(width, halfHeight + controlY),
                Point(halfWidth + controlX, height),
                Point(halfWidth, height))
            .CubicTo(
                Point(halfWidth - controlX, height),
                Point(0, halfHeight + controlY),
                Point(0, halfHeight))
            .Close();
    }

    private static GraphicsPath RoundRect(double width, double height, int adjustment)
    {
        double radius = Radius(width, height, adjustment);
        if (radius <= 0) return Rectangle(width, height);

        double control = radius * QuarterCircle;

        return new GraphicsPath()
            .MoveTo(Point(0, radius))
            .CubicTo(
                Point(0, radius - control), Point(radius - control, 0), Point(radius, 0))
            .LineTo(Point(width - radius, 0))
            .CubicTo(
                Point(width - radius + control, 0), Point(width, radius - control), Point(width, radius))
            .LineTo(Point(width, height - radius))
            .CubicTo(
                Point(width, height - radius + control),
                Point(width - radius + control, height),
                Point(width - radius, height))
            .LineTo(Point(radius, height))
            .CubicTo(
                Point(radius - control, height), Point(0, height - radius + control), Point(0, height - radius))
            .Close();
    }

    private static DocPoint Point(double x, double y)
        => new(Length.FromEmu((long)Math.Round(x)), Length.FromEmu((long)Math.Round(y)));

    private static DocRect Rect(double x, double y, double width, double height)
        => new(
            Length.FromEmu((long)Math.Round(x)),
            Length.FromEmu((long)Math.Round(y)),
            Length.FromEmu((long)Math.Round(width)),
            Length.FromEmu((long)Math.Round(height)));
}
