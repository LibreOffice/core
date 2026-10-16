#!/usr/bin/env python3
"""Dump a PDF page's font resources and the content-stream operators around a
given piece of text, so a Tf can be attributed to a real font resource."""
import sys, re, zlib
import pymupdf

path = sys.argv[1]
pno  = int(sys.argv[2]) if len(sys.argv) > 2 else 0
needle = sys.argv[3] if len(sys.argv) > 3 else None

doc = pymupdf.open(path)
page = doc[pno]

print("== font resources on page", pno)
for f in page.get_fonts(full=True):
    xref, ext, ftype, basefont, name, enc = f[:6]
    print(f"  /{name:10s} xref={xref:<5} basefont={basefont:<40} type={ftype} enc={enc}")

raw = b"".join(doc.xref_stream(x) for x in page.get_contents())
txt = raw.decode("latin-1")
print("== content stream bytes:", len(txt))
if needle:
    i = txt.find(needle)
    if i < 0:
        print("!! needle not found:", needle); sys.exit(1)
    print(txt[max(0, i-1400):i+1400])
