using System.Globalization;
using System.Xml.Linq;

namespace Paperless.Presentations.Ooxml;

/// <summary>
/// Runs a diagram's layout-atom program over its data model, producing the shape tree the
/// authoring application would have baked.
/// </summary>
/// <remarks>
/// <para>
/// <strong>Why this exists at all.</strong> Every diagram a modern Office writes carries a
/// <c>diagramDrawing</c> part holding the result already laid out, and reading that is both
/// exact and cheap — so it is what <see cref="PptxDiagram.Baked"/> does. Office 2007 wrote none,
/// because the drawing vocabulary's namespace is dated 2008 and the files predate it, and
/// LibreOffice's own import fixtures have theirs removed by hand precisely so that this path is
/// what gets tested. Measured over LibreOffice's <c>sd/qa/unit/data/pptx</c>: 66 decks carry a
/// diagram, 37 of them have no usable baked drawing, and those 37 draw as nothing at all
/// without an evaluator.
/// </para>
/// <para>
/// <strong>The program.</strong> The layout definition is a tree of atoms —
/// <c>forEach</c>, <c>choose</c>/<c>if</c>/<c>else</c>, <c>layoutNode</c>, <c>alg</c>,
/// <c>constr</c>, <c>rule</c>, <c>shape</c> — walked twice. The first walk creates one shape per
/// (layout node, iteration) pair, which is possible only because the authoring application
/// already ran the program once and wrote its answer into the data model as
/// <c>type="pres"</c> points: a layout node named <c>textNode</c> gets exactly as many shapes as
/// there are presentation points whose <c>presName</c> is <c>textNode</c>. The second walk sizes
/// and positions them, innermost algorithm first. LibreOffice's two walks are
/// <c>ShapeCreationVisitor</c> and <c>ShapeLayoutingVisitor</c>,
/// <c>oox/source/drawingml/diagram/layoutatomvisitors.cxx</c>.
/// </para>
/// <para>
/// <strong>What it declines.</strong> An algorithm this does not implement makes the whole
/// diagram decline rather than draw approximately — see <see cref="Supported"/>. Half-evaluating
/// a <c>snake</c> puts every node at the origin on top of every other, which reads as a bug in
/// the shapes rather than as an unimplemented algorithm, and it would move files in the render
/// sweep that currently draw nothing honestly.
/// </para>
/// </remarks>
internal sealed class PptxDiagramEvaluator
{
    /// <summary>Sixtieths of a thousandth of a degree, DrawingML's rotation unit.</summary>
    private const int PerDegree = 60000;

    /// <summary>
    /// The algorithms this evaluates, measured against LibreOffice's rendering.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>lin</c> and <c>composite</c> between them are the commonest by a wide margin — of the
    /// 37 decks in <c>sd/qa/unit/data/pptx</c> with no baked drawing, 20 use only these five —
    /// and they are also the two that carry the constraint machinery, so everything else is a
    /// smaller increment on top rather than a separate subsystem.
    /// </para>
    /// <para>
    /// <c>snake</c>, <c>cycle</c>, <c>pyra</c>, <c>hierRoot</c> and <c>hierChild</c> are not
    /// here. Each is a different geometry with its own hand-tuned constants in LibreOffice —
    /// <c>snake</c> alone is 330 lines that search for a grid — and none of them can be checked
    /// by the same measurement, so they are separate work rather than four more cases in a
    /// switch.
    /// </para>
    /// </remarks>
    private static readonly HashSet<string> Supported =
        new(StringComparer.Ordinal) { "lin", "composite", "sp", "tx", "conn" };

    private readonly DiagramData _data;
    private readonly DiagramLayoutNodeAtom _root;
    private readonly Dictionary<string, DiagramForEachAtom> _named =
        new(StringComparer.Ordinal);
    private readonly Dictionary<DiagramPoint, DiagramShape> _byPresentationPoint = [];

    private PptxDiagramEvaluator(DiagramData data, DiagramLayoutNodeAtom root)
    {
        _data = data;
        _root = root;
        Index(root);
    }

    /// <summary>
    /// Evaluates a diagram, or returns null when it uses an algorithm this does not implement.
    /// </summary>
    /// <param name="data">The data model.</param>
    /// <param name="definition">The <c>dgm:layoutDef</c> root element.</param>
    /// <param name="width">The frame's width in EMUs.</param>
    /// <param name="height">The frame's height in EMUs.</param>
    public static DiagramShape? Evaluate(
        DiagramData data, XElement? definition, int width, int height)
    {
        if (data.Root is null) return null;
        if (PptxDiagramLayout.Read(definition) is not { } root) return null;
        if (!Evaluable(root)) return null;

        DiagramShape frame = new()
        {
            InternalName = root.Name,
            IsGroup = true,
            Width = width,
            Height = height,
        };

        PptxDiagramEvaluator evaluator = new(data, root);
        new CreationWalk(evaluator, data.Root, frame).Visit(root);
        new LayoutWalk(evaluator, data.Root).Visit(root);

        Order(frame);
        RemoveEmptyGroups(frame);
        return frame;
    }

    /// <summary>True when every algorithm the definition names is one this implements.</summary>
    private static bool Evaluable(DiagramAtom atom)
    {
        if (atom is DiagramAlgorithmAtom algorithm && !Supported.Contains(algorithm.Type))
        {
            return false;
        }

        foreach (DiagramAtom child in atom.Children)
        {
            if (!Evaluable(child)) return false;
        }

        return true;
    }

    private void Index(DiagramAtom atom)
    {
        if (atom is DiagramForEachAtom loop && loop.Name.Length != 0) _named[loop.Name] = loop;
        foreach (DiagramAtom child in atom.Children) Index(child);
    }

    // ---------------------------------------------------------------- walking

    /// <summary>
    /// The shared half of the two walks: the loop, the conditional, and the layout node's guard.
    /// </summary>
    /// <remarks>
    /// Ported from <c>LayoutAtomVisitorBase</c>,
    /// <c>oox/source/drawingml/diagram/layoutatomvisitorbase.cxx</c>. The loop counter is the
    /// state that matters: <c>Index</c> is both "which iteration of the enclosing
    /// <c>forEach</c>" and "which presentation point of this layout node's name", and the two
    /// being the same number is the whole trick that lets a program with no data in it produce
    /// one shape per node.
    /// </remarks>
    private abstract class Walk(PptxDiagramEvaluator evaluator, DiagramPoint? current)
    {
        protected readonly PptxDiagramEvaluator Evaluator = evaluator;
        protected DiagramPoint? Current = current;
        protected int Index;
        protected int Step = 1;
        protected int Count;

        public void Visit(DiagramAtom atom)
        {
            switch (atom)
            {
                case DiagramChooseAtom choose: VisitChoose(choose); break;
                case DiagramConditionAtom condition: VisitDefault(condition); break;
                case DiagramForEachAtom loop: VisitForEach(loop); break;
                case DiagramLayoutNodeAtom node: VisitLayoutNode(node); break;
                case DiagramAlgorithmAtom algorithm: VisitAlgorithm(algorithm); break;
                case DiagramConstraintAtom constraint: VisitConstraint(constraint); break;
                case DiagramRuleAtom rule: VisitRule(rule); break;
                case DiagramShapeAtom: break;
            }
        }

        protected void VisitDefault(DiagramAtom atom)
        {
            foreach (DiagramAtom child in atom.Children) Visit(child);
        }

        protected virtual void VisitAlgorithm(DiagramAlgorithmAtom atom) { }

        protected virtual void VisitConstraint(DiagramConstraintAtom atom) { }

        protected virtual void VisitRule(DiagramRuleAtom atom) { }

        protected abstract void VisitLayoutNode(DiagramLayoutNodeAtom atom);

        private void VisitChoose(DiagramChooseAtom atom)
        {
            foreach (DiagramAtom child in atom.Children)
            {
                if (child is not DiagramConditionAtom condition) continue;
                if (!Evaluator.Decide(condition, Current)) continue;

                Visit(condition);
                break;
            }
        }

        private void VisitForEach(DiagramForEachAtom atom)
        {
            if (atom.Reference.Length != 0)
            {
                if (Evaluator._named.TryGetValue(atom.Reference, out DiagramForEachAtom? target))
                {
                    Visit(target);
                }

                return;
            }

            // A list's last gap is not drawn, and the layout says so by hiding the final
            // transition rather than by iterating one fewer time.
            if (atom.Iterator.HideLastTransition
                && atom.Iterator.Axis.Count != 0
                && atom.Iterator.Axis[0] == "followSib"
                && Index + Step >= Count)
            {
                return;
            }

            int children = 1;
            if (atom.Iterator.PointType is "node" or "nonAsst")
            {
                children = Evaluator.ShallowCount(atom);
            }

            int count = Math.Min(children, atom.Iterator.Count == -1 ? children : atom.Iterator.Count);

            int oldIndex = Index;
            int oldStep = Step;
            int oldCount = Count;
            int step = atom.Iterator.Step;

            Step = step;
            Count = count;

            for (Index = 0; Index < count && step > 0; Index += step)
            {
                foreach (DiagramAtom child in atom.Children) Visit(child);
            }

            Index = oldIndex;
            Step = oldStep;
            Count = oldCount;
        }

        /// <summary>
        /// The presentation point this layout node stands for on this iteration, or null.
        /// </summary>
        /// <remarks>
        /// Both walks share this guard and both stop dead when it fails: a layout node only
        /// produces a shape when the data model holds a presentation point of its name at the
        /// current index <em>and</em> a connection makes that point a child of the enclosing
        /// node's point. Without the second half a nested list would draw its inner nodes once
        /// per outer node.
        /// </remarks>
        protected DiagramPoint? PointFor(DiagramLayoutNodeAtom atom)
        {
            IReadOnlyList<DiagramPoint> points = Evaluator._data.ByPresentationName(atom.Name);
            if (Index >= points.Count) return null;

            DiagramPoint point = points[Index];
            if (Current is null) return null;

            return Evaluator._data.IsChild(Current.ModelId, point.ModelId) ? point : null;
        }
    }

    /// <summary>How many presentation points a <c>forEach</c>'s body could produce.</summary>
    /// <remarks>
    /// <c>ShallowPresNameVisitor</c>, <c>layoutatomvisitorbase.cxx:159</c>: the largest number of
    /// presentation points any layout node directly inside the loop has. It is a count of what
    /// the authoring application already produced, not of the data — which is why an iteration
    /// count never has to be derived from the connection graph.
    /// </remarks>
    private int ShallowCount(DiagramAtom atom)
    {
        int count = 0;
        foreach (DiagramAtom child in atom.Children) count = Math.Max(count, ShallowCountOf(child));
        return count;
    }

    private int ShallowCountOf(DiagramAtom atom)
    {
        switch (atom)
        {
            case DiagramLayoutNodeAtom node:
                return _data.ByPresentationName(node.Name).Count;
            case DiagramConstraintAtom:
            case DiagramRuleAtom:
            case DiagramAlgorithmAtom:
            case DiagramShapeAtom:
                return 0;
            default:
            {
                int count = 0;
                foreach (DiagramAtom child in atom.Children)
                {
                    count = Math.Max(count, ShallowCountOf(child));
                }

                return count;
            }
        }
    }

    // --------------------------------------------------------------- creation

    /// <summary>The first walk: one shape per layout node per iteration.</summary>
    private sealed class CreationWalk(
        PptxDiagramEvaluator evaluator, DiagramPoint? current, DiagramShape parent)
        : Walk(evaluator, current)
    {
        private DiagramShape _parent = parent;
        private bool _lookForAlgorithm;

        protected override void VisitAlgorithm(DiagramAlgorithmAtom atom)
        {
            if (!_lookForAlgorithm) return;

            _parent.AspectRatio = atom.AspectRatio;
            _parent.VerticalShapesCount = VerticalShapesCount(atom, _parent);
        }

        protected override void VisitLayoutNode(DiagramLayoutNodeAtom atom)
        {
            if (_lookForAlgorithm) return;
            if (PointFor(atom) is not { } point) return;

            DiagramShape shape = _parent;

            // The root layout node reuses the frame rather than making a shape inside it —
            // LayoutNode::setExistingShape, diagram.cxx:133.
            if (ReferenceEquals(atom, Evaluator._root))
            {
                Evaluator.SetUp(atom, shape, point);
                Evaluator._byPresentationPoint[point] = shape;
            }
            else if (Template(atom) is { } created)
            {
                shape = created;
                shape.InternalName = atom.Name;
                Evaluator.SetUp(atom, shape, point);
                _parent.Children.Add(shape);
                Evaluator._byPresentationPoint[point] = shape;
            }

            DiagramPoint? previousPoint = Current;
            DiagramShape previousParent = _parent;
            Current = point;
            _parent = shape;

            _lookForAlgorithm = false;
            VisitDefault(atom);
            _lookForAlgorithm = true;
            VisitDefault(atom);
            _lookForAlgorithm = false;

            _parent = previousParent;
            Current = previousPoint;
        }

        /// <summary>
        /// The <c>dgm:shape</c> a layout node's shape is copied from, or null when it has none.
        /// </summary>
        /// <remarks>
        /// <para>
        /// Only <c>choose</c> and <c>if</c>/<c>else</c> are traversed looking for it, so a
        /// nested layout node's own shape is not mistaken for this one's —
        /// <c>ShapeTemplateVisitor</c>, <c>layoutatomvisitors.cxx:143-188</c>.
        /// </para>
        /// <para>
        /// <strong>A layout node with no <c>dgm:shape</c> produces nothing at all</strong>, and
        /// the difference is visible rather than academic. <c>vertical-bracket-list.pptx</c>
        /// has a spacer node — <c>&lt;dgm:layoutNode name="spH"&gt;&lt;dgm:alg
        /// type="sp"/&gt;&lt;/dgm:layoutNode&gt;</c>, with no shape element — and creating an
        /// empty group for it gives the linear algorithm a fourth child to place, which moves
        /// everything after it left by the 9.6 pt the spacer's own constraint asked for.
        /// LibreOffice warns and skips (<c>layoutatomvisitors.cxx:112</c>); the walk still
        /// descends, with the parent unchanged.
        /// </para>
        /// </remarks>
        private DiagramShape? Template(DiagramLayoutNodeAtom atom)
        {
            if (FindTemplate(atom) is not { } template) return null;

            return new DiagramShape
            {
                IsGroup = !template.HasType,
                PresetType = template.PresetType,
                HideGeometry = template.HideGeometry,
                DiagramRotation = template.Rotation * PerDegree,
                ZOrderOffset = template.ZOrderOffset,
            };
        }

        private DiagramShapeAtom? FindTemplate(DiagramAtom atom)
        {
            foreach (DiagramAtom child in atom.Children)
            {
                switch (child)
                {
                    case DiagramShapeAtom shape:
                        return shape;
                    case DiagramChooseAtom:
                    {
                        foreach (DiagramAtom branch in child.Children)
                        {
                            if (branch is not DiagramConditionAtom condition) continue;
                            if (!Evaluator.Decide(condition, Current)) continue;
                            if (FindTemplate(condition) is { } found) return found;

                            break;
                        }

                        break;
                    }

                    case DiagramConditionAtom:
                        if (FindTemplate(child) is { } nested) return nested;
                        break;
                }
            }

            return null;
        }

        /// <summary>
        /// How many shapes stack vertically inside one, which the hierarchy algorithms divide by.
        /// </summary>
        /// <remarks>
        /// <c>AlgAtom::getVerticalShapesCount</c>, <c>diagramlayoutatoms.cxx:1035</c>. Kept even
        /// though the hierarchy algorithms are not implemented, because it is also what tells a
        /// linear layout that a connector occupies no row of its own.
        /// </remarks>
        private static int VerticalShapesCount(DiagramAlgorithmAtom atom, DiagramShape shape)
        {
            if (shape.Children.Count == 0) return shape.PresetType == "conn" ? 0 : 1;

            string direction = atom.Type == "hierRoot"
                ? "fromT"
                : atom.Parameter("linDir", "fromL");

            string secondary = atom.Parameter("secLinDir", "");

            int count = 0;
            if (direction is "fromT" or "fromB")
            {
                foreach (DiagramShape child in shape.Children) count += child.VerticalShapesCount;
            }
            else if (direction is "fromL" or "fromR" && secondary == "fromT")
            {
                foreach (DiagramShape child in shape.Children) count += child.VerticalShapesCount;
                count = (count + 1) / 2;
            }
            else
            {
                foreach (DiagramShape child in shape.Children)
                {
                    count = Math.Max(count, child.VerticalShapesCount);
                }
            }

            return count;
        }
    }

    // -------------------------------------------------------------- layouting

    /// <summary>The second walk: constraints, then rules, then the algorithm, then the children.</summary>
    /// <remarks>
    /// The order is the point. An algorithm sizes the shapes it owns, so the outer one has to run
    /// before the inner ones have a parent to divide up — and the constraints it uses include
    /// every constraint an enclosing layout node stated, which is why the list is inherited down
    /// the walk and truncated back on the way out
    /// (<c>ShapeLayoutingVisitor::visit(LayoutNode)</c>, <c>layoutatomvisitors.cxx:213</c>).
    /// </remarks>
    private sealed class LayoutWalk(PptxDiagramEvaluator evaluator, DiagramPoint? current)
        : Walk(evaluator, current)
    {
        private readonly List<DiagramConstraint> _constraints = [];
        private readonly List<string> _rules = [];
        private Stage _stage = Stage.LayoutNode;

        private enum Stage
        {
            Constraint,
            Rule,
            Algorithm,
            LayoutNode,
        }

        protected override void VisitConstraint(DiagramConstraintAtom atom)
        {
            if (_stage != Stage.Constraint) return;
            if (Accept(atom.Constraint, requireForName: true)) _constraints.Add(atom.Constraint);
        }

        protected override void VisitRule(DiagramRuleAtom atom)
        {
            if (_stage != Stage.Rule) return;
            if (atom.ForName.Length != 0) _rules.Add(atom.ForName);
        }

        protected override void VisitAlgorithm(DiagramAlgorithmAtom atom)
        {
            if (_stage != Stage.Algorithm) return;
            if (Current is null) return;
            if (!Evaluator._byPresentationPoint.TryGetValue(Current, out DiagramShape? shape)) return;

            PptxDiagramAlgorithms.Apply(atom, shape, _constraints, _rules);
        }

        protected override void VisitLayoutNode(DiagramLayoutNodeAtom atom)
        {
            if (_stage != Stage.LayoutNode) return;
            if (PointFor(atom) is not { } point) return;

            int inherited = _constraints.Count;
            int inheritedRules = _rules.Count;
            DiagramPoint? previous = Current;
            Current = point;

            _stage = Stage.Constraint;
            VisitDefault(atom);
            _stage = Stage.Rule;
            VisitDefault(atom);
            _stage = Stage.Algorithm;
            VisitDefault(atom);
            _stage = Stage.LayoutNode;
            VisitDefault(atom);

            Current = previous;
            _constraints.RemoveRange(inherited, _constraints.Count - inherited);
            _rules.RemoveRange(inheritedRules, _rules.Count - inheritedRules);
        }
    }

    /// <summary>
    /// Whether a constraint is one of the kinds the algorithms honour.
    /// </summary>
    /// <remarks>
    /// <c>ConstraintAtom::parseConstraint</c>, <c>diagramlayoutatoms.cxx:963</c>. Only equality
    /// is accepted: an inequality is a bound rather than a value, and treating one as a value
    /// sizes a shape to its minimum. Spacing and margins are allowed to have no
    /// <c>forName</c> because they apply to the node that states them.
    /// </remarks>
    private static bool Accept(DiagramConstraint constraint, bool requireForName)
    {
        if (requireForName)
        {
            if (constraint.Type is "sp" or "lMarg" or "rMarg" or "tMarg" or "bMarg")
            {
                requireForName = false;
            }

            if (constraint.PointType == "sibTrans") requireForName = false;
        }

        if (requireForName && constraint.ForName.Length == 0) return false;

        return constraint.Operator is "none" or "equ" && constraint.Type != "none";
    }

    // -------------------------------------------------------------- decisions

    /// <summary>
    /// Evaluates a <c>dgm:if</c>.
    /// </summary>
    /// <remarks>
    /// <c>ConditionAtom::getDecision</c>, <c>diagramlayoutatoms.cxx:890</c>. The unimplemented
    /// functions return <em>true</em> rather than false, which is deliberate and is LibreOffice's
    /// behaviour: a <c>choose</c> takes its first matching branch, so defaulting to true takes
    /// the first branch, while defaulting to false would fall through every branch to the
    /// <c>else</c> and lay a diagram out as its degenerate case.
    /// </remarks>
    private bool Decide(DiagramConditionAtom condition, DiagramPoint? point)
    {
        if (condition.IsElse) return true;
        if (point is null) return false;

        switch (condition.Function)
        {
            case "var" when condition.Argument == "dir":
                return Compare(condition.Operator, point.Direction, condition.Value);

            case "var" when condition.Argument == "hierBranch":
            {
                string branch = point.HierarchyBranch ?? "";
                if (branch.Length == 0)
                {
                    string parent = _data.Navigate(
                        "presParOf", point.ModelId, sourceToDestination: false);
                    branch = _data.ById(parent)?.HierarchyBranch ?? "std";
                }

                return Compare(condition.Operator, branch, condition.Value);
            }

            case "cnt":
                return Compare(condition.Operator, NodeCount(condition, point), Number(condition.Value));

            case "maxDepth":
                return Compare(
                    condition.Operator,
                    MaximumDepth(point.PresentationAssociationId, 0),
                    Number(condition.Value));

            default:
                return true;
        }
    }

    private int NodeCount(DiagramConditionAtom condition, DiagramPoint point)
    {
        string nodeId = point.PresentationAssociationId;

        // "HACK: special case - count children of first child" — diagramlayoutatoms.cxx:876.
        if (condition.Iterator.Axis.Count == 2
            && condition.Iterator.Axis[0] == "ch"
            && condition.Iterator.Axis[1] == "ch")
        {
            nodeId = _data.Navigate("parOf", nodeId, sourceToDestination: true);
        }

        if (nodeId.Length == 0) return 0;

        int count = 0;
        foreach (DiagramConnection connection in _data.Connections)
        {
            if (connection.Type == "parOf" && connection.SourceId == nodeId) count++;
        }

        return count;
    }

    private int MaximumDepth(string nodeId, int guard)
    {
        if (guard > _data.Connections.Count) return 0;

        int longest = 0;
        foreach (DiagramConnection connection in _data.Connections)
        {
            if (connection.Type != "parOf" || connection.SourceId != nodeId) continue;

            longest = Math.Max(longest, MaximumDepth(connection.DestId, guard + 1) + 1);
        }

        return longest;
    }

    private static bool Compare(string op, int first, int second)
        => op switch
        {
            "equ" => first == second,
            "gt" => first > second,
            "gte" => first >= second,
            "lt" => first < second,
            "lte" => first <= second,
            "neq" => first != second,
            _ => false,
        };

    private static bool Compare(string op, string first, string second)
        => op switch
        {
            "equ" => first == second,
            "neq" => first != second,
            _ => false,
        };

    private static int Number(string text)
        => int.TryParse(text, NumberStyles.Integer, CultureInfo.InvariantCulture, out int value)
            ? value
            : 0;

    // ----------------------------------------------------------- shape set-up

    /// <summary>
    /// Fills a freshly created shape from the data nodes its presentation point presents.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>LayoutNode::setupShape</c>, <c>diagramlayoutatoms.cxx:1866</c>. A presentation point
    /// can present several data nodes at once — a heading and the bullets under it — and the
    /// <em>shallowest</em> of them supplies the formatting while all of them supply text, each
    /// paragraph tagged with its own node's depth.
    /// </para>
    /// <para>
    /// <strong>Depth zero never happens, and reading the source as though it does costs an
    /// afternoon.</strong> LibreOffice's <c>setupShape</c> has a branch for <c>mnDepth == 0</c>
    /// that replaces the shape's geometry as well as its fill — which would throw away the
    /// preset the layout definition asked for — and it is unreachable: the map that holds the
    /// depths stores a computed zero as −1 (<c>datamodel_svx.cxx:989</c>,
    /// <c>mnDepth = nDepth != 0 ? nDepth : -1</c>), so every entry is either −1 or one or more.
    /// The shallowest-depth branch below is the one that runs, and it takes fill and line only.
    /// </para>
    /// </remarks>
    private void SetUp(DiagramLayoutNodeAtom atom, DiagramShape shape, DiagramPoint point)
    {
        shape.StyleLabel = point.StyleLabel.Length != 0 ? point.StyleLabel : atom.StyleLabel;
        shape.StyleIndex = Math.Max(point.StyleIndex, 0);

        if (_data.PresentationOf(point.ModelId) is not { } sources)
        {
            shape.ShapeProperties ??= point.ShapeProperties;
            return;
        }

        int shallowest = int.MaxValue;
        foreach (DiagramPresentationSource source in sources.Values)
        {
            shallowest = Math.Min(shallowest, source.Depth);
        }

        foreach (DiagramPresentationSource source in sources.Values)
        {
            if (_data.ById(source.SourceId) is not { } data) continue;

            shape.DataNodeType = data.Type;
            shape.CustomText |= data.CustomText;

            if (source.Depth == shallowest) shape.ShapeProperties = data.ShapeProperties;

            if (data.TextBody is not { } body) continue;

            List<XElement> paragraphs = body
                .Elements(XName.Get("p", Paperless.Ooxml.OoxmlNamespaces.DrawingML))
                .Where(HasRuns)
                .ToList();

            if (paragraphs.Count == 0) continue;

            shape.TextBodySource ??= body;

            foreach (XElement paragraph in paragraphs)
            {
                shape.Text.Add(new DiagramTextParagraph(paragraph, source.Depth));
            }
        }
    }

    /// <summary>
    /// True when a paragraph has at least one run, which is what makes a data node contribute.
    /// </summary>
    /// <remarks>
    /// The guard LibreOffice writes as
    /// <c>!getParagraphs().empty() &amp;&amp; !getParagraphs().front()-&gt;getRuns().empty()</c>.
    /// It matters because a data model states an empty <c>dgm:t</c> for every transition point
    /// it has, and taking those as text gives every arrow in a diagram an empty label whose
    /// height still pushes the real one off centre.
    /// </remarks>
    private static bool HasRuns(XElement paragraph)
        => paragraph.Elements(XName.Get("r", Paperless.Ooxml.OoxmlNamespaces.DrawingML)).Any();

    // ------------------------------------------------------------------ order

    /// <summary>
    /// Applies the <c>zOrderOff</c> the layout definition states, then sorts by the result.
    /// </summary>
    /// <remarks>
    /// <c>sortChildrenByZOrder</c>, <c>diagram.cxx:67</c>. An offset moves one shape up the
    /// stack and pushes the ones it passed down by one each, so the permutation is a rotation
    /// rather than a swap — which is how a chevron list gets its overlaps in the right order
    /// without the layout stating an index for every shape.
    /// </remarks>
    private static void Order(DiagramShape shape)
    {
        List<DiagramShape> children = shape.Children;

        for (int i = 0; i < children.Count; i++) children[i].ZOrder = i;

        for (int i = 0; i < children.Count; i++)
        {
            int offset = children[i].ZOrderOffset;
            if (offset <= 0) continue;

            children[i].ZOrder += offset;
            children[i].ZOrderOffset = 0;

            for (int j = 0; j < offset; j++)
            {
                int index = i + j + 1;
                if (index >= children.Count) break;

                children[index].ZOrder--;
            }
        }

        children.Sort((a, b) => a.ZOrder.CompareTo(b.ZOrder));

        foreach (DiagramShape child in children) Order(child);
    }

    /// <summary>
    /// Drops the grouping shapes that only existed to take up room.
    /// </summary>
    /// <remarks>
    /// Spacing between a diagram's nodes is a real shape in the layout definition — a
    /// <c>dgm:layoutNode</c> with an <c>sp</c> algorithm and a group for a template — so it
    /// cannot be removed before the algorithm above it has divided the space. Afterwards it
    /// draws nothing and only costs a shape (<c>removeUnneededGroupShapes</c>,
    /// <c>diagram.cxx:106</c>).
    /// </remarks>
    private static void RemoveEmptyGroups(DiagramShape shape)
    {
        shape.Children.RemoveAll(child => child.IsGroup && child.Children.Count == 0);
        foreach (DiagramShape child in shape.Children) RemoveEmptyGroups(child);
    }
}
