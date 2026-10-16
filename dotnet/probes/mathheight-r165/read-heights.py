#!/usr/bin/env python3
"""Read 26.2.4.2's own `svg:height` for each fixture's formula object out of its fodt."""
import os, re, sys
HERE = os.path.dirname(os.path.abspath(__file__))
IN = "in" if len(sys.argv) < 2 else sys.argv[1]

def pt(v):
    m = re.match(r"^(-?[0-9.]+)(in|cm|mm|pt)$", v)
    n = float(m.group(1))
    return {"in": 72.0, "cm": 72 / 2.54, "mm": 7.2 / 2.54, "pt": 1.0}[m.group(2)] * n

rows = []
for name in sorted(os.listdir(os.path.join(HERE, "fodt"))):
    if not name.endswith(".fodt"):
        continue
    s = open(os.path.join(HERE, "fodt", name), encoding="utf-8").read()
    hs = [pt(m) for m in re.findall(r'<draw:frame[^>]*\ssvg:height="([^"]+)"', s)]
    ws = [pt(m) for m in re.findall(r'<draw:frame[^>]*\ssvg:width="([^"]+)"', s)]
    rows.append((name[:-5], hs, ws))
w = max(len(r[0]) for r in rows)
print(f"{'fixture':<{w}}  {'n':>2}  {'height pt':>9}  {'width pt':>9}")
for nm, hs, ws in rows:
    print(f"{nm:<{w}}  {len(hs):>2}  " + ("  ".join(f"{h:9.3f}" for h in hs) if hs else " (none)  ")
          + "  " + "  ".join(f"{x:9.3f}" for x in ws))
