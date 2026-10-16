#!/usr/bin/env python3
"""Which run's size does 26.2.4.2 resolve a PPT percentage paragraph space against?

Same question as `ulpct.py`, with the loose free parameter removed.  The exported margin is
`convertMasterUnitToMm100(fontHeightPt * pct / 10)` and `pct` is unknown, so `ulpct.py` admitted
a candidate whenever ANY whole percentage fitted -- which admits 64-76 % of paragraphs by chance.

Here the percentage set is derived from the document itself: every paragraph whose runs are all
one size pins `pct` exactly when only one whole value reproduces its margin.  The multi-size
paragraphs are then scored against that set only, and the base rate is reported beside the count.
"""
import re, sys, collections
from xml.etree import ElementTree as ET

NS = {
 'style':'urn:oasis:names:tc:opendocument:xmlns:style:1.0',
 'text':'urn:oasis:names:tc:opendocument:xmlns:text:1.0',
 'fo':'urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0',
}
def q(t):
    p,l=t.split(':'); return '{%s}%s'%(NS[p],l)

def length_mm100(s):
    if s is None: return None
    m=re.match(r'^(-?[\d.]+)(cm|mm|in|pt)$', s.strip())
    if not m: return None
    return round(float(m.group(1))*{'cm':1000.0,'mm':100.0,'in':2540.0,'pt':2540.0/72.0}[m.group(2)])

def points(s):
    m=re.match(r'^([\d.]+)pt$', s.strip()) if s else None
    return float(m.group(1)) if m else None

def master_to_mm100(n): return (n*2540 + 288)//576

def pcts(mm100, size_pt, hi=200):
    return {p for p in range(1,hi+1) if master_to_mm100(int(size_pt*p*100)//1000)==mm100}

def parse(path):
    root=ET.parse(path).getroot()
    pmargin={}; tsize={}
    for st in root.iter(q('style:style')):
        fam=st.get(q('style:family'))
        if fam=='paragraph':
            pp=st.find(q('style:paragraph-properties'))
            if pp is not None:
                pmargin[st.get(q('style:name'))]=length_mm100(pp.get(q('fo:margin-top')))
        elif fam=='text':
            tp=st.find(q('style:text-properties'))
            if tp is not None: tsize[st.get(q('style:name'))]=points(tp.get(q('fo:font-size')))
    single=[]; multi=[]
    for p in root.iter(q('text:p')):
        sizes=[tsize[sp.get(q('text:style-name'))]
               for sp in p.iter(q('text:span'))
               if tsize.get(sp.get(q('text:style-name')))]
        if not sizes: continue
        mt=pmargin.get(p.get(q('text:style-name')))
        if not mt: continue
        (single if len(set(sizes))==1 else multi).append((mt,sizes))
    return single, multi

def main(paths):
    tally=collections.Counter(); total=0; rows=[]
    allpct=collections.Counter()
    for path in paths:
        doc=path.split('/')[-1][:-5]
        single, multi = parse(path)
        pinned=set()
        for mt,sizes in single:
            cand=pcts(mt,sizes[0])
            if len(cand)==1: pinned |= cand
        allpct[doc]=sorted(pinned)
        if not pinned: continue
        for mt,sizes in multi:
            total+=1
            f,l,g = sizes[0], sizes[-1], max(sizes)
            a={'first': bool(pcts(mt,f)&pinned),
               'last':  bool(pcts(mt,l)&pinned),
               'largest': bool(pcts(mt,g)&pinned)}
            for k,v in a.items():
                if v: tally[k]+=1
            rows.append((doc,mt,f,l,g,a))
    print("# percentages pinned by this document's own single-size paragraphs:")
    for d,ps in allpct.items(): print(f"#   {d[:60]:60} {ps}")
    print(f"# multi-size paragraphs scored: {total}")
    for k in ('first','last','largest'):
        print(f"#   admitted by {k:8}: {tally[k]:4d}   ({100.0*tally[k]/total if total else 0:.1f} %)")
    print()
    print(f"{'document':40} {'mm100':>6} {'first':>6} {'last':>6} {'largest':>7}  admitted-by")
    for d,mt,f,l,g,a in rows:
        print(f"{d[:40]:40} {mt:6d} {f:6} {l:6} {g:7}  " + (','.join(k for k,v in a.items() if v) or '-none-'))

if __name__=='__main__': main(sys.argv[1:])
