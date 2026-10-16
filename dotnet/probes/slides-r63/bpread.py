import sys
sys.path.insert(0,'/c/sandbox/workdir/scratch-r56-slides')
sys.path.insert(0,'/c/sandbox/workdir/scratch-r62-slides')
from rotruns import runs
from pg import page_stream
BOXTOP = 540 - 900000/12700.0
def lines(path, pg):
    rs = [r for r in runs(page_stream(path, pg)) if r[4].strip() and r[1] < BOXTOP+1]
    rs.sort(key=lambda r: (-r[1], r[0]))
    out=[]
    for r in rs:
        for L in out:
            if abs(L[0][1]-r[1]) < 8:      # same line: bullet may sit a couple of pt off
                L.append(r); break
        else:
            out.append([r])
    for L in out: L.sort(key=lambda r: r[0])
    return out
def summarise(path, pg):
    L = lines(path, pg)
    if not L: return None
    first = L[0]
    text = max(first, key=lambda r: len(r[4]))
    bullet = first[0] if first[0] is not text else None
    # pitch: text baseline of line 0 vs line 1
    p = 0.0
    if len(L) > 1:
        t2 = max(L[1], key=lambda r: len(r[4]))
        p = text[1] - t2[1]
    return bullet, text, p
CASES=open('cases.txt').read().split('\n') if False else None
for pg in range(21):
    so = summarise('bp-ours/bullet-probe.pdf', pg)
    sr = summarise('bp/bullet-probe.pdf', pg)
    def fmt(s):
        b,t,p = s
        return (f"asc {BOXTOP-t[1]:7.3f} pitch {p:7.3f} sz {t[3]:6.3f}"
                + (f"  bullet sz {b[3]:6.3f} d {b[1]-t[1]:+7.3f}" if b else "  (no bullet)"))
    a, c = fmt(so), fmt(sr)
    print(f"p{pg+1:2d} OURS {a}")
    print(f"    REF  {c}   {'<<<' if a!=c else ''}")
