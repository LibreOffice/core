#!/usr/bin/env python3
"""Sum the per-page differing-pixel column over a sweep, keyed on MANIFEST.tsv.

`abs_ink` is a *signed-area* measure per region and it rises whenever one one-sided offset is
replaced by a smaller two-sided one -- which is what a geometry fix looks like.  The `diff%`
column is the fraction of pixels that differ at all, and it does not have that property.
Refuses to print unless every manifest path found a report.
"""
import os, sys

MANIFEST = "/c/sandbox/workdir/sample-files/MANIFEST.tsv"


def rows(family):
    out = {}
    with open(MANIFEST, encoding="utf-8") as fh:
        hdr = fh.readline().rstrip("\n").split("\t")
        for line in fh:
            r = dict(zip(hdr, line.rstrip("\n").split("\t")))
            if r["family"] == family:
                stem, ext = os.path.splitext(os.path.basename(r["path"]))
                out[r["path"]] = f"{stem}__{ext[1:].lower()}.txt"
    return out


def read(out, name):
    p = os.path.join(out, "cmp", name)
    if not os.path.exists(p):
        return None
    total, pages = 0.0, 0
    for line in open(p, encoding="utf-8"):
        f = line.split("\t")
        if len(f) >= 2 and f[0].isdigit():
            try:
                total += float(f[1]); pages += 1
            except ValueError:
                pass
    return total, pages


if __name__ == "__main__":
    family = sys.argv[1]
    outs = sys.argv[2:]
    manifest = rows(family)
    data = []
    for out in outs:
        got, miss = {}, []
        for path, name in manifest.items():
            r = read(out, name)
            if r is None:
                miss.append(path)
            else:
                got[path] = r
        if miss:
            raise SystemExit(f"REFUSING TO PRINT: {len(miss)} manifest paths have no report in {out}")
        data.append((out, got))
    for out, got in data:
        print(f"{out}: diff-pixels {sum(v[0] for v in got.values()):.2f} "
              f"over {sum(v[1] for v in got.values())} pages, {len(got)} documents")
    if len(data) == 2:
        (a, ga), (b, gb) = data
        moved = sorted(((gb[p][0] - ga[p][0], p) for p in ga), key=lambda t: t[0])
        moved = [m for m in moved if abs(m[0]) >= 0.005]
        print(f"\n{len(moved)} documents moved on differing pixels "
              f"({sum(1 for d, _ in moved if d < 0)} improved, "
              f"{sum(1 for d, _ in moved if d > 0)} worsened)")
        for d, p in moved:
            print(f"   {d:+8.2f}  {ga[p][0]:8.2f} -> {gb[p][0]:8.2f}  {p}")
