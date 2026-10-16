#!/usr/bin/env python3
"""Where a chart part's paragraph default disagrees with the run beside it.

`DrawingChartPlot.SizeOf`, `BoldOf` and `LiteralFamily` take the **first** `a:defRPr` or
`a:rPr` in document order under a titled element.  A `c:rich` writes `a:pPr/a:defRPr`
before `a:r/a:rPr`, so wherever a run states something different from its paragraph's
default, the default is what we read and the run is what LibreOffice draws.

Censused across all three families, because `XRay.Office.Ooxml` serves all three.
Refuses to report unless every manifest row produced output.
"""
import collections, os, re, sys, zipfile
import xml.etree.ElementTree as ET

CORPUS = "/c/sandbox/workdir/sample-files"
C = "{http://schemas.openxmlformats.org/drawingml/2006/chart}"
A = "{http://schemas.openxmlformats.org/drawingml/2006/main}"

rows = []
with open(os.path.join(CORPUS, "MANIFEST.tsv"), encoding="utf-8") as fh:
    fh.readline()
    for line in fh:
        f = line.rstrip("\n").split("\t")
        rows.append({"family": f[0], "path": f[2], "ext": f[3], "status": f[7]})


def props(el):
    """(sz, b, latin) of a run-properties element."""
    latin = el.find(A + "latin")
    return (el.get("sz"), el.get("b"),
            (latin.get("typeface") if latin is not None else None))


def scan(part):
    """Per titled element: does the first defRPr disagree with the first rPr?"""
    out = collections.Counter()
    try:
        root = ET.fromstring(part)
    except ET.ParseError:
        return out, 0

    holders = []
    for name in ("title", "dLbls", "dLbl", "txPr"):
        holders += [(name, e) for e in root.iter(C + name)]
    charts = 1

    for kind, holder in holders:
        rich = holder.find(".//" + C + "rich")
        if rich is None:
            rich = holder.find(".//" + A + "p")
            if rich is None:
                continue
        firstdef = firstrun = None
        for p in holder.iter():
            if p.tag == A + "defRPr" and firstdef is None:
                firstdef = p
            elif p.tag == A + "rPr" and firstrun is None:
                firstrun = p
        if firstdef is None or firstrun is None:
            continue
        d, r = props(firstdef), props(firstrun)
        # only count where the run states something and it differs
        for i, field in enumerate(("sz", "b", "latin")):
            if r[i] is not None and d[i] is not None and r[i] != d[i]:
                out["%s.%s" % (kind, field)] += 1
            elif r[i] is not None and d[i] is None:
                out["%s.%s(default absent)" % (kind, field)] += 1
    return out, charts


results, errors = {}, {}
for row in rows:
    full = os.path.join(CORPUS, row["path"])
    per = collections.Counter()
    charts = 0
    try:
        if zipfile.is_zipfile(full):
            with zipfile.ZipFile(full) as z:
                for n in z.namelist():
                    low = n.lower()
                    if "/charts/chart" not in low or not low.endswith(".xml"):
                        continue
                    c, k = scan(z.read(n))
                    per += c
                    charts += k
        results[row["path"]] = (row, per, charts)
    except Exception as exc:                      # noqa: BLE001
        errors[row["path"]] = repr(exc)

if errors:
    print("REFUSING TO REPORT — %d of %d inputs failed" % (len(errors), len(rows)), file=sys.stderr)
    for k, v in list(errors.items())[:10]:
        print("  ", k, v, file=sys.stderr)
    sys.exit(2)

assert len(results) == len(rows)
print("inputs: %d manifest rows, %d produced output, 0 failures\n" % (len(rows), len(results)))

byfam = collections.defaultdict(lambda: [0, 0, collections.Counter(), 0])
for path, (row, per, charts) in results.items():
    slot = byfam[row["family"]]
    slot[3] += 1
    if charts:
        slot[0] += 1
    if per:
        slot[1] += 1
        slot[2] += per

for fam, (withchart, withdiff, counts, total) in sorted(byfam.items()):
    print("%-8s %d documents, %d hold a chart part, %d have a run that disagrees with its "
          "paragraph default" % (fam, total, withchart, withdiff))
    for k, v in counts.most_common(12):
        print("      %-34s %d" % (k, v))

print("\ndocuments affected, by name (sheets first):")
for fam in ("sheets", "slides", "words"):
    names = sorted(os.path.basename(p) for p, (row, per, _) in results.items()
                   if row["family"] == fam and per)
    print("  %-7s %d: %s" % (fam, len(names), ", ".join(names[:6]) + (" …" if len(names) > 6 else "")))
