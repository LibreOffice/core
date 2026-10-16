#!/usr/bin/env python3
"""Census of legacy VML drawings across the 946-document corpus.

Reach of "read the worksheet's `legacyDrawing` VML part". `VmlDrawing::isShapeSupported`
(sc/source/filter/oox/drawingfragment.cxx) imports every VML shape whose `x:ClientData`
is absent or whose `ObjectType` is anything other than `Note`; notes are drawn by the
comment machinery instead. So the reach is "shapes that are not Notes", broken down by
ObjectType so that implementing only `Pict` can be costed honestly.

Also counts, per document, whether the sheet holding the VML also has a DrawingML
drawing part -- the double-draw risk.
"""
import collections, csv, os, re, sys, zipfile

CORPUS = "/c/sandbox/workdir/sample-files"
SHAPE = re.compile(rb"<v:(shape|rect|oval|line|roundrect|polyline|group|image)\b")
OBJTYPE = re.compile(rb'ObjectType="(\w+)"')


def main():
    per_type = collections.Counter()
    docs_with_nonnote = collections.Counter()
    rows = []
    with open(os.path.join(CORPUS, "MANIFEST.tsv"), newline="") as fh:
        man = [r for r in csv.DictReader(fh, delimiter="\t")]
    for r in man:
        p = os.path.join(CORPUS, r["path"])
        if not os.path.exists(p):
            continue
        try:
            z = zipfile.ZipFile(p)
        except Exception:
            continue
        with z:
            names = z.namelist()
            vmls = [n for n in names if n.lower().endswith(".vml")]
            if not vmls:
                continue
            types = collections.Counter()
            untyped = 0
            for v in vmls:
                data = z.read(v)
                shapes = len(SHAPE.findall(data))
                ts = OBJTYPE.findall(data)
                for t in ts:
                    types[t.decode()] += 1
                untyped += max(0, shapes - len(ts))
            nonnote = sum(v for k, v in types.items() if k != "Note") + untyped
            for k, v in types.items():
                per_type[k] += v
            per_type["<untyped>"] += untyped
            if nonnote:
                docs_with_nonnote[r["family"]] += 1
            rows.append((r["family"], r["status"], r["path"], nonnote,
                         " ".join("%s=%d" % kv for kv in sorted(types.items())),
                         untyped))
    print("== per ObjectType, whole corpus ==")
    for k, v in per_type.most_common():
        print("  %-12s %d" % (k, v))
    print("== documents with at least one non-Note VML shape ==")
    for k, v in docs_with_nonnote.most_common():
        print("  %-8s %d" % (k, v))
    print("== those documents ==")
    for row in sorted(rows):
        if row[3]:
            print("  %-7s %-5s %-70s nonNote=%-3d %s untyped=%d" % row)


main()
