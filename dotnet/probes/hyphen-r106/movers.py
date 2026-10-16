#!/usr/bin/env python3
"""Which documents the hyphenator moves, and which family they are in."""
import sys, collections
off = dict(l.split("\t")[0:2] for l in open(sys.argv[1]).read().splitlines() if l)
on  = dict(l.split("\t")[0:2] for l in open(sys.argv[2]).read().splitlines() if l)
common = [k for k in off if k in on]
bad = [k for k in common if off[k].startswith(("FAILED", "ERROR")) or on[k].startswith(("FAILED", "ERROR"))]
moved = [k for k in common if k not in bad and off[k] != on[k]]
fam = collections.Counter(k.split("/")[0] for k in moved)
tot = collections.Counter(k.split("/")[0] for k in common if k not in bad)
print(f"scored {len(common)} of {len(off)}; unrenderable either side {len(bad)}")
print(f"moved {len(moved)}")
for f in sorted(tot):
    print(f"  {f}\t{fam.get(f,0)} of {tot[f]}")
for k in sorted(moved):
    print("MOVER\t" + k)
for k in sorted(bad):
    print("FAILED\t" + k + "\t" + off[k] + "\t" + on[k])
