#!/usr/bin/env python3
"""Two minimal DOCX differing only in WHAT the page-anchored object is: a shape or a chart.

    ./fixture.py <outdir>

`probes/ofpie-r190/page-anchor-fixture.py` built the shape half alone and 26.2.4.2 drew it at the
page's own top edge, which refuted "page means margin" and left `028`'s 71.6 pt unexplained. The
missing variable is the object kind: a chart's `a:graphicData` becomes a
`com.sun.star.drawing.OLE2Shape`, which `DomainMapper_Impl` replaces with a
`SwXTextEmbeddedObject` and then gives `IsFollowingTextFlow` straight from `layoutInCell` with no
`IsInTable()` test (`DomainMapper_Impl.cxx`:5096-5112 and :9792) -- where both of
`GraphicImport`'s own writes of that property are guarded (`GraphicImport.cxx`:1316-1318, :1859-1861).

So every file here states the same `wp:anchor` at the same offset on the same page, and varies one
of three things: the object kind, `w:pgMar/@w:top`, and `layoutInCell`. Render them with

    /opt/libreoffice26.2/program/soffice --headless --convert-to pdf --outdir <d> <f>.docx

and read the object's top edge out of the page: the shape is one red fill, and the chart draws its
own white background rectangle and a plot frame. `.claude/skills/render-comparison/scripts/pdf-ops.py
dump <pdf> --only fill` prints both.

The prediction, from the sources above: the shape is drawn at `offset` below the SHEET's top at
every margin, and the chart at `offset` below the BODY's top -- so `top1440` and `top2880` move the
chart by 72 pt and leave the shape alone, and `incell0` puts the chart back where the shape is.
"""
import pathlib
import sys
import zipfile

OUT = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else '.')

CHART_URI = 'http://schemas.openxmlformats.org/drawingml/2006/chart'
WPS_URI = 'http://schemas.microsoft.com/office/word/2010/wordprocessingShape'
CHART_TYPE = ('application/vnd.openxmlformats-officedocument.drawingml.chart+xml')
CHART_REL = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships/chart'
SETTINGS_REL = ('http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings')

# A chart part small enough to read and complete enough for LibreOffice to import as an OLE object.
CHART = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<c:chartSpace xmlns:c="http://schemas.openxmlformats.org/drawingml/2006/chart"
 xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"
 xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">
<c:chart><c:plotArea><c:layout/>
<c:barChart><c:barDir val="col"/><c:grouping val="clustered"/>
<c:ser><c:idx val="0"/><c:order val="0"/>
<c:cat><c:strRef><c:f>Sheet1!$A$1:$A$2</c:f><c:strCache><c:ptCount val="2"/>
<c:pt idx="0"><c:v>A</c:v></c:pt><c:pt idx="1"><c:v>B</c:v></c:pt>
</c:strCache></c:strRef></c:cat>
<c:val><c:numRef><c:f>Sheet1!$B$1:$B$2</c:f><c:numCache><c:formatCode>General</c:formatCode>
<c:ptCount val="2"/><c:pt idx="0"><c:v>1</c:v></c:pt><c:pt idx="1"><c:v>2</c:v></c:pt>
</c:numCache></c:numRef></c:val>
</c:ser><c:axId val="111111111"/><c:axId val="222222222"/></c:barChart>
<c:catAx><c:axId val="111111111"/><c:scaling><c:orientation val="minMax"/></c:scaling>
<c:delete val="0"/><c:axPos val="b"/><c:crossAx val="222222222"/></c:catAx>
<c:valAx><c:axId val="222222222"/><c:scaling><c:orientation val="minMax"/></c:scaling>
<c:delete val="0"/><c:axPos val="l"/><c:crossAx val="111111111"/></c:valAx>
</c:plotArea><c:plotVisOnly val="1"/></c:chart></c:chartSpace>'''

SETTINGS = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
<w:compat><w:compatSetting w:name="compatibilityMode"
 w:uri="http://schemas.microsoft.com/office/word" w:val="15"/></w:compat>
</w:settings>'''

SHAPE_BODY = f'''<a:graphic><a:graphicData uri="{WPS_URI}">
<wps:wsp><wps:cNvSpPr/>
<wps:spPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="2743200" cy="1828800"/></a:xfrm>
<a:prstGeom prst="rect"><a:avLst/></a:prstGeom>
<a:solidFill><a:srgbClr val="FF0000"/></a:solidFill>
<a:ln><a:noFill/></a:ln></wps:spPr>
<wps:bodyPr/></wps:wsp>
</a:graphicData></a:graphic>'''

CHART_BODY = f'''<a:graphic><a:graphicData uri="{CHART_URI}">
<c:chart xmlns:c="{CHART_URI}"
 xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" r:id="rId2"/>
</a:graphicData></a:graphic>'''


def document(body: str, top: str, incell: str | None) -> str:
    # An absent `layoutInCell` is not the same fixture as `layoutInCell="1"`: `m_bLayoutInCell` is
    # initialised true, so the two should render identically, and that is worth a variant rather than an
    # assumption.
    cell = '' if incell is None else f' layoutInCell="{incell}"'
    return f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
 xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"
 xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"
 xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"
 xmlns:wps="{WPS_URI}">
<w:body>
<w:p><w:r><w:drawing>
<wp:anchor distT="0" distB="0" distL="114300" distR="114300" simplePos="0" relativeHeight="1"
 behindDoc="0" locked="0"{cell} allowOverlap="1">
<wp:simplePos x="0" y="0"/>
<wp:positionH relativeFrom="page"><wp:posOffset>914400</wp:posOffset></wp:positionH>
<wp:positionV relativeFrom="page"><wp:posOffset>2286000</wp:posOffset></wp:positionV>
<wp:extent cx="2743200" cy="1828800"/>
<wp:effectExtent l="0" t="0" r="0" b="0"/>
<wp:wrapSquare wrapText="bothSides"/>
<wp:docPr id="1" name="Object"/>
{body}
</wp:anchor>
</w:drawing></w:r></w:p>
<w:p><w:r><w:t>Body text, so the anchor paragraph is in the page body.</w:t></w:r></w:p>
<w:sectPr>
<w:pgSz w:w="11906" w:h="16838"/>
<w:pgMar w:top="{top}" w:right="1440" w:bottom="1440" w:left="1440"
 w:header="0" w:footer="0" w:gutter="0"/>
</w:sectPr>
</w:body></w:document>'''


def write(name: str, kind: str, top: str, incell: str | None) -> None:
    chart = kind == 'chart'
    types = ['<?xml version="1.0" encoding="UTF-8" standalone="yes"?>',
             '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">',
             '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package'
             '.relationships+xml"/>',
             '<Default Extension="xml" ContentType="application/xml"/>',
             '<Override PartName="/word/document.xml" ContentType="application/vnd'
             '.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>',
             '<Override PartName="/word/settings.xml" ContentType="application/vnd'
             '.openxmlformats-officedocument.wordprocessingml.settings+xml"/>']
    if chart:
        types.append(f'<Override PartName="/word/charts/chart1.xml" ContentType="{CHART_TYPE}"/>')
    types.append('</Types>')

    rels = ['<?xml version="1.0" encoding="UTF-8" standalone="yes"?>',
            '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">',
            f'<Relationship Id="rId1" Type="{SETTINGS_REL}" Target="settings.xml"/>']
    if chart:
        rels.append(f'<Relationship Id="rId2" Type="{CHART_REL}" Target="charts/chart1.xml"/>')
    rels.append('</Relationships>')

    target = OUT / f'{name}.docx'
    with zipfile.ZipFile(target, 'w', zipfile.ZIP_DEFLATED) as out:
        out.writestr('[Content_Types].xml', '\n'.join(types))
        out.writestr('_rels/.rels',
                     '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
                     '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/'
                     'relationships">\n<Relationship Id="rId1" Type="http://schemas.openxmlformats'
                     '.org/officeDocument/2006/relationships/officeDocument" '
                     'Target="word/document.xml"/>\n</Relationships>')
        out.writestr('word/_rels/document.xml.rels', '\n'.join(rels))
        out.writestr('word/settings.xml', SETTINGS)
        out.writestr('word/document.xml',
                     document(CHART_BODY if chart else SHAPE_BODY, top, incell))
        if chart:
            out.writestr('word/charts/chart1.xml', CHART)
    print('wrote', target)


OUT.mkdir(parents=True, exist_ok=True)
for kind in ('shape', 'chart'):
    for top in ('1440', '2880'):
        write(f'{kind}-top{top}', kind, top, '1')
    write(f'{kind}-incell0', kind, '1440', '0')
    write(f'{kind}-noincell', kind, '1440', None)
