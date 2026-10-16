import zipfile, pathlib
import xml.etree.ElementTree as ET
W='{http://schemas.openxmlformats.org/wordprocessingml/2006/main}'
corpus=pathlib.Path('/home/user/sample-files')
hits=[]; scanned=0
for p in sorted(corpus.rglob('*')):
    if not p.is_file() or p.suffix.lower() not in ('.docx','.docm','.dotx'): continue
    scanned+=1
    try:
        z=zipfile.ZipFile(p); root=ET.fromstring(z.read('word/styles.xml'))
    except Exception: continue
    order={}; info={}
    for i,st in enumerate(root.findall(W+'style')):
        if st.get(W+'type') not in (None,'paragraph'): continue
        sid=st.get(W+'styleId')
        if sid is None: continue
        order[sid]=i
        pPr=st.find(W+'pPr')
        based=st.find(W+'basedOn')
        info[sid]=dict(
            ctx = pPr is not None and pPr.find(W+'contextualSpacing') is not None,
            ctxoff = (pPr is not None and pPr.find(W+'contextualSpacing') is not None
                      and pPr.find(W+'contextualSpacing').get(W+'val') in ('0','false')),
            spacing = pPr is not None and pPr.find(W+'spacing') is not None,
            parent = based.get(W+'val') if based is not None else None)
    bad=[]
    for sid,d in info.items():
        if d['ctx'] or not d['spacing']: continue
        par=d['parent']
        seen=set()
        while par and par in info and par not in seen:
            seen.add(par)
            if info[par]['ctx'] and not info[par]['ctxoff']:
                if order[par] > order[sid]: bad.append((sid,par))
                break
            par=info[par]['parent']
    if bad: hits.append((str(p.relative_to(corpus)), bad))
print('scanned', scanned, 'hits', len(hits))
for h in hits: print(' ', h[0], h[1][:4])
