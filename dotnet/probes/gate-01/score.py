"""Old metric vs new metric, per track, from artefacts already on disk.

Inputs: `census.tsv` (raw and corrected word counts for both sides of all 534 documents, read
by one pdftotext) and each track's `gate.tsv` (this round's own sweep, page and font columns).
No document is re-rendered, so old and new are computed over the SAME PDF bytes -- which is
what makes the comparison a comparison of metrics rather than of runs.
"""
import os, sys, collections

G = "/tmp/claude-0/-c-sandbox-workdir/5cf9a493-a19e-4a73-944b-74fd16d25b38/scratchpad/gate"
sys.path.insert(0, G)
from verdict import verdict            # the transcribed rule, replayed over 9552 stored rows


def load_census():
    out = {}
    for line in open(f"{G}/census.tsv", encoding="utf-8"):
        f = line.rstrip("\n").split("\t")
        if f[0] == "track":
            continue
        out[f[1]] = dict(track=f[0], ext=f[2], rawO=int(f[3]), rawR=int(f[4]),
                         alnumO=int(f[5]), alnumR=int(f[6]), nonO=int(f[7]), nonR=int(f[8]))
    return out


def load_gate(track):
    out = {}
    for line in open(f"{G}/{track}/gate.tsv", encoding="utf-8"):
        f = line.rstrip("\n").split("\t")
        if f[0] == "path":
            continue
        op, rp = f[2].split("/", 1)
        ow, rw = f[3].split("/", 1)
        out[f[0]] = dict(op=op, rp=rp, ow=ow, rw=rw, un=f[5], stored=f[6])
    return out


def main():
    cen = load_census()
    grand = collections.Counter()
    for track in ("words", "slides", "sheets"):
        gate = load_gate(track)
        old = new = 0
        changed = []
        ctrl_lost = []
        ctrl_kept = 0
        repro = 0
        for path, g in sorted(gate.items()):
            c = cen.get(path)
            if c is None:
                print("NO CENSUS ROW", path); continue
            vo = verdict(g["op"], g["rp"], g["ow"], g["rw"], g["un"])
            if vo == g["stored"]:
                repro += 1
            # control: the census's own raw counts must equal the sweep's `wc -w` counts
            assert str(c["rawO"]) == g["ow"] and str(c["rawR"]) == g["rw"], (path, c, g)
            vn = verdict(g["op"], g["rp"], str(c["alnumO"]), str(c["alnumR"]), g["un"])
            old += vo == "match"
            new += vn == "match"
            if vo != vn:
                changed.append((path, vo, vn, c))
            if vo == "match":
                if vn == "match":
                    ctrl_kept += 1
                else:
                    ctrl_lost.append((path, vn, c))
        n = len(gate)
        grand["n"] += n; grand["old"] += old; grand["new"] += new
        grand["kept"] += ctrl_kept; grand["lost"] += len(ctrl_lost)
        print(f"\n=== {track}: {n} documents   OLD {old}   NEW {new}   "
              f"(sweep verdicts reproduced by the transcribed rule: {repro}/{n})")
        print(f"    control over already-matching: {ctrl_kept} of {old} stay matching, "
              f"{len(ctrl_lost)} flip to failing")
        for path, vo, vn, c in changed:
            d = "->"
            print(f"    {vo:12s} {d} {vn:12s}  raw {c['rawO']}/{c['rawR']}  "
                  f"corrected {c['alnumO']}/{c['alnumR']}  "
                  f"non-alnum {c['nonO']}/{c['nonR']}  {path}")
    print(f"\n=== TOTAL {grand['n']}   OLD {grand['old']}   NEW {grand['new']}   "
          f"control {grand['kept']} kept / {grand['lost']} lost")


main()
