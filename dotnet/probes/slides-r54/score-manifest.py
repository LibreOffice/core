#!/usr/bin/env python3
"""Score a track sweep against MANIFEST.tsv rather than against its own TOTAL.

The slides sweep glob visits 311 files for 302 manifest paths, so its own MATCH line is not
the scoreboard.  This joins `ink.tsv`/`parity.tsv` to the manifest's `path` column and prints
the passing count, the ink totals and every row whose verdict disagrees with `status`.

    score-manifest.py <sweep-outdir> [family] [--against <other-outdir>]
"""
import collections, os, sys

MANIFEST = "/c/sandbox/workdir/sample-files/MANIFEST.tsv"


def load(out):
    ink, verdict = {}, {}
    with open(os.path.join(out, "ink.tsv"), encoding="utf-8") as fh:
        for line in fh:
            if line.startswith("#") or line.startswith("path\t"):
                continue
            f = line.rstrip("\n").split("\t")
            ink[f[0]] = f
    with open(os.path.join(out, "parity.tsv"), encoding="utf-8") as fh:
        for line in fh:
            if line.startswith("#") or line.startswith("path\t"):
                continue
            f = line.rstrip("\n").split("\t")
            verdict[f[0]] = f[6]
    return ink, verdict


def manifest(family):
    rows = {}
    with open(MANIFEST, encoding="utf-8") as fh:
        hdr = fh.readline().rstrip("\n").split("\t")
        for line in fh:
            r = dict(zip(hdr, line.rstrip("\n").split("\t")))
            if r["family"] == family:
                rows[r["path"]] = r
    return rows


if __name__ == "__main__":
    out = sys.argv[1]
    family = sys.argv[2] if len(sys.argv) > 2 and not sys.argv[2].startswith("-") else "slides"
    other = None
    if "--against" in sys.argv:
        other = sys.argv[sys.argv.index("--against") + 1]

    rows = manifest(family)
    ink, verdict = load(out)
    passing = missing = 0
    abs_ink = signed = 0.0
    major = 0
    disagree = []
    for path, r in sorted(rows.items()):
        if path not in verdict:
            missing += 1
            print(f"  MISSING from sweep: {path}")
            continue
        v = verdict[path]
        if v == "match":
            passing += 1
        if (v == "match") != (r["status"] == "done"):
            disagree.append((path, r["status"], v))
        f = ink.get(path)
        if f and f[2] not in ("-", "?"):
            abs_ink += float(f[2])
            signed += float(f[3])
            major += int(f[4])
    print(f"{family}: {passing} of {len(rows)}   (missing {missing})")
    print(f"abs_ink {abs_ink:.2f}  signed {signed:.2f}  major {major}")
    print(f"manifest disagreements: {len(disagree)}")
    for p, s, v in disagree:
        print(f"   {p}: manifest={s} sweep={v}")

    if other:
        oink, overdict = load(other)
        moved = []
        for path in sorted(rows):
            a, b = ink.get(path), oink.get(path)
            if not a or not b or a[2] in ("-", "?") or b[2] in ("-", "?"):
                continue
            d = float(b[2]) - float(a[2])
            if abs(d) >= 0.005:
                moved.append((d, path, float(a[2]), float(b[2])))
        moved.sort()
        print(f"\nagainst {other}: {len(moved)} documents moved "
              f"({sum(1 for d, *_ in moved if d < 0)} improved, "
              f"{sum(1 for d, *_ in moved if d > 0)} worsened)")
        for d, p, a, b in moved:
            print(f"   {d:+8.2f}  {a:8.2f} -> {b:8.2f}  {p}")
        vm = [(p, verdict[p], overdict[p]) for p in sorted(rows)
              if p in verdict and p in overdict and verdict[p] != overdict[p]]
        print(f"verdict changes: {len(vm)}")
        for p, a, b in vm:
            print(f"   {p}: {a} -> {b}")
