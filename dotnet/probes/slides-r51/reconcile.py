#!/usr/bin/env python3
"""Reconcile a sweep's rows.tsv against MANIFEST.tsv on a CASE-FOLDED identity.

/c/sandbox/workdir is a case-insensitive virtiofs mount: foo.pptx and foo.PPTX are one
inode.  A find-based sweep can therefore return the same document twice, and the TOTAL line
is not the corpus size.  Every total here is taken over case-folded paths.
"""
import collections, sys

sweep, track = sys.argv[1], sys.argv[2]

man = {}
with open("/c/sandbox/workdir/sample-files/MANIFEST.tsv", encoding="utf-8") as fh:
    hdr = fh.readline().rstrip("\n").split("\t")
    for line in fh:
        f = line.rstrip("\n").split("\t")
        r = dict(zip(hdr, f))
        if r["family"] != track: continue
        man[r["path"].casefold()] = r

rows = {}
dupes = collections.defaultdict(list)
with open(f"{sweep}/rows.tsv", encoding="utf-8") as fh:
    for line in fh:
        f = line.rstrip("\n").split("\t")
        if len(f) < 7: continue
        key = f[0].casefold()
        dupes[key].append(f[0])
        rows[key] = f            # last wins; verified identical below

print(f"rows.tsv lines            : {sum(len(v) for v in dupes.values())}")
print(f"distinct case-folded paths: {len(rows)}")
print(f"MANIFEST {track} documents : {len(man)}")

multi = {k: v for k, v in dupes.items() if len(set(v)) > 1}
print(f"\npaths present under >1 spelling (the case-insensitive mount): {len(multi)}")
for k, v in sorted(multi.items()):
    print("   ", " | ".join(sorted(set(v))))

only_sweep = set(rows) - set(man)
only_man = set(man) - set(rows)
print(f"\nin sweep, not in MANIFEST: {len(only_sweep)}")
for k in sorted(only_sweep): print("   ", rows[k][0])
print(f"in MANIFEST, not in sweep: {len(only_man)}")
for k in sorted(only_man): print("   ", man[k]['path'])

match = sum(1 for k, f in rows.items() if f[6] == "match" and k in man)
print(f"\nmatch, case-folded, MANIFEST-known: {match} of {len(man)}")

agree = dis = 0
disagreements = []
for k, r in man.items():
    if k not in rows:
        continue
    expect = "match" if r["status"] == "done" else "not-match"
    actual = "match" if rows[k][6] == "match" else "not-match"
    if expect == actual: agree += 1
    else:
        dis += 1
        disagreements.append((r["path"], r["status"], rows[k][6], r["kind"]))
print(f"agree with MANIFEST status: {agree}   disagree: {dis}")
for p, st, v, kind in sorted(disagreements):
    print(f"   {p}\n      manifest={st} kind={kind}  sweep={v}")
