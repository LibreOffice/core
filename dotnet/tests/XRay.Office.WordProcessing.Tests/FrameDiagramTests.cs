using System.Xml.Linq;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Units;
using XRay.Office.Ooxml.DrawingML;
using XRay.Office.WordProcessing.Layout;
using XRay.Office.WordProcessing.Ooxml;
using Shouldly;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A SmartArt diagram anchored in a document draws its nodes, which until now it did not.
/// </summary>
/// <remarks>
/// <para>
/// Diagram support has been thorough since the slides track needed it — the parts, the baked
/// <c>dsp:spTree</c>, and a layout-atom evaluator for files that carry none — and every line of it
/// was reachable only from a deck. Three corpus documents carry a diagram and all three drew an
/// empty frame; on <c>024_Unit_Circle_Chart_Colorful_Circles</c> the whole deficit against both
/// reference binaries was the five nodes' <c>YOUR TEXT</c>, ten words and forty characters.
/// </para>
/// <para>
/// What these assert is the <em>mechanism</em>, not a glyph count on one document: that the five
/// parts resolve against the part that names them, that the baked drawing is preferred and an
/// emptied one is not treated as an answer, that each node becomes one shape at the offset the
/// drawing states, that the frame is the diagram's own child space rather than something to
/// stretch it to, and that a node's text survives the hop into WordprocessingML with the size,
/// colour and spacing it was stated with.
/// </para>
/// </remarks>
public sealed class FrameDiagramTests
{
    /// <summary>Each baked node becomes one placeable shape, at the offset the drawing states.</summary>
    /// <remarks>
    /// Through <see cref="DocxFrames"/> rather than stopping at the translation, because the claim
    /// worth pinning is that the shapes are <em>placed</em>: the first frame is the group's own
    /// envelope, which carries the anchor's wrap, and each member carries its own rectangle in the
    /// group's coordinates.
    /// </remarks>
    [Fact]
    public void EachNodeBecomesOneShapeAtTheOffsetTheDrawingStates()
    {
        IReadOnlyList<PageFrame> frames = Frames();

        // The envelope plus one per node.
        frames.Count.ShouldBe(3);

        frames[1].GroupOffset.X.ShouldBe(Length.FromEmu(0));
        frames[1].GroupOffset.Y.ShouldBe(Length.FromEmu(228600));
        frames[1].Size.ShouldBe(new DocSize(Length.FromEmu(1371600), Length.FromEmu(1371600)));

        frames[2].GroupOffset.X.ShouldBe(Length.FromEmu(2286000));
        frames[2].GroupOffset.Y.ShouldBe(Length.FromEmu(228600));
        frames[2].Size.ShouldBe(new DocSize(Length.FromEmu(1371600), Length.FromEmu(1371600)));
    }

    /// <summary>
    /// The child space is the frame, so a diagram that does not fill its frame is not stretched
    /// to.
    /// </summary>
    /// <remarks>
    /// <para>
    /// This is the one decision in the hop that could not be taken by analogy. <c>DocxFrames</c>
    /// resizes a <c>wpg:wgp</c> whose members do not cover their child space — measured behaviour,
    /// and right for a group Word wrote — and a diagram's baked shapes are stated in the frame's
    /// coordinates instead, which is
    /// <c>pParentShape-&gt;setChildSize(pParentShape-&gt;getSize())</c> in
    /// <c>oox/source/drawingml/diagram/diagram.cxx</c>. So the synthesised container is a canvas
    /// and not a group.
    /// </para>
    /// <para>
    /// The nodes here cover 3 657 600 × 1 828 800 of a 4 572 000 × 2 743 200 frame, so a refit
    /// would scale them by 1.25 across and 1.5 down and nothing about the numbers would look
    /// wrong.
    /// </para>
    /// </remarks>
    [Fact]
    public void TheChildSpaceIsTheFrameSoTheDiagramIsNotStretchedToFillIt()
    {
        IReadOnlyList<PageFrame> frames = Frames(
            frameWidth: 4572000, frameHeight: 2743200);

        frames[1].Size.Width.ShouldBe(Length.FromEmu(1371600));
        frames[1].Size.Height.ShouldBe(Length.FromEmu(1371600));
        frames[2].GroupOffset.X.ShouldBe(Length.FromEmu(2286000));
    }

    /// <summary>A node's text arrives as laid-out paragraphs rather than being lost.</summary>
    [Fact]
    public void ANodesTextIsCarriedIntoTheShape()
    {
        XElement canvas = Translated();

        canvas.Descendants(W + "t").Select(text => text.Value)
            .ShouldBe(["First node", "Second node"]);
    }

    /// <summary>
    /// A run keeps the size the diagram states, in the half-points Word measures one in.
    /// </summary>
    [Fact]
    public void TheRunKeepsTheSizeTheDiagramStates() =>
        Translated().Descendants(W + "sz").First().Attribute(W + "val")!.Value
            .ShouldBe("28");

    /// <summary>
    /// A run stating no colour takes the shape's <c>a:fontRef</c>, resolved against the theme.
    /// </summary>
    /// <remarks>
    /// The usual case, and the one that decides whether a node is legible: a SmartArt run carries
    /// a size and nothing else, and the <c>a:fontRef</c> beside it is what makes the text white on
    /// a coloured node. Without this it would be drawn in the document default's black.
    /// </remarks>
    [Fact]
    public void ARunWithNoColourTakesTheShapeStyleFontReference() =>
        Translated().Descendants(W + "color").First().Attribute(W + "val")!.Value
            .ShouldBe("FFFFFF");

    /// <summary>
    /// The paragraph states its own spacing and alignment, so it cannot inherit the document's.
    /// </summary>
    /// <remarks>
    /// Word writes 8 pt after and 1.08 line spacing into <c>docDefaults</c> for nearly every file,
    /// and a diagram node is a fixed shape with two words in it — inheriting a paragraph gap
    /// pushes the text out of the node. <c>a:spcAft</c> is a percentage of the run's own size, so
    /// 35 % of 14 pt is 4.9 pt and Word's twips are 98; <c>a:lnSpc</c> of 90 % is 90 % of Word's
    /// own 240.
    /// </remarks>
    [Fact]
    public void TheParagraphStatesItsOwnSpacingAndAlignment()
    {
        XElement spacing = Translated().Descendants(W + "spacing").First();

        spacing.Attribute(W + "before")!.Value.ShouldBe("0");
        spacing.Attribute(W + "after")!.Value.ShouldBe("98");
        spacing.Attribute(W + "line")!.Value.ShouldBe("216");
        spacing.Attribute(W + "lineRule")!.Value.ShouldBe("auto");

        Translated().Descendants(W + "jc").First().Attribute(W + "val")!.Value.ShouldBe("center");
    }

    /// <summary>
    /// A relationship is resolved against the part that states it, not against the main document.
    /// </summary>
    /// <remarks>
    /// The silent trap of the whole feature. <c>rId5</c> in <c>word/document.xml</c> and
    /// <c>rId5</c> in <c>word/header1.xml</c> are different relationships in different
    /// <c>.rels</c> files, and Word numbers both from one — so a resolver that ignored the owning
    /// part would answer a header's diagram with whatever the document calls <c>rId5</c>, which
    /// does not fail, it finds something else.
    /// </remarks>
    [Fact]
    public void TheRelationshipIsResolvedAgainstThePartThatStatesIt()
    {
        Package package = new();
        package.Relate("word/document.xml", "rId5", "word/diagrams/data1.xml");
        package.Relate("word/header1.xml", "rId5", "word/diagrams/data2.xml");
        package.Add("word/diagrams/data1.xml", DataModel("rId9"));
        package.Add("word/diagrams/data2.xml", DataModel("rId9"));
        package.Relate("word/document.xml", "rId9", "word/diagrams/drawing1.xml");
        package.Relate("word/header1.xml", "rId9", "word/diagrams/drawing2.xml");
        package.Add("word/diagrams/drawing1.xml", Drawing(Node(0, "In the body")));
        package.Add("word/diagrams/drawing2.xml", Drawing(Node(0, "In the header")));

        Text(package, "word/document.xml").ShouldBe(["In the body"]);
        Text(package, "word/header1.xml").ShouldBe(["In the header"]);
    }

    /// <summary>
    /// A drawing part with no <c>dsp:sp</c> in it is not an answer, and the layout definition is
    /// asked for instead.
    /// </summary>
    /// <remarks>
    /// LibreOffice counts the shapes rather than trusting the relationship —
    /// <c>DiagramShapeCounter</c>, and "Ignore ext drawings which don't actually have any shapes"
    /// — because a stripped drawing part is common enough in the wild to matter: 15 of the 86
    /// diagram documents in LibreOffice's own corpus have one of exactly 436 bytes holding
    /// nothing but its <c>dsp:nvGrpSpPr</c>. Taking the part's existence as the answer draws an
    /// eleven-node organisation chart as nothing.
    /// </remarks>
    [Fact]
    public void AnEmptiedDrawingPartFallsThroughToTheLayoutDefinition()
    {
        Package package = Built();
        package.Add(
            "word/diagrams/drawing1.xml",
            XElement.Parse(
                $"""
                 <dsp:drawing xmlns:dsp="{Dsp}"><dsp:spTree>
                   <dsp:nvGrpSpPr><dsp:cNvPr id="0" name=""/><dsp:cNvGrpSpPr/></dsp:nvGrpSpPr>
                   <dsp:grpSpPr/>
                 </dsp:spTree></dsp:drawing>
                 """));

        DocxDiagram.Read(GraphicData, Frame, package.Source, "word/document.xml", null, null)
            .ShouldBeNull();

        // Not "it drew nothing" but "it went on looking": the layout definition is what the
        // evaluator would run, and it was asked for.
        package.Loaded.ShouldContain("word/diagrams/layout1.xml");
    }

    /// <summary>A diagram whose data model will not resolve leaves the frame as it was.</summary>
    /// <remarks>
    /// Declining rather than approximating, which is the same decision the evaluator takes for an
    /// algorithm it does not implement. An empty frame is visibly wrong; a diagram drawn from
    /// half a model is confidently wrong.
    /// </remarks>
    [Fact]
    public void AnUnresolvableDataModelDrawsNothingRatherThanSomething()
    {
        Package package = new();

        DocxDiagram.Read(GraphicData, Frame, package.Source, "word/document.xml", null, null)
            .ShouldBeNull();
    }

    /// <summary>The parts resolve out of a real package, through the document's own scope.</summary>
    /// <remarks>
    /// The other tests hand the resolution a dictionary, which proves the reading and not the
    /// reach. This one opens a package built in memory — the relationships, the content types and
    /// the six parts — and asks the reader that a rendering run would ask, so it fails if the hop
    /// from <see cref="DocxPictures"/> to the diagram is ever unwired again.
    /// </remarks>
    [Fact]
    public void TheDiagramIsReachedFromAnActualPackage()
    {
        using MemoryStream stream = new(DocxDiagramPackage.Bytes());
        using DocxFile file = DocxFile.Open(stream);

        XElement anchor = file.Document
            .Descendants(XName.Get("anchor", Wp)).ShouldHaveSingleItem();

        XElement group = new DocxPictures(file, null)
            .Diagram(anchor, Frame, file.Theme)
            .ShouldNotBeNull();

        group.Descendants(W + "t").Select(text => text.Value)
            .ShouldBe(["First node", "Second node"]);
    }

    /// <summary>
    /// The anchored diagram becomes frames through the ordinary drawing reader, end to end.
    /// </summary>
    /// <remarks>
    /// The whole chain in one assertion — package, relationships scoped to the main document, the
    /// baked drawing, the translation and the placement — because every other test here holds one
    /// end of it. Before the wiring existed this returned a single empty frame, which is exactly
    /// what the three corpus documents drew.
    /// </remarks>
    [Fact]
    public void TheAnchoredDiagramBecomesFramesThroughTheOrdinaryDrawingReader()
    {
        using MemoryStream stream = new(DocxDiagramPackage.Bytes());
        using DocxFile file = DocxFile.Open(stream);

        XElement drawing = file.Document
            .Descendants(W + "drawing").ShouldHaveSingleItem();

        IReadOnlyList<PageFrame> frames = DocxFrames.ReadAll(
            drawing,
            content: null,
            anchorOffset: 0,
            new DocxPictures(file, null),
            new DocxFrameContext(file.Theme, CompatibilityMode: 15));

        // The envelope, which keeps the anchor's wrap, plus one frame per node.
        frames.Count.ShouldBe(3);
        frames[1].GroupOffset.X.ShouldBe(Length.FromEmu(0));
        frames[2].GroupOffset.X.ShouldBe(Length.FromEmu(2286000));
        frames[2].Size.Width.ShouldBe(Length.FromEmu(1371600));
    }

    /// <summary>The two nodes' text, read through a package whose parts are given.</summary>
    private static IReadOnlyList<string> Text(Package package, string partName)
        => [.. DocxDiagram
            .Read(GraphicData, Frame, package.Source, partName, null, null)!
            .Descendants(W + "t")
            .Select(element => element.Value)];

    /// <summary>The translated canvas for the standard two-node fixture.</summary>
    private static XElement Translated()
        => DocxDiagram
            .Read(GraphicData, Frame, Built().Source, "word/document.xml", null, Theme)
            .ShouldNotBeNull();

    /// <summary>The frames a drawing holding the standard fixture produces.</summary>
    private static IReadOnlyList<PageFrame> Frames(
        long frameWidth = 3657600, long frameHeight = 1828800)
    {
        XElement canvas = DocxDiagram.Read(
            GraphicData,
            new DocSize(Length.FromEmu(frameWidth), Length.FromEmu(frameHeight)),
            Built().Source,
            "word/document.xml",
            null,
            Theme)!;

        XElement drawing = XElement.Parse(
            $"""
             <w:drawing xmlns:w="{Wns}" xmlns:wp="{Wp}" xmlns:a="{Ans}">
               <wp:anchor>
                 <wp:extent cx="{frameWidth}" cy="{frameHeight}"/>
                 <wp:wrapSquare wrapText="bothSides"/>
                 <a:graphic><a:graphicData/></a:graphic>
               </wp:anchor>
             </w:drawing>
             """);

        drawing.Descendants(XName.Get("graphicData", Ans)).Single().Add(canvas);

        return DocxFrames.ReadAll(drawing, content: null, anchorOffset: 0);
    }

    /// <summary>The standard fixture: a data model naming a baked drawing of two nodes.</summary>
    private static Package Built()
    {
        Package package = new();
        package.Relate("word/document.xml", "rId5", "word/diagrams/data1.xml");
        package.Relate("word/document.xml", "rId6", "word/diagrams/layout1.xml");
        package.Relate("word/document.xml", "rId9", "word/diagrams/drawing1.xml");
        package.Add("word/diagrams/data1.xml", DataModel("rId9"));
        package.Add("word/diagrams/layout1.xml", XElement.Parse($"""<dgm:layoutDef xmlns:dgm="{Dgm}"/>"""));
        package.Add(
            "word/diagrams/drawing1.xml",
            Drawing(Node(0, "First node"), Node(2286000, "Second node")));

        return package;
    }

    /// <summary>A data model whose extension names a drawing part by relationship.</summary>
    private static XElement DataModel(string relationshipId) => XElement.Parse(
        $"""
         <dgm:dataModel xmlns:dgm="{Dgm}" xmlns:a="{Ans}">
           <dgm:ptLst/><dgm:cxnLst/><dgm:bg/><dgm:whole/>
           <dgm:extLst><a:ext uri="{Dsp}">
             <dsp:dataModelExt xmlns:dsp="{Dsp}" relId="{relationshipId}"/>
           </a:ext></dgm:extLst>
         </dgm:dataModel>
         """);

    /// <summary>A baked drawing holding the given shapes.</summary>
    private static XElement Drawing(params string[] shapes) => XElement.Parse(
        $"""
         <dsp:drawing xmlns:dsp="{Dsp}" xmlns:a="{Ans}"><dsp:spTree>
           <dsp:nvGrpSpPr><dsp:cNvPr id="0" name=""/><dsp:cNvGrpSpPr/></dsp:nvGrpSpPr>
           <dsp:grpSpPr/>
           {string.Concat(shapes)}
         </dsp:spTree></dsp:drawing>
         """);

    /// <summary>One baked node: an ellipse with centred 14 pt text and a themed font reference.</summary>
    private static string Node(long x, string text) =>
        $"""
         <dsp:sp modelId="node{x}">
           <dsp:nvSpPr><dsp:cNvPr id="0" name="{text}"/><dsp:cNvSpPr/></dsp:nvSpPr>
           <dsp:spPr>
             <a:xfrm><a:off x="{x}" y="228600"/><a:ext cx="1371600" cy="1371600"/></a:xfrm>
             <a:prstGeom prst="ellipse"><a:avLst/></a:prstGeom>
             <a:solidFill><a:srgbClr val="4472C4"/></a:solidFill>
           </dsp:spPr>
           <dsp:style>
             <a:lnRef idx="0"><a:scrgbClr r="0" g="0" b="0"/></a:lnRef>
             <a:fillRef idx="1"><a:scrgbClr r="0" g="0" b="0"/></a:fillRef>
             <a:effectRef idx="0"><a:scrgbClr r="0" g="0" b="0"/></a:effectRef>
             <a:fontRef idx="minor"><a:schemeClr val="lt1"/></a:fontRef>
           </dsp:style>
           <dsp:txBody>
             <a:bodyPr lIns="45720" tIns="22860" rIns="45720" bIns="22860" anchor="ctr">
               <a:noAutofit/>
             </a:bodyPr>
             <a:lstStyle/>
             <a:p>
               <a:pPr marL="0" lvl="0" indent="0" algn="ctr">
                 <a:lnSpc><a:spcPct val="90000"/></a:lnSpc>
                 <a:spcBef><a:spcPct val="0"/></a:spcBef>
                 <a:spcAft><a:spcPct val="35000"/></a:spcAft>
                 <a:buNone/>
               </a:pPr>
               <a:r><a:rPr lang="en-GB" sz="1400" kern="1200"/><a:t>{text}</a:t></a:r>
             </a:p>
           </dsp:txBody>
           <dsp:txXfrm><a:off x="{x}" y="228600"/><a:ext cx="1371600" cy="1371600"/></dsp:txXfrm>
         </dsp:sp>
         """;

    /// <summary>An in-memory package: parts by name, and relationships by owning part.</summary>
    /// <remarks>
    /// A dictionary rather than a real OPC package because what these tests are about is the two
    /// lookups a diagram needs, and <see cref="DiagramPartSource"/> is exactly those two — so a
    /// fake that answers them exercises the whole resolution and records what was asked for.
    /// </remarks>
    private sealed class Package
    {
        private readonly Dictionary<(string Part, string Id), string> _relationships = [];
        private readonly Dictionary<string, XElement> _parts = new(StringComparer.Ordinal);
        private readonly List<string> _loaded = [];

        /// <summary>The parts this package was actually asked for, in order.</summary>
        public IReadOnlyList<string> Loaded => _loaded;

        /// <summary>The package as the two lookups a diagram's parts need.</summary>
        public DiagramPartSource Source => new(
            (part, id) => _relationships.GetValueOrDefault((part, id)),
            name =>
            {
                _loaded.Add(name);
                return _parts.GetValueOrDefault(name);
            });

        /// <summary>Declares a relationship on one part.</summary>
        public void Relate(string partName, string relationshipId, string target)
            => _relationships[(partName, relationshipId)] = target;

        /// <summary>Adds or replaces a part.</summary>
        public void Add(string partName, XElement root) => _parts[partName] = root;
    }

    /// <summary>The <c>a:graphicData</c> a diagram-bearing frame carries.</summary>
    private static XElement GraphicData => XElement.Parse(
        $"""
         <a:graphicData xmlns:a="{Ans}" uri="{Dgm}">
           <dgm:relIds xmlns:dgm="{Dgm}" xmlns:r="{Rel}"
                       r:dm="rId5" r:lo="rId6" r:qs="rId7" r:cs="rId8"/>
         </a:graphicData>
         """);

    private static DocSize Frame
        => new(Length.FromEmu(3657600), Length.FromEmu(1828800));

    /// <summary>The stock Office theme, cut to the one colour a node's font reference names.</summary>
    private static readonly DrawingTheme? Theme = DrawingTheme.Read(XElement.Parse(
        $"""
         <a:theme xmlns:a="{Ans}">
           <a:themeElements>
             <a:clrScheme name="Office">
               <a:lt1><a:sysClr lastClr="FFFFFF" val="window"/></a:lt1>
               <a:dk1><a:sysClr lastClr="000000" val="windowText"/></a:dk1>
               <a:accent1><a:srgbClr val="4472C4"/></a:accent1>
             </a:clrScheme>
             <a:fontScheme name="Office">
               <a:majorFont><a:latin typeface="Calibri Light"/></a:majorFont>
               <a:minorFont><a:latin typeface="Calibri"/></a:minorFont>
             </a:fontScheme>
           </a:themeElements>
         </a:theme>
         """));

    private const string Wns = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
    private const string Wp = "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing";
    private const string Ans = "http://schemas.openxmlformats.org/drawingml/2006/main";
    private const string Dgm = "http://schemas.openxmlformats.org/drawingml/2006/diagram";
    private const string Dsp = "http://schemas.microsoft.com/office/drawing/2008/diagram";
    private const string Rel = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";

    private static readonly XNamespace W = Wns;
}
