#!/usr/bin/env python3
import sys, pymupdf
a,b,pno=sys.argv[1],sys.argv[2],int(sys.argv[3])
for tag,path in (('ours',a),('ref',b)):
    d=pymupdf.open(path); p=d[pno-1]
    print('== %s  page size %s'%(tag,p.rect))
    n=0
    for blk in p.get_text('dict')['blocks']:
        for ln in blk.get('lines',[]):
            for sp in ln['spans']:
                n+=1
                if n<=25:
                    print('   x %7.2f y %7.2f sz %5.2f %-22s %r'%(sp['bbox'][0],sp['bbox'][1],sp['size'],sp['font'][:22],sp['text'][:40]))
    print('   total spans %d'%n)
    d.close()
