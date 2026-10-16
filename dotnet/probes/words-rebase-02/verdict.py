"""The verdict rule, transcribed from batch-check.sh / ours-gate.sh, plus a replay control.

verdict(op,rp,ow,rw,un) reproduces the shell block:
    pages     if op != rp
    words     if rw>0 and |ow-rw| > rw*0.02 and |ow-rw| > 3 ; elif rw==0 and ow>3
    unembedded if un != 0
Comparisons in the shell are STRING comparisons for pages/fonts, so '07' != '7'.
"""
import sys, glob, os

def verdict(op, rp, ow, rw, un):
    v = []
    if op != rp:
        v.append("pages")
    try:
        rwi = int(rw)
    except ValueError:
        rwi = 0
    if rwi > 0:
        try: owi = int(ow)
        except ValueError: owi = 0
        d = abs(owi - rwi)
        if d > rwi * 0.02 and d > 3:
            v.append("words")
    else:
        try: owi = int(ow)
        except ValueError: owi = 0
        if owi > 3:
            v.append("words")
    if str(un) != "0":
        v.append("unembedded")
    return ",".join(v) if v else "match"

def replay(paths):
    n = bad = 0
    for p in paths:
        for line in open(p, encoding="utf-8", errors="replace"):
            line = line.rstrip("\n")
            if not line or line.startswith("#") or line.startswith("path\t"):
                continue
            f = line.split("\t")
            if len(f) < 7: continue
            pages, words, fonts, un, stored = f[2], f[3], f[4], f[5], f[6]
            if stored in ("ref-failed", "ours-failed", "both-failed"): continue
            if "/" not in pages or "/" not in words: continue
            op, rp = pages.split("/", 1); ow, rw = words.split("/", 1)
            got = verdict(op, rp, ow, rw, un)
            n += 1
            if got != stored:
                bad += 1
                if bad <= 10:
                    print("MISMATCH", os.path.basename(p), f[0], "stored=", stored, "got=", got)
    print(f"replayed {n} rows, {bad} mismatches")
    return n, bad

if __name__ == "__main__":
    replay(sys.argv[1:])
