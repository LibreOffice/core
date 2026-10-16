import re,sys,subprocess
PDF=sys.argv[1]; lo=int(sys.argv[2]); hi=int(sys.argv[3])
for p in range(lo,hi+1):
    out=subprocess.run(["python3","/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-ops.py","dump",PDF,"--page",str(p)],capture_output=True,text=True).stdout
    ys={}
    for ln in out.splitlines():
        m=re.match(r'text\s+p\d+\s+\(\s*([\d.]+),\s*([\d.]+)\)',ln)
        if not m: continue
        x=float(m.group(1)); y=float(m.group(2))
        txt=ln.split('show(s)',1)[1].strip().strip('"') if 'show(s)' in ln else ''
        ys.setdefault(round(y,2),[]).append((x,txt))
    body=[y for y in ys if 60.0 < y < 745.0]
    body.sort(reverse=True)
    if not body: print(p,"empty"); continue
    first=sorted(ys[body[0]])[0][1][:38]
    last=sorted(ys[body[-1]])[0][1][:38]
    print(f"p{p:3d} n={len(body):3d} top={body[0]:7.2f} bot={body[-1]:7.2f}  first=[{first}]  last=[{last}]")
