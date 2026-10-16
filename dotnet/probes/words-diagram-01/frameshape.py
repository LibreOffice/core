import zipfile, os, sys
A="http://schemas.openxmlformats.org/drawingml/2006/main"
DSP="http://schemas.microsoft.com/office/drawing/2008/diagram"
DGM="http://schemas.openxmlformats.org/drawingml/2006/diagram"
R="http://schemas.openxmlformats.org/officeDocument/2006/relationships"
W="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
WP="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"


def node(x, y, D, name):
    return f'''<dsp:sp modelId="{name}"><dsp:nvSpPr><dsp:cNvPr id="0" name="{name}"/><dsp:cNvSpPr/></dsp:nvSpPr>
<dsp:spPr><a:xfrm><a:off x="{x}" y="{y}"/><a:ext cx="{D}" cy="{D}"/></a:xfrm>
<a:prstGeom prst="ellipse"><a:avLst/></a:prstGeom>
<a:solidFill><a:srgbClr val="4472C4"/></a:solidFill></dsp:spPr>
<dsp:style><a:lnRef idx="0"><a:scrgbClr r="0" g="0" b="0"/></a:lnRef><a:fillRef idx="1"><a:scrgbClr r="0" g="0" b="0"/></a:fillRef><a:effectRef idx="0"><a:scrgbClr r="0" g="0" b="0"/></a:effectRef><a:fontRef idx="minor"><a:schemeClr val="lt1"/></a:fontRef></dsp:style>
<dsp:txBody><a:bodyPr anchor="ctr"><a:noAutofit/></a:bodyPr><a:lstStyle/><a:p><a:pPr algn="ctr"/><a:r><a:rPr lang="en-GB" sz="1200"/><a:t>{name}</a:t></a:r></a:p></dsp:txBody>
<dsp:txXfrm><a:off x="{x}" y="{y}"/><a:ext cx="{D}" cy="{D}"/></dsp:txXfrm></dsp:sp>'''

def build(path, eff, FW, FH, D, pts):
    l,t,r,b = eff
    parts = {
"[Content_Types].xml": f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="xml" ContentType="application/xml"/>
<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
<Override PartName="/word/theme/theme1.xml" ContentType="application/vnd.openxmlformats-officedocument.theme+xml"/>
<Override PartName="/word/diagrams/data1.xml" ContentType="application/vnd.openxmlformats-officedocument.drawingml.diagramData+xml"/>
<Override PartName="/word/diagrams/layout1.xml" ContentType="application/vnd.openxmlformats-officedocument.drawingml.diagramLayout+xml"/>
<Override PartName="/word/diagrams/drawing1.xml" ContentType="application/vnd.ms-office.drawingml.diagramDrawing+xml"/>
</Types>''',
"_rels/.rels": f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="{R}/officeDocument" Target="word/document.xml"/></Relationships>''',
"word/_rels/document.xml.rels": f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rId1" Type="{R}/theme" Target="theme/theme1.xml"/>
<Relationship Id="rId5" Type="{R}/diagramData" Target="diagrams/data1.xml"/>
<Relationship Id="rId6" Type="{R}/diagramLayout" Target="diagrams/layout1.xml"/>
<Relationship Id="rId9" Type="http://schemas.microsoft.com/office/2007/relationships/diagramDrawing" Target="diagrams/drawing1.xml"/>
</Relationships>''',
"word/document.xml": f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="{W}" xmlns:r="{R}" xmlns:wp="{WP}" xmlns:a="{A}"><w:body>
<w:p><w:r><w:drawing><wp:anchor distT="0" distB="0" distL="0" distR="0" simplePos="0" relativeHeight="251659264" behindDoc="0" locked="0" layoutInCell="1" allowOverlap="1">
<wp:simplePos x="0" y="0"/>
<wp:positionH relativeFrom="column"><wp:posOffset>0</wp:posOffset></wp:positionH>
<wp:positionV relativeFrom="paragraph"><wp:posOffset>0</wp:posOffset></wp:positionV>
<wp:extent cx="{FW}" cy="{FH}"/>
<wp:effectExtent l="{l}" t="{t}" r="{r}" b="{b}"/>
<wp:wrapNone/><wp:docPr id="1" name="Diagram 1"/>
<a:graphic><a:graphicData uri="{DGM}"><dgm:relIds xmlns:dgm="{DGM}" r:dm="rId5" r:lo="rId6"/></a:graphicData></a:graphic>
</wp:anchor></w:drawing></w:r></w:p>
<w:sectPr><w:pgSz w:w="11906" w:h="16838"/><w:pgMar w:top="720" w:right="720" w:bottom="720" w:left="720"/></w:sectPr>
</w:body></w:document>''',
"word/theme/theme1.xml": f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<a:theme xmlns:a="{A}" name="T"><a:themeElements>
<a:clrScheme name="T"><a:dk1><a:sysClr val="windowText" lastClr="000000"/></a:dk1><a:lt1><a:sysClr val="window" lastClr="FFFFFF"/></a:lt1><a:dk2><a:srgbClr val="44546A"/></a:dk2><a:lt2><a:srgbClr val="E7E6E6"/></a:lt2><a:accent1><a:srgbClr val="4472C4"/></a:accent1><a:accent2><a:srgbClr val="ED7D31"/></a:accent2><a:accent3><a:srgbClr val="A5A5A5"/></a:accent3><a:accent4><a:srgbClr val="FFC000"/></a:accent4><a:accent5><a:srgbClr val="5B9BD5"/></a:accent5><a:accent6><a:srgbClr val="70AD47"/></a:accent6><a:hlink><a:srgbClr val="0563C1"/></a:hlink><a:folHlink><a:srgbClr val="954F72"/></a:folHlink></a:clrScheme>
<a:fontScheme name="T"><a:majorFont><a:latin typeface="Calibri Light"/><a:ea typeface=""/><a:cs typeface=""/></a:majorFont><a:minorFont><a:latin typeface="Calibri"/><a:ea typeface=""/><a:cs typeface=""/></a:minorFont></a:fontScheme>
<a:fmtScheme name="T"><a:fillStyleLst><a:solidFill><a:schemeClr val="phClr"/></a:solidFill><a:solidFill><a:schemeClr val="phClr"/></a:solidFill><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:fillStyleLst>
<a:lnStyleLst><a:ln w="6350"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln><a:ln w="12700"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln><a:ln w="19050"><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:ln></a:lnStyleLst>
<a:effectStyleLst><a:effectStyle><a:effectLst/></a:effectStyle><a:effectStyle><a:effectLst/></a:effectStyle><a:effectStyle><a:effectLst/></a:effectStyle></a:effectStyleLst>
<a:bgFillStyleLst><a:solidFill><a:schemeClr val="phClr"/></a:solidFill><a:solidFill><a:schemeClr val="phClr"/></a:solidFill><a:solidFill><a:schemeClr val="phClr"/></a:solidFill></a:bgFillStyleLst></a:fmtScheme>
</a:themeElements></a:theme>''',
"word/diagrams/data1.xml": f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<dgm:dataModel xmlns:dgm="{DGM}" xmlns:a="{A}"><dgm:ptLst/><dgm:cxnLst/><dgm:bg/><dgm:whole/>
<dgm:extLst><a:ext uri="{DSP}"><dsp:dataModelExt xmlns:dsp="{DSP}" relId="rId9"/></a:ext></dgm:extLst></dgm:dataModel>''',
"word/diagrams/layout1.xml": f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<dgm:layoutDef xmlns:dgm="{DGM}" uniqueId="urn:probe"/>''',
"word/diagrams/drawing1.xml": f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<dsp:drawing xmlns:dsp="{DSP}" xmlns:a="{A}"><dsp:spTree>
<dsp:nvGrpSpPr><dsp:cNvPr id="0" name=""/><dsp:cNvGrpSpPr/></dsp:nvGrpSpPr><dsp:grpSpPr/>
{"".join(node(x,y,D,n) for (x,y,n) in pts)}</dsp:spTree></dsp:drawing>''',
    }
    with zipfile.ZipFile(path,"w",zipfile.ZIP_DEFLATED) as z:
        for n,c in parts.items(): z.writestr(n,c)


OUT=sys.argv[1]
os.makedirs(OUT, exist_ok=True)

# The 024 shape: a non-square frame, five square nodes on a square span that does not fill it.
F24W, F24H = 6998335, 5848350
D24 = 1610468
P24 = [(2693933,2118940,"CTR"),(2693933,21800,"TOP"),(4791073,2118940,"RGT"),
       (2693933,4216081,"BOT"),(596792,2118940,"LFT")]

cases = [
  # name, effectExtent, frame, diameter, points
  ("like024",      (0,19050,0,57150), F24W, F24H, D24, P24),
  ("like024-noeff",(0,0,0,0),         F24W, F24H, D24, P24),
  # the same nodes in a frame exactly the size of their own span
  ("span-frame",   (0,0,0,0),         5804749, 5804749, D24,
       [(x-596792, y-21800, n) for (x,y,n) in P24]),
]
for name, eff, fw, fh, d, pts in cases:
    build(os.path.join(OUT, name+".docx"), eff, fw, fh, d, pts)
    print("built", name, "frame", fw, fh)
print("expected dx = %d EMU = %.3f pt ; dy = %d EMU = %.3f pt ; diameter %.3f pt"
      % (2097140, 2097140/12700, 2097140, 2097140/12700, D24/12700))
