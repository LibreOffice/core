#!/usr/bin/env python3
"""The first page on which the *flow* diverges, over all 200, matching documents included.

`first-divergence.py` answers "where does the ink first differ", which a font or a hairline
moves. This answers a narrower question that only a flow defect can move: on which page do
the two renderings stop holding the same amount of text? A page's extracted word count is
robust to a glyph substitution and to a wrong colour, and moves the moment a line lands on a
different page.

Reported with a tolerance so that a couple of tokens — a bullet the extractor surfaces on one
side, a footer field — do not read as a break in the flow. The exact-equality column is
printed beside it so the tolerance can be seen rather than trusted.
"""
from __future__ import annotations

import re
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

WORD = re.compile(r"[^\W_]+", re.UNICODE)
REFS = Path("/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/words")
TOL = 2


def counts(pdf: Path) -> list[int]:
    """Words per page, in one pdftotext pass — the per-page form is 200x slower."""
    text = subprocess.run(["pdftotext", str(pdf), "-"], capture_output=True, text=True).stdout
    return [len(WORD.findall(page)) for page in text.split("\f")[:-1]]


def first_break(a: list[int], b: list[int], tol: int) -> int | None:
    for i in range(min(len(a), len(b))):
        if abs(a[i] - b[i]) > tol:
            return i + 1
    return None if len(a) == len(b) else min(len(a), len(b)) + 1


def row(args):
    ident, ours = args
    ref = REFS / f"{ident}.pdf"
    if not ref.exists() or not ours.exists():
        return None
    o, r = counts(ours), counts(ref)
    return ident, len(o), len(r), first_break(o, r, TOL), first_break(o, r, 0)


if __name__ == "__main__":
    gate = {}
    for line in open(sys.argv[1]):
        f = line.rstrip("\n").split("\t")
        if len(f) < 7 or f[0] == "path":
            continue
        stem = Path(f[0]).stem
        gate[f"{stem}__{f[1]}"] = f[6]

    ourdir = Path(sys.argv[2])
    jobs = [(i, ourdir / f"{i}.pdf") for i in gate]
    with ThreadPoolExecutor(max_workers=8) as pool:
        rows = [r for r in pool.map(row, jobs) if r]

    print(f"{len(rows)} documents scored")
    buckets: dict[str, dict[str, int]] = {}
    for ident, no, nr, tolerant, exact in rows:
        v = gate[ident]
        key = "none" if tolerant is None else ("1" if tolerant == 1 else
                                               "2-4" if tolerant <= 4 else
                                               "5-20" if tolerant <= 20 else "21+")
        buckets.setdefault(key, {}).setdefault(v, 0)
        buckets[key][v] += 1
    verdicts = sorted({gate[i[0]] for i in rows})
    print("\nfirst page whose word count differs by more than", TOL)
    print(f"{'page':<8}" + "".join(f"{v:>14}" for v in verdicts))
    for key in ["none", "1", "2-4", "5-20", "21+"]:
        if key not in buckets:
            continue
        print(f"{key:<8}" + "".join(f"{buckets[key].get(v, 0):>14}" for v in verdicts))

    print("\npage failures, first flow break (tolerant / exact):")
    for ident, no, nr, tolerant, exact in sorted(rows, key=lambda r: (r[3] or 999)):
        if "pages" not in gate[ident]:
            continue
        print(f"  {str(tolerant):>4} / {str(exact):>4}   {no:>4}/{nr:<4}  {ident[:64]}")
