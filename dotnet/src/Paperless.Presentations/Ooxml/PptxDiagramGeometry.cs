namespace Paperless.Presentations.Ooxml;

/// <summary>
/// The four SmartArt layout algorithms that place their children by geometry rather than by
/// constraint — <c>snake</c>, <c>cycle</c>, <c>hierRoot</c>/<c>hierChild</c> and <c>pyra</c>.
/// </summary>
/// <remarks>
/// <para>
/// They are kept apart from <see cref="PptxDiagramAlgorithms"/> because they are a different kind
/// of thing. <c>composite</c> and <c>lin</c> read the layout definition's constraints and do what
/// they say; these four largely ignore them and lay their children out on a grid, a circle, a
/// tree or a stack using constants that are LibreOffice's own — a snake's rows are searched for
/// against an aspect ratio of <strong>0.54</strong>, a hierarchy's gaps are
/// <strong>0.1</strong> of a box across and <strong>0.3</strong> down, a pyramid's step is
/// <strong>0.32</strong>. None of those numbers is in the file. They are what LibreOffice
/// chose, so porting them is the only way the two renderings can agree, and inventing better
/// ones would guarantee they do not.
/// </para>
/// <para>
/// <strong>The arithmetic is integer, and the compound assignments are the trap.</strong>
/// LibreOffice holds a position in an <c>awt::Point</c>, whose fields are <c>sal_Int32</c>, and
/// writes <c>aCurrPos.X += nIncX * (aCurrSize.Width + fSpace * aCurrSize.Width)</c> — which in
/// C++ is <c>X = (sal_Int32)(X + …)</c>, truncating the <em>sum</em>. Writing the obvious
/// <c>x += (int)(…)</c> instead truncates the addend, and the two differ whenever the addend is
/// negative: a right-to-left snake stepping by −2.7 lands on 8 rather than 7, once per column,
/// and the error accumulates across the row. Every compound assignment below therefore casts the
/// whole sum.
/// </para>
/// <para>
/// Ported from <c>oox/source/drawingml/diagram/diagramlayoutatoms.cxx</c>:
/// <c>SnakeAlg::layoutShapeChildren</c> (line 110), <c>PyraAlg::layoutShapeChildren</c>
/// (line 428), and the <c>XML_cycle</c> and <c>XML_hierChild</c>/<c>XML_hierRoot</c> cases of
/// <c>AlgAtom::layoutShape</c> (lines 1193 and 1262).
/// </para>
/// </remarks>
internal static class PptxDiagramGeometry
{
    /// <summary>Sixtieths of a thousandth of a degree, DrawingML's rotation unit.</summary>
    private const int PerDegree = 60000;

    // ------------------------------------------------------------------ snake

    /// <summary>
    /// Fills a rectangle with children row by row, wrapping at a column count it searches for.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>SnakeAlg::layoutShapeChildren</c>, <c>diagramlayoutatoms.cxx:110</c>. Nothing in the
    /// file says how many columns a snake has: the search starts at one row and adds rows until
    /// the resulting block is at least <see cref="SnakeAspectRatio"/> as tall as it is wide, and
    /// that ratio is a constant LibreOffice picked ("diagram should not spill outside, earlier it
    /// was 0.6"). It is the whole of the layout — get the grid wrong and every node after the
    /// first row is in the wrong place.
    /// </para>
    /// <para>
    /// <strong>Widths can come from constraints, but only through the point type.</strong> A
    /// constraint carrying a <c>ptType</c> is filed by type rather than by name, and a child's
    /// width is then looked up by <em>its</em> data-node type — which is how a list whose gaps
    /// are real <c>sibTrans</c> shapes gets narrow gaps and wide boxes. LibreOffice only trusts
    /// that when the second child really is a <c>sibTrans</c>, because otherwise the widths are
    /// a fraction of a row width that no shape corresponds to.
    /// </para>
    /// </remarks>
    public static void Snake(
        DiagramAlgorithmAtom atom,
        DiagramShape shape,
        IReadOnlyList<DiagramConstraint> constraints)
    {
        if (shape.Children.Count == 0 || shape.Width == 0 || shape.Height == 0) return;

        // A child that asks for an aspect ratio narrows the box it is laid out in rather than
        // being stretched to fit one.
        double childAspectRatio = shape.Children[0].AspectRatio;
        double shapeHeight = shape.Height;
        double shapeWidth = shape.Width;
        if (childAspectRatio != 0 && shapeHeight != 0 && childAspectRatio < shapeWidth / shapeHeight)
        {
            shapeWidth = shapeHeight * childAspectRatio;
        }

        double spaceFactor = 1.0;
        Dictionary<string, Dictionary<string, int>> byName = new(StringComparer.Ordinal);
        Dictionary<string, Dictionary<string, int>> byType = new(StringComparer.Ordinal);
        Dictionary<string, int> parent = Map(byName, "");
        parent["w"] = (int)shapeWidth;
        parent["h"] = (int)shapeHeight;

        foreach (DiagramConstraint constraint in constraints)
        {
            if (constraint.RefType is "w" or "h"
                && constraint.Type == "sp"
                && constraint.ForName.Length == 0)
            {
                spaceFactor = constraint.Factor;
            }

            if (!byName.TryGetValue(constraint.RefForName, out Dictionary<string, int>? reference))
            {
                continue;
            }

            if (!reference.TryGetValue(constraint.RefType, out int value)) continue;

            // A constraint that also states an absolute value is a bound rather than a scaling.
            if (constraint.Value != 0.0) continue;

            int scaled = (int)(value * constraint.Factor);

            if (constraint.PointType == "none")
            {
                Map(byName, constraint.ForName)[constraint.Type] = scaled;
            }
            else
            {
                Map(byType, constraint.PointType)[constraint.Type] = scaled;
            }
        }

        int[] widths = new int[shape.Children.Count];
        for (int i = 0; i < widths.Length; i++)
        {
            widths[i] = (int)shapeWidth;

            DiagramShape child = shape.Children[i];
            if (child.DataNodeType.Length == 0) continue;
            if (!byType.TryGetValue(child.DataNodeType, out Dictionary<string, int>? own)) continue;
            if (!own.TryGetValue("w", out int stated)) continue;

            widths[i] = stated;
        }

        bool spaceFromConstraints = spaceFactor != 1.0;

        string direction = atom.Parameter("grDir", "tL");
        int incrementX = direction is "tR" or "bR" ? -1 : 1;
        int incrementY = direction is "bL" or "bR" ? -1 : 1;
        bool horizontal = direction is "tL" or "tR";

        int count = shape.Children.Count;
        double space = spaceFromConstraints ? spaceFactor : DefaultSnakeSpace;

        (int Columns, int Rows, int MaximumRowWidth) grid =
            SnakeGrid(count, widths, childAspectRatio, shapeHeight);

        int columns = grid.Columns;
        int rows = grid.Rows;
        int maximumRowWidth = grid.MaximumRowWidth;

        int width = (int)(shape.Width / (columns + (columns - 1) * space));
        int childWidth = width;
        int childHeight = (int)(width * SnakeAspectRatio);

        if (columns == 1 && rows > 1)
        {
            // One column, so the height comes from the parent's height rather than from the
            // width. Space occurs between the children, and twice as much outside them on both
            // sides when the factor came from a constraint rather than from the default.
            int gaps = spaceFromConstraints ? 3 : -1;
            int height = (int)(shape.Height / (rows + (rows + gaps) * space));

            if (childAspectRatio > 1)
            {
                childWidth = Math.Min(shape.Width, (int)(height * childAspectRatio));
                childHeight = height;
            }

            horizontal = false;
        }

        int x = 0;
        int y = 0;
        if (incrementX == -1) x = shape.Width - childWidth;
        if (incrementY == -1)
        {
            y = shape.Height - childHeight;
        }
        else if (spaceFromConstraints && !horizontal)
        {
            y = (int)(childHeight * space * 2);
        }

        if (atom.Parameter("contDir", "sameDir") == "revDir")
        {
            SnakeReversing(shape, x, y, childWidth, childHeight, columns, rows, count, space, incrementX, incrementY);
            return;
        }

        SnakeSameDirection(
            shape, x, y, childWidth, childHeight, columns, rows, count, space,
            incrementX, incrementY, widths, maximumRowWidth, childAspectRatio, shapeHeight: shape.Height);
    }

    /// <summary>The row-to-width ratio the grid search stops at.</summary>
    private const double SnakeAspectRatio = 0.54;

    /// <summary>The gap between snake cells, as a fraction of a cell, when no constraint says.</summary>
    private const double DefaultSnakeSpace = 0.3;

    /// <summary>
    /// Searches for the grid a snake wraps on.
    /// </summary>
    /// <remarks>
    /// Rows are added one at a time until the block they make is at least
    /// <see cref="SnakeAspectRatio"/> as tall as the first row is wide. A child aspect ratio of
    /// N is a statement that at most N shapes fit across, so a snake with no more children than
    /// that skips the search and becomes a single column.
    /// </remarks>
    private static (int Columns, int Rows, int MaximumRowWidth) SnakeGrid(
        int count, int[] widths, double childAspectRatio, double shapeHeight)
    {
        if (count <= childAspectRatio) return (1, count, 0);

        int columns = 1;
        int maximumRowWidth = 0;

        for (int rows = 1; rows < count; rows++)
        {
            columns = (int)Math.Ceiling(count / (double)rows);

            int rowWidth = 0;
            for (int i = 0; i < columns && i < count; i++) rowWidth += widths[i];

            double totalHeight = shapeHeight * rows;
            if (rowWidth != 0 && totalHeight / rowWidth >= SnakeAspectRatio)
            {
                if (rowWidth > maximumRowWidth) maximumRowWidth = rowWidth;
                return (columns, rows, maximumRowWidth);
            }
        }

        return (columns, count, maximumRowWidth);
    }

    /// <summary>Lays a snake out with every row running the same way.</summary>
    private static void SnakeSameDirection(
        DiagramShape shape,
        int x,
        int y,
        int childWidth,
        int childHeight,
        int columns,
        int rows,
        int count,
        double space,
        int incrementX,
        int incrementY,
        int[] widths,
        int maximumRowWidth,
        double childAspectRatio,
        int shapeHeight)
    {
        // Only trustworthy when the gaps are real child shapes: the widths are a portion of a
        // row width, and without a shape per gap the portions do not add up to the row.
        bool widthsFromConstraints =
            count >= 2 && shape.Children[1].DataNodeType == "sibTrans";

        int startX = x;
        int columnIndex = 0;
        int index = 0;
        int rowHeight = 0;

        foreach (DiagramShape child in shape.Children)
        {
            child.X = x;
            child.Y = y;

            int currentWidth = childWidth;
            int currentHeight = childHeight;

            if (widthsFromConstraints && maximumRowWidth != 0)
            {
                double factor = (double)widths[index] / maximumRowWidth;
                currentWidth = (int)(shape.Width * factor);
            }

            if (childAspectRatio != 0)
            {
                currentHeight = (int)(currentWidth / childAspectRatio);

                // A child is not allowed to leave its parent.
                currentHeight = Math.Min(currentHeight, (int)(shapeHeight / (rows + (rows - 1) * space)));
            }

            if (currentHeight > rowHeight) rowHeight = currentHeight;

            child.Width = currentWidth;
            child.Height = currentHeight;

            index++;

            if (index % columns == 0 || (index / columns) + 1 != rows)
            {
                x = (int)(x + (incrementX * (currentWidth + space * currentWidth)));
            }

            if (++columnIndex == columns)
            {
                // A last row that is not full is centred under the ones above it, unless the
                // widths came from constraints, in which case it starts where they do.
                if ((index + 1) % columns != 0
                    && index + 1 >= 3
                    && ((index + 1) / columns) + 1 == rows
                    && count != rows * columns)
                {
                    x = widthsFromConstraints
                        ? startX
                        : (int)(startX + (incrementX * (currentWidth + space * currentWidth) / 2));
                }
                else
                {
                    x = startX;
                }

                y = (int)(y + (incrementY * (rowHeight + space * rowHeight)));
                columnIndex = 0;
                rowHeight = 0;
            }

            if (index % columns != 0 && index >= 3 && (index / columns) + 1 == rows)
            {
                x = (int)(x + (incrementX * (currentWidth + space * currentWidth)));
            }
        }
    }

    /// <summary>
    /// Lays a snake out with alternate rows running backwards, which is what a snake is.
    /// </summary>
    /// <remarks>
    /// <strong>Transcribed and unmeasured.</strong> <c>contDir="revDir"</c> appears in no layout
    /// definition in LibreOffice's corpus — all nine snake layouts there state <c>sameDir</c> or
    /// nothing — so this is the one part of these four algorithms with no reference rendering
    /// behind it. It is <c>diagramlayoutatoms.cxx:388-425</c> line for line rather than a
    /// tidied version, precisely because nothing can yet tell a tidy from a wrong one.
    /// </remarks>
    private static void SnakeReversing(
        DiagramShape shape,
        int x,
        int y,
        int childWidth,
        int childHeight,
        int columns,
        int rows,
        int count,
        double space,
        int incrementX,
        int incrementY)
    {
        int startX = x;
        int columnIndex = 0;
        int index = 0;

        foreach (DiagramShape child in shape.Children)
        {
            child.X = x;
            child.Y = y;
            child.Width = childWidth;
            child.Height = childHeight;

            index++;

            bool odd = ((index / columns) + 1) % 2 != 0;

            if ((index % columns == 0 || (index / columns) + 1 != rows) && odd)
            {
                x = (int)(x + childWidth + (space * childWidth));
            }
            else if (index % columns != 0 && (index / columns) + 1 != rows)
            {
                x = (int)(x - childWidth - (space * childWidth));
            }

            if (++columnIndex == columns)
            {
                bool shortLastRow = (index + 1) % columns != 0
                                    && index + 1 >= 4
                                    && ((index + 1) / columns) + 1 == rows
                                    && count != rows * columns;

                if (shortLastRow && !odd) x -= childWidth * 3 / 2;
                else if (shortLastRow) x = (int)(startX + (incrementX * (childWidth + (space * childWidth)) / 2));
                else if (odd) x = startX;

                y = (int)(y + (incrementY * (childHeight + (space * childHeight))));
                columnIndex = 0;
            }

            if (index % columns != 0 && index >= 3 && (index / columns) + 1 == rows)
            {
                x = ((index / columns) + 1) % 2 == 0
                    ? (int)(x - (incrementX * (childWidth + (space * childWidth))))
                    : (int)(x + (incrementX * (childWidth + (space * childWidth))));
            }
        }
    }

    // ------------------------------------------------------------------ cycle

    /// <summary>
    /// Places children around a circle, and their connectors on a smaller one between them.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>AlgAtom::layoutShape</c> case <c>XML_cycle</c>, <c>diagramlayoutatoms.cxx:1193</c>. A
    /// cycle's child is a quarter of the frame each way and a connector a twelfth, whatever the
    /// constraints say, and the radius is what is left after half a child — so a cycle of two
    /// nodes and a cycle of eight draw the same size of box.
    /// </para>
    /// <para>
    /// <strong>The connector radius is smaller by an integer division.</strong>
    /// <c>nRadius * cos(spanAngle / shapes)</c> divides two <c>sal_Int32</c>s, so a 360° span
    /// over seven shapes is 51° rather than 51.43°, and the connectors sit a few EMUs further out
    /// than exact arithmetic would put them. Doing that division in doubles disagrees with the
    /// reference on every connector of every cycle.
    /// </para>
    /// <para>
    /// Connectors are turned here rather than in the <c>conn</c> algorithm for the same reason a
    /// linear layout turns them: <c>conn</c> has no reference to the shapes on either side of it.
    /// </para>
    /// </remarks>
    public static void Cycle(DiagramAlgorithmAtom atom, DiagramShape shape)
    {
        if (shape.Children.Count == 0) return;

        int startAngle = atom.Number("stAng", 0);
        int spanAngle = atom.Number("spanAng", 360);
        string rotationPath = atom.Parameter("rotPath", "none");
        string centreMap = atom.Parameter("ctrShpMap", "none");

        int centreX = shape.Width / 2;
        int centreY = shape.Height / 2;
        int childWidth = shape.Width / 4;
        int childHeight = shape.Height / 4;
        int connectorWidth = shape.Width / 12;
        int connectorHeight = shape.Height / 12;
        int radius = Math.Min((shape.Width - childWidth) / 2, (shape.Height - childHeight) / 2);

        List<DiagramShape> children = [.. shape.Children];

        if (centreMap == "fNode")
        {
            DiagramShape centre = children[0];
            children.RemoveAt(0);
            centre.X = centreX - (childWidth / 2);
            centre.Y = centreY - (childHeight / 2);
            centre.Width = childWidth;
            centre.Height = childHeight;
        }

        if (children.Count == 0) return;

        int connectorRadius = (int)(radius * Math.Cos(Radians(spanAngle / children.Count)));
        int connectorAngle = spanAngle > 0 ? 0 : 180;

        for (int index = 0; index < children.Count; index++)
        {
            DiagramShape child = children[index];
            double angle = ((double)index * spanAngle / children.Count) + startAngle;

            int currentWidth = childWidth;
            int currentHeight = childHeight;
            int currentRadius = radius;

            if (child.PresetType == "conn")
            {
                currentWidth = connectorWidth;
                currentHeight = connectorHeight;
                currentRadius = connectorRadius;
            }

            child.X = (int)(centreX + (currentRadius * Math.Sin(Radians(angle))) - (currentWidth / 2));
            child.Y = (int)(centreY - (currentRadius * Math.Cos(Radians(angle))) - (currentHeight / 2));
            child.Width = currentWidth;
            child.Height = currentHeight;

            if (rotationPath == "alongPath") child.Rotation = (int)(angle * PerDegree);

            if (child.PresetType == "conn")
            {
                child.Rotation = (int)((connectorAngle + angle) * PerDegree);
            }
        }
    }

    private static double Radians(double degrees) => degrees * Math.PI / 180.0;

    // -------------------------------------------------------------- hierarchy

    /// <summary>The gap between two boxes across, as a fraction of a box.</summary>
    private const double HierarchySpaceWidth = 0.1;

    /// <summary>The gap between two boxes down, as a fraction of a box.</summary>
    private const double HierarchySpaceHeight = 0.3;

    /// <summary>
    /// Stacks an organisation chart: <c>hierRoot</c> down the page, <c>hierChild</c> across it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>AlgAtom::layoutShape</c> cases <c>XML_hierChild</c> and <c>XML_hierRoot</c>,
    /// <c>diagramlayoutatoms.cxx:1262</c>. The pair alternates all the way down a tree:
    /// <c>hierRoot</c> is the manager-to-employees vertical path and <c>hierChild</c> the
    /// first-employee-to-last horizontal one, so a chart is a <c>hierRoot</c> of a box and a
    /// <c>hierChild</c>, and that <c>hierChild</c> holds one <c>hierRoot</c> per subordinate.
    /// </para>
    /// <para>
    /// <strong>A subtree's height is its leaf count, which is why the vertical shape count is
    /// computed during the creation walk.</strong> A branch three deep takes three rows of the
    /// height its parent hands it, and the only way to know that before laying it out is to have
    /// counted it while the tree was being built — <c>AlgAtom::getVerticalShapesCount</c>,
    /// which runs in <c>ShapeCreationVisitor</c> and not here.
    /// </para>
    /// <para>
    /// <strong>A connector takes a row of no width and no place in the order.</strong>
    /// LibreOffice gives it a width of one EMU and <em>continues</em> before advancing the
    /// cursor, so an org chart with an arrow between every pair of boxes lays its boxes out as
    /// though the arrows were not there. Advancing the cursor for them instead spreads a
    /// five-box chart over nine columns.
    /// </para>
    /// </remarks>
    public static void Hierarchy(DiagramAlgorithmAtom atom, DiagramShape shape)
    {
        if (shape.Children.Count == 0 || shape.Width == 0 || shape.Height == 0) return;

        bool isRoot = atom.Type == "hierRoot";
        string direction = isRoot ? "fromT" : atom.Parameter("linDir", "fromL");
        string secondary = atom.Parameter("secLinDir", "");

        int count = shape.Children.Count;

        if (!isRoot)
        {
            // Connectors must not influence the size of the shapes they connect.
            count = shape.Children.Count(child => child.PresetType != "conn");
        }

        if (isRoot && count == 3)
        {
            // An assistant hangs above the employees whatever order the data model states.
            if (!ContainsDataNodeType(shape.Children[1], "asst")
                && ContainsDataNodeType(shape.Children[2], "asst"))
            {
                (shape.Children[1], shape.Children[2]) = (shape.Children[2], shape.Children[1]);
            }
        }

        int horizontalShapes = 1;
        if (secondary == "fromT") horizontalShapes = 2;
        else if (direction is "fromL" or "fromR") horizontalShapes = count;

        int childWidth = (int)(shape.Width / (horizontalShapes + ((horizontalShapes - 1) * HierarchySpaceWidth)));
        int childHeight = (int)(shape.Height
                                / (shape.VerticalShapesCount
                                   + ((shape.VerticalShapesCount - 1) * HierarchySpaceHeight)));

        int connectorWidth = 1;
        int connectorHeight = childHeight;

        int x = 0;
        int y = 0;

        // Children are indented so that they read as descendants rather than as siblings.
        if (!isRoot && horizontalShapes == 1)
        {
            const double ChildIndent = 0.1;
            x = (int)(childWidth * ChildIndent);
            childWidth = (int)(childWidth * (1 - (2 * ChildIndent)));
        }

        int index = 0;
        int rowHeight = 0;

        foreach (DiagramShape child in shape.Children)
        {
            child.X = x;
            child.Y = y;

            if (!isRoot && child.PresetType == "conn")
            {
                child.Width = connectorWidth;
                child.Height = connectorHeight;
                continue;
            }

            int currentWidth = childWidth;
            int currentHeight = (int)(childHeight
                                      * (child.VerticalShapesCount
                                         + ((child.VerticalShapesCount - 1) * HierarchySpaceHeight)));

            child.Width = currentWidth;
            child.Height = currentHeight;

            if (direction is "fromT" or "fromB")
            {
                y = (int)(y + currentHeight + (childHeight * HierarchySpaceHeight));
            }
            else
            {
                x = (int)(x + currentWidth + (currentWidth * HierarchySpaceWidth));
            }

            rowHeight = Math.Max(rowHeight, currentHeight);

            if (secondary == "fromT" && index % 2 == 1)
            {
                x = 0;
                y = (int)(y + rowHeight + (childHeight * HierarchySpaceHeight));
                rowHeight = 0;
            }

            index++;
        }
    }

    /// <summary>True when a shape, or anything inside it, presents a data node of a given type.</summary>
    /// <remarks><c>containsDataNodeType</c>, <c>diagramlayoutatoms.cxx:96</c>.</remarks>
    private static bool ContainsDataNodeType(DiagramShape shape, string type)
    {
        if (shape.DataNodeType == type) return true;

        foreach (DiagramShape child in shape.Children)
        {
            if (ContainsDataNodeType(child, type)) return true;
        }

        return false;
    }

    // ---------------------------------------------------------------- pyramid

    /// <summary>
    /// Stacks children into a triangle, each one wider than the last.
    /// </summary>
    /// <remarks>
    /// <c>PyraAlg::layoutShapeChildren</c>, <c>diagramlayoutatoms.cxx:428</c>. Every child starts
    /// at a width and height of the frame divided by the number of children, and each one is then
    /// a child's height wider than the one above it while its left edge moves left by a fraction
    /// of that — which is what makes the stack a triangle rather than a column. The 0.32 is
    /// LibreOffice's, and the direction parameters (<c>linDir</c>, <c>pyraAcctPos</c>,
    /// <c>txDir</c>, <c>pyraLvlNode</c>) are read and commented out in the original, so an
    /// inverted pyramid draws the same way up in both.
    /// </remarks>
    public static void Pyramid(DiagramShape shape)
    {
        if (shape.Children.Count == 0 || shape.Width == 0 || shape.Height == 0) return;

        const double AspectRatio = 0.32;

        int count = shape.Children.Count;
        int childWidth = shape.Width / count;
        int childHeight = shape.Height / count;

        int x = (int)(AspectRatio * childWidth * (count - 1));
        int y = (int)(AspectRatio * childHeight);

        foreach (DiagramShape child in shape.Children)
        {
            child.X = x;
            child.Y = y;

            if (count > 1) x -= childHeight / (count - 1);

            childWidth += childHeight;
            child.Width = childWidth;
            child.Height = childHeight;

            y += childHeight;
        }
    }

    // ----------------------------------------------------------------- shared

    private static Dictionary<string, int> Map(
        Dictionary<string, Dictionary<string, int>> properties, string name)
    {
        if (properties.TryGetValue(name, out Dictionary<string, int>? map)) return map;

        map = new Dictionary<string, int>(StringComparer.Ordinal);
        properties[name] = map;
        return map;
    }
}
