#!/usr/bin/env python3
"""Print resolved row heights (twips) per sheet out of a flat-ODF spreadsheet.

The reference's own view: soffice --convert-to fods writes every row's
style:row-height into an automatic <style:style style:family="table-row">.
"""
import sys, re
import xml.etree.ElementTree as ET

NS = {
 'office':'urn:oasis:names:tc:opendocument:xmlns:office:1.0',
 'style':'urn:oasis:names:tc:opendocument:xmlns:style:1.0',
 'table':'urn:oasis:names:tc:opendocument:xmlns:table:1.0',
 'text':'urn:oasis:names:tc:opendocument:xmlns:text:1.0',
 'fo':'urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0',
 'calcext':'urn:org:documentfoundation:names:experimental:calc:xmlns:calcext:1.0',
}
for k,v in NS.items(): ET.register_namespace(k,v)
def q(t):
    p,l=t.split(':'); return '{%s}%s'%(NS[p],l)

UNIT=re.compile(r'^([-0-9.]+)([a-z]*)$')
def twips(s):
    m=UNIT.match(s.strip())
    v=float(m.group(1)); u=m.group(2)
    return {'in':1440.0,'cm':1440/2.54,'mm':144/2.54,'pt':20.0,'pc':240.0,
            'twip':1.0,'':1.0}[u]*v

def rowheights(path):
    t=ET.parse(path); r=t.getroot()
    styles={}
    for st in r.iter(q('style:style')):
        if st.get(q('style:family'))!='table-row': continue
        rp=st.find(q('style:table-row-properties'))
        if rp is None: continue
        h=rp.get(q('style:row-height'))
        opt=rp.get(q('style:use-optimal-row-height'))
        styles[st.get(q('style:name'))]=(twips(h) if h else None, opt)
    out=[]
    body=r.find(q('office:body')).find(q('office:spreadsheet'))
    for tb in body.findall(q('table:table')):
        name=tb.get(q('table:name'))
        rows=[]
        idx=0
        def walk(node):
            nonlocal idx
            for ch in node:
                if ch.tag==q('table:table-row'):
                    rep=int(ch.get(q('table:number-rows-repeated'),'1'))
                    sn=ch.get(q('table:style-name'))
                    h,opt=styles.get(sn,(None,None))
                    ncells=sum(int(c.get(q('table:number-columns-repeated'),'1'))
                               for c in ch if c.tag in (q('table:table-cell'),q('table:covered-table-cell')))
                    nfull=sum(1 for c in ch if c.tag==q('table:table-cell') and len(c))
                    rows.append((idx,rep,sn,h,opt,nfull))
                    idx+=rep
                elif ch.tag in (q('table:table-row-group'),q('table:table-header-rows'),q('table:table-rows')):
                    walk(ch)
        walk(tb)
        out.append((name,rows))
    return out

if __name__=='__main__':
    for name,rows in rowheights(sys.argv[1]):
        if len(sys.argv)>2 and sys.argv[2]!=name: continue
        print('== sheet %s'%name)
        for idx,rep,sn,h,opt,nfull in rows:
            print('  rows %5d-%-5d  %-10s  %8s  opt=%-5s cells=%d'%(idx,idx+rep-1,sn,
                  ('%.1f'%h) if h is not None else '-', opt, nfull))
