using Paperless.Core.Diagnostics;
using Paperless.MsBinary.Records;

namespace Paperless.MsBinary.Escher;

/// <summary>
/// Reads an Escher (MS-ODRAW) drawing container into a shape tree.
/// </summary>
/// <remarks>
/// <para>
/// The same reader serves DOC, XLS and PPT, because all three delegate their drawings to this
/// one format: a <c>DgContainer</c> per page or sheet, holding a patriarch
/// <c>SpgrContainer</c> whose children are <c>SpContainer</c> shapes and further
/// <c>SpgrContainer</c> groups. LibreOffice's counterpart is
/// <c>filter/source/msfilter/msdffimp.cxx</c>.
/// </para>
/// <para>
/// The one structural rule that is easy to get wrong: a group's <em>first</em> child
/// <c>SpContainer</c> is not a shape on the page but the group itself, carrying the group's
/// flags and its <c>msofbtSpgr</c> coordinate space. Treating it as content puts a phantom
/// empty shape in front of every group.
/// </para>
/// </remarks>
public sealed class EscherDrawingReader
{
    /// <summary>
    /// How deeply groups may nest before the reader stops descending.
    /// </summary>
    /// <remarks>
    /// Real files nest a handful deep. The cap exists because the record tree comes from
    /// untrusted input and an eight-byte container costs one stack frame, so a small file can
    /// otherwise ask for tens of thousands of them.
    /// </remarks>
    public const int MaxGroupDepth = 64;

    private readonly DffRecordBuffer _stream;
    private readonly List<Diagnostic> _diagnostics;
    private bool _reportedDepth;

    /// <summary>Creates a reader over a record stream.</summary>
    /// <param name="stream">The stream the drawing lives in.</param>
    /// <param name="diagnostics">Where to record what could not be read.</param>
    public EscherDrawingReader(DffRecordBuffer stream, List<Diagnostic> diagnostics)
    {
        ArgumentNullException.ThrowIfNull(stream);
        ArgumentNullException.ThrowIfNull(diagnostics);
        _stream = stream;
        _diagnostics = diagnostics;
    }

    /// <summary>
    /// Reads a <c>DgContainer</c>, returning its shapes in document order.
    /// </summary>
    /// <remarks>
    /// The patriarch group's own shape record is dropped — it describes the drawing, not
    /// anything drawn — but a page background shape, which some hosts store as a sibling of
    /// the patriarch rather than inside it, is kept and flagged so callers can decide.
    /// </remarks>
    public IReadOnlyList<EscherShape> ReadDrawing(DffRecordHeader drawingContainer)
    {
        List<EscherShape> shapes = [];

        foreach (DffRecordHeader child in _stream.Children(drawingContainer))
        {
            switch (child.Type)
            {
                case EscherRecordTypes.ShapeGroupContainer:
                    shapes.AddRange(ReadGroupChildren(child, depth: 0));
                    break;

                case EscherRecordTypes.ShapeContainer:
                    shapes.Add(ReadShape(child, depth: 0));
                    break;

                default:
                    break;
            }
        }

        return shapes;
    }

    /// <summary>
    /// The shapes of the patriarch group, which is a group whose own shape record describes
    /// the drawing rather than a drawn object.
    /// </summary>
    private List<EscherShape> ReadGroupChildren(DffRecordHeader group, int depth)
    {
        List<EscherShape> shapes = [];
        bool first = true;

        foreach (DffRecordHeader child in _stream.Children(group))
        {
            if (child.Type == EscherRecordTypes.ShapeContainer && first)
            {
                // The group's own shape. At the patriarch level it is the drawing itself.
                first = false;
                continue;
            }

            first = false;
            if (child.Type == EscherRecordTypes.ShapeContainer) shapes.Add(ReadShape(child, depth));
            else if (child.Type == EscherRecordTypes.ShapeGroupContainer) shapes.Add(ReadGroup(child, depth));
        }

        return shapes;
    }

    /// <summary>Reads a nested group: its own shape record, then the shapes it holds.</summary>
    private EscherShape ReadGroup(DffRecordHeader group, int depth)
    {
        if (depth >= MaxGroupDepth)
        {
            if (!_reportedDepth)
            {
                _reportedDepth = true;
                _diagnostics.Add(new Diagnostic(
                    DiagnosticSeverity.Warning, "PL2350",
                    $"Escher shape groups nest deeper than {MaxGroupDepth}; the deeper shapes "
                    + "were skipped. A tree this deep is either damaged or hostile."));
            }

            return new EscherShape { Flags = EscherShapeAttributes.Group };
        }

        DffRecordHeader? own = null;
        List<EscherShape> children = [];
        bool first = true;

        foreach (DffRecordHeader child in _stream.Children(group))
        {
            if (first && child.Type == EscherRecordTypes.ShapeContainer)
            {
                own = child;
                first = false;
                continue;
            }

            first = false;
            if (child.Type == EscherRecordTypes.ShapeContainer) children.Add(ReadShape(child, depth + 1));
            else if (child.Type == EscherRecordTypes.ShapeGroupContainer) children.Add(ReadGroup(child, depth + 1));
        }

        EscherShape shape = own is { } header
            ? ReadShape(header, depth)
            : new EscherShape { Flags = EscherShapeAttributes.Group };

        return Rebuild(shape, children);
    }

    private static EscherShape Rebuild(EscherShape shape, List<EscherShape> children) => new()
    {
        ShapeId = shape.ShapeId,
        ShapeType = shape.ShapeType,

        // A group's own shape record does not always set the group flag — some writers leave
        // it off and rely on the container's type — so it is forced on here, where the
        // container has already established what this is.
        Flags = shape.Flags | EscherShapeAttributes.Group,
        Properties = shape.Properties,
        MasterProperties = shape.MasterProperties,
        ChildAnchor = shape.ChildAnchor,
        GroupBounds = shape.GroupBounds,
        ClientAnchor = shape.ClientAnchor,
        ClientData = shape.ClientData,
        ClientTextbox = shape.ClientTextbox,
        Children = children,
    };

    /// <summary>Reads one <c>SpContainer</c>.</summary>
    private EscherShape ReadShape(DffRecordHeader container, int depth)
    {
        _ = depth;

        uint shapeId = 0;
        ushort shapeType = 0;
        EscherShapeAttributes flags = EscherShapeAttributes.None;
        EscherPropertyTable properties = EscherPropertyTable.Empty;
        EscherPropertyTable master = EscherPropertyTable.Empty;
        EscherRectangle? childAnchor = null;
        EscherRectangle? groupBounds = null;
        DffRecordHeader? clientAnchor = null;
        DffRecordHeader? clientData = null;
        DffRecordHeader? clientTextbox = null;

        foreach (DffRecordHeader record in _stream.Children(container))
        {
            ReadOnlySpan<byte> content = _stream.Content(record);

            switch (record.Type)
            {
                case EscherRecordTypes.Shape:
                    shapeId = DffRecordBuffer.ReadUInt32(content);
                    flags = (EscherShapeAttributes)DffRecordBuffer.ReadUInt32(content[4..]);
                    shapeType = record.Instance;
                    break;

                case EscherRecordTypes.ShapeProperties:
                    properties = EscherPropertyTable.Read(content, record.Instance);
                    break;

                case EscherRecordTypes.SecondaryShapeProperties:
                case EscherRecordTypes.TertiaryShapeProperties:
                    if (master.Count == 0) master = EscherPropertyTable.Read(content, record.Instance);
                    break;

                case EscherRecordTypes.ShapeGroup:
                    groupBounds = ReadRectangle(content);
                    break;

                case EscherRecordTypes.ChildAnchor:
                    childAnchor = ReadRectangle(content);
                    break;

                case EscherRecordTypes.ClientAnchor:
                    clientAnchor = record;
                    break;

                case EscherRecordTypes.ClientData:
                    clientData = record;
                    break;

                case EscherRecordTypes.ClientTextbox:
                    clientTextbox = record;
                    break;

                default:
                    break;
            }
        }

        return new EscherShape
        {
            ShapeId = shapeId,
            ShapeType = shapeType,
            Flags = flags,
            Properties = properties,
            MasterProperties = master,
            ChildAnchor = childAnchor,
            GroupBounds = groupBounds,
            ClientAnchor = clientAnchor,
            ClientData = clientData,
            ClientTextbox = clientTextbox,
        };
    }

    /// <summary>Reads the four 32-bit bounds a group or child anchor states.</summary>
    private static EscherRectangle? ReadRectangle(ReadOnlySpan<byte> content)
        => content.Length < 16
            ? null
            : new EscherRectangle(
                unchecked((int)DffRecordBuffer.ReadUInt32(content)),
                unchecked((int)DffRecordBuffer.ReadUInt32(content[4..])),
                unchecked((int)DffRecordBuffer.ReadUInt32(content[8..])),
                unchecked((int)DffRecordBuffer.ReadUInt32(content[12..])));
}
