#!/usr/bin/env python3
"""Renderings changed between two sweeps' ours/ directories, /CreationDate normalised out."""
import re, sys
from pathlib import Path

a, b = Path(sys.argv[1]), Path(sys.argv[2])
DATE = re.compile(rb"/CreationDate\s*\([^)]*\)")
ID = re.compile(rb"/ID\s*\[[^\]]*\]")


def norm(p):
    d = p.read_bytes()
    return ID.sub(b"/ID[]", DATE.sub(b"/CreationDate()", d))


names = sorted(set(p.name for p in a.glob("*.pdf")) | set(p.name for p in b.glob("*.pdf")))
changed = []
same = 0
for n in names:
    pa, pb = a / n, b / n
    if not pa.exists() or not pb.exists():
        changed.append(n + "  (missing one side)")
        continue
    if norm(pa) == norm(pb):
        same += 1
    else:
        changed.append(n)
print(f"{len(names)} renderings, {len(changed)} changed, {same} byte-identical")
for c in changed:
    print("  " + c)
