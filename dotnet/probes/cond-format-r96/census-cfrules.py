import glob, zipfile, collections, sys, re
import xml.etree.ElementTree as ET
NS='{http://schemas.openxmlformats.org/spreadsheetml/2006/main}'
types=collections.Counter(); docs=collections.defaultdict(set)
withdxf=collections.Counter(); dxfdocs=collections.defaultdict(set)
ops=collections.Counter()
multi=0; multibad=0; multibadex=[]
files=sorted(glob.glob('/home/user/sample-files/sheets/*/xlsx/*'))+ \
      sorted(glob.glob('/home/user/sample-files/sheets/*/xlsm/*'))
seen=0
for f in files:
    try: z=zipfile.ZipFile(f)
    except Exception: continue
    seen+=1
    for n in z.namelist():
        if not (n.startswith('xl/worksheets/') and n.endswith('.xml')): continue
        try: root=ET.fromstring(z.read(n))
        except Exception: continue
        for blk in root.iter(NS+'conditionalFormatting'):
            sq=blk.get('sqref') or ''
            parts=sq.split()
            if len(parts)>1:
                multi+=1
                # componentwise min vs GetTopLeftCorner
                def parse(p):
                    a=p.split(':')[0].replace('$','')
                    m=re.match(r'([A-Za-z]+)(\d+)',a)
                    if not m: return None
                    c=0
                    for ch in m.group(1).upper(): c=c*26+ord(ch)-64
                    return (c-1,int(m.group(2))-1)
                st=[parse(p) for p in parts]
                if all(st):
                    cm=(min(s[0] for s in st), min(s[1] for s in st))
                    tl=min(st)  # (col,row) ordering
                    if cm!=tl:
                        multibad+=1
                        if len(multibadex)<5: multibadex.append((f.split('/')[-1],sq[:60]))
            for r in blk.iter(NS+'cfRule'):
                t=r.get('type') or '?'
                types[t]+=1; docs[t].add(f)
                if r.get('dxfId') is not None:
                    withdxf[t]+=1; dxfdocs[t].add(f)
                if t in ('containsText','notContainsText','beginsWith','endsWith'):
                    ops[(t,r.get('operator'),'text' if r.get('text') is not None else 'NOTEXT')]+=1
print("workbooks scanned",seen)
print(f"{'type':22} {'rules':>6} {'docs':>5} {'w/dxfId':>8} {'dxfdocs':>8}")
for t,c in types.most_common():
    print(f"{t:22} {c:6d} {len(docs[t]):5d} {withdxf[t]:8d} {len(dxfdocs[t]):8d}")
print("multi-range sqref blocks:",multi," where componentwise-min != GetTopLeftCorner:",multibad, multibadex)
print("text-rule operator/text census:")
for k,v in ops.most_common(): print("  ",k,v)
