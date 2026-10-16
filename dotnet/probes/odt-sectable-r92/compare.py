#!/usr/bin/env python3
"""Two sweeps of the same track compared, with every row that failed on either side excluded.

A gate run under contention undercounts on the *reference* side, so two runs are comparable only
over the rows that rendered on both sides in both of them (`dotnet/CLAUDE.md`).  Prints the verdict
totals over that common set and every row whose verdict moved.
"""
import sys

def rows(path):
    out = {}
    for line in open(path):
        f = line.rstrip('\n').split('\t')
        if len(f) >= 7:
            out[f[0]] = f
    return out

a, b = rows(sys.argv[1]), rows(sys.argv[2])
common = sorted(set(a) & set(b))
bad = {k for k in common if 'failed' in a[k][6] or 'failed' in b[k][6]}
scored = [k for k in common if k not in bad]

print(f"{len(a)} and {len(b)} rows, {len(common)} common, {len(bad)} excluded for a failure on either "
      f"side in either run, {len(scored)} scored")
for name, table in (('before', a), ('after', b)):
    counts = {}
    for k in scored:
        counts[table[k][6]] = counts.get(table[k][6], 0) + 1
    print(f"  {name}: " + ", ".join(f"{k} {v}" for k, v in sorted(counts.items())))

print("moved:")
for k in scored:
    if a[k][6] != b[k][6]:
        print(f"  {a[k][6]:>12} -> {b[k][6]:<12} pages {a[k][2]}->{b[k][2]} glyphs {a[k][-1]}->{b[k][-1]}  {k}")
