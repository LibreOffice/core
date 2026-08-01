using System.Globalization;
using System.Xml.Linq;

namespace Paperless.Presentations.Ooxml;

/// <summary>
/// One <c>dgm:constr</c>: an equation between two properties of two named layout nodes.
/// </summary>
/// <remarks>
/// Read literally, a constraint says "<c>ForName</c>'s <c>Type</c> is <c>Factor</c> times
/// <c>RefForName</c>'s <c>RefType</c>", and with no reference it says "<c>ForName</c>'s
/// <c>Type</c> is <c>Value</c>". That is the whole of the sizing language: a diagram's boxes are
/// where they are because of a dozen of these, and nothing else states a coordinate.
/// </remarks>
internal sealed record DiagramConstraint
{
    /// <summary>Which relatives it applies to — <c>ch</c>, <c>des</c> or <c>self</c>.</summary>
    public string For { get; init; } = "none";

    /// <summary>The layout node it constrains.</summary>
    public string ForName { get; init; } = "";

    /// <summary>The data-point type it selects, when it selects by type rather than by name.</summary>
    public string PointType { get; init; } = "none";

    /// <summary>What is being set: <c>w</c>, <c>h</c>, <c>l</c>, <c>ctrX</c>, <c>sp</c>, …</summary>
    public string Type { get; init; } = "none";

    /// <summary>The layout node the reference reads from.</summary>
    public string RefForName { get; init; } = "";

    /// <summary>What is being read.</summary>
    public string RefType { get; init; } = "none";

    /// <summary>The multiplier on the reference, defaulting to one.</summary>
    public double Factor { get; init; } = 1.0;

    /// <summary>The absolute value, used when there is no reference.</summary>
    public double Value { get; init; }

    /// <summary><c>equ</c>, <c>lte</c>, <c>gte</c> — only equality is honoured.</summary>
    public string Operator { get; init; } = "none";
}

/// <summary>The <c>AG_IteratorAttributes</c> a <c>dgm:forEach</c> or a <c>dgm:if</c> carries.</summary>
internal sealed record DiagramIterator
{
    /// <summary>The axes to walk, in order — <c>ch</c>, <c>des</c>, <c>self</c>, <c>followSib</c>.</summary>
    public IReadOnlyList<string> Axis { get; init; } = [];

    /// <summary>How many to take, or −1 for all of them.</summary>
    public int Count { get; init; } = -1;

    /// <summary>Whether the final transition is suppressed, which is how a list loses its last gap.</summary>
    public bool HideLastTransition { get; init; } = true;

    /// <summary>Which point type to walk — <c>node</c>, <c>all</c>, <c>sibTrans</c>, …</summary>
    public string PointType { get; init; } = "all";

    /// <summary>Where to start.</summary>
    public int Start { get; init; }

    /// <summary>How far to advance each iteration.</summary>
    public int Step { get; init; } = 1;
}

/// <summary>A node of the layout-atom program the evaluator walks.</summary>
/// <remarks>
/// <para>
/// <c>layout1.xml</c> is not a description of a diagram; it is a small declarative program whose
/// statements are these atoms and whose input is the data model. LibreOffice models it as a tree
/// of <c>LayoutAtom</c> subclasses visited by three visitors
/// (<c>oox/source/drawingml/diagram/diagramlayoutatoms.hxx</c>), and this is that tree.
/// </para>
/// <para>
/// The one field every atom carries and few use is <see cref="Owner"/>: constraint atoms are
/// read out of the <em>layout node</em> that encloses them rather than through the visitor, so
/// each atom has to know which one that is. LibreOffice keeps the same back-reference, as
/// <c>LayoutAtom::mrLayoutNode</c>.
/// </para>
/// </remarks>
internal abstract class DiagramAtom
{
    /// <summary>The <c>name</c> attribute — how a <c>forEach ref=</c> finds an atom again.</summary>
    public string Name { get; set; } = "";

    /// <summary>The atoms nested inside this one, in document order.</summary>
    public List<DiagramAtom> Children { get; } = [];

    /// <summary>The atom this one is nested in, or null at the root.</summary>
    public DiagramAtom? Parent { get; set; }

    /// <summary>The nearest enclosing layout node, which owns this atom's constraints.</summary>
    public DiagramLayoutNodeAtom? Owner { get; set; }

    /// <summary>Adds a child and records the back-reference in one step.</summary>
    public void Add(DiagramAtom child)
    {
        child.Parent = this;
        Children.Add(child);
    }

    /// <summary>The nearest enclosing layout node, walking outwards.</summary>
    public DiagramLayoutNodeAtom? EnclosingLayoutNode()
    {
        for (DiagramAtom? atom = Parent; atom is not null; atom = atom.Parent)
        {
            if (atom is DiagramLayoutNodeAtom node) return node;
        }

        return null;
    }
}

/// <summary>A <c>dgm:layoutNode</c>: one shape's worth of the program.</summary>
internal sealed class DiagramLayoutNodeAtom : DiagramAtom
{
    /// <summary>The quick-style label, used when the presentation point states none.</summary>
    public string StyleLabel { get; set; } = "";

    /// <summary>
    /// <c>b</c> or <c>t</c> — whether later children stack behind earlier ones or in front.
    /// </summary>
    public string ChildOrder { get; set; } = "b";
}

/// <summary>A <c>dgm:alg</c>: which layout algorithm the enclosing node runs.</summary>
internal sealed class DiagramAlgorithmAtom : DiagramAtom
{
    /// <summary><c>lin</c>, <c>composite</c>, <c>sp</c>, <c>tx</c>, <c>conn</c>, …</summary>
    public string Type { get; set; } = "";

    /// <summary>The <c>dgm:param</c> list, by type.</summary>
    public Dictionary<string, string> Parameters { get; } = new(StringComparer.Ordinal);

    /// <summary>
    /// <c>param type="ar"</c>, kept apart from the rest because it is the only real-valued one.
    /// </summary>
    public double AspectRatio { get; set; }

    /// <summary>A parameter's value, or a default.</summary>
    public string Parameter(string type, string whenAbsent)
        => Parameters.TryGetValue(type, out string? value) ? value : whenAbsent;

    /// <summary>A parameter read as a number, or a default.</summary>
    public int Number(string type, int whenAbsent)
        => Parameters.TryGetValue(type, out string? value)
           && int.TryParse(value, NumberStyles.Integer, CultureInfo.InvariantCulture, out int number)
            ? number
            : whenAbsent;
}

/// <summary>A <c>dgm:constr</c>.</summary>
internal sealed class DiagramConstraintAtom : DiagramAtom
{
    /// <summary>The equation this atom states.</summary>
    public DiagramConstraint Constraint { get; set; } = new();
}

/// <summary>A <c>dgm:rule</c>: what may be shrunk when the constraints do not fit.</summary>
internal sealed class DiagramRuleAtom : DiagramAtom
{
    /// <summary>The layout node the rule allows to change.</summary>
    public string ForName { get; set; } = "";
}

/// <summary>A <c>dgm:forEach</c>.</summary>
internal sealed class DiagramForEachAtom : DiagramAtom
{
    /// <summary>What to iterate over.</summary>
    public DiagramIterator Iterator { get; set; } = new();

    /// <summary>The name of another <c>forEach</c> to run instead of this one's body.</summary>
    public string Reference { get; set; } = "";
}

/// <summary>A <c>dgm:choose</c>.</summary>
internal sealed class DiagramChooseAtom : DiagramAtom;

/// <summary>A <c>dgm:if</c> or a <c>dgm:else</c>.</summary>
internal sealed class DiagramConditionAtom : DiagramAtom
{
    /// <summary>True for <c>dgm:else</c>, which always matches.</summary>
    public bool IsElse { get; set; }

    /// <summary>The iterator half of the attributes, which <c>cnt</c> reads.</summary>
    public DiagramIterator Iterator { get; set; } = new();

    /// <summary>The test — <c>var</c>, <c>cnt</c>, <c>maxDepth</c>, …</summary>
    public string Function { get; set; } = "";

    /// <summary>Which variable, for <c>func="var"</c>.</summary>
    public string Argument { get; set; } = "none";

    /// <summary>The comparison — <c>equ</c>, <c>gt</c>, <c>lte</c>, …</summary>
    public string Operator { get; set; } = "";

    /// <summary>The value compared against, as written.</summary>
    public string Value { get; set; } = "";
}

/// <summary>A <c>dgm:shape</c>: the template a layout node's shape is copied from.</summary>
internal sealed class DiagramShapeAtom : DiagramAtom
{
    /// <summary>
    /// True when the element states a <c>type</c> at all, which is what makes it a shape rather
    /// than a group.
    /// </summary>
    /// <remarks>
    /// The distinction is not the same as having a drawn geometry, and conflating the two costs
    /// a diagram its labels: a <c>type="rect" hideGeom="1"</c> shape is a real shape that draws
    /// no outline and holds the text, and treating it as a group deletes it as empty spacing.
    /// LibreOffice makes the split on the attribute's presence alone
    /// (<c>layoutnodecontext.cxx:230</c>).
    /// </remarks>
    public bool HasType { get; set; }

    /// <summary>The preset geometry as stated, whether or not it is drawn.</summary>
    public string PresetType { get; set; } = "";

    /// <summary>
    /// The drawn geometry every shape cloned from this template shares.
    /// </summary>
    /// <remarks>
    /// One object per <c>dgm:shape</c> element, handed to each clone rather than copied — see
    /// <see cref="DiagramGeometry"/> for why the sharing is the port rather than an oversight.
    /// </remarks>
    public DiagramGeometry Geometry { get; } = new();

    /// <summary>True when the element states <c>hideGeom="1"</c>.</summary>
    public bool HideGeometry { get; set; }

    /// <summary>The <c>rot</c> attribute, in whole degrees as the attribute states it.</summary>
    public int Rotation { get; set; }

    /// <summary>How far this shape moves up the stacking order.</summary>
    public int ZOrderOffset { get; set; }

    /// <summary>The <c>dgm:adjLst</c>, which is DrawingML's <c>a:avLst</c> under another name.</summary>
    public XElement? Adjustments { get; set; }
}

/// <summary>
/// Reads <c>layout1.xml</c> into the atom tree.
/// </summary>
/// <remarks>
/// A straight port of <c>LayoutNodeContext::onCreateContext</c>,
/// <c>oox/source/drawingml/diagram/layoutnodecontext.cxx:194</c>. Two things it does
/// <em>not</em> read are worth naming: <c>dgm:presOf</c>, because the presentation-of mapping
/// comes from the data model's connections and the layout definition's copy of it is advisory
/// (LibreOffice reads and discards it at line 279); and <c>dgm:sampData</c>, <c>dgm:styleData</c>
/// and <c>dgm:clrData</c>, which are the thumbnails the gallery shows and contain a complete
/// second data model that would otherwise be mistaken for the real one.
/// </remarks>
internal static class PptxDiagramLayout
{
    /// <summary>Parses a <c>dgm:layoutDef</c>, returning its root layout node.</summary>
    public static DiagramLayoutNodeAtom? Read(XElement? definition)
    {
        XElement? root = definition?.Element(XName.Get("layoutNode", PptxDiagram.Uri));
        if (root is null) return null;

        DiagramLayoutNodeAtom node = new()
        {
            Name = root.Attribute("name")?.Value ?? "",
            StyleLabel = root.Attribute("styleLbl")?.Value ?? "",
            ChildOrder = root.Attribute("chOrder")?.Value ?? "b",
        };

        node.Owner = node;
        ReadChildren(root, node, node);
        return node;
    }

    private static void ReadChildren(XElement element, DiagramAtom atom, DiagramLayoutNodeAtom owner)
    {
        foreach (XElement child in element.Elements())
        {
            if (child.Name.NamespaceName != PptxDiagram.Uri) continue;

            switch (child.Name.LocalName)
            {
                case "layoutNode":
                {
                    DiagramLayoutNodeAtom node = new()
                    {
                        Name = child.Attribute("name")?.Value ?? "",
                        StyleLabel = child.Attribute("styleLbl")?.Value ?? "",
                        // An unstated child order is inherited rather than defaulted, so a
                        // reversed branch stays reversed all the way down.
                        ChildOrder = child.Attribute("chOrder")?.Value ?? owner.ChildOrder,
                    };

                    node.Owner = node;
                    atom.Add(node);
                    ReadChildren(child, node, node);
                    break;
                }

                case "alg":
                {
                    DiagramAlgorithmAtom algorithm = new()
                    {
                        Type = child.Attribute("type")?.Value ?? "",
                    };

                    foreach (XElement parameter in child.Elements(XName.Get("param", PptxDiagram.Uri)))
                    {
                        string type = parameter.Attribute("type")?.Value ?? "";
                        string value = parameter.Attribute("val")?.Value ?? "";

                        if (type == "ar")
                        {
                            algorithm.AspectRatio = Number(value);
                        }
                        else if (type.Length != 0)
                        {
                            algorithm.Parameters[type] = value;
                        }
                    }

                    algorithm.Owner = owner;
                    atom.Add(algorithm);
                    break;
                }

                case "shape":
                {
                    string preset = child.Attribute("type")?.Value ?? "";
                    bool hidden = child.Attribute("hideGeom")?.Value is "1" or "true";

                    DiagramShapeAtom shape = new()
                    {
                        HasType = preset.Length != 0,
                        PresetType = preset,
                        Geometry = { PresetType = preset },
                        HideGeometry = hidden,
                        Rotation = (int)Number(child.Attribute("rot")?.Value),
                        ZOrderOffset = (int)Number(child.Attribute("zOrderOff")?.Value),
                        Adjustments = child.Element(XName.Get("adjLst", PptxDiagram.Uri)),
                        Owner = owner,
                    };

                    atom.Add(shape);
                    break;
                }

                case "choose":
                {
                    DiagramChooseAtom choose = new()
                    {
                        Name = child.Attribute("name")?.Value ?? "",
                        Owner = owner,
                    };

                    atom.Add(choose);
                    ReadChildren(child, choose, owner);
                    break;
                }

                case "if":
                case "else":
                {
                    DiagramConditionAtom condition = new()
                    {
                        Name = child.Attribute("name")?.Value ?? "",
                        IsElse = child.Name.LocalName == "else",
                        Iterator = ReadIterator(child),
                        Function = child.Attribute("func")?.Value ?? "",
                        Argument = child.Attribute("arg")?.Value ?? "none",
                        Operator = child.Attribute("op")?.Value ?? "",
                        Value = child.Attribute("val")?.Value ?? "",
                        Owner = owner,
                    };

                    atom.Add(condition);
                    ReadChildren(child, condition, owner);
                    break;
                }

                case "forEach":
                {
                    DiagramForEachAtom loop = new()
                    {
                        Name = child.Attribute("name")?.Value ?? "",
                        Iterator = ReadIterator(child),
                        Reference = child.Attribute("ref")?.Value ?? "",
                        Owner = owner,
                    };

                    atom.Add(loop);
                    ReadChildren(child, loop, owner);
                    break;
                }

                case "constrLst":
                {
                    foreach (XElement constraint in child.Elements(XName.Get("constr", PptxDiagram.Uri)))
                    {
                        atom.Add(new DiagramConstraintAtom
                        {
                            Owner = owner,
                            Constraint = new DiagramConstraint
                            {
                                For = constraint.Attribute("for")?.Value ?? "none",
                                ForName = constraint.Attribute("forName")?.Value ?? "",
                                PointType = constraint.Attribute("ptType")?.Value ?? "none",
                                Type = constraint.Attribute("type")?.Value ?? "none",
                                RefForName = constraint.Attribute("refForName")?.Value ?? "",
                                RefType = constraint.Attribute("refType")?.Value ?? "none",
                                Factor = Number(constraint.Attribute("fact")?.Value, 1.0),
                                Value = Number(constraint.Attribute("val")?.Value),
                                Operator = constraint.Attribute("op")?.Value ?? "none",
                            },
                        });
                    }

                    break;
                }

                case "ruleLst":
                {
                    foreach (XElement rule in child.Elements(XName.Get("rule", PptxDiagram.Uri)))
                    {
                        atom.Add(new DiagramRuleAtom
                        {
                            Owner = owner,
                            ForName = rule.Attribute("forName")?.Value ?? "",
                        });
                    }

                    break;
                }
            }
        }
    }

    private static DiagramIterator ReadIterator(XElement element)
    {
        string[] axis = (element.Attribute("axis")?.Value ?? "")
            .Split(' ', StringSplitOptions.RemoveEmptyEntries);

        // "better to keep first token instead of error when multiple values" —
        // IteratorAttr::loadFromXAttr, diagramlayoutatoms.cxx:749.
        string[] pointTypes = (element.Attribute("ptType")?.Value ?? "")
            .Split(' ', StringSplitOptions.RemoveEmptyEntries);

        return new DiagramIterator
        {
            Axis = axis,
            Count = (int)Number(element.Attribute("cnt")?.Value, -1),
            HideLastTransition = element.Attribute("hideLastTrans")?.Value is not ("0" or "false"),
            PointType = pointTypes.Length == 0 ? "all" : pointTypes[0],
            Start = (int)Number(element.Attribute("st")?.Value),
            Step = (int)Number(element.Attribute("step")?.Value, 1),
        };
    }

    private static double Number(string? text, double whenAbsent = 0)
        => text is not null
           && double.TryParse(text, NumberStyles.Float, CultureInfo.InvariantCulture, out double value)
            ? value
            : whenAbsent;
}
