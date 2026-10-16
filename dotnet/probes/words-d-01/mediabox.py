#!/usr/bin/env python3
"""Compare the set of page sizes ours and the reference emit, over every document.

Measures the reach of a page-*geometry* difference without parsing any document: the PDF's
MediaBox is what each renderer decided the sheet is. Reported for all 200, matching documents
included, because a geometry difference that moves no page count is invisible to the gate and
is exactly what a page-count census would miss.
"""
from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

REFS = Path("/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/words")
TOL = 0.05  # pt; below this two MediaBoxes are the same sheet


def boxes(pdf: Path):
    out = subprocess.run(["pdfinfo", "-l", "100000", str(pdf)],
                         capture_output=True, text=True).stdout
    got = []
    for line in out.splitlines():
        m = re.match(r"Page\s+(\d+) size:\s+([\d.]+) x ([\d.]+)", line)
        if m:
            got.append((float(m.group(2)), float(m.group(3))))
    return got


def main() -> int:
    gate, oursdir = Path(sys.argv[1]), Path(sys.argv[2])
    print("verdict\tsame\tours\tref\tdocument")
    n = differ = 0
    for line in gate.read_text().splitlines()[1:]:
        f = line.split("\t")
        src = Path(f[0])
        ident = f"{src.stem}__{src.suffix.lower().lstrip('.')}.pdf"
        so = {(round(w, 2), round(h, 2)) for w, h in boxes(oursdir / ident)}
        sr = {(round(w, 2), round(h, 2)) for w, h in boxes(REFS / ident)}
        n += 1
        # A sheet in ours counts as matched when some reference sheet is within TOL of it.
        def near(a, pool):
            return any(abs(a[0] - b[0]) <= TOL and abs(a[1] - b[1]) <= TOL for b in pool)
        same = all(near(a, sr) for a in so) and all(near(b, so) for b in sr)
        if not same:
            differ += 1
        print(f"{f[6]}\t{'same' if same else 'DIFFER'}\t"
              f"{sorted(so)}\t{sorted(sr)}\t{f[0]}")
    print(f"\n# {differ} of {n} documents emit a sheet the reference does not", file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
