#!/usr/bin/env python3
"""Score both legs of a change against a freshly rendered reference and report the movement.

    ./score.py   (paths are the round's own; edit them or copy the shape)

Prints, per document: worst-page diff% before and after, summed diff% before and after, MAJOR
pages before and after, page count, path. Both legs must be rendered with one output directory
per DOCUMENT -- see r187/par.sh -- and the reference half is rendered once and reused, which is
sound whenever the diff under test is confined to dotnet/src.
"""
import hashlib, pathlib, re, subprocess, sys
B=pathlib.Path('/home/user/r190/sl-before'); A=pathlib.Path('/home/user/r190/sl-after')
R=pathlib.Path('/home/user/r190/sl-ref')
DIFF='/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-image-diff.py'
def only(d):
    p=sorted(d.glob('*.pdf')) if d.is_dir() else []
    return p[0] if p else None
def score(pdf, ref, tag):
    out=subprocess.run(['python3',DIFF,str(pdf),str(ref),'--outdir',f'/tmp/s-{tag}'],
                       capture_output=True,text=True,timeout=1800).stdout
    subprocess.run(['rm','-rf',f'/tmp/s-{tag}'])
    worst=0.0; total=0.0; pages=0; major=0
    for line in out.splitlines():
        f=line.split('\t')
        if len(f)>=6 and re.fullmatch(r'\d+',f[0]):
            pages+=1; d=float(f[1]); total+=d; worst=max(worst,d)
            if 'MAJOR' in f[5]: major+=1
    return worst,total,pages,major
for path in [l.strip() for l in open('mv-targets.txt') if l.strip()]:
    i=hashlib.md5(path.encode()).hexdigest()[:12]
    b,a,r=only(B/i),only(A/i),R/f'{i}.pdf'
    if b is None or a is None or not r.exists():
        print(f'-\t-\t{path}\tmissing', flush=True); continue
    wb,tb,pb,mb=score(b,r,i+'b'); wa,ta,pa,ma=score(a,r,i+'a')
    print(f'{wb:.2f}\t{wa:.2f}\t{tb:.2f}\t{ta:.2f}\t{mb}\t{ma}\t{pb}\t{path}', flush=True)
