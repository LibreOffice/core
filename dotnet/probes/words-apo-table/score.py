"""Score a sweep against the gate at the base commit, row for row.

Refuses to print unless every path the gate holds for this track found a row in the sweep —
the alias drift described in `dotnet/CLAUDE.md` means a sweep's own TOTAL is not a denominator.
"""
import sys
from collections import Counter

def load(path):
    rows = {}
    for line in open(path, encoding="utf-8"):
        if line.startswith("#") or line.startswith("path\t"):
            continue
        f = line.rstrip("\n").split("\t")
        if len(f) < 7:
            continue
        rows[f[0]] = f
    return rows

gate = load(sys.argv[1])
now = load(sys.argv[2])
prefix = sys.argv[3] if len(sys.argv) > 3 else ""

want = {k: v for k, v in gate.items() if k.startswith(prefix)}
missing = [k for k in want if k not in now]
extra = [k for k in now if k not in want]

print(f"gate rows for {prefix!r}: {len(want)}   sweep rows: {len(now)}")
if missing:
    print(f"MISSING {len(missing)}:")
    for k in missing[:20]:
        print("   ", k)
if extra:
    print(f"EXTRA {len(extra)}:")
    for k in extra[:20]:
        print("   ", k)
if missing:
    print("refusing to score: the sweep did not cover the gate's path list")
    sys.exit(1)

before = Counter(want[k][6] for k in want)
after = Counter(now[k][6] for k in want)
print("verdicts before:", dict(before))
print("verdicts after: ", dict(after))

moved = []
for k in want:
    a, b = want[k], now[k]
    if a[6] != b[6] or a[2] != b[2] or a[3] != b[3] or a[4] != b[4]:
        moved.append((k, a, b))

print(f"\nrows whose verdict or any counted column moved: {len(moved)}")
for k, a, b in moved:
    print(f"  {k}\n      before pages={a[2]} words={a[3]} fonts={a[4]} verdict={a[6]}"
          f"\n      after  pages={b[2]} words={b[3]} fonts={b[4]} verdict={b[6]}")
