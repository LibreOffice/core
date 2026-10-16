using System.IO.Compression;
using System.Text;

namespace XRay.Office.WordProcessing.Tests;

/// <summary>
/// A minimal DOCX carrying one anchored SmartArt diagram with a baked drawing, built in memory.
/// </summary>
/// <remarks>
/// <para>
/// Built rather than committed because the point of it is the <em>package</em> — the content
/// types, the two <c>.rels</c> parts and the five diagram parts a <c>dgm:relIds</c> reaches
/// through them — and a reader can see all of that here in one screen instead of having to unzip a
/// fixture to find out what it is asserting. It is also the smallest thing that can fail if the
/// hop from a document to its diagram is ever unwired: nothing else in the suite opens a package
/// for one.
/// </para>
/// <para>
/// The parts are deliberately the least each reader will accept, not a reduction of a real file:
/// two ellipses in the drawing, an empty <c>dgm:ptLst</c> in the data model (the baked path takes
/// its text from the drawing, not from the model), and a <c>dgm:layoutDef</c> that exists only so
/// the <c>r:lo</c> relationship resolves.
/// </para>
/// </remarks>
internal static class DocxDiagramPackage
{
    private const string Rel = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";
    private const string A = "http://schemas.openxmlformats.org/drawingml/2006/main";
    private const string W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main";
    private const string Wp = "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing";
    private const string Dgm = "http://schemas.openxmlformats.org/drawingml/2006/diagram";
    private const string Dsp = "http://schemas.microsoft.com/office/drawing/2008/diagram";

    /// <summary>The package as bytes, ready to open over a <see cref="MemoryStream"/>.</summary>
    public static byte[] Bytes()
    {
        using MemoryStream stream = new();

        using (ZipArchive archive = new(stream, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml", ContentTypes);
            Write(archive, "_rels/.rels", RootRelationships);
            Write(archive, "word/document.xml", Document);
            Write(archive, "word/_rels/document.xml.rels", DocumentRelationships);
            Write(archive, "word/theme/theme1.xml", Theme);
            Write(archive, "word/diagrams/data1.xml", DataModel);
            Write(archive, "word/diagrams/layout1.xml", Layout);
            Write(archive, "word/diagrams/drawing1.xml", Drawing);
        }

        return stream.ToArray();
    }

    private static void Write(ZipArchive archive, string name, string content)
    {
        using Stream entry = archive.CreateEntry(name).Open();
        entry.Write(Encoding.UTF8.GetBytes(content));
    }

    private const string ContentTypes =
        """
        <?xml version="1.0" encoding="UTF-8" standalone="yes"?>
        <Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
          <Default Extension="rels"
                   ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
          <Default Extension="xml" ContentType="application/xml"/>
          <Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
        </Types>
        """;

    private static readonly string RootRelationships =
        $"""
         <?xml version="1.0" encoding="UTF-8" standalone="yes"?>
         <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
           <Relationship Id="rId1" Type="{Rel}/officeDocument" Target="word/document.xml"/>
         </Relationships>
         """;

    private static readonly string DocumentRelationships =
        $"""
         <?xml version="1.0" encoding="UTF-8" standalone="yes"?>
         <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
           <Relationship Id="rId1" Type="{Rel}/theme" Target="theme/theme1.xml"/>
           <Relationship Id="rId5" Type="{Rel}/diagramData" Target="diagrams/data1.xml"/>
           <Relationship Id="rId6" Type="{Rel}/diagramLayout" Target="diagrams/layout1.xml"/>
           <Relationship Id="rId9" Type="http://schemas.microsoft.com/office/2007/relationships/diagramDrawing" Target="diagrams/drawing1.xml"/>
         </Relationships>
         """;

    private static readonly string Document =
        $"""
         <?xml version="1.0" encoding="UTF-8" standalone="yes"?>
         <w:document xmlns:w="{W}" xmlns:r="{Rel}" xmlns:wp="{Wp}" xmlns:a="{A}">
           <w:body>
             <w:p><w:r><w:t>Before the diagram.</w:t></w:r></w:p>
             <w:p><w:r><w:drawing>
               <wp:anchor distT="0" distB="0" distL="114300" distR="114300" simplePos="0"
                          relativeHeight="251659264" behindDoc="0" locked="0" layoutInCell="1"
                          allowOverlap="1">
                 <wp:simplePos x="0" y="0"/>
                 <wp:positionH relativeFrom="column"><wp:posOffset>0</wp:posOffset></wp:positionH>
                 <wp:positionV relativeFrom="paragraph"><wp:posOffset>0</wp:posOffset></wp:positionV>
                 <wp:extent cx="3657600" cy="1828800"/>
                 <wp:wrapSquare wrapText="bothSides"/>
                 <wp:docPr id="1" name="Diagram 1"/>
                 <a:graphic><a:graphicData uri="{Dgm}">
                   <dgm:relIds xmlns:dgm="{Dgm}" r:dm="rId5" r:lo="rId6"/>
                 </a:graphicData></a:graphic>
               </wp:anchor>
             </w:drawing></w:r></w:p>
             <w:sectPr>
               <w:pgSz w:w="11906" w:h="16838"/>
               <w:pgMar w:top="1134" w:right="1134" w:bottom="1134" w:left="1134"/>
             </w:sectPr>
           </w:body>
         </w:document>
         """;

    private static readonly string Theme =
        $"""
         <?xml version="1.0" encoding="UTF-8" standalone="yes"?>
         <a:theme xmlns:a="{A}" name="Test">
           <a:themeElements>
             <a:clrScheme name="Test">
               <a:dk1><a:sysClr val="windowText" lastClr="000000"/></a:dk1>
               <a:lt1><a:sysClr val="window" lastClr="FFFFFF"/></a:lt1>
               <a:accent1><a:srgbClr val="4472C4"/></a:accent1>
             </a:clrScheme>
             <a:fontScheme name="Test">
               <a:majorFont><a:latin typeface="Calibri Light"/></a:majorFont>
               <a:minorFont><a:latin typeface="Calibri"/></a:minorFont>
             </a:fontScheme>
             <a:fmtScheme name="Test">
               <a:fillStyleLst><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:fillStyleLst>
               <a:lnStyleLst><a:ln w="6350"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln></a:lnStyleLst>
             </a:fmtScheme>
           </a:themeElements>
         </a:theme>
         """;

    private static readonly string DataModel =
        $"""
         <?xml version="1.0" encoding="UTF-8" standalone="yes"?>
         <dgm:dataModel xmlns:dgm="{Dgm}" xmlns:a="{A}">
           <dgm:ptLst>
             <dgm:pt modelId="doc" type="doc"><dgm:t><a:bodyPr/><a:lstStyle/><a:p><a:endParaRPr/></a:p></dgm:t></dgm:pt>
             {Point("n0", null, "First node")}
             {Point("n1", null, "Second node")}
             {Point("n2", null, "Third node")}
             {Point("p0", "pres", "Layout generated")}
             {Point("s0", "sibTrans", "Connector")}
           </dgm:ptLst>
           <dgm:cxnLst/><dgm:bg/><dgm:whole/>
           <dgm:extLst><a:ext uri="{Dsp}">
             <dsp:dataModelExt xmlns:dsp="{Dsp}" relId="rId9"/>
           </a:ext></dgm:extLst>
         </dgm:dataModel>
         """;

    /// <summary>One <c>dgm:pt</c>, with the author's text in its <c>dgm:t</c> body.</summary>
    /// <remarks>
    /// <c>Third node</c> exists in the model and <em>not</em> in the baked drawing, and
    /// <c>Layout generated</c> and <c>Connector</c> exist on point types no reader sees. Between
    /// them they make the two possible sources — the data model and the baked shape tree —
    /// distinguishable by their output rather than by inspection.
    /// </remarks>
    private static string Point(string id, string? type, string text) =>
        $"""
         <dgm:pt modelId="{id}"{(type is null ? "" : $" type=\"{type}\"")}>
           <dgm:t><a:bodyPr/><a:lstStyle/>
             <a:p><a:r><a:rPr lang="en-GB"/><a:t>{text}</a:t></a:r></a:p>
           </dgm:t>
         </dgm:pt>
         """;

    private static readonly string Layout =
        $"""
         <?xml version="1.0" encoding="UTF-8" standalone="yes"?>
         <dgm:layoutDef xmlns:dgm="{Dgm}" uniqueId="urn:test"/>
         """;

    private static readonly string Drawing =
        $"""
         <?xml version="1.0" encoding="UTF-8" standalone="yes"?>
         <dsp:drawing xmlns:dsp="{Dsp}" xmlns:a="{A}"><dsp:spTree>
           <dsp:nvGrpSpPr><dsp:cNvPr id="0" name=""/><dsp:cNvGrpSpPr/></dsp:nvGrpSpPr>
           <dsp:grpSpPr/>
           {Node(0, "First node")}
           {Node(2286000, "Second node")}
         </dsp:spTree></dsp:drawing>
         """;

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
             <a:bodyPr anchor="ctr"><a:noAutofit/></a:bodyPr>
             <a:lstStyle/>
             <a:p>
               <a:pPr algn="ctr"/>
               <a:r><a:rPr lang="en-GB" sz="1400"/><a:t>{text}</a:t></a:r>
             </a:p>
           </dsp:txBody>
           <dsp:txXfrm><a:off x="{x}" y="228600"/><a:ext cx="1371600" cy="1371600"/></dsp:txXfrm>
         </dsp:sp>
         """;
}
