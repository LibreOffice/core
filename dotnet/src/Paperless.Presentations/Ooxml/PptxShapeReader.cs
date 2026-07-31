using System.Xml.Linq;
using Paperless.Containers.Ooxml;
using Paperless.Core.Extraction;
using Paperless.Ooxml;
using Paperless.Ooxml.DrawingML;

namespace Paperless.Presentations.Ooxml;

/// <summary>
/// Walks a slide's <c>p:spTree</c>, reading every shape that carries content.
/// </summary>
/// <remarks>
/// <para>
/// Shapes are read in document order rather than sorted by position. Document order is what the
/// file states and what the authoring application used; inferring a reading order from
/// coordinates would guess, and guess differently for every layout.
/// </para>
/// <para>
/// Groups are descended into and shapes with text bodies are read. LibreOffice's own HTML export
/// loses both, so a comparison against it shows Paperless finding text the reference does not —
/// that is the reference being lossy, not this being wrong, and it is the same deliberate
/// improvement the ODP path already makes.
/// </para>
/// <para>
/// The layout's and master's own shapes are <strong>not</strong> read as slide content. They are
/// the deck's furniture — and, far worse, a master placeholder carries prompt text ("Click to
/// edit the outline text format") which would otherwise appear on every slide of every deck.
/// LibreOffice never copies it either: <c>Shape::applyShapeReference</c> takes a placeholder's
/// geometry, fill, line and list style but resets the text body, and
/// <c>oox/source/ppt/pptgraphicshapecontext.cxx:135-155</c> passes <c>bUseText=false</c> for
/// every placeholder type there is.
/// </para>
/// </remarks>
internal sealed class PptxShapeReader
{
    private const int MaxGroupDepth = 32;

    private readonly PptxFile _file;
    private readonly string _partName;
    private readonly PptxTextStyles _styles;

    public PptxShapeReader(PptxFile file, string partName, PptxTextStyles styles)
    {
        _file = file;
        _partName = partName;
        _styles = styles;
    }

    /// <summary>Reads a shape tree's contents into a section.</summary>
    public void Read(XElement shapeTree, ContentNode target)
    {
        ArgumentNullException.ThrowIfNull(shapeTree);
        ArgumentNullException.ThrowIfNull(target);
        ReadShapes(shapeTree, target, depth: 0);
    }

    private void ReadShapes(XElement parent, ContentNode target, int depth)
    {
        foreach (XElement shape in parent.Elements())
        {
            if (Ppt.Is(shape, "sp") || Ppt.Is(shape, "cxnSp")) ReadShape(shape, target);
            else if (Ppt.Is(shape, "grpSp")) ReadGroup(shape, target, depth);
            else if (Ppt.Is(shape, "graphicFrame")) ReadGraphicFrame(shape, target);
            else if (Ppt.Is(shape, "pic")) ReadPicture(shape, target);

            // p:nvGrpSpPr and p:grpSpPr describe the tree itself; p:contentPart points at a
            // part in another vocabulary entirely. Neither holds text.
        }
    }

    private void ReadGroup(XElement group, ContentNode target, int depth)
    {
        // A group that contains itself is not expressible in a well-formed package, but a
        // pathological nesting depth is, and it costs stack rather than being caught by
        // anything else.
        if (depth >= MaxGroupDepth) return;
        ReadShapes(group, target, depth + 1);
    }

    private void ReadShape(XElement shape, ContentNode target)
    {
        XElement? body = Ppt.Child(shape, "txBody");

        // An empty placeholder is one the author left alone. It must not be reported, and — the
        // reason this test exists at all — it must not go looking for text to fill itself with.
        if (DrawingTextBody.IsEmpty(body)) return;

        DrawingTextBody.Read(body!, target, _styles.For(shape, ResolveHyperlink));
    }

    /// <summary>
    /// Reads a <c>p:graphicFrame</c>: a table, a chart, a diagram or an embedded object.
    /// </summary>
    /// <remarks>
    /// The frame is a generic wrapper and <c>a:graphicData/@uri</c> is the only thing that says
    /// what is inside it. Only the table is read as content here; a chart's series and a
    /// diagram's synthesised shapes live in their own parts and their own vocabularies, and
    /// re-executing a SmartArt layout algorithm to recover its text is a project of its own. So
    /// they are recorded as graphics rather than dropped, which keeps "there is something here"
    /// distinguishable from "there is nothing here".
    /// </remarks>
    private void ReadGraphicFrame(XElement frame, ContentNode target)
    {
        XElement? data = Drawing.Child(Drawing.Child(frame, "graphic"), "graphicData");
        string? uri = Drawing.Attribute(data, "uri");

        if (uri == DrawingTable.TableUri && Drawing.Child(data, "tbl") is { } table)
        {
            target.Children.Add(DrawingTable.Read(
                table,
                new DrawingTextOptions { ResolveHyperlink = ResolveHyperlink }));
            return;
        }

        if (uri == DiagramUri && ReadDiagram(data!, target)) return;

        target.Children.Add(new ContentImage
        {
            AlternativeText = Description(frame),
            MediaType = uri,
        });
    }

    /// <summary>The <c>a:graphicData</c> URI that identifies a SmartArt diagram.</summary>
    private const string DiagramUri = "http://schemas.openxmlformats.org/drawingml/2006/diagram";

    /// <summary>
    /// Reads the text of a SmartArt diagram from its data model.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A diagram's text is <em>typed by the author</em> and lives in <c>data1.xml</c> as ordinary
    /// DrawingML text bodies, one per <c>dgm:pt</c>. Only the <em>shapes</em> need the layout
    /// algorithm — the declarative "layout atom" program in <c>layout1.xml</c> that LibreOffice
    /// executes in <c>oox/source/drawingml/diagram/</c>, and which is the single largest
    /// subsystem in the PPTX importer. Extraction needs none of it: the words are already there.
    /// </para>
    /// <para>
    /// Skipping that is worth roughly a dozen files in LibreOffice's own PPTX test corpus that
    /// otherwise extract to nothing at all, and it is the reason "SmartArt: fallback or
    /// implement?" is answered "extract the text, decline the layout" rather than deferred.
    /// </para>
    /// <para>
    /// Points are read in <c>dgm:ptLst</c> order. LibreOffice walks the connection list instead,
    /// which is the difference between authoring order and drawn order — they agree for every
    /// diagram measured, and reconstructing a tree from <c>dgm:cxnLst</c> to reorder text is
    /// work the extracted output would not visibly benefit from.
    /// </para>
    /// </remarks>
    private bool ReadDiagram(XElement data, ContentNode target)
    {
        XName relIds = XName.Get("relIds", DiagramUri);
        string? dataModelId = data.Element(relIds)
            ?.Attribute(XName.Get("dm", OoxmlNamespaces.Relationships))?.Value;

        if (_file.Relationship(_partName, dataModelId) is not { IsExternal: false } relationship)
            return false;
        if (_file.Load(relationship.Target) is not { } model) return false;

        int before = target.Children.Count;
        foreach (XElement point in model.Element(XName.Get("ptLst", DiagramUri))
                                       ?.Elements(XName.Get("pt", DiagramUri)) ?? [])
        {
            // "doc" is the diagram itself, "pres" is a generated presentation node, and the two
            // transition types are the connectors between points. None of them carries text a
            // reader sees, and a "pres" point can duplicate a real one's.
            string? type = point.Attribute("type")?.Value;
            if (type is "doc" or "pres" or "parTrans" or "sibTrans") continue;

            XElement? body = point.Element(XName.Get("t", DiagramUri));
            if (DrawingTextBody.IsEmpty(body)) continue;

            DrawingTextBody.Read(body!, target, new DrawingTextOptions
            {
                ResolveHyperlink = ResolveHyperlink,
            });
        }

        return target.Children.Count > before;
    }

    private void ReadPicture(XElement picture, ContentNode target)
    {
        // r:embed names a part in the package; r:link names an external file. Only the first has
        // a part name to report, and reporting the second's URL as a part name would be a lie.
        XElement? blip = Drawing.Child(Drawing.Child(picture, "blipFill"), "blip");
        string? relationshipId = blip?.Attribute(XName.Get("embed", OoxmlNamespaces.Relationships))?.Value;

        OpcXml.Relationship? relationship = _file.Relationship(_partName, relationshipId);
        string? partName = relationship is { IsExternal: false } resolved ? resolved.Target : null;

        target.Children.Add(new ContentImage
        {
            AlternativeText = Description(picture),
            MediaType = partName is null ? null : _file.Package.GetPart(partName)?.MediaType,
            PartName = partName,
        });
    }

    /// <summary>
    /// A shape's alternative text.
    /// </summary>
    /// <remarks>
    /// <c>descr</c> is the description a screen reader reads and is the right answer when it is
    /// there. <c>title</c> is the shorter label beside it, and <c>name</c> is the author-visible
    /// object name — useful, but "Picture 4" is not alternative text, so it comes last and only
    /// when nothing better exists.
    /// </remarks>
    private static string? Description(XElement shape)
    {
        foreach (XElement child in shape.Elements())
        {
            if (Ppt.Child(child, "cNvPr") is not { } properties) continue;

            foreach (string attribute in (string[])["descr", "title", "name"])
            {
                if (Ppt.Attribute(properties, attribute) is { Length: > 0 } value) return value;
            }
        }
        return null;
    }

    private string? ResolveHyperlink(string relationshipId)
        => _file.Relationship(_partName, relationshipId)?.Target;
}
