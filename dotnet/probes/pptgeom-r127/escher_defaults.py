"""The Escher adjustment defaults, read out of the C++ table, for cross-check against the
binary's own `draw:modifiers`."""
import re, json
src=open('/home/user/libreoffice-core/svx/source/customshapes/EnhancedCustomShapeGeometry.cxx').read()
enum=open('/home/user/libreoffice-core/include/svx/msdffdef.hxx').read()
nums={}
for m in re.finditer(r'mso_spt(\w+)\s*=\s*(\d+)', enum):
    nums[m.group(1)]=int(m.group(2))
# fill implicit enum numbering
mspt=re.search(r'enum MSO_SPT\s*:?\s*\w*\s*\{(.*?)\n\};', enum, re.S)
cur=-1; order={}
for item in mspt.group(1).split(','):
    item=item.split('//')[0].strip()
    if not item: continue
    mm=re.match(r'mso_spt(\w+)\s*(?:=\s*(-?\d+))?$', item)
    if not mm: continue
    cur = int(mm.group(2)) if mm.group(2) else cur+1
    order[mm.group(1)]=cur
# case mso_sptX : pCustomShape = &msoY; break;
case={}
for m in re.finditer(r'case\s+mso_spt(\w+)\s*:\s*pCustomShape\s*=\s*&(\w+);', src):
    case[m.group(1)]=m.group(2)
# msoY = { ... pDefData ... }
defname={}
for m in re.finditer(r'const mso_CustomShape (\w+)\s*=\s*\{(.*?)\n\};', src, re.S):
    body=m.group(2)
    d=re.search(r'const_cast<sal_Int32\*>\((\w+)\)', body)
    defname[m.group(1)]=d.group(1) if d else None
arrays={}
for m in re.finditer(r'const sal_Int32 (mso_spt\w*Default\w*)\[\]\s*=\s*\{([^}]*)\}', src):
    arrays[m.group(1)]=[int(x) for x in re.findall(r'-?\d+', m.group(2))]
out={}
for name,spt in order.items():
    shp=case.get(name)
    if shp is None: continue
    dn=defname.get(shp)
    vals=arrays.get(dn) if dn else None
    out[spt]= (vals[1:1+vals[0]] if vals else [])
json.dump(out, open('escher_defaults.json','w'))
print(len(out), 'types with an Escher definition')
for k in (2,5,7,8,13,63,49,34,38):
    print(k, out.get(k))
