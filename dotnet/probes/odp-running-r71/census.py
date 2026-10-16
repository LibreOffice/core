#!/usr/bin/env python3
"""Census the .odp column for a master page's running objects.

Environment: corpus /home/user/corpus-odf (LibreOffice 26.2.4.2's own export of the
sample corpus), read as XML only -- nothing is rendered here.
"""
import re, sys, zipfile
from pathlib import Path
from xml.etree import ElementTree as ET

NS = {
 'office':'urn:oasis:names:tc:opendocument:xmlns:office:1.0',
 'style':'urn:oasis:names:tc:opendocument:xmlns:style:1.0',
 'draw':'urn:oasis:names:tc:opendocument:xmlns:drawing:1.0',
 'pres':'urn:oasis:names:tc:opendocument:xmlns:presentation:1.0',
 'text':'urn:oasis:names:tc:opendocument:xmlns:text:1.0',
 'drawooo':'http://openoffice.org/2010/draw',
}
def q(p):
    a,b = p.split(':'); return '{%s}%s' % (NS[a], b)

RUNNING = {'footer','date-time','page-number','header'}

def texts(el):
    """All character data under el, with text:page-number replaced by '#'."""
    out=[]
    def walk(e):
        for ch in e:
            if ch.tag == q('text:page-number'):
                out.append('#')
            elif ch.tag == q('text:s'):
                out.append(' ')
            else:
                if ch.text: out.append(ch.text)
                walk(ch)
            if ch.tail: out.append(ch.tail)
    if el.text: out.append(el.text)
    walk(el)
    return ''.join(out)

def alnum(s): return sum(1 for c in s if c.isalnum())

def displayed(el):
    d = el.get(q('drawooo:display')) or el.get(q('draw:display'))
    return d in (None,'always','printer')

def main(corpus, out):
    corpus = Path(corpus)
    docs = sorted(p for p in corpus.rglob('*.odp'))
    rows=[]
    for doc in docs:
        rel = str(doc.relative_to(corpus))
        try:
            z = zipfile.ZipFile(doc)
            styles = ET.fromstring(z.read('styles.xml'))
            content = ET.fromstring(z.read('content.xml'))
        except Exception as e:
            rows.append((rel,'ERR',str(e))); continue

        # drawing-page styles, name -> {prop: value}, from both files, with parent chains
        dpstyles={}
        for root in (styles, content):
            for st in root.iter(q('style:style')):
                if st.get(q('style:family')) != 'drawing-page': continue
                props={}
                pp = st.find(q('style:drawing-page-properties'))
                if pp is not None: props=dict(pp.attrib)
                dpstyles[st.get(q('style:name'))]={'parent':st.get(q('style:parent-style-name')),'p':props}
        def prop(name, attr):
            seen=set()
            while name and name in dpstyles and name not in seen:
                seen.add(name)
                v = dpstyles[name]['p'].get(attr)
                if v is not None: return v
                name = dpstyles[name]['parent']
            return None

        # masters: name -> list of (class, text, alnum, displayed)
        masters={}
        for mp in styles.iter(q('style:master-page')):
            name = mp.get(q('style:name'))
            items=[]
            for el in mp:
                if el.tag == q('pres:notes'): continue
                cls = el.get(q('pres:class'))
                if cls in RUNNING:
                    t = texts(el)
                    items.append((cls, t, alnum(t.replace('#','')), displayed(el),
                                  '#' in t, el.find('.//'+q('pres:footer')) is not None
                                            or el.find('.//'+q('pres:header')) is not None
                                            or el.find('.//'+q('pres:date-time')) is not None))
            masters[name]=items

        body = content.find(q('office:body'))
        pres = body.find(q('office:presentation')) if body is not None else None
        pages = list(pres.iter(q('draw:page'))) if pres is not None else []
        tot=0; npages=0; on={'footer':0,'date-time':0,'page-number':0,'header':0}
        own={'footer':0,'date-time':0,'page-number':0,'header':0}
        fields=0; pagenumfields=0
        for pg in pages:
            npages+=1
            dp = pg.get(q('draw:style-name'))
            mname = pg.get(q('draw:master-page-name'))
            # a slide's own running frames
            for el in pg:
                c = el.get(q('pres:class'))
                if c in RUNNING: own[c]+=1
            for (cls, t, a, disp, haspn, hasfield) in masters.get(mname,[]):
                if not disp: continue
                key = {'footer':'display-footer','date-time':'display-date-time',
                       'page-number':'display-page-number','header':'display-header'}[cls]
                v = prop(dp, q('pres:'+key))
                if v == 'true':
                    on[cls]+=1
                    tot += a + (len(str(npages)) if haspn else 0)
                    if hasfield: fields+=1
                    if haspn: pagenumfields+=1
        rows.append((rel,'OK',npages,tot,on['footer'],on['date-time'],on['page-number'],on['header'],
                     own['footer'],own['date-time'],own['page-number'],own['header'],fields,pagenumfields))
    with open(out,'w') as fh:
        fh.write('# corpus=%s  XML census only, nothing rendered\n' % corpus)
        fh.write('path\tstatus\tpages\trunning_alnum\ton_footer\ton_date\ton_pagenum\ton_header\town_footer\town_date\town_pagenum\town_header\tdecl_fields\tpagenum_fields\n')
        for r in rows:
            fh.write('\t'.join(str(x) for x in r)+'\n')
    print('rows', len(rows))

main(sys.argv[1], sys.argv[2])
