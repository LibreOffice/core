#!/usr/bin/env python3
"""Which documents this round's pie change can reach — corrected twice.

The first cut of this census matched `<c:pieChart>` and answered 5 sheets, 3 slides, 2 words
documents.  It was wrong in two independent ways and the corpus sweep found the first one:

 1. **A chart part may bind the chart namespace as the default**, with no `c:` prefix at all.
    `microsoft_learn_multi_chart_examples.xlsx` does exactly that, and a prefix-anchored regex
    reports it as holding no chart of any kind.  Its word count moved in the sweep, from a
    document the census said could not be touched.
 2. **`c:dLblPos` is optional and a pie's default placement is `bestFit`**
    (`typegroupconverter.cxx:95-107`), so counting documents that *state* it counts a floor.

Both corrections are in this file; the prediction file's blind spot 3 named the second before
the sweep and did not name the first.

A doughnut is excluded: `bMovementAllowed && !m_bUseRings` gates the whole mechanism off for a
ring chart, so a `doughnutChart` part reaches nothing however its labels are placed.
"""
import collections, os, re, sys, zipfile

CORPUS = "/c/sandbox/workdir/sample-files"

PIE = re.compile(rb'<(?:c:)?(pieChart|pie3DChart|ofPieChart)\b')
RING = re.compile(rb'<(?:c:)?doughnutChart\b')
POS = re.compile(rb'<(?:c:)?dLblPos +val="([A-Za-z]+)"')
DLBLS = re.compile(rb'<(?:c:)?dLbls>')
KEY = re.compile(rb'<(?:c:)?showLegendKey +val="1"')

rows = []
with open(os.path.join(CORPUS, "MANIFEST.tsv"), encoding="utf-8") as fh:
    fh.readline()
    for line in fh:
        f = line.rstrip("\n").split("\t")
        rows.append((f[0], f[2], f[3].lower(), f[7]))

reach = collections.defaultdict(list)
seen = 0
failures = []

for family, path, ext, status in rows:
    if ext not in ("xlsx", "xlsm", "pptx", "docx"):
        seen += 1
        continue
    try:
        why = []
        with zipfile.ZipFile(os.path.join(CORPUS, path)) as z:
            for name in z.namelist():
                low = name.lower()
                if "/charts/" not in low or not low.endswith(".xml"):
                    continue
                data = z.read(name)
                if not PIE.search(data):
                    continue
                if not DLBLS.search(data):
                    continue
                stated = {m.group(1).decode() for m in POS.finditer(data)}
                if "bestFit" in stated:
                    why.append("states bestFit")
                elif not stated:
                    why.append("no dLblPos — the pie default is bestFit")
                if KEY.search(data):
                    why.append("showLegendKey")
                if "outEnd" in stated:
                    why.append("outEnd (unchanged this round)")
        if why:
            reach[family].append((path, status, sorted(set(why))))
        seen += 1
    except Exception as exc:                                    # noqa: BLE001
        failures.append((path, repr(exc)))

if failures or seen != len(rows):
    print("REFUSING TO SUMMARISE — %d of %d rows produced no result"
          % (len(rows) - seen, len(rows)), file=sys.stderr)
    for p, e in failures[:20]:
        print("   ", p, e, file=sys.stderr)
    sys.exit(2)

print("inputs: %d manifest rows, %d produced output, 0 failures" % (len(rows), seen))
for family in ("sheets", "slides", "words"):
    print("\n=== %s — %d documents a bestFit pie label can reach ===" % (family, len(reach[family])))
    for path, status, why in sorted(reach[family]):
        print("  %-6s %-78s %s" % (status, path[len(family) + 1:], "; ".join(why)))
