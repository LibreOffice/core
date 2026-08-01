using Paperless.Core.Units;

namespace Paperless.Presentations.Ooxml;

/// <summary>
/// The layout algorithms a <c>dgm:alg</c> names, ported one for one from
/// <c>oox/source/drawingml/diagram/diagramlayoutatoms.cxx</c>.
/// </summary>
/// <remarks>
/// <para>
/// Every one of these takes a shape whose size is already decided and divides it among the
/// shapes inside. Nothing here decides its own size — the frame's size comes from the slide, and
/// each level down gets whatever the level above gave it — which is why the outermost algorithm
/// has to run first and why the walk is ordered the way it is.
/// </para>
/// <para>
/// <strong>The arithmetic is deliberately integer.</strong> LibreOffice divides
/// <c>sal_Int32</c> EMUs by a <c>double</c> and assigns straight back to a <c>sal_Int32</c>, so
/// every step truncates towards zero, and the truncation is visible: a four-node linear layout
/// across a 4 333 884 EMU frame gives children of 1 083 471 EMU each, which is 84 EMU — a
/// ten-thousandth of a point — short of a quarter. Doing the same arithmetic in doubles and
/// rounding at the end disagrees with the reference at the last decimal on nearly every shape,
/// for no gain.
/// </para>
/// </remarks>
internal static class PptxDiagramAlgorithms
{
    /// <summary>Sixtieths of a thousandth of a degree, DrawingML's rotation unit.</summary>
    private const int PerDegree = 60000;

    /// <summary>
    /// What a margin constraint's factor becomes before it multiplies a width in EMUs.
    /// </summary>
    /// <remarks>
    /// <c>convertPointToMms</c>, <c>include/oox/drawingml/drawingmltypes.hxx:199</c> — the
    /// conversion that makes a "fraction of the primary font size" into a fraction of the
    /// shape's width. See <see cref="Text"/> for why it is not a unit error.
    /// </remarks>
    private const double MillimetresPerPoint = 25.4 / 72.0;

    /// <summary>Runs one algorithm over one shape.</summary>
    public static void Apply(
        DiagramAlgorithmAtom atom,
        DiagramShape shape,
        IReadOnlyList<DiagramConstraint> constraints,
        IReadOnlyList<string> rules)
    {
        // Spacing shapes only survive into the linear algorithm, which is the only one that
        // knows how to give them room. Everywhere else an empty group is dead weight that would
        // take a share of the space and then draw nothing.
        if (atom.Type != "lin")
        {
            shape.Children.RemoveAll(child => child.IsGroup && child.Children.Count == 0);
        }

        switch (atom.Type)
        {
            case "composite":
                Composite(atom, shape, constraints);
                break;
            case "lin":
                Linear(atom, shape, constraints, rules);
                break;
            case "conn":
                Connector(atom, shape);
                break;
            case "snake":
                PptxDiagramGeometry.Snake(atom, shape, constraints);
                break;
            case "cycle":
                PptxDiagramGeometry.Cycle(atom, shape);
                break;
            case "hierRoot":
            case "hierChild":
                PptxDiagramGeometry.Hierarchy(atom, shape);
                break;
            case "pyra":
                PptxDiagramGeometry.Pyramid(shape);
                break;
            case "tx":
                Text(atom, shape, constraints);
                break;
            case "sp":
                // "text should appear only in tx node; we're assigning it earlier, so let's
                // remove it here" — diagramlayoutatoms.cxx:1675.
                shape.Text.Clear();
                break;
        }
    }

    // ------------------------------------------------------------- composite

    /// <summary>
    /// Places each child exactly where the constraints say, in the parent's own coordinates.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>CompositeAlg::layoutShapeChildren</c>, <c>diagramlayoutatoms.cxx:550</c>. The shared
    /// state is a map from layout-node name to a set of properties — left, top, width, height,
    /// centre — seeded with the parent's own under the empty name, so a constraint that reads
    /// <c>refType="w"</c> with no <c>refForName</c> reads the parent's width.
    /// </para>
    /// <para>
    /// <strong>Order of application matters and cannot be made not to.</strong> Constraints
    /// refer to each other, so applying A before B leaves A stale when B changes what it read.
    /// LibreOffice says so in a comment and then relies on document order anyway; this does the
    /// same, because any other order produces different numbers and the file was written against
    /// that one.
    /// </para>
    /// </remarks>
    private static void Composite(
        DiagramAlgorithmAtom atom, DiagramShape shape, IReadOnlyList<DiagramConstraint> constraints)
    {
        Dictionary<string, Dictionary<string, int>> properties = new(StringComparer.Ordinal);
        Dictionary<string, int> parent = Properties(properties, "");

        int parentXOffset = 0;

        if (atom.AspectRatio != 1.0)
        {
            parent["w"] = shape.Width;
            parent["h"] = shape.Height;
            parent["l"] = 0;
            parent["t"] = 0;
            parent["r"] = shape.Width;
            parent["b"] = shape.Height;
        }
        else
        {
            // A square aspect ratio is honoured by narrowing the box and centring it, not by
            // stretching what goes in it.
            parent["w"] = Math.Min(shape.Width, shape.Height);
            parent["h"] = shape.Height;
            if (parent["w"] < shape.Width) parentXOffset = (shape.Width - parent["w"]) / 2;
            parent["l"] = parentXOffset;
            parent["t"] = 0;
            parent["r"] = shape.Width - parent["l"];
            parent["b"] = shape.Height;
        }

        foreach (DiagramConstraint constraint in constraints) ApplyToLayout(constraint, properties);

        int verticalMinimum = int.MaxValue;
        int verticalMaximum = 0;

        foreach (DiagramShape child in shape.Children)
        {
            foreach (DiagramConstraint constraint in constraints)
            {
                if (constraint.ForName != child.InternalName) continue;

                ApplyToLayout(constraint, properties);
            }

            ApplyOwnConstraints(atom, child, properties);

            int width = shape.Width;
            int height = shape.Height;
            int x = 0;
            int y = 0;

            if (properties.TryGetValue(child.InternalName, out Dictionary<string, int>? own))
            {
                if (own.TryGetValue("w", out int stated)) width = Math.Min(stated, shape.Width);
                if (own.TryGetValue("h", out stated)) height = Math.Min(stated, shape.Height);

                if (own.TryGetValue("l", out stated)) x = stated;
                else if (own.TryGetValue("ctrX", out stated)) x = stated - width / 2;
                else if (own.TryGetValue("r", out stated)) x = stated - width;

                if (own.TryGetValue("t", out stated)) y = stated;
                else if (own.TryGetValue("ctrY", out stated)) y = stated - height / 2;
                else if (own.TryGetValue("b", out stated)) y = stated - height;

                if (own.TryGetValue("l", out int left) && own.TryGetValue("r", out int right))
                {
                    width = right - left;
                }

                if (own.TryGetValue("t", out int top) && own.TryGetValue("b", out int bottom))
                {
                    height = bottom - top;
                }

                x += parentXOffset;
                width = Math.Min(width, shape.Width - x);
                height = Math.Min(height, shape.Height - y);
            }

            child.X = x;
            child.Y = y;
            child.Width = width;
            child.Height = height;

            verticalMinimum = Math.Min(y, verticalMinimum);
            verticalMaximum = Math.Max(y + height, verticalMaximum);
        }

        // Whatever vertical space the constraints left over is split above and below, so a
        // composite whose children do not fill it comes out centred rather than top-aligned.
        if (verticalMinimum < 0 || verticalMinimum > verticalMaximum) return;
        if (verticalMaximum > parent["h"]) return;

        int slack = parent["h"] - (verticalMaximum - verticalMinimum);
        if (slack <= 0) return;

        foreach (DiagramShape child in shape.Children) child.Y += slack / 2;
    }

    /// <summary>
    /// Applies the constraints a child's <em>own</em> layout node states about itself.
    /// </summary>
    /// <remarks>
    /// A layout node can state its own width as a fraction of its own — <c>forName</c> and
    /// <c>refForName</c> both absent — and mean "of whatever my parent gave me". Those are
    /// rewritten to name the child and applied here, on top of the parent's state and of every
    /// earlier sibling's, which is what makes the result depend on sibling order.
    /// </remarks>
    private static void ApplyOwnConstraints(
        DiagramAlgorithmAtom atom,
        DiagramShape child,
        Dictionary<string, Dictionary<string, int>> properties)
    {
        if (atom.Owner is not { } owner) return;

        foreach (DiagramAtom candidate in owner.Children)
        {
            if (candidate is not DiagramLayoutNodeAtom node) continue;
            if (node.Name != child.InternalName) continue;

            foreach (DiagramAtom inner in node.Children)
            {
                if (inner is not DiagramConstraintAtom holder) continue;

                DiagramConstraint constraint = holder.Constraint;
                if (constraint.ForName.Length != 0) continue;
                if (constraint.RefForName.Length != 0) continue;
                if (constraint.Value == 0.0 && constraint.RefType == "none") continue;

                ApplyToLayout(
                    constraint with { ForName = node.Name, RefForName = node.Name }, properties);
            }
        }
    }

    private static void ApplyToLayout(
        DiagramConstraint constraint, Dictionary<string, Dictionary<string, int>> properties)
    {
        if (constraint.ForName.Length == 0) return;
        if (!properties.TryGetValue(constraint.RefForName, out Dictionary<string, int>? reference))
        {
            return;
        }

        if (reference.TryGetValue(constraint.RefType, out int value))
        {
            Properties(properties, constraint.ForName)[constraint.Type] =
                (int)(value * constraint.Factor);
            return;
        }

        if (Infer(reference, constraint.RefType, out int inferred))
        {
            Properties(properties, constraint.ForName)[constraint.Type] =
                (int)(inferred * constraint.Factor);
            return;
        }

        // No reference to read: the constraint states an absolute value, and it is never in
        // EMUs. A length is millimetres and a font size is points, and getting the two the wrong
        // way round scales a shape by 25.4/72.
        double emus = IsFontUnit(constraint.RefType)
            ? Length.FromPoints(constraint.Value).Emu
            : Length.FromMillimetres(constraint.Value).Emu;

        Properties(properties, constraint.ForName)[constraint.Type] = (int)emus;
    }

    /// <summary>
    /// Derives a property the map does not hold directly from ones it does.
    /// </summary>
    /// <remarks>
    /// Only the right edge, and only from a left edge and a width —
    /// <c>CompositeAlg::inferFromLayoutProperty</c>, <c>diagramlayoutatoms.cxx:482</c>. It exists
    /// because a layout definition may set <c>l</c> and <c>w</c> on one node and then read
    /// <c>r</c> off it from the next; without the inference the second node falls back to the
    /// absolute-value branch and lands at the origin.
    /// </remarks>
    private static bool Infer(Dictionary<string, int> map, string refType, out int value)
    {
        value = 0;
        if (refType != "r") return false;
        if (!map.TryGetValue("l", out int left)) return false;
        if (!map.TryGetValue("w", out int width)) return false;

        value = left + width;
        return true;
    }

    private static bool IsFontUnit(string type) => type is "primFontSz" or "secFontSz";

    private static Dictionary<string, int> Properties(
        Dictionary<string, Dictionary<string, int>> properties, string name)
    {
        if (properties.TryGetValue(name, out Dictionary<string, int>? map)) return map;

        map = new Dictionary<string, int>(StringComparer.Ordinal);
        properties[name] = map;
        return map;
    }

    // ----------------------------------------------------------------- linear

    /// <summary>
    /// Spreads children evenly along one axis and stretches them across the other.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>AlgAtom::layoutShape</c> case <c>XML_lin</c>, <c>diagramlayoutatoms.cxx:1365</c>.
    /// Three things happen in order: the constraints say how wide each named child wants to be,
    /// the rules say which children may be shrunk if that adds up to more than the parent has,
    /// and everything left over is divided equally.
    /// </para>
    /// <para>
    /// <strong>Rules are what make spacing work.</strong> With no rules the empty spacing groups
    /// are thrown away and the real children divide the whole width — which is why a list with
    /// no rules has no gaps between its boxes. With rules the groups stay, count towards the
    /// divisor, and take their stated fraction, so the gaps are real shapes with real widths.
    /// </para>
    /// </remarks>
    private static void Linear(
        DiagramAlgorithmAtom atom,
        DiagramShape shape,
        IReadOnlyList<DiagramConstraint> constraints,
        IReadOnlyList<string> rules)
    {
        if (shape.Children.Count == 0 || shape.Width == 0 || shape.Height == 0) return;

        string direction = atom.Parameter("linDir", "fromL");
        int incrementX = direction == "fromL" ? 1 : direction == "fromR" ? -1 : 0;
        int incrementY = direction == "fromT" ? 1 : direction == "fromB" ? -1 : 0;

        double count = shape.Children.Count;

        int connectorAngle = direction switch
        {
            "fromR" => 180,
            "fromT" => 270,
            "fromB" => 90,
            _ => 0,
        };

        int spaceWidth = 0;
        int spaceHeight = 0;

        Dictionary<string, Dictionary<string, int>> properties = new(StringComparer.Ordinal);

        foreach (DiagramConstraint constraint in constraints)
        {
            if (constraint.ForName.Length == 0) continue;

            Dictionary<string, int> property = Properties(properties, constraint.ForName);

            if (constraint.Type == "w")
            {
                property["w"] = Math.Min((int)(shape.Width * constraint.Factor), shape.Width);
            }

            if (constraint.Type == "h")
            {
                property["h"] = Math.Min((int)(shape.Height * constraint.Factor), shape.Height);
            }

            // "TODO: get values from differently named constraints as well" — the three names
            // real layout definitions use for a gap.
            if (constraint.ForName is "sp" or "space" or "sibTrans")
            {
                if (constraint.Type == "w") spaceWidth = (int)(shape.Width * constraint.Factor);
                if (constraint.Type == "h") spaceHeight = (int)(shape.Height * constraint.Factor);
            }
        }

        HashSet<string> shrinkable = new(rules, StringComparer.Ordinal);

        // "TODO consider rules for vertical linear layout as well" — a vertical list divides
        // equally whatever its rules say.
        if (direction is "fromT" or "fromB") shrinkable.Clear();

        if (shrinkable.Count != 0)
        {
            HashSet<string> dependants = new(StringComparer.Ordinal);

            foreach (DiagramShape child in shape.Children)
            {
                if (shrinkable.Contains(child.InternalName)) continue;
                if (count <= 1.0) continue;

                count -= 1.0;

                bool isDependency = false;
                double factor = 0;

                foreach (DiagramConstraint constraint in constraints)
                {
                    if (constraint.ForName != child.InternalName) continue;

                    bool horizontal = direction is "fromL" or "fromR";
                    if (horizontal && constraint.Type != "w") continue;
                    if (horizontal && constraint.Type == "w") factor = constraint.Factor;
                    if (!horizontal && constraint.Type != "h") continue;
                    if (!horizontal && constraint.Type == "h") factor = constraint.Factor;

                    if (!shrinkable.Contains(constraint.RefForName)) continue;

                    // A child whose size is a fraction of a child that will shrink is itself
                    // part of the division, at that fraction.
                    count += constraint.Factor;
                    dependants.Add(child.InternalName);
                    isDependency = true;
                    break;
                }

                if (isDependency || !child.IsGroup) continue;

                bool scaleEmptySpacing = false;
                if (direction is "fromL" or "fromR")
                {
                    scaleEmptySpacing = Property(properties, child.InternalName, "w") > 0;
                }

                if (!scaleEmptySpacing && direction is "fromT" or "fromB")
                {
                    scaleEmptySpacing = Property(properties, child.InternalName, "h") > 0;
                }

                if (scaleEmptySpacing && child.Children.Count == 0)
                {
                    count += factor;
                    dependants.Add(child.InternalName);
                }
            }

            shrinkable.UnionWith(dependants);

            // Spacings are children in their own right here, so a stated gap would count twice.
            spaceWidth = 0;
            spaceHeight = 0;
        }
        else
        {
            shape.Children.RemoveAll(child => child.IsGroup && child.Children.Count == 0);
            count = shape.Children.Count;
        }

        if (shape.Children.Count == 0) return;

        int childWidth = shape.Width;
        int childHeight = shape.Height;
        if (direction is "fromL" or "fromR") childWidth = (int)(childWidth / count);
        else if (direction is "fromT" or "fromB") childHeight = (int)(childHeight / count);

        int x = incrementX == -1 ? shape.Width - childWidth : 0;
        int y = incrementY == -1 ? shape.Height - childHeight : 0;

        int totalWidth = 0;
        int totalHeight = 0;
        foreach (DiagramShape child in shape.Children)
        {
            totalWidth += Stated(properties, child.InternalName, "w") ?? childWidth;
            totalHeight += Stated(properties, child.InternalName, "h") ?? childHeight;
        }

        totalWidth += (int)((count - 1) * spaceWidth);
        totalHeight += (int)((count - 1) * spaceHeight);

        double widthScale = 1.0;
        double heightScale = 1.0;
        if (incrementX != 0 && totalWidth > shape.Width)
        {
            widthScale = (double)shape.Width / totalWidth;
        }

        if (incrementY != 0 && totalHeight > shape.Height)
        {
            heightScale = (double)shape.Height / totalHeight;
        }

        spaceWidth = (int)(spaceWidth * widthScale);
        spaceHeight = (int)(spaceHeight * heightScale);

        foreach (DiagramShape child in shape.Children)
        {
            int width = Stated(properties, child.InternalName, "w") ?? childWidth;
            int height = Stated(properties, child.InternalName, "h") ?? childHeight;

            if (shrinkable.Count == 0 || shrinkable.Contains(child.InternalName))
            {
                width = (int)(width * widthScale);
                height = (int)(height * heightScale);
            }

            child.Width = width;
            child.Height = height;

            // The other axis centres — "probably some parameter controls it".
            if (incrementX != 0) y = (shape.Height - height) / 2;
            if (incrementY != 0) x = (shape.Width - width) / 2;
            if (x < 0) x = 0;
            if (y < 0) y = 0;

            child.X = x;
            child.Y = y;

            x += incrementX * (width + spaceWidth);
            y += incrementY * (height + spaceHeight);

            // Connectors are laid out here rather than in the conn algorithm, which has no
            // reference to the shapes either side of it.
            if (child.PresetType == "conn") child.Rotation = connectorAngle * PerDegree;
        }

        // Newer shapes stack in front of older ones unless the layout node reversed the order.
        if (atom.EnclosingLayoutNode()?.ChildOrder == "t") shape.Children.Reverse();
    }

    private static int Property(
        Dictionary<string, Dictionary<string, int>> properties, string name, string key)
        => Stated(properties, name, key) ?? 0;

    private static int? Stated(
        Dictionary<string, Dictionary<string, int>> properties, string name, string key)
        => properties.TryGetValue(name, out Dictionary<string, int>? map)
           && map.TryGetValue(key, out int value)
            ? value
            : null;

    // -------------------------------------------------------------- connector

    /// <summary>
    /// Turns a <c>conn</c> placeholder into a real arrow and resizes it from its own constraints.
    /// </summary>
    /// <remarks>
    /// There is no <c>conn</c> preset in DrawingML, so the shape has to become something —
    /// which arrow depends on the <c>begSty</c> and <c>endSty</c> parameters, and a bent route
    /// becomes nothing at all because LibreOffice hides those ("connectors are hidden in org
    /// chart as they don't work anyway", <c>diagramlayoutatoms.cxx:1024</c>). The resize keeps
    /// the centre, so an arrow narrowed by a constraint stays between the two boxes it joins.
    /// </remarks>
    private static void Connector(DiagramAlgorithmAtom atom, DiagramShape shape)
    {
        if (shape.PresetType == "conn")
        {
            string route = atom.Parameter("connRout", "");
            string begin = atom.Parameter("begSty", "");
            string end = atom.Parameter("endSty", "");

            string substitute = route == "bend"
                ? ""
                : begin == "arr" && end == "arr"
                    ? "leftRightArrow"
                    : begin == "arr"
                        ? "leftArrow"
                        : "rightArrow";

            // The sub-type is this shape's; the geometry belongs to the template, so every other
            // clone of the same dgm:shape draws the arrow too. LibreOffice's shape copy shares
            // its custom-shape properties by reference and gets the same result by accident —
            // see DiagramGeometry, and smartart-cycle.pptx, where four of five connectors are
            // never reached by this algorithm and are drawn as arrows regardless.
            shape.PresetType = substitute;
            shape.Geometry.PresetType = substitute;
        }

        Dictionary<string, Dictionary<string, int>> properties = new(StringComparer.Ordinal);
        Dictionary<string, int> parent = Properties(properties, "");
        parent["w"] = shape.Width;
        parent["h"] = shape.Height;
        parent["l"] = 0;
        parent["t"] = 0;
        parent["r"] = shape.Width;
        parent["b"] = shape.Height;

        if (atom.Owner is { } owner)
        {
            foreach (DiagramAtom child in owner.Children)
            {
                if (child is not DiagramConstraintAtom holder) continue;

                DiagramConstraint constraint = holder.Constraint;
                if (constraint.Operator is not ("none" or "equ")) continue;
                if (constraint.Type == "none") continue;

                if (!properties.TryGetValue(
                        constraint.RefForName, out Dictionary<string, int>? reference))
                {
                    continue;
                }

                if (!reference.TryGetValue(constraint.RefType, out int value)) continue;

                Properties(properties, constraint.ForName)[constraint.Type] =
                    (int)(value * constraint.Factor);
            }
        }

        int width = parent["w"];
        int height = parent["h"];

        shape.X += (shape.Width - width) / 2;
        shape.Y += (shape.Height - height) / 2;
        shape.Width = width;
        shape.Height = height;
    }

    // ------------------------------------------------------------------- text

    /// <summary>
    /// Decides how the text inside a shape is set: its size, its margins, its anchor, its bullets.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>AlgAtom::layoutShape</c> case <c>XML_tx</c>, <c>diagramlayoutatoms.cxx:1680</c>. This
    /// algorithm moves nothing — the shape it applies to was already placed by the composite or
    /// linear algorithm above it — and only sets text properties.
    /// </para>
    /// <para>
    /// <strong>The margin conversion is not a unit error, though it reads as one.</strong> A
    /// margin constraint states a factor of <c>primFontSz</c>, and LibreOffice multiplies the
    /// <em>shape's width in EMUs</em> by that factor converted from points to millimetres —
    /// "PowerPoint takes size as points, but gives margin as MMs". On the corpus's commonest
    /// case, factor 0.3 on a 341 pt box, that is 36 pt of inset per side. Reading it as a
    /// straight fraction of the width instead gives 102 pt and the label lands somewhere else
    /// entirely.
    /// </para>
    /// </remarks>
    private static void Text(
        DiagramAlgorithmAtom atom, DiagramShape shape, IReadOnlyList<DiagramConstraint> constraints)
    {
        double fontSize = 0;
        int left = 0;
        int top = 0;
        int right = 0;
        int bottom = 0;
        bool anyInset = false;

        foreach (DiagramConstraint constraint in constraints)
        {
            if (constraint.RefType == "w" && constraint.ForName.Length == 0)
            {
                int inset = (int)(shape.Width * (constraint.Factor * MillimetresPerPoint));

                switch (constraint.Type)
                {
                    case "lMarg": left = inset; anyInset = true; break;
                    case "rMarg": right = inset; anyInset = true; break;
                    case "tMarg": top = inset; anyInset = true; break;
                    case "bMarg": bottom = inset; anyInset = true; break;
                }
            }

            if (constraint.Type == "primFontSz") fontSize = constraint.Value;
        }

        if (shape.Text.Count == 0) return;

        shape.FontSize = fontSize;
        shape.TextInsets = (left, top, right, bottom);
        shape.HasTextInsets = anyInset;

        shape.TextAnchor = atom.Parameter("txAnchorVert", "mid");

        // Levels are normalised so that the shallowest paragraph is level zero, whatever depth
        // it happened to sit at in the author's tree.
        int baseLevel = int.MaxValue;
        foreach (DiagramTextParagraph paragraph in shape.Text)
        {
            baseLevel = Math.Min(baseLevel, Math.Max(paragraph.Level, 0));
        }

        for (int i = 0; i < shape.Text.Count; i++)
        {
            shape.Text[i] = shape.Text[i] with
            {
                Level = Math.Max(shape.Text[i].Level, 0) - baseLevel,
            };
        }

        // Bullets start at the second level by default, so a node's own line is unbulleted and
        // the lines under it are not.
        shape.BulletLevel = atom.Number("stBulletLvl", 2) - 1;

        bool bulleted = false;
        foreach (DiagramTextParagraph paragraph in shape.Text)
        {
            if (paragraph.Level >= shape.BulletLevel) bulleted = true;
        }

        // A list keeps whatever alignment its text states; anything else is centred.
        shape.TextAlignment = atom.Parameters.TryGetValue("parTxLTRAlign", out string? alignment)
            ? alignment
            : bulleted
                ? null
                : "ctr";
    }
}
