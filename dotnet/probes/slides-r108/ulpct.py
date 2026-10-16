#!/usr/bin/env python3
"""Which run's size does 26.2.4.2 resolve a PPT percentage paragraph space against?

Reads a flat ODP that 26.2.4.2 itself wrote from a .ppt.  For every paragraph carrying more
than one distinct run size, the exported `fo:margin-top` / `fo:margin-bottom` is compared with
the three candidate resolutions -- the FIRST run's size, the LAST run's, and the LARGEST --
through the reference's own arithmetic:

    master = fontHeightPt * pct / 10        (svdfppt.cxx:6303-6305)
    mm100  = (master * 2540 + 288) / 576    (o3tl master -> mm100)

pct is unknown per paragraph, so the test is inverted: a candidate is admitted when SOME whole
percentage reproduces the exported length exactly.  The base rate -- how often each candidate is
admitted by chance -- is therefore reported beside the count.
"""
import re, sys, collections
from xml.etree import ElementTree as ET

NS = {
 'office':'urn:oasis:names:tc:opendocument:xmlns:office:1.0',
 'style':'urn:oasis:names:tc:opendocument:xmlns:style:1.0',
 'text':'urn:oasis:names:tc:opendocument:xmlns:text:1.0',
 'fo':'urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0',
 'draw':'urn:oasis:names:tc:opendocument:xmlns:drawing:1.0',
}
def q(t):
    p,l=t.split(':'); return '{%s}%s'%(NS[p],l)

def length_mm100(s):
    if s is None: return None
    m=re.match(r'^(-?[\d.]+)(cm|mm|in|pt)$', s.strip())
    if not m: return None
    v=float(m.group(1)); u=m.group(2)
    return round(v*{'cm':1000.0,'mm':100.0,'in':2540.0,'pt':2540.0/72.0}[u])

def points(s):
    if s is None: return None
    m=re.match(r'^([\d.]+)pt$', s.strip())
    return float(m.group(1)) if m else None

def master_to_mm100(n):  # o3tl::convert(n, master, mm100)
    return (n*2540 + 288)//576

def admits(mm100, size_pt):
    """Is there a whole percentage pct with (fround?) master = int(size*pct/10) giving mm100?"""
    if size_pt is None: return False
    for pct in range(1, 401):
        master = int(size_pt*pct*100)//1000
        if master_to_mm100(master) == mm100:
            return True
    return False

def main(paths):
    tally=collections.Counter(); total=0
    detail=[]
    for path in paths:
        tree=ET.parse(path); root=tree.getroot()
        # paragraph styles -> margin-top/bottom;  text styles -> font size
        pmargin={}
        for st in root.iter(q('style:style')):
            if st.get(q('style:family'))=='paragraph':
                pp=st.find(q('style:paragraph-properties'))
                if pp is not None:
                    pmargin[st.get(q('style:name'))]=(
                        length_mm100(pp.get(q('fo:margin-top'))),
                        length_mm100(pp.get(q('fo:margin-bottom'))))
            elif st.get(q('style:family'))=='text':
                tp=st.find(q('style:text-properties'))
                if tp is not None:
                    pmargin.setdefault('__t__',{})
        tsize={}
        for st in root.iter(q('style:style')):
            if st.get(q('style:family'))=='text':
                tp=st.find(q('style:text-properties'))
                if tp is not None:
                    tsize[st.get(q('style:name'))]=points(tp.get(q('fo:font-size')))
        for p in root.iter(q('text:p')):
            sizes=[]
            for sp in p.iter(q('text:span')):
                nm=sp.get(q('text:style-name'))
                if nm in tsize and tsize[nm]:
                    txt=''.join(sp.itertext())
                    sizes.append((tsize[nm], len(txt)))
            if len(sizes)<2: continue
            distinct={s for s,_ in sizes}
            if len(distinct)<2: continue
            mt,_mb = pmargin.get(p.get(q('text:style-name')), (None,None))
            if not mt: continue
            total+=1
            first=sizes[0][0]; last=sizes[-1][0]; largest=max(s for s,_ in sizes)
            longest=max(sizes, key=lambda kv: kv[1])[0]
            a={'first':admits(mt,first),'last':admits(mt,last),
               'largest':admits(mt,largest),'longest':admits(mt,longest)}
            for k,v in a.items():
                if v: tally[k]+=1
            detail.append((path.split('/')[-2], mt, first, last, largest, a))
    print(f"# paragraphs with two or more distinct run sizes and a stated margin-top: {total}")
    for k in ('first','last','largest','longest'):
        print(f"#   admitted by {k:8}: {tally[k]:4d}   ({100.0*tally[k]/total if total else 0:.1f} %)")
    print()
    print(f"{'document':44} {'mm100':>6} {'first':>6} {'last':>6} {'largest':>7}  admitted-by")
    for d,mt,f,l,g,a in detail:
        print(f"{d[:44]:44} {mt:6d} {f:6} {l:6} {g:7}  "
              + ','.join(k for k,v in a.items() if v))

if __name__=='__main__':
    main(sys.argv[1:])
