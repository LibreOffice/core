#!/usr/bin/env python3
"""Patch one attribute of a corpus .odp at a time and read the baselines back.

The question is why 26.2.4.2 draws a 15.99 pt baseline pitch inside a paragraph of
16 pt text and 23.19 pt across a paragraph boundary, when a fixed-cell-height line
is 1.2 em = 19.19 pt throughout.  Each variant changes exactly one thing.
"""
import os, re, shutil, subprocess, sys, zipfile

SRC = sys.argv[1] if len(sys.argv) > 1 else \
    "/home/user/corpus-odf/slides/done-004/odp/0335fab9-79f0-4944-b92c-f223837ca2d8.odp"
OUT = sys.argv[2] if len(sys.argv) > 2 else "/tmp/odp-spacing"
PAGE = int(sys.argv[3]) if len(sys.argv) > 3 else 5
HERE = os.path.dirname(os.path.abspath(__file__))

def rebuild(dst, edits):
    zin = zipfile.ZipFile(SRC)
    with zipfile.ZipFile(dst, "w", zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            data = zin.read(item.filename)
            if item.filename in edits:
                data = edits[item.filename](data.decode("utf-8")).encode("utf-8")
            if item.filename == "mimetype":
                zout.writestr(zipfile.ZipInfo("mimetype"), data, zipfile.ZIP_STORED)
            else:
                zout.writestr(item, data)
    zin.close()

def drop_links(x):
    x = re.sub(r'<text:a [^>]*>', '', x)
    return x.replace('</text:a>', '')

VARIANTS = {
    "v0-control":       {},
    "v1-no-links":      {"content.xml": drop_links},
    "v2-fils-false":    {"content.xml": lambda x: x.replace('style:font-independent-line-spacing="true"', 'style:font-independent-line-spacing="false"'),
                         "styles.xml":  lambda x: x.replace('style:font-independent-line-spacing="true"', 'style:font-independent-line-spacing="false"')},
    "v3-no-lineheight": {"styles.xml":  lambda x: x.replace(' fo:line-height="100%"', '')},
    "v4-no-underline":  {"content.xml": lambda x: re.sub(r'style:text-underline-[a-z-]+="[^"]*"', '', x)},
}

os.makedirs(OUT, exist_ok=True)
for name, edits in VARIANTS.items():
    doc = os.path.join(OUT, name + ".odp")
    rebuild(doc, edits)
    subprocess.run([os.path.join(HERE, "ref-render.sh"), doc, OUT], check=False)

import pymupdf
for name in VARIANTS:
    pdf = os.path.join(OUT, name + ".pdf")
    if not os.path.exists(pdf):
        print(name, "NO OUTPUT"); continue
    doc = pymupdf.open(pdf)
    if PAGE >= doc.page_count:
        print(name, "page missing"); continue
    ys = []
    for b in doc[PAGE].get_text("dict")["blocks"]:
        if b["type"]: continue
        for l in b["lines"]:
            t = "".join(s["text"] for s in l["spans"])
            if t.strip():
                ys.append((l["spans"][0]["origin"][1], round(l["spans"][0]["size"], 2), t[:34]))
    ys.sort()
    print("==", name)
    prev = None
    for y, sz, t in ys:
        print("   %8.3f  %+7.3f  %5.2f  %s" % (y, (y - prev) if prev else 0, sz, t))
        prev = y
