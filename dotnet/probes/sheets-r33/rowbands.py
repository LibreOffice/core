#!/usr/bin/env python3
"""Simulate ScTable::UpdatePageBreaks down the rows, for a stated page height."""
import re, sys
FODS, TABLE = sys.argv[1], sys.argv[2]
LAST = int(sys.argv[3])
REPEAT = (int(sys.argv[4]), int(sys.argv[5]))   # repeated row band, or (-1,-1)
s=open(FODS,encoding='utf-8').read()
styles={}
for m in re.finditer(r'<style:style style:name="(ro\d+)"[^>]*>\s*<style:table-row-properties([^>]*)/>', s):
    w=re.search(r'style:row-height="([\d.]+)(in|cm|mm|pt)"', m.group(2))
    v,u=float(w.group(1)),w.group(2)
    styles[m.group(1)]=v*{'in':1440,'cm':1440/2.54,'mm':144/2.54,'pt':20}[u]
tm=re.search(r'<table:table table:name="%s"[^>]*>(.*?)</table:table>'%re.escape(TABLE), s, re.S)
body=tm.group(1)
heights=[]; hidden=[]
for rm in re.finditer(r'<table:table-row([^>]*)>', body):
    a=rm.group(1)
    st=re.search(r'table:style-name="(ro\d+)"',a)
    rep=re.search(r'table:number-rows-repeated="(\d+)"',a)
    n=int(rep.group(1)) if rep else 1
    h=styles.get(st.group(1),0) if st else 0
    hid='table:visibility="collapse"' in a or 'table:visibility="filter"' in a
    for k in range(min(n, LAST+1-len(heights))):
        heights.append(h); hidden.append(hid)
    if len(heights)>LAST: break
def bands(pageSize, rnd=round):
    ps=pageSize; running=0; breaks=set(); done=False
    for i in range(0, LAST+1):
        size = 0 if hidden[i] else rnd(heights[i])
        start=False
        if running + size > ps:
            breaks.add(i); running=0; start=True
        elif i!=0: breaks.discard(i)
        else: start=True
        if start and REPEAT[0]>=0 and i>REPEAT[0] and not done:
            ps -= sum(0 if hidden[j] else rnd(heights[j]) for j in range(REPEAT[0],REPEAT[1]+1))
            ps = max(1,ps)
            while i<=REPEAT[1]:
                i+=1; breaks.discard(i)
            done=True
        running += size
    ends=[]; saw=False
    for i in range(0, LAST+1):
        if i>0 and saw and i in breaks:
            ends.append(i-1); saw=False
        if not hidden[i]: saw=True
    if saw: ends.append(LAST)
    return ends
for ph in [int(x) for x in sys.argv[6].split(',')]:
    e=bands(ph)
    print('pageHeight %6d -> %2d bands  ends %s' % (ph, len(e), e))
