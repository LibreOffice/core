#!/usr/bin/env python3
"""Score a re-swept track against the banked whole-corpus gate.

    score.py <new rows.tsv or parity.tsv> <path prefix>

Names every verdict that moved, in both directions, and refuses to report a
summary unless every banked row for the prefix found a row in the new sweep —
a sweep that lost a worker writes a short TSV whose own totals look plausible.
"""
import pathlib
import sys

GATE = pathlib.Path("/home/user/gate-2f47/parity.tsv")


def load(path):
    out = {}
    for line in pathlib.Path(path).read_text().splitlines():
        if line.startswith("#") or line.startswith("path\t"):
            continue
        f = line.split("\t")
        if len(f) < 9:
            continue
        out[f[0]] = f
    return out


new = load(sys.argv[1])
prefix = sys.argv[2]
old = {k: v for k, v in load(GATE).items() if k.startswith(prefix)}

missing = sorted(k for k in old if k not in new)
extra = sorted(k for k in new if k not in old)
if missing:
    print(f"!! {len(missing)} banked rows have no row in the new sweep — it is short:")
    for k in missing[:20]:
        print("   ", k)
if extra:
    print(f"!! {len(extra)} rows in the new sweep are not in the banked gate:")
    for k in extra[:20]:
        print("   ", k)

moved = []
for k, o in sorted(old.items()):
    n = new.get(k)
    if n is None:
        continue
    if o[6] != n[6] or o[2] != n[2] or o[8] != n[8]:
        moved.append((k, o, n))

print(f"\nbanked {len(old)}  reswept {len(new)}  compared {len(old) - len(missing)}")
before = sum(1 for o in old.values() if o[6] == "match")
after = sum(1 for k, o in old.items() if k in new and new[k][6] == "match")
print(f"MATCH before {before}  after {after}\n")

verdict_moves = [m for m in moved if m[1][6] != m[2][6]]
print(f"verdict changed: {len(verdict_moves)}")
for k, o, n in verdict_moves:
    print(f"  {o[6]:>12} -> {n[6]:<12}  pages {o[2]}->{n[2]}  glyphs {o[8]}->{n[8]}  {k}")

other = [m for m in moved if m[1][6] == m[2][6]]
print(f"\nsame verdict, numbers moved: {len(other)}")
for k, o, n in other:
    print(f"  {o[6]:>12}                pages {o[2]}->{n[2]}  glyphs {o[8]}->{n[8]}  {k}")
