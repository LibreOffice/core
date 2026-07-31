using System.Globalization;
using System.Xml.Linq;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Units;

namespace Paperless.Ooxml.DrawingML;

/// <summary>
/// Evaluates DrawingML shape geometry: the guide formulas, then the path they describe.
/// </summary>
/// <remarks>
/// <para>
/// A preset shape is a small program. <c>a:avLst</c> gives the adjustment handles their defaults,
/// <c>a:gdLst</c> is a list of formulas evaluated in order against the bounding box and those
/// handles, and <c>a:pathLst</c> is a list of drawing commands whose operands are the names the
/// formulas defined. <c>a:custGeom</c> — a shape whose path the file states outright — is the same
/// program with the author's own guides and paths, so one evaluator serves both and the preset
/// definitions are only data.
/// </para>
/// <para>
/// Ported from <c>oox/source/drawingml/customshapegeometry.cxx</c>, which parses the same
/// vocabulary, and <c>svx/source/customshapes/EnhancedCustomShape2d.cxx</c>, which evaluates it.
/// LibreOffice goes through a third representation on the way — it translates each formula into an
/// <c>EnhancedCustomShapeParameter</c> and each command into a <c>drawing::EnhancedCustomShape</c>
/// segment, because that model also has to serve ODF's <c>draw:enhanced-geometry</c> and the
/// binary Escher syntax. Nothing here needs the intermediate form, so the formulas are evaluated
/// directly and the commands emitted straight into a <see cref="GraphicsPath"/>.
/// </para>
/// <para>
/// <strong>Everything is in the shape's own coordinates, y downwards, and every angle is in
/// sixtieth-thousandths of a degree.</strong> That is DrawingML's unit and it is also the unit
/// <c>a:xfrm/@rot</c> uses, so no conversion happens until a point becomes a
/// <see cref="DocPoint"/>. Positive angles turn clockwise, which in a y-down space is what the
/// ordinary sine and cosine give — the reason none of the trigonometry here has a sign flip in it.
/// </para>
/// </remarks>
public static class CustomShapeGeometry
{
    /// <summary>How many units of an angle guide make one degree.</summary>
    public const double UnitsPerDegree = 60000.0;

    /// <summary>
    /// The result of evaluating a shape's geometry: its outline and where its text goes.
    /// </summary>
    /// <param name="Outline">The path, in the shape's own coordinates with the origin at its top left.</param>
    /// <param name="TextRectangle">
    /// The rectangle <c>a:rect</c> names, or the whole bounding box when the shape states none.
    /// </param>
    public readonly record struct Geometry(GraphicsPath Outline, DocRect TextRectangle);

    /// <summary>
    /// Evaluates a preset by name.
    /// </summary>
    /// <param name="preset">The <c>a:prstGeom/@prst</c> value.</param>
    /// <param name="size">The shape's extent.</param>
    /// <param name="adjustments">
    /// The <c>a:avLst</c> the shape states, overriding the preset's own defaults by name. Null or
    /// empty for the preset's defaults.
    /// </param>
    /// <returns>The geometry, or null when the name is not a preset this knows.</returns>
    public static Geometry? Preset(
        string? preset, DocSize size, IReadOnlyDictionary<string, double>? adjustments = null)
    {
        if (preset is null || PresetShapeGeometry.Find(preset) is not { } definition) return null;

        return Evaluate(definition, size, adjustments);
    }

    /// <summary>
    /// Evaluates an <c>a:custGeom</c> — a shape whose guides and paths the file states itself.
    /// </summary>
    /// <param name="geometry">The <c>a:custGeom</c> element.</param>
    /// <param name="size">The shape's extent.</param>
    public static Geometry? Custom(XElement? geometry, DocSize size)
    {
        if (geometry is null) return null;

        List<(string Name, string Formula)> adjustments = [];
        foreach (XElement guide in Drawing.Children(Drawing.Child(geometry, "avLst"), "gd"))
        {
            if (Drawing.Attribute(guide, "name") is { } name
                && Drawing.Attribute(guide, "fmla") is { } formula)
            {
                adjustments.Add((name, formula));
            }
        }

        List<(string Name, string Formula)> guides = [];
        foreach (XElement guide in Drawing.Children(Drawing.Child(geometry, "gdLst"), "gd"))
        {
            if (Drawing.Attribute(guide, "name") is { } name
                && Drawing.Attribute(guide, "fmla") is { } formula)
            {
                guides.Add((name, formula));
            }
        }

        XElement? rect = Drawing.Child(geometry, "rect");
        string[]? textRectangle = rect is null
            ? null
            :
            [
                Drawing.Attribute(rect, "l") ?? "l", Drawing.Attribute(rect, "t") ?? "t",
                Drawing.Attribute(rect, "r") ?? "r", Drawing.Attribute(rect, "b") ?? "b",
            ];

        List<PresetPath> paths = [];
        foreach (XElement path in Drawing.Children(Drawing.Child(geometry, "pathLst"), "path"))
        {
            List<PresetCommand> commands = [];
            foreach (XElement command in path.Elements())
            {
                if (Command(command) is { } read) commands.Add(read);
            }

            paths.Add(new PresetPath(
                Drawing.Attribute(path, "w") ?? "0",
                Drawing.Attribute(path, "h") ?? "0",
                commands));
        }

        return paths.Count == 0
            ? null
            : Evaluate(
                new PresetShape(string.Empty, adjustments, guides, textRectangle, paths),
                size,
                adjustments: null);
    }

    private static PresetCommand? Command(XElement element)
    {
        static string[] Points(XElement parent, int count)
        {
            string[] operands = new string[count * 2];
            int i = 0;

            foreach (XElement point in Drawing.Children(parent, "pt"))
            {
                if (i + 1 >= operands.Length) break;
                operands[i++] = Drawing.Attribute(point, "x") ?? "0";
                operands[i++] = Drawing.Attribute(point, "y") ?? "0";
            }

            return operands;
        }

        return element.Name.LocalName switch
        {
            "moveTo" => new PresetCommand(PresetVerb.MoveTo, Points(element, 1)),
            "lnTo" => new PresetCommand(PresetVerb.LineTo, Points(element, 1)),
            "quadBezTo" => new PresetCommand(PresetVerb.QuadraticTo, Points(element, 2)),
            "cubicBezTo" => new PresetCommand(PresetVerb.CubicTo, Points(element, 3)),
            "close" => new PresetCommand(PresetVerb.Close, []),
            "arcTo" => new PresetCommand(
                PresetVerb.ArcTo,
                [
                    Drawing.Attribute(element, "wR") ?? "0",
                    Drawing.Attribute(element, "hR") ?? "0",
                    Drawing.Attribute(element, "stAng") ?? "0",
                    Drawing.Attribute(element, "swAng") ?? "0",
                ]),
            _ => null,
        };
    }

    /// <summary>Evaluates a definition against a size.</summary>
    private static Geometry Evaluate(
        PresetShape shape, DocSize size, IReadOnlyDictionary<string, double>? adjustments)
    {
        Dictionary<string, double> values = Builtins(size);

        // The adjustment handles first, in the order the preset declares them, with the shape's
        // own a:avLst overriding a default by name. A shape states the handles it moved and
        // nothing else, so an unstated one has to keep the preset's value rather than becoming
        // zero — which is the difference between a rounded rectangle and a rectangle.
        foreach ((string name, string formula) in shape.Adjustments)
        {
            values[name] = adjustments is not null && adjustments.TryGetValue(name, out double stated)
                ? stated
                : Formula(formula, values);
        }

        // Then the guides, strictly in order: each may name any guide before it, and several
        // presets define a name twice, in which case the later one wins from that point on.
        foreach ((string name, string formula) in shape.Guides)
        {
            values[name] = Formula(formula, values);
        }

        GraphicsPath outline = new();
        DocPoint current = default;
        DocPoint start = default;

        foreach (PresetPath path in shape.Paths)
        {
            // A subpath may state its own coordinate space, in which case its numbers are in that
            // space and scale to the shape. Absent or zero means the shape's own.
            double width = Value(path.Width, values);
            double height = Value(path.Height, values);
            double scaleX = width > 0 ? size.Width.Emu / width : 1.0;
            double scaleY = height > 0 ? size.Height.Emu / height : 1.0;

            foreach (PresetCommand command in path.Commands)
            {
                Emit(command, outline, values, scaleX, scaleY, ref current, ref start);
            }
        }

        return new Geometry(outline, TextRectangle(shape, values, size));
    }

    private static DocRect TextRectangle(
        PresetShape shape, Dictionary<string, double> values, DocSize size)
    {
        if (shape.TextRectangle is not { Length: 4 } rectangle)
            return new DocRect(Length.Zero, Length.Zero, size.Width, size.Height);

        double left = Value(rectangle[0], values);
        double top = Value(rectangle[1], values);
        double right = Value(rectangle[2], values);
        double bottom = Value(rectangle[3], values);

        return new DocRect(
            Emu(left), Emu(top), Emu(Math.Max(0, right - left)), Emu(Math.Max(0, bottom - top)));
    }

    private static void Emit(
        PresetCommand command,
        GraphicsPath outline,
        Dictionary<string, double> values,
        double scaleX,
        double scaleY,
        ref DocPoint current,
        ref DocPoint start)
    {
        DocPoint Point(int index)
        {
            double x = Value(command.Operands[index], values) * scaleX;
            double y = Value(command.Operands[index + 1], values) * scaleY;
            return new DocPoint(Emu(x), Emu(y));
        }

        switch (command.Verb)
        {
            case PresetVerb.MoveTo:
                current = start = Point(0);
                outline.MoveTo(current);
                break;

            case PresetVerb.LineTo:
                current = Point(0);
                outline.LineTo(current);
                break;

            case PresetVerb.CubicTo:
                outline.CubicTo(Point(0), Point(2), current = Point(4));
                break;

            case PresetVerb.QuadraticTo:
            {
                // Raised to a cubic, because the display list has no quadratic: the two control
                // points sit two thirds of the way from each end towards the quadratic's own,
                // which is exact rather than an approximation.
                DocPoint control = Point(0);
                DocPoint end = Point(2);

                outline.CubicTo(
                    Between(current, control, 2.0 / 3.0),
                    Between(end, control, 2.0 / 3.0),
                    end);

                current = end;
                break;
            }

            case PresetVerb.ArcTo:
                current = Arc(command, outline, values, scaleX, scaleY, current);
                break;

            case PresetVerb.Close:
                outline.Close();
                current = start;
                break;
        }
    }

    /// <summary>
    /// Draws an <c>a:arcTo</c>, which is an elliptical arc stated by its radii and two angles.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The command names no centre and no end point: the arc <em>starts</em> at the current point,
    /// so the centre is wherever the start angle puts it —
    /// <c>current − (wR·cos(stAng), hR·sin(stAng))</c> — and the end is the swept angle away from
    /// it. That is <c>EnhancedCustomShape2d::CreateSubPath</c>'s <c>ARCANGLETO</c>
    /// (<c>svx/source/customshapes/EnhancedCustomShape2d.cxx</c>), and getting it the other way
    /// round — treating the current point as the centre — draws every rounded corner of every
    /// preset in the wrong place.
    /// </para>
    /// <para>
    /// Swept in quarter-circle cubic segments, which is how every renderer approximates an arc and
    /// what keeps a preset ellipse agreeing with the reference's to well under a printer dot.
    /// </para>
    /// </remarks>
    private static DocPoint Arc(
        PresetCommand command,
        GraphicsPath outline,
        Dictionary<string, double> values,
        double scaleX,
        double scaleY,
        DocPoint current)
    {
        double radiusX = Value(command.Operands[0], values) * scaleX;
        double radiusY = Value(command.Operands[1], values) * scaleY;
        double startAngle = Value(command.Operands[2], values) / UnitsPerDegree;

        // MS Office clamps a swing to one full turn and neither specification says so, which is
        // why LibreOffice does it only for shapes that came from OOXML
        // (EnhancedCustomShape2d.cxx:2418-2424, tdf#122323).
        double sweepAngle = Math.Clamp(
            Value(command.Operands[3], values) / UnitsPerDegree, -360.0, 360.0);

        if (radiusX == 0 || radiusY == 0)
        {
            // A zero radius is a degenerate arc, which several presets use as "no rounding here".
            // A line to the same point keeps the path closed rather than leaving a gap.
            outline.LineTo(current);
            return current;
        }

        // Split at the quadrant boundaries rather than into equal parts, which is what
        // createPolygonFromEllipseSegment does. It matters beyond curve quality: an arc broken
        // into three eighty-degree pieces has no on-curve point at 180°, so it never reaches the
        // ellipse's own leftmost point and the shape comes out narrower than it is.
        List<double> stops = [startAngle];
        double direction = Math.Sign(sweepAngle);

        if (direction != 0)
        {
            double quadrant = (Math.Floor(startAngle / 90.0) + (direction > 0 ? 1 : 0)) * 90.0;
            double finish = startAngle + sweepAngle;

            while ((direction > 0 && quadrant < finish) || (direction < 0 && quadrant > finish))
            {
                if (Math.Abs(quadrant - startAngle) > 1e-9) stops.Add(quadrant);
                quadrant += 90.0 * direction;
            }
        }

        if (Math.Abs(stops[^1] - (startAngle + sweepAngle)) > 1e-9)
        {
            stops.Add(startAngle + sweepAngle);
        }

        int segments = Math.Max(1, stops.Count - 1);

        double[] parameters = new double[segments + 1];
        parameters[0] = Eccentric(startAngle, radiusX, radiusY);

        for (int i = 1; i <= segments; i++)
        {
            double stated = i < stops.Count ? stops[i] : startAngle + sweepAngle;
            double raw = Eccentric(stated, radiusX, radiusY);

            // Unwrapped against the previous parameter, because atan2 comes back in (−π, π] and a
            // segment that crosses the branch cut would otherwise sweep the long way round.
            while (raw - parameters[i - 1] > Math.PI) raw -= 2 * Math.PI;
            while (parameters[i - 1] - raw > Math.PI) raw += 2 * Math.PI;

            parameters[i] = raw;
        }

        double centreX = current.X.Emu - (radiusX * Math.Cos(parameters[0]));
        double centreY = current.Y.Emu - (radiusY * Math.Sin(parameters[0]));

        DocPoint end = current;

        for (int i = 0; i < segments; i++)
        {
            double from = parameters[i];
            double to = parameters[i + 1];

            // The control-point distance for a cubic approximating an arc of this angle. Reduces
            // to 0.5523 — four thirds of (root two minus one) — at a quarter circle, which is the
            // number every ellipse in every renderer is drawn with.
            double alpha = 4.0 / 3.0 * Math.Tan((to - from) / 4.0);

            double sinFrom = Math.Sin(from);
            double cosFrom = Math.Cos(from);
            double sinTo = Math.Sin(to);
            double cosTo = Math.Cos(to);

            DocPoint next = new(
                Emu(centreX + (radiusX * cosTo)), Emu(centreY + (radiusY * sinTo)));

            outline.CubicTo(
                new DocPoint(
                    Emu(centreX + (radiusX * (cosFrom - (alpha * sinFrom)))),
                    Emu(centreY + (radiusY * (sinFrom + (alpha * cosFrom))))),
                new DocPoint(
                    Emu(centreX + (radiusX * (cosTo + (alpha * sinTo)))),
                    Emu(centreY + (radiusY * (sinTo - (alpha * cosTo))))),
                next);

            end = next;
        }

        return end;
    }

    /// <summary>
    /// The ellipse parameter that an <c>a:arcTo</c> angle names.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>An arc's angle is a direction, not a parameter</strong>, and on an ellipse the two
    /// differ. The angle names the ray from the centre; the point drawn is where that ray crosses
    /// the ellipse, which for <c>x = a·cos t, y = b·sin t</c> is <c>tan t = (a/b)·tan θ</c> —
    /// <c>lcl_getNormalizedCircleAngleRad</c>
    /// (<c>svx/source/customshapes/EnhancedCustomShape2d.cxx:2040-2057</c>).
    /// </para>
    /// <para>
    /// It is invisible on every circular arc, which is nearly all of them: every rounded corner
    /// has <c>wR == hR</c>, and every quarter-turn lands on a multiple of 90° where the conversion
    /// is the identity — which LibreOffice special-cases outright, to keep the floating point
    /// exact. So the six presets that were transcribed by hand agreed without it, and
    /// <c>pie</c> with a 240° sweep on a 3:2 box does not: the reference ends its arc at 249°
    /// in parameter terms, 7.6 pt from where the stated angle alone would put it.
    /// </para>
    /// </remarks>
    private static double Eccentric(double degrees, double radiusX, double radiusY)
    {
        double normalised = degrees % 360.0;
        if (normalised < 0) normalised += 360.0;

        // Exactly on an axis the conversion is the identity, and saying so keeps a quarter turn
        // free of the rounding atan2 would otherwise introduce.
        if (normalised is 0.0 or 90.0 or 180.0 or 270.0)
            return normalised * Math.PI / 180.0;

        double radians = normalised * Math.PI / 180.0;
        return Math.Atan2(radiusX * Math.Sin(radians), radiusY * Math.Cos(radians));
    }

    private static DocPoint Between(DocPoint from, DocPoint to, double fraction) => new(
        Length.FromEmu((long)Math.Round(from.X.Emu + ((to.X.Emu - from.X.Emu) * fraction))),
        Length.FromEmu((long)Math.Round(from.Y.Emu + ((to.Y.Emu - from.Y.Emu) * fraction))));

    private static Length Emu(double value) => Length.FromEmu((long)Math.Round(value));

    private static double Radians(double units) => units / UnitsPerDegree * Math.PI / 180.0;

    /// <summary>
    /// The names every shape may use without defining them.
    /// </summary>
    /// <remarks>
    /// <c>EnhancedCustomShape2d</c> keeps the same table
    /// (<c>oox/source/drawingml/customshapegeometry.cxx</c>'s <c>GeomGuideList</c>): the box's
    /// edges and centres, the shorter and longer sides, halves and smaller fractions of each, and
    /// the four quadrant angles. <c>ss</c> — the shorter side — is the one that matters most: it
    /// is what a rounded corner's radius is a fraction of, and deriving it from the width alone
    /// turns a wide shallow box into a stadium.
    /// </remarks>
    private static Dictionary<string, double> Builtins(DocSize size)
    {
        double width = size.Width.Emu;
        double height = size.Height.Emu;
        double shorter = Math.Min(width, height);
        double longer = Math.Max(width, height);

        Dictionary<string, double> values = new(StringComparer.Ordinal)
        {
            ["l"] = 0,
            ["t"] = 0,
            ["r"] = width,
            ["b"] = height,
            ["w"] = width,
            ["h"] = height,
            ["hc"] = width / 2,
            ["vc"] = height / 2,
            ["ss"] = shorter,
            ["ls"] = longer,

            // A full turn is 21600000 units, so a quarter is 5400000. Written as the divisions the
            // preset file names rather than as literals, because that is how the formulas read.
            ["cd2"] = 10800000,
            ["cd4"] = 5400000,
            ["cd8"] = 2700000,
            ["3cd4"] = 16200000,
            ["3cd8"] = 8100000,
            ["5cd8"] = 13500000,
            ["7cd8"] = 18900000,
        };

        foreach (int divisor in (int[])[2, 3, 4, 5, 6, 8, 10, 12, 16, 20, 24, 32])
        {
            values[$"wd{divisor}"] = width / divisor;
            values[$"hd{divisor}"] = height / divisor;
            values[$"ssd{divisor}"] = shorter / divisor;
            values[$"lsd{divisor}"] = longer / divisor;
        }

        return values;
    }

    /// <summary>
    /// A named value, a literal, or zero.
    /// </summary>
    /// <remarks>
    /// Zero rather than a throw for a name nothing defined: real files reference guides they never
    /// declared, and the shape that comes out is wrong in one coordinate rather than absent.
    /// </remarks>
    private static double Value(string token, Dictionary<string, double> values)
    {
        if (values.TryGetValue(token, out double value)) return value;

        return double.TryParse(token, NumberStyles.Float, CultureInfo.InvariantCulture,
                               out double literal)
            ? literal
            : 0;
    }

    /// <summary>
    /// Evaluates one guide formula.
    /// </summary>
    /// <remarks>
    /// The seventeen operators of <c>ST_ShapeType</c>'s formula grammar, ported one for one from
    /// <c>oox/source/drawingml/customshapegeometry.cxx</c>'s <c>GeomGuide</c> parser. The three
    /// that are easy to get wrong: <c>?:</c> tests the first operand against zero and yields the
    /// second when it is <em>greater</em>, <c>mod</c> is the length of a three-dimensional vector
    /// rather than a remainder, and <c>pin</c> clamps its <em>middle</em> operand between the
    /// other two rather than its first.
    /// </remarks>
    private static double Formula(string formula, Dictionary<string, double> values)
    {
        string[] tokens = formula.Split(' ', StringSplitOptions.RemoveEmptyEntries);
        if (tokens.Length == 0) return 0;

        double Operand(int index)
            => index < tokens.Length ? Value(tokens[index], values) : 0;

        return tokens[0] switch
        {
            "*/" => Operand(3) != 0 ? Operand(1) * Operand(2) / Operand(3) : 0,
            "+-" => Operand(1) + Operand(2) - Operand(3),
            "+/" => Operand(3) != 0 ? (Operand(1) + Operand(2)) / Operand(3) : 0,
            "?:" => Operand(1) > 0 ? Operand(2) : Operand(3),
            "abs" => Math.Abs(Operand(1)),
            "at2" => Math.Atan2(Operand(2), Operand(1)) * 180.0 / Math.PI * UnitsPerDegree,
            "cat2" => Operand(1) * Math.Cos(Math.Atan2(Operand(3), Operand(2))),
            "cos" => Operand(1) * Math.Cos(Radians(Operand(2))),
            "max" => Math.Max(Operand(1), Operand(2)),
            "min" => Math.Min(Operand(1), Operand(2)),
            "mod" => Math.Sqrt(
                (Operand(1) * Operand(1)) + (Operand(2) * Operand(2)) + (Operand(3) * Operand(3))),
            "pin" => Math.Clamp(Operand(2), Math.Min(Operand(1), Operand(3)),
                                Math.Max(Operand(1), Operand(3))),
            "sat2" => Operand(1) * Math.Sin(Math.Atan2(Operand(3), Operand(2))),
            "sin" => Operand(1) * Math.Sin(Radians(Operand(2))),
            "sqrt" => Math.Sqrt(Math.Max(0, Operand(1))),
            "tan" => Operand(1) * Math.Tan(Radians(Operand(2))),
            "val" => Operand(1),
            _ => 0,
        };
    }
}

/// <summary>One drawing command of a preset path.</summary>
/// <param name="Verb">What it draws.</param>
/// <param name="Operands">Its operands, each a guide name or a literal.</param>
public readonly record struct PresetCommand(PresetVerb Verb, IReadOnlyList<string> Operands);

/// <summary>What a preset path command draws.</summary>
public enum PresetVerb
{
    /// <summary>Starts a new subpath.</summary>
    MoveTo,

    /// <summary>A straight segment.</summary>
    LineTo,

    /// <summary>An elliptical arc, by radii and two angles.</summary>
    ArcTo,

    /// <summary>A quadratic Bezier.</summary>
    QuadraticTo,

    /// <summary>A cubic Bezier.</summary>
    CubicTo,

    /// <summary>Closes the current subpath.</summary>
    Close,
}

/// <summary>One subpath of a preset, with the coordinate space its numbers are in.</summary>
/// <param name="Width">The space's width, or "0" for the shape's own.</param>
/// <param name="Height">Its height.</param>
/// <param name="Commands">Its commands, in order.</param>
public sealed record PresetPath(string Width, string Height, IReadOnlyList<PresetCommand> Commands);

/// <summary>One preset shape's definition.</summary>
/// <param name="Name">Its <c>a:prstGeom/@prst</c> name.</param>
/// <param name="Adjustments">The adjustment handles and their default formulas, in order.</param>
/// <param name="Guides">The guides, in the order they must be evaluated.</param>
/// <param name="TextRectangle">Its <c>a:rect</c> as four expressions, or null when it states none.</param>
/// <param name="Paths">Its subpaths.</param>
public sealed record PresetShape(
    string Name,
    IReadOnlyList<(string Name, string Formula)> Adjustments,
    IReadOnlyList<(string Name, string Formula)> Guides,
    string[]? TextRectangle,
    IReadOnlyList<PresetPath> Paths);
