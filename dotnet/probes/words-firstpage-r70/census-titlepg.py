import zipfile, pathlib, re, sys, collections
import xml.etree.ElementTree as ET
W='{http://schemas.openxmlformats.org/wordprocessingml/2006/main}'
corpus=pathlib.Path('/home/user/sample-files')
rows=[]
tot=0
for p in sorted(corpus.rglob('*')):
    if not p.is_file(): continue
    if p.suffix.lower() not in ('.docx','.docm','.dotx'): continue
    tot+=1
    try:
        z=zipfile.ZipFile(p); doc=z.read('word/document.xml')
    except Exception as e:
        continue
    try: root=ET.fromstring(doc)
    except Exception: continue
    n_tp=0; n_hdr_nofirst=0; n_ftr_nofirst=0
    for sect in root.iter(W+'sectPr'):
        if sect.find(W+'titlePg') is None: continue
        n_tp+=1
        types={h.get(W+'type','default') for h in sect.findall(W+'headerReference')}
        ftypes={h.get(W+'type','default') for h in sect.findall(W+'footerReference')}
        if types and 'first' not in types: n_hdr_nofirst+=1
        if ftypes and 'first' not in ftypes: n_ftr_nofirst+=1
    if n_tp: rows.append((str(p.relative_to(corpus)), n_tp, n_hdr_nofirst, n_ftr_nofirst))
print('docx-family files scanned:', tot)
print('with any titlePg section:', len(rows))
print('with a titlePg section declaring headers but no first header:', sum(1 for r in rows if r[2]))
print('  ... same for footers:', sum(1 for r in rows if r[3]))
for r in rows:
    if r[2] or r[3]: print('  ', r)
