import sys, re
sys.path.insert(0, "/tmp/claude-0/-c-sandbox-workdir/5cf9a493-a19e-4a73-944b-74fd16d25b38/scratchpad/words")
from pathlib import Path
from tfsize import sizes

SP="/tmp/claude-0/-c-sandbox-workdir/5cf9a493-a19e-4a73-944b-74fd16d25b38/scratchpad/words2/fw"
OLD="/tmp/claude-0/-c-sandbox-workdir/5cf9a493-a19e-4a73-944b-74fd16d25b38/scratchpad/words/fw"

def authored(fodt):
    return [float(x) for x in re.findall(r'fo:font-size="([0-9.]+)pt"', Path(fodt).read_text())]

def rnd(x):  # half away from zero
    import math
    return math.floor(x + 0.5) if x >= 0 else math.ceil(x - 0.5)

names = ["w-whole", "w-fine8", "w-fine11", "w-thirds", "w-hund"]
allpairs = []
same = diff = 0
for n in names:
    a = authored(f"{SP}/{n}.fodt")
    new = sizes(Path(f"{SP}/{n}.pdf"))
    old = sizes(Path(f"{OLD}/{n}.pdf"))
    assert len(new) == len(a), (n, len(new), len(a))
    for x, y in zip(new, old):
        if abs(x - y) < 1e-9: same += 1
        else: diff += 1
    allpairs += list(zip(a, new))
    print(f"{n:10s} {len(a):3d} sizes  new==old on {sum(1 for x,y in zip(new,old) if abs(x-y)<1e-9)}/{len(old)}")
print(f"TOTAL observations {len(allpairs)}: drawn size identical to the font-starved render on {same}, differs on {diff}")

def modelC(pt, dpi):
    tw = rnd(pt * 20)
    return rnd(tw * dpi / (72 * 20)) * 72.0 / dpi
def modelA(pt, dpi):          # sheets law: snap to 1/100 mm first
    hmm = rnd(pt * 2540.0 / 72.0)
    p = hmm * 72.0 / 2540.0
    return rnd(p * dpi / 72.0) * 72.0 / dpi
def modelD(pt, dpi):
    return rnd(pt * dpi / 72.0) * 72.0 / dpi

for label, m in (("A (1/100 mm then 720dpi)", modelA), ("C (twips then 720dpi)", modelC), ("D (720dpi only)", modelD)):
    ok = sum(1 for pt, drawn in allpairs if abs(m(pt, 720) - drawn) < 0.005)
    print(f"  {label:26s} reproduces {ok}/{len(allpairs)} at 720 dpi")
best = []
for dpi in range(30, 4001):
    ok = sum(1 for pt, drawn in allpairs if abs(modelC(pt, dpi) - drawn) < 0.005)
    best.append((ok, dpi))
best.sort(reverse=True)
print("  model C swept 30-4000 dpi, top 5:", best[:5])

# The sheets law transcribed faithfully from sheets-r23/README.md lines 21-23:
#   h = round(twips*127/72)  -> 1/100 mm ; p = round(h*720/2540) ; L = round(p*2540/720)
def modelA2(pt, dpi=720, scale=1.0):
    tw = rnd(pt * 20)
    h  = rnd(tw * 127 / 72)
    p  = rnd(h * scale * dpi / 2540)
    L  = rnd(p * 2540 / (dpi * scale))
    return L * 72.0 / 2540.0
ok = sum(1 for pt, drawn in allpairs if abs(modelA2(pt) - drawn) < 0.005)
print(f"  A' (sheets-r23 README verbatim)  reproduces {ok}/{len(allpairs)} at 720 dpi")
# where do C and A' disagree?
dis = [(pt, drawn, modelA2(pt), modelC(pt,720)) for pt, drawn in allpairs if abs(modelA2(pt)-drawn) >= 0.005]
print(f"  A' misses {len(dis)}; first few (authored, drawn, A', C):", [(round(a,3), b, round(c,4), d) for a,b,c,d in dis[:5]])
