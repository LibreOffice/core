import sys, os
sys.path.insert(0, "/tmp/claude-0/-c-sandbox-workdir/5cf9a493-a19e-4a73-944b-74fd16d25b38/scratchpad/words2")
from decompose import *
print()
keys = sorted(ref_F)
def i(x):
    try: return int(x)
    except: return 0
# REFERENCE page movement, 24.2.7.2 -> 26.2.4.2 WITH the correct font set (the real version effect)
d = {k: i(ref_F[k][0]) - i(ref_24[k][0]) for k in keys}
mv = [(v,k) for k,v in d.items() if v]
print(f"ref pages 24.2.7.2 -> 26.2.4.2+DejaVu: {len(mv)} of 200 moved, total |d| {sum(abs(v) for v,_ in mv)}, "
      f"up {sum(1 for v,_ in mv if v>0)}, down {sum(1 for v,_ in mv if v<0)}")
for v,k in sorted(mv, key=lambda t:-abs(t[0]))[:8]:
    print(f"   {v:+5d}  {ref_24[k][0]:>4s} -> {ref_F[k][0]:<4s}  {os.path.basename(k)}")
# vs the font-starved figure the predecessor reported
dN = {k: i(ref_N[k][0]) - i(ref_24[k][0]) for k in keys}
mvN = [(v,k) for k,v in dN.items() if v]
print(f"  (font-starved, the predecessor's figure: {len(mvN)} moved, {sum(abs(v) for v,_ in mvN)} pages)")
# ref word movement with the tool artefact netted out on the 86
dOw = {k: i(ours_head_F[k][1]) - i(ours_r47[k][1]) for k in keys}
dRw = {k: i(ref_F[k][1]) - i(ref_24[k][1]) for k in keys}
band = sum(1 for k in keys if abs(dRw[k]) > 0.02*i(ref_24[k][1]) and abs(dRw[k]) > 3)
bandnet = sum(1 for k in keys if abs(dRw[k]-dOw[k]) > 0.02*i(ref_24[k][1]) and abs(dRw[k]-dOw[k]) > 3)
print(f"ref words beyond the 2%+3 band vs 24.2.7.2: {band} of 200 raw; "
      f"{bandnet} of 200 once our own identically-measured column is differenced out")
