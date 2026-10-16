#!/usr/bin/env python3
"""Which slides documents hold a body the fit search is asked to solve?

Two readers, two shapes:

  OOXML  -- <a:normAutofit> on a a:bodyPr, wherever it is stated, PLUS every SmartArt
            drawing, whose tx nodes are autofitted whether they ask or not.
  binary -- a text shape whose TextHeaderAtom kind is Body/HalfBody/QuarterBody and whose
            Escher OPT does not set fFitShapeToText (see ppt-autofit-census.py).

This is a CANDIDATE count and says so.  A candidate only moves if its text actually
overflows its box, and only changes if the level the reference lands on differs from the
scale our bisection lands on.  Both of those are properties of the rendering, not of the
markup, and neither is visible from here.
"""
import collections, os, re, sys, zipfile

MANIFEST = "/c/sandbox/workdir/sample-files/MANIFEST.tsv"
CORPUS = "/c/sandbox/workdir/sample-files"

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

rows = []
with open(MANIFEST, encoding="utf-8") as fh:
    hdr = fh.readline().rstrip("\n").split("\t")
    for line in fh:
        r = dict(zip(hdr, line.rstrip("\n").split("\t")))
        if r["family"] == "slides":
            rows.append(r)

NORM = re.compile(rb"<a:normAutofit")
DGM = re.compile(rb"drawingml/2006/diagram")

ooxml = collections.Counter()
detail = []
binary = []

for r in rows:
    path = os.path.join(CORPUS, r["path"])
    ext = r["ext"].lower()
    if ext == "ppt":
        binary.append(r)
        continue
    try:
        z = zipfile.ZipFile(path)
    except Exception as exc:
        print("SKIP", r["path"], exc, file=sys.stderr)
        continue
    slides = layouts = masters = other = smart = 0
    for n in z.namelist():
        if not n.endswith(".xml"):
            continue
        try:
            data = z.read(n)
        except Exception:
            continue
        c = len(NORM.findall(data))
        if c:
            if n.startswith("ppt/slides/"):
                slides += c
            elif n.startswith("ppt/slideLayouts/"):
                layouts += c
            elif n.startswith("ppt/slideMasters/"):
                masters += c
            else:
                other += c
        if n.startswith("ppt/diagrams/") and n.endswith("drawing.xml"):
            smart += 1
    z.close()
    if slides or layouts or masters or other or smart:
        detail.append((r["path"], r["status"], slides, layouts, masters, other, smart))
    ooxml["docs"] += 1
    if slides:
        ooxml["slide-stated"] += 1
    if layouts or masters:
        ooxml["inherited-only"] += 1 if not slides else 0
    if smart:
        ooxml["smartart"] += 1

print(f"{'document':70} {'status':9} {'sld':>4} {'lay':>4} {'mst':>4} {'oth':>4} {'dgm':>4}")
print("-" * 106)
for p, st, a, b, c, d, e in sorted(detail, key=lambda t: -t[2]):
    print(f"{os.path.basename(p)[:69]:70} {st:9} {a:4d} {b:4d} {c:4d} {d:4d} {e:4d}")

print()
print(f"OOXML slides documents                                : {ooxml['docs']}")
print(f"  with >=1 a:normAutofit on a SLIDE part              : {ooxml['slide-stated']}")
print(f"  with a:normAutofit only in a layout/master          : {ooxml['inherited-only']}")
print(f"  with >=1 SmartArt drawing (autofit whether or not)  : {ooxml['smartart']}")
print(f"  candidates (any of the above)                       : {len(detail)}")
print(f"binary .ppt documents                                 : {len(binary)}")
