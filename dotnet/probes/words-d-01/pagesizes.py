#!/usr/bin/env python3
"""Per-page media-box sequence for ours and the reference, for a set of documents.

A page-count failure that is a *geometry* failure shows here as two different sequences of
page shapes; one that is a flow failure shows as the same shapes in the same order with one
repeated or missing. Neither is visible to a page *count*.
"""
from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

REFS = Path("/c/sandbox/workdir/refpdfs-26.2.4.2-fonts/words")


def boxes(pdf: Path):
    out = subprocess.run(["pdfinfo", "-l", "100000", str(pdf)],
                         capture_output=True, text=True).stdout
    got = []
    for line in out.splitlines():
        m = re.match(r"Page\s+(\d+) size:\s+([\d.]+) x ([\d.]+)", line)
        if m:
            w, h = float(m.group(2)), float(m.group(3))
            got.append((round(w), round(h)))
    return got


def code(seq):
    """Compress a shape sequence to letters, one per distinct shape."""
    order, letters = {}, []
    for s in seq:
        if s not in order:
            order[s] = chr(ord("A") + len(order))
        letters.append(order[s])
    # run-length
    out, prev, n = [], None, 0
    for ch in letters:
        if ch == prev:
            n += 1
        else:
            if prev:
                out.append(f"{prev}{n}" if n > 1 else prev)
            prev, n = ch, 1
    if prev:
        out.append(f"{prev}{n}" if n > 1 else prev)
    return "".join(out), {v: k for k, v in order.items()}


def main() -> int:
    gate = Path(sys.argv[1])
    want = sys.argv[2] if len(sys.argv) > 2 else "pages"
    oursdir = Path(sys.argv[3])
    for line in gate.read_text().splitlines()[1:]:
        f = line.split("\t")
        if want not in f[6]:
            continue
        src = Path(f[0])
        ident = f"{src.stem}__{src.suffix.lower().lstrip('.')}.pdf"
        o, r = oursdir / ident, REFS / ident
        so, sr = boxes(o), boxes(r)
        co, mo = code(so)
        cr, mr = code(sr)
        flag = "SAME-SHAPES" if sorted(set(so)) == sorted(set(sr)) else "SHAPES-DIFFER"
        same = "same-seq" if co == cr else "SEQ-DIFFERS"
        print(f"{f[2]}\t{same}\t{flag}\t{src.name}")
        print(f"    ours {co}   {mo}")
        print(f"    ref  {cr}   {mr}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
