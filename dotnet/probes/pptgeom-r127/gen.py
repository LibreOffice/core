import re, json, os
LO='/home/user/libreoffice-core'
WT='/home/user/wt-pptgeom/dotnet'

src=open(LO+'/svx/source/customshapes/EnhancedCustomShapeTypeNames.cxx').read()
i=src.index('constexpr NameTypeTable pNameTypeTableArray[]'); j=src.index('};', i)
names=[n for n,_ in re.findall(r'\{\s*u"([^"]+)"_ustr,\s*mso_spt(\w+)\s*\}', src[i:j])]

cs=open(WT+'/src/XRay.Office.Presentations/MsBinary/PptShapeGeometry.cs').read()
k=cs.index('public static string? PresetOf(ushort shapeType) => shapeType switch')
m=cs.index('_ => null,', k)
mapped=[(int(a), b) for a,b in re.findall(r'(\d+)\s*=>\s*"([^"]+)"', cs[k:m])]
print('mapped', len(mapped))
json.dump({'names':names,'mapped':mapped}, open('mapping.json','w'))

# grid: 4cm shapes, 5cm pitch
COLS=12
cells=[]
parts=[]
for idx,(spt,preset) in enumerate(mapped):
    r,c = divmod(idx, COLS)
    x=1+5*c; y=1+5*r
    odf=names[spt]
    cells.append({'spt':spt,'preset':preset,'odf':odf,'x':x*1000,'y':y*1000,'w':4000,'h':4000})
    parts.append(f'   <draw:custom-shape draw:style-name="gr1" svg:width="4cm" svg:height="4cm" svg:x="{x}cm" svg:y="{y}cm">'
                 f'<draw:enhanced-geometry draw:type="{odf}"/></draw:custom-shape>')
json.dump(cells, open('cells.json','w'))
doc=f'''<?xml version="1.0" encoding="UTF-8"?>
<office:document xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
 xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
 xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0"
 xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
 xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"
 office:version="1.3" office:mimetype="application/vnd.oasis.opendocument.graphics">
 <office:automatic-styles>
  <style:page-layout style:name="PM1"><style:page-layout-properties fo:page-width="21cm" fo:page-height="29.7cm"/></style:page-layout>
  <style:style style:name="dp1" style:family="drawing-page"/>
  <style:style style:name="gr1" style:family="graphic"><style:graphic-properties draw:fill="none" draw:stroke="solid" svg:stroke-width="0.02cm"/></style:style>
 </office:automatic-styles>
 <office:master-styles><style:master-page style:name="M1" style:page-layout-name="PM1" draw:style-name="dp1"/></office:master-styles>
 <office:body><office:drawing>
  <draw:page draw:name="p1" draw:master-page-name="M1">
{chr(10).join(parts)}
  </draw:page>
 </office:drawing></office:body>
</office:document>
'''
open('census.fodg','w').write(doc)
print('cells', len(cells))
