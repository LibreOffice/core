#!/usr/bin/env python3
"""Total |ink|% against a 26.2.4.2 reference for a list of renderings, before and after.

    ink.py <ids.txt> <before-dir> <after-dir> <ref26-dir>

Sums `pdf-image-diff.py`'s own per-page `|ink|%` column, which is what
`words-ink-r67/rank.py` ranks on.
"""
import re
import subprocess
import sys
from pathlib import Path

SCRIPT = "/home/user/wt-wordsgap/.claude/skills/render-comparison/scripts/pdf-image-diff.py"
ids = [line.strip() for line in open(sys.argv[1]) if line.strip()]
before, after, ref = (Path(sys.argv[i]) for i in (2, 3, 4))


def pages(path):
    out = subprocess.run(["pdfinfo", str(path)], capture_output=True, text=True).stdout
    found = re.search(r"^Pages:\s+(\d+)", out, re.M)
    return int(found.group(1)) if found else 0


def ink(ours, reference):
    out = subprocess.run([sys.executable, SCRIPT, str(ours), str(reference), "--long-edge", "512"],
                         capture_output=True, text=True).stdout
    total = 0.0
    for line in out.splitlines():
        fields = line.split("\t")
        if len(fields) >= 4 and fields[0].strip().isdigit():
            try:
                total += abs(float(fields[3]))
            except ValueError:
                pass
    return total


print(f"{'document':66s} {'b/a/ref':>12s} {'|ink| before':>13s} {'|ink| after':>12s}")
totals = [0.0, 0.0]
for one in ids:
    b = ink(before / f"{one}.pdf", ref / f"{one}.pdf")
    a = ink(after / f"{one}.pdf", ref / f"{one}.pdf")
    totals[0] += b
    totals[1] += a
    shape = f"{pages(before / f'{one}.pdf')}/{pages(after / f'{one}.pdf')}/{pages(ref / f'{one}.pdf')}"
    print(f"{one[:66]:66s} {shape:>12s} {b:13.2f} {a:12.2f}")
print(f"{'TOTAL':66s} {'':12s} {totals[0]:13.2f} {totals[1]:12.2f}")
