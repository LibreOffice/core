import subprocess,re,sys,glob
def dump(pdf,page):
    return subprocess.run(["python3","/home/user/libreoffice-core/.claude/skills/render-comparison/scripts/pdf-ops.py","dump",pdf,"--page",str(page)],capture_output=True,text=True).stdout
def rows(pdf,pages):
    data=[]
    for p in pages:
        out=dump(pdf,p)
        rules=[]; ys=set()
        for ln in out.splitlines():
            m=re.match(r'stroke\s+p\d+\s+\(\s*([\d.]+),\s*([\d.]+)\)-\(\s*([\d.]+),\s*([\d.]+)\)',ln)
            if m:
                x0,y0,x1,y1=[float(g) for g in m.groups()]
                if abs(y0-y1)<0.01 and x1-x0>300 and 40<y0<745: rules.append(round(y0,2))
            m=re.match(r'text\s+p\d+\s+\(\s*([\d.]+),\s*([\d.]+)\)',ln)
            if m:
                y=float(m.group(2))
                if 40<y<745: ys.add(round(y,2))
        rules=sorted(set(rules),reverse=True); ys=sorted(ys,reverse=True)
        for i in range(len(rules)-1):
            top,bot=rules[i],rules[i+1]
            n=sum(1 for y in ys if bot-1 < y < top-1)
            data.append((n, top-bot))
    return data
import statistics
for label,pdf in (("REF",glob.glob('ref/*/*.pdf')[0]),("OURS",glob.glob('ours/*.pdf')[0])):
    d=rows(pdf,range(4,13))
    buckets={}
    for n,h in d: buckets.setdefault(n,[]).append(h)
    print(label)
    for n in sorted(buckets):
        v=buckets[n]
        print(f"  lines={n} count={len(v):3d} mean={statistics.mean(v):8.4f} min={min(v):7.2f} max={max(v):7.2f}")
