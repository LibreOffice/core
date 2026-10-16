#!/usr/bin/env python3
"""Dump every drawn text line of one page: size (from the content stream via pymupdf spans),
origin, and text.  Sizes are the /Tf size; origins are the Td/Tm baseline pymupdf reports."""
import sys, pymupdf
doc=pymupdf.open(sys.argv[1]); pg=int(sys.argv[2])
p=doc[pg-1]
print(f"# {sys.argv[1]} page {pg}  mediabox={p.rect}")
for b in p.get_text("dict")["blocks"]:
    if "lines" not in b: continue
    for l in b["lines"]:
        for s in l["spans"]:
            print(f"{s['size']:8.3f}  x={s['origin'][0]:8.2f} y={s['origin'][1]:8.2f}  {s['font'][:28]:28} {s['text'][:70]!r}")
