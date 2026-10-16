#!/usr/bin/env python3
"""Does any corpus workbook give cellXfs[0] and the `Normal` cellStyleXf different content?"""
import sys, zipfile, os
import xml.etree.ElementTree as ET
M='http://schemas.openxmlformats.org/spreadsheetml/2006/main'
def m(t): return '{%s}%s'%(M,t)
def canon(xf):
    if xf is None: return None
    d = {k: v for k, v in xf.attrib.items() if k != 'xfId'}
    # apply* flags describe whether the id is used; keep them, they change the meaning
    al = xf.find(m('alignment'))
    d['_align'] = tuple(sorted(al.attrib.items())) if al is not None else None
    pr = xf.find(m('protection'))
    d['_prot'] = tuple(sorted(pr.attrib.items())) if pr is not None else None
    return tuple(sorted((k, v) for k, v in d.items()))
bad=[]; n=0; nostyles=0
for path in sys.argv[1:]:
    try:
        z=zipfile.ZipFile(path)
        if 'xl/styles.xml' not in z.namelist(): nostyles+=1; continue
        st=ET.fromstring(z.read('xl/styles.xml'))
    except Exception as e:
        print('SKIP', os.path.basename(path), e); continue
    cx=list(st.find(m('cellXfs')) or [])
    sx=list(st.find(m('cellStyleXfs')) or [])
    if not cx or not sx: nostyles+=1; continue
    idx=0
    for s in (st.find(m('cellStyles')) or []):
        if s.get('builtinId')=='0':
            idx=int(s.get('xfId','0')); break
    normal = sx[idx] if idx < len(sx) else None
    n+=1
    a,b=canon(cx[0]),canon(normal)
    # compare only the parts that decide a format: font/fill/border/numFmt ids and alignment
    def keep(t):
        if t is None: return None
        return tuple((k,v) for k,v in t if k in ('fontId','fillId','borderId','numFmtId','_align','_prot'))
    if keep(a)!=keep(b):
        bad.append((os.path.basename(path), keep(a), keep(b)))
print('workbooks with both tables: %d   (skipped %d)' % (n, nostyles))
print('differing: %d' % len(bad))
for x in bad[:25]: print('  ', x[0], '\n     cellXfs[0]=', x[1], '\n     Normal    =', x[2])
