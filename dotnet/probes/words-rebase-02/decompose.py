import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from verdict import verdict

def read_gate(p):
    """gate.tsv / baseline.tsv shape: path ext op/rp ow/rw of/rf un verdict"""
    ours, ref = {}, {}
    for line in open(p, encoding="utf-8", errors="replace"):
        line = line.rstrip("\n")
        if not line or line.startswith("#") or line.startswith("path\t"): continue
        f = line.split("\t")
        if len(f) < 7: continue
        op, rp = f[2].split("/", 1); ow, rw = f[3].split("/", 1); of, rf = f[4].split("/", 1)
        ours[f[0]] = (op, ow, of, f[5])          # pages, words, fonts, unembedded
        ref[f[0]]  = (rp, rw, rf)
    return ours, ref

def read_refbase(p, track="words"):
    ref = {}
    for line in open(p, encoding="utf-8", errors="replace"):
        line = line.rstrip("\n")
        if not line or line.startswith("#") or line.startswith("track\t"): continue
        f = line.split("\t")
        if f[0] != track: continue
        ref[f[1]] = (f[3], f[4], f[5])
    return ref

def score(ours, ref, label, dump=None):
    keys = sorted(set(ours) & set(ref))
    assert len(keys) == len(ours) == len(ref), \
        f"{label}: key mismatch ours={len(ours)} ref={len(ref)} common={len(keys)}"
    m = 0; rows = []
    for k in keys:
        op, ow, of, un = ours[k]
        rp, rw, rf = ref[k]
        v = verdict(op, rp, ow, rw, un)
        rows.append((k, op, rp, ow, rw, un, v))
        if v == "match": m += 1
    print(f"{label:52s} {m:3d} / {len(keys)}")
    if dump:
        with open(dump, "w") as fh:
            fh.write("path\tourspages\trefpages\toursword\trefwords\tunemb\tverdict\n")
            for r in rows: fh.write("\t".join(map(str, r)) + "\n")
    return {k: r[6] for k, *r in [(r[0], *r) for r in rows]}, rows

SPD = os.path.dirname(os.path.abspath(__file__))
OLD = "/tmp/claude-0/-c-sandbox-workdir/5cf9a493-a19e-4a73-944b-74fd16d25b38/scratchpad/words"
PR  = "/c/sandbox/workdir/libreoffice-core/dotnet/probes"

ours_head_F, ref_F_gate = read_gate(f"{SPD}/gate/gate.tsv")
ours_head_N, ref_N_gate = read_gate(f"{OLD}/gate/gate.tsv")
ours_r47,    ref_24     = read_gate(f"{PR}/words-r47/baseline.tsv")
ref_F = read_refbase("/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/ref-baseline-all.tsv")
ref_N = read_refbase("/c/sandbox/workdir/refpdfs-26.2.4.2/ref-baseline-all.tsv")

# cross-check: the ref column my gate read out of the PDFs must equal the banked TSV
dis = [k for k in ref_F if ref_F_gate.get(k) != ref_F[k]]
print(f"ref column: gate-read vs banked TSV disagree on {len(dis)} of {len(ref_F)}")
dis2 = [k for k in ref_N if ref_N_gate.get(k) != ref_N[k]]
print(f"ref(nofont): predecessor gate vs banked TSV disagree on {len(dis2)} of {len(ref_N)}")
print()

print("=== the five legs ===")
score(ours_r47,    ref_24, "A  ours@r47      x ref@24.2.7.2   (stored)")
score(ours_r47,    ref_N,  "B  ours@r47      x ref@26.2.4.2-fonts (computed)")
score(ours_r47,    ref_F,  "C  ours@r47      x ref@26.2.4.2+fonts (computed)")
score(ours_head_N, ref_N,  "D  ours@HEAD-f   x ref@26.2.4.2-fonts (measured)")
score(ours_head_F, ref_F,  "E  ours@HEAD+f   x ref@26.2.4.2+fonts (MEASURED)", dump=f"{SPD}/gate-verdicts.tsv")
print()
print("=== incoherent mixtures, for reference only ===")
score(ours_head_F, ref_N,  "X  ours@HEAD+f   x ref-fonts   (mismatched env)")
score(ours_head_N, ref_F,  "Y  ours@HEAD-f   x ref+fonts   (mismatched env)")
