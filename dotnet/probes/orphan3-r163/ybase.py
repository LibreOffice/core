import re,sys,subprocess
PDF=sys.argv[1]; PAGE=sys.argv[2]
out=subprocess.run(["python3","/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-ops.py","dump",PDF,"--page",PAGE],capture_output=True,text=True).stdout
ys={}
for ln in out.splitlines():
    m=re.match(r'text\s+p\d+\s+\(\s*([\d.]+),\s*([\d.]+)\)\s+([\d.]+)pt',ln)
    if not m: continue
    x=float(m.group(1)); y=float(m.group(2)); sz=float(m.group(3))
    txt=ln.split('show(s)',1)[1].strip() if 'show(s)' in ln else ''
    ys.setdefault(round(y,2),[]).append((x,txt))
for y in sorted(ys,reverse=True):
    items=sorted(ys[y])
    print(f"{y:8.2f}  " + " | ".join(t[:40] for _,t in items)[:150])
