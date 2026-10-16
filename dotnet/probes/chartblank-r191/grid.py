import re, subprocess, collections
S='/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-ops.py'
pat=re.compile(r'stroke p\d+\s+\(\s*([-\d.]+),\s*([-\d.]+)\)-\(\s*([-\d.]+),\s*([-\d.]+)\)\s+(#\w+)')
for side in ('ours','ref'):
    out=subprocess.run(['python3',S,'dump',f'dj/{side}/Demick_JetBlue.pdf','--page','5','--only','stroke'],capture_output=True,text=True).stdout
    vert=collections.defaultdict(list);horz=collections.defaultdict(list)
    for m in pat.finditer(out):
        x0,y0,x1,y1=(float(m.group(i)) for i in (1,2,3,4)); c=m.group(5)
        if abs(x1-x0)<0.05 and abs(y1-y0)>50: vert[c].append((x0,y0,y1))
        if abs(y1-y0)<0.05 and abs(x1-x0)>50: horz[c].append((y0,x0,x1))
    print(f'=== {side}')
    for c,v in sorted(vert.items(), key=lambda kv:-len(kv[1])):
        v.sort(); print(f'  V {c} n={len(v)} x {v[0][0]:.2f}..{v[-1][0]:.2f} y {v[0][1]:.2f}..{v[0][2]:.2f} pitch {(v[-1][0]-v[0][0])/max(1,len(v)-1):.4f}')
    for c,h in sorted(horz.items(), key=lambda kv:-len(kv[1])):
        h.sort(); print(f'  H {c} n={len(h)} y {h[0][0]:.2f}..{h[-1][0]:.2f} x {h[0][1]:.2f}..{h[0][2]:.2f} pitch {(h[-1][0]-h[0][0])/max(1,len(h)-1):.4f}')
