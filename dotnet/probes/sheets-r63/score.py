#!/usr/bin/env python3
"""Score a sweep's parity.tsv against MANIFEST.tsv's paths for one family.

Round 58's `score.py` with the family taken from `$FAMILY` (default `sheets`), because this
round measures the two other tracks in this worktree as well and a track's own score must come
from its own manifest rows rather than from `batch-check.sh`'s TOTAL.


`batch-check.sh`'s own TOTAL is 325 for 307 documents, because this mount gives 18 corpus
files a second, case-variant directory entry pointing at the same inode.  A sweep total is
therefore not a corpus score.  This keys on the manifest's paths.

Refuses to print unless every manifest path found a row: a missing input read as zero
reads as a finding.
"""
import os, sys, collections

FAMILY = os.environ.get("FAMILY", "sheets")
CORPUS = "/c/sandbox/workdir/sample-files"

paths = []
with open(os.path.join(CORPUS, "MANIFEST.tsv"), encoding="utf-8") as fh:
    fh.readline()
    for line in fh:
        f = line.rstrip("\n").split("\t")
        if f[0] == FAMILY:
            paths.append((f[2], f[7]))

def load(tsv):
    rows = {}
    with open(tsv, encoding="utf-8") as fh:
        for line in fh:
            if line.startswith("#"):
                continue
            f = line.rstrip("\n").split("\t")
            if f[0] == "path":
                continue
            rows[f[0]] = f
            rows.setdefault("~lc~" + f[0].lower(), f)
    return rows

def score(tsv):
    rows = load(tsv)
    got, missing = [], []
    for p, status in paths:
        r = rows.get(p) or rows.get("~lc~" + p.lower())
        (got if r else missing).append((p, status, r))
    if missing:
        print("REFUSING TO SCORE %s — %d manifest paths have no row:" % (tsv, len(missing)),
              file=sys.stderr)
        for m, _, _ in missing[:20]:
            print("  ", m, file=sys.stderr)
        sys.exit(2)
    return got

sweeps = [(t, score(t)) for t in sys.argv[1:]]

for tsv, got in sweeps:
    c = collections.Counter(r[6] for _, _, r in got)
    print("%s: %d manifest paths, MATCH %d of %d" % (tsv, len(got), c["match"], len(got)))
    for k, v in c.most_common():
        print("    %-14s %d" % (k, v))

if len(sweeps) == 1:
    print("\nmismatches:")
    for p, status, r in sorted(sweeps[0][1]):
        if r[6] != "match":
            print("  %-6s %-72s %s" % (status, p[7:], "\t".join(r[2:7])))
else:
    (ta, a), (tb, b) = sweeps[0], sweeps[1]
    da = {p: r for p, _, r in a}
    db = {p: r for p, _, r in b}
    print("\nper-document movement (before -> after), split by which side moved:")
    for p in sorted(da):
        ra, rb = da[p], db[p]
        if ra[2:7] == rb[2:7]:
            continue
        oursa, refa = ra[3].split("/")
        oursb, refb = rb[3].split("/")
        side = []
        if ra[2] != rb[2]:
            side.append("pages %s->%s" % (ra[2], rb[2]))
        if oursa != oursb:
            side.append("OURS words %s->%s" % (oursa, oursb))
        if refa != refb:
            side.append("REF words %s->%s" % (refa, refb))
        if ra[4] != rb[4] or ra[5] != rb[5]:
            side.append("fonts %s/%s->%s/%s" % (ra[4], ra[5], rb[4], rb[5]))
        print("  %-8s -> %-8s %-64s %s" % (ra[6], rb[6], p[7:], "; ".join(side)))
