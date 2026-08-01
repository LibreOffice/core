using System.Globalization;
using System.Xml.Linq;

namespace Paperless.Presentations.Ooxml;

/// <summary>
/// One <c>dgm:pt</c> of a diagram's data model: a node the author typed, or one the layout
/// generated to present it.
/// </summary>
/// <remarks>
/// <para>
/// The two kinds live in the same list and are told apart by <see cref="Type"/>. A
/// <c>node</c> (the default) is the author's; a <c>pres</c> is the layout definition's, created
/// by the authoring application when it ran the layout and named after the
/// <c>dgm:layoutNode</c> that produced it. That name — <see cref="PresentationName"/> — is the
/// join between the data part and the layout part, and the evaluator does nothing until it has
/// matched the two.
/// </para>
/// <para>
/// A <c>parTrans</c> and a <c>sibTrans</c> are the two transitions on a connection: the edge
/// from a parent to a child, and the gap between two siblings. Both can carry text and a shape,
/// which is why an arrow between two boxes has a label at all.
/// </para>
/// </remarks>
internal sealed class DiagramPoint
{
    /// <summary>The GUID that connections refer to this point by.</summary>
    public string ModelId { get; init; } = "";

    /// <summary>
    /// <c>doc</c>, <c>node</c>, <c>asst</c>, <c>pres</c>, <c>parTrans</c> or <c>sibTrans</c>.
    /// </summary>
    public string Type { get; init; } = "node";

    /// <summary>The <c>dgm:layoutNode</c> whose evaluation created this point, for a <c>pres</c>.</summary>
    public string PresentationName { get; init; } = "";

    /// <summary>The data point this presentation point stands for (<c>presAssocID</c>).</summary>
    public string PresentationAssociationId { get; init; } = "";

    /// <summary>The quick-style and colour-transform label this point resolves against.</summary>
    public string StyleLabel { get; init; } = "";

    /// <summary>
    /// Which of the shapes sharing <see cref="StyleLabel"/> this one is (<c>presStyleIdx</c>).
    /// </summary>
    /// <remarks>
    /// It is what the colour transform's list is indexed by, and it belongs to the presentation
    /// point rather than to the shape's position among its siblings — so a diagram whose nodes
    /// are created out of order is still coloured in the author's order.
    /// </remarks>
    public int StyleIndex { get; init; } = -1;

    /// <summary><c>norm</c> or <c>rev</c> — the layout variable a <c>choose</c> tests.</summary>
    public string Direction { get; init; } = "norm";

    /// <summary><c>dgm:hierBranch</c>, absent when the point defers to its parent's.</summary>
    public string? HierarchyBranch { get; init; }

    /// <summary>
    /// Whether the author formatted this point's text, which is what suppresses autofit.
    /// </summary>
    public bool CustomText { get; init; }

    /// <summary>The point's own <c>dgm:spPr</c>, whose fill and line override the style's.</summary>
    public XElement? ShapeProperties { get; init; }

    /// <summary>The point's <c>dgm:t</c>, an ordinary DrawingML text body.</summary>
    public XElement? TextBody { get; init; }
}

/// <summary>One <c>dgm:cxn</c>: a directed edge of the data model.</summary>
/// <param name="Type">
/// <c>parOf</c> (the default) for the author's tree, <c>presOf</c> for "this presentation point
/// shows that data point", <c>presParOf</c> for the tree of presentation points.
/// </param>
/// <param name="SourceId">The model id the edge leaves.</param>
/// <param name="DestId">The model id the edge arrives at.</param>
/// <param name="ParentTransitionId">The point standing for the edge itself, if any.</param>
/// <param name="SiblingTransitionId">The point standing for the gap after the destination.</param>
/// <param name="SourceOrder">Where among the source's outgoing edges this one sits.</param>
/// <param name="DestOrder">Where among the destination's incoming edges this one sits.</param>
internal readonly record struct DiagramConnection(
    string Type,
    string SourceId,
    string DestId,
    string ParentTransitionId,
    string SiblingTransitionId,
    int SourceOrder,
    int DestOrder);

/// <summary>Which data point a presentation point shows, and at what outline depth.</summary>
internal readonly record struct DiagramPresentationSource(string SourceId, int Depth);

/// <summary>
/// A diagram's data model — <c>data1.xml</c> — with the two indexes the evaluator navigates by.
/// </summary>
/// <remarks>
/// <para>
/// The part is a flat list of points and a flat list of connections, and every question the
/// layout-atom evaluator asks is a graph query over them. LibreOffice builds the same two maps
/// once, in <c>svx/source/diagram/datamodel_svx.cxx:840</c>
/// (<c>DiagramData_svx::buildDiagramDataModel</c>), and the evaluator does nothing but look
/// things up in them afterwards.
/// </para>
/// <para>
/// <strong>Depth is a property of the connection, not of the point.</strong> A
/// <c>presOf</c> connection says a presentation point shows a data point; how deep that data
/// point sits in the author's tree decides which outline level its paragraphs take, and it is
/// computed by walking <c>parOf</c> edges backwards
/// (<c>datamodel_svx.cxx:819</c>, <c>calcDepth</c>). A node with no parent gets 0 which the
/// caller then stores as −1, meaning "do not set a level at all" — the distinction matters,
/// because level 0 and no level are different things to a text body that inherits one.
/// </para>
/// </remarks>
internal sealed class DiagramData
{
    private readonly List<DiagramPoint> _points = [];
    private readonly List<DiagramConnection> _connections = [];
    private readonly Dictionary<string, DiagramPoint> _byModelId = new(StringComparer.Ordinal);
    private readonly Dictionary<string, List<DiagramPoint>> _byPresentationName =
        new(StringComparer.Ordinal);
    private readonly Dictionary<string, SortedDictionary<int, DiagramPresentationSource>>
        _presentationOf = new(StringComparer.Ordinal);

    /// <summary>Every point, in the order <c>dgm:ptLst</c> states them.</summary>
    public IReadOnlyList<DiagramPoint> Points => _points;

    /// <summary>Every connection, in the order <c>dgm:cxnLst</c> states them.</summary>
    public IReadOnlyList<DiagramConnection> Connections => _connections;

    /// <summary>The <c>doc</c> point, which is where evaluation starts.</summary>
    public DiagramPoint? Root { get; private set; }

    /// <summary>The point a model id names, or null.</summary>
    public DiagramPoint? ById(string? modelId)
        => modelId is not null && _byModelId.TryGetValue(modelId, out DiagramPoint? point)
            ? point
            : null;

    /// <summary>
    /// The presentation points a <c>dgm:layoutNode</c>'s name produced, in document order.
    /// </summary>
    /// <remarks>
    /// A layout node inside a <c>dgm:forEach</c> produces one point per iteration, and the
    /// evaluator indexes this list by the loop counter — so the order here <em>is</em> the
    /// drawn order, and reading the part in any other order silently transposes a diagram.
    /// </remarks>
    public IReadOnlyList<DiagramPoint> ByPresentationName(string name)
        => _byPresentationName.TryGetValue(name, out List<DiagramPoint>? points) ? points : [];

    /// <summary>The data points a presentation point shows, keyed by <c>destOrd</c>.</summary>
    public IReadOnlyDictionary<int, DiagramPresentationSource>? PresentationOf(string modelId)
        => _presentationOf.TryGetValue(modelId, out SortedDictionary<int, DiagramPresentationSource>? map)
            ? map
            : null;

    /// <summary>
    /// Follows one edge of a given type, forwards or backwards.
    /// </summary>
    /// <remarks>
    /// The port of <c>navigate</c>, <c>oox/source/drawingml/diagram/diagramlayoutatoms.cxx:837</c>.
    /// </remarks>
    public string Navigate(string type, string from, bool sourceToDestination)
    {
        foreach (DiagramConnection connection in _connections)
        {
            if (connection.Type != type) continue;

            if (sourceToDestination)
            {
                if (connection.SourceId == from) return connection.DestId;
            }
            else if (connection.DestId == from)
            {
                return connection.SourceId;
            }
        }

        return "";
    }

    /// <summary>True when a connection makes one point the direct child of another.</summary>
    public bool IsChild(string parentModelId, string childModelId)
    {
        foreach (DiagramConnection connection in _connections)
        {
            if (connection.SourceId == parentModelId && connection.DestId == childModelId) return true;
        }

        return false;
    }

    /// <summary>Reads a <c>dgm:dataModel</c> and builds the indexes over it.</summary>
    public static DiagramData Read(XElement model)
    {
        DiagramData data = new();

        XName pointList = XName.Get("ptLst", PptxDiagram.Uri);
        XName point = XName.Get("pt", PptxDiagram.Uri);

        foreach (XElement element in model.Element(pointList)?.Elements(point) ?? [])
        {
            data._points.Add(ReadPoint(element));
        }

        XName connectionList = XName.Get("cxnLst", PptxDiagram.Uri);
        XName connection = XName.Get("cxn", PptxDiagram.Uri);

        foreach (XElement element in model.Element(connectionList)?.Elements(connection) ?? [])
        {
            data._connections.Add(new DiagramConnection(
                element.Attribute("type")?.Value ?? "parOf",
                element.Attribute("srcId")?.Value ?? "",
                element.Attribute("destId")?.Value ?? "",
                element.Attribute("parTransId")?.Value ?? "",
                element.Attribute("sibTransId")?.Value ?? "",
                Integer(element, "srcOrd"),
                Integer(element, "destOrd")));
        }

        data.Build();
        return data;
    }

    private static DiagramPoint ReadPoint(XElement element)
    {
        XElement? properties = element.Element(XName.Get("prSet", PptxDiagram.Uri));
        XElement? variables = properties?.Element(XName.Get("presLayoutVars", PptxDiagram.Uri));

        return new DiagramPoint
        {
            ModelId = element.Attribute("modelId")?.Value ?? "",
            Type = element.Attribute("type")?.Value ?? "node",
            PresentationName = properties?.Attribute("presName")?.Value ?? "",
            PresentationAssociationId = properties?.Attribute("presAssocID")?.Value ?? "",
            StyleLabel = properties?.Attribute("presStyleLbl")?.Value ?? "",
            StyleIndex = Integer(properties, "presStyleIdx", -1),
            CustomText = properties?.Attribute("custT")?.Value is "1" or "true",
            Direction = Value(variables, "dir") ?? "norm",
            HierarchyBranch = Value(variables, "hierBranch"),
            ShapeProperties = element.Element(XName.Get("spPr", PptxDiagram.Uri)),
            TextBody = element.Element(XName.Get("t", PptxDiagram.Uri)),
        };
    }

    /// <summary>The <c>val</c> of a <c>dgm:presLayoutVars</c> child, or null when absent.</summary>
    private static string? Value(XElement? variables, string localName)
        => variables?.Element(XName.Get(localName, PptxDiagram.Uri))?.Attribute("val")?.Value;

    private static int Integer(XElement? element, string name, int whenAbsent = 0)
        => element?.Attribute(name)?.Value is { } text
           && int.TryParse(text, NumberStyles.Integer, CultureInfo.InvariantCulture, out int value)
            ? value
            : whenAbsent;

    private void Build()
    {
        foreach (DiagramPoint point in _points)
        {
            _byModelId.TryAdd(point.ModelId, point);

            if (point.Type == "doc") Root ??= point;

            if (point.PresentationName.Length == 0) continue;

            if (!_byPresentationName.TryGetValue(point.PresentationName, out List<DiagramPoint>? list))
            {
                list = [];
                _byPresentationName[point.PresentationName] = list;
            }

            list.Add(point);
        }

        foreach (DiagramConnection connection in _connections)
        {
            if (connection.Type != "presOf") continue;

            if (!_presentationOf.TryGetValue(
                    connection.DestId, out SortedDictionary<int, DiagramPresentationSource>? map))
            {
                map = [];
                _presentationOf[connection.DestId] = map;
            }

            map[connection.DestOrder] = new DiagramPresentationSource(connection.SourceId, 0);
        }

        foreach (SortedDictionary<int, DiagramPresentationSource> map in _presentationOf.Values)
        {
            foreach (int order in map.Keys.ToList())
            {
                int depth = Depth(map[order].SourceId, 0);
                map[order] = map[order] with { Depth = depth != 0 ? depth : -1 };
            }
        }
    }

    /// <summary>
    /// How deep a data point sits under the root, counted along complete <c>parOf</c> edges.
    /// </summary>
    /// <remarks>
    /// "Complete" is load-bearing and is why this reads four attributes rather than one: only an
    /// edge that names both transitions <em>and</em> both endpoints counts, so a generated
    /// connection contributes no depth. <c>calcDepth</c>,
    /// <c>svx/source/diagram/datamodel_svx.cxx:819</c>. The guard is against a data model whose
    /// <c>parOf</c> edges form a cycle, which recursion alone would follow forever — real files
    /// do not, but a reader that trusts them crashes rather than mis-drawing.
    /// </remarks>
    private int Depth(string modelId, int guard)
    {
        if (guard > _connections.Count) return 0;

        foreach (DiagramConnection connection in _connections)
        {
            if (connection.Type != "parOf") continue;
            if (connection.ParentTransitionId.Length == 0) continue;
            if (connection.SiblingTransitionId.Length == 0) continue;
            if (connection.SourceId.Length == 0 || connection.DestId.Length == 0) continue;
            if (connection.DestId != modelId) continue;

            return Depth(connection.SourceId, guard + 1) + 1;
        }

        return 0;
    }
}
