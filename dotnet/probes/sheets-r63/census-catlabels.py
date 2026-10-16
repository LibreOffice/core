#!/usr/bin/env python3
"""Which documents can move when the axis wrap limit changes.

Round 62's `census-chartreach.py` counts every document holding chart *text*, which was the right
reach for a change to the advance width.  This change is narrower: `ChartAxisLabels.Wraps` is
asked only about a **category axis with labels of its own**, and only where line breaking is on
and overlap is not allowed.  A chart with no `c:catAx`, or one whose `c:catAx` is deleted, or one
whose labels are turned off, cannot move however much text it holds.

Counted per document and per part, by family, from `MANIFEST.tsv`.  What it cannot see is written
into `prediction.md` beside it: whether we draw the chart at all, whether the chart is on a
printed page, and the BIFF and ODF charts, which take the same limit and are counted separately
here rather than folded in.
"""
import collections, os, re, sys, zipfile

CORPUS = "/c/sandbox/workdir/sample-files"
CHART = re.compile(r"^(xl|word|ppt)/charts/chart\d+\.xml$", re.I)
ODF_CHART = re.compile(r"^Object ?\d+/content\.xml$", re.I)


def parts(path):
    try:
        with zipfile.ZipFile(path) as z:
            for n in z.namelist():
                if CHART.match(n) or ODF_CHART.match(n):
                    yield n, z.read(n).decode("utf-8", "replace")
    except Exception:
        return


def has_labelled_category_axis(xml):
    for ax in re.findall(r"<c:catAx>.*?</c:catAx>|<c:dateAx>.*?</c:dateAx>", xml, re.S):
        if re.search(r'<c:delete val="1"', ax):
            continue
        if re.search(r'<c:tickLblPos val="none"', ax):
            continue
        return True
    # ODF: a chart:axis with dimension x that carries labels.
    return bool(re.search(r'chart:dimension="x"', xml))


rows = []
with open(os.path.join(CORPUS, "MANIFEST.tsv"), encoding="utf-8") as fh:
    fh.readline()
    for line in fh:
        rows.append(line.rstrip("\n").split("\t"))

docs = collections.defaultdict(set)
partn = collections.Counter()
binary = collections.Counter()
seen = 0
for f in rows:
    family, path, ext, status = f[0], f[2], f[3], f[7]
    full = os.path.join(CORPUS, path)
    if not os.path.exists(full):
        print("MISSING", path, file=sys.stderr)
        continue
    seen += 1
    if ext.lower() in ("xls", "ppt", "doc"):
        try:
            blob = open(full, "rb").read()
        except Exception:
            continue
        # The BIFF `Chart` record — rt=0x1002, cb=16 — which opens a chart substream and is
        # the only record that does. Matching the substream BOF's `vt=0x0020` instead matches
        # almost every binary document in the corpus and is worth nothing.
        if b"\x02\x10\x10\x00" in blob:
            binary[family] += 1
        continue
    n = sum(1 for _, xml in parts(full) if has_labelled_category_axis(xml))
    if n:
        docs[family].add(path)
        partn[family] += n

if seen != len(rows):
    sys.exit("refusing to summarise: %d of %d manifest rows answered" % (seen, len(rows)))

print("manifest rows %d" % len(rows))
for family in sorted(set(list(docs) + list(binary))):
    print("  %-7s %3d documents / %3d parts with a labelled category axis"
          " (+ %d binary-format documents holding a chart substream)"
          % (family, len(docs[family]), partn[family], binary[family]))
