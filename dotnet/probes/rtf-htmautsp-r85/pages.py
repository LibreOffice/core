#!/usr/bin/env python3
"""Page count of every PDF in a directory, paired by the probe's own stem.

    pages.py <outdir/ref>
"""
import sys, pathlib, pymupdf
D = pathlib.Path(sys.argv[1])
rows = {}
for f in sorted(D.glob("*.pdf")):
    doc = pymupdf.open(f); n = doc.page_count; doc.close()
    stem, _, arm = f.stem.rpartition("-")
    rows.setdefault(stem, {})[arm] = n
print(f"{'probe':>14s} {'word':>6s} {'none':>6s}  verdict")
for k in sorted(rows):
    r = rows[k]
    w, o = r.get("word"), r.get("none")
    v = "?" if w is None or o is None else ("honoured" if w != o else "IGNORED")
    print(f"{k:>14s} {str(w):>6s} {str(o):>6s}  {v}")
