using System.Globalization;
using System.Xml.Linq;
using Paperless.Core.Geometry;
using Paperless.Core.Units;
using Paperless.OpenDocument;
using Paperless.Ooxml.DrawingML;

namespace Paperless.Presentations.OpenDocument;

/// <summary>
/// Reads a <c>draw:enhanced-geometry</c> — ODF's spelling of a custom shape — into the shared
/// evaluator's model.
/// </summary>
/// <remarks>
/// <para>
/// <strong>The same shapes in a different language, and the sameness stops at the syntax.</strong>
/// LibreOffice turns every DrawingML preset into a <c>draw:enhanced-geometry</c> on import and back
/// again on export, so the two vocabularies describe exactly one set of shapes; what they do not
/// share is a single character of notation. A guide is <c>&lt;a:gd fmla="*/ w 3 4"/&gt;</c> there
/// and <c>&lt;draw:equation draw:formula="logwidth*3/4"/&gt;</c> here — prefix against infix, with
/// disjoint operator sets and different built-in names. A path is an element per command there and
/// a single command-letter string here, with twenty-one letters against DrawingML's six.
/// </para>
/// <para>
/// So the split is: this reads ODF's notation and resolves every operand to a number, and
/// <see cref="CustomShapeGeometry.Evaluate"/> draws it. Nothing about the *geometry* is duplicated
/// — the eccentric angle an arc's stated angle means, the quadrant split that keeps an ellipse's
/// leftmost point on the path, the cubic control distance — which is the part that is hard and the
/// part that was already measured. LibreOffice makes the same split in the same place: its two
/// parsers meet at <c>EnhancedCustomShapeParameter</c> and only
/// <c>EnhancedCustomShape2d::CreateSubPath</c> draws.
/// </para>
/// <para>
/// Ported from <c>xmloff/source/draw/ximpcustomshape.cxx</c> (the notation),
/// <c>svx/source/customshapes/EnhancedCustomShapeFunctionParser.cxx</c> (the expression grammar)
/// and <c>svx/source/customshapes/EnhancedCustomShape2d.cxx</c> (what the commands mean).
/// </para>
/// </remarks>
internal static class OdfEnhancedGeometry
{
    /// <summary>How deep an equation may refer to other equations before it is abandoned.</summary>
    private const int MaxEquationDepth = 64;

    /// <summary>
    /// Reads a shape's geometry, or returns null when it states no path.
    /// </summary>
    /// <param name="geometry">The <c>draw:enhanced-geometry</c> element.</param>
    /// <param name="size">The shape's extent, from its <c>svg:width</c> and <c>svg:height</c>.</param>
    public static CustomShapeGeometry.Geometry? Read(XElement geometry, DocSize size)
    {
        ArgumentNullException.ThrowIfNull(geometry);

        string? path = Attribute(geometry, "enhanced-path");
        if (string.IsNullOrWhiteSpace(path)) return null;

        Space space = SpaceOf(geometry, size);
        Values values = new(geometry, space, size);

        List<PresetPath> paths = Paths(path, values, space, geometry);
        if (paths.Count == 0) return null;

        return CustomShapeGeometry.Evaluate(
            new PresetShape(
                string.Empty, [], [], TextRectangle(geometry, values, space, size), paths),
            size);
    }

    /// <summary>
    /// The coordinate system a path's numbers are in, and what one unit of it is worth.
    /// </summary>
    /// <param name="Left">The view box's origin across, subtracted from every x.</param>
    /// <param name="Top">Its origin down.</param>
    /// <param name="Width">Its width, which the shape's width is divided into.</param>
    /// <param name="Height">Its height.</param>
    private readonly record struct Space(double Left, double Top, double Width, double Height);

    /// <summary>
    /// The <c>svg:viewBox</c>, with the degenerate case that means "already in 1/100 mm".
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>A shape LibreOffice imported from OOXML states <c>svg:viewBox="0 0 0 0"</c>, and
    /// that is not a malformed file.</strong> Its path coordinates are in the drawing layer's own
    /// unit rather than in a normalised box, and <c>SetPathSize</c> answers a zero coordinate width
    /// with a scale of exactly one for that reason
    /// (<c>EnhancedCustomShape2d.cxx:674-697</c>, under <c>m_bOOXMLShape</c>). Here the same thing
    /// is said as a view box the size of the shape in 1/100 mm, which makes the scale 360 EMU per
    /// unit and needs no second code path.
    /// </para>
    /// <para>
    /// It is also why <c>logwidth</c> and <c>logheight</c> are the shape's size in 1/100 mm and
    /// not in view-box units: they are <c>m_aLogicRect.GetWidth()</c>
    /// (<c>EnhancedCustomShape2d.cxx:879-880</c>), which is the drawing layer's rectangle. Reading
    /// them as the view box gives a shape of the right proportions and the wrong size on every
    /// converted deck, because every one of those equations is stated in terms of them.
    /// </para>
    /// </remarks>
    private static Space SpaceOf(XElement geometry, DocSize size)
    {
        double[] box = Numbers(Attribute(geometry, "viewBox", OdfNamespaces.SvgCompatible));

        double width = box.Length > 2 ? Math.Abs(box[2]) : 0;
        double height = box.Length > 3 ? Math.Abs(box[3]) : 0;

        if (width == 0 || height == 0)
        {
            return new Space(0, 0, size.Width.Mm100, size.Height.Mm100);
        }

        return new Space(box[0], box[1], width, height);
    }

    /// <summary>
    /// The names an ODF equation may use, and the equations themselves.
    /// </summary>
    /// <remarks>
    /// An equation is evaluated on demand and remembered, because ODF places no ordering
    /// requirement on the list — <c>?f0</c> may be stated in terms of <c>?f7</c>. The depth guard
    /// is what stops a file whose equations refer to each other in a circle from recursing until
    /// the stack ends; LibreOffice guards the same thing with a level counter
    /// (<c>EnhancedCustomShape2d.cxx:904</c>).
    /// </remarks>
    private sealed class Values
    {
        private readonly Dictionary<string, string> _formulas = new(StringComparer.Ordinal);
        private readonly Dictionary<string, double> _evaluated = new(StringComparer.Ordinal);
        private readonly double[] _modifiers;
        private readonly Space _space;
        private readonly DocSize _extent;
        private int _depth;

        public Values(XElement geometry, Space space, DocSize extent)
        {
            _space = space;
            _extent = extent;
            _modifiers = Numbers(Attribute(geometry, "modifiers"));

            foreach (XElement equation in
                     geometry.Elements(XName.Get("equation", OdfNamespaces.Draw)))
            {
                if (Attribute(equation, "name") is { } name
                    && Attribute(equation, "formula") is { } formula)
                {
                    _formulas[name] = formula;
                }
            }
        }

        /// <summary>Evaluates one <c>draw:formula</c> or path parameter.</summary>
        public double Evaluate(string expression)
        {
            if (_depth > MaxEquationDepth) return 0;

            _depth++;
            try
            {
                int at = 0;
                double value = Additive(expression, ref at);
                return double.IsFinite(value) ? value : 0;
            }
            finally
            {
                _depth--;
            }
        }

        /// <summary>The value of a named equation, computed once.</summary>
        private double Equation(string name)
        {
            if (_evaluated.TryGetValue(name, out double cached)) return cached;
            if (!_formulas.TryGetValue(name, out string? formula)) return 0;

            // Recorded before the recursion rather than after it, so an equation that refers to
            // itself resolves to zero instead of looping.
            _evaluated[name] = 0;
            double value = Evaluate(formula);
            _evaluated[name] = value;
            return value;
        }

        private double Modifier(int index)
            => index >= 0 && index < _modifiers.Length ? _modifiers[index] : 0;

        /// <summary>
        /// The names a formula may use without declaring them.
        /// </summary>
        /// <remarks>
        /// <c>EnhancedCustomShape2d::GetEnumFunc</c> (<c>EnhancedCustomShape2d.cxx:863-884</c>).
        /// <c>hasstroke</c> and <c>hasfill</c> are the shape's actual line and fill state there and
        /// are answered "yes" here: they exist so that a shape can shrink its outline by half a
        /// pen width, nothing in the corpus states one, and the alternative — threading the
        /// resolved stroke into the geometry — would make the outline depend on the style.
        /// <c>xstretch</c> and <c>ystretch</c> are the binary format's stretch point, absent from
        /// everything ODF writes.
        /// </remarks>
        private double Builtin(string name) => name switch
        {
            "pi" => Math.PI,
            "left" => _space.Left,
            "top" => _space.Top,
            "right" => _space.Left + _space.Width,
            "bottom" => _space.Top + _space.Height,
            "width" => _space.Width,
            "height" => _space.Height,
            "logwidth" => _extent.Width.Mm100,
            "logheight" => _extent.Height.Mm100,
            "hasstroke" => 1,
            "hasfill" => 1,
            "xstretch" => 0,
            "ystretch" => 0,
            _ => 0,
        };

        // The grammar, one method per precedence level, from
        // EnhancedCustomShapeFunctionParser.cxx's ExpressionGrammar.

        private double Additive(string text, ref int at)
        {
            double value = Multiplicative(text, ref at);

            while (true)
            {
                Skip(text, ref at);
                if (at >= text.Length) return value;

                char op = text[at];
                if (op is not ('+' or '-')) return value;

                at++;
                double right = Multiplicative(text, ref at);
                value = op == '+' ? value + right : value - right;
            }
        }

        private double Multiplicative(string text, ref int at)
        {
            double value = Unary(text, ref at);

            while (true)
            {
                Skip(text, ref at);
                if (at >= text.Length) return value;

                char op = text[at];
                if (op is not ('*' or '/')) return value;

                at++;
                double right = Unary(text, ref at);

                // A division by zero is zero rather than an infinity, which is o3tl's
                // div_allow_zero and what keeps a degenerate shape a shape.
                value = op == '*' ? value * right : right == 0 ? 0 : value / right;
            }
        }

        private double Unary(string text, ref int at)
        {
            Skip(text, ref at);
            if (at < text.Length && text[at] == '-')
            {
                at++;
                return -Basic(text, ref at);
            }

            return Basic(text, ref at);
        }

        private double Basic(string text, ref int at)
        {
            Skip(text, ref at);
            if (at >= text.Length) return 0;

            char first = text[at];

            if (first == '(')
            {
                at++;
                double inner = Additive(text, ref at);
                Expect(text, ref at, ')');
                return inner;
            }

            if (first == '?')
            {
                at++;
                return Equation(Word(text, ref at));
            }

            if (first == '$')
            {
                at++;
                return Modifier((int)Number(text, ref at));
            }

            if (char.IsAsciiDigit(first) || first == '.') return Number(text, ref at);

            string name = Word(text, ref at);
            Skip(text, ref at);

            if (at < text.Length && text[at] == '(')
            {
                at++;
                double a = Additive(text, ref at);
                double b = 0;
                double c = 0;

                Skip(text, ref at);
                if (at < text.Length && text[at] == ',')
                {
                    at++;
                    b = Additive(text, ref at);
                }

                Skip(text, ref at);
                if (at < text.Length && text[at] == ',')
                {
                    at++;
                    c = Additive(text, ref at);
                }

                Expect(text, ref at, ')');

                // The trigonometric functions take radians, unlike DrawingML's, whose angles are
                // in sixtieth-thousandths of a degree — a formula ported between the two without
                // the conversion produces a shape that is wrong in a way no reader would spot.
                return name switch
                {
                    "abs" => Math.Abs(a),
                    "sqrt" => Math.Sqrt(Math.Max(0, a)),
                    "sin" => Math.Sin(a),
                    "cos" => Math.Cos(a),
                    "tan" => Math.Tan(a),
                    "atan" => Math.Atan(a),
                    "min" => Math.Min(a, b),
                    "max" => Math.Max(a, b),
                    "atan2" => Math.Atan2(a, b),
                    "if" => a > 0 ? b : c,
                    _ => 0,
                };
            }

            return Builtin(name);
        }

        private static void Expect(string text, ref int at, char token)
        {
            Skip(text, ref at);
            if (at < text.Length && text[at] == token) at++;
        }

        private static void Skip(string text, ref int at)
        {
            while (at < text.Length && char.IsWhiteSpace(text[at])) at++;
        }

        private static string Word(string text, ref int at)
        {
            int start = at;
            while (at < text.Length && char.IsAsciiLetterOrDigit(text[at])) at++;
            return text[start..at];
        }

        private static double Number(string text, ref int at)
        {
            int start = at;
            while (at < text.Length && (char.IsAsciiDigit(text[at]) || text[at] == '.')) at++;

            return double.TryParse(
                text.AsSpan(start, at - start), NumberStyles.Float,
                CultureInfo.InvariantCulture, out double value)
                ? value
                : 0;
        }
    }

    /// <summary>
    /// The text rectangle, from the first frame <c>draw:text-areas</c> states.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Four parameters — left, top, right, bottom — in the path's own coordinates; a shape may
    /// state several frames and LibreOffice uses the first
    /// (<c>EnhancedCustomShape2d::GetTextRect</c>, index zero). A shape stating none gets its
    /// bounding box, which the evaluator supplies.
    /// </para>
    /// <para>
    /// <strong>Scaled here rather than by the evaluator, because DrawingML's is not.</strong> An
    /// <c>a:rect</c>'s four expressions are in the shape's own EMUs, so the evaluator maps them
    /// straight through; ODF's are in view-box units like everything else in the path. Leaving them
    /// unscaled put every ODF text rectangle 360 times too small, which showed up not as a missing
    /// rectangle but as a text box whose one line had wrapped to five.
    /// </para>
    /// </remarks>
    private static string[]? TextRectangle(
        XElement geometry, Values values, Space space, DocSize size)
    {
        string? areas = Attribute(geometry, "text-areas");
        if (string.IsNullOrWhiteSpace(areas)) return null;

        int at = 0;
        double[] corners = new double[4];
        for (int i = 0; i < 4; i++)
        {
            if (Parameter(areas, ref at, values) is not { } corner) return null;
            corners[i] = corner;
        }

        double scaleX = space.Width > 0 ? size.Width.Emu / space.Width : 1.0;
        double scaleY = space.Height > 0 ? size.Height.Emu / space.Height : 1.0;

        return
        [
            Literal((corners[0] - space.Left) * scaleX),
            Literal((corners[1] - space.Top) * scaleY),
            Literal((corners[2] - space.Left) * scaleX),
            Literal((corners[3] - space.Top) * scaleY),
        ];
    }

    /// <summary>
    /// Parses <c>draw:enhanced-path</c> into subpaths of commands the shared evaluator draws.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Two stages, as <c>GetEnhancedPath</c> and <c>CreateSubPath</c> are: the string becomes a
    /// list of coordinate pairs and a list of (command, count) segments, and the segments are then
    /// walked consuming pairs. That is not tidiness — it is the only way to honour the two rules
    /// ODF 1.2 §19.145 states about repetition. <strong>A command repeated is written once</strong>
    /// (<c>L 0 0 10 0 10 10</c> is three <c>lineto</c>s), and <strong>a moveto followed by more
    /// than one pair becomes a moveto and then linetos</strong>. A reader that treated each letter
    /// as one command draws the first segment of every run and drops the rest.
    /// </para>
    /// <para>
    /// <c>N</c> ends a subpath, and a subpath is where a stated coordinate space applies, so the
    /// commands are split into one <see cref="PresetPath"/> per <c>N</c> and each takes its own
    /// entry from <c>draw:sub-view-size</c> when the shape states one.
    /// </para>
    /// </remarks>
    private static List<PresetPath> Paths(
        string text, Values values, Space space, XElement geometry)
    {
        List<(double X, double Y)> coordinates = [];
        List<(char Command, int Count)> segments = [];

        Parse(text, values, space, coordinates, segments);

        double[] subViews = Numbers(Attribute(geometry, "sub-view-size"));

        List<PresetPath> paths = [];
        List<PresetCommand> commands = [];
        int point = 0;
        int subpath = 0;
        (double X, double Y) current = default;
        bool xDirection = true;

        void Flush()
        {
            if (commands.Count == 0) return;

            // A subpath's own coordinate space, when the shape states one; otherwise the view box.
            double width = subViews.Length > (subpath * 2) ? subViews[subpath * 2] : 0;
            double height = subViews.Length > ((subpath * 2) + 1) ? subViews[(subpath * 2) + 1] : 0;
            if (width == 0 || height == 0)
            {
                width = space.Width;
                height = space.Height;
            }

            paths.Add(new PresetPath(Literal(width), Literal(height), [.. commands]));
            commands = [];
            subpath++;
        }

        foreach ((char command, int count) in segments)
        {
            switch (command)
            {
                case 'M':
                case 'L':
                    for (int i = 0; i < count && point < coordinates.Count; i++)
                    {
                        current = coordinates[point++];
                        commands.Add(new PresetCommand(
                            command == 'M' ? PresetVerb.MoveTo : PresetVerb.LineTo,
                            [Literal(current.X), Literal(current.Y)]));
                    }

                    xDirection = true;
                    break;

                case 'C':
                    for (int i = 0; i < count && point + 2 < coordinates.Count; i++)
                    {
                        (double X, double Y) a = coordinates[point++];
                        (double X, double Y) b = coordinates[point++];
                        current = coordinates[point++];
                        commands.Add(new PresetCommand(
                            PresetVerb.CubicTo,
                            [
                                Literal(a.X), Literal(a.Y), Literal(b.X), Literal(b.Y),
                                Literal(current.X), Literal(current.Y),
                            ]));
                    }

                    break;

                case 'Q':
                    for (int i = 0; i < count && point + 1 < coordinates.Count; i++)
                    {
                        (double X, double Y) a = coordinates[point++];
                        current = coordinates[point++];
                        commands.Add(new PresetCommand(
                            PresetVerb.QuadraticTo,
                            [
                                Literal(a.X), Literal(a.Y),
                                Literal(current.X), Literal(current.Y),
                            ]));
                    }

                    break;

                case 'Z':
                    commands.Add(new PresetCommand(PresetVerb.Close, []));
                    break;

                case 'N':
                    Flush();
                    break;

                case 'T':
                case 'U':
                    for (int i = 0; i < count && point + 2 < coordinates.Count; i++)
                    {
                        (double X, double Y) centre = coordinates[point++];
                        (double X, double Y) radii = coordinates[point++];
                        (double X, double Y) angles = coordinates[point++];

                        // ODF: the segment runs clockwise in user view from the start angle to the
                        // end angle, and only an exact 360 draws the whole ellipse
                        // (EnhancedCustomShape2d.cxx:2301-2327).
                        double sweep = Math.Abs(Math.Abs(angles.Y - angles.X) - 360.0) < 1e-9
                            ? 360.0
                            : Clockwise(angles.Y - angles.X);

                        commands.Add(new PresetCommand(
                            command == 'U' ? PresetVerb.AngleEllipse : PresetVerb.AngleEllipseTo,
                            [
                                Literal(centre.X), Literal(centre.Y),
                                Literal(radii.X), Literal(radii.Y),
                                Literal(angles.X), Literal(sweep),
                            ]));

                        current = Ellipse(centre, radii, angles.X + sweep);
                    }

                    break;

                case 'A':
                case 'B':
                case 'W':
                case 'V':
                    for (int i = 0; i < count && point + 3 < coordinates.Count; i++)
                    {
                        (double X, double Y) one = coordinates[point];
                        (double X, double Y) two = coordinates[point + 1];
                        (double X, double Y) from = coordinates[point + 2];
                        (double X, double Y) to = coordinates[point + 3];
                        point += 4;

                        if (BoxArc(command, one, two, from, to) is not { } arc) continue;

                        commands.Add(arc.Command);
                        current = arc.End;
                    }

                    break;

                case 'G':
                    for (int i = 0; i < count && point + 1 < coordinates.Count; i++)
                    {
                        (double X, double Y) radii = coordinates[point++];
                        (double X, double Y) angles = coordinates[point++];

                        // The one command the two vocabularies share outright: a:arcTo. Its angles
                        // are in sixtieth-thousandths of a degree there and in whole degrees here,
                        // so the conversion happens at the boundary rather than in the evaluator.
                        commands.Add(new PresetCommand(
                            PresetVerb.ArcTo,
                            [
                                Literal(radii.X), Literal(radii.Y),
                                Literal(angles.X * CustomShapeGeometry.UnitsPerDegree),
                                Literal(angles.Y * CustomShapeGeometry.UnitsPerDegree),
                            ]));
                    }

                    break;

                case 'X':
                case 'Y':
                    // The direction alternates with every point, so a run of X commands draws a
                    // rounded corner one way and then the other (EnhancedCustomShape2d.cxx:2573).
                    xDirection = command == 'X';
                    for (int i = 0; i < count && point < coordinates.Count; i++)
                    {
                        (double X, double Y) end = coordinates[point++];

                        if (Quadrant(current, end, xDirection) is { } quadrant)
                        {
                            commands.Add(quadrant);
                            current = end;
                        }

                        xDirection = !xDirection;
                    }

                    break;

                default:
                    // F and S say the subpath is not filled or not stroked, and H, I, J and K
                    // shade its fill. Read as far as being skipped, which is where the OOXML side
                    // leaves a:path/@fill and @stroke as well: both need a preset to produce more
                    // than one PlacedShape, which is a change to the output rather than to this.
                    break;
            }
        }

        Flush();
        return paths;
    }

    /// <summary>
    /// One <c>A</c>, <c>B</c>, <c>W</c> or <c>V</c>: an arc stated by a box and two points.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The box gives the centre and the radii; the two points give the directions their rays
    /// leave the centre in, and the ellipse is met where those rays cross it — which is the same
    /// eccentric-angle question <c>a:arcTo</c> asks, so the answer is the same evaluator's.
    /// <c>W</c> and <c>V</c> sweep the other way, and both pairs draw from the third pair to the
    /// fourth: <c>CreateSubPath</c> swaps the two points <em>and</em> reverses the polygon
    /// (<c>EnhancedCustomShape2d.cxx:2377,2394-2396</c>), which cancels out except in direction.
    /// </para>
    /// <para>
    /// <strong>The reference draws these as a polyline, not as curves.</strong>
    /// <c>tools::Polygon</c>'s arc constructor emits between 16 and 256 straight segments, the
    /// count a function of the radii (<c>tools/source/generic/poly.cxx:260-266</c>). Cubics are the
    /// same ellipse and the same on-curve points at the quadrants, so a bounding-box comparison
    /// agrees and a vertex-for-vertex one cannot.
    /// </para>
    /// </remarks>
    private static (PresetCommand Command, (double X, double Y) End)? BoxArc(
        char command,
        (double X, double Y) one,
        (double X, double Y) two,
        (double X, double Y) from,
        (double X, double Y) to)
    {
        double left = Math.Min(one.X, two.X);
        double right = Math.Max(one.X, two.X);
        double top = Math.Min(one.Y, two.Y);
        double bottom = Math.Max(one.Y, two.Y);

        double radiusX = (right - left) / 2;
        double radiusY = (bottom - top) / 2;
        if (radiusX == 0 || radiusY == 0) return null;

        (double X, double Y) centre = (left + radiusX, top + radiusY);

        double start = Degrees(from.X - centre.X, from.Y - centre.Y);
        double finish = Degrees(to.X - centre.X, to.Y - centre.Y);

        bool clockwise = command is 'W' or 'V';
        double sweep = clockwise
            ? Clockwise(finish - start)
            : -Clockwise(start - finish);

        return (
            new PresetCommand(
                command is 'B' or 'V' ? PresetVerb.AngleEllipse : PresetVerb.AngleEllipseTo,
                [
                    Literal(centre.X), Literal(centre.Y),
                    Literal(radiusX), Literal(radiusY),
                    Literal(start), Literal(sweep),
                ]),
            Ellipse(centre, (radiusX, radiusY), start + sweep));
    }

    /// <summary>
    /// One <c>X</c> or <c>Y</c>: a quarter ellipse from the current point to a stated one.
    /// </summary>
    /// <remarks>
    /// The centre is the corner of the two points' bounding box that the tangent condition puts it
    /// at — the current point's x and the end point's y for <c>X</c>, the other way round for
    /// <c>Y</c> — so the arc leaves the current point vertically and arrives horizontally, or the
    /// reverse. Both endpoints then sit exactly on an axis, where the eccentric conversion is the
    /// identity, so the quarter meets the two stated points exactly however unequal the radii are.
    /// The eight cases <c>EnhancedCustomShape2d.cxx:2515-2570</c> spells out reduce to this and a
    /// ninety-degree sweep in whichever direction reaches the end.
    /// </remarks>
    private static PresetCommand? Quadrant(
        (double X, double Y) from, (double X, double Y) to, bool xDirection)
    {
        (double X, double Y) centre = xDirection ? (from.X, to.Y) : (to.X, from.Y);

        double radiusX = Math.Abs(to.X - from.X);
        double radiusY = Math.Abs(to.Y - from.Y);
        if (radiusX == 0 || radiusY == 0) return null;

        double start = Degrees(from.X - centre.X, from.Y - centre.Y);
        double finish = Degrees(to.X - centre.X, to.Y - centre.Y);

        // The shorter way round, signed: the two points are a quarter turn apart by construction.
        double sweep = (((finish - start + 540) % 360) + 360) % 360 - 180;

        return new PresetCommand(
            PresetVerb.AngleEllipseTo,
            [
                Literal(centre.X), Literal(centre.Y),
                Literal(radiusX), Literal(radiusY),
                Literal(start), Literal(sweep),
            ]);
    }

    /// <summary>An angle in degrees, measured clockwise from the positive x axis in a y-down space.</summary>
    private static double Degrees(double x, double y) => Math.Atan2(y, x) * 180.0 / Math.PI;

    /// <summary>A difference of angles as a positive sweep, a full turn rather than none.</summary>
    private static double Clockwise(double degrees)
    {
        double sweep = ((degrees % 360.0) + 360.0) % 360.0;
        return sweep == 0 ? 360.0 : sweep;
    }

    /// <summary>Where an ellipse angle lands, in the path's own coordinates.</summary>
    /// <remarks>
    /// Tracked so that an <c>X</c> or a <c>Y</c> following an arc knows where it starts from. The
    /// eccentric conversion is applied here as well as in the evaluator, because the two must agree
    /// about which point an angle names or a rounded corner meets the next edge at a visible step.
    /// </remarks>
    private static (double X, double Y) Ellipse(
        (double X, double Y) centre, (double X, double Y) radii, double degrees)
    {
        double normalised = ((degrees % 360.0) + 360.0) % 360.0;

        double parameter = normalised is 0.0 or 90.0 or 180.0 or 270.0
            ? normalised * Math.PI / 180.0
            : Math.Atan2(
                radii.X * Math.Sin(normalised * Math.PI / 180.0),
                radii.Y * Math.Cos(normalised * Math.PI / 180.0));

        return (
            centre.X + (radii.X * Math.Cos(parameter)),
            centre.Y + (radii.Y * Math.Sin(parameter)));
    }

    /// <summary>
    /// Splits a path string into coordinate pairs and (command, count) segments.
    /// </summary>
    /// <remarks>
    /// <c>GetEnhancedPath</c> (<c>xmloff/source/draw/ximpcustomshape.cxx:577-847</c>), with the
    /// segment count folded in the same way: a command whose letter is not repeated but whose
    /// parameters are simply increments the previous segment's count.
    /// </remarks>
    private static void Parse(
        string text,
        Values values,
        Space space,
        List<(double X, double Y)> coordinates,
        List<(char Command, int Count)> segments)
    {
        int at = 0;
        int pending = 0;
        int needed = 1;
        char latest = 'M';

        while (at < text.Length)
        {
            char token = text[at];

            if (Needs(token) is { } arity)
            {
                latest = token;
                needed = arity;
                at++;
            }
            else if (token is '$' or '?' or '.' or '-' || char.IsAsciiDigit(token)
                     || char.IsAsciiLetter(token))
            {
                if (Parameter(text, ref at, values) is not { } x) break;
                if (Parameter(text, ref at, values) is not { } y) break;

                coordinates.Add((x - space.Left, y - space.Top));
                pending++;
            }
            else
            {
                at++;
                continue;
            }

            if (pending == 0 && needed == 0)
            {
                segments.Add((latest, 0));
                needed = int.MaxValue;
            }
            else if (pending >= needed)
            {
                if (latest == 'M')
                {
                    // ODF 1.2 §19.145: "If a moveto is followed by multiple pairs of coordinates,
                    // they are treated as lineto." So the moveto takes the first pair and the
                    // command becomes a lineto for whatever follows.
                    segments.Add(('M', 1));
                    latest = 'L';
                    needed = 1;
                }
                else if (segments.Count > 0 && segments[^1].Command == latest)
                {
                    segments[^1] = (latest, segments[^1].Count + 1);
                }
                else
                {
                    segments.Add((latest, 1));
                }

                pending = 0;
            }
        }
    }

    /// <summary>How many coordinate pairs a command letter consumes, or null when it is not one.</summary>
    private static int? Needs(char command) => command switch
    {
        'M' or 'L' or 'X' or 'Y' => 1,
        'G' or 'Q' => 2,
        'T' or 'U' => 3,
        'A' or 'B' or 'W' or 'V' => 4,
        'Z' or 'N' or 'F' or 'S' or 'H' or 'I' or 'J' or 'K' => 0,
        _ => null,
    };

    /// <summary>
    /// One path parameter: a number, a <c>$</c> modifier, a <c>?</c> equation or a built-in name.
    /// </summary>
    /// <remarks>
    /// Resolved to a number here rather than carried as a name, which is what lets the shared
    /// evaluator take ODF's paths without knowing ODF's expression language: an equation may refer
    /// to any other equation in either direction, so there is no ordering to preserve and nothing
    /// is lost by evaluating early.
    /// </remarks>
    private static double? Parameter(string text, ref int at, Values values)
    {
        while (at < text.Length && (char.IsWhiteSpace(text[at]) || text[at] == ',')) at++;
        if (at >= text.Length) return null;

        bool negative = text[at] == '-';
        if (negative) at++;

        int start = at;

        if (at < text.Length && text[at] == '?')
        {
            at++;
            while (at < text.Length && char.IsAsciiLetterOrDigit(text[at])) at++;
        }
        else if (at < text.Length && text[at] == '$')
        {
            at++;
            while (at < text.Length && char.IsAsciiDigit(text[at])) at++;
        }
        else if (at < text.Length && char.IsAsciiLetter(text[at]))
        {
            while (at < text.Length && char.IsAsciiLetter(text[at])) at++;
        }
        else
        {
            while (at < text.Length && (char.IsAsciiDigit(text[at]) || text[at] == '.')) at++;
        }

        if (at == start) return null;

        double value = values.Evaluate(text[start..at]);
        return negative ? -value : value;
    }

    /// <summary>
    /// A resolved number as an operand the shared evaluator will parse back.
    /// </summary>
    /// <remarks>
    /// Round-trip format, which is exact for a double on .NET, so nothing is lost between the two
    /// halves. An operand is a string there because a DrawingML one is a guide <em>name</em>
    /// resolved against a table built in order; ODF has no such ordering, so its operands arrive
    /// already resolved and only need to survive the crossing.
    /// </remarks>
    private static string Literal(double value)
        => value.ToString("R", CultureInfo.InvariantCulture);

    private static double[] Numbers(string? value)
    {
        if (string.IsNullOrWhiteSpace(value)) return [];

        string[] tokens = value.Split(
            [' ', ',', '\t', '\n', '\r'], StringSplitOptions.RemoveEmptyEntries);

        List<double> numbers = [];
        foreach (string token in tokens)
        {
            if (double.TryParse(
                    token, NumberStyles.Float, CultureInfo.InvariantCulture, out double number))
            {
                numbers.Add(number);
            }
        }

        return [.. numbers];
    }

    private static string? Attribute(XElement element, string name, string? ns = null)
        => element.Attribute(XName.Get(name, ns ?? OdfNamespaces.Draw))?.Value;
}
