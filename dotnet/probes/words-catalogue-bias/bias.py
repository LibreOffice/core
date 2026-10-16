#!/usr/bin/env python3
"""The signed placement offset of every drawing record on a document, ours against a reference.

`pdf-ops.py diff` says *which* records differ; this says by how much and **in which direction**, over
every page at once, because the question a sub-point residual raises is whether it is biased. Fills and
strokes are paired greedily by nearest anchor within a tolerance, and only pairs of the same kind and
compatible size are kept, so a mis-pair cannot manufacture a bias.

    bias.py ours.pdf ref.pdf [--tol 3.0]
"""
import argparse, re, subprocess, sys, math, collections
from pathlib import Path

OPS = Path(__file__).resolve().parents[3] / ".claude/skills/render-comparison/scripts/pdf-ops.py"

FILL = re.compile(r"\s*(fill|stroke)\s+p(\d+)\s+\(\s*([-\d.]+),\s*([-\d.]+)\)-\(\s*([-\d.]+),\s*([-\d.]+)\)\s+(\S+)")


def records(pdf):
    out = subprocess.run([sys.executable, str(OPS), "dump", pdf], capture_output=True, text=True)
    if out.returncode or not out.stdout.strip():
        raise SystemExit(f"{pdf}: pdf-ops produced nothing ({out.stderr.strip()[:200]})")
    rows = []
    for line in out.stdout.splitlines():
        m = FILL.match(line)
        if m:
            rows.append(dict(kind=m.group(1), page=int(m.group(2)),
                             x0=float(m.group(3)), y0=float(m.group(4)),
                             x1=float(m.group(5)), y1=float(m.group(6)), colour=m.group(7)))
    return rows


def pair(ours, ref, tol):
    """Greedy nearest pairing within `tol`, same kind, same size to within a point."""
    out = []
    left = list(ref)
    for a in ours:
        best, bestd = None, None
        for b in left:
            if b["kind"] != a["kind"] or b["page"] != a["page"]:
                continue
            if abs((a["x1"] - a["x0"]) - (b["x1"] - b["x0"])) > 1.0:
                continue
            if abs((a["y1"] - a["y0"]) - (b["y1"] - b["y0"])) > 1.0:
                continue
            d = math.hypot(a["x0"] - b["x0"], a["y0"] - b["y0"])
            if d <= tol and (bestd is None or d < bestd):
                best, bestd = b, d
        if best is not None:
            left.remove(best)
            out.append((a, best))
    return out


def summarise(name, values):
    if not values:
        print(f"  {name}: nothing paired")
        return
    n = len(values)
    pos = sum(1 for v in values if v > 0.005)
    neg = sum(1 for v in values if v < -0.005)
    zero = n - pos - neg
    mean = sum(values) / n
    absmean = sum(abs(v) for v in values) / n
    sd = math.sqrt(sum((v - mean) ** 2 for v in values) / n) if n > 1 else 0.0
    stderr = sd / math.sqrt(n) if n else 0.0
    print(f"  {name}: n={n}  mean {mean:+.4f} pt  (se {stderr:.4f}, so {abs(mean)/stderr if stderr else 0:.1f} se from zero)"
          f"  mean|.| {absmean:.4f}  +{pos} / -{neg} / 0 {zero}")


ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
ap.add_argument("ours"); ap.add_argument("ref"); ap.add_argument("--tol", type=float, default=3.0)
args = ap.parse_args()

a, b = records(args.ours), records(args.ref)
print(f"{len(a)} drawing records in ours, {len(b)} in the reference")
pairs = pair(a, b, args.tol)
print(f"{len(pairs)} paired within {args.tol} pt\n")

dx = [p[0]["x0"] - p[1]["x0"] for p in pairs]
dy = [p[0]["y0"] - p[1]["y0"] for p in pairs]
summarise("dx, all", dx)
summarise("dy, all", dy)
for kind in ("fill", "stroke"):
    summarise(f"dx, {kind}", [p[0]["x0"] - p[1]["x0"] for p in pairs if p[0]["kind"] == kind])
    summarise(f"dy, {kind}", [p[0]["y0"] - p[1]["y0"] for p in pairs if p[0]["kind"] == kind])

print("\nper page (dy):")
bypage = collections.defaultdict(list)
for o, r in pairs:
    bypage[o["page"]].append(o["y0"] - r["y0"])
for page in sorted(bypage):
    v = bypage[page]
    m = sum(v) / len(v)
    pos = sum(1 for x in v if x > 0.005); neg = sum(1 for x in v if x < -0.005)
    print(f"  p{page:3d}: n={len(v):3d} mean dy {m:+.4f}  +{pos:3d} / -{neg:3d}")
