import sys, os
sys.path.insert(0, "/tmp/claude-0/-c-sandbox-workdir/5cf9a493-a19e-4a73-944b-74fd16d25b38/scratchpad/words2")
from decompose import *
PR = "/c/sandbox/workdir/libreoffice-core/dotnet/probes"
ours_r47f, ref_24f = read_gate(f"{PR}/words-r47/after-unnamed.tsv")
keys = sorted(ref_F)
def V(o, r, k): return verdict(o[k][0], r[k][0], o[k][1], r[k][1], o[k][3])
vA = {k: V(ours_r47f, ref_24f, k) for k in keys}
vE = {k: V(ours_head_F, ref_F, k) for k in keys}
lost = [k for k in keys if vA[k]=="match" and vE[k]!="match"]
won  = [k for k in keys if vA[k]!="match" and vE[k]=="match"]
print(f"\nSAME CODE, 24.2.7.2 -> 26.2.4.2+DejaVu: lost {len(lost)}, gained {len(won)}, net {len(won)-len(lost)}")
for k in lost:
    print(f"  LOST  {vE[k]:12s} pages {ours_r47f[k][0]}/{ref_24f[k][0]} -> {ours_head_F[k][0]}/{ref_F[k][0]}   {os.path.basename(k)}")
for k in won:
    print(f"  GAIN  was {vA[k]:8s} pages {ours_r47f[k][0]}/{ref_24f[k][0]} -> {ours_head_F[k][0]}/{ref_F[k][0]}   {os.path.basename(k)}")
