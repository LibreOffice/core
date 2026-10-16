#!/usr/bin/env python3
"""Which corpus documents state `c:minorGridlines` (or ODF's minor-grid) on an axis?

`c:minorGridlines` is unread by this reader -- it appears nowhere in `Core/Charts` -- and the
reference draws it as a full mesh across the plot area.  On `Demick_JetBlue.pptx` page 4 that
mesh is the whole of the cmp report's "a solid area drawn differently (31.18% of page)".

Counts, per document, the axes that state one, split by whether the axis is DELETED
(`c:delete val="1"`), because a deleted axis' gridlines still draw in chart2 but its labels do
not -- and by whether the chart part is actually referenced by a slide/sheet/story.

What it CANNOT see:
  * ODF charts (`chart:grid class="minor"`) inside .odp/.ods -- counted separately by tag;
  * legacy .xls/.ppt embedded BIFF charts, whose minor-gridline flag is a record bit;
  * whether the axis has enough room for a minor interval to differ from the major one.

    minorgrid-census.py <corpus-root> [family ...]
"""
import collections, os, re, sys, zipfile

OOXML = re.compile(rb"<c:minorGridlines\s*/?>")
DELETE = re.compile(rb"<c:delete\s+val=\"1\"\s*/>")
ODF = re.compile(rb"chart:grid[^>]*class=\"minor\"")


def scan(path):
    n = nodf = 0
    parts = 0
    try:
        with zipfile.ZipFile(path) as z:
            for name in z.namelist():
                low = name.lower()
                if low.endswith(".xml") and ("chart" in low or low.endswith("content.xml")):
                    try:
                        data = z.read(name)
                    except Exception:
                        continue
                    hits = len(OOXML.findall(data))
                    o = len(ODF.findall(data))
                    if hits or o:
                        parts += 1
                    n += hits
                    nodf += o
    except Exception:
        return None
    return n, nodf, parts


if __name__ == "__main__":
    root = sys.argv[1]
    fams = sys.argv[2:] or ["slides", "sheets", "words"]
    man = os.path.join(root, "MANIFEST.tsv")
    rows = []
    with open(man, encoding="utf-8") as fh:
        hdr = fh.readline().rstrip("\n").split("\t")
        for line in fh:
            r = dict(zip(hdr, line.rstrip("\n").split("\t")))
            if r["family"] in fams:
                rows.append(r)
    per = collections.Counter()
    docs = collections.Counter()
    for r in rows:
        got = scan(os.path.join(root, r["path"]))
        if not got:
            continue
        n, nodf, parts = got
        if n or nodf:
            docs[r["family"]] += 1
            per[r["family"]] += n + nodf
            print(f"{r['family']:7} {n:4} ooxml {nodf:4} odf  {parts:2} parts  {r['path']}")
    print("---")
    for f in fams:
        print(f"{f}: {docs[f]} documents, {per[f]} minorGridlines elements")
