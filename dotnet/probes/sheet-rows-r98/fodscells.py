#!/usr/bin/env python3
import sys
import xml.etree.ElementTree as ET
NS={'office':'urn:oasis:names:tc:opendocument:xmlns:office:1.0',
 'style':'urn:oasis:names:tc:opendocument:xmlns:style:1.0',
 'table':'urn:oasis:names:tc:opendocument:xmlns:table:1.0',
 'text':'urn:oasis:names:tc:opendocument:xmlns:text:1.0',
 'fo':'urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0'}
def q(t):
    p,l=t.split(':'); return '{%s}%s'%(NS[p],l)
def colname(i):
    s=''
    i+=1
    while i: i,r=divmod(i-1,26); s=chr(65+r)+s
    return s
path,sheet=sys.argv[1],sys.argv[2]
lo,hi=int(sys.argv[3]),int(sys.argv[4])
t=ET.parse(path); r=t.getroot()
body=r.find(q('office:body')).find(q('office:spreadsheet'))
for tb in body.findall(q('table:table')):
    if tb.get(q('table:name'))!=sheet: continue
    idx=0
    for ch in tb:
        if ch.tag!=q('table:table-row'): continue
        rep=int(ch.get(q('table:number-rows-repeated'),'1'))
        if idx+rep-1>=lo and idx<=hi:
            print('--- row %d (rep %d) style=%s'%(idx,rep,ch.get(q('table:style-name'))))
            c=0
            for cell in ch:
                crep=int(cell.get(q('table:number-columns-repeated'),'1'))
                sn=cell.get(q('table:style-name'))
                vt=cell.get(q('office:value-type'))
                f=cell.get(q('table:formula'))
                txt=''.join(cell.itertext())
                if vt or f or txt.strip():
                    print('    %s%s%s sty=%s vt=%s f=%s txt=%r np=%d'%(
                       colname(c),'' if crep==1 else ':'+colname(c+crep-1),
                       '', sn,vt,(f or '')[:60],txt[:30],len(cell.findall(q('text:p')))))
                c+=crep
        idx+=rep
        if idx>hi: break
