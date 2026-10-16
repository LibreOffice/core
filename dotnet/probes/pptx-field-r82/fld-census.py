import zipfile, collections
from pathlib import Path
import xml.etree.ElementTree as ET
A='{http://schemas.openxmlformats.org/drawingml/2006/main}'
ROOT=Path('/home/user/sample-files')
kinds=collections.Counter(); docs=collections.defaultdict(set); lens=collections.Counter()
def recognised(t):
    if not t: return False
    return (t.startswith('datetime') or t in ('slidenum','slidecount','slidename','author')
            or t.startswith('file'))
for path in sorted(ROOT.rglob('*.pptx')):
    try: zf=zipfile.ZipFile(path)
    except Exception: continue
    rel=str(path.relative_to(ROOT))
    for name in zf.namelist():
        if not name.startswith('ppt/slides/slide') or not name.endswith('.xml'): continue
        try: root=ET.fromstring(zf.read(name))
        except Exception: continue
        for f in root.iter(A+'fld'):
            t=f.get('type'); tt=f.find(A+'t')
            txt=(tt.text or '') if tt is not None else ''
            k=('recognised' if recognised(t) else 'plain-text')+':'+(t or 'none')
            kinds[k]+=1; docs[k].add(rel); lens[len(txt)]+=1
    zf.close()
for k,v in kinds.most_common(12): print(v, len(docs[k]), k)
print('max cached text length', max(lens) if lens else 0)
