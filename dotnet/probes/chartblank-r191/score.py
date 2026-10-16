#!/usr/bin/env python3
"""Worst-page and summed diff% for each mover, before and after, from pdf-image-diff's own table."""
import hashlib, pathlib, re, subprocess, sys
D='/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-image-diff.py'
docs=[l.strip() for l in open('cb/slides-charts.txt') if l.strip()]
row=re.compile(r'^(\d+)\t([\d.]+)\t([-\d.]+)\t([\d.]+)\t(\d+)\t(\w+)$')
def score(ours, ref, tag):
    out=subprocess.run(['python3',D,str(ours),str(ref),'--outdir',f'/tmp/pid-{tag}'],
                       capture_output=True,text=True).stdout
    pages={}
    for line in out.splitlines():
        m=row.match(line.rstrip())
        if m: pages[int(m.group(1))]=(float(m.group(2)), m.group(6))
    diffs=[v[0] for v in pages.values()]
    major=sum(1 for v in pages.values() if v[1]=='MAJOR')
    return max(diffs, default=0.0), sum(diffs), len(diffs), major, pages
for n in sys.argv[1:]:
    src=[d for d in docs if d.endswith('/'+n+'.pptx')][0]
    k=hashlib.md5(src.encode()).hexdigest()[:12]
    ref=pathlib.Path('cb/ref')/f'{n}.pdf'
    legs={}
    for leg in ('before','after'):
        p=list((pathlib.Path('cb')/leg/k).glob('*.pdf'))[0]
        legs[leg]=score(p,ref,f'{n}-{leg}')
    b,a=legs['before'],legs['after']
    print(f'{n[:46]:46s} worst {b[0]:6.2f}->{a[0]:6.2f}  sum {b[1]:8.2f}->{a[1]:8.2f}  MAJOR {b[3]}->{a[3]}  pages {b[2]}')
    for pg in sorted(b[4]):
        if abs(b[4][pg][0]-a[4].get(pg,(0,))[0])>0.005:
            print(f'      page {pg}: {b[4][pg][0]:6.2f} -> {a[4][pg][0]:6.2f}')
