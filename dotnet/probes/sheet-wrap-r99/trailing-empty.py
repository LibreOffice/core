#!/usr/bin/env python3
"""Census: cells of a flat ODF whose last <text:p> is empty, i.e. that end in a
trailing empty paragraph. Reports per sheet/row, and whether the cell is rich
(more than one text:span or any span with its own style)."""
import sys
import xml.etree.ElementTree as ET

NS={'office':'urn:oasis:names:tc:opendocument:xmlns:office:1.0',
 'table':'urn:oasis:names:tc:opendocument:xmlns:table:1.0',
 'text':'urn:oasis:names:tc:opendocument:xmlns:text:1.0'}
def q(t):
    p,l=t.split(':'); return '{%s}%s'%(NS[p],l)

def cells(path):
    for ev, el in ET.iterparse(path, events=('start','end')):
        pass

def census(path, rowsonly=None):
    t=ET.parse(path); r=t.getroot()
    body=r.find(q('office:body')).find(q('office:spreadsheet'))
    out=[]
    for tb in body.findall(q('table:table')):
        name=tb.get(q('table:name'))
        idx=0
        for ch in tb:
            if ch.tag!=q('table:table-row'):
                continue
            rep=int(ch.get(q('table:number-rows-repeated'),'1'))
            col=0
            for c in ch:
                crep=int(c.get(q('table:number-columns-repeated'),'1'))
                if c.tag==q('table:table-cell'):
                    ps=c.findall(q('text:p'))
                    if len(ps)>1:
                        last=ps[-1]
                        empty = ''.join(last.itertext())=='' 
                        spans=sum(len(p.findall(q('text:span'))) for p in ps)
                        if empty:
                            out.append((name, idx, col, len(ps), spans))
                col+=crep
            idx+=rep
    return out

if __name__=='__main__':
    rows=census(sys.argv[1])
    print('cells whose last of >1 text:p is empty: %d'%len(rows))
    import collections
    bysheet=collections.Counter(s for s,_,_,_,_ in rows)
    for s,n in bysheet.most_common(): print('   %-32s %d'%(s,n))
    print('distinct (sheet,row): %d'%len({(s,r) for s,r,_,_,_ in rows}))
    rich=[x for x in rows if x[4]>0]
    print('of those, holding at least one text:span: %d'%len(rich))
    for x in rows: print("   ",x)
