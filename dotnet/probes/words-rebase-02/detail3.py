import sys, os
sys.path.insert(0, "/tmp/claude-0/-c-sandbox-workdir/5cf9a493-a19e-4a73-944b-74fd16d25b38/scratchpad/words2")
from decompose import *
print()
keys = sorted(ref_F)
def i(x):
    try: return int(x)
    except: return 0

dO = {k: i(ours_head_F[k][1]) - i(ours_r47[k][1]) for k in keys}   # ours words, r47 -> HEAD+f
dR = {k: i(ref_F[k][1])       - i(ref_24[k][1])   for k in keys}   # ref  words, 24.2 -> 26.2+f
pos = sum(1 for k in keys if dO[k] > 0); neg = sum(1 for k in keys if dO[k] < 0)
print(f"ours word delta r47->HEAD: {pos} up, {neg} down, {200-pos-neg} unchanged; sum {sum(dO.values())}, |sum| {sum(abs(v) for v in dO.values())}")
posR = sum(1 for k in keys if dR[k] > 0); negR = sum(1 for k in keys if dR[k] < 0)
print(f"ref  word delta 24->26+f : {posR} up, {negR} down, {200-posR-negR} unchanged; sum {sum(dR.values())}")
same = [k for k in keys if dO[k] == dR[k] and dO[k] != 0]
print(f"documents where OURS and REF word counts moved by the SAME nonzero amount: {len(same)}")
# same page count on both sides too => a pure measurement-tool signature
samepg = [k for k in same if ours_head_F[k][0]==ours_r47[k][0] and ref_F[k][0]==ref_24[k][0]]
print(f"   of those, page count unchanged on both sides: {len(samepg)}")
for k in samepg[:8]:
    print(f"      d={dO[k]:+5d}  ours {ours_r47[k][1]}->{ours_head_F[k][1]}   ref {ref_24[k][1]}->{ref_F[k][1]}   {os.path.basename(k)}")
print()
# magnitude distribution of ours-only word movement
import statistics
vals = sorted(abs(v) for v in dO.values() if v)
print("ours |word delta| deciles:", [vals[int(len(vals)*q)] for q in (0,.25,.5,.75,.9)], "max", vals[-1])
big = sorted(((dO[k], k) for k in keys), key=lambda t: -abs(t[0]))[:8]
for d,k in big:
    print(f"   {d:+6d}  ours {ours_r47[k][1]:>7s} -> {ours_head_F[k][1]:>7s}   ref {ref_24[k][1]:>7s} -> {ref_F[k][1]:>7s}   {os.path.basename(k)}")
print()
# the single document whose ours page count moved r47 -> HEAD
mv = [k for k in keys if ours_head_F[k][0] != ours_r47[k][0]]
for k in mv:
    print("ours page count moved r47->HEAD on:", os.path.basename(k), ours_r47[k][0], "->", ours_head_F[k][0], " ref:", ref_24[k][0], "->", ref_F[k][0])
