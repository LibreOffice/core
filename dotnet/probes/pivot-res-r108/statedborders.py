#!/usr/bin/env python3
"""What do the workbook's own cells inside each pivot's stated range state?

Counts, per pivot range, the cells whose `cellXfs` entry states a border, a bold weight, a
horizontal alignment other than general, or an indent — the four things the generated
formatting would otherwise be laid over. Calc clears the range before regenerating
(`pivottablebuffer.cxx`:1331-1336, `dpoutput.cxx`:1226), so where these are all zero, replacing
and merging are the same thing.

    statedborders.py <workbook> ...
"""
import sys, zipfile, os, re
import xml.etree.ElementTree as ET
M='http://schemas.openxmlformats.org/spreadsheetml/2006/main'
R='http://schemas.openxmlformats.org/officeDocument/2006/relationships'
def m(t): return '{%s}%s'%(M,t)
def colnum(s):
    n=0
    for ch in s: n=n*26+ord(ch)-64
    return n-1
def parse_ref(ref):
    a,_,b=ref.partition(':'); b=b or a
    ma=re.match(r'\$?([A-Z]+)\$?(\d+)',a); mb=re.match(r'\$?([A-Z]+)\$?(\d+)',b)
    return colnum(ma.group(1)),int(ma.group(2))-1,colnum(mb.group(1)),int(mb.group(2))-1

def stated(z):
    """cellXfs index -> (border, bold, aligned, indented)."""
    st=ET.fromstring(z.read('xl/styles.xml'))
    borders=[]
    for b in st.find(m('borders')) or []:
        any_=False
        for side in ('left','right','top','bottom'):
            e=b.find(m(side))
            if e is not None and e.get('style') not in (None,'none'): any_=True
        borders.append(any_)
    bolds=[]
    for f in st.find(m('fonts')) or []:
        e=f.find(m('b'))
        bolds.append(e is not None and e.get('val') not in ('0','false'))
    out=[]
    for xf in st.find(m('cellXfs')) or []:
        bid=int(xf.get('borderId','0')); fid=int(xf.get('fontId','0'))
        al=xf.find(m('alignment'))
        h=al.get('horizontal') if al is not None else None
        ind=int(al.get('indent','0')) if al is not None else 0
        out.append((bid<len(borders) and borders[bid],
                    fid<len(bolds) and bolds[fid],
                    h not in (None,'general'),
                    ind>0))
    return out

for path in sys.argv[1:]:
    z=zipfile.ZipFile(path); name=os.path.basename(path)
    xfb=stated(z)
    wb=ET.fromstring(z.read('xl/workbook.xml'))
    rels=ET.fromstring(z.read('xl/_rels/workbook.xml.rels'))
    tgt={r.get('Id'):r.get('Target') for r in rels}
    for sh in wb.find(m('sheets')):
        t=tgt[sh.get('{%s}id'%R)]
        part=t.lstrip('/') if t.startswith('/') else ('xl/'+t if not t.startswith('xl/') else t)
        rp=os.path.join(os.path.dirname(part),'_rels',os.path.basename(part)+'.rels')
        if rp not in z.namelist(): continue
        pts=[]
        for r in ET.fromstring(z.read(rp)):
            if r.get('Type','').endswith('/pivotTable'):
                raw=r.get('Target')
                pts.append((raw.lstrip('/') if raw.startswith('/') else os.path.normpath(os.path.join(os.path.dirname(part),raw))).replace('\\','/'))
        if not pts: continue
        ws=ET.fromstring(z.read(part))
        cells={}
        for row in ws.find(m('sheetData')) or []:
            for c in row:
                a=c.get('r')
                if not a: continue
                mm=re.match(r'([A-Z]+)(\d+)',a)
                cells[(int(mm.group(2))-1, colnum(mm.group(1)))]=int(c.get('s','0'))
        # row/col default styles
        for pt in pts:
            root=ET.fromstring(z.read(pt))
            loc=root.find(m('location'))
            c0,r0,c1,r1=parse_ref(loc.get('ref'))
            n=[0,0,0,0]
            for r in range(r0,r1+1):
                for c in range(c0,c1+1):
                    s=cells.get((r,c))
                    if s is None or s>=len(xfb): continue
                    for k in range(4):
                        if xfb[s][k]: n[k]+=1
            print('%s\t%s\t%s\tref=%s\tborder=%d bold=%d aligned=%d indented=%d'%(
                name, sh.get('name'), os.path.basename(pt), loc.get('ref'), *n))
