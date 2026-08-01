using System.Xml.Linq;
using Paperless.Containers.Ooxml;
using Paperless.Ooxml;
using Paperless.Ooxml.DrawingML;

namespace Paperless.Presentations.Ooxml;

/// <summary>
/// Resolves a SmartArt diagram's parts, and turns its baked shape tree into one the ordinary
/// PresentationML walkers can read.
/// </summary>
/// <remarks>
/// <para>
/// A diagram is five parts, not four. <c>dgm:relIds</c> names <em>four</em> — <c>r:dm</c> the
/// data model, <c>r:lo</c> the layout definition, <c>r:qs</c> the quick style and <c>r:cs</c>
/// the colour transform — and the fifth, the one worth having, is not among them. PowerPoint
/// also writes <c>diagrams/drawingN.xml</c>: a <c>dsp:spTree</c> holding the diagram
/// <em>already laid out</em>, one shape per node with real geometry, fills, lines and text.
/// Reaching it means going through the data model, whose
/// <c>dgm:extLst/a:ext/dsp:dataModelExt/@relId</c> names a relationship of Microsoft's own
/// <c>…/2007/relationships/diagramDrawing</c> type.
/// </para>
/// <para>
/// <strong>That relationship is declared on the part holding the shape, not on the data part.</strong>
/// It is the data part that carries the id and the <em>slide</em> that resolves it, which is the
/// trap in this feature: <c>ppt/diagrams/_rels/data1.xml.rels</c> usually does not exist at all,
/// and where it does it holds the diagram's images under ids that collide with the slide's. In
/// <c>sd/qa/unit/data/pptx/smartart-picture-strip.pptx</c> the drawing's <c>rId1</c> is
/// <c>image1.png</c> and the slide's <c>rId1</c> is <c>slideLayout1.xml</c>, so resolving
/// against the wrong part does not fail — it silently finds something else.
/// </para>
/// <para>
/// LibreOffice does exactly this, in <c>loadDiagram</c>
/// (<c>oox/source/drawingml/diagram/diagram.cxx:608-624</c>): it collects the <c>relId</c>s the
/// data model's <c>dsp:dataModelExt</c> elements carry, resolves each against
/// <c>rRelations</c> — the relations of the part being imported — and keeps only those that
/// resolve. The layout-atom evaluator runs only when none does
/// (<c>diagram.cxx:701</c>, <c>bCreate = pShape-&gt;getExtDrawings().empty()</c>).
/// </para>
/// <para>
/// <strong>A drawing part can be present and empty</strong>, which is why LibreOffice counts
/// <c>dsp:sp</c> elements rather than trusting the relationship — <c>DiagramShapeCounter</c>,
/// <c>diagram.cxx:521-556</c>, and "Ignore ext drawings which don't actually have any shapes".
/// 15 of the 86 diagram documents in LibreOffice's own corpus have a drawing part of exactly
/// 436 bytes whose <c>dsp:spTree</c> holds nothing but its <c>dsp:nvGrpSpPr</c> — somebody
/// stripped them so the layout-atom evaluator is what gets tested. <c>smartart-org-chart.pptx</c>
/// is one, so a reader that took the part's existence as the answer would draw an eleven-node
/// organisation chart as nothing.
/// </para>
/// </remarks>
internal static class PptxDiagram
{
    /// <summary>The <c>a:graphicData</c> URI that identifies a SmartArt diagram.</summary>
    public const string Uri = "http://schemas.openxmlformats.org/drawingml/2006/diagram";

    /// <summary>
    /// Microsoft's diagram-drawing vocabulary, which <c>drawingN.xml</c> is written in.
    /// </summary>
    /// <remarks>
    /// Not an ECMA namespace and not versioned with the rest: it appeared in Office 2007 and
    /// still carries a 2008 date. Every OOXML-family document that has a baked drawing uses it.
    /// </remarks>
    public const string DrawingNamespace = "http://schemas.microsoft.com/office/drawing/2008/diagram";

    /// <summary>A diagram's baked shape tree, and the part it came from.</summary>
    /// <param name="ShapeTree">
    /// The <c>dsp:spTree</c>, renamed into PresentationML — see <see cref="Baked"/>.
    /// </param>
    /// <param name="PartName">
    /// The drawing part, which is what an <c>r:embed</c> inside the tree resolves against.
    /// </param>
    public readonly record struct BakedDrawing(XElement ShapeTree, string PartName);

    /// <summary>The diagram's data model part, which holds the author's text.</summary>
    public static XElement? DataModel(PptxFile file, string partName, XElement graphicData)
        => Part(file, partName, graphicData, "dm");

    /// <summary>One of the four parts a <c>dgm:relIds</c> names, loaded, or null.</summary>
    private static XElement? Part(
        PptxFile file, string partName, XElement graphicData, string attribute)
    {
        string? id = graphicData.Element(XName.Get("relIds", Uri))
            ?.Attribute(XName.Get(attribute, OoxmlNamespaces.Relationships))?.Value;

        return file.Relationship(partName, id) is { IsExternal: false } relationship
            ? file.Load(relationship.Target)
            : null;
    }

    /// <summary>
    /// Evaluates the layout definition, for a diagram whose baked drawing is absent or empty.
    /// </summary>
    /// <remarks>
    /// <para>
    /// This is the path LibreOffice takes when <c>getExtDrawings()</c> is empty
    /// (<c>diagram.cxx:701</c>), and it is not a rare one: Office 2007 wrote no baked drawing at
    /// all because the vocabulary's namespace is dated 2008, and 37 of the 66 diagram-bearing
    /// decks in <c>sd/qa/unit/data/pptx</c> have none — every one of which draws as nothing
    /// without this.
    /// </para>
    /// <para>
    /// The result is the same kind of tree the baked path produces, so both feed the ordinary
    /// slide layouter and a difference between the two is a difference in the evaluation rather
    /// than in the drawing. Returns null when the layout definition names an algorithm the
    /// evaluator does not implement; see <see cref="PptxDiagramEvaluator"/> for why declining
    /// beats approximating.
    /// </para>
    /// </remarks>
    /// <param name="file">The package.</param>
    /// <param name="partName">The part carrying the <c>dgm:relIds</c>, which resolves its ids.</param>
    /// <param name="graphicData">The <c>a:graphicData</c> holding the <c>dgm:relIds</c>.</param>
    /// <param name="themePart">The <c>a:theme</c> root, whose format scheme the styles index.</param>
    /// <param name="theme">The theme as a colour model.</param>
    /// <param name="width">The frame's width in EMUs.</param>
    /// <param name="height">The frame's height in EMUs.</param>
    public static BakedDrawing? Evaluated(
        PptxFile file,
        string partName,
        XElement graphicData,
        XElement? themePart,
        DrawingTheme? theme,
        int width,
        int height)
    {
        if (DataModel(file, partName, graphicData) is not { } model) return null;

        DiagramData data = DiagramData.Read(model);

        XElement? definition = Part(file, partName, graphicData, "lo");
        if (PptxDiagramEvaluator.Evaluate(data, definition, width, height) is not { } diagram)
        {
            return null;
        }

        PptxDiagramStyles styles = PptxDiagramStyles.Read(
            Part(file, partName, graphicData, "qs"),
            Part(file, partName, graphicData, "cs"),
            themePart,
            theme);

        XElement? background = model.Element(XName.Get("bg", Uri));
        if (PptxDiagramShapeTree.Build(diagram, styles, background) is not { } tree) return null;

        return new BakedDrawing(tree, partName);
    }

    /// <summary>
    /// The diagram's baked shape tree, or null when the file has none worth drawing.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The tree is returned <strong>renamed from <c>dsp:</c> into PresentationML</strong>, which
    /// sounds like a liberty and is the port. <c>dsp:sp</c>, <c>dsp:spPr</c>, <c>dsp:txBody</c>
    /// and their siblings are the <em>same elements</em> as <c>p:sp</c>, <c>p:spPr</c> and
    /// <c>p:txBody</c> under a different namespace, and everything inside them — <c>a:xfrm</c>,
    /// <c>a:prstGeom</c>, <c>a:solidFill</c>, <c>a:ln</c>, <c>a:p</c> — is already DrawingML and
    /// is left alone. LibreOffice makes the substitution in one line and then runs its ordinary
    /// slide parser over the result: <c>PPTShapeGroupContext::onCreateContext</c>,
    /// <c>oox/source/ppt/pptshapegroupcontext.cxx:60-61</c>,
    /// <c>if (getNamespace(aElementToken) == NMSP_dsp) aElementToken = NMSP_ppt |
    /// getBaseToken(aElementToken);</c>.
    /// </para>
    /// <para>
    /// Renaming buys the whole slide layouter at once — 187 preset geometries, custom geometry,
    /// gradients, bitmap fills, dashes, arrowheads and text layout — for a diagram whose shapes
    /// use all of them. Measured over the 61 drawing parts in LibreOffice's corpus, the baked
    /// tree is a <em>flat</em> list of 469 <c>dsp:sp</c> with 403 <c>a:prstGeom</c>, 66
    /// <c>a:custGeom</c>, 64 <c>a:gradFill</c> and 16 <c>a:blipFill</c> between them, and no
    /// <c>dsp:grpSp</c> or <c>dsp:pic</c> at all — a diagram's pictures arrive as blip
    /// <em>fills</em> on ordinary shapes.
    /// </para>
    /// <para>
    /// The one element with no PresentationML counterpart is <c>dsp:txXfrm</c>, the text-area
    /// rectangle, which becomes <c>p:txXfrm</c> and is read by name where the text rectangle is
    /// decided.
    /// </para>
    /// </remarks>
    public static BakedDrawing? Baked(PptxFile file, string partName, XElement graphicData)
    {
        if (DataModel(file, partName, graphicData) is not { } model) return null;

        foreach (XElement extension in model
                     .Descendants(XName.Get("dataModelExt", DrawingNamespace)))
        {
            string? id = extension.Attribute("relId")?.Value;
            if (file.Relationship(partName, id) is not { IsExternal: false } relationship) continue;
            if (file.Load(relationship.Target) is not { } drawing) continue;

            XElement? tree = drawing.Element(XName.Get("spTree", DrawingNamespace));
            if (tree is null) continue;

            // LibreOffice's DiagramShapeCounter: a drawing with no dsp:sp is not a fallback, and
            // treating it as one draws the diagram as nothing rather than falling through.
            if (!tree.Descendants(XName.Get("sp", DrawingNamespace)).Any()) continue;

            return new BakedDrawing(Rename(tree), relationship.Target);
        }

        return null;
    }

    /// <summary>
    /// Copies a subtree, renaming every <c>dsp:</c> element into PresentationML and leaving
    /// everything else — which is all DrawingML — exactly as it was.
    /// </summary>
    /// <remarks>
    /// A copy rather than a rename in place because <see cref="PptxFile.Load"/> caches parts and
    /// hands the same tree to whoever asks next; mutating it would make the extraction walk and
    /// the layout walk disagree about what namespace the file is in depending on their order.
    /// </remarks>
    private static XElement Rename(XElement element)
    {
        if (element.Name.NamespaceName != DrawingNamespace) return new XElement(element);

        return new XElement(
            Ppt.Name(element.Name.LocalName),
            element.Attributes().Where(a => !a.IsNamespaceDeclaration),
            element.Elements().Select(Rename));
    }
}
