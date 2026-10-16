#!/usr/bin/env python3
"""Two independent censuses of the same corpus, and their per-type union.

Leg A - markup: OOXML chart parts unzipped and grepped, BIFF chart-type records
        scanned out of .xls Workbook streams and out of .ppt ExOleObjStg objects.
Leg B - the reference's own resolution: chart:class on <chart:chart> in
        /home/user/corpus-odf, which is LibreOffice 26.2.4.2's export of the
        same 947 documents (meta:generator checked).
"""
import csv, collections

def rows(p): return list(csv.DictReader(open(p), delimiter="\t"))

ox, odf = rows("ooxml-raw.tsv"), rows("odf-joined.tsv")
bx, pp  = rows("biff-raw.tsv"), rows("ppt-ole.tsv")

ox_docs  = {r["path"] for r in ox}
bx_docs  = {r["path"] for r in bx if r["ext"] == "xls" and r["feature"] == "CHCHART"}
pp_docs  = {r["path"] for r in pp if r["feature"] == "CHCHART"}
odf_docs = {r["srcpath"] for r in odf if r["feature"].startswith("class:")}

print("leg A, documents stating chart markup")
print("  OOXML chart part          %3d" % len(ox_docs))
print("  .xls  BIFF CHCHART        %3d" % len(bx_docs))
print("  .ppt  OLE  CHCHART        %3d" % len(pp_docs))
print("  union                     %3d" % len(ox_docs | bx_docs | pp_docs))
print("leg B, documents the reference resolves to a chart")
print("  <chart:chart chart:class> %3d" % len(odf_docs))
print()
print("A minus B (%d):" % len((ox_docs | bx_docs | pp_docs) - odf_docs))
for x in sorted((ox_docs | bx_docs | pp_docs) - odf_docs): print("   ", x)
print("B minus A (%d):" % len(odf_docs - (ox_docs | bx_docs | pp_docs)))
for x in sorted(odf_docs - (ox_docs | bx_docs | pp_docs)): print("   ", x)

# per-type union over all three markup censuses
TYPE = collections.defaultdict(set)
for r in ox:
    if r["feature"].startswith(("c:", "cx:")): TYPE[r["feature"]].add(r["path"])
for src in (bx, pp):
    for r in src:
        if r["feature"].startswith("CH") and r["feature"] not in ("CHCHART", "CHTYPEGROUP"):
            TYPE[r["feature"]].add(r["path"])
print("\nper-type document counts, markup leg")
for k in sorted(TYPE, key=lambda x: -len(TYPE[x])):
    print("  %-24s %3d" % (k, len(TYPE[k])))
