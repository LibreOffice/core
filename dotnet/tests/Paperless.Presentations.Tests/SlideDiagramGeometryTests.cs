using Paperless.Presentations.Ooxml;
using Shouldly;

namespace Paperless.Presentations.Tests;

/// <summary>
/// The four SmartArt algorithms that place children by geometry rather than by constraint.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Why these are pinned as arithmetic rather than as rendered decks.</strong> Every one of
/// <c>snake</c>, <c>cycle</c>, <c>hierRoot</c>/<c>hierChild</c> and <c>pyra</c> lays its children
/// out on constants that are LibreOffice's own choices — a grid searched against an aspect ratio
/// of 0.54, a hierarchy gap of 0.1 across and 0.3 down, a pyramid step of 0.32 — and the numbers
/// only agree with the reference because the integer truncation is reproduced step for step. That
/// is what regresses, and it regresses silently, so it is what these assert.
/// </para>
/// <para>
/// The correctness check is elsewhere and is a measurement: over the 37 decks in LibreOffice's
/// <c>sd/qa/unit/data/pptx</c> that carry a diagram with no baked drawing, every filled path's
/// bounding box agrees with LibreOffice's own rendering to within 0.080 pt — three roundings of
/// its internal hundredth of a millimetre. See <c>src/Paperless.Presentations/TODO.md</c>.
/// </para>
/// </remarks>
public class SlideDiagramGeometryTests
{
    /// <summary>
    /// A snake with no constraints wraps onto the grid the aspect-ratio search finds.
    /// </summary>
    /// <remarks>
    /// Four children across a 4000 EMU square: one row is four boxes wide and 4000/16000 of their
    /// width tall, which is under the 0.54 the search wants, so it takes two rows of two. The
    /// cell is then 4000/(2 + 0.3) = 1739 wide and 0.54 of that tall, and the step is a cell plus
    /// three tenths of one — 2260, which is 2260.7 truncated rather than rounded.
    /// </remarks>
    [Fact]
    public void SnakeWrapsOntoTheGridItSearchesFor()
    {
        DiagramShape frame = Frame(4000, 4000, Children(4));

        PptxDiagramGeometry.Snake(Algorithm("snake"), frame, []);

        frame.Children.Select(child => (child.X, child.Y, child.Width, child.Height))
            .ShouldBe([(0, 0, 1739, 939), (2260, 0, 1739, 939),
                       (0, 1220, 1739, 939), (2260, 1220, 1739, 939)]);
    }

    /// <summary>
    /// A snake growing right to left starts at the far edge and steps back towards the origin.
    /// </summary>
    /// <remarks>
    /// The <c>grDir</c> parameter is the whole of it, and it is where the compound assignment
    /// matters: LibreOffice writes <c>aCurrPos.X += nIncX * (…)</c> on a <c>sal_Int32</c>, which
    /// truncates the <em>sum</em>. Truncating the addend instead lands one EMU further right on
    /// every negative step, which is invisible on one shape and accumulates across a row.
    /// </remarks>
    [Fact]
    public void SnakeGrowingRightToLeftStartsAtTheFarEdge()
    {
        DiagramShape frame = Frame(4000, 4000, Children(4));

        PptxDiagramGeometry.Snake(Algorithm("snake", ("grDir", "tR")), frame, []);

        frame.Children[0].X.ShouldBe(4000 - 1739);
        frame.Children[1].X.ShouldBe(4000 - 1739 - 2261);
        frame.Children[2].Y.ShouldBe(1220);
    }

    /// <summary>
    /// A cycle puts its children on a circle a quarter of the frame in from the edge.
    /// </summary>
    /// <remarks>
    /// A child is a quarter of the frame each way whatever the constraints say, and the radius is
    /// what is left after half a child — so a cycle of three nodes and a cycle of eight draw the
    /// same size of box, and only the angle between them changes. The first child is at the top
    /// because <c>stAng</c> defaults to zero and the angle is measured from twelve o'clock.
    /// </remarks>
    [Fact]
    public void CyclePlacesChildrenOnACircle()
    {
        DiagramShape frame = Frame(1200, 1200, Children(4));

        PptxDiagramGeometry.Cycle(Algorithm("cycle"), frame);

        foreach (DiagramShape child in frame.Children)
        {
            child.Width.ShouldBe(300);
            child.Height.ShouldBe(300);
        }

        // Twelve o'clock, three o'clock, six and nine, on a radius of (1200 - 300) / 2.
        frame.Children[0].X.ShouldBe(450);
        frame.Children[0].Y.ShouldBe(0);
        frame.Children[1].X.ShouldBe(900);
        frame.Children[2].Y.ShouldBe(900);
        frame.Children[3].X.ShouldBe(0);
    }

    /// <summary>
    /// A cycle asked to put its first node in the middle does, and circles the rest.
    /// </summary>
    /// <remarks>
    /// <c>ctrShpMap="fNode"</c>. The centred node is taken out of the ring before the angles are
    /// divided, so three nodes around a hub are 120° apart rather than 90°.
    /// </remarks>
    [Fact]
    public void CycleCentresTheFirstNodeWhenAskedTo()
    {
        DiagramShape frame = Frame(1200, 1200, Children(4));

        PptxDiagramGeometry.Cycle(Algorithm("cycle", ("ctrShpMap", "fNode")), frame);

        frame.Children[0].X.ShouldBe(450);
        frame.Children[0].Y.ShouldBe(450);
        frame.Children[1].X.ShouldBe(450);
        frame.Children[1].Y.ShouldBe(0);
    }

    /// <summary>
    /// A hierarchy root stacks its children down the page and leaves three tenths between them.
    /// </summary>
    /// <remarks>
    /// The height each child gets is the frame divided by the <em>vertical shape count</em> the
    /// creation walk computed, plus a gap of 0.3 of a row for each gap — so a two-row root gives
    /// each child 1000/2.3 and steps by that plus 0.3 of it. A branch that is itself two rows
    /// deep then takes twice the height plus its own internal gap, which is the second assertion.
    /// </remarks>
    [Fact]
    public void HierarchyRootStacksItsChildrenDownThePage()
    {
        DiagramShape frame = Frame(1000, 1000, Children(2));
        frame.VerticalShapesCount = 2;

        PptxDiagramGeometry.Hierarchy(Algorithm("hierRoot"), frame);

        frame.Children.Select(child => (child.X, child.Y, child.Width, child.Height))
            .ShouldBe([(0, 0, 1000, 434), (0, 564, 1000, 434)]);
    }

    /// <summary>
    /// A hierarchy child spreads its children across, indenting when there is only one.
    /// </summary>
    /// <remarks>
    /// A single subordinate is indented a tenth of its width on each side so that it reads as a
    /// descendant rather than as a sibling of the box above it —
    /// <c>diagramlayoutatoms.cxx:1317</c>. Two or more are not indented at all; they are spread
    /// across instead, which is what makes them read as siblings.
    /// </remarks>
    [Fact]
    public void HierarchyChildIndentsItsOnlySubordinate()
    {
        DiagramShape frame = Frame(1000, 1000, Children(1));

        PptxDiagramGeometry.Hierarchy(Algorithm("hierChild"), frame);

        frame.Children[0].X.ShouldBe(100);
        frame.Children[0].Width.ShouldBe(800);
    }

    /// <summary>
    /// A connector in a hierarchy takes no width and no place in the order.
    /// </summary>
    /// <remarks>
    /// LibreOffice gives it a width of one EMU and continues before advancing the cursor, so the
    /// boxes are laid out as though the arrows between them were not there. Advancing for them
    /// too spreads a two-box branch over three columns and halves every box.
    /// </remarks>
    [Fact]
    public void AConnectorTakesNoRoomInAHierarchy()
    {
        List<DiagramShape> children = Children(3);
        children[1].PresetType = "conn";
        DiagramShape frame = Frame(1000, 1000, children);

        PptxDiagramGeometry.Hierarchy(Algorithm("hierChild"), frame);

        frame.Children[1].Width.ShouldBe(1);

        // Two real boxes, so each is 1000/2.1 across and the second starts a tenth further on.
        frame.Children[0].Width.ShouldBe(476);
        frame.Children[2].X.ShouldBe(523);
    }

    /// <summary>
    /// A pyramid widens each row by a whole row's height and steps its left edge back.
    /// </summary>
    /// <remarks>
    /// Every child starts at the frame divided by the number of children and then grows by a
    /// child's height per row, while the left edge moves left by that height divided by the gaps
    /// — which is what turns a column into a triangle. The last row is wider than the frame,
    /// deliberately: the 0.32 head start is what keeps the apex inside it.
    /// </remarks>
    [Fact]
    public void PyramidWidensEachRow()
    {
        DiagramShape frame = Frame(900, 900, Children(3));

        PptxDiagramGeometry.Pyramid(frame);

        frame.Children.Select(child => (child.X, child.Y, child.Width, child.Height))
            .ShouldBe([(192, 96, 600, 300), (42, 396, 900, 300), (-108, 696, 1200, 300)]);
    }

    // ----------------------------------------------------------------- set-up

    private static DiagramShape Frame(int width, int height, List<DiagramShape> children)
    {
        DiagramShape frame = new() { IsGroup = true, Width = width, Height = height };
        frame.Children.AddRange(children);
        return frame;
    }

    private static List<DiagramShape> Children(int count)
        => [.. Enumerable.Range(0, count).Select(
            index => new DiagramShape { InternalName = $"node{index}", PresetType = "rect" })];

    private static DiagramAlgorithmAtom Algorithm(
        string type, params (string Type, string Value)[] parameters)
    {
        DiagramAlgorithmAtom atom = new() { Type = type };
        foreach ((string key, string value) in parameters) atom.Parameters[key] = value;
        return atom;
    }
}
