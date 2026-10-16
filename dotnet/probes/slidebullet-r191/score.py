#!/usr/bin/env python3
"""Worst-page and summed diff% for each mover, before and after, from pdf-image-diff's own table."""
import hashlib, pathlib, re, subprocess, sys
D='/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-image-diff.py'
ROW=re.compile(r'^(\d+)\t([\d.]+)\t([-\d.]+)\t([\d.]+)\t(\d+)\t(\w+)$')
def score(ours, ref, tag):
    out=subprocess.run(['python3',D,str(ours),str(ref),'--outdir',f'/tmp/s10-{tag}'],
                       capture_output=True,text=True).stdout
    pages={}
    for line in out.splitlines():
        m=ROW.match(line.rstrip())
        if m: pages[int(m.group(1))]=(float(m.group(2)), m.group(6))
    d=[v[0] for v in pages.values()]
    return (max(d,default=0.0), sum(d), len(d), sum(1 for v in pages.values() if v[1]=='MAJOR'), pages)
docs=[l.strip() for l in open('ss/movers9.txt') if l.strip()]
tw=tsb=tsa=0.0; mb=ma=0
for i,doc in enumerate(docs):
    stem=pathlib.Path(doc).stem
    ref=pathlib.Path('ss/ref10')/f'{stem}.pdf'
    key=hashlib.md5(doc.encode()).hexdigest()[:12]
    b=sorted((pathlib.Path('ss/sweep-before')/key).glob('*.pdf'))[0]
    a=sorted((pathlib.Path('ss/sweep-after2')/key).glob('*.pdf'))[0]
    B=score(b,ref,f'{i}b'); A=score(a,ref,f'{i}a')
    tsb+=B[1]; tsa+=A[1]; mb+=B[3]; ma+=A[3]
    flag='  <= worse' if A[1] > B[1]+0.05 else ('  better' if A[1] < B[1]-0.05 else '')
    print(f'{stem[:44]:46s} worst {B[0]:6.2f}->{A[0]:6.2f}  sum {B[1]:8.2f}->{A[1]:8.2f}  '
          f'MAJOR {B[3]}->{A[3]}  pages {B[2]}{flag}')
print(f'{"TOTAL":46s} {"":18s}  sum {tsb:8.2f}->{tsa:8.2f}  MAJOR {mb}->{ma}')
