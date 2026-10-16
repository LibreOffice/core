#!/usr/bin/env python3
"""Census 2: a master page's running objects, with LibreOffice's own defaults and with
`presentation:footer`/`-header`/`-date-time` fields resolved through the page's decls.

Defaults are HeaderFooterSettings() (sd/source/core/sdpage.cxx:3222-3230): header true,
footer true, date-time true, slide number FALSE.

Environment: corpus /home/user/corpus-odf, XML only, nothing rendered.
"""
import sys, zipfile
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

RUNNING = {'footer':True, 'date-time':True, 'page-number':False, 'header':True}
ATTR = {'footer':'display-footer','date-time':'display-date-time',
        'page-number':'display-page-number','header':'display-header'}
USE  = {'footer':'use-footer-name','date-time':'use-date-time-name','header':'use-header-name'}

def alnum(s): return sum(1 for c in s if c.isalnum())

def displayed(el):
    d = el.get(q('drawooo:display')) or el.get(q('draw:display'))
    return d in (None,'always','printer')

def frame_text(el, decls, pageno):
    """Text the frame draws: literal characters, with the three decl fields resolved from the
    page and text:page-number replaced by the slide number."""
    out=[]
    def walk(e):
        for ch in e:
            t = ch.tag
            if t == q('text:page-number'):
                out.append(str(pageno))
            elif t == q('text:s'):
                out.append(' ')
            elif t == q('pres:footer'):
                out.append(decls.get('footer',''))
            elif t == q('pres:header'):
                out.append(decls.get('header',''))
            elif t == q('pres:date-time'):
                out.append(decls.get('date-time',''))
            else:
                if ch.text: out.append(ch.text)
                walk(ch)
            if ch.tail: out.append(ch.tail)
    if el.text: out.append(el.text)
    walk(el)
    return ''.join(out)

def main(corpus, out):
    corpus = Path(corpus)
    rows=[]
    for doc in sorted(corpus.rglob('*.odp')):
        rel = str(doc.relative_to(corpus))
        try:
            z = zipfile.ZipFile(doc)
            styles = ET.fromstring(z.read('styles.xml'))
            content = ET.fromstring(z.read('content.xml'))
        except Exception as e:
            rows.append([rel,'ERR',str(e)]); continue

        dpstyles={}
        for root in (styles, content):
            for st in root.iter(q('style:style')):
                if st.get(q('style:family')) != 'drawing-page': continue
                pp = st.find(q('style:drawing-page-properties'))
                dpstyles[st.get(q('style:name'))]={
                    'parent':st.get(q('style:parent-style-name')),
                    'p':dict(pp.attrib) if pp is not None else {}}
        def prop(name, attr):
            seen=set()
            while name and name in dpstyles and name not in seen:
                seen.add(name); v = dpstyles[name]['p'].get(attr)
                if v is not None: return v
                name = dpstyles[name]['parent']
            return None

        # document-level decls
        decl_by_name={}
        for kind in ('header','footer','date-time'):
            for d in content.iter(q('pres:%s-decl'%kind)):
                decl_by_name[(kind, d.get(q('pres:name')))] = ''.join(d.itertext())

        masters={}
        for mp in styles.iter(q('style:master-page')):
            items=[]
            for el in mp:
                if el.tag == q('pres:notes'): continue
                cls = el.get(q('pres:class'))
                if cls in RUNNING and displayed(el):
                    items.append((cls, el))
            masters[mp.get(q('style:name'))]=items

        body = content.find(q('office:body'))
        pres = body.find(q('office:presentation')) if body is not None else None
        pages = list(pres.iter(q('draw:page'))) if pres is not None else []

        tot=0; stated=0; defaulted=0; npages=0
        on={k:0 for k in RUNNING}
        for i,pg in enumerate(pages, start=1):
            npages+=1
            dp = pg.get(q('draw:style-name'))
            decls={}
            for kind in ('header','footer','date-time'):
                nm = pg.get(q('pres:'+USE[kind]))
                if nm is not None: decls[kind]=decl_by_name.get((kind,nm),'')
            for cls, el in masters.get(pg.get(q('draw:master-page-name')),[]):
                v = prop(dp, q('pres:'+ATTR[cls]))
                if v is None:
                    vis = RUNNING[cls]; defaulted += 1
                else:
                    vis = (v == 'true'); stated += 1
                if not vis: continue
                on[cls]+=1
                tot += alnum(frame_text(el, decls, i))
        rows.append([rel,'OK',npages,tot,on['footer'],on['date-time'],on['page-number'],
                     on['header'],stated,defaulted])
    with open(out,'w') as fh:
        fh.write('# corpus=%s  XML census only, nothing rendered.\n' % corpus)
        fh.write('# defaults: header/footer/date-time visible, page-number NOT (HeaderFooterSettings())\n')
        fh.write('path\tstatus\tpages\trunning_alnum\ton_footer\ton_date\ton_pagenum\ton_header\tstated\tdefaulted\n')
        for r in rows: fh.write('\t'.join(str(x) for x in r)+'\n')
    print('rows', len(rows))

main(sys.argv[1], sys.argv[2])
