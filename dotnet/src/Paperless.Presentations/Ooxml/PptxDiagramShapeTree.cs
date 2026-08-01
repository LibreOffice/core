using System.Globalization;
using System.Xml.Linq;
using Paperless.Ooxml;
using Paperless.Ooxml.DrawingML;

namespace Paperless.Presentations.Ooxml;

/// <summary>
/// Turns the evaluated diagram into the <c>p:spTree</c> the slide layouter already draws.
/// </summary>
/// <remarks>
/// <para>
/// This is deliberately the same target the baked-drawing path reaches by renaming
/// <c>dsp:</c> to <c>p:</c>: one flat list of <c>p:sp</c>, each with an <c>a:xfrm</c> in the
/// diagram's own coordinates, an <c>a:prstGeom</c>, a fill, a line and a <c>p:txBody</c>.
/// Everything downstream — 187 preset geometries, gradients, text layout, theme colours — is
/// then the code that draws an ordinary slide, and a diagram that draws wrongly is failing in
/// the evaluator rather than in a second renderer.
/// </para>
/// <para>
/// <strong>The tree is flattened.</strong> The evaluator's shapes nest, because the algorithms
/// divide a parent among its children, but the nesting carries no transform beyond a
/// translation — no child extent, no rotation on a group — so composing the offsets here and
/// emitting a flat list is exact and saves the layouter a group level per composite. It also
/// matches what a baked drawing is: 469 baked shapes across LibreOffice's corpus and not one
/// <c>dsp:grpSp</c> among them.
/// </para>
/// </remarks>
internal static class PptxDiagramShapeTree
{
    /// <summary>Sixtieths of a thousandth of a degree, DrawingML's rotation unit.</summary>
    private const int PerDegree = 60000;

    /// <summary>Builds the shape tree, or returns null when nothing would be drawn.</summary>
    /// <param name="diagram">The evaluated shape tree, in the frame's coordinates.</param>
    /// <param name="styles">The quick style and colour transform.</param>
    /// <param name="background">The data model's <c>dgm:bg</c> fill, drawn behind everything.</param>
    public static XElement? Build(
        DiagramShape diagram, PptxDiagramStyles styles, XElement? background)
    {
        XElement tree = new(Ppt.Name("spTree"));
        int id = 1;

        if (background is not null && background.Elements().Any())
        {
            tree.Add(Shape(
                new DiagramShape { PresetType = "rect", Width = diagram.Width, Height = diagram.Height },
                0,
                0,
                styles,
                background,
                ref id));
        }

        Emit(diagram, 0, 0, styles, tree, ref id);

        return tree.Elements().Any() ? tree : null;
    }

    private static void Emit(
        DiagramShape shape,
        int originX,
        int originY,
        PptxDiagramStyles styles,
        XElement tree,
        ref int id)
    {
        foreach (DiagramShape child in shape.Children)
        {
            int x = originX + child.X;
            int y = originY + child.Y;

            if (Drawn(child)) tree.Add(Shape(child, x, y, styles, null, ref id));

            Emit(child, x, y, styles, tree, ref id);
        }
    }

    /// <summary>
    /// Whether a shape contributes anything to the page.
    /// </summary>
    /// <remarks>
    /// A group draws nothing, and a shape whose geometry is hidden draws nothing unless it has
    /// text — which is exactly the case a diagram's text nodes are, so the second half of the
    /// test is what keeps the labels.
    /// </remarks>
    private static bool Drawn(DiagramShape shape)
    {
        if (shape.Width <= 0 || shape.Height <= 0) return false;
        if (shape.IsGroup) return false;

        return shape.DrawnPreset.Length != 0 || shape.Text.Count != 0;
    }

    private static XElement Shape(
        DiagramShape shape,
        int x,
        int y,
        PptxDiagramStyles styles,
        XElement? backgroundFill,
        ref int id)
    {
        XElement properties = new(Ppt.Name("spPr"), Transform(shape, x, y));

        if (shape.DrawnPreset is { Length: > 0 } preset)
        {
            properties.Add(new XElement(
                Drawing.Name("prstGeom"),
                new XAttribute("prst", preset),
                new XElement(Drawing.Name("avLst"))));
        }

        if (Fill(shape, styles, backgroundFill) is { } fill) properties.Add(fill);
        if (Line(shape, styles) is { } line) properties.Add(line);

        XElement result = new(
            Ppt.Name("sp"),
            new XElement(
                Ppt.Name("nvSpPr"),
                new XElement(
                    Ppt.Name("cNvPr"),
                    new XAttribute("id", id.ToString(CultureInfo.InvariantCulture)),
                    new XAttribute("name", shape.InternalName)),
                new XElement(Ppt.Name("cNvSpPr")),
                new XElement(Ppt.Name("nvPr"))),
            properties);

        id++;

        if (backgroundFill is null && styles.FontReference(shape.StyleLabel, shape.StyleIndex) is { } style)
        {
            result.Add(style);
        }

        if (shape.Text.Count != 0) result.Add(PptxDiagramText.Body(shape));

        return result;
    }

    private static XElement Transform(DiagramShape shape, int x, int y)
    {
        XElement transform = new(
            Drawing.Name("xfrm"),
            new XElement(
                Drawing.Name("off"),
                new XAttribute("x", x.ToString(CultureInfo.InvariantCulture)),
                new XAttribute("y", y.ToString(CultureInfo.InvariantCulture))),
            new XElement(
                Drawing.Name("ext"),
                new XAttribute("cx", shape.Width.ToString(CultureInfo.InvariantCulture)),
                new XAttribute("cy", shape.Height.ToString(CultureInfo.InvariantCulture))));

        if (shape.Rotation != 0)
        {
            transform.SetAttributeValue(
                "rot", shape.Rotation.ToString(CultureInfo.InvariantCulture));
        }

        // A half turn of the geometry inside an unchanged box is a flip in both directions; see
        // DiagramShape.DiagramRotation for why it cannot become a rotation of the box.
        if (Math.Abs(shape.DiagramRotation) % (360 * PerDegree) == 180 * PerDegree)
        {
            transform.SetAttributeValue("flipH", "1");
            transform.SetAttributeValue("flipV", "1");
        }

        return transform;
    }

    /// <summary>
    /// A node's fill: the data point's own, then the quick style's, then nothing.
    /// </summary>
    /// <remarks>
    /// The data point's wins because that is where a user's recolouring of one node lands — the
    /// colour transform describes the diagram's scheme and a <c>dgm:pt/dgm:spPr</c> describes a
    /// departure from it. LibreOffice orders them the same way, theme first and
    /// <c>getFillProperties()</c> assigned over it (<c>shape.cxx:2820-2834</c>).
    /// </remarks>
    private static XElement? Fill(
        DiagramShape shape, PptxDiagramStyles styles, XElement? backgroundFill)
    {
        XElement? source = backgroundFill ?? shape.ShapeProperties;

        foreach (string kind in FillKinds)
        {
            if (Drawing.Child(source, kind) is { } stated) return new XElement(stated);
        }

        if (shape.HideGeometry || shape.StyleLabel.Length == 0) return null;

        return styles.Fill(shape.StyleLabel, shape.StyleIndex);
    }

    private static readonly string[] FillKinds =
        ["noFill", "solidFill", "gradFill", "blipFill", "pattFill", "grpFill"];

    private static XElement? Line(DiagramShape shape, PptxDiagramStyles styles)
    {
        if (Drawing.Child(shape.ShapeProperties, "ln") is { } stated) return new XElement(stated);
        if (shape.HideGeometry || shape.StyleLabel.Length == 0) return null;

        return styles.Line(shape.StyleLabel, shape.StyleIndex);
    }
}
