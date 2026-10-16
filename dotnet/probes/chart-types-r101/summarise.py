#!/usr/bin/env python3
"""Fold the four raw censuses into the counts the results table quotes."""
import csv, collections, os

def load(p): return list(csv.DictReader(open(p), delimiter="\t"))

ooxml = load("ooxml-raw.tsv")
odf   = load("odf-joined.tsv")
biff  = load("biff-raw.tsv")
pptole= load("ppt-ole.tsv")

def docs(rows, feat, key="path", ffield="feature", pred=None):
    return {r[key] for r in rows if r[ffield]==feat and (pred is None or pred(r))}

print("== OOXML c:/cx: markup, distinct documents (den. 766 docx+xlsx+pptx+xlsm)")
d=collections.defaultdict(set); parts=collections.defaultdict(set)
for r in ooxml:
    d[r["feature"]].add(r["path"]); parts[r["feature"]].add(r["path"]+"|"+r["part"])
for f in sorted(d, key=lambda x:-len(d[x])):
    print("  %-24s %3d docs  %3d parts" % (f, len(d[f]), len(parts[f])))

print("\n== OOXML documents holding any chart part, by ext")
byext=collections.defaultdict(set)
for r in ooxml: byext[r["ext"]].add(r["path"])
for e in sorted(byext): print("  %-6s %3d" % (e, len(byext[e])))

print("\n== reference-resolved chart:class (corpus-odf, LibreOffice 26.2.4.2 export)")
dd=collections.defaultdict(set)
for r in odf: dd[r["feature"]].add(r["srcpath"])
for f in sorted(dd, key=lambda x:-len(dd[x])):
    print("  %-56s %3d docs" % (f, len(dd[f])))

print("\n== reference-resolved diagram class by source ext")
t=collections.defaultdict(set)
for r in odf:
    if r["feature"].startswith("class:"): t[(r["feature"],r["srcext"])].add(r["srcpath"])
for k in sorted(t): print("  %-56s %-5s %3d" % (k[0],k[1],len(t[k])))

print("\n== BIFF chart-type records in .xls (den. 64)")
b=collections.defaultdict(set)
for r in biff: b[r["feature"]].add(r["path"])
for f in sorted(b, key=lambda x:-len(b[x])): print("  %-22s %3d docs" % (f, len(b[f])))

print("\n== OLE-embedded charts in .ppt (den. 51)")
p=collections.defaultdict(set)
for r in pptole: p[r["feature"]].add(r["path"])
for f in sorted(p, key=lambda x:-len(p[x])):
    if f.startswith("CH") or f.startswith("clsid:0002080") or f.startswith("clsid:0002082"):
        print("  %-46s %3d docs" % (f, len(p[f])))
