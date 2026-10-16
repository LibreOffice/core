#!/usr/bin/env python3
"""Print every text span of one page with its bbox, font and size.

    spans.py <pdf> [page] [--limit N]

PyMuPDF reports a *rotated* span's axis-aligned box, so the line's own `dir` vector is
printed beside it: a 45-degree label comes back as a square and reads exactly like an
unrotated one far too wide for its slot.
"""
import sys, pymupdf

pdf = sys.argv[1]
page = int(sys.argv[2]) if len(sys.argv) > 2 else 0
limit = int(sys.argv[sys.argv.index("--limit") + 1]) if "--limit" in sys.argv else 60

d = pymupdf.open(pdf)
p = d[page]
print(f"# {pdf} page {page + 1} of {d.page_count}  rect={p.rect}")
n = 0
for block in p.get_text("dict")["blocks"]:
    for line in block.get("lines", []):
        for span in line["spans"]:
            x0, y0, x1, y1 = span["bbox"]
            print(f"{y0:9.3f} {y1:9.3f} {x0:9.3f} {x1:9.3f} "
                  f"{span['size']:6.2f} {span['font']:28s} dir={line['dir']} "
                  f"{span['text'][:50]!r}")
            n += 1
            if n >= limit:
                sys.exit(0)
