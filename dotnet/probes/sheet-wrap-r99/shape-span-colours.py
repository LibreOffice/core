#!/usr/bin/env python3
"""Every span of every DRAWING shape's text in a flat ODF, with the colour and slant the
reference resolved for it. The reference's own answer to "does this run's stated colour
reach the page".
"""
import sys, collections
import xml.etree.ElementTree as ET
NS={'office':'urn:oasis:names:tc:opendocument:xmlns:office:1.0',
 'style':'urn:oasis:names:tc:opendocument:xmlns:style:1.0',
 'draw':'urn:oasis:names:tc:opendocument:xmlns:drawing:1.0',
 'table':'urn:oasis:names:tc:opendocument:xmlns:table:1.0',
 'text':'urn:oasis:names:tc:opendocument:xmlns:text:1.0',
 'fo':'urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0'}
def q(t):
    p,l=t.split(':'); return '{%s}%s'%(NS[p],l)

def load(path):
    r=ET.parse(path).getroot()
    styles={}
    parents={}
    for st in r.iter(q('style:style')):
        nm=st.get(q('style:name'))
        parents[nm]=st.get(q('style:parent-style-name'))
        tp=st.find(q('style:text-properties'))
        styles[nm]=(tp.get(q('fo:color')) if tp is not None else None,
                    tp.get(q('fo:font-style')) if tp is not None else None)
    def resolve(nm, seen=None):
        seen=seen or set()
        col=sl=None
        while nm and nm not in seen:
            seen.add(nm)
            c,s=styles.get(nm,(None,None))
            col=col or c; sl=sl or s
            nm=parents.get(nm)
        return col,sl
    return r,resolve

DRAW={q('draw:frame'),q('draw:custom-shape'),q('draw:text-box'),q('draw:rect'),
      q('draw:line'),q('draw:g'),q('draw:polygon'),q('draw:path'),q('draw:ellipse'),
      q('draw:connector'),q('draw:caption'),q('draw:control')}

for path in sys.argv[1:]:
    r,resolve=load(path)
    counts=collections.Counter()
    seen_texts=[]
    for node in r.iter():
        if node.tag not in DRAW: continue
        for p in node.iter(q('text:p')):
            pcol,psl=resolve(p.get(q('text:style-name')))
            spans=p.findall(q('text:span'))
            if not spans:
                txt=''.join(p.itertext())
                if txt.strip(): counts[(pcol or '(none)', psl or '(upright)')]+=1
                continue
            for sp in spans:
                col,sl=resolve(sp.get(q('text:style-name')))
                col=col or pcol; sl=sl or psl
                txt=''.join(sp.itertext())
                if not txt.strip(): continue
                counts[(col or '(none)', sl or '(upright)')]+=1
                if col and col.lower() not in ('#000000',):
                    seen_texts.append((col,sl,txt[:50]))
    print('== %s'%path.split('/')[-1])
    for (c,s),n in counts.most_common():
        print('   colour %-10s slant %-10s spans %d'%(c,s,n))
    for c,s,t in seen_texts[:15]:
        print('    non-black: %s %s %r'%(c,s,t))
    print('    non-black spans: %d'%len(seen_texts))
