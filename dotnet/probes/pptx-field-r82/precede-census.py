import zipfile, collections
from pathlib import Path
import xml.etree.ElementTree as ET
A='{http://schemas.openxmlformats.org/drawingml/2006/main}'
R='{http://schemas.openxmlformats.org/officeDocument/2006/relationships}'
ROOT=Path('/home/user/sample-files')
cnt=collections.Counter()
for path in sorted(ROOT.rglob('*.pptx')):
    try: zf=zipfile.ZipFile(path)
    except Exception: continue
    for name in zf.namelist():
        if not name.startswith('ppt/slides/slide') or not name.endswith('.xml'): continue
        try: data=zf.read(name)
        except Exception: continue
        if b'hlinkClick' not in data: continue
        try: root=ET.fromstring(data)
        except Exception: continue
        for p in root.iter(A+'p'):
            text=''
            for ch in p:
                if ch.tag!=A+'r': continue
                rpr=ch.find(A+'rPr'); t=ch.find(A+'t')
                s=(t.text or '') if t is not None else ''
                hl=rpr.find(A+'hlinkClick') if rpr is not None else None
                if hl is not None and (hl.get(R+'id') or hl.get('action') or hl.get('tooltip')):
                    if text=='':
                        cnt['starts paragraph']+=1
                    elif text[-1].isspace():
                        cnt['after whitespace']+=1
                    else:
                        cnt['glued to previous text']+=1
                        cnt['glued sample:'+repr(text[-12:]+'|'+s[:12])]+=0
                text+=s
    zf.close()
for k,v in cnt.most_common(8): print(v,k)
