import sys, os
sys.path.insert(0, "/tmp/claude-0/-c-sandbox-workdir/5cf9a493-a19e-4a73-944b-74fd16d25b38/scratchpad/words2")
from decompose import *
print()
PR = "/c/sandbox/workdir/libreoffice-core/dotnet/probes"
ours_r47f, ref_24f = read_gate(f"{PR}/words-r47/after-unnamed.tsv")
print("ref@24.2.7.2 column identical between r47 baseline.tsv and after-unnamed.tsv:",
      sum(1 for k in ref_24 if ref_24[k] == ref_24f.get(k)), "/ 200")
score(ours_r47f, ref_24f, "A' ours@r47-FINAL x ref@24.2.7.2  (stored)")
score(ours_r47f, ref_F,   "C' ours@r47-FINAL x ref@26.2.4.2+fonts (computed)")
keys = sorted(ref_F)
print()
print("ours@HEAD(+DejaVu) vs the STORED ours@r47-FINAL column — SAME SOURCE CODE:")
print("  page counts identical:", sum(1 for k in keys if ours_head_F[k][0] == ours_r47f[k][0]), "/ 200")
print("  word counts identical:", sum(1 for k in keys if ours_head_F[k][1] == ours_r47f[k][1]), "/ 200")
mv = [k for k in keys if ours_head_F[k][0] != ours_r47f[k][0]]
for k in mv:
    print(f"    page count differs: {os.path.basename(k)} {ours_r47f[k][0]} -> {ours_head_F[k][0]}")
def i(x):
    try: return int(x)
    except: return 0
dO = {k: i(ours_head_F[k][1]) - i(ours_r47f[k][1]) for k in keys}
dR = {k: i(ref_F[k][1]) - i(ref_24f[k][1]) for k in keys}
lock = sum(1 for k in keys if dO[k] == dR[k] != 0)
print(f"  word deltas: {sum(1 for v in dO.values() if v>0)} up, {sum(1 for v in dO.values() if v<0)} down; "
      f"identical to the reference's delta on {lock} documents")
