#!/usr/bin/env python3
"""Effective reach of the run-over-default change, simulating both readers.

The first cut of this census counted every place a run states something different from
its paragraph's default, and that OVER-counts: the old reader skipped a `defRPr` that did
not state the field at all and went on to the run, so "the default is absent" is not a
change.  This one runs both rules and counts only where the ANSWER moves.

Old: the first `defRPr`-or-`rPr` in document order that states the field.
New: the first `rPr` that states it; failing that, the first `defRPr` that does.

Titled elements are the only consumers: `c:title` (chart and axis) for size, weight and
family; `c:txPr` and `c:dLbls` for label size and weight.  Refuses to report unless every
manifest row produced output.
"""
import collections, os, sys, zipfile
import xml.etree.ElementTree as ET

CORPUS = "/c/sandbox/workdir/sample-files"
C = "{http://schemas.openxmlformats.org/drawingml/2006/chart}"
A = "{http://schemas.openxmlformats.org/drawingml/2006/main}"
FIELDS = ("sz", "b", "latin")


def stated(el, field):
    if field == "latin":
        latin = el.find(A + "latin")
        face = latin.get("typeface") if latin is not None else None
        return None if not face or face.startswith("+") else face
    v = el.get(field)
    return v if v not in (None, "") else None


def answers(holder):
    seq = [e for e in holder.iter() if e.tag in (A + "defRPr", A + "rPr")]
    out = {}
    for f in FIELDS:
        old = next((stated(e, f) for e in seq if stated(e, f) is not None), None)
        new = next((stated(e, f) for e in seq if e.tag == A + "rPr" and stated(e, f) is not None),
                   None)
        if new is None:
            new = next((stated(e, f) for e in seq
                        if e.tag == A + "defRPr" and stated(e, f) is not None), None)
        out[f] = (old, new)
    return out


rows = []
with open(os.path.join(CORPUS, "MANIFEST.tsv"), encoding="utf-8") as fh:
    fh.readline()
    for line in fh:
        f = line.rstrip("\n").split("\t")
        rows.append({"family": f[0], "path": f[2], "status": f[7]})

results, errors = {}, {}
for row in rows:
    full = os.path.join(CORPUS, row["path"])
    moved = collections.Counter()
    examples = []
    try:
        if zipfile.is_zipfile(full):
            with zipfile.ZipFile(full) as z:
                for n in z.namelist():
                    low = n.lower()
                    if "/charts/chart" not in low or not low.endswith(".xml"):
                        continue
                    try:
                        root = ET.fromstring(z.read(n))
                    except ET.ParseError:
                        continue
                    for kind in ("title", "txPr", "dLbls"):
                        for holder in root.iter(C + kind):
                            for f, (old, new) in answers(holder).items():
                                if old != new:
                                    moved["%s.%s" % (kind, f)] += 1
                                    if len(examples) < 3:
                                        examples.append("%s %s %s->%s" % (kind, f, old, new))
        results[row["path"]] = (row, moved, examples)
    except Exception as exc:                       # noqa: BLE001
        errors[row["path"]] = repr(exc)

if errors:
    print("REFUSING TO REPORT — %d of %d failed" % (len(errors), len(rows)), file=sys.stderr)
    sys.exit(2)
assert len(results) == len(rows)
print("inputs: %d manifest rows, %d produced output, 0 failures\n" % (len(rows), len(results)))

for fam in ("sheets", "slides", "words"):
    sub = [(p, m, e) for p, (r, m, e) in results.items() if r["family"] == fam]
    hit = [(p, m, e) for p, m, e in sub if m]
    tot = collections.Counter()
    for _, m, _ in hit:
        tot += m
    print("%-7s %d documents, %d change an answer" % (fam, len(sub), len(hit)))
    for k, v in tot.most_common():
        print("        %-16s %d sites" % (k, v))
    for p, m, e in sorted(hit)[:6 if fam == "sheets" else 20]:
        print("        %-62s %s" % (os.path.basename(p)[:62], "; ".join(e)))
    if fam == "sheets" and len(hit) > 6:
        print("        … and %d more, all of the same shape" % (len(hit) - 6))
